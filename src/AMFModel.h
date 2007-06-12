#pragma once
#include <string>
#include <vector>

#include <Tiki/tiki.h>
#include <Tiki/plxcompat.h>
#include <Tiki/gl.h>
#include <Tiki/texture.h>

#include "Tiki/drawable.h"

typedef Tiki::Math::Vector Vector3f;

using namespace std;
using namespace Tiki::GL;

class AMFModel {
	class AMFVertex {
	public:
		Vector3f position;
		Vector3f normal;
		Vector3f tangent;
		Vector3f binormal;

		void read(ifstream& infile);
	};

	class AMFTexCoord {
	public:
		float u, v;

		void read(ifstream& infile);
	};

	class AMFFace {
	public:
		AMFVertex vertices[3];
		AMFTexCoord texCoords[3];

		void read(ifstream& infile);
	};

	class AMFFrame {
	public:
		vector<AMFFace> faces;

		AMFFrame();
		~AMFFrame();
		void read(ifstream& infile, const int totalFaces);
	};

	class AMFAnimation {
	public:
		string name;
		int totalFrames;
		vector<AMFFrame> frames;

		void read(ifstream& infile, const int totalFaces);
	};

public:
	AMFModel(void);
	~AMFModel(void);

	void setFilename(const string& modelFilename);
	void load();

	int getAnimationIndex(const string& animationName) const;
	
	void draw(const Vector3f& position, const int animation, const Vector3f& color, const unsigned char height) const;

	Texture *getSkin() const;
	int getAnimationFrames(int animationIndex) const;
	int getTotalVertices() const;

	bool isValid;

private:
	//file data
	string filename;

	//model data
	int version;
	int totalFaces;
	Texture *skin;

	//OpenGL display lists
	GLuint *displayLists;
	
	int totalAnimations;
	vector<AMFAnimation> animations;
};

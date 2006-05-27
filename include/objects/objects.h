/* objects.h - ZZT objects
 * Copyright (c) 2000-2006 Sam Steele
 *
 * This file is part of DreamZZT.
 *
 * DreamZZT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * DreamZZT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */ 

//TRANSPORTER
#define ZZT_TRANSPORTER_SHAPE 0xE8
#define ZZT_TRANSPORTER_NAME "transporter"
#define ZZT_TRANSPORTER_FLAGS F_OBJECT
#define ZZT_TRANSPORTER_CLASS Transporter

class Transporter : public ZZTObject {
public:
	Transporter(int type, int x, int y, int shape, int flags, std::string name) : ZZTObject(type, x, y, shape, flags, name) { }
	void create();
	void update();
	void message(ZZTObject *them, std::string msg);
private:
	int m_counter;
	int m_anim;
};
	
//BOMB
#define ZZT_BOMB_SHAPE 0x0B
#define ZZT_BOMB_NAME "bomb"
#define ZZT_BOMB_FLAGS F_NONE
#define ZZT_BOMB_CLASS Bomb

class Bomb : public ZZTObject {
public:
	Bomb(int type, int x, int y, int shape, int flags, std::string name) : ZZTObject(type, x, y, shape, flags, name) { 
		m_counter = 0;
	}
	void create();
	void update();
	void message(ZZTObject *them, std::string msg);
private:
	int m_counter;
};

//EXPLOSION
#define ZZT_EXPLOSION_SHAPE 177
#define ZZT_EXPLOSION_NAME "explosion"
#define ZZT_EXPLOSION_FLAGS F_NONE
#define ZZT_EXPLOSION_CLASS Explosion

class Explosion : public ZZTObject {
public:
	Explosion(int type, int x, int y, int shape, int flags, std::string name) : ZZTObject(type, x, y, shape, flags, name) { }
	void create();
	void update();
private:
	int m_counter;
};

//BULLET
#define ZZT_BULLET_SHAPE (unsigned char)249
#define ZZT_BULLET_NAME "bullet"
#define ZZT_BULLET_FLAGS F_OBJECT
#define ZZT_BULLET_CLASS Bullet
#define BULLET_DAMAGE 10

class Bullet : public ZZTObject {
public:
	Bullet(int type, int x, int y, int shape, int flags, std::string name) : ZZTObject(type, x, y, shape, flags, name) { 
		m_fg = 15;
		m_bg = 0;
		m_owner = 0;
	}
	void setParam(int arg, int val);
	void update();
	void message(ZZTObject *them, std::string msg);
private:
	int m_owner;
};

//PUSHER
#define ZZT_PUSHER_SHAPE 227
#define ZZT_PUSHER_NAME "pusher"
#define ZZT_PUSHER_FLAGS F_OBJECT
#define ZZT_PUSHER_CLASS Pusher

class Pusher : public ZZTObject {
public:
	Pusher(int type, int x, int y, int shape, int flags, std::string name) : ZZTObject(type, x, y, shape, flags, name) { }
	void create();
	void update();
};

//SLIDER NS
#define ZZT_SLIDER_NS_SHAPE 0x12
#define ZZT_SLIDER_NS_NAME "sliderns"
#define ZZT_SLIDER_NS_FLAGS F_PUSHABLE
#define ZZT_SLIDER_NS_CLASS ZZTObject

//SLIDER EW
#define ZZT_SLIDER_EW_SHAPE 0x1D
#define ZZT_SLIDER_EW_NAME "sliderew"
#define ZZT_SLIDER_EW_FLAGS F_PUSHABLE
#define ZZT_SLIDER_EW_CLASS ZZTObject

//CONVEYER CW
#define ZZT_CONVEYER_CW_SHAPE '/'
#define ZZT_CONVEYER_CW_NAME "conveyercw"
#define ZZT_CONVEYER_CW_FLAGS F_NONE
#define ZZT_CONVEYER_CW_CLASS Conveyer

//CONVEYER CCW
#define ZZT_CONVEYER_CCW_SHAPE '\\'
#define ZZT_CONVEYER_CCW_NAME "conveyerccw"
#define ZZT_CONVEYER_CCW_FLAGS F_NONE
#define ZZT_CONVEYER_CCW_CLASS Conveyer

class Conveyer : public ZZTObject {
public:
	Conveyer(int type, int x, int y, int shape, int flags, std::string name) : ZZTObject(type, x, y, shape, flags, name) { 
		m_animIndex = 0;
	}
	void update();
private:
	int cw(ZZTObject *them);
	int ccw(ZZTObject *them);
	
	int m_animIndex;
};

//OBJECT
#define ZZT_OBJECT_SHAPE 'O'
#define ZZT_OBJECT_NAME "object"
#define ZZT_OBJECT_FLAGS F_OBJECT
#define ZZT_OBJECT_CLASS ZZTOOP

class ZZTOOP : public ZZTObject {
public:
	ZZTOOP(int type, int x, int y, int shape, int flags, std::string name) : ZZTObject(type, x, y, shape, flags, name) { 
		m_walk = IDLE;
	}
	void setParam(int arg, int val);
	void create();
	void update();
	void message(ZZTObject *them, std::string msg);
	
	void zzt_goto(std::string label);
	void send(std::string cmd);
	void zap(std::string label);
	void restore(std::string label);
	std::string get_zztobj_name();
	void exec(std::string cmd);
protected:
	int playedLast,goagain;
	direction m_walk;
};

//SCROLL
#define ZZT_SCROLL_SHAPE 0xE8
#define ZZT_SCROLL_NAME "scroll"
#define ZZT_SCROLL_FLAGS F_ITEM|F_OBJECT
#define ZZT_SCROLL_CLASS Scroll

class Scroll : public ZZTOOP {
public:
	Scroll(int type, int x, int y, int shape, int flags, std::string name) : ZZTOOP(type, x, y, shape, flags, name) { }
	void message(ZZTObject *them, std::string msg);
	void update();
};

//PASSAGE
#define ZZT_PASSAGE_SHAPE 0xF0
#define ZZT_PASSAGE_NAME "passage"
#define ZZT_PASSAGE_FLAGS F_GLOW|F_OBJECT
#define ZZT_PASSAGE_CLASS Passage

class Passage : public ZZTObject {
public:
	Passage(int type, int x, int y, int shape, int flags, std::string name) : ZZTObject(type, x, y, shape, flags, name) { }
	void setParam(int arg, int val);
	void create();
	void message(ZZTObject *them, std::string msg);
private:
	int m_dest;
};

//DUPLICATOR
#define ZZT_DUPLICATOR_SHAPE '.'
#define ZZT_DUPLICATOR_NAME "duplicator"
#define ZZT_DUPLICATOR_FLAGS F_OBJECT
#define ZZT_DUPLICATOR_CLASS Duplicator

class Duplicator : public ZZTObject {
public:
	Duplicator(int type, int x, int y, int shape, int flags, std::string name) : ZZTObject(type, x, y, shape, flags, name) { 
		m_animIndex = 0;
		m_rate = 0;
	}
	void setParam(int arg, int val);
	void create();
	void update();
private:
	int m_animIndex;
	int m_rate;
};

//PLAYER
#define ZZT_PLAYER_SHAPE 1
#define ZZT_PLAYER_NAME "player"
#define ZZT_PLAYER_FLAGS F_PUSHABLE | F_GLOW |F_OBJECT
#define ZZT_PLAYER_CLASS Player

class Player : public ZZTOOP {
public:
	Player(int type, int x, int y, int shape, int flags, std::string name) : ZZTOOP(type, x, y, shape, flags, name) {
		printf("Player created at %i, %i\n",x,y);
	}
	void create();
	void update();
	void message(ZZTObject *them, std::string msg);
	void processEvent(const Tiki::Hid::Event & evt);
};
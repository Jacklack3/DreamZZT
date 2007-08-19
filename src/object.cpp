/* object.cpp - ZZTObject base class
 * Copyright (C) 2000 - 2007 Sam Steele
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

#include <Tiki/tiki.h>
#include <Tiki/hid.h>
#include "console.h"
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>

using namespace Tiki;
using namespace Tiki::Hid;
using namespace Tiki::Audio;
using namespace Tiki::GL;
using namespace std;

#include "object.h"
#include "board.h"
#include "word.h"
#include "sound.h"
#include "task.h"

#ifdef USE_3DMODEL
#include "GraphicsLayer.h"

extern GraphicsLayer *gl;
#endif

extern int zztascii[];
extern Player *player;
extern ZZTMusicStream *zm;
extern struct world_header world;
extern ConsoleText *ct;
extern ConsoleText *st;
extern struct board_info_node *currentbrd;

int dir_to_delta[5] = { 0,-1,1,-1,1 };
extern struct board_info_node *currentbrd;

int id_pool=0;

direction ZZTObject::opposite(direction dir) {
	if(dir==LEFT) {
		return RIGHT;
	}
	if(dir==RIGHT) {
		return LEFT;
	}
	if(dir==UP) {
		return DOWN;
	}
	if(dir==DOWN) {
		return UP;
	}
	return IDLE;
}

direction ZZTObject::clockwise(direction dir) {
	if(dir==LEFT) {
		return UP;
	}
	if(dir==RIGHT) {
		return DOWN;
	}
	if(dir==UP) {
		return RIGHT;
	}
	if(dir==DOWN) {
		return LEFT;
	}
	return IDLE;
}

direction ZZTObject::toward(ZZTObject *them) {
	//relative direction between me and them
	direction dirx=IDLE,diry=IDLE;
	Vector dist = m_position - them->position();

	if(dist.x < 0) {
		dirx=RIGHT;
	}
	if(dist.x > 0) {
		dirx=LEFT;
	}
	if(dist.y < 0) {
		diry=DOWN;
	}
	if(dist.y > 0) {
		diry=UP;
	}
	if(dist.x<0) {
		dist.x*=-1;
	}
	if(dist.y<0) {
		dist.y*=-1;
	}
	if(abs((int)dist.x)>abs((int)dist.y)) {
		if(!::is_empty(currentbrd, (int)(m_position.x + dir_to_delta[dirx]), (int)m_position.y) &&
		        (diry!=IDLE && ::is_empty(currentbrd, (int)m_position.x, (int)(m_position.y + dir_to_delta[diry]))))
			return diry;
		else
			return dirx;
	} else {
		if(!::is_empty(currentbrd, (int)m_position.x, (int)(m_position.y + dir_to_delta[diry])) &&
		        (dirx!=IDLE && ::is_empty(currentbrd, (int)(m_position.x + dir_to_delta[diry]), (int)m_position.y)))
			return dirx;
		else
			return diry;
	}
}

int ZZTObject::distance(ZZTObject *them) {
	//distance between me and them
	Vector dist = m_position - them->position();

	if(dist.x<0) {
		dist.x*=-1;
	}
	if(dist.y<0) {
		dist.y*=-1;
	}
	return int(dist.x + dist.y);
}

int ZZTObject::dist_x(ZZTObject *them) {
	//distance between me and them (x axis only)
	Vector dist = m_position - them->position();

	if(dist.x<0) {
		dist.x*=-1;
	}
	if(dist.y<0) {
		dist.y*=-1;
	}
	return (int)dist.x;
}

int ZZTObject::dist_y(ZZTObject *them) {
	//distance between me and them (x axis only)
	Vector dist = m_position - them->position();

	if(dist.x<0) {
		dist.x*=-1;
	}
	if(dist.y<0) {
		dist.y*=-1;
	}
	return (int)dist.y;
}

int dist(int x1, int y1, int x2, int y2) {
	int a,b;
	a=(x1-x2)*(x1-x2);
	b=(y1-y2)*(y1-y2);
	return (int)sqrt((float)a+b);
}

bool ZZTObject::is_empty(direction d, bool ignorePlayer) {
	switch(d) {
	case UP:
		return !!::is_empty(currentbrd, (int)m_position.x, (int)m_position.y - 1, ignorePlayer);
	case DOWN:
		return !!::is_empty(currentbrd, (int)m_position.x, (int)m_position.y + 1, ignorePlayer);
	case LEFT:
		return !!::is_empty(currentbrd, (int)m_position.x - 1, (int)m_position.y, ignorePlayer);
	case RIGHT:
		return !!::is_empty(currentbrd, (int)m_position.x + 1, (int)m_position.y, ignorePlayer);
	}
	return false;
}

ZZTObject *ZZTObject::create_object(int type, direction d) {
	ZZTObject *o = ::create_object(type);
	switch(d) {
	case UP:
		o->setPosition(m_position + Vector(0, -1, 0));
		break;
	case DOWN:
		o->setPosition(m_position + Vector(0, 1, 0));
		break;
	case LEFT:
		o->setPosition(m_position + Vector(-1, 0, 0));
		break;
	case RIGHT:
		o->setPosition(m_position + Vector(1, 0, 0));
		break;
	default:
		delete o;
		o = NULL;
	}
	return o;
}

ZZTObject *ZZTObject::get
	(direction d) {
	int dx=0,dy=0;

	switch(d) {
	case UP:
		dx=0;
		dy=-1;
		break;
	case DOWN:
		dx=0;
		dy=1;
		break;
	case LEFT:
		dx=-1;
		dy=0;
		break;
	case RIGHT:
		dx=1;
		dy=0;
		break;
	case IDLE:
	case SHOOTING:
		return NULL;
	}

	if(m_position.x + dx >= 0 && m_position.x + dx < BOARD_X && m_position.y + dy >= 0 && m_position.y + dy < BOARD_Y) {
		return currentbrd->board[(int)m_position.x + dx][(int)m_position.y + dy].obj;
	}

	return NULL;
}

void ZZTObject::remove() {
	remove_from_board(m_board, this);
}

bool ZZTObject::move(enum direction dir, bool trying, bool origin) {
	//move in the specified direction
	ZZTObject *them=NULL;
	int x,y,oldx,oldy,suc=0;

	if(m_type==ZZT_SLIDER_NS&&(dir==LEFT||dir==RIGHT)) {
		return 0;
	}
	if(m_type==ZZT_SLIDER_EW&&(dir==UP||dir==DOWN)) {
		return 0;
	}
	oldx = (int)m_position.x;
	oldy = (int)m_position.y;
	x = (int)m_position.x;
	y = (int)m_position.y;
	switch(dir) {
	case LEFT:
		x = (int)m_position.x - 1;
		break;
	case RIGHT:
		x = (int)m_position.x + 1;
		break;
	case UP:
		y = (int)m_position.y - 1;
		break;
	case DOWN:
		y = (int)m_position.y + 1;
		break;
	case IDLE:
		return true;
	default:
		printf("Unknown direction %i\n",dir);
		break;
	}
	if(x<0) {
		x=0;
	}
	if(y<0) {
		y=0;
	}
	if(x>=BOARD_X) {
		x=BOARD_X-1;
	}
	if(y>=BOARD_Y) {
		y=BOARD_Y-1;
	}
	if((m_type != ZZT_SHARK && is_empty(dir)) || ((m_type==ZZT_BULLET || m_type==ZZT_SHARK) && m_board->board[x][y].obj->type()==ZZT_WATER)) {
		m_position.x = (float)x;
		m_position.y = (float)y;
		m_heading = dir;
		if(!m_board->board[x][y].obj->isValid())
			printf("Warning: putting invalid object under %i,%i\n",x,y);
		m_board->board[x][y].under=m_board->board[x][y].obj;
		m_board->board[x][y].obj=this;
		m_bg=m_board->board[x][y].under->bg();
		m_board->board[oldx][oldy].obj=m_board->board[oldx][oldy].under;
		if(m_board->board[oldx][oldy].obj != NULL && !m_board->board[oldx][oldy].obj->isValid()) {
			printf("Warning: putting invalid object at %i,%i\n",oldx,oldy);
			m_board->board[oldx][oldy].obj = NULL;
		}
		m_board->board[oldx][oldy].under=NULL;
		if(m_board->board[oldx][oldy].obj==NULL || !m_board->board[oldx][oldy].obj->isValid()) {
			m_board->board[oldx][oldy].obj=::create_object(ZZT_EMPTY);
			m_board->board[oldx][oldy].obj->setPosition(Vector(oldx, oldy, 0));
		}
		m_board->board[x][y].obj->draw();
		m_board->board[oldx][oldy].obj->draw();
		suc = 1;
	} else {
		them=m_board->board[x][y].obj;
		if(them!=NULL && them!=this && them->isValid() == 1) {
			if(them->flags()&F_ITEM && m_name == "player") {
				them->message(this,"get");
				if(is_empty(dir)) {
					suc=1;
					task_get(them);
				}
			} else if(suc==0 && them->flags()&F_PUSHABLE && m_name!="bullet" && (m_flags&F_PUSHER || !origin) ) {
				if(them->move(dir,trying,false)) {
					suc=1;
				} else {
					them->message(this,"crush");
					if(is_empty(dir)) {
						suc=1;
						task_get(them);
					}
				}
			}
			if(suc==0) {
				if(!trying && !(m_flags & F_SLEEPING))
					message(them,"thud");
				if(!trying && !(them->flags() & F_SLEEPING) && m_type != ZZT_BULLET) {
					them->message(this,"touch");
					task_touch(them);
				}
			} else {
				move(dir);
			}
		} else {
			if(!(m_flags & F_SLEEPING))
				message(this,"thud");
		}
	}

	return !!suc;
}

std::string ZZTObject::int_to_color(int color) {
	switch(color) {
	case 0:
	case 8:
		return "black";
		break;
	case 1:
	case 9:
		return "blue";
		break;
	case 2:
	case 10:
		return "green";
		break;
	case 3:
	case 11:
		return "cyan";
		break;
	case 4:
	case 12:
		return "red";
		break;
	case 5:
	case 13:
		return "purple";
		break;
	case 6:
	case 14:
		return "yellow";
		break;
	case 7:
	case 15:
		return "white";
		break;
	}
	return "unknown";
}

ZZTObject::ZZTObject() {
	m_position.x = 0.0f;
	m_position.y = 0.0f;
	m_step.x = 0.0f;
	m_step.y = 0.0f;
	m_heading = IDLE;
	m_type = 0;
	m_cycle=0;
	m_flags=0;
	m_shape='!';
	m_color=&m_fg;
	m_fg=15;
	m_bg=0;
	m_name="";
	m_updated=0;
	m_pushed=0;
	m_progpos=0;
	m_prog="";
	m_proglen=0;
	m_highlighted = false;
	m_modelName="";
	m_height=0;
	m_board=currentbrd;
	m_isValid = true;
}

void ZZTObject::inherit(ZZTObject *o) {
	if(o->type() != ZZT_EMPTY) {
		setParam(1,o->param(1));
		setParam(2,o->param(2));
		setParam(3,o->param(3));
		setParam(4,o->param(4));
		setFg(o->fg());
		setBg(o->bg());
		setCycle(o->cycle());
		setColor(o->color());
	}
	if(o->type()==ZZT_OBJECT) {
		setShape(o->shape());
		setProg(o->prog(),o->progLen(),o->progPos());
	}
}

extern bool debug_show_objects;
extern int disp_off_x;
extern int disp_off_y;

void ZZTObject::draw() {
	float a,b;
	Vector dist(0,0,0);

	if(m_position.x - disp_off_x < 0 || m_position.x - disp_off_x >= ct->getCols() || m_position.y - disp_off_y < 0 || m_position.y - disp_off_y >= ct->getRows())
		return;

	if(m_type == ZZT_OBJECT && debug_show_objects) {
		ct->putColor((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, (HIGH_INTENSITY | ((rand() % 6)+1)));
		ct->putChar((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, (rand()%125) + 129);
		return;
	}

	if(m_highlighted && m_flash++ < 30) {
		ct->putColor((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, WHITE);
		ct->putChar((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, 0xDB);
		return;
	}

	if(player!=NULL)
		dist = position() - player->position();

	a=(dist.x)*(dist.x)/2.0f;
	b=(dist.y)*(dist.y);

	if(m_board->dark && world.editing == 0 && !(m_flags&F_GLOW) && (world.torch_cycle<1 || (b==(5*5) || sqrt(a+b) > 5))) {
		int color = (HIGH_INTENSITY | BLACK);
		ct->putColor((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, color);
		ct->putChar((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, 177);
#ifdef USE_3DMODEL

		gl->clear((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y);
#endif

	} else if(m_board->dark && world.editing == 0 && !(m_flags&F_GLOW) && (world.torch_cycle<1 || (b==(4*4) || sqrt(a+b) > 4))) {
		int color = (HIGH_INTENSITY | BLACK);
#ifdef USE_3DMODEL

		if(!gl->put((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, color, m_height, m_modelName, m_highlighted && (m_flash < 30))) {
#endif
			ct->putColor((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, color);
			ct->putChar((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, m_shape);
#ifdef USE_3DMODEL

		}
#endif

	} else if(m_board->dark && world.editing == 0 && !(m_flags&F_GLOW) && (world.torch_cycle<1 || (b==(3*3) || sqrt(a+b) > 3))) {
		int color = m_fg%8;
#ifdef USE_3DMODEL

		if(!gl->put((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, color, m_height, m_modelName, m_highlighted && (m_flash < 30))) {
#endif
			ct->putColor((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, color);
			ct->putChar((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, m_shape);
#ifdef USE_3DMODEL

		}
#endif

	} else {
		if(m_position.x > BOARD_X) {
			int color = ((m_fg > 7) ? HIGH_INTENSITY : 0) | (m_fg%8) | (m_bg << 8);
			st->putColor((int)m_position.x - BOARD_X, (int)m_position.y, color);
			if(world.editing==1 && m_type == ZZT_INVISIBLE) {
#ifdef USE_3DMODEL
				if(!gl->put((int)m_position.x - BOARD_X, (int)m_position.y, color, m_height, m_modelName, m_highlighted && (m_flash < 30)))
#endif

					st->putChar((int)m_position.x - BOARD_X, (int)m_position.y, 0xB0);
			} else {
#ifdef USE_3DMODEL
				if(!gl->put((int)m_position.x - BOARD_X, (int)m_position.y, color, m_height, m_modelName, m_highlighted && (m_flash < 30)))
#endif

					st->putChar((int)m_position.x - BOARD_X, (int)m_position.y, m_shape);
			}
		} else {
			int color = ((m_fg > 7) ? HIGH_INTENSITY : 0) | (m_fg%8) | (m_bg << 8);
			ct->putColor((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, ((m_fg > 7) ? HIGH_INTENSITY : 0) | (m_fg%8) | (m_bg << 8));
			if(world.editing==1 && m_type == ZZT_INVISIBLE) {
				ct->putChar((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, 0xB0);
			} else {
#ifdef USE_3DMODEL
				if(!gl->put((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, color, m_height, m_modelName, m_highlighted && (m_flash < 30)))
#endif

					ct->putChar((int)m_position.x - disp_off_x, (int)m_position.y - disp_off_y, m_shape);
			}
		}
	}

	if(m_flash > 40)
		m_flash = 0;
}

void ZZTObject::edit() {
	TUIWindow w(m_name);

	addEditWidgets(&w);
	w.addWidget(new TUINumericInput("Cycle                ",&m_cycle,0,255));
	w.addWidget(new TUILabel(""));
	w.addWidget(new TUIHyperLink("editprog","Edit Program"));
	w.addWidget(new TUILabel(""));
	w.addWidget(new TUILabel("             Advanced Tweaking",true));
	w.addWidget(new TUINumericInput("X Step  ",&m_step.x,-128,127));
	w.addWidget(new TUINumericInput("Y Step  ",&m_step.y,-128,127));

	w.doMenu();
	if(w.getLabel() == "editprog") {
		TUIWindow e("Edit Program");
		e.addWidget(new TUITextInput("",&m_prog,false,true));
		e.doMenu();
		m_proglen = m_prog.length();
	}
	create();
}

std::map <int, ZZTObject *> ZZTObjectTypes;

void buildZZTObjectTypes() {
	ZZTObjectTypes[ZZT_EMPTY] = new Empty;//00
	ZZTObjectTypes[ZZT_EDGE] = NULL;//01
	ZZTObjectTypes[ZZT_EXPLOSION] = new Explosion;//02
	ZZTObjectTypes[ZZT_PLAYER] = new Player;//04
	ZZTObjectTypes[ZZT_AMMO] = new Ammo;//05
	ZZTObjectTypes[ZZT_TORCH] = new Torch;//06
	ZZTObjectTypes[ZZT_GEM] = new Gem;//07
	ZZTObjectTypes[ZZT_KEY] = new Key;//08
	ZZTObjectTypes[ZZT_DOOR] = new Door;//09
	ZZTObjectTypes[ZZT_SCROLL] = new Scroll;//0A
	ZZTObjectTypes[ZZT_PASSAGE] = new Passage;//0B
	ZZTObjectTypes[ZZT_DUPLICATOR] = new Duplicator;//0C
	ZZTObjectTypes[ZZT_BOMB] = new Bomb;//0D
	ZZTObjectTypes[ZZT_ENERGIZER] = new Energizer;//0E
	ZZTObjectTypes[ZZT_STAR] = NULL;//0F
	ZZTObjectTypes[ZZT_CONVEYER_CW] = new ConveyerCW; //10
	ZZTObjectTypes[ZZT_CONVEYER_CCW] = new ConveyerCCW; //11
	ZZTObjectTypes[ZZT_BULLET] = new Bullet; //12, 45
	ZZTObjectTypes[ZZT_WATER] = new Water;//13
	ZZTObjectTypes[ZZT_FOREST] = new Forest;//14
	ZZTObjectTypes[ZZT_SOLID] = new Solid;//15
	ZZTObjectTypes[ZZT_NORMAL] = new Normal;//16
	ZZTObjectTypes[ZZT_BREAKABLE] = new Breakable;//17
	ZZTObjectTypes[ZZT_BOULDER] = new Boulder;//18
	ZZTObjectTypes[ZZT_SLIDER_NS] = new SliderNS;//19
	ZZTObjectTypes[ZZT_SLIDER_EW] = new SliderEW;//1A
	ZZTObjectTypes[ZZT_FAKE] = new Fake;//1B
	ZZTObjectTypes[ZZT_INVISIBLE] = new Invisible;//1C
	ZZTObjectTypes[ZZT_BLINK] = new Blink;//1D
	ZZTObjectTypes[ZZT_TRANSPORTER] = new Transporter;//1E
	ZZTObjectTypes[ZZT_LINE] = new Line;//1F
	ZZTObjectTypes[ZZT_RICOCHET] = new Ricochet;//20
	ZZTObjectTypes[ZZT_HORIZONTAL_BLINK] = new HorizontalLaser;//21, 46
	ZZTObjectTypes[ZZT_BEAR] = new Bear;//22
	ZZTObjectTypes[ZZT_RUFFIAN] = new Ruffian;//23
	ZZTObjectTypes[ZZT_OBJECT] = new ZZTOOP;//24
	ZZTObjectTypes[ZZT_SLIME] = new Slime;//25
	ZZTObjectTypes[ZZT_SHARK] = new Shark;//26
	ZZTObjectTypes[ZZT_SPINNING_GUN] = new SpinningGun;//27
	ZZTObjectTypes[ZZT_PUSHER] = new Pusher;//28
	ZZTObjectTypes[ZZT_LION] = new Lion;//29
	ZZTObjectTypes[ZZT_TIGER] = new Tiger;//2A
	ZZTObjectTypes[ZZT_VERTICAL_BLINK] = new VerticalLaser;//2B, 47
	ZZTObjectTypes[ZZT_CENTIPEDE_HEAD] = new CentipedeHead;//2C
	ZZTObjectTypes[ZZT_CENTIPEDE_BODY] = new CentipedeBody;//2D
	if(world.magic == MAGIC_SZT) {
		ZZTObjectTypes[SZT_FLOOR]= new Floor;//FF)
		ZZTObjectTypes[SZT_WATER_N] = new RiverN;//FF)
		ZZTObjectTypes[SZT_WATER_S] = new RiverS;//FF)
		ZZTObjectTypes[SZT_WATER_W] = new RiverW;//FF)
		ZZTObjectTypes[SZT_WATER_E] = new RiverE;//FF)
	}
	ZZTObjectTypes[ZZT_BLUE_TEXT] = new BlueText;//2F :] = new //49)
	ZZTObjectTypes[ZZT_GREEN_TEXT] = new GreenText;//30 :] = new //4A)
	ZZTObjectTypes[ZZT_CYAN_TEXT] = new CyanText;//31 :] = new //4B)
	ZZTObjectTypes[ZZT_RED_TEXT] = new RedText;//32 :] = new //4C)
	ZZTObjectTypes[ZZT_PURPLE_TEXT] = new PurpleText;//33 :] = new //4D)
	ZZTObjectTypes[ZZT_YELLOW_TEXT] = new YellowText;//34 :] = new //4E)
	ZZTObjectTypes[ZZT_WHITE_TEXT] = new WhiteText;//35 :] = new //4F)
/*	ZZTObjectTypes[ZZT_EMPTY] = new //36	 | (set in colour byte) | White blinking text
	ZZTObjectTypes[ZZT_EMPTY] = new //37	 | (set in colour byte) | Blue blinking text
	ZZTObjectTypes[ZZT_EMPTY] = new //38	 | (set in colour byte) | Green blinking text
	ZZTObjectTypes[ZZT_EMPTY] = new //39	 | (set in colour byte) | Cyan blinking text
	ZZTObjectTypes[ZZT_EMPTY] = new //3A	 | (set in colour byte) | Red blinking text
	ZZTObjectTypes[ZZT_EMPTY] = new //3B	 | (set in colour byte) | Purple blinking text
	ZZTObjectTypes[ZZT_EMPTY] = new //3C	 | (set in colour byte) | Yellow blinking text
	ZZTObjectTypes[ZZT_EMPTY] = new //3D	 | (set in colour byte) | Grey blinking text*/
	ZZTObjectTypes[ZZT_LUA] = new ZZTLUA;//60
}

ZZTObject *create_object(int type) {
	if(ZZTObjectTypes.empty()) buildZZTObjectTypes();
	
	if(ZZTObjectTypes.find(type) != ZZTObjectTypes.end()) {
		return ZZTObjectTypes[type]->alloc();
	} else {
		return NULL;
	}
}
/* enemy.cpp - ZZT Enemys
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace Tiki;
using namespace Tiki::GL;
using namespace Tiki::Hid;
using namespace Tiki::Audio;

#include "object.h"
#include "board.h"
#include "status.h"
#include "sound.h"
#include "debug.h"
#include "task.h"

extern ZZTMusicStream *zm;

extern struct board_info_node *currentbrd;
extern struct world_header world;
extern Player *player;

void Lion::update() {
	if(player != NULL && rand()%10<m_intel) {
		move(toward(player));
	} else {
		move((direction)((rand()%4)+1));
	}
}

Bear::Bear() {
	m_shape = ((world.magic == MAGIC_SZT) ? 0xEB : 0x99);
	m_name = "bear";
	m_flags = F_PUSHABLE|F_OBJECT;
	m_intel = 8;
}

void Bear::update() {
	if(player != NULL && (dist_x(player) < (9-m_intel) || dist_y(player) < (9-m_intel))) {
		move(toward(player));
	}
}

void Ruffian::setParam(int arg, unsigned char val) {
	if(arg == 2)
		m_rest = val;

	Enemy::setParam(arg,val);
}

unsigned char Ruffian::param(int arg) {
	if(arg == 2)
		return m_rest;

	return Enemy::param(arg);
}

void Ruffian::update() {
	if(m_moveCounter-- > 0) {
		move(m_heading);
	} else if(m_restCounter-- <= 0) {
		if(rand()%9 < m_intel) {
			m_heading = toward(player);
		} else {
			m_heading = (direction)(((rand()%4)+1));
		}
		m_moveCounter = rand() % 10;
	} else if(m_moveCounter <= 0) {
		m_restCounter = 6 + (rand() % m_rest);
	}
}

char gun[4] = {27,24,26,25};

void SpinningGun::setParam(int arg, unsigned char val) {
	if(arg == 2)
		m_rate = val;

	Enemy::setParam(arg,val);
}

unsigned char SpinningGun::param(int arg) {
	if(arg == 2)
		return m_rate;

	return Enemy::param(arg);
}

void SpinningGun::update() {
	m_animIndex++;
	m_animIndex%=4;
	m_shape=gun[m_animIndex];
	draw();

	if(player != NULL && rand()%10<m_rate) {
		if(rand()%10<m_intel) {
			if(((m_position - player->position()).x >= -2 && (m_position - player->position()).x <= 2) ||
			        ((m_position - player->position()).y >= -2 && (m_position - player->position()).y <= 2)) {
				shoot(toward(player));
			}
		} else {
			if(rand()%(m_intel+2)==0) {
				shoot(toward(player));
			}
		}
	}
}

void Tiger::setParam(int arg, unsigned char val) {
	if(arg == 2)
		m_rate = val;

	Enemy::setParam(arg,val);
}

unsigned char Tiger::param(int arg) {
	if(arg == 2)
		return m_rate;

	return Enemy::param(arg);
}

void Tiger::update() {
	Lion::update();

	if(rand()%10<m_rate) {
		if(player != NULL && (((m_position - player->position()).x >= -1 && (m_position - player->position()).x <= 1) ||
		                      ((m_position - player->position()).y >= -1 && (m_position - player->position()).y <= 1))) {
			shoot(toward(player));
		}
	}
}

void Roton::setParam(int arg, unsigned char val) {
	if(arg == 2)
		m_rate = val;

	Enemy::setParam(arg,val);
}

unsigned char Roton::param(int arg) {
	if(arg == 2)
		return m_rate;

	return Enemy::param(arg);
}

void Roton::update() {
	if(m_rate == 0) return;
	m_counter++;
	if(m_counter > (9 - m_rate) || m_heading == IDLE) {
		m_heading = toward(player);
		if(rand()%18 < (9-m_intel)) {
			m_heading = opposite(clockwise(m_heading));
		}
		m_counter = 0;
	}
	move(m_heading);
}

void Lion::create() {
	m_fg = 12;
}

void Tiger::create() {
	m_fg = 11;
}

void Bear::create() {
	m_fg = ((world.magic == 65535) ? 6 : 2);
}

void Ruffian::create() {
	m_fg = 13;
}

void Shark::create() {
	m_fg = 7;
}

void Bear::message(ZZTObject *them, std::string message) {
	Enemy::message(them, message);
	if(them->type() == ZZT_BREAKABLE) {
		remove_from_board(currentbrd,them);
		remove_from_board(currentbrd,this);
		if(zm!=NULL)
			zm->setTune("t+c-c-c");
		if(zm!=NULL)
			zm->start();
	}
}

void Enemy::message(ZZTObject *them, std::string message) {
	if(((message == "shot" && them->type() == ZZT_BULLET && them->param(1) == 0) || them->type()==ZZT_PLAYER || message == "bombed")) {
		if(message == "touch" || message == "thud") {
			them->message(this,"shot");
		} else {
			give_score((m_shape==((world.magic == MAGIC_SZT) ? 0xEB : 0x99))?1:2);
			draw_score();
		}
		if(zm!=NULL)
			zm->setTune("t+c---c++++c--c");
		if(zm!=NULL)
			zm->start();
		debug("\x1b[0;37mA \x1b[1;37m%s\x1b[0;37m was killed.\n",m_name.c_str());
		task_kill(this);
		remove_from_board(currentbrd,this);
	}
	if(message == "crush") {
		if(zm!=NULL)
			zm->setTune("t+c---c++++c--c");
		if(zm!=NULL)
			zm->start();
		debug("\x1b[0;37mA \x1b[1;37m%s\x1b[0;37m was crushed.\n",m_name.c_str());
		remove_from_board(currentbrd,this);
	}
}

void Slime::message(ZZTObject *them, std::string message) {
	if(message=="touch" && them->type() == ZZT_PLAYER) {
		if(zm!=NULL)
			zm->setTune("t+c---c++++c--c");
		if(zm!=NULL)
			zm->start();
		debug("\x1b[0;37mA \x1b[1;37m%s\x1b[0;37m was killed.\n",m_name.c_str());
		currentbrd->board[(int)m_position.x][(int)m_position.y].under=this;
		currentbrd->board[(int)m_position.x][(int)m_position.y].obj=::create_object(ZZT_BREAKABLE);
		currentbrd->board[(int)m_position.x][(int)m_position.y].obj->setPosition(m_position);
		currentbrd->board[(int)m_position.x][(int)m_position.y].obj->setColor(m_fg, m_bg);
		setFlag(F_DELETED);
	}
}

void Slime::create() {
	m_counter = m_rate * 2;
}

void Slime::update() {
	ZZTObject *s;

	if(m_counter == 0) {
		for(int d = (int)LEFT; d <= (int)DOWN; d++) {
			if(is_empty((direction)d)) {
				s=create_object(ZZT_SLIME, (direction)d);
				s->setParam(2,m_rate);
				s->setCycle(m_cycle);
				s->setColor(m_fg,m_bg);
				put(s, s->position().x, s->position().y);
			}
		}
		currentbrd->board[(int)m_position.x][(int)m_position.y].under=this;
		currentbrd->board[(int)m_position.x][(int)m_position.y].obj=::create_object(ZZT_BREAKABLE);
		currentbrd->board[(int)m_position.x][(int)m_position.y].obj->setPosition(m_position);
		currentbrd->board[(int)m_position.x][(int)m_position.y].obj->setColor(m_fg, m_bg);
		setFlag(F_DELETED);
	}
	m_counter--;
}

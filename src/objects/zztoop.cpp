/* zztoop.cpp - ZZT-OOP scriptable object
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

#include <Tiki/tiki.h>
#include <Tiki/hid.h>
#include <stdlib.h>
#include <string>
#include <cctype>
#include <algorithm>
#include <vector>

using namespace Tiki;
using namespace Tiki::Hid;
using namespace Tiki::Audio;
using namespace Tiki::GL;
using namespace std;

#include "sound.h"
#include "object.h"
#include "word.h"
#include "board.h"
#include "status.h"
#include "http.h"
#include "debug.h"

extern struct world_header world;
extern Player *player;
extern ConsoleText *ct;
extern ZZTMusicStream *zm;

void ZZTOOP::setParam(int arg, int val) {
	if(arg == 1) m_shape = val;
}

void ZZTOOP::create() {
  m_heading=IDLE;
}

void ZZTOOP::message(ZZTObject *them, std::string message) {
  if(!(m_flags & F_SLEEPING)) zzt_goto(message);
}

extern struct board_info_node *currentbrd;

ZZTObject *find_zztobj_by_name(int &x, int &y, std::string target) {
  ZZTObject *myobj;
  int z;
  int i,j;
	
	transform(target.begin(), target.end(), target.begin(), ::tolower);
	
  for(j=y;j<BOARD_Y;j++) {
    for(i=0;i<BOARD_X;i++) {
      if(i==0&&j==y) i=x;
      if(i>=BOARD_X) { i=0; j++; }
      myobj=currentbrd->board[i][j].obj;
      if(myobj->getType()==ZZT_OBJECT) {
				//printf("Comparing '%s' and '%s'\n",((ZZTOOP *)myobj)->get_zztobj_name().c_str(), target.c_str());
        if(((ZZTOOP *)myobj)->get_zztobj_name() == target) {
					printf("Match\n");
          x=i;
          y=j;
	        return myobj;
        }
      }
    }
  }
	//printf("No match\n");
  return NULL;
}

std::string ZZTOOP::get_zztobj_name() {
  int y;
  std::string name;

  if((m_type==ZZT_OBJECT || m_type==ZZT_SCROLL) && m_prog[0]=='@') {
    y=1;
    while(m_prog[y]!='\r' && y < m_proglen) {
			name += m_prog[y];
	    y++;
    }
		
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    
		return name;
  }
  return m_name;
}

void ZZTOOP::send(std::string cmd) {
  int x,y;
  ZZTObject *obj=NULL;
  struct board_info_node *brd;
	std::string target;
	std::string label;
	std::string name;
	
  for(x=0;x<cmd.length();x++) {
    if(cmd[x]==':') {
      break;
    }
    target += cmd[x];
  }
  
  if(x==cmd.length()) {
    zzt_goto(target);
    return;
  }
	
  x++;
	for(y=0;y<cmd.length()-x;y++) {
    label += cmd[x+y];
  }
  
  x=0;
  if(target == "all" || target == "others") {
    for(y=0;y<BOARD_Y;y++) {
      for(x=0;x<BOARD_X;x++) {
        obj=currentbrd->board[x][y].obj;
        if(obj->getType()==ZZT_OBJECT) {
          if(obj==this && target != "others") {
            ((ZZTOOP *)obj)->zzt_goto(label);
          }
        }
      }
    }
  } else {
    x=0; y=0;
    obj=find_zztobj_by_name(x,y,target);
    while(obj!=NULL) {
      ((ZZTOOP *)obj)->zzt_goto(label);
      x++;
      obj=find_zztobj_by_name(x,y,target);
    }
  }
}

void ZZTOOP::zap(std::string label) {
	std::string text;
  int x,y,newline=1,goagain=1;

	debug("%s zapping %s",get_zztobj_name().c_str(),label.c_str());
  
	for(x=0;x<m_proglen;x++) {
    switch(m_prog[x]) {
    case '\r':
      newline=1;
      break;
    case ':':
      if(newline==1) {
				y=1;
				text = "";
				
				while(m_prog[x+y]!='\r') {
					text += m_prog[x+y];
					y++;
				}
				
				if(label == text) {
					m_prog[x]='\'';
					return;
				}
				x+=y-1;
				newline=0;
      }
      break;
    default:
      newline=0;
      break;
    }
  }
}

void ZZTOOP::restore(std::string label) {
	std::string text;
  int x,y,newline=0,goagain=1;
	
	debug("%s restoring %s\n",get_zztobj_name().c_str(),label.c_str());
  
	for(x=0;x<m_proglen;x++) {
    switch(m_prog[x]) {
    case '\r':
      newline=1;
      break;
    case '\'':
      if(newline==1) {
				y=1;
				text="";
				while(m_prog[x+y]!='\r') {
					text += m_prog[x+y];
					y++;
				}
				
				if(label == text) {
					m_prog[x]=':';
					return;
				}
				x+=y-1;
				newline=0;
      }
      break;
    default:
      newline=0;
      break;
    }
  }
}

void ZZTOOP::zzt_goto(std::string label) {
	std::string text;
  int x,y,newline=0;
  if(label=="") return;

  if(label[0]=='#') {
		label.erase(0,1);
  }

	debug("\x1b[1;37m%s\x1b[0;37m recieved \x1b[1;37m%s\n",get_zztobj_name().c_str(), label.c_str());
  
	for(x=0;x<m_proglen;x++) {
    switch(m_prog[x]) {
    case '\r':
      newline=1;
      break;
    case ':':
      x++;
      if(newline==1) {
				y=0;
				text = "";
				while(m_prog[x+y]!='\r') {
					text += m_prog[x+y];
					y++;
				}
				
				//printf("Comparing labels: '%s' and '%s'\n",label.c_str(),text.c_str());
				if(label == text) {
					m_progpos=x+y;
					//printf("Position set\n");
					//printf("Next characters: '%c,%c,%c,%c\n'",m_prog[m_progpos],m_prog[m_progpos+1],m_prog[m_progpos+2],m_prog[m_progpos+3]);
					return;
				}
				x+=y;
				newline=0;
      }
      break;
    default:
      newline=0;
      break;
    }
  }
  if(label == "restart") {
    m_progpos=0;
  } else {
    //printf("Label not found\n");
  }
}

void ZZTOOP::exec(std::string text) {
	std::vector<std::string> words;
	std::string tmp;
	std::string lbl;
	std::string args;
	ZZTObject *theirobj;
  struct board_data *b;
  static int textfg=1,textbg=0;
  int i,j,in,went=0,color,color2,neg,res;
	
	transform(text.begin(), text.end(), text.begin(), ::tolower);
	
	words = wordify(text,' ');
	if(words.size() > 1) {
				args = text.substr(words[0].length() + 1);
	} else {
				args = "";
	}

	debug("\x1b[1;37m%s: \x1b[0;37m%s\n",get_zztobj_name().c_str(), text.c_str());
	
	if(words[0] == "go") {
				res=str_to_direction(args);
				if(move((direction)res,true)) {
					move((direction)res);
				} else {
					m_progpos-=(text.length()+2);
				}
				goagain=0;
	}
	else if(words[0] == "walk") {
		m_walk=str_to_direction(args);
		if(m_walk==IDLE) goagain=1;
	}
	else if(words[0] == "play") {
				if(zm != NULL) {
					if(zm->isPlaying()) {
						m_progpos-=(text.length()+2);
					} else {
						if(playedLast==0) {
							zm->setTune(args);
						} else {
							zm->appendTune(args);
						}
						goagain = 1;
						playedLast = 1;
					}
				}
	}
	else if(words[0] == "try") {
		res=str_to_direction(args);
		if(move((direction)res,true)) {
			move((direction)res);
		} else {
			zzt_goto(words[words.size() - 1]);
		}
	}
	else if(words[0] == "cycle") {
		m_cycle=atoi(args.c_str());
		if(m_cycle==0) m_cycle=1;
		goagain=1;
	}
	else if(words[0] == "put") {
		color=str_to_color(words[2]);
		if(color!=-1) words.erase(words.begin() + 2);
		if(words[1].find("w") == 0) {
			i=m_position.x-1; j=m_position.y;
		} else if(words[1].find("e") == 0) {
			i=m_position.x+1; j=m_position.y;
		} else if(words[1].find("n") == 0) {
			i=m_position.x; j=m_position.y-1;
		} else if(words[1].find("s") == 0) {
			i=m_position.x; j=m_position.y+1;
		}
				if(i>=0 && i<BOARD_X && j>=0 && j<BOARD_Y) {
					b=&currentbrd->board[i][j];
					if(str_to_obj(words[2])==-1) {
						set_msg("ERR: undefined item");
					} else {
						b->under=b->obj;
						b->obj=::create_object(str_to_obj(words[2]),i,j);
						b->obj->create();
						if(color!=-1) b->obj->setColor(color);
						if(b->obj->getBg()>7) b->obj->setBg(b->obj->getBg() - 8);
						draw_block(i,j);
					}
				}
		goagain=1;
	}
	else if(words[0] == "become") {
		color=str_to_color(words[1]);
		if(color!=-1) words.erase(words.begin() + 1);
		b=&currentbrd->board[(int)m_position.x][(int)m_position.y];
		if(str_to_obj(words[1])==-1) {
			set_msg("ERR: undefined item");
		} else {
			b->under=b->obj;
			b->obj=::create_object(str_to_obj(words[1]),i,j);
			if(color!=-1) b->obj->setColor(color);
			draw_block(m_position.x,m_position.y);
		}
		goagain=1;
	}
	else if(words[0] == "change") {
		goagain=1;
		color=str_to_color(words[1]);
		if(color!=-1) words.erase(words.begin() + 1);
				color2=str_to_color(words[2]);
				if(color2!=-1) words.erase(words.begin() + 2);
        i=0; j=0;
        do {
          res=0;
          b=get_block_by_type(str_to_obj(words[1]),i,j);
          if(b!=NULL) {
						//printf("Type: %s\nColor: (%i) (%i)\n",b->obj->name,*b->obj->color,color);
            if(color==-1 || b->obj->getColor()==color || (color>=8 && b->obj->getColor()==color-8)) {
							if(words[1] == words[2]) {
								b->obj->setColor(color2);
							} else {
								b->obj->setFlag(F_DELETED);
								if(str_to_obj(words[2])==-1) {
									set_msg("ERR: undefined item");
								} else {
									b->under=b->obj;
									//printf("Creating a %s...\n",get_word(2));
									b->obj=::create_object(str_to_obj(words[2]),i,j);
									/*b->obj->arg1=b->under->arg1;
									b->obj->arg2=b->under->arg2;
									b->obj->arg3=b->under->arg3;
									b->obj->arg4=b->under->arg4;
									b->obj->prog=b->under->prog;
									b->obj->progpos=b->under->progpos;
									b->obj->proglen=b->under->proglen;
									if(b->obj->type==ZZT_OBJECT) b->obj->shape=b->under->shape;
									b->obj->fg=b->under->fg;
									b->obj->bg=b->under->bg;*/
									b->obj->create();
									if(color!=-1) b->obj->setColor(color); 
									else if(color2!=-1) b->obj->setColor(color2);
									else b->obj->setColor(b->under->getColor());
									if(b->obj->getBg()>=8) b->obj->setBg(b->obj->getBg() - 8);
									draw_block(i,j);
									delete b->under;
									b->under=NULL;
								}
							}
            }
          }
          i++;
        } while(b!=NULL);
	}
	else if(words[0] == "__dark") {
				currentbrd->dark=atoi(words[1].c_str());
				draw_board();
	}
	else if(words[0] == "set") {
		for(i=0;i<10;i++) {
			printf("Flag[%i].len=%i\n",i,world.flag[i].len);
			if(world.flag[i].len==0) {
				printf("Storing flag in slot %i\n",i);
				strcpy((char *)world.flag[i].string,words[1].c_str());
				world.flag[i].len=words[1].length();
				break;
			}
		}
	}
	else if(words[0] == "clear") {
		for(i=0;i<10;i++) {
			if(std::string((const char *)world.flag[i].string) == words[1]) {
				world.flag[i].string[0]='\0';
				world.flag[i].len=0;
				break;
			}
		}
	}
	else if(words[0] == "if") {
		res=0;
		if(words[1] == "not") {
			neg=1;
			words.erase(words.begin() + 1);
		} else {
			neg=0;
		}
				
				if(words[2] == "then") words.erase(words.begin() + 2);
				if(words[3] == "then") words.erase(words.begin() + 3);
				
				if(words[1] == "alligned" || words[1] == "aligned") {
					if(player->getPosition().x==m_position.x || player->getPosition().y == m_position.y) res=1;
          lbl = words[2];
        } else if(words[1] == "any") {
          color=str_to_color(words[2]);
          if(color!=-1) words.erase(words.begin() + 2);
          i=0; j=0;
          do {
            b=get_block_by_type(str_to_obj(words[2]),i,j);
            if(b!=NULL) {
							if(color==-1) break;
              if(color<8 && b->obj->getColor()>=8) color+=8;
              if(color>=8 && b->obj->getColor()<8) color-=8;
            }
            i++;
          } while(b!=NULL && (color!=-1 && b->obj->getColor()!=color));
          if(b!=NULL) res=1;
          lbl = words[3];
        } else if(words[1] == "contact") {
          lbl = words[2];
          if(currentbrd->board[(int)m_position.x][(int)m_position.y-1].obj!=NULL) {
            if(currentbrd->board[(int)m_position.x][(int)m_position.y-1].obj->getType()==ZZT_PLAYER) {
              res=1;
            }
          }
          if(currentbrd->board[(int)m_position.x][(int)m_position.y+1].obj!=NULL) {
            if(currentbrd->board[(int)m_position.x][(int)m_position.y+1].obj->getType()==ZZT_PLAYER) {
              res=1;
            }
          }
          if(currentbrd->board[(int)m_position.x-1][(int)m_position.y].obj!=NULL) {
            if(currentbrd->board[(int)m_position.x-1][(int)m_position.y].obj->getType()==ZZT_PLAYER) {
              res=1;
            }
          }
          if(currentbrd->board[(int)m_position.x+1][(int)m_position.y].obj!=NULL) {
            if(currentbrd->board[(int)m_position.x+1][(int)m_position.y].obj->getType()==ZZT_PLAYER) {
              res=1;
            }
          }
        } else if(words[1] == "blocked") {
          switch(str_to_direction(text.substr((words[0] + words[1]).length() + 2))) {
						case UP:
							if(::is_empty(currentbrd,m_position.x,m_position.y-1)==0) {
								res=1;
							}
							break;
						case DOWN:
							if(::is_empty(currentbrd,m_position.x,m_position.y+1)==0) {
								res=1;
							}
							break;
						case RIGHT:
							if(::is_empty(currentbrd,m_position.x+1,m_position.y)==0) {
								res=1;
							}
							break;
						case LEFT:
							if(::is_empty(currentbrd,m_position.x-1,m_position.y)==0) {
								res=1;
							}
							break;
          }
          lbl = words[3];
        } else {
          lbl = words[2];
					if(words[1] == "dreamcast") {
#ifdef DREAMCAST
						res=1;
#else
						res=0;
#endif
					} else if (words[1] == "pc") {
#ifndef DREAMCAST
						res=1;
#else
						res=0;
#endif
					} else {
						for(i=0;i<10;i++) {
							if(std::string((const char *)world.flag[i].string) == words[1]) {
								res=1;
								break;
							}
						}
					}
        }
        if(neg==1) res=!res;
        if(res==1) send(lbl);
        goagain=1;
	}
	else if(words[0] == "bind") {
		i=0;
		j=0;
		theirobj=find_zztobj_by_name(i,j,words[1]);
		if(theirobj!=NULL)  {
			m_prog=theirobj->getProg();
			m_progpos=0;
			m_proglen=theirobj->getProgLen();
			return;
		}
		return;
	}
	else if(words[0] == "zap") {
				tmp = words[1];
				lbl = "";
				for(i=0;i<tmp.length();i++) {
					lbl += tmp[i];
					if(tmp[i]==':') break;
				}
				
				if(i<tmp.length()) {
					int x=0,y=0;
					theirobj=find_zztobj_by_name(x,y,lbl);
					while(theirobj!=NULL) {
						((ZZTOOP *)theirobj)->zap(tmp.substr(i+1));
						x++;
						theirobj=find_zztobj_by_name(x,y,lbl);
					}
				} else {
      		zap(lbl);
				}
        goagain=1;
	}
	else if(words[0] == "restore") {
				tmp = words[1];
				lbl = "";
				
				for(i=0;i<tmp.length();i++) {
					lbl += tmp[i];
					if(tmp[i]==':') break;
				}
				
				if(i<tmp.length()) {
					int x=0,y=0;
					((ZZTOOP *)find_zztobj_by_name(x,y,lbl))->restore(tmp.substr(i+1));
				} else {
      		restore(lbl);
				}
        goagain=1;
	}
	else if(words[0] == "lock") {
		m_flags|=F_SLEEPING;
		goagain=1;
	}
	else if(words[0] == "unlock") {
		m_flags&=~F_SLEEPING;
		goagain=1;
	}
	else if(words[0] == "send") {
				send(words[1]);
		goagain=1;
	}
	/*else if(words[0] == "teleport") {
		if(::is_empty(currentbrd,atoi(words[1]),atoi(words[2]))==0) {
			//set_msg("Invalid teleport.");
		} else {
			m_position.x=atoi(get_word(1));
			m_position.y=atoi(get_word(2));
		}
	}*/
	else if(words[0] == "color") {
		m_fg=atoi(words[1].c_str());
	}
	else if(words[0] == "char") {
		m_shape=atoi(words[1].c_str());
		draw();
				goagain=1;
	}
	else if(words[0] == "give") {
		if(words[1] == "ammo") {
			give_ammo(atoi(words[2].c_str()));
		}
		if(words[1] == "gems") {
			give_gems(atoi(words[2].c_str()));
		}
		if(words[1] == "torch") {
			give_torch(atoi(words[2].c_str()));
		}
		if(words[1] == "score") {
			give_score(atoi(words[2].c_str()));
		}
		if(words[1] == "health") {
			give_health(atoi(words[2].c_str()));
		}
		if(words[1] == "time") {
			give_time(atoi(words[2].c_str()));
		}
		goagain=1;
	}
	else if(words[0] == "take") {
		if(words[1] == "gems") {
			if(world.gems<atoi(words[2].c_str())) { zzt_goto(words[3]); }
			else take_gems(atoi(words[2].c_str()));
		}
		if(words[1] == "torch") {
			if(world.torches<atoi(words[2].c_str())) { zzt_goto(words[3]); }
			else take_torch(atoi(words[2].c_str()));
		}
		if(words[1] == "ammo") {
			if(world.ammo<atoi(words[2].c_str())) { zzt_goto(words[3]); }
			else take_ammo(atoi(words[2].c_str()));
		}
		if(words[1] == "score") {
			if(world.score<atoi(words[2].c_str())) { zzt_goto(words[3]); }
			else take_score(atoi(words[2].c_str()));
		}
		if(words[1] == "health") {
			if(world.health<atoi(words[2].c_str())) { zzt_goto(words[3]); }
			else take_health(atoi(words[2].c_str()));
		}
		if(words[1] == "time") {
			if(currentbrd->time<atoi(words[2].c_str())) { zzt_goto(words[3]); }
			else take_time(atoi(words[2].c_str()));
		}
		goagain=1;
	}
	else if(words[0] == "die") {
		remove_from_board(currentbrd,this);
		m_progpos=-1;
	}
	else if(words[0] == "end") {
		m_progpos=-1;
		return;
	}
	else if(words[0] == "endgame") {
		world.health=0;
		return;
	}
	else if(words[0] == "throwstar") {
	}
	else if(words[0] == "shoot") {
		shoot(str_to_direction(args));
	}
	else {
		send(words[0]);
		goagain=1;
	}	
}

void ZZTOOP::update() {
	std::string text;
  int x=0,y=0,z,newline=0,linecount=0,ty=0,i,j,went=0;
	std::string lbl;
	
	if(m_proglen<1) return;
	
	playedLast = 0;
	goagain = 1;
	
  while(goagain) {
    goagain=0;
    if(m_progpos>m_proglen || m_progpos==-1) { break; }
    text="";
    switch(m_prog[m_progpos]) {
    case ':':
      while(m_prog[m_progpos]!='\r') {
	      m_progpos++;
      }
      goagain=1;
      break;
		case '?':
      x=++m_progpos;
      y=0;
      while(m_prog[y+x]!='\r' && m_prog[y+x]!='#' && m_prog[y+x]!='?' && m_prog[y+x]!='/') {
	      text += m_prog[y+x];
	      y++;
      }
			m_progpos += y+1;
				
			debug("\x1b[1;37m%s: \x1b[0;37m%s\n",get_zztobj_name().c_str(), text.c_str());
			move(str_to_direction(text), true);			
			goagain=0;
			break;
    case '/':
      x=++m_progpos;
      y=0;
      while(m_prog[y+x]!='\r' && m_prog[y+x]!='#' && m_prog[y+x]!='?' && m_prog[y+x]!='/') {
	      text += m_prog[y+x];
	      y++;
      }
			debug("\x1b[1;37m%s: \x1b[0;37m%s\n",get_zztobj_name().c_str(), text.c_str());
			
			if(move(str_to_direction(text), true)) {
				m_progpos+=(y-1);
			} else {
				m_progpos-=2;
			}
				
			goagain=0;
			break;
    case '\'':
    case '@':
      while(m_prog[m_progpos]!='\r') {
	      m_progpos++;
      }
      goagain=1;
      break;
    case '^':
    case '#':
      x=++m_progpos;
      y=0;
      while(m_prog[y+x]!='\r') {
	      text += m_prog[y+x];
	      y++;
	      m_progpos++;
      }
      
			exec(text);	
			break;
    case '\r':
      goagain=1;
      break;
    default:
      x=m_progpos;
      y=0;
			text = "";
			
      while(m_prog[y+x]!='\0') {
	      if(m_prog[y+x]=='\r') {
	        newline=1;
	        linecount++;
	      }
	      if((newline==1 && m_progpos!=-1) && (m_prog[y+x]=='#'||m_prog[y+x]=='/'||m_prog[y+x]=='?' || m_prog[x+y]==':' || m_prog[x+y]=='\'' || m_proglen <= x+y+1)) {
	        if(text[0]!=':') {
	          if(linecount>1) {
							if(zm!=NULL) zm->start();
	            TextWindow *t= new TextWindow(ct,get_zztobj_name(),text);
							t->doMenu();
              draw_board();
              if(t->getLabel()!='\0') { zzt_goto(t->getLabel()); }
							delete t;
	          } else {
	            set_msg((char *)text.c_str());
	            goagain=1;
	          }
	        }
	        m_progpos--;
	        break;
	      }
	      text += m_prog[y+x];
        if(text[y]!='\r') newline=0;
	      y++;
	      m_progpos++;
      }
    }
    if(m_progpos>0) m_progpos++;
    if(went++>6) goagain=0;
  }
  if(m_walk!=IDLE) {
    move(m_walk);
  }
	if(zm!=NULL) zm->start();
}

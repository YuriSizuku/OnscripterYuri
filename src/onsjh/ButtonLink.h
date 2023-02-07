/* -*- C++ -*-
 * 
 *  ButtonLink.h - Base button class
 *
 *  Copyright (c) 2001-2013 Ogapee. All rights reserved.
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __BUTTON_LINK_H__
#define __BUTTON_LINK_H__

#include "AnimationInfo.h"

struct ButtonLink{
    enum { NORMAL_BUTTON     = 0,
           SPRITE_BUTTON     = 1,
           LOOKBACK_BUTTON   = 2,
           TMP_SPRITE_BUTTON = 3
    };

    ButtonLink *next;
    int button_type;
    int no;
    int sprite_no;
    char *exbtn_ctl[3];
    SDL_Rect select_rect;
    SDL_Rect image_rect;
    AnimationInfo *anim[2];
    int show_flag; // 0...show nothing, 1... show anim[0], 2 ... show anim[1]

    ButtonLink(){
        button_type = NORMAL_BUTTON;
        next = NULL;
        exbtn_ctl[0] = exbtn_ctl[1] = exbtn_ctl[2] = NULL;
        anim[0] = anim[1] = NULL;
        show_flag = 0;
    };

    ~ButtonLink(){
        if ( (button_type == NORMAL_BUTTON || button_type == TMP_SPRITE_BUTTON) && 
             anim[0] ) delete anim[0];
        for (int i=0 ; i<3 ; i++)
            if ( exbtn_ctl[i] ) delete[] exbtn_ctl[i];
    };

    void insert( ButtonLink *button ){
        button->next = this->next;
        this->next = button;
    };

    void removeSprite( int no ){
        ButtonLink *bl = this;
        while( bl->next ){
            if ( bl->next->sprite_no == no &&
                 bl->next->button_type == SPRITE_BUTTON ){
                ButtonLink *bl2 = bl->next;
                bl->next = bl->next->next;
                delete bl2;
            }
            else{
                bl = bl->next;
            }
        }
    };
};

#endif // __BUTTON_LINK_H__

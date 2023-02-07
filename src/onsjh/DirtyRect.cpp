/* -*- C++ -*-
 * 
 *  DirtyRect.cpp - Invalid region on text_surface which should be updated
 *
 *  Copyright (c) 2001-2012 Ogapee. All rights reserved.
 *            (C) 2014 jh10001 <jh10001@live.cn>
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

#include "DirtyRect.h"

DirtyRect::DirtyRect()
{
    screen_width = screen_height = 0;
    bounding_box.w = bounding_box.h = 0;
}

DirtyRect::DirtyRect( const DirtyRect &d )
{
    screen_width  = d.screen_width;
    screen_height = d.screen_height;
    bounding_box = d.bounding_box;
}

DirtyRect& DirtyRect::operator =( const DirtyRect &d )
{
    screen_width  = d.screen_width;
    screen_height = d.screen_height;
    bounding_box = d.bounding_box;

    return *this;
}

DirtyRect::~DirtyRect()
{
}

void DirtyRect::setDimension(int w, int h)
{
    screen_width  = w;
    screen_height = h;
}

void DirtyRect::add( SDL_Rect src )
{
    //utils::printInfo("add %d %d %d %d\n", src.x, src.y, src.w, src.h );
    if ( src.w == 0 || src.h == 0 ) return;

    if (src.x < 0){
        if (src.w < -src.x) return;
        src.w += src.x;
        src.x = 0;
    }
    if (src.y < 0){
        if (src.h < -src.y) return;
        src.h += src.y;
        src.y = 0;
    }

    if (src.x >= screen_width) return;
    if (src.x+src.w >= screen_width)
        src.w = screen_width-src.x;

    if (src.y >= screen_height) return;
    if (src.y+src.h >= screen_height)
        src.h = screen_height-src.y;

    bounding_box = calcBoundingBox( bounding_box, src );
}

SDL_Rect DirtyRect::calcBoundingBox( SDL_Rect src1, SDL_Rect &src2 )
{
    if ( src2.w == 0 || src2.h == 0 ){
        return src1;
    }
    if ( src1.w == 0 || src1.h == 0 ){
        return src2;
    }

    if ( src1.x > src2.x ){
        src1.w += src1.x - src2.x;
        src1.x = src2.x;
    }
    if ( src1.y > src2.y ){
        src1.h += src1.y - src2.y;
        src1.y = src2.y;
    }
    if ( src1.x + src1.w < src2.x + src2.w ){
        src1.w = src2.x + src2.w - src1.x;
    }
    if ( src1.y + src1.h < src2.y + src2.h ){
        src1.h = src2.y + src2.h - src1.y;
    }

    return src1;
}

void DirtyRect::clear()
{
    bounding_box.w = bounding_box.h = 0;
}

void DirtyRect::fill( int w, int h )
{
    bounding_box.x = bounding_box.y = 0;
    bounding_box.w = w;
    bounding_box.h = h;
}

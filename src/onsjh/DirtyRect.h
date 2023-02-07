/* -*- C++ -*-
 * 
 *  DirtyRect.h - Invalid region on text_surface which should be updated
 *
 *  Copyright (c) 2001-2012 Ogapee. All rights reserved.
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

#ifndef __DIRTY_RECT__
#define __DIRTY_RECT__

#include <SDL.h>

struct DirtyRect
{
    DirtyRect();
    DirtyRect( const DirtyRect &d );
    DirtyRect& operator =( const DirtyRect &d );
    ~DirtyRect();
    
    void setDimension(int w, int h);
    void add( SDL_Rect src );
    void clear();
    void fill( int w, int h );

    SDL_Rect calcBoundingBox( SDL_Rect src1, SDL_Rect &src2 );

    int screen_width, screen_height;
    SDL_Rect bounding_box;
};

#endif // __DIRTY_RECT__

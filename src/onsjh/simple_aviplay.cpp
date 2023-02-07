/* -*- C++ -*-
 * 
 *  simple_aviplay.cpp - sample program for AVIWrapper class
 *
 *  Copyright (c) 2001-2004 Ogapee. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include "AVIWrapper.h"

#define DEFAULT_AUDIOBUF  4096
#define ONS_MIX_CHANNELS 50

int main( int argc, char **argv )
{
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO ) < 0 ){
        fprintf( stderr, "Couldn't initialize SDL: %s\n", SDL_GetError() );
        exit(-1);
    }

    AVIWrapper avi;
    if ( avi.init( argv[1], true ) ) exit(-1);
    SDL_Surface *screen_surface = SDL_SetVideoMode( avi.getWidth(), avi.getHeight(), 32, SDL_SWSURFACE );
    if ( avi.initAV( screen_surface, true ) ) exit(-1);
    avi.play( true );

    exit(0);
}

/* -*- C++ -*-
 * 
 *  AVIWrapper.h - avifile library wrapper class to play AVI video & audio stream
 *
 *  Copyright (c) 2001-2008 Ogapee. All rights reserved.
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

#ifndef __AVI_WRAPPER_H__
#define __AVI_WRAPPER_H__

#include <SDL.h>
#include <SDL_thread.h>
#include <avifile.h>
#include <avm_fourcc.h>
#include <utils.h>

class AVIWrapper
{
public:
    enum { AVI_STOP = 0,
           AVI_PLAYING = 1
    };
    AVIWrapper();
    ~AVIWrapper();

    int init( char *filename, bool debug_flag );
    int initAV( SDL_Surface *surface, bool audio_open_flag );
    int play( bool click_flag );

    void audioCallback( void *userdata, Uint8 *stream, int len );
    int playVideo( void *userdata );

    unsigned int getWidth(){ return width; };
    unsigned int getHeight(){ return height; };

private:    
    double getAudioTime();
    int drawFrame( avm::CImage *image );

    SDL_Overlay *screen_overlay;
    SDL_Rect screen_rect;
    unsigned int width;
    unsigned int height;

    IAviReadFile *i_avi;
    IAviReadStream *v_stream;
    IAviReadStream *a_stream;
    int remaining_count;
    char *remaining_buffer;

    bool debug_flag;
    int status;
    SDL_Thread *thread_id;
    int64_t time_start;
    double frame_start;
};

#endif // __AVI_WRAPPER_H__

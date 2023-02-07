/* -*- C++ -*-
 * 
 *  AVIWrapper.cpp - avifile library wrapper class to play AVI video & audio stream
 *
 *  Copyright (c) 2001-2014 Ogapee. All rights reserved.
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

#include "AVIWrapper.h"
#include <SDL_mixer.h>
#include <audiodecoder.h>
#include <avm_cpuinfo.h>
#include <avm_output.h>
#include <avifile.h>
#include <videodecoder.h>
#include <image.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_AUDIOBUF 4096
#define AVI_FINISH_EVENT 12345

#define AVIFILE_VERSION 747

AVIWrapper::AVIWrapper()
{
    screen_overlay = NULL;
    i_avi = NULL;
    v_stream = NULL;
    a_stream = NULL;
    remaining_buffer = new char[DEFAULT_AUDIOBUF*4];
    remaining_count = 0;
}

AVIWrapper::~AVIWrapper()
{
    if ( v_stream )
        v_stream->StopStreaming();
    if ( a_stream )
        a_stream->StopStreaming();
    if ( i_avi ) delete i_avi;
    if ( screen_overlay ) SDL_FreeYUVOverlay( screen_overlay );
    if ( remaining_buffer ) delete[] remaining_buffer;
}

int AVIWrapper::init( char *filename, bool debug_flag )
{
    this->debug_flag = debug_flag;
#if AVIFILE_VERSION >= 747
    if ( !debug_flag ) avm::AvmOutput::singleton()->resetDebugLevels(-1);
#else
    if ( !debug_flag ) avm::out.resetDebugLevels(-1);
#endif

    i_avi = CreateIAviReadFile( filename );
    if ( i_avi == NULL || i_avi->IsValid() == false ){
        fprintf( stderr, "can't CreateIAviReadFile from %s\n", filename );
        return -1;
    }

    v_stream = i_avi->GetStream(0, AviStream::Video );
    if ( v_stream == NULL ){
        fprintf( stderr, "Video Stream is NULL\n" );
        return -1;
    }

    width  = v_stream->GetStreamInfo()->GetVideoWidth();
    height = v_stream->GetStreamInfo()->GetVideoHeight();
    if ( debug_flag ) fprintf( stderr, "width %d height %d\n", width, height );

    return 0;
}

int AVIWrapper::initAV( SDL_Surface *surface, bool audio_open_flag )
{
    screen_rect.x = screen_rect.y = 0;
    screen_rect.w = surface->w;
    screen_rect.h = surface->h;

    v_stream->StartStreaming();
    if ( v_stream->GetVideoDecoder() == NULL ){
        if ( debug_flag ) fprintf( stderr, "GetVideoDecoder() return 0.\n" );
        return -1;
    }
    avm::IVideoDecoder::CAPS cap = v_stream->GetVideoDecoder()->GetCapabilities();
    if ( debug_flag ) printf("cap %x\n", cap );

    if ( cap & avm::IVideoDecoder::CAP_YV12 ){
        v_stream->GetVideoDecoder()->SetDestFmt( 0, fccYV12 );
        screen_overlay = SDL_CreateYUVOverlay( width, height, SDL_YV12_OVERLAY, surface );
    }
    else if ( cap & avm::IVideoDecoder::CAP_YUY2 ){
        v_stream->GetVideoDecoder()->SetDestFmt( 0, fccYUY2 );
        screen_overlay = SDL_CreateYUVOverlay( width, height, SDL_YUY2_OVERLAY, surface );
    }
    else{
        screen_overlay = SDL_CreateYUVOverlay( width, height, SDL_YV12_OVERLAY, surface );
    }

    if ( !audio_open_flag ) return 0;
    a_stream = i_avi->GetStream(0, AviStream::Audio);
    if ( a_stream == NULL ){
        if ( debug_flag ) fprintf( stderr, "Audio Stream is NULL\n" );
        return 0;
    }

    a_stream->StartStreaming();
    WAVEFORMATEX wave_fmt;
    a_stream->GetAudioDecoder()->GetOutputFormat( &wave_fmt );
    if ( debug_flag ) printf(" format %d ch %d sample %d bit %d avg Bps %d\n", wave_fmt.wFormatTag, wave_fmt.nChannels, wave_fmt.nSamplesPerSec, wave_fmt.wBitsPerSample, wave_fmt.nAvgBytesPerSec );
    
    if ( Mix_OpenAudio( wave_fmt.nSamplesPerSec, MIX_DEFAULT_FORMAT, wave_fmt.nChannels, DEFAULT_AUDIOBUF ) < 0 ){
        fprintf( stderr, "can't open audio device\n" );
        a_stream->StopStreaming();
        delete a_stream;
        a_stream = NULL;
    }

    return 0;
}

static void audioCallback( void *userdata, Uint8 *stream, int len )
{
    AVIWrapper &avi = *(AVIWrapper*)userdata;
    avi.audioCallback( userdata, stream, len );
}

void AVIWrapper::audioCallback( void *userdata, Uint8 *stream, int len )
{
    if ( len == 0 )
        status = AVI_STOP;

    size_t ocnt;
    size_t samples;
    size_t count = 0;

    if ( remaining_count > 0 ){
        if ( remaining_count <= len ){
            memcpy( stream, remaining_buffer, remaining_count );
            count = remaining_count;
            len -= remaining_count;
            remaining_count = 0;
        }
        else{
            memmove( stream, remaining_buffer, len );
            count = len;
            len = 0;
            remaining_count -= len;
            return;
        }
    }
        
    while ( len > 0 && !a_stream->Eof() ){
        a_stream->ReadFrames( remaining_buffer, (size_t)len, (size_t)len, samples, ocnt );
        if ( (int)ocnt <= len ){
            memcpy( stream+count, remaining_buffer, ocnt );
            len -= ocnt;
        }
        else{
            memcpy( stream+count, remaining_buffer, len );
            if ( (int)ocnt-len < DEFAULT_AUDIOBUF*4 - len ){
                memmove( remaining_buffer, remaining_buffer+len, ocnt-len );
                remaining_count = ocnt-len;
            }
            else{
                remaining_count = 0;
            }
            len = 0;
        }
        count += ocnt;
    }
}

double AVIWrapper::getAudioTime()
{
    if ( time_start == 0 )
    {
        frame_start = (v_stream) ? v_stream->GetTime() : 0.;
#if AVIFILE_VERSION >= 747
        time_start = avm_get_time_us();
#else
        time_start = longcount();
#endif
    }

    if ( a_stream )
        return a_stream->GetTime();
    else
#if AVIFILE_VERSION >= 747
        return frame_start + to_float(avm_get_time_us(), time_start);
#else
        return frame_start + to_float(longcount(), time_start);
#endif
}

static int playVideo( void *userdata )
{
    AVIWrapper &avi = *(AVIWrapper*)userdata;
    return avi.playVideo( userdata );
}

#define NUM_CACHES 3
int AVIWrapper::playVideo( void *userdata )
{
    int i;

    struct{
        bool valid;
        avm::CImage *image;
        double time;
    } cache[NUM_CACHES];
    for ( i=0 ; i<NUM_CACHES ; i++ ){
        cache[i].valid = false;
    }
    int remaining_cache = NUM_CACHES;

    while ( status == AVI_PLAYING && !v_stream->Eof() ){
        avm::CImage *image = v_stream->GetFrame( true );
        if ( image == NULL ) break;

        double current_time = v_stream->GetTime();
        double minimum_time = current_time;
        
        // look for the nearest in the cache
        int nearest_cache=-1;
        for ( i=0 ; i<NUM_CACHES ; i++ ){
            if ( cache[i].valid ){
                if ( nearest_cache == -1 ||
                     (nearest_cache >= 0 &&
                      cache[i].time < cache[nearest_cache].time) ){
                    nearest_cache = i;
                    if ( minimum_time > cache[nearest_cache].time )
                        minimum_time = cache[nearest_cache].time;
                }
            }
        }

        double async = getAudioTime() - minimum_time;
        //printf("audio %f (%f - %f) minimum %d %f cur %f\n", async, getAudioTime(), minimum_time, nearest_cache, minimum_time, current_time );
        if ( async < -0.01 ){
            if ( remaining_cache == 0 ){
                //printf("sync0 %f %f %f %f\n", async, (a_stream)?a_stream->GetTime():0.0, v_stream->GetTime(), minimum_time );
                SDL_Delay( (int)(-async*1000) );
            }
        }
        if ( (async < -0.01 && remaining_cache > 0) ||
             nearest_cache >= 0 ){
            // add cache
            for ( i=0 ; i<NUM_CACHES ; i++ ){
                if ( cache[i].valid == false ){
                    cache[i].valid = true;
                    cache[i].image = new avm::CImage(image);
                    cache[i].time = current_time;
                    remaining_cache--;
                    break;
                }
            }
            if ( async < -0.01 ){
                image->Release();
                continue;
            }
        }
                
        if ( nearest_cache >= 0 && minimum_time == cache[nearest_cache].time ){
            //printf("draw cache %d %f\n", nearest_cache, cache[nearest_cache].time );
            //if ( async <= 0.033 ) // drop frame if necessary
            drawFrame( cache[nearest_cache].image );
            cache[nearest_cache].image->Release();
            cache[nearest_cache].valid = false;
            remaining_cache++;
        }
        else{
            //printf("draw real %f\n", current_time );
            //if ( async <= 0.033 ) // drop frame if necessary
            drawFrame( image );
        }
        image->Release();
    }
    status = AVI_STOP;

    for ( i=0 ; i<NUM_CACHES ; i++ )
        if ( cache[i].valid ) cache[i].image->Release();

    return 0;
}

int AVIWrapper::play( bool click_flag )
{
    int ret = 0;
    time_start = 0;
    status = AVI_PLAYING;
    if ( v_stream )
        thread_id = SDL_CreateThread( ::playVideo, this );
    if ( a_stream )
        Mix_HookMusic( ::audioCallback, this );
    
    bool done_flag = false;
    while( !(done_flag & click_flag) && status == AVI_PLAYING ){
        SDL_Event event;

        while( SDL_PollEvent( &event ) ){
            switch (event.type){
              case SDL_KEYUP:
                if ( ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_RETURN ||
                     ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_KP_ENTER ||
                     ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_SPACE ||
                     ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_ESCAPE )
                    done_flag = true;
                break;
              case SDL_QUIT:
                ret = 1;
              case SDL_MOUSEBUTTONUP:
                done_flag = true;
                break;
              default:
                break;
            }
        }
        SDL_Delay( 10 );
    }

    status = AVI_STOP;
    if ( v_stream )
        SDL_WaitThread( thread_id, NULL );
    if ( a_stream )
        Mix_HookMusic( NULL, NULL );
    
    return ret;
}

int AVIWrapper::drawFrame( avm::CImage *image )
{
    if ( image == NULL ) return -1;

    unsigned int i, j;
    uint32_t comp = image->GetFmt()->biCompression;
        
    unsigned char *buf = image->Data();
    SDL_LockYUVOverlay( screen_overlay );
    Uint8 *dst_y = screen_overlay->pixels[0];
    Uint8 *dst_u = screen_overlay->pixels[1];
    Uint8 *dst_v = screen_overlay->pixels[2];

    if ( comp == 0 ){ // BGR
        image->ToYUV();
        for ( i=0 ; i<width*height ; i++ )
            *dst_y++ = buf[i*3];

        for ( i=0 ; i<height/2 ; i++ ){
            for ( j=0 ; j<width/2 ; j++ ){
                *dst_v++ = buf[(i*width*2+j*2)*3+1];
                *dst_u++ = buf[(i*width*2+j*2)*3+2];
            }
        }
    }
    else if ( comp == IMG_FMT_YUY2 ){
        memcpy( dst_y, buf, width*height*2 );
    }
    else if ( comp == IMG_FMT_YV12 ){ 
        memcpy( dst_y, buf, width*height+width*height/2 );
    }
    SDL_UnlockYUVOverlay( screen_overlay );
    SDL_DisplayYUVOverlay( screen_overlay, &screen_rect );

    return 0;
}

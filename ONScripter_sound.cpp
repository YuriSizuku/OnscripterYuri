/* -*- C++ -*-
 * 
 *  ONScripter_sound.cpp - Methods for playing sound
 *
 *  Copyright (c) 2001-2018 Ogapee. All rights reserved.
 *            (C) 2014-2019 jh10001 <jh10001@live.cn>
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

#include "ONScripter.h"
#include "Utils.h"
#include <new>
#if defined(LINUX)
#include <signal.h>
#endif
#if !defined(WINRT) && (defined(WIN32) || defined(_WIN32))
#include <stdlib.h>
#endif

#ifdef ANDROID
extern "C" void playVideoAndroid(const char *filename);
#endif

#if defined(IOS)
extern "C" void playVideoIOS(const char *filename, bool click_flag, bool loop_flag);
#endif

#if defined(USE_AVIFILE)
#include "AVIWrapper.h"
#endif

#if defined(USE_SMPEG)
extern "C" void mp3callback( void *userdata, Uint8 *stream, int len )
{
    SMPEG_playAudio( (SMPEG*)userdata, stream, len );
}
#endif

extern bool ext_music_play_once_flag;

extern "C"{
    extern void musicFinishCallback();
    extern Uint32 SDLCALL cdaudioCallback( Uint32 interval, void *param );
}
extern void midiCallback( int sig );
extern SDL_TimerID timer_cdaudio_id;

extern SDL_TimerID timer_bgmfade_id;
extern "C" Uint32 SDLCALL bgmfadeCallback( Uint32 interval, void *param );

#define TMP_MUSIC_FILE "tmp.mus"

int ONScripter::playSound(const char *filename, int format, bool loop_flag, int channel)
{
    if ( !audio_open_flag ) return SOUND_NONE;

    long length = script_h.cBR->getFileLength( filename );
    if (length == 0) return SOUND_NONE;
    if (!mode_wave_demo_flag &&
        ((skip_mode & SKIP_NORMAL) || ctrl_pressed_status) && (format & SOUND_CHUNK) &&
        ((channel < ONS_MIX_CHANNELS) || (channel == MIX_WAVE_CHANNEL))) {
           return SOUND_NONE;
    }

    unsigned char *buffer;

    if (format & SOUND_MUSIC && 
        length == music_buffer_length &&
        music_buffer ){
        buffer = music_buffer;
    }
    else{
        buffer = new(std::nothrow) unsigned char[length];
        if (buffer == NULL){
            utils::printError("failed to load [%s] because file size [%lu] is too large.\n", filename, length);
            return SOUND_NONE;
        }
        script_h.cBR->getFile( filename, buffer );
    }
    
    if (format & SOUND_MUSIC){
#if SDL_MIXER_MAJOR_VERSION >= 2
        music_info = Mix_LoadMUS_RW( SDL_RWFromMem( buffer, length ), 0);
#else
        music_info = Mix_LoadMUS_RW(SDL_RWFromMem(buffer, length));
#endif
        if (music_info == NULL) {
            utils::printError("can't load music \"%s\": %s\n", filename, Mix_GetError());
        }
        Mix_VolumeMusic( music_volume );
        if ( Mix_PlayMusic( music_info, (music_play_loop_flag&&music_loopback_offset==0.0)?-1:0 ) == 0 ){
            music_buffer = buffer;
            music_buffer_length = length;
            return SOUND_MUSIC;
        }
        Mix_HookMusicFinished(musicFinishCallback);
    }
    
    if (format & SOUND_CHUNK){
        Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_RWFromMem(buffer, length), 1);
        if (chunk == NULL) {
            utils::printError("can't load chunk \"%s\": %s\n", filename, Mix_GetError());
        }
        if (playWave(chunk, format, loop_flag, channel) == 0){
            delete[] buffer;
            return SOUND_CHUNK;
        }
    }

    /* check WMA */
    if ( buffer[0] == 0x30 && buffer[1] == 0x26 &&
         buffer[2] == 0xb2 && buffer[3] == 0x75 ){
        delete[] buffer;
        return SOUND_OTHER;
    }

    if (format & SOUND_MIDI){
        FILE *fp;
        if ( (fp = fopen(TMP_MUSIC_FILE, "wb", true)) == NULL){
            utils::printError("can't open temporaly MIDI file %s\n", TMP_MUSIC_FILE);
        }
        else{
            fwrite(buffer, 1, length, fp);
            fclose( fp );
            ext_music_play_once_flag = !loop_flag;
            if (playMIDI(loop_flag) == 0){
                delete[] buffer;
                return SOUND_MIDI;
            }
        }
    }

    delete[] buffer;
    
    return SOUND_OTHER;
}

void ONScripter::playCDAudio()
{
    if ( cdaudio_flag ){
#ifdef USE_CDROM
        if ( cdrom_info ){
            int length = cdrom_info->track[current_cd_track - 1].length / 75;
            SDL_CDPlayTracks( cdrom_info, current_cd_track - 1, 0, 1, 0 );
            timer_cdaudio_id = SDL_AddTimer( length * 1000, cdaudioCallback, NULL );
        }
#endif
    }
    else{
        char filename[256];
        sprintf( filename, "cd\\track%2.2d.mp3", current_cd_track );
        int ret = playSound( filename, SOUND_MUSIC, cd_play_loop_flag );
        if (ret == SOUND_MUSIC) return;

        sprintf( filename, "cd\\track%2.2d.ogg", current_cd_track );
        ret = playSound( filename, SOUND_MUSIC, cd_play_loop_flag );
        if (ret == SOUND_MUSIC) return;

        sprintf( filename, "cd\\track%2.2d.wav", current_cd_track );
        ret = playSound( filename, SOUND_MUSIC|SOUND_CHUNK, cd_play_loop_flag, MIX_BGM_CHANNEL );
    }
}

int ONScripter::playWave(Mix_Chunk *chunk, int format, bool loop_flag, int channel)
{
    if (!chunk) return -1;

    Mix_Pause( channel );
    if ( wave_sample[channel] ) Mix_FreeChunk( wave_sample[channel] );
    wave_sample[channel] = chunk;

    if      (channel == 0)               Mix_Volume( channel, voice_volume * MIX_MAX_VOLUME / 100 );
    else if (channel == MIX_BGM_CHANNEL) Mix_Volume( channel, music_volume * MIX_MAX_VOLUME / 100 );
    else                                 Mix_Volume( channel, se_volume * MIX_MAX_VOLUME / 100 );

    if ( !(format & SOUND_PRELOAD) )
        Mix_PlayChannel( channel, wave_sample[channel], loop_flag?-1:0 );

    return 0;
}

int ONScripter::playMIDI(bool loop_flag)
{
    Mix_SetMusicCMD(midi_cmd);
    
    char midi_filename[256];
    sprintf(midi_filename, "%s%s", save_dir?save_dir:archive_path, TMP_MUSIC_FILE);
    if ((midi_info = Mix_LoadMUS(midi_filename)) == NULL) return -1;

    int midi_looping = loop_flag ? -1 : 0;

#if defined(LINUX)
    signal(SIGCHLD, midiCallback);
    if (midi_cmd) midi_looping = 0;
#endif
    
    Mix_VolumeMusic(music_volume);
    Mix_PlayMusic(midi_info, midi_looping);
    current_cd_track = -2; 
    
    return 0;
}

#if defined(USE_SMPEG)
struct OverlayInfo{
    SDL_Overlay overlay;
    SDL_mutex *mutex;
};
static void smpeg_filter_callback( SDL_Overlay * dst, SDL_Overlay * src, SDL_Rect * region, SMPEG_FilterInfo * filter_info, void * data )
{
    if (dst){
        dst->w = 0;
        dst->h = 0;
    }

    OverlayInfo *oi = (OverlayInfo*)data;

    SDL_mutexP(oi->mutex);
    memcpy(oi->overlay.pixels[0], src->pixels[0],
           oi->overlay.w*oi->overlay.h + (oi->overlay.w/2)*(oi->overlay.h/2)*2);
    SDL_mutexV(oi->mutex);
}

static void smpeg_filter_destroy( struct SMPEG_Filter * filter )
{
}
#endif

int ONScripter::playMPEG(const char *filename, bool click_flag, bool loop_flag)
{
    unsigned long length = script_h.cBR->getFileLength( filename );
    if (length == 0){
        utils::printError(" *** can't find file [%s] ***\n", filename );
        return 0;
    }

#ifdef ANDROID
    playVideoAndroid(filename);
    return 0;
#endif

#ifdef IOS
    char *absolute_filename = new char[ strlen(archive_path) + strlen(filename) + 1 ];
    sprintf( absolute_filename, "%s%s", archive_path, filename );
    playVideoIOS(absolute_filename, click_flag, loop_flag);
    delete[] absolute_filename;
#endif

    int ret = 0;
#if defined(USE_SMPEG)
    stopSMPEG();
    layer_smpeg_buffer = new unsigned char[length];
    script_h.cBR->getFile( filename, layer_smpeg_buffer );
    SMPEG_Info info;
    layer_smpeg_sample = SMPEG_new_rwops( SDL_RWFromMem( layer_smpeg_buffer, length ), &info, 0 );
    if (SMPEG_error( layer_smpeg_sample )){
        stopSMPEG();
        return ret;
    }

    SMPEG_enableaudio( layer_smpeg_sample, 0 );
    if (audio_open_flag){
        int mpegversion, frequency, layer, bitrate;
        char mode[10];
        sscanf(info.audio_string,
               "MPEG-%d Layer %d %dkbit/s %dHz %s",
               &mpegversion, &layer, &bitrate, &frequency, mode);
        printf("MPEG-%d Layer %d %dkbit/s %dHz %s\n",
               mpegversion, layer, bitrate, frequency, mode);
        
        openAudio(frequency);

        SMPEG_actualSpec( layer_smpeg_sample, &audio_format );
        SMPEG_enableaudio( layer_smpeg_sample, 1 );
    }
    SMPEG_enablevideo( layer_smpeg_sample, 1 );
    
    SMPEG_setdisplay( layer_smpeg_sample, accumulation_surface, NULL,  NULL );

    OverlayInfo oi;
    Uint16 pitches[3];
    Uint8 *pixels[3];
    oi.overlay.format = SDL_YV12_OVERLAY;
    oi.overlay.w = info.width;
    oi.overlay.h = info.height;
    oi.overlay.planes = 3;
    pitches[0] = info.width;
    pitches[1] = info.width/2;
    pitches[2] = info.width/2;
    oi.overlay.pitches = pitches;
    Uint8 *pixel_buf = new Uint8[info.width*info.height + (info.width/2)*(info.height/2)*2];
    pixels[0] = pixel_buf;
    pixels[1] = pixel_buf + info.width*info.height;
    pixels[2] = pixel_buf + info.width*info.height + (info.width/2)*(info.height/2);
    oi.overlay.pixels = pixels;
    oi.mutex = SDL_CreateMutex();

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_TARGET, info.width, info.height);

    layer_smpeg_filter.data = &oi;
    layer_smpeg_filter.callback = smpeg_filter_callback;
    layer_smpeg_filter.destroy = smpeg_filter_destroy;
    SMPEG_filter( layer_smpeg_sample, &layer_smpeg_filter );
    SMPEG_setvolume( layer_smpeg_sample, music_volume );
    SMPEG_loop( layer_smpeg_sample, loop_flag?1:0 );

    if (info.has_audio) Mix_HookMusic( mp3callback, layer_smpeg_sample );
    SMPEG_play( layer_smpeg_sample );

    bool done_flag = false;
    while( !(done_flag & click_flag) && SMPEG_status(layer_smpeg_sample) == SMPEG_PLAYING ){
        SDL_Event event;

        while( SDL_PollEvent( &event ) ){
            switch (event.type){
              case SDL_KEYUP:
                if ( ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_RETURN ||
                     ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_SPACE ||
                     ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_ESCAPE )
                    done_flag = true;
                if ( ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_RCTRL)
                    ctrl_pressed_status &= ~0x01;
                    
                if ( ((SDL_KeyboardEvent *)&event)->keysym.sym == SDLK_LCTRL)
                    ctrl_pressed_status &= ~0x02;
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
        SDL_mutexP(oi.mutex);
        flushDirectYUV(&oi.overlay);
        SDL_mutexV(oi.mutex);
        SDL_Delay( 1 );
    }
    Mix_HookMusic( NULL, NULL );
    stopSMPEG();
    openAudio();
    delete[] pixel_buf;
    SDL_DestroyMutex(oi.mutex);
    texture = SDL_CreateTextureFromSurface(renderer, accumulation_surface);
#elif !defined(WINRT) && (defined(WIN32) || defined(_WIN32))
    char filename2[256];
    strcpy(filename2, filename);
    for (unsigned int i=0; i<strlen(filename2); i++)
        if (filename2[i] == '/' || filename2[i] == '\\')
            filename2[i] = DELIMITER;
    system(filename2);
#elif !defined(IOS)
    utils::printError( "mpegplay command is disabled.\n" );
#endif

    return ret;
}

int ONScripter::playAVI( const char *filename, bool click_flag )
{
    unsigned long length = script_h.cBR->getFileLength( filename );
    if (length == 0){
        utils::printError( " *** can't find file [%s] ***\n", filename );
        return 0;
    }

#ifdef ANDROID
    playVideoAndroid(filename);
    return 0;
#endif

#if !defined(WINRT) && (defined(WIN32) || defined(_WIN32))
    system(filename);
#else
    utils::printError( "avi command is disabled.\n" );
#endif

    return 0;
}

void ONScripter::stopBGM( bool continue_flag )
{
    removeBGMFadeEvent();
    if (timer_bgmfade_id) SDL_RemoveTimer( timer_bgmfade_id );
    timer_bgmfade_id = NULL;
    mp3fadeout_duration_internal = 0;

#ifdef USE_CDROM
    if ( cdaudio_flag && cdrom_info ){
        extern SDL_TimerID timer_cdaudio_id;

        if ( timer_cdaudio_id ){
            SDL_RemoveTimer( timer_cdaudio_id );
            timer_cdaudio_id = NULL;
        }
        if (SDL_CDStatus( cdrom_info ) >= CD_PLAYING )
            SDL_CDStop( cdrom_info );
    }
#endif

    if ( wave_sample[MIX_BGM_CHANNEL] ){
        Mix_Pause( MIX_BGM_CHANNEL );
        Mix_FreeChunk( wave_sample[MIX_BGM_CHANNEL] );
        wave_sample[MIX_BGM_CHANNEL] = NULL;
    }

    if ( music_info ){
        ext_music_play_once_flag = true;
        Mix_HaltMusic();
        Mix_FreeMusic( music_info );
        music_info = NULL;
    }

    if ( midi_info ){
        ext_music_play_once_flag = true;
        Mix_HaltMusic();
        Mix_FreeMusic( midi_info );
        midi_info = NULL;
    }

    if ( !continue_flag ){
        setStr( &music_file_name, NULL );
        music_play_loop_flag = false;
        if (music_buffer){
            delete[] music_buffer;
            music_buffer = NULL;
        }

        setStr( &midi_file_name, NULL );
        midi_play_loop_flag = false;

        current_cd_track = -1;
    }
}

void ONScripter::stopAllDWAVE()
{
    for (int ch=0; ch<ONS_MIX_CHANNELS ; ch++)
        if ( wave_sample[ch] ){
            Mix_Pause( ch );
            Mix_FreeChunk( wave_sample[ch] );
            wave_sample[ch] = NULL;
        }
}

void ONScripter::playClickVoice()
{
    if      ( clickstr_state == CLICK_NEWPAGE ){
        if ( clickvoice_file_name[CLICKVOICE_NEWPAGE] )
            playSound(clickvoice_file_name[CLICKVOICE_NEWPAGE], 
                      SOUND_CHUNK, false, MIX_WAVE_CHANNEL);
    }
    else if ( clickstr_state == CLICK_WAIT ){
        if ( clickvoice_file_name[CLICKVOICE_NORMAL] )
            playSound(clickvoice_file_name[CLICKVOICE_NORMAL], 
                      SOUND_CHUNK, false, MIX_WAVE_CHANNEL);
    }
}

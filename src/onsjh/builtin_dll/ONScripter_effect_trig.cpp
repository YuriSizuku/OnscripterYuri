/* -*- C++ -*-
 * 
 *  ONScripter_effect_trig.cpp - for ONScripter trig-tables,
 *    emulation of Takashi Toyama's "whirl.dll" and "trvswave.dll"
 *    NScripter plugin effects
 *
 *  Copyright (c) 2008-2011 "Uncle" Mion Sonozaki
 *            (C) 2015 jh10001 <jh10001@live.cn>
 *
 *  UncleMion@gmail.com
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
 *  along with this program; if not, see <http://www.gnu.org/licenses/>
 *  or write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifdef USE_BUILTIN_EFFECTS
#include "../ONScripter.h"

enum {
  //some constants for trig tables
  TRIG_TABLE_SIZE = 256,
  TRIG_FACTOR = 16384
};

void ONScripter::buildSinTable()
{
    if (!sin_table) {
        //integer-based trig table, scaled up by TRIG_FACTOR
        sin_table = new int[TRIG_TABLE_SIZE];
        for (int i=0; i<TRIG_TABLE_SIZE; i++) {
            sin_table[i] = (int)(sin((float) i * M_PI * 2 / TRIG_TABLE_SIZE) *
                                 TRIG_FACTOR);
        }
    }
}

void ONScripter::buildCosTable()
{
    if (!cos_table) {
        //integer-based trig table, scaled up by TRIG_FACTOR
        cos_table = new int[TRIG_TABLE_SIZE];
        for (int i=0; i<TRIG_TABLE_SIZE; i++) {
            cos_table[i] = (int)(cos((float) i * M_PI * 2 / TRIG_TABLE_SIZE) *
                                 TRIG_FACTOR);
        }
    }
}

//
// Emulation of Takashi Toyama's "trvswave.dll" NScripter plugin effect
//
void ONScripter::effectTrvswave( char *params, int duration )
{
    enum {
        //some constants for trvswave
        TRVSWAVE_AMPLITUDE   = 9,
        TRVSWAVE_WVLEN_END   = 32,
        TRVSWAVE_WVLEN_START = 256
    };

    SDL_Rect src_rect={0, 0, screen_width, 1};
    SDL_Rect dst_rect={0, 0, screen_width, 1};
    int ampl, wvlen;
    int y_offset = -screen_height / 2;
    int width = 256 * effect_counter / duration;
    alphaBlend( NULL, ALPHA_BLEND_CONST, width, &dirty_rect.bounding_box, NULL, NULL, effect_tmp_surface );
    if (effect_counter * 2 < duration) {
        ampl = TRVSWAVE_AMPLITUDE * 2 * effect_counter / duration;
        wvlen = (Sint16)(1.0/(((1.0/TRVSWAVE_WVLEN_END - 1.0/TRVSWAVE_WVLEN_START) * 2 * effect_counter / duration) + (1.0/TRVSWAVE_WVLEN_START)));
    } else {
        ampl = TRVSWAVE_AMPLITUDE * 2 * (duration - effect_counter) / duration;
        wvlen = (Sint16)(1.0/(((1.0/TRVSWAVE_WVLEN_END - 1.0/TRVSWAVE_WVLEN_START) * 2 * (duration - effect_counter) / duration) + (1.0/TRVSWAVE_WVLEN_START)));
    }
    SDL_FillRect( accumulation_surface, NULL, SDL_MapRGBA( accumulation_surface->format, 0, 0, 0, 0xff ) );
    for (int i=0; i<screen_height; i++) {
        int theta = TRIG_TABLE_SIZE * y_offset / wvlen;
        while (theta < 0) theta += TRIG_TABLE_SIZE;
        theta %= TRIG_TABLE_SIZE;
        dst_rect.x = (Sint16)(ampl * sin_table[theta] / TRIG_FACTOR);
        //dst_rect.x = (Sint16)(ampl * sin(M_PI * 2.0 * y_offset / wvlen));
        SDL_BlitSurface(effect_tmp_surface, &src_rect, accumulation_surface, &dst_rect);
        ++src_rect.y;
        ++dst_rect.y;
        ++y_offset;
    }
}

//
// Emulation of Takashi Toyama's "whirl.dll" NScripter plugin effect
//
#define CENTER_X (screen_width/2)
#define CENTER_Y (screen_height/2)

void ONScripter::buildWhirlTable()
{
    if (whirl_table) return;

    whirl_table = new int[screen_height * screen_width];
    int *dst_buffer = whirl_table;

    for ( int i=0 ; i<screen_height ; ++i ){
        for ( int j=0; j<screen_width ; ++j, ++dst_buffer ){
            int x = j - CENTER_X, y = i - CENTER_Y;
            // actual x = x + 0.5, actual y = y + 0.5;
            // (x+0.5)^2 + (y+0.5)^2 = x^2 + x + 0.25 + y^2 + y + 0.25
            *dst_buffer = (int)(sqrt((float)(x * x + x + y * y + y) + 0.5) * 4);
        }
    }
}

void ONScripter::effectWhirl( char *params, int duration )
{
//#define OMEGA (M_PI / 64)

    int direction = (params[0] == 'r') ? -1 : 1;

    int t = (effect_counter * (TRIG_TABLE_SIZE/4) / duration) %
            TRIG_TABLE_SIZE;
    int rad_amp = (sin_table[t] + cos_table[t] - TRIG_FACTOR) *
                  (TRIG_TABLE_SIZE/2) / TRIG_FACTOR;
    int rad_base = ((TRIG_FACTOR - cos_table[t]) *
                    TRIG_TABLE_SIZE / TRIG_FACTOR) + rad_amp;
    //float t = (float) effect_counter * M_PI / (duration * 2);
    //float one_minus_cos = 1 - cos(t);
    //float rad_amp = M_PI * (sin(t) - one_minus_cos);
    //float rad_base = M_PI * 2 * one_minus_cos + rad_amp;

    int width = 256 * effect_counter / duration;
    alphaBlend( NULL, ALPHA_BLEND_CONST, width, &dirty_rect.bounding_box,
                 NULL, NULL, effect_tmp_surface );

    SDL_LockSurface( effect_tmp_surface );
    SDL_LockSurface( accumulation_surface );
    ONSBuf *src_buffer = (ONSBuf *)effect_tmp_surface->pixels;
    ONSBuf *dst_buffer = (ONSBuf *)accumulation_surface->pixels;
    int *whirl_buffer = whirl_table;

    for ( int i=0 ; i<screen_height ; ++i ){
        for ( int j=0 ; j<screen_width ; ++j, ++dst_buffer, ++whirl_buffer ){
            int ii=i, jj=j;
            // actual x = x + 0.5, actual y = y + 0.5
            int x = j - CENTER_X, y = i - CENTER_Y;
            //whirl factor
            int theta = *whirl_buffer;
            while (theta < 0) theta += TRIG_TABLE_SIZE;
            theta %= TRIG_TABLE_SIZE;
            theta = ((rad_amp * sin_table[theta] / TRIG_FACTOR) + rad_base) *
                    direction;
            //float theta = direction * (rad_base + rad_amp * 
            //                           sin(sqrt(x * x + y * y) * OMEGA));

            //perform rotation
            while (theta < 0) theta += TRIG_TABLE_SIZE;
            theta %= TRIG_TABLE_SIZE;
            //working on x+0.5, hence (2x+1)/2
            jj = (((x + x + 1) * cos_table[theta] -
                   (y + y + 1) * sin_table[theta])/TRIG_FACTOR - 1)/2 +
                 CENTER_X;
            ii = (((x + x + 1) * sin_table[theta] +
                   (y + y + 1) * cos_table[theta])/TRIG_FACTOR - 1)/2 +
                 CENTER_Y;
            //jj = (int) (x * cos_theta - y * sin_theta + CENTER_X);
            //ii = (int) (x * sin_theta + y * cos_theta + CENTER_Y);
            if (jj < 0) jj = 0;
            if (jj >= screen_width) jj = screen_width-1;
            if (ii < 0) ii = 0;
            if (ii >= screen_height) ii = screen_height-1;

            // change pixel value!
            *dst_buffer = *(src_buffer + screen_width * ii + jj);
        }
    }

    SDL_UnlockSurface( accumulation_surface );
    SDL_UnlockSurface( effect_tmp_surface );
}
#endif

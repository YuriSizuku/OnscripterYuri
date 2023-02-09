/* -*- C++ -*-
 * 
 *  ONScripter_effect_breakup.cpp
 *    - Emulation of Takashi Toyama's "breakup.dll" NScripter plugin effect
 *
 *  Copyright (c) 2008-2012 "Uncle" Mion Sonozaki
 *
 *  UncleMion@gmail.com
 *
 *  Copyright (c) 2001-2012 Ogapee
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

#define BREAKUP_CELLWIDTH 24
#define BREAKUP_CELLFORMS 16
#define BREAKUP_MAX_CELL_X ((screen_width + BREAKUP_CELLWIDTH - 1)/BREAKUP_CELLWIDTH)
#define BREAKUP_MAX_CELL_Y ((screen_height + BREAKUP_CELLWIDTH - 1)/BREAKUP_CELLWIDTH)
#define BREAKUP_MAX_CELLS (BREAKUP_MAX_CELL_X * BREAKUP_MAX_CELL_Y)
#define BREAKUP_DIRECTIONS 8
#define BREAKUP_MOVE_FRAMES 40
#define BREAKUP_STILL_STATE (BREAKUP_CELLFORMS - BREAKUP_CELLWIDTH/2)

#define BREAKUP_MODE_LOWER  1
#define BREAKUP_MODE_LEFT   2
#define BREAKUP_MODE_PILEUP 4
#define BREAKUP_MODE_JUMBLE 8

const int breakup_disp_x[BREAKUP_DIRECTIONS] = { -7,-7,-5,-4,-2,1,3,5 }; 
const int breakup_disp_y[BREAKUP_DIRECTIONS] = {  0, 2, 4, 6, 7,7,6,5 }; 
int n_cells, tot_frames, last_frame;
int breakup_mode;
SDL_Rect breakup_window;  // window of _cells_, not pixels

void ONScripter::buildBreakupCellforms()
{
// build the 32x32 mask for each cellform
    if (breakup_cellforms) return;

    int w = BREAKUP_CELLWIDTH * BREAKUP_CELLFORMS;
    int h = BREAKUP_CELLWIDTH;
    breakup_cellforms = new bool[w*h];

    for (int n=0, rad2=1; n<BREAKUP_CELLFORMS; n++, rad2=(n+1)*(n+1)) {
        for (int x=0, xd=-BREAKUP_CELLWIDTH/2; x<BREAKUP_CELLWIDTH; x++, xd++) {
            for (int y=0, yd=-BREAKUP_CELLWIDTH/2; y<BREAKUP_CELLWIDTH; y++, yd++) {
                if (((xd * xd + xd + yd * yd + yd)*2 + 1) < 2*rad2)
                    breakup_cellforms[y*w + n*BREAKUP_CELLWIDTH + x] = true;
                else
                    breakup_cellforms[y*w + n*BREAKUP_CELLWIDTH + x] = false;
            }
        }
    }
}

void ONScripter::buildBreakupMask()
{
// build the cell area mask for the breakup effect
    int w = BREAKUP_CELLWIDTH * BREAKUP_MAX_CELL_X;
    int h = BREAKUP_CELLWIDTH * BREAKUP_MAX_CELL_Y;
    if (! breakup_mask) {
        breakup_mask = new bool[w*h];
    }

    SDL_LockSurface( effect_src_surface );
    SDL_LockSurface( effect_dst_surface );
    ONSBuf *buffer1 = (ONSBuf *)effect_src_surface->pixels;
    ONSBuf *buffer2 = (ONSBuf *)effect_dst_surface->pixels;
    SDL_PixelFormat *fmt = effect_dst_surface->format;
    int surf_w = effect_src_surface->w;
    int surf_h = effect_src_surface->h;
    int x1=w, y1=-1, x2=0, y2=0;
    for (int i=0; i<h; ++i) {
        for (int j=0; j<w; ++j) {
            if ((j >= surf_w) || (i >= surf_h)) {
                breakup_mask[i*w+j] = false;
                continue;
            }
            ONSBuf pix1 = buffer1[i*surf_w+j];
            ONSBuf pix2 = buffer2[i*surf_w+j];
            int pix1c = ((pix1 & fmt->Bmask) >> fmt->Bshift) << fmt->Bloss;
            int pix2c = ((pix2 & fmt->Bmask) >> fmt->Bshift) << fmt->Bloss;
            breakup_mask[i*w+j] = true;
            if (abs(pix1c - pix2c) > 8) {
                if (y1 < 0) y1 = i;
                if (j < x1) x1 = j;
                if (j > x2) x2 = j;
                y2 = i;
                continue;
            }
            pix1c = ((pix1 & fmt->Gmask) >> fmt->Gshift) << fmt->Gloss;
            pix2c = ((pix2 & fmt->Gmask) >> fmt->Gshift) << fmt->Gloss;
            if (abs(pix1c - pix2c) > 8) {
                if (y1 < 0) y1 = i;
                if (j < x1) x1 = j;
                if (j > x2) x2 = j;
                y2 = i;
                continue;
            }
            pix1c = ((pix1 & fmt->Rmask) >> fmt->Rshift) << fmt->Rloss;
            pix2c = ((pix2 & fmt->Rmask) >> fmt->Rshift) << fmt->Rloss;
            if (abs(pix1c - pix2c) > 8) {
                if (y1 < 0) y1 = i;
                if (j < x1) x1 = j;
                if (j > x2) x2 = j;
                y2 = i;
                continue;
            }
            pix1c = ((pix1 & fmt->Amask) >> fmt->Ashift) << fmt->Aloss;
            pix2c = ((pix2 & fmt->Amask) >> fmt->Ashift) << fmt->Aloss;
            if (abs(pix1c - pix2c) > 8) {
                if (y1 < 0) y1 = i;
                if (j < x1) x1 = j;
                if (j > x2) x2 = j;
                y2 = i;
                continue;
            }
            breakup_mask[i*w+j] = false;
        }
    }
    if (breakup_mode & BREAKUP_MODE_LEFT)
        x1 = 0;
    else
        x2 = surf_w-1;
    if (breakup_mode & BREAKUP_MODE_LOWER)
        y2 = surf_h-1;
    else
        y1 = 0;
    breakup_window.x = x1 / BREAKUP_CELLWIDTH;
    breakup_window.y = y1 / BREAKUP_CELLWIDTH;
    breakup_window.w = x2/BREAKUP_CELLWIDTH - breakup_window.x + 1;
    breakup_window.h = y2/BREAKUP_CELLWIDTH - breakup_window.y + 1;

    SDL_UnlockSurface( effect_dst_surface );
    SDL_UnlockSurface( effect_src_surface );
}

void ONScripter::initBreakup( char *params )
{
    while (*params != 0 && *params != '/') params++;
    if (*params == '/') params++;

    buildBreakupCellforms();

    breakup_mode = 0;
    if (params[0] == 'l')
        breakup_mode |= BREAKUP_MODE_LOWER;
    if (params[1] == 'l')
        breakup_mode |= BREAKUP_MODE_LEFT;
    if ((params[2] >= 'A') && (params[2] <= 'Z'))
        breakup_mode |= BREAKUP_MODE_JUMBLE;
    if ((params[2] == 'p') || (params[2] == 'P'))
        breakup_mode |= BREAKUP_MODE_PILEUP;

    if (!breakup_cells)
        breakup_cells = new BreakupCell[BREAKUP_MAX_CELLS];
    buildBreakupMask();
    int n_cell_x = breakup_window.w;
    int n_cell_y = breakup_window.h;
    int n_cell_diags = n_cell_x + n_cell_y;
    n_cells = n_cell_x * n_cell_y;
    tot_frames = BREAKUP_MOVE_FRAMES + n_cell_diags + BREAKUP_CELLFORMS - BREAKUP_CELLWIDTH/2 + 1;
    last_frame = 0;

    int n = 0, dir = 1, i = 0, diag_n = 0;
    for (i=0; i<n_cell_x; i++) {
        int state = BREAKUP_MOVE_FRAMES + BREAKUP_STILL_STATE + diag_n;
        if (breakup_mode & BREAKUP_MODE_PILEUP)
            state = 0 - diag_n;
        for (int j=i, k=0; (j>=0) && (k<n_cell_y); j--, k++) {
            breakup_cells[n].cell_x = j + breakup_window.x;
            breakup_cells[n].cell_y = k + breakup_window.y;
            if (!(breakup_mode & BREAKUP_MODE_LEFT))
                breakup_cells[n].cell_x = breakup_window.x + breakup_window.w - j - 1;
            if (breakup_mode & BREAKUP_MODE_LOWER)
                breakup_cells[n].cell_y = breakup_window.y + breakup_window.h - k - 1;
            breakup_cells[n].dir = dir;
            breakup_cells[n].state = state;
            breakup_cells[n].radius = 0;
            ++dir &= (BREAKUP_DIRECTIONS-1);
            ++n;
        }
        ++diag_n;
    }
    for (int i=1; i<n_cell_y; i++) {
        int state = BREAKUP_MOVE_FRAMES + BREAKUP_STILL_STATE + diag_n;
        if (breakup_mode & BREAKUP_MODE_PILEUP)
            state = 0 - diag_n;
        for (int j=n_cell_x-1, k=i; (k<n_cell_y) && (j>=0); j--, k++) {
            breakup_cells[n].cell_x = j + breakup_window.x;
            breakup_cells[n].cell_y = k + breakup_window.y;
            if (!(breakup_mode & BREAKUP_MODE_LEFT))
                breakup_cells[n].cell_x = breakup_window.x + n_cell_x - j - 1;
            if (breakup_mode & BREAKUP_MODE_LOWER)
                breakup_cells[n].cell_y = breakup_window.y + n_cell_y - k - 1;
            breakup_cells[n].dir = dir;
            breakup_cells[n].state = state;
            breakup_cells[n].radius = 0;
            ++dir &= (BREAKUP_DIRECTIONS-1);
            ++n;
        }
        ++diag_n;
    }
}

void ONScripter::effectBreakup( char *params, int duration )
{
    while (*params != 0 && *params != '/') params++;
    if (*params == '/') params++;
    
    int x_dir = -1;
    int y_dir = -1;

    int frame = tot_frames * effect_counter / duration;
    int frame_diff = frame - last_frame;
    if (frame_diff == 0) 
        return;

    SDL_Surface *bg = effect_dst_surface;
    SDL_Surface *chr = effect_src_surface;
    last_frame += frame_diff;
    frame_diff = -frame_diff;
    if (breakup_mode & BREAKUP_MODE_PILEUP) {
        bg = effect_src_surface;
        chr = effect_dst_surface;
        frame_diff = -frame_diff;
        x_dir = -x_dir;
        y_dir = -y_dir;
    }
    SDL_BlitSurface(bg, NULL, accumulation_surface, NULL);
    SDL_Surface *dst = accumulation_surface;

    if (breakup_mode & BREAKUP_MODE_JUMBLE) {
        x_dir = -x_dir;
        y_dir = -y_dir;
    }
    if (!(breakup_mode & BREAKUP_MODE_LEFT)) {
        x_dir = -x_dir;
    }
    if (breakup_mode & BREAKUP_MODE_LOWER) {
        y_dir = -y_dir;
    }

    SDL_LockSurface( chr );
    SDL_LockSurface( dst );
    ONSBuf *chr_buf = (ONSBuf *)chr->pixels;
    ONSBuf *buffer  = (ONSBuf *)dst->pixels;
    bool *msk_buf = breakup_cellforms;

    for (int n=0; n<n_cells; ++n) {
        SDL_Rect rect;
        rect.x = breakup_cells[n].cell_x * BREAKUP_CELLWIDTH;
        rect.y = breakup_cells[n].cell_y * BREAKUP_CELLWIDTH;
        rect.w = BREAKUP_CELLWIDTH;
        rect.h = BREAKUP_CELLWIDTH;
        breakup_cells[n].state += frame_diff;
        if (breakup_cells[n].state >= (BREAKUP_MOVE_FRAMES + BREAKUP_STILL_STATE)) {
            for (int i=0; i<BREAKUP_CELLWIDTH; ++i) {
                for (int j=0; j<BREAKUP_CELLWIDTH; ++j) {
                    int x = rect.x + j;
                    int y = rect.y + i;
                    if ((x < 0) || (x >= dst->w) || (x >= chr->w) ||
                        (y < 0) || (y >= dst->h) || (y >= chr->h))
                        continue;
                    if ( breakup_mask[y*BREAKUP_CELLWIDTH*BREAKUP_MAX_CELL_X + x] )
                        buffer[y*dst->w + x] = chr_buf[y*chr->w + x];
                }
            }
        }
        else if (breakup_cells[n].state >= BREAKUP_MOVE_FRAMES) {
            breakup_cells[n].radius = breakup_cells[n].state - (BREAKUP_MOVE_FRAMES*3/4) + 1;
            for (int i=0; i<BREAKUP_CELLWIDTH; i++) {
                for (int j=0; j<BREAKUP_CELLWIDTH; j++) {
                    int x = rect.x + j;
                    int y = rect.y + i;
                    if ((x < 0) || (x >= dst->w) || (x >= chr->w) ||
                        (y < 0) || (y >= dst->h) || (y >= chr->h))
                        continue;
                    int msk_off = BREAKUP_CELLWIDTH*breakup_cells[n].radius;
                    if ( msk_buf[BREAKUP_CELLWIDTH * BREAKUP_CELLFORMS * i + msk_off + j] &&
                         breakup_mask[y*BREAKUP_CELLWIDTH*BREAKUP_MAX_CELL_X + x] )
                        buffer[y*dst->w + x] = chr_buf[y*chr->w + x];
                }
            }
        }
        else if (breakup_cells[n].state >= 0) {
            int state = breakup_cells[n].state;
            int disp_x = x_dir * breakup_disp_x[breakup_cells[n].dir] * (state-BREAKUP_MOVE_FRAMES);
            int disp_y = y_dir * breakup_disp_y[breakup_cells[n].dir] * (BREAKUP_MOVE_FRAMES-state);

            breakup_cells[n].radius = 0;
            if (breakup_cells[n].state >= (BREAKUP_MOVE_FRAMES/2))
                breakup_cells[n].radius = (breakup_cells[n].state/2) - (BREAKUP_MOVE_FRAMES/4) + 1;
            for (int i=0; i<BREAKUP_CELLWIDTH; i++) {
                for (int j=0; j<BREAKUP_CELLWIDTH; j++) {
                    int x = disp_x + rect.x + j;
                    int y = disp_y + rect.y + i;
                    if ((x < 0) || (x >= dst->w) ||
                        (y < 0) || (y >= dst->h))
                        continue;
                    if (((rect.x+j)<0) || ((rect.x+j) >= chr->w) ||
                        ((rect.y+i)<0) || ((rect.y+i) >= chr->h))
                        continue;
                    int msk_off = BREAKUP_CELLWIDTH*breakup_cells[n].radius;
                    if ( msk_buf[BREAKUP_CELLWIDTH * BREAKUP_CELLFORMS * i + msk_off + j] &&
                         breakup_mask[(rect.y+i)*BREAKUP_CELLWIDTH*BREAKUP_MAX_CELL_X + rect.x + j] )
                        buffer[y*dst->w + x] =
                            chr_buf[(rect.y+i)*chr->w + rect.x + j];
                }
            }
        }
    }

    SDL_UnlockSurface( accumulation_surface );
    SDL_UnlockSurface( chr );
}

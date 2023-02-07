/* -*- C++ -*-
 * 
 *  AnimationInfo.h - General image storage class of ONScripter
 *
 *  Copyright (c) 2001-2016 Ogapee. All rights reserved.
 *            (C) 2015-2016 jh10001 <jh10001@live.cn>
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

#ifndef __ANIMATION_INFO_H__
#define __ANIMATION_INFO_H__

#include <SDL.h>
#include <string.h>

typedef unsigned char uchar3[3];

class AnimationInfo{
public:
    typedef Uint32 ONSBuf;
    enum { TRANS_ALPHA          = 1,
           TRANS_TOPLEFT        = 2,
           TRANS_COPY           = 3,
           TRANS_STRING         = 4,
           TRANS_DIRECT         = 5,
           TRANS_PALLETTE       = 6,
           TRANS_TOPRIGHT       = 7,
           TRANS_MASK           = 8,
#ifdef USE_BUILTIN_LAYER_EFFECTS
           TRANS_LAYER          = 9
#endif
    };

    /* variables set from the image tag */
    int trans_mode;
    unsigned char default_alpha;
#ifdef USE_BUILTIN_LAYER_EFFECTS
    int layer_no;
#endif
    uchar3 direct_color;
    int pallette_number;
    uchar3 color;
    SDL_Rect orig_pos; // position and size of the image before resizing
    SDL_Rect pos; // position and size of the current cell
    SDL_Rect affine_pos; // topleft position and width/height for affince transformation

    int num_of_cells;
    int current_cell;
    int direction;
    int *duration_list;
    uchar3 *color_list;
    int loop_mode;
    bool is_animatable;
    bool is_single_line;
    bool is_tight_region; // valid under TRANS_STRING, if false, ruby is parsed
    bool is_ruby_drawable;
        
    char *file_name;
    char *mask_file_name;

    /* Variables from AnimationInfo */
    bool visible;
    bool abs_flag;
    bool affine_flag;
    int trans;
    char *image_name;
    char *surface_name; // used to avoid reloading images
    char *mask_surface_name; // used to avoid reloading images
    SDL_Surface *image_surface;
    unsigned char *alpha_buf;
    Uint32 texture_format;
    SDL_mutex *mutex;
        
    /* Variables for extended sprite (lsp2, drawsp2, etc.) */
    int scale_x, scale_y, rot;
    int mat[2][2], inv_mat[2][2];
    int corner_xy[4][2];
    SDL_Rect bounding_rect;

    enum { BLEND_NORMAL = 0,
           BLEND_ADD    = 1,
           BLEND_SUB    = 2
    };
    int blending_mode;
    int cos_i, sin_i;

    int font_size_xy[2]; // used by prnum and lsp string
    int font_pitch[2]; // used by lsp string
    int next_time;

    int param; // used by prnum and bar
    int max_param; // used by bar
    int max_width; // used by bar
    
    AnimationInfo();
    AnimationInfo(const AnimationInfo &anim);
    ~AnimationInfo();

    AnimationInfo& operator =(const AnimationInfo &anim);

    void scalePosXY(int screen_ratio1, int screen_ratio2){
        pos.x = orig_pos.x * screen_ratio1 / screen_ratio2;
        pos.y = orig_pos.y * screen_ratio1 / screen_ratio2;
    };
    void scalePosWH(int screen_ratio1, int screen_ratio2){
        pos.w = orig_pos.w * screen_ratio1 / screen_ratio2;
        pos.h = orig_pos.h * screen_ratio1 / screen_ratio2;
    };
                 
    void reset();
    
    void deleteImageName();
    void setImageName( const char *name );
    void deleteSurface(bool delete_surface_name=true);
    void remove();
    void removeTag();

    bool proceedAnimation(int current_time);

    void setCell(int cell);
    static int doClipping( SDL_Rect *dst, SDL_Rect *clip, SDL_Rect *clipped=NULL );
    void blendOnSurface( SDL_Surface *dst_surface, int dst_x, int dst_y,
                         SDL_Rect &clip, int alpha=255 );
    void blendOnSurface2( SDL_Surface *dst_surface, int dst_x, int dst_y,
                          SDL_Rect &clip, int alpha=255 );
    void blendText( SDL_Surface *surface, int dst_x, int dst_y, 
                    SDL_Color &color, SDL_Rect *clip, bool rotate_flag );
    void calcAffineMatrix();
    
    static SDL_Surface *allocSurface( int w, int h, Uint32 texture_format );
    static SDL_Surface *alloc32bitSurface( int w, int h, Uint32 texture_format );
    void allocImage( int w, int h, Uint32 texture_format );
    void copySurface( SDL_Surface *surface, SDL_Rect *src_rect, SDL_Rect *dst_rect = NULL );
    void fill( Uint8 r, Uint8 g, Uint8 b, Uint8 a );
    SDL_Surface *setupImageAlpha( SDL_Surface *surface, SDL_Surface *surface_m, bool has_alpha );
    void setImage( SDL_Surface *surface, Uint32 texture_format );
    unsigned char getAlpha(int x, int y);

#ifdef USE_SMPEG
    void convertFromYUV(SDL_Overlay *src);
#endif
};

#endif // __ANIMATION_INFO_H__

/* -*- C++ -*-
 *
 *  AnimationInfo.cpp - General image storage class of ONScripter
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

#include "AnimationInfo.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#if defined(USE_OMP_PARALLEL) || defined(USE_PARALLEL)
#include "Parallel.h"
#endif
#ifdef USE_SIMD
#include "simd/simd.h"
#endif
#include "builtin_layer.h"


#define RMASK 0x00ff0000
#define GMASK 0x0000ff00
#define BMASK 0x000000ff
#define AMASK 0xff000000
#define RBMASK (RMASK|BMASK)

static bool is_inv_alpha_lut_initialized = false;
static Uint32 inv_alpha_lut[256];

AnimationInfo::AnimationInfo()
{
    image_name = NULL;
    surface_name = NULL;
    mask_surface_name = NULL;
    image_surface = NULL;
    alpha_buf = NULL;
    mutex = SDL_CreateMutex();

    duration_list = NULL;
    color_list = NULL;
    file_name = NULL;
    mask_file_name = NULL;

    trans_mode = TRANS_TOPLEFT;
    affine_flag = false;

    if (!is_inv_alpha_lut_initialized){
        inv_alpha_lut[0] = 255;
        for (int i=1; i<255; i++)
            inv_alpha_lut[i] = (Uint32)(0xffff / i);
        is_inv_alpha_lut_initialized = true;
    }

    reset();
}

AnimationInfo::AnimationInfo(const AnimationInfo &anim)
{
    *this = anim;
}

AnimationInfo::~AnimationInfo()
{
    reset();
    if (mutex) SDL_DestroyMutex(mutex);
}

AnimationInfo& AnimationInfo::operator =(const AnimationInfo &anim)
{
    if (this != &anim){
        memcpy(this, &anim, sizeof(AnimationInfo));

        mutex = SDL_CreateMutex();

        if (image_name){
            image_name = new char[ strlen(anim.image_name) + 1 ];
            strcpy( image_name, anim.image_name );
        }
        if (surface_name){
            surface_name = new char[ strlen(anim.surface_name) + 1 ];
            strcpy( surface_name, anim.surface_name );
        }
        if (mask_surface_name){
            mask_surface_name = new char[ strlen(anim.mask_surface_name) + 1 ];
            strcpy( mask_surface_name, anim.mask_surface_name );
        }
        if (file_name){
            file_name = new char[ strlen(anim.file_name) + 1 ];
            strcpy( file_name, anim.file_name );
        }
        if (mask_file_name){
            mask_file_name = new char[ strlen(anim.mask_file_name) + 1 ];
            strcpy( mask_file_name, anim.mask_file_name );
        }
        if (color_list){
            color_list = new uchar3[ anim.num_of_cells ];
            memcpy(color_list, anim.color_list, sizeof(uchar3)*anim.num_of_cells);
        }
        if (duration_list){
            duration_list = new int[ anim.num_of_cells ];
            memcpy(duration_list, anim.duration_list, sizeof(int)*anim.num_of_cells);
        }
        
        if (image_surface){
            image_surface = allocSurface( anim.image_surface->w, anim.image_surface->h, texture_format );
            memcpy(image_surface->pixels, anim.image_surface->pixels, anim.image_surface->pitch*anim.image_surface->h);
        }
    }

    return *this;
}

void AnimationInfo::reset()
{
    remove();

    trans = -1;
    default_alpha = 0xff;
    orig_pos.x = orig_pos.y = 0;
    pos.x = pos.y = 0;
    visible = false;
    abs_flag = true;
    scale_x = scale_y = rot = 0;
    blending_mode = BLEND_NORMAL;

    font_size_xy[0] = font_size_xy[1] = -1;
    font_pitch[0] = font_pitch[1] = -1;

    mat[0][0] = 1024;
    mat[0][1] = 0;
    mat[1][0] = 0;
    mat[1][1] = 1024;

#ifdef USE_BUILTIN_LAYER_EFFECTS
    layer_no = -1;
#endif
}

void AnimationInfo::deleteImageName(){
    if ( image_name ) delete[] image_name;
    image_name = NULL;
}

void AnimationInfo::setImageName( const char *name ){
    deleteImageName();
    image_name = new char[ strlen(name) + 1 ];
    strcpy( image_name, name );
}

void AnimationInfo::deleteSurface(bool delete_surface_name)
{
    if (delete_surface_name){
        if ( surface_name ) delete[] surface_name;
        surface_name = NULL;
        if ( mask_surface_name ) delete[] mask_surface_name;
        mask_surface_name = NULL;
    }
    SDL_mutexP(mutex);
    if ( image_surface ) SDL_FreeSurface( image_surface );
    image_surface = NULL;
    SDL_mutexV(mutex);
    if (alpha_buf) delete[] alpha_buf;
    alpha_buf = NULL;
}

void AnimationInfo::remove()
{
    deleteImageName();
    deleteSurface();
    removeTag();
}

void AnimationInfo::removeTag()
{
    if ( duration_list ){
        delete[] duration_list;
        duration_list = NULL;
    }
    if ( color_list ){
        delete[] color_list;
        color_list = NULL;
    }
    if ( file_name ){
        delete[] file_name;
        file_name = NULL;
    }
    if ( mask_file_name ){
        delete[] mask_file_name;
        mask_file_name = NULL;
    }
    current_cell = 0;
    num_of_cells = 0;
    next_time = 0;
    is_animatable = false;
    is_single_line = true;
    is_tight_region = true;
    is_ruby_drawable = false;
    direction = 1;

    color[0] = color[1] = color[2] = 0;
}

bool AnimationInfo::proceedAnimation(int current_time)
{
    if (!visible || !is_animatable || next_time > current_time) return false;

    bool is_changed = false;
    
#ifdef USE_BUILTIN_LAYER_EFFECTS
    if (trans_mode == AnimationInfo::TRANS_LAYER) {
        if (layer_no >= 0) {
            LayerInfo *tmp = &layer_info[layer_no];
            if (tmp->handler) {
                tmp->handler->update();
                is_changed = true;
                next_time += tmp->interval;
                if (next_time <= current_time) {
                    next_time = current_time + 1;
                }
            }
        }
    }
    else
#endif
    while(next_time <= current_time){
        if ( loop_mode != 3 && num_of_cells > 0 ){
            current_cell += direction;
            is_changed = true;
        }

        if ( current_cell < 0 ){ // loop_mode must be 2
            if (num_of_cells == 1)
                current_cell = 0;
            else
                current_cell = 1;
            direction = 1;
        }
        else if ( current_cell >= num_of_cells ){
            if ( loop_mode == 0 ){
                current_cell = 0;
            }
            else if ( loop_mode == 1 ){
                current_cell = num_of_cells - 1;
                is_changed = false;
            }
            else{
                current_cell = num_of_cells - 2;
                if (current_cell < 0)
                    current_cell = 0;
                direction = -1;
            }
        }

        next_time += duration_list[ current_cell ];

        if (duration_list[ current_cell ] <= 0){
            next_time = current_time;
            break;
        }
    }

    return is_changed;
}

void AnimationInfo::setCell(int cell)
{
    if (cell < 0) cell = 0;
    else if (cell >= num_of_cells) cell = num_of_cells - 1;

    current_cell = cell;
}

int AnimationInfo::doClipping( SDL_Rect *dst, SDL_Rect *clip, SDL_Rect *clipped )
{
    if ( clipped ) clipped->x = clipped->y = 0;

    if ( !dst ||
         dst->x >= clip->x + clip->w || dst->x + dst->w <= clip->x ||
         dst->y >= clip->y + clip->h || dst->y + dst->h <= clip->y )
        return -1;

    if ( dst->x < clip->x ){
        dst->w -= clip->x - dst->x;
        if ( clipped ) clipped->x = clip->x - dst->x;
        dst->x = clip->x;
    }
    if ( clip->x + clip->w < dst->x + dst->w ){
        dst->w = clip->x + clip->w - dst->x;
    }
    
    if ( dst->y < clip->y ){
        dst->h -= clip->y - dst->y;
        if ( clipped ) clipped->y = clip->y - dst->y;
        dst->y = clip->y;
    }
    if ( clip->y + clip->h < dst->y + dst->h ){
        dst->h = clip->y + clip->h - dst->y;
    }
    if ( clipped ){
        clipped->w = dst->w;
        clipped->h = dst->h;
    }

    return 0;
}

#ifdef USE_SIMD
static void blendPixel32(const Uint32 *src_buffer, Uint32 *__restrict dst_buffer, Uint8 alpha, Uint8 *alphap) {
    using namespace simd;
    uint8x4 src = load(src_buffer), dst = load(dst_buffer);
    ivec128 zero = ivec128::zero();
    uint16x4 r1 = widen(src, zero);
    uint16x4 dstu = widen(dst, zero);
    r1 -= dstu;
    uint16x4 a((alpha * *alphap) >> 8);
    r1 = (r1 * a) >> immint<8>();
    uint8x4 r = narrow_hz(r1);
    r += dst;
    *dst_buffer = uint8x4::cvt2i32(r) | AMASK;
}

static void blend4Pixel32(const Uint32 *src_buffer, Uint32 *__restrict dst_buffer, simd::uint16x8 alpha, simd::uint8x16 alpha_mask, simd::ivec128 zero, simd::uint8x16 amask) {
    using namespace simd;
    uint8x16 src = load_u(src_buffer), dst = load_u(dst_buffer);
    uint16x8 dstu = widen_lo(dst, zero);
    uint16x8 r1 = widen_lo(src, zero);
    r1 -= dstu;
    dstu = widen_hi(dst, zero);
    uint16x8 r2 = widen_hi(src, zero);
    r2 -= dstu;
#ifdef USE_SIMD_X86_SSSE3
    uint8x16 an = shuffle(src, alpha_mask);
    uint16x8 a = widen_lo(an, zero);
#else
	Uint8 *alphap = (Uint8*)src_buffer + 3;
    uint16x8 a = uint16x8::set2(*alphap, *(alphap + 4));
    alphap += 8;
#endif
    a = (a * alpha) >> immint<8>();
    r1 = (r1 * a) >> immint<8>();
#ifdef USE_SIMD_X86_SSSE3
    a = widen_hi(an, zero);
#else
    a = uint16x8::set2(*alphap, *(alphap + 4));
#endif
    a = (a * alpha) >> immint<8>();
    r2 = (r2 * a) >> immint<8>();
    uint8x16 r = pack_hz(r1, r2);
    r += dst;
    r |= amask;
    store_u(dst_buffer, r);
}

#ifdef USE_SIMD_X86_AVX2
static void blend8Pixel32(const Uint32 *src_buffer, Uint32 *__restrict dst_buffer, simd::uint16x16 alpha, simd::uint8x32 alpha_mask, simd::ivec256 zero, simd::uint8x32 amask) {
    using namespace simd;
    uint8x32 src = load256_u(src_buffer), dst = load256_u(dst_buffer);
    uint16x16 dstu = widen_lo(dst, zero);
    uint16x16 r1 = widen_lo(src, zero);
    r1 -= dstu;
    dstu = widen_hi(dst, zero);
    uint16x16 r2 = widen_hi(src, zero);
    r2 -= dstu;
	uint8x32 an = shuffle(src, alpha_mask);
    uint16x16 a = widen_lo(an, zero);
    a = (a * alpha) >> immint<8>();
    r1 = (r1 * a) >> immint<8>();
    a = widen_hi(an, zero);
    a = (a * alpha) >> immint<8>();
    r2 = (r2 * a) >> immint<8>();
    uint8x32 r = pack_hz(r1, r2);
    r += dst;
    r |= amask;
    store256_u(dst_buffer, r);
}
#endif

#define BLEND_PIXEL() do{\
  blendPixel32(src_buffer, dst_buffer, alpha, alphap);\
  alphap += 4;\
}while(0)
#else
#define BLEND_PIXEL() do{\
    Uint32 mask2 = (*alphap * alpha) >> 8;\
    Uint32 temp = *dst_buffer & 0xff00ff;\
    Uint32 mask_rb = (((((*src_buffer & 0xff00ff) - temp ) * mask2 ) >> 8 ) + temp ) & 0xff00ff;\
    temp = *dst_buffer & 0x00ff00;\
    Uint32 mask_g  = (((((*src_buffer & 0x00ff00) - temp ) * mask2 ) >> 8 ) + temp ) & 0x00ff00;\
    *dst_buffer = mask_rb | mask_g | 0xff000000;\
    alphap += 4;\
}while(0)
#endif
// Originally, the above looks like this.
//      mask1 = mask2 ^ 0xff;
//      Uint32 mask_rb = (((*dst_buffer & 0xff00ff) * mask1 +
//                         (*src_buffer & 0xff00ff) * mask2) >> 8) & 0xff00ff;
//      Uint32 mask_g  = (((*dst_buffer & 0x00ff00) * mask1 +
//                         (*src_buffer & 0x00ff00) * mask2) >> 8) & 0x00ff00;

#define ADDBLEND_PIXEL() do{\
    Uint32 mask2 = (*alphap * alpha) >> 8;\
    Uint32 mask_rb = (*dst_buffer & RBMASK) + ((((*src_buffer & RBMASK) * mask2) >> 8) & RBMASK);\
    mask_rb |= ((mask_rb & AMASK) ? RMASK : 0) | ((mask_rb & GMASK) ? BMASK : 0);\
    Uint32 mask_g = (*dst_buffer & GMASK) + ((((*src_buffer & GMASK) * mask2) >> 8) & GMASK);\
    mask_g |= ((mask_g & RMASK) ? GMASK : 0);\
    *dst_buffer = (mask_rb & RBMASK) | (mask_g & GMASK) | 0xff000000;\
    alphap += 4;\
}while(0)

inline void rainAddBlendPixel32(const Uint32 *src_buffer, Uint32 *__restrict dst_buffer) {
#ifdef USE_SIMD
    using namespace simd;
    uint8x4 src = uint8x4::cvt2vec(*src_buffer);
    uint8x4 dst = uint8x4::cvt2vec(*dst_buffer);
    dst = adds(dst, src);
    *dst_buffer = uint8x4::cvt2i32(dst);
#else
    const Uint8 *src = (const Uint8*)src_buffer;
    Uint8 *dst = (Uint8*)dst_buffer;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    ++dst; ++src;
#endif
    for (int i = 0; i < 3; ++i, ++src, ++dst) {
        int result = (*dst) + (*src);
        (*dst) = (result < 255) ? result : 255;
    }
#endif
}

#ifdef USE_SIMD
inline void rainAddBlend32(const Uint32 *src_buffer, Uint32 *__restrict dst_buffer, int remain) {
    using namespace simd;
    while (remain >= 4) {
        uint8x16 srcvec = load_u(src_buffer),
            dstvec = load_u(dst_buffer);
        uint8x16 r = adds(srcvec, dstvec);
        store_u(dst_buffer, r);
        remain -= 4; src_buffer += 4; dst_buffer += 4;
    }
    while (remain > 0) {
        rainAddBlendPixel32(src_buffer, dst_buffer);
        --remain; ++src_buffer; ++dst_buffer;
    }
}
#endif // USE_SIMD

#define SUBBLEND_PIXEL(){\
    Uint32 mask2 = (*alphap * alpha) >> 8;\
    Uint32 mask_r = (*dst_buffer & RMASK) -\
                    ((((*src_buffer & RMASK) * mask2) >> 8) & RMASK);\
    mask_r &= ((mask_r & AMASK) ? 0 : RMASK);\
    Uint32 mask_g = (*dst_buffer & GMASK) -\
                    ((((*src_buffer & GMASK) * mask2) >> 8) & GMASK);\
    mask_g &= ((mask_g & ~(GMASK | BMASK)) ? 0 : GMASK);\
    Uint32 mask_b = (*dst_buffer & BMASK) -\
                    ((((*src_buffer & BMASK) * mask2) >> 8) & BMASK);\
    mask_b &= ((mask_b & ~BMASK) ? 0 : BMASK);\
    *dst_buffer = (mask_r & RMASK) | (mask_g & GMASK) | (mask_b & BMASK) | 0xff000000;\
    alphap += 4;\
}

void AnimationInfo::blendOnSurface( SDL_Surface *dst_surface, int dst_x, int dst_y,
                                    SDL_Rect &clip, int alpha )
{
    if ( image_surface == NULL ) return;
    
    SDL_Rect dst_rect, src_rect;
    dst_rect.x = dst_x;
    dst_rect.y = dst_y;
    dst_rect.w = pos.w;
    dst_rect.h = pos.h;
    if ( doClipping( &dst_rect, &clip, &src_rect ) ) return;
    if (alpha == 0) return;

    /* ---------------------------------------- */
    
    SDL_mutexP(mutex);
    SDL_LockSurface( dst_surface );
    SDL_LockSurface( image_surface );
    
    alpha &= 0xff;
    int pitch = image_surface->pitch / sizeof(ONSBuf);

    struct Blender {
        ONSBuf *const stsrc_buffer, *const stdst_buffer;
        const int alpha, dst_rect_w, dst_rect_h, pitch, dst_surface_w, blendmode;

        void operator()(const int i) const {
            const ONSBuf *src_buffer = stsrc_buffer + (pitch)* i;
            ONSBuf *dst_buffer = stdst_buffer + (dst_surface_w)* i;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    unsigned char *alphap = (unsigned char *)src_buffer + 3;
#else
    unsigned char *alphap = (unsigned char *)src_buffer;
#endif //SDL_BYTEORDER == SDL_LIL_ENDIAN
#ifdef USE_BUILTIN_LAYER_EFFECTS
            if (blendmode == AnimationInfo::BLEND_ADD) {
#ifdef USE_SIMD
                rainAddBlend32(src_buffer, dst_buffer, dst_rect_w);
#else
                for (int j = dst_rect_w; j != 0; j--, src_buffer++, dst_buffer++) {
                    if (*src_buffer != AMASK) rainAddBlendPixel32(src_buffer, dst_buffer);
                }
#endif //USE_SIMD
            }
            else
#endif
#ifdef USE_SIMD
            {
                using namespace simd;
#ifdef USE_SIMD_X86_AVX2
                ivec256 zero = ivec256::zero();
                uint8x32 mask = uint8x32::set8(3, 7, 11, 15, 19, 23, 27, 31);
                uint8x32 amask =
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                    uint8x32::set(0, 0, 0, 0xFF);
#else
                    uint8x32::set(0xFF, 0, 0, 0);
#endif
                ivec128 zerol = zero.lo();
                uint8x16 maskl = mask.lo();
                uint8x16 amaskl = amask.lo();
#else
                ivec128 zerol = ivec128::zero();
                uint8x16 maskl = uint8x16::set4(3, 7, 11, 15);
                uint8x16 amaskl =
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                    uint8x16::set(0, 0, 0, 0xFF);
#else
                    uint8x16::set(0xFF, 0, 0, 0);
#endif
#endif
                int remain = dst_rect_w;
                while (remain > 0) {
                    if (*alphap == 0) {
                        --remain; ++src_buffer; ++dst_buffer; alphap += 4;
                    }
                    else if ((*alphap == 255) && (alpha == 255)) {
                        *dst_buffer = *src_buffer;
                        --remain; ++src_buffer; ++dst_buffer; alphap += 4;
                    }
#ifdef USE_SIMD_X86_AVX2
                    else if (remain >= 8) {
                        blend8Pixel32(src_buffer, dst_buffer, uint16x16(alpha), mask, zero, amask);
                        remain -= 8; src_buffer += 8; dst_buffer += 8; alphap += 32;
                    }
#endif
                    else if (remain >= 4) {
                        blend4Pixel32(src_buffer, dst_buffer, uint16x8(alpha), maskl, zerol, amaskl);
                        remain -= 4; src_buffer += 4; dst_buffer += 4; alphap += 16;
                    }
                    else {
                        BLEND_PIXEL();
                        --remain; ++src_buffer; ++dst_buffer;
                    }
                }
            }
#else
                for (int j = dst_rect_w; j != 0; j--, src_buffer++, dst_buffer++) {
                    BLEND_PIXEL();
                }
#endif
        }
    } blender = {(ONSBuf *)image_surface->pixels + pitch * src_rect.y + image_surface->w * current_cell / num_of_cells + src_rect.x,
        (ONSBuf *)dst_surface->pixels + dst_surface->w * dst_rect.y + dst_rect.x,
        alpha, dst_rect.w, dst_rect.h, pitch, dst_surface->w, blending_mode};
#if defined(USE_PARALLEL) || defined(USE_OMP_PARALLEL)
    parallel::For(0, dst_rect.h, 1, blender, dst_rect.h * dst_rect.w);
#else
    for (int i = 0; i < dst_rect.h; i++) blender(i);
#endif

    SDL_UnlockSurface( image_surface );
    SDL_UnlockSurface( dst_surface );
    SDL_mutexV(mutex);
}

void AnimationInfo::blendOnSurface2( SDL_Surface *dst_surface, int dst_x, int dst_y,
                                     SDL_Rect &clip, int alpha )
{
    if ( image_surface == NULL ) return;
    if (scale_x == 0 || scale_y == 0) return;

    // project corner point and calculate bounding box
    int min_xy[2]={bounding_rect.x, bounding_rect.y};
    int max_xy[2]={bounding_rect.x+bounding_rect.w-1, 
                   bounding_rect.y+bounding_rect.h-1};

    // clip bounding box
    if (max_xy[0] <  clip.x) return;
    if (max_xy[0] >= clip.x + clip.w) max_xy[0] = clip.x + clip.w - 1;
    if (min_xy[0] >= clip.x + clip.w) return;
    if (min_xy[0] <  clip.x) min_xy[0] = clip.x;
    if (max_xy[1] <  clip.y) return;
    if (max_xy[1] >= clip.y + clip.h) max_xy[1] = clip.y + clip.h - 1;
    if (min_xy[1] >= clip.y + clip.h) return;
    if (min_xy[1] <  clip.y) min_xy[1] = clip.y;

    if (min_xy[1] <  0)               min_xy[1] = 0;
    if (max_xy[1] >= dst_surface->h)  max_xy[1] = dst_surface->h - 1;

    SDL_mutexP(mutex);
    SDL_LockSurface( dst_surface );
    SDL_LockSurface( image_surface );
    
    alpha &= 0xff;
    int pitch = image_surface->pitch / sizeof(ONSBuf);
    int cx2 = affine_pos.x*2 + affine_pos.w; // center x multiplied by 2
    int cy2 = affine_pos.y*2 + affine_pos.h; // center y multiplied by 2

    int src_rect[2][2]; // clipped source image bounding box
    src_rect[0][0] = affine_pos.x;
    src_rect[0][1] = affine_pos.y;
    src_rect[1][0] = affine_pos.x + affine_pos.w - 1;
    src_rect[1][1] = affine_pos.y + affine_pos.h - 1;
    if (src_rect[0][0] < 0) src_rect[0][0] = 0;
    if (src_rect[0][1] < 0) src_rect[0][1] = 0;
    if (src_rect[1][0] >= pos.w) src_rect[1][0] = pos.w - 1;
    if (src_rect[1][1] >= pos.h) src_rect[1][1] = pos.h - 1;
    // set pixel by inverse-projection with raster scan
    struct Blender {
        const int(*corner_xy)[2], *min_xy, *max_xy;
        const int(*inv_mat)[2];
        ONSBuf *const pixels;
        const int cellw, blending_mode;
        SDL_Surface *dst_surface;
        const int alpha, pitch, dst_x, dst_y, cx2, cy2;
        const int(*src_rect)[2];

        void blendLine(Uint32* line_buffer, int size, ONSBuf** dst_buffer_p) const {
            ONSBuf*& src_buffer = line_buffer;
            ONSBuf* dst_buffer = *dst_buffer_p;
            if (blending_mode == BLEND_NORMAL) {
#ifdef USE_SIMD
                if (size >= 4) {
                    using namespace simd;
#ifdef USE_SIMD_X86_AVX2
                    ivec256 zero = ivec256::zero();
                    uint8x32 mask = uint8x32::set8(3, 7, 11, 15, 19, 23, 27, 31);
                    uint8x32 amask =
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                        uint8x32::set(0, 0, 0, 0xFF);
#else
                        uint8x32::set(0xFF, 0, 0, 0);
#endif
                    ivec128 zerol = zero.lo();
                    uint8x16 maskl = mask.lo();
                    uint8x16 amaskl = amask.lo();
                    while (size >= 8) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                        unsigned char *alphap = (unsigned char *)line_buffer + 3;
#else
                        unsigned char *alphap = (unsigned char *)line_buffer;
#endif
                        blend8Pixel32(line_buffer, dst_buffer, uint16x16(alpha), mask, zero, amask);
                        dst_buffer += 8; line_buffer += 8; size -= 8;
                    }
#else
                    ivec128 zerol = ivec128::zero();
                    uint8x16 maskl = uint8x16::set4(3, 7, 11, 15);
                    uint8x16 amaskl =
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                        uint8x16::set(0, 0, 0, 0xFF);
#else
                        uint8x16::set(0xFF, 0, 0, 0);
#endif
#endif
                    while (size >= 4) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                        unsigned char* alphap = (unsigned char *)line_buffer + 3;
#else
                        unsigned char *alphap = (unsigned char *)line_buffer;
#endif
                        blend4Pixel32(line_buffer, dst_buffer, uint16x8(alpha), maskl, zerol, amaskl);
                        dst_buffer += 4; line_buffer += 4; size -= 4;
                    }
                }
#endif
                while (size > 0) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                    unsigned char* alphap = (unsigned char *)line_buffer + 3;
#else
                    unsigned char *alphap = (unsigned char *)line_buffer;
#endif
                    BLEND_PIXEL();
                    ++dst_buffer; ++line_buffer; --size;
                }
            }
            else {
                if (blending_mode == BLEND_ADD) {
                    while (size > 0) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                        unsigned char* alphap = (unsigned char *)line_buffer + 3;
#else
                        unsigned char *alphap = (unsigned char *)line_buffer;
#endif
                        ADDBLEND_PIXEL();
                        ++dst_buffer; ++line_buffer; --size;
                    }
                }
                else {
                    while (size > 0) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                        unsigned char* alphap = (unsigned char *)line_buffer + 3;
#else
                        unsigned char *alphap = (unsigned char *)line_buffer;
#endif
                        SUBBLEND_PIXEL();
                        ++dst_buffer; ++line_buffer; --size;
                    }
                }
            }
            *dst_buffer_p = dst_buffer;
        }
		
        void operator()(const int y) const {
            // calculate the start and end point for each raster scan
            int raster_min = min_xy[0], raster_max = max_xy[0];
            for (int i = 0; i < 4; i++) {
                int i2 = (i + 1) & 3; // = (i+1)%4
                if (corner_xy[i][1] == corner_xy[i2][1]) continue;
                int x = (corner_xy[i2][0] - corner_xy[i][0])*(y - corner_xy[i][1]) / (corner_xy[i2][1] - corner_xy[i][1]) + corner_xy[i][0];
                if (corner_xy[i2][1] - corner_xy[i][1] > 0) {
                    if (raster_min < x) raster_min = x;
                }
                else {
                    if (raster_max > x) raster_max = x;
                }
            }

            if (raster_min < 0)               raster_min = 0;
            if (raster_max >= dst_surface->w) raster_max = dst_surface->w - 1;

            ONSBuf *dst_buffer_s = (ONSBuf *)dst_surface->pixels + dst_surface->w * y + raster_min;
            ONSBuf *dst_buffer = dst_buffer_s;

            // inverse-projection
            int x_offset2 = (inv_mat[0][1] * (y - dst_y) >> 9) + cx2;
            int y_offset2 = (inv_mat[1][1] * (y - dst_y) >> 9) + cy2;
            if (raster_max - raster_min + 1 <= 0) return;
            Uint32* line_buffer = new Uint32[raster_max - raster_min + 1];
            int line_pos = 0;
            for (int x = raster_min - dst_x; x <= raster_max - dst_x; x++, dst_buffer++) {
                int x2 = ((inv_mat[0][0] * x >> 9) + x_offset2) >> 1;
                int y2 = ((inv_mat[1][0] * x >> 9) + y_offset2) >> 1;

                if (x2 < src_rect[0][0] || x2 >= src_rect[1][0] ||
                    y2 < src_rect[0][1] || y2 >= src_rect[1][1]) {
                    blendLine(line_buffer, line_pos, &dst_buffer_s);
                    dst_buffer_s += line_pos;
                    line_pos = 0;
                    continue;
                }

                ONSBuf *src_buffer = pixels + pitch * y2 + x2 + cellw;
                line_buffer[line_pos++] = *src_buffer;
            }
            blendLine(line_buffer, line_pos, &dst_buffer_s);
            delete[] line_buffer;
        }
    } blender = {corner_xy, min_xy, max_xy, inv_mat, (ONSBuf*)image_surface->pixels, pos.w*current_cell, blending_mode, dst_surface, alpha, pitch, dst_x, dst_y, cx2, cy2, src_rect};
#if defined(USE_PARALLEL) || defined(USE_OMP_PARALLEL)
    parallel::For(min_xy[1], max_xy[1] + 1, 1, blender, (max_xy[1] - min_xy[1] + 1) * (max_xy[0] + 1 - min_xy[0]) * 4);
#else
    for (int y = min_xy[1]; y <= max_xy[1]; y++) blender(y);
#endif
    
    // unlock surface
    SDL_UnlockSurface( image_surface );
    SDL_UnlockSurface( dst_surface );
    SDL_mutexV(mutex);
}

#define BLEND_TEXT_ALPHA()\
{\
    Uint32 mask2 = *src_buffer;                                         \
    if (mask2 == 255){													\
    	*dst_buffer = src_color;										\
    }																	\
    else if (mask2 != 0){                                               \
        Uint32 alpha = *dst_buffer >> 24;                               \
        Uint32 mask1 = ((0xff ^ mask2)*alpha) >> 8;                     \
       	alpha = inv_alpha_lut[ mask1+mask2 ];                           \
        Uint32 mask_rb = ((*dst_buffer & 0xff00ff) * mask1 +            \
                          src_color1 * mask2);                          \
        mask_rb = (((mask_rb >> 16) * alpha) & 0x00ff0000) |            \
                  (((mask_rb & 0xffff) * alpha >> 16) & 0xff);          \
        Uint32 mask_g = (((*dst_buffer & 0x00ff00) * mask1 +            \
                         src_color2 * mask2) * alpha >> 16) & 0x00ff00; \
        *dst_buffer = mask_rb | mask_g | ((mask1+mask2) << 24);         \
    }                                                                   \
}
// Originally, the above looks like this.
//        Uint32 alpha = *dst_buffer >> 24;                             
//        Uint32 mask1 = ((0xff ^ mask2)*alpha) >> 8;                     
//        alpha = 0xff ^ ((0xff ^ alpha)*(0xff ^ mask2) >> 8);            
//        Uint32 mask_rb =  ((*dst_buffer & 0xff00ff) * mask1 +           
//                           src_color1 * mask2);                         
//        mask_rb = ((mask_rb / alpha) & 0x00ff0000) |                    
//            (((mask_rb & 0xffff) / alpha) & 0xff);                      
//        Uint32 mask_g  = (((*dst_buffer & 0x00ff00) * mask1 +           
//                           src_color2 * mask2) / alpha) & 0x00ff00;     
//        *dst_buffer = mask_rb | mask_g | (alpha << 24);                 

// used to draw characters on text_surface
// Alpha = 1 - (1-Da)(1-Sa)
// Color = (DaSaSc + Da(1-Sa)Dc + Sa(1-Da)Sc)/A
void AnimationInfo::blendText( SDL_Surface *surface, int dst_x, int dst_y, SDL_Color &color,
                               SDL_Rect *clip, bool rotate_flag )
{
    if (image_surface == NULL || surface == NULL) return;
    
    SDL_Rect dst_rect;
    dst_rect.x = dst_x;
    dst_rect.y = dst_y;
    dst_rect.w = surface->w;
    dst_rect.h = surface->h;
    if (rotate_flag){
        dst_rect.w = surface->h;
        dst_rect.h = surface->w;
    }
    SDL_Rect src_rect = {0, 0, 0, 0};
    SDL_Rect clipped_rect;

    /* ---------------------------------------- */
    /* 1st clipping */
    if ( clip ){
        if ( doClipping( &dst_rect, clip, &clipped_rect ) ) return;

        src_rect.x += clipped_rect.x;
        src_rect.y += clipped_rect.y;
    }
    
    /* ---------------------------------------- */
    /* 2nd clipping */
    SDL_Rect clip_rect;
    clip_rect.x = clip_rect.y = 0;
    clip_rect.w = image_surface->w;
    clip_rect.h = image_surface->h;
    if ( doClipping( &dst_rect, &clip_rect, &clipped_rect ) ) return;
    
    src_rect.x += clipped_rect.x;
    src_rect.y += clipped_rect.y;

    /* ---------------------------------------- */
    
    SDL_mutexP(mutex);
    SDL_LockSurface( surface );
    SDL_LockSurface( image_surface );
    
    SDL_PixelFormat *fmt = image_surface->format;

    int pitch = image_surface->pitch / sizeof(ONSBuf);
    Uint32 src_color1 = (((color.r >> fmt->Rloss) << fmt->Rshift) |
                         ((color.b >> fmt->Bloss) << fmt->Bshift));
    Uint32 src_color2 =  ((color.g >> fmt->Gloss) << fmt->Gshift);
    Uint32 src_color  = src_color1 | src_color2 | fmt->Amask;

    ONSBuf *dst_buffer = (ONSBuf *)image_surface->pixels + pitch * dst_rect.y + image_surface->w*current_cell/num_of_cells + dst_rect.x;

    if (!rotate_flag){
        unsigned char *src_buffer = (unsigned char*)surface->pixels + 
            surface->pitch*src_rect.y + src_rect.x;
        for (int i=dst_rect.h ; i!=0 ; i--){
            for (int j=dst_rect.w ; j!=0 ; j--){
                BLEND_TEXT_ALPHA();
                src_buffer++;
                dst_buffer++;
            }
            dst_buffer += pitch - dst_rect.w;
            src_buffer += surface->pitch - dst_rect.w;
        }
    }
    else{
        for (int i=0 ; i<dst_rect.h ; i++){
            unsigned char *src_buffer = (unsigned char*)surface->pixels + 
                surface->pitch*(surface->h - src_rect.x - 1) + src_rect.y + i;
            for (int j=dst_rect.w ; j!=0 ; j--){
                BLEND_TEXT_ALPHA();
                src_buffer -= surface->pitch;
                dst_buffer++;
            }
            dst_buffer += pitch - dst_rect.w;
        }
    }

    SDL_UnlockSurface( image_surface );
    SDL_UnlockSurface( surface );
	SDL_mutexV(mutex);
}

void AnimationInfo::calcAffineMatrix()
{
    // calculate forward matrix
    // |mat[0][0] mat[0][1]|
    // |mat[1][0] mat[1][1]|
    int cos_i = 1024, sin_i = 0;
    if (rot != 0){
        cos_i = (int)(1024.0 * cos(-M_PI*rot/180));
        sin_i = (int)(1024.0 * sin(-M_PI*rot/180));
    }
    mat[0][0] =  cos_i*scale_x/100;
    mat[0][1] = -sin_i*scale_y/100;
    mat[1][0] =  sin_i*scale_x/100;
    mat[1][1] =  cos_i*scale_y/100;

    // calculate bounding box
    int min_xy[2] = { 0, 0 }, max_xy[2] = { 0, 0 };
    for (int i=0 ; i<4 ; i++){
        int c_x = (i<2)?(-affine_pos.w/2):(affine_pos.w/2);
        int c_y = ((i+1)&2)?(affine_pos.h/2):(-affine_pos.h/2);
        if (scale_x < 0) c_x = -c_x;
        if (scale_y < 0) c_y = -c_y;
        corner_xy[i][0] = (mat[0][0] * c_x + mat[0][1] * c_y) / 1024 + pos.x;
        corner_xy[i][1] = (mat[1][0] * c_x + mat[1][1] * c_y) / 1024 + pos.y;

        if (i==0 || min_xy[0] > corner_xy[i][0]) min_xy[0] = corner_xy[i][0];
        if (i==0 || max_xy[0] < corner_xy[i][0]) max_xy[0] = corner_xy[i][0];
        if (i==0 || min_xy[1] > corner_xy[i][1]) min_xy[1] = corner_xy[i][1];
        if (i==0 || max_xy[1] < corner_xy[i][1]) max_xy[1] = corner_xy[i][1];
    }

    bounding_rect.x = min_xy[0];
    bounding_rect.y = min_xy[1];
    bounding_rect.w = max_xy[0]-min_xy[0]+1;
    bounding_rect.h = max_xy[1]-min_xy[1]+1;
    
    // calculate inverse matrix
    int denom = scale_x*scale_y;
    if (denom == 0) return;

    inv_mat[0][0] =  mat[1][1] * 10000 / denom;
    inv_mat[0][1] = -mat[0][1] * 10000 / denom;
    inv_mat[1][0] = -mat[1][0] * 10000 / denom;
    inv_mat[1][1] =  mat[0][0] * 10000 / denom;
}

SDL_Surface *AnimationInfo::allocSurface( int w, int h, Uint32 texture_format )
{
    SDL_Surface *surface;
    if (texture_format == SDL_PIXELFORMAT_RGB565)
        surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 16, 0xf800, 0x07e0, 0x001f, 0);
    else if (texture_format == SDL_PIXELFORMAT_ABGR8888)
        surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    else // texture_format == SDL_PIXELFORMAT_ARGB8888
        surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

    return surface;
}

SDL_Surface *AnimationInfo::alloc32bitSurface( int w, int h, Uint32 texture_format )
{
    if (texture_format == SDL_PIXELFORMAT_RGB565)
        return allocSurface(w, h, SDL_PIXELFORMAT_ARGB8888);
    else
        return allocSurface(w, h, texture_format);
}

void AnimationInfo::allocImage( int w, int h, Uint32 texture_format )
{
    if (!image_surface ||
        image_surface->w != w ||
        image_surface->h != h){
        deleteSurface(false);

        this->texture_format = texture_format;
        SDL_mutexP(mutex);
        image_surface = allocSurface( w, h, texture_format );
        SDL_mutexV(mutex);      
    }

    abs_flag = true;
    pos.w = w / num_of_cells;
    pos.h = h;

    affine_pos.x = 0;
    affine_pos.y = 0;
    affine_pos.w = pos.w;
    affine_pos.h = pos.h;
}

void AnimationInfo::copySurface( SDL_Surface *surface, SDL_Rect *src_rect, SDL_Rect *dst_rect )
{
    if (!image_surface || !surface) return;
    
    SDL_Rect _dst_rect = {0, 0};
    if (dst_rect) _dst_rect = *dst_rect;

    SDL_Rect _src_rect;
    _src_rect.x = _src_rect.y = 0;
    _src_rect.w = surface->w;
    _src_rect.h = surface->h;
    if (src_rect) _src_rect = *src_rect;

    if (_src_rect.x >= surface->w) return;
    if (_src_rect.y >= surface->h) return;
    
    if (_src_rect.x+_src_rect.w >= surface->w)
        _src_rect.w = surface->w - _src_rect.x;
    if (_src_rect.y+_src_rect.h >= surface->h)
        _src_rect.h = surface->h - _src_rect.y;
        
    if (_dst_rect.x+_src_rect.w > image_surface->w)
        _src_rect.w = image_surface->w - _dst_rect.x;
    if (_dst_rect.y+_src_rect.h > image_surface->h)
        _src_rect.h = image_surface->h - _dst_rect.y;

    SDL_mutexP(mutex);
    SDL_LockSurface( surface );
    SDL_LockSurface( image_surface );

    int i;
    for (i=0 ; i<_src_rect.h ; i++)
        memcpy( (ONSBuf*)((unsigned char*)image_surface->pixels + image_surface->pitch * (_dst_rect.y+i)) + _dst_rect.x,
                (ONSBuf*)((unsigned char*)surface->pixels + surface->pitch * (_src_rect.y+i)) + _src_rect.x,
                _src_rect.w*sizeof(ONSBuf) );

    SDL_UnlockSurface( image_surface );
    SDL_UnlockSurface( surface );
    SDL_mutexV(mutex);
}

void AnimationInfo::fill( Uint8 r, Uint8 g, Uint8 b, Uint8 a )
{
    if (!image_surface) return;
    
    SDL_mutexP(mutex);
    SDL_LockSurface( image_surface );

    SDL_PixelFormat *fmt = image_surface->format;
    Uint32 rgb = (((r >> fmt->Rloss) << fmt->Rshift) |
                  ((g >> fmt->Gloss) << fmt->Gshift) |
                  ((b >> fmt->Bloss) << fmt->Bshift) |
                  ((a >> fmt->Aloss) << fmt->Ashift));

    int pitch = image_surface->pitch / sizeof(ONSBuf);
    for (int i=0 ; i<image_surface->h ; i++){
        ONSBuf *dst_buffer = (ONSBuf *)image_surface->pixels + pitch*i;
        for (int j=0 ; j<image_surface->w ; j++){
            *dst_buffer++ = rgb;
        }
    }
    SDL_UnlockSurface( image_surface );
    SDL_mutexV(mutex);
}

SDL_Surface *AnimationInfo::setupImageAlpha( SDL_Surface *surface, SDL_Surface *surface_m, bool has_alpha )
{
    if (surface == NULL) return NULL;

    SDL_LockSurface( surface );
    Uint32 *buffer = (Uint32 *)surface->pixels;
    SDL_PixelFormat *fmt = surface->format;

    int w = surface->w;
    int h = surface->h;
    int w2 = w / num_of_cells;
    orig_pos.w = w;
    orig_pos.h = h;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    unsigned char *alphap = (unsigned char *)buffer + 3;
#else
    unsigned char *alphap = (unsigned char *)buffer;
#endif

    Uint32 ref_color = 0;
    if ( trans_mode == TRANS_TOPLEFT ){
        ref_color = *buffer;
    }
    else if ( trans_mode == TRANS_TOPRIGHT ){
        ref_color = *(buffer + surface->w - 1);
    }
    else if ( trans_mode == TRANS_DIRECT ) {
        ref_color = 
            direct_color[0] << fmt->Rshift |
            direct_color[1] << fmt->Gshift |
            direct_color[2] << fmt->Bshift;
    }
    ref_color &= 0xffffff;

    int i, j, c;
    if ( trans_mode == TRANS_ALPHA && !has_alpha ){
        const int w22 = w2/2;
        const int w3 = w22 * num_of_cells;
        orig_pos.w = w3;
        SDL_PixelFormat *fmt = surface->format;
        SDL_Surface *surface2 = SDL_CreateRGBSurface( SDL_SWSURFACE, w3, h,
                                                      fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask );
        SDL_LockSurface( surface2 );
        Uint32 *buffer2 = (Uint32 *)surface2->pixels;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        alphap = (unsigned char *)buffer2 + 3;
#else
        alphap = (unsigned char *)buffer2;
#endif

        for (i=h ; i!=0 ; i--){
            for (c=num_of_cells ; c!=0 ; c--){
                for (j=w22 ; j!=0 ; j--, buffer++, alphap+=4){
                    *buffer2++ = *buffer;
                    *alphap = (*(buffer + w22) & 0xff) ^ 0xff;
                }
                buffer += (w2 - w22);
            }
            buffer  +=  surface->w  - w2 *num_of_cells;
            buffer2 +=  surface2->w - w22*num_of_cells;
            alphap  += (surface2->w - w22*num_of_cells)*4;
        }

        SDL_UnlockSurface( surface );
        SDL_FreeSurface( surface );
        surface = surface2;
    }
    else if ( trans_mode == TRANS_MASK ){
        if (surface_m){
            SDL_LockSurface( surface_m );
            const int mw  = surface_m->w;
            const int mwh = surface_m->w * surface_m->h;

            int i2 = 0;
            for (i=h ; i!=0 ; i--){
                Uint32 *buffer_m = (Uint32 *)surface_m->pixels + i2;
                for (c=num_of_cells ; c!=0 ; c--){
                    int j2 = 0;
                    for (j=w2 ; j!=0 ; j--, buffer++, alphap+=4){
                        *alphap = (*(buffer_m + j2) & 0xff) ^ 0xff;
                        if (j2 >= mw) j2 = 0;
                        else          j2++;
                    }
                }
                if (i2 >= mwh) i2 = 0;
                else           i2 += mw;
            }
            SDL_UnlockSurface( surface_m );
        }
    }
    else if ( trans_mode == TRANS_TOPLEFT ||
              trans_mode == TRANS_TOPRIGHT ||
              trans_mode == TRANS_DIRECT ){
        for (i=h ; i!=0 ; i--){
            for (j=w ; j!=0 ; j--, buffer++, alphap+=4){
                if ( (*buffer & 0xffffff) == ref_color )
                    *alphap = 0x00;
                else
                    *alphap = 0xff;
            }
        }
    }
    else if ( trans_mode == TRANS_STRING ){
        for (i=h ; i!=0 ; i--){
            for (j=w ; j!=0 ; j--, buffer++, alphap+=4)
                *alphap = *buffer >> 24;
        }
    }
    else if ( trans_mode != TRANS_ALPHA ){ // TRANS_COPY
        for (i=h ; i!=0 ; i--){
            for (j=w ; j!=0 ; j--, buffer++, alphap+=4)
                *alphap = 0xff;
        }
    }
    
    SDL_UnlockSurface( surface );

    return surface;
}

void AnimationInfo::setImage( SDL_Surface *surface, Uint32 texture_format )
{
    if (surface == NULL) return;

    this->texture_format = texture_format;
    image_surface = surface; // deleteSurface() should be called beforehand
    allocImage(surface->w, surface->h, texture_format);
}

unsigned char AnimationInfo::getAlpha(int x, int y)
{
    if (image_surface == NULL) return 0;

    x -= pos.x;
    y -= pos.y;
    int offset_x = (image_surface->w/num_of_cells)*current_cell;
    
    unsigned char alpha = 0;
    int pitch = image_surface->pitch / 4;
    SDL_mutexP(mutex);
    SDL_LockSurface( image_surface );
    ONSBuf *buf = (ONSBuf *)image_surface->pixels + pitch*y + offset_x + x;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    alpha = *((unsigned char *)buf + 3);
#else
    alpha = *((unsigned char *)buf);
#endif
    SDL_UnlockSurface( image_surface );
    SDL_mutexV(mutex);

    return alpha;
}

#ifdef USE_SMPEG
void AnimationInfo::convertFromYUV(SDL_Overlay *src)
{
    SDL_mutexP(mutex);
    if (!image_surface){
        SDL_mutexV(mutex);
        return;
    }
    
    SDL_Surface *ls = image_surface;

    SDL_LockSurface(ls);
    SDL_PixelFormat *fmt = ls->format;
    for (int i=0 ; i<ls->h ; i++){
        int i2 = src->h*i/ls->h;
        int i3 = (src->h/2)*i/ls->h;
        Uint8 *y = src->pixels[0]+src->pitches[0]*i2;
        Uint8 *v = src->pixels[1]+src->pitches[1]*i3;
        Uint8 *u = src->pixels[2]+src->pitches[2]*i3;
        ONSBuf *p = (ONSBuf*)ls->pixels + (ls->pitch/sizeof(ONSBuf))*i;
        
        for (int j=0 ; j<ls->w ; j++){
            int j2 = src->w*j/ls->w;
            int j3 = (src->w/2)*j/ls->w;

            Sint32 y2 = *(y+j2);
            Sint32 u2 = *(u+j3)-128;
            Sint32 v2 = *(v+j3)-128;

            //Sint32 r = 1.0*y2            + 1.402*v2;
            //Sint32 g = 1.0*y2 - 0.344*u2 - 0.714*v2;
            //Sint32 b = 1.0*y2 + 1.772*u2;
            y2 <<= 10;
            Sint32 r = y2           + 1436*v2;
            Sint32 g = y2 -  352*u2 -  731*v2;
            Sint32 b = y2 + 1815*u2;

            if (r < 0) r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;

            *(p+j) =
                (((r >> (10+fmt->Rloss)) & 0xff) << fmt->Rshift) |
                (((g >> (10+fmt->Gloss)) & 0xff) << fmt->Gshift) |
                (((b >> (10+fmt->Bloss)) & 0xff) << fmt->Bshift) |
                AMASK;
        }
    }
    SDL_UnlockSurface(ls);

    SDL_mutexV(mutex);
}
#endif

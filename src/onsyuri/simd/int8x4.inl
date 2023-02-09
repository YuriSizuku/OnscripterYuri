/* -*- C++ -*-
*
*  int8x4.inl
*
*  Copyright (C) 2015 jh10001 <jh10001@live.cn>
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
#ifndef __SIMD_H__
#error "This file must be included through simd.h"
#endif
#include "int16x4.h"

namespace simd {
  //Arithmetic
  inline uint8x4 operator+(uint8x4 a, uint8x4 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_add_epi8(a, b); //PADDW xmm1, xmm2
#elif USE_SIMD_ARM_NEON
    return vadd_u8(a, b);
#endif
  }

  inline uint8x4 operator+=(uint8x4 &a, uint8x4 b) {
    return a = a + b;
  }

  inline uint8x4 adds(uint8x4 a, uint8x4 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_adds_epu8(a, b); //PADDUSB xmm1, xmm2
#elif USE_SIMD_ARM_NEON
    return vqadd_u8(a, b);
#endif
  }

  //Load
  inline uint8x4 load(const uint32_t *rm) {
#ifdef USE_SIMD_X86_SSE2
    return uint8x4::cvt2vec(*rm);
#elif USE_SIMD_ARM_NEON
    uint32x2_t a;
    return vreinterpret_u8_u32(vld1_lane_u32(rm, a, 0));
#endif
  }

  //Set
  inline void setzero(uint8x4 &a) {
#ifdef USE_SIMD_X86_SSE2
    a = _mm_setzero_si128(); //PXOR xmm, xmm
#elif USE_SIMD_ARM_NEON
    a = veor_u8(a, a);
#endif
  }

  //Swizzle
  inline uint16x4 widen(uint8x4 a,uint8x4 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_unpacklo_epi8(a, b);
#elif USE_SIMD_ARM_NEON
    return vget_low_u16(vmovl_u8(a));
#endif
  }
}
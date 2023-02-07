/* -*- C++ -*-
*
*  int8x32.inl
*
*  Copyright (C) 2018-2019 jh10001 <jh10001@live.cn>
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
#include "int16x16.h"

namespace simd {
  //Arithmetic
  inline uint8x32 operator+(uint8x32 a, uint8x32 b){
#ifdef USE_SIMD_X86_AVX2
    return _mm256_add_epi8(a, b);  //PADDB xmm1, xmm2
#endif
  }

  inline uint8x32 operator+=(uint8x32 &a, uint8x32 b){
    return a = a + b;
  }

  inline uint8x32 adds(uint8x32 a, uint8x32 b) {
#ifdef USE_SIMD_X86_AVX2
    return _mm256_adds_epu8(a, b);  //PADDUSB xmm1, xmm2
#endif
  }

  //Logical
  inline uint8x32 operator|(uint8x32 a, uint8x32 b) {
#ifdef USE_SIMD_X86_AVX2
    return _mm256_or_si256(a, b);  //POR xmm1, xmm2
#endif
  }

  inline uint8x32 operator|=(uint8x32 &a, uint8x32 b) {
    return a = a | b;
  }

  //Set
  inline void setzero(uint8x32 &a) {
#ifdef USE_SIMD_X86_AVX2
    a = _mm256_setzero_si256(); //PXOR xmm, xmm
#endif
  }

  //Shuffle
  inline uint8x32 shuffle(uint8x32 a, uint8x32 mask) {
#ifdef USE_SIMD_X86_AVX2
	  return _mm256_shuffle_epi8(a, mask);
#endif
  }

  //Swizzle
  inline uint16x16 widen_hi(uint8x32 a, uint8x32 b) {
#ifdef USE_SIMD_X86_AVX2
    return _mm256_unpackhi_epi8(a, b);
#endif
  }

  inline uint16x16 widen_lo(uint8x32 a, uint8x32 b) {
#ifdef USE_SIMD_X86_AVX2
    return _mm256_unpacklo_epi8(a, b);
#endif
  }
}
/* -*- C++ -*-
*
*  int8x4.h
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
#pragma once

#ifndef __SIMD_H__
#error "This file must be included through simd.h"
#endif
#include <stdint.h>

namespace simd {
  class uint8x4 {
#ifdef USE_SIMD_X86_SSE2
    __m128i v_;
#elif USE_SIMD_ARM_NEON
    uint8x8_t v_;
#endif
  public:
    uint8x4() = default;
    uint8x4(const uint8x4&) = default;
    uint8x4 &operator=(const uint8x4&) = default;
#ifdef USE_SIMD_X86_SSE2
    uint8x4(__m128i v) : v_(v) {};
    operator __m128i() const { return v_; }
    static uint8x4 cvt2vec(uint32_t rm) { return _mm_cvtsi32_si128(rm);  /* MOVD xmm, r32 */ }
    static uint32_t cvt2i32(uint8x4 a) { return _mm_cvtsi128_si32(a);  /* MOVD r32, xmm */ }
#elif USE_SIMD_ARM_NEON
    uint8x4(uint8x8_t v) : v_(v) {};
    operator uint8x8_t() const { return v_; }
    static uint8x4 cvt2vec(uint32_t rm) {
      uint32x2_t r;
      return vreinterpret_u8_u32(vset_lane_u32(rm, r, 0));
    }
    static uint32_t cvt2i32(uint8x4 a) { return vget_lane_u32(vreinterpret_u32_u8(a.v_), 0); }
#endif
  };

  //Arithmetic
  static uint8x4 operator+(uint8x4 a, uint8x4 b);

  static uint8x4 operator+=(uint8x4 &a, uint8x4 b);

  static uint8x4 adds(uint8x4 a, uint8x4 b);

  //load
  static uint8x4 load(const uint32_t *rm);

  //Set
  static void setzero(uint8x4 &a);

  //Swizzle
  class uint16x4;
  static uint16x4 widen(uint8x4 a, uint8x4 b);
}
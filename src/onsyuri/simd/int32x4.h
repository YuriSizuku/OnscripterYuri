/* -*- C++ -*-
*
*  int32x4.h
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
  class uint32x4 {
#ifdef USE_SIMD_X86_SSE2
    __m128i v_;
#elif USE_SIMD_ARM_NEON
    uint32x4_t v_;
#endif
  public:
    uint32x4() = default;
    uint32x4(const uint32x4&) = default;
    uint32x4 &operator=(const uint32x4&) = default;
#ifdef USE_SIMD_X86_SSE2
    uint32x4(__m128i v) : v_(v) {}
    operator __m128i() const { return v_; }
    uint32x4(uint32_t rm) : v_(_mm_set1_epi32(rm)) {}
    static uint32x4 cvt2vec(uint32_t rm) { return _mm_cvtsi32_si128(rm);  /* MOVD xmm, r32 */ }
    static uint32_t cvt2i32(uint32x4 a) { return _mm_cvtsi128_si32(a);  /* MOVD r32, xmm */ }
#elif USE_SIMD_ARM_NEON
    uint32x4(uint32x4_t v) : v_(v) {};
    operator uint32x4_t() const { return v_; }
    uint32x4(uint32_t rm) { v_ = vdupq_n_u32(rm); }
    static uint32x4 cvt2vec(uint32_t rm) {
      uint32x4 r;
      r = vsetq_lane_u32(rm, r, 0);
      return r;
    }
    static uint32_t cvt2i32(uint32x4 a) { return vgetq_lane_u32(a, 0); }
#endif
  };

  //Logical
  static uint32x4 operator|(uint32x4 a, uint32x4 b);

  static uint32x4 operator|=(uint32x4 &a, uint32x4 b);
}

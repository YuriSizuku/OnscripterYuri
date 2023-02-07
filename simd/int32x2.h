/* -*- C++ -*-
*
*  int32x2.h
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
  class uint32x2 {
#ifdef USE_SIMD_X86_SSE2
    __m128i v_;
#elif USE_SIMD_ARM_NEON
    uint32x2_t v_;
#endif
  public:
    uint32x2() = default;
    uint32x2(const uint32x2&) = default;
    uint32x2 &operator=(const uint32x2&) = default;
#ifdef USE_SIMD_X86_SSE2
    uint32x2(__m128i v) : v_(v) {};
    operator __m128i() const { return v_; }
    uint32x2(uint32_t rm) { v_ = _mm_set1_epi32(rm); }
    static uint32x2 cvt2vec(uint32_t rm) { return _mm_cvtsi32_si128(rm);  /* MOVD xmm, r32 */ }
    static uint32_t cvt2i32(uint32x2 a) { return _mm_cvtsi128_si32(a);  /* MOVD r32, xmm */ }
    __m128i cvt2vu8() const { return v_; }
#elif USE_SIMD_ARM_NEON
    uint32x2(uint32x2_t v) : v_(v) {};
    operator uint32x2_t() const { return v_; }
    uint32x2(uint32_t rm) { v_ = vdup_n_u32(rm); }
    static uint32x2 cvt2vec(uint32_t rm) {
      uint32x2 r;
      r = vset_lane_u32(rm, r, 0);
      return r;
    }
    static uint32_t cvt2i32(uint32x2 a) { return vget_lane_u32(a, 0); }
    uint8x8_t cvt2vu8() const { return vreinterpret_u8_u32(v_); }
#endif
  };
}

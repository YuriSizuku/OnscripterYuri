/* -*- C++ -*-
*
*  int16x8.h
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
  class uint8x16;
  class uint16x4;
  class uint16x8 {
#ifdef USE_SIMD_X86_SSE2
    __m128i v_;
#elif USE_SIMD_ARM_NEON
    uint16x8_t v_;
#endif
  public:
    uint16x8() = default;
    uint16x8(const uint16x8&) = default;
    uint16x8 &operator=(const uint16x8&) = default;
#ifdef USE_SIMD_X86_SSE2
    uint16x8(__m128i v) : v_(v) {};
    operator __m128i() const { return v_; }
    uint16x8(uint16_t rm) { v_ = _mm_set1_epi16(rm); }
    uint16x4 lo() { return v_; }
#elif USE_SIMD_ARM_NEON
    uint16x8(uint16x8_t v) : v_(v) {};
    operator uint16x8_t() const { return v_; }
    uint16x8(uint16_t rm) { v_ = vdupq_n_u16(rm); }
    uint16x4 lo() { return vget_low_u16(v_); }
#endif
    //Swizzle
    static uint16x8 set2(uint16_t rm1, uint16_t rm2) {
#ifdef USE_SIMD_X86_SSE2
      uint16x8 r;
      r = _mm_cvtsi32_si128(rm1);  // MOVD r32, xmm
      r = _mm_shufflelo_epi16(r, 0);  //PSHUFLW xmm1, xmm2, imm
      r = _mm_insert_epi16(r, rm2, 4);  //PINSRW xmm, r32, imm
      r = _mm_shufflehi_epi16(r, 0);  //PSUFHW xmm1, xmm2, imm
      return r;
#elif USE_SIMD_ARM_NEON
      uint16x4_t rl = vdup_n_u16(rm1), rr = vdup_n_u16(rm2);
      return vcombine_u16(rl, rr);
#endif
    };
  };

  //Arithmetic
  static uint16x8 operator-(uint16x8 a, uint16x8 b);

  static uint16x8 operator-=(uint16x8 &a, uint16x8 b);

  static uint16x8 operator*(uint16x8 a, uint16x8 b);

  static uint16x8 operator*=(uint16x8 &a, uint16x8 b);

  //Miscellaneous
  static uint8x16 pack_hz(uint16x8 a, uint16x8 b);

  //Set
  static void setzero(uint16x8 &a);

  //Shift
  static uint16x8 operator>>(uint16x8 a, immint<8> imm8);

  static uint16x8 operator>>=(uint16x8 &a, immint<8> imm8);
}

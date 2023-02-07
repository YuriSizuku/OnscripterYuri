/* -*- C++ -*-
*
*  int16x4.h
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

namespace simd {
  class uint16x4 {
#ifdef USE_SIMD_X86_SSE2
    __m128i v_;
#elif USE_SIMD_ARM_NEON
    uint16x4_t v_;
#endif
  public:
    uint16x4() = default;
    uint16x4(const uint16x4&) = default;
    uint16x4 &operator=(const uint16x4&) = default;
#ifdef USE_SIMD_X86_SSE2
    uint16x4(__m128i v) : v_(v) {};
    operator __m128i() const { return v_; }
    uint16x4(uint16_t rm) { v_ = _mm_shufflelo_epi16(_mm_cvtsi32_si128(rm), 0);  /*MOVD r32, xmm, PSHUFLW xmm1, xmm2, imm*/ }
#elif USE_SIMD_ARM_NEON
    uint16x4(uint16x4_t v) : v_(v) {};
    operator uint16x4_t() const { return v_; }
    uint16x4(uint16_t rm) { v_ = vdup_n_u16(rm); }
#endif
  };

  //Arithmetic
  static uint16x4 operator+(uint16x4 a, uint16x4 b);

  static uint16x4 operator+=(uint16x4 &a, uint16x4 b);

  static uint16x4 operator-(uint16x4 a, uint16x4 b);

  static uint16x4 operator-=(uint16x4 &a, uint16x4 b);

  static uint16x4 operator*(uint16x4 a, uint16x4 b);

  static uint16x4 operator*=(uint16x4 &a, uint16x4 b);

  //Miscellaneous
  static uint8x4 narrow_hz(uint16x4 a);

  //Shift
  static uint16x4 operator>>(uint16x4 a, immint<8> imm8);

  static uint16x4 operator>>=(uint16x4 &a, immint<8> imm8);
}
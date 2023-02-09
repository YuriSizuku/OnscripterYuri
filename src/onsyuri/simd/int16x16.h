/* -*- C++ -*-
*
*  int16x16.h
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
#pragma once

#ifndef __SIMD_H__
#error "This file must be included through simd.h"
#endif
#include <stdint.h>

namespace simd {
  class uint8x32;
  class uint16x8;
  class uint16x16 {
#ifdef USE_SIMD_X86_AVX2
    __m256i v_;
#endif
  public:
    uint16x16() = default;
    uint16x16(const uint16x16&) = default;
    uint16x16 &operator=(const uint16x16&) = default;
#ifdef USE_SIMD_X86_AVX2
    uint16x16(__m256i v) : v_(v) {};
    operator __m256i() const { return v_; }
    uint16x16(uint16_t rm) { v_ = _mm256_set1_epi16(rm); }
    static uint16x16 set4(uint16_t m1, uint16_t m2, uint16_t m3, uint16_t m4) {
      return _mm256_set_epi16(m4, m4, m4, m4, m3, m3, m3, m3, m2, m2, m2, m2, m1, m1, m1, m1);
    }
    uint16x8 lo() { return _mm256_castsi256_si128(v_); }
#endif
  };

  //Arithmetic
  static uint16x16 operator-(uint16x16 a, uint16x16 b);

  static uint16x16 operator-=(uint16x16 &a, uint16x16 b);

  static uint16x16 operator*(uint16x16 a, uint16x16 b);

  static uint16x16 operator*=(uint16x16 &a, uint16x16 b);

  //Miscellaneous
  static uint8x32 pack_hz(uint16x16 a, uint16x16 b);

  //Set
  static void setzero(uint16x16 &a);

  //Shift
  static uint16x16 operator>>(uint16x16 a, immint<8> imm8);

  static uint16x16 operator>>=(uint16x16 &a, immint<8> imm8);
}

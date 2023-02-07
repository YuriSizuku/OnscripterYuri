/* -*- C++ -*-
*
*  int8x32.h
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

namespace simd {
  class uint8x32 {
#ifdef USE_SIMD_X86_AVX2
    __m256i v_;
#endif
  public:
    uint8x32() = default;
    uint8x32(const uint8x32&) = default;
    uint8x32 &operator=(const uint8x32&) = default;
#ifdef USE_SIMD_X86_AVX2
    uint8x32(__m256i v) : v_(v) {};
    operator __m256i() const { return v_; }
	  uint8x16 lo() { return _mm256_castsi256_si128(v_); }
#endif

    static uint8x32 set(uint8_t rm1, uint8_t rm2, uint8_t rm3, uint8_t rm4) {
#ifdef USE_SIMD_X86_AVX2
      return _mm256_set_epi8(rm4, rm3, rm2, rm1, rm4, rm3, rm2, rm1, rm4, rm3, rm2, rm1, rm4, rm3, rm2, rm1,
        rm4, rm3, rm2, rm1, rm4, rm3, rm2, rm1, rm4, rm3, rm2, rm1, rm4, rm3, rm2, rm1);
#endif
    }

    static uint8x32 set8(uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, uint8_t m5, uint8_t m6, uint8_t m7, uint8_t m8) {
#ifdef USE_SIMD_X86_AVX2
      return _mm256_set_epi8(m8, m8, m8, m8, m7, m7, m7, m7, m6, m6, m6, m6, m5, m5, m5, m5,
        m4, m4, m4, m4, m3, m3, m3, m3, m2, m2, m2, m2, m1, m1, m1, m1);
#endif
    }
  };

  //Arithmetic
  static uint8x32 operator+(uint8x32 a, uint8x32 b);

  static uint8x32 operator+=(uint8x32 &a, uint8x32 b);

  static uint8x32 adds(uint8x32 a, uint8x32 b);

  //Logical
  static uint8x32 operator|(uint8x32 a, uint8x32 b);

  static uint8x32 operator|=(uint8x32 &a, uint8x32 b);

  //Set
  static void setzero(uint8x32 &a);

  //Shuffle
  static uint8x32 shuffle(uint8x32 a, uint8x32 mask);

  //Swizzle
  class uint16x16;
  static uint16x16 widen_hi(uint8x32 a, uint8x32 b);

  static uint16x16 widen_lo(uint8x32 a, uint8x32 b);
}
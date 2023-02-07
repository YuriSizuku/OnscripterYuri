/* -*- C++ -*-
*
*  int8x16.h
*
*  Copyright (C) 2015-2019 jh10001 <jh10001@live.cn>
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
  class uint8x16 {
#ifdef USE_SIMD_X86_SSE2
    __m128i v_;
#elif USE_SIMD_ARM_NEON
    uint8x16_t v_;
#endif
  public:
    uint8x16() = default;
    uint8x16(const uint8x16&) = default;
    uint8x16 &operator=(const uint8x16&) = default;
#ifdef USE_SIMD_X86_SSE2
    uint8x16(__m128i v) : v_(v) {}
    operator __m128i() const { return v_; }
    uint8x16(uint8_t i) : v_(_mm_set1_epi8(i)) {}
#elif USE_SIMD_ARM_NEON
    uint8x16(uint8x16_t v) : v_(v) {}
    operator uint8x16_t() const { return v_; }
    uint8x16(uint8_t i) : v_(vdupq_n_u8(i)) {}
#endif
    static uint8x16 set(uint8_t rm1, uint8_t rm2, uint8_t rm3, uint8_t rm4) {
#ifdef USE_SIMD_X86_SSE2
      return _mm_set_epi8(rm4, rm3, rm2, rm1, rm4, rm3, rm2, rm1, rm4, rm3, rm2, rm1, rm4, rm3, rm2, rm1);
#elif USE_SIMD_ARM_NEON
      union {
        uint32_t v32[1];
        uint8_t v[4];
      };
      v[0] = rm1; v[1] = rm2; v[2] = rm3; v[3] = rm4;
      return reinterpret_cast<uint8x16_t>(vld1q_dup_u32(v32));
#endif
    };
    static uint8x16 set(uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, uint8_t m5, uint8_t m6, uint8_t m7, uint8_t m8,
        uint8_t m9, uint8_t m10, uint8_t m11, uint8_t m12, uint8_t m13, uint8_t m14, uint8_t m15, uint8_t m16) {
  #ifdef USE_SIMD_X86_SSE2
          return _mm_set_epi8(m16, m15, m14, m13, m12, m11, m10, m9, m8, m7, m6, m5, m4, m3, m2, m1);
  #elif USE_SIMD_ARM_NEON
          return uint8x16_t{m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16};
  #endif
	  }
    static uint8x16 set4(uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4) {
  #ifdef USE_SIMD_X86_SSE2
      return _mm_set_epi8(m4, m4, m4, m4, m3, m3, m3, m3, m2, m2, m2, m2, m1, m1, m1, m1);
  #elif USE_SIMD_ARM_NEON
      return uint8x16_t{m1, m1, m1, m1, m2, m2, m2, m2, m3, m3, m3, m3, m4, m4, m4, m4};
  #endif
    }
  };

  //Arithmetic
  static uint8x16 operator+(uint8x16 a, uint8x16 b);

  static uint8x16 operator+=(uint8x16 &a, uint8x16 b);

  static uint8x16 adds(uint8x16 a, uint8x16 b);

  //Load
  static uint8x16 load_u(const void* m);

  //Logical
  static uint8x16 operator|(uint8x16 a, uint8x16 b);

  static uint8x16 operator|=(uint8x16 &a, uint8x16 b);

  //Set
  static void setzero(uint8x16 &a);

  //Store
  static void store_u(void* m, uint8x16 a);

  static void store_u_32(void* m, uint8x16 a);

  //Shuffle
  static uint8x16 shuffle(uint8x16 a, uint8x16 mask);

  //Swizzle
  class uint16x8;
  static uint16x8 widen_hi(uint8x16 a, uint8x16 b);

  static uint16x8 widen_lo(uint8x16 a, uint8x16 b);
}
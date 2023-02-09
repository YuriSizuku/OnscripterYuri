/* -*- C++ -*-
*
*  vec256.h
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
  class uint8x32;
  class uint16x16;
  class ivec256 {
#ifdef USE_SIMD_X86_AVX2
    __m256i v_;
#endif
  public:
    ivec256() = default;
    ivec256(const ivec256&) = default;
    ivec256 &operator=(const ivec256&) = default;
#ifdef USE_SIMD_X86_AVX2
    ivec256(__m256i v) : v_(v) {};
    operator __m256i() const { return v_; }
    operator uint8x32() const { return v_; }
    operator uint16x16() const { return v_; }
    ivec128 lo() { return _mm256_castsi256_si128(v_); }
    static ivec256 zero() { return _mm256_setzero_si256(); }
#endif
  };

  //Load
  static ivec256 load256_u(const void* m);

  //Store
  static void store256_u(void* m, __m256i a);
}

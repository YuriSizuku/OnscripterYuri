/* -*- C++ -*-
*
*  vec128.h
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
  class uint8x4;
  class uint8x8;
  class uint8x16;
  class uint16x4;
  class uint16x8;
  class ivec128 {
#ifdef USE_SIMD_X86_SSE2
    __m128i v_;
#endif
  public:
    ivec128() = default;
    ivec128(const ivec128&) = default;
    ivec128 &operator=(const ivec128&) = default;
#ifdef USE_SIMD_X86_SSE2
    ivec128(__m128i v) : v_(v) {};
    operator __m128i() const { return v_; }
    operator uint8x4() const { return v_; }
    operator uint8x8() const { return v_; }
    operator uint8x16() const { return v_; }
    operator uint16x4() const { return v_; }
    operator uint16x8() const { return v_; }
    static ivec128 zero() { return _mm_setzero_si128(); }
#elif USE_SIMD_ARM_NEON
    static ivec128 zero() { return ivec128(); }
    operator uint8x4() const { return uint8x4(); }
    operator uint8x8() const { return uint8x8(); }
    operator uint8x16() const { return uint8x16(); }
    operator uint16x4() const { return uint16x4(); }
    operator uint16x8() const { return uint16x8(); }
#endif
  };
}
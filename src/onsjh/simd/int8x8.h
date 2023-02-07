/* -*- C++ -*-
*
*  int8x8.h
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
  class uint8x8 {
#ifdef USE_SIMD_X86_SSE2
    __m128i v_;
#elif USE_SIMD_ARM_NEON
    uint8x8_t v_;
#endif
  public:
    uint8x8() = default;
    uint8x8(const uint8x8&) = default;
    uint8x8 &operator=(const uint8x8&) = default;
#ifdef USE_SIMD_X86_SSE2
    uint8x8(__m128i v) : v_(v) {};
    operator __m128i() const { return v_; }
#elif USE_SIMD_ARM_NEON
    uint8x8(uint8x8_t v) : v_(v) {};
    operator uint8x8_t() const { return v_; }
#endif
  };

  class uint16x8;
  static uint16x8 widen(uint8x8 a, uint8x8 b);
}
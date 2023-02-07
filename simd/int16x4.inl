/* -*- C++ -*-
*
*  int16x4.inl
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
#ifndef __SIMD_H__
#error "This file must be included through simd.h"
#endif
#include <stdint.h>
#include "int8x16.h"

namespace simd{
  //Arithmetic
  inline uint16x4 operator+(uint16x4 a, uint16x4 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_add_epi16(a, b); //PADDW xmm1, xmm2
#elif USE_SIMD_ARM_NEON
    return vadd_u16(a, b);
#endif
  }

  inline uint16x4 operator+=(uint16x4 &a, uint16x4 b) {
    return a = a + b;
  }

  inline uint16x4 operator-(uint16x4 a, uint16x4 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_sub_epi16(a, b); //PSUBW xmm1, xmm2
#elif USE_SIMD_ARM_NEON
    return vsub_u16(a, b);
#endif
  }

  inline uint16x4 operator-=(uint16x4 &a, uint16x4 b) {
    return a = a - b;
  }

  inline uint16x4 operator*(uint16x4 a, uint16x4 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_mullo_epi16(a, b); //PMULLW xmm1, xmm2
#elif USE_SIMD_ARM_NEON
    return vmul_u16(a, b);
#endif
  }

  inline uint16x4 operator*=(uint16x4 &a, uint16x4 b) {
    return a = a * b;
  }

  //Miscellaneous
  inline uint8x4 narrow_hz(uint16x4 a) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_packus_epi16(a, a);
#elif USE_SIMD_ARM_NEON
    uint8x8_t t = vreinterpret_u8_u16(static_cast<uint16x4_t>(a));
    return vuzp_u8(t, t).val[0];
#endif
  }

  //Shift
  template<unsigned imm8>
  inline uint16x4 shiftr(uint16x4 a) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_srli_epi16(a, imm8); //PSRLW xmm1, imm
#elif USE_SIMD_ARM_NEON
    return vshr_n_u16(a, imm8);
#endif
  }
  
  inline uint16x4 operator>>(uint16x4 a, immint<8> imm8) { return shiftr<8>(a); }
  inline uint16x4 operator>>=(uint16x4 &a, immint<8> imm8) { return a = a >> imm8; }
}
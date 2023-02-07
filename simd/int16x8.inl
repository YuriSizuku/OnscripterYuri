/* -*- C++ -*-
*
*  int16x8.inl
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

namespace simd {
  //Arithmetic
  inline uint16x8 operator-(uint16x8 a, uint16x8 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_sub_epi16(a, b); //PSUBW xmm1, xmm2
#elif USE_SIMD_ARM_NEON
    return vsubq_u16(a, b);
#endif
  }

  inline uint16x8 operator-=(uint16x8 &a, uint16x8 b) {
    return a = a - b;
  }

  inline uint16x8 operator*(uint16x8 a, uint16x8 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_mullo_epi16(a, b); //PMULLW xmm1, xmm2
#elif USE_SIMD_ARM_NEON
    return vmulq_u16(a, b);
#endif
  }

  inline uint16x8 operator*=(uint16x8 &a, uint16x8 b) {
    return a = a * b;
  }

  //Miscellaneous
  inline uint8x16 pack_hz(uint16x8 a, uint16x8 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_packus_epi16(a, b);
#elif USE_SIMD_ARM_NEON
    return vuzpq_u8(reinterpret_cast<uint8x16_t>(static_cast<uint16x8_t>(a)), reinterpret_cast<uint8x16_t>(static_cast<uint16x8_t>(b))).val[0];
#endif
  }

  //Set
  inline void setzero(uint16x8 &a) {
#ifdef USE_SIMD_X86_SSE2
    a = _mm_setzero_si128();  //PXOR xmm, xmm
#elif USE_SIMD_ARM_NEON
    a = veorq_u16(a, a);
#endif
  }

  //Shift
  template<unsigned imm8>
  inline uint16x8 shiftr(uint16x8 a) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_srli_epi16(a, imm8); //PSRLW xmm1, imm
#elif USE_SIMD_ARM_NEON
    return vshrq_n_u16(a, imm8);
#endif
  }
  
  inline uint16x8 operator>>(uint16x8 a, immint<8> imm8) { return shiftr<8>(a); }
  inline uint16x8 operator>>=(uint16x8 &a, immint<8> imm8) { return a = a >> imm8; }
}
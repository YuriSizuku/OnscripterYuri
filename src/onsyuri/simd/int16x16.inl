/* -*- C++ -*-
*
*  int16x16.inl
*
*  Copyright (C) 2018 jh10001 <jh10001@live.cn>
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
  inline uint16x16 operator-(uint16x16 a, uint16x16 b) {
#ifdef USE_SIMD_X86_AVX2
    return _mm256_sub_epi16(a, b);
#endif
  }

  inline uint16x16 operator-=(uint16x16 &a, uint16x16 b) {
    return a = a - b;
  }

  inline uint16x16 operator*(uint16x16 a, uint16x16 b) {
#ifdef USE_SIMD_X86_AVX2
    return _mm256_mullo_epi16(a, b);
#endif
  }

  inline uint16x16 operator*=(uint16x16 &a, uint16x16 b) {
    return a = a * b;
  }

  //Miscellaneous
  inline uint8x32 pack_hz(uint16x16 a, uint16x16 b) {
#ifdef USE_SIMD_X86_AVX2
    return _mm256_packus_epi16(a, b);
#endif
  }

  //Set
  inline void setzero(uint16x16 &a) {
#ifdef USE_SIMD_X86_AVX2
    a = _mm256_setzero_si256();
#endif
  }

  //Shift
  template<unsigned imm8>
  inline uint16x16 shiftr(uint16x16 a) {
#ifdef USE_SIMD_X86_AVX2
    return _mm256_srli_epi16(a, imm8);
#endif
  }
  
  inline uint16x16 operator>>(uint16x16 a, immint<8> imm8) { return shiftr<8>(a); }
  inline uint16x16 operator>>=(uint16x16 &a, immint<8> imm8) { return a = a >> imm8; }
}
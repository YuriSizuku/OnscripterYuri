/* -*- C++ -*-
*
*  int8x16.inl
*
*  Copyright (C) 2015-2016 jh10001 <jh10001@live.cn>
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
#include "int16x8.h"

namespace simd {
  //Arithmetic
  inline uint8x16 operator+(uint8x16 a, uint8x16 b){
#ifdef USE_SIMD_X86_SSE2
    return _mm_add_epi8(a, b);  //PADDB xmm1, xmm2
#elif USE_SIMD_ARM_NEON
    return vaddq_u8(a, b);
#endif
  }

  inline uint8x16 operator+=(uint8x16 &a, uint8x16 b){
    return a = a + b;
  }

  inline uint8x16 adds(uint8x16 a, uint8x16 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_adds_epu8(a, b);  //PADDUSB xmm1, xmm2
#elif USE_SIMD_ARM_NEON
    return vqaddq_u8(a, b);
#endif
  }

  //Load
  inline uint8x16 load_a(const void *m) {
#if USE_SIMD_X86_SSE2
    return _mm_load_si128(reinterpret_cast<const __m128i*>(m));  //MOVDQU xmm1, m128
#elif USE_SIMD_ARM_NEON
    return vld1q_u8(reinterpret_cast<const uint8_t*>(m));
#endif
  }

  inline uint8x16 load_u(const void *m) {
#if USE_SIMD_X86_SSE3
    return _mm_lddqu_si128(reinterpret_cast<const __m128i*>(m));  //LDDQU xmm1, m128
#elif USE_SIMD_X86_SSE2
    return _mm_loadu_si128(reinterpret_cast<const __m128i*>(m));  //MOVDQU xmm1, m128
#elif USE_SIMD_ARM_NEON
    return vld1q_u8(reinterpret_cast<const uint8_t*>(m));
#endif
  }

  //Logical
  inline uint8x16 operator|(uint8x16 a, uint8x16 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_or_si128(a, b);  //POR xmm1, xmm2
#elif USE_SIMD_ARM_NEON
    return vorrq_u8(a, b);
#endif
  }

  inline uint8x16 operator|=(uint8x16 &a, uint8x16 b) {
    return a = a | b;
  }

  //Set
  inline void setzero(uint8x16 &a) {
#ifdef USE_SIMD_X86_SSE2
    a = _mm_setzero_si128(); //PXOR xmm, xmm
#elif USE_SIMD_ARM_NEON
    a = veorq_u8(a, a);
#endif
  }

  //Store
  inline void store_a(void* m, uint8x16 a) {
#ifdef USE_SIMD_X86_SSE2
    _mm_store_si128(reinterpret_cast<__m128i*>(m), a);
#elif USE_SIMD_ARM_NEON
    vst1q_u8(reinterpret_cast<uint8_t*>(m), a);
#endif
  }

  inline void store_u(void* m, uint8x16 a) {
#ifdef USE_SIMD_X86_SSE2
    _mm_storeu_si128(reinterpret_cast<__m128i*>(m), a);
#elif USE_SIMD_ARM_NEON
    vst1q_u8(reinterpret_cast<uint8_t*>(m), a);
#endif
  }

  inline void store_u_32(void* m, uint8x16 a) {
#ifdef USE_SIMD_X86_SSE2
#if !defined(__clang_major__) || __clang_major__ >= 8
    _mm_storeu_si32(reinterpret_cast<__m128i*>(m), a);
#else
    // _mm_storeu_si32 is unavailable in Clang 7;
    _mm_store_ss(reinterpret_cast<float*>(m), _mm_castsi128_ps(a));
#endif
#elif USE_SIMD_ARM_NEON
    vst1q_lane_u32(reinterpret_cast<uint32_t*>(m), a, 1);
#endif
  }

  //Shuffle
  inline uint8x16 shuffle(uint8x16 a, uint8x16 mask) {
#ifdef USE_SIMD_X86_SSSE3
    return _mm_shuffle_epi8(a, mask);
#endif
  }

  //Swizzle
  inline uint16x8 widen_hi(uint8x16 a, uint8x16 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_unpackhi_epi8(a, b);
#elif USE_SIMD_ARM_NEON
    return vmovl_u8(vget_high_u8(a));
#endif
  }

  inline uint16x8 widen_lo(uint8x16 a, uint8x16 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_unpacklo_epi8(a, b);
#elif USE_SIMD_ARM_NEON
    return vmovl_u8(vget_low_u8(a));
#endif
  }
}
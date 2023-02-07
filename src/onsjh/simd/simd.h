/* -*- C++ -*-
*
*  simd.h
*
*  Copyright (C) 2015-2018 jh10001 <jh10001@live.cn>
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
#define __SIMD_H__

#ifdef USE_SIMD_X86_AVX2
#include <immintrin.h>
#define USE_SIMD_X86_SSSE3 1
#endif

#ifdef USE_SIMD_X86_SSSE3
#include <tmmintrin.h>
#define USE_SIMD_X86_SSE3 1
#endif

#ifdef USE_SIMD_X86_SSE3
#include <pmmintrin.h>
#define USE_SIMD_X86_SSE2 1
#endif

#ifdef USE_SIMD_X86_SSE2
#include <emmintrin.h>
#define USE_SIMD_X86_SSE 1
#endif

#ifdef USE_SIMD_X86_SSE
#include <xmmintrin.h>
#endif

#ifdef USE_SIMD_ARM_NEON
#include <arm_neon.h>
#endif

#ifdef __GNUC__
#define inline static __inline __attribute__((__always_inline__))
#elif defined(_MSC_VER)
#define inline __forceinline
#endif

namespace simd {
  template<unsigned imm>
  struct immint {
#if _MSC_VER < 1900
    static const int value = imm;
    operator int() const { return value; }
#else
    static constexpr int value = imm;
    constexpr operator int() const { return value; }
#endif
  };
}

#if defined(USE_SIMD_X86_SSE2) || defined(USE_SIMD_ARM_NEON)
#include "int8x4.h"
#include "int8x4.inl"
#include "int8x8.h"
#include "int8x8.inl"
#include "int8x16.h"
#include "int8x16.inl"
#include "int16x4.h"
#include "int16x4.inl"
#include "int16x8.h"
#include "int16x8.inl"
#include "int32x2.h"
#include "int32x2.inl"
#include "int32x4.h"
#include "int32x4.inl"
#include "vec128.h"
#include "vec128.inl"
#ifdef USE_SIMD_X86_AVX2
#include "int8x32.h"
#include "int8x32.inl"
#include "int16x16.h"
#include "int16x16.inl"
#include "vec256.h"
#include "vec256.inl"
#endif
#endif

#undef inline

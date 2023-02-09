/* -*- C++ -*-
*
*  int8x8.inl
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
#include "int16x8.h"

namespace simd {
  inline uint16x8 widen(uint8x8 a, uint8x8 b) {
#ifdef USE_SIMD_X86_SSE2
    return _mm_unpacklo_epi8(a, b);
#elif USE_SIMD_ARM_NEON
    return vmovl_u8(a);
#endif
  }
}

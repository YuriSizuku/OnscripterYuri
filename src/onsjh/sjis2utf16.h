/* -*- C++ -*-
*
*  sjis2utf16.h
*
*  Copyright (C) 2014-2016 jh10001 <jh10001@live.cn>
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

#ifndef __SJIS2UTF16_H__
#define __SJIS2UTF16_H__

#include "coding2utf16.h"

class SJIS2UTF16 : public Coding2UTF16 {
public:
  void init() final;
  uint16_t conv2UTF16(uint16_t) const final;
  uint16_t convUTF162Coding(uint16_t) const final;
};

#endif

/* -*- C++ -*-
*
*  coding2utf16.h
*
*  Copyright (C) 2014-2015 jh10001 <jh10001@live.cn>
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

#ifndef __CODING2UTF16_H__
#define __CODING2UTF16_H__

#include <stdint.h>

class Coding2UTF16{
public:
  static char space[4];
  static char minus[4];
  static char bracket[8];
  static char num_str[24];
  static char DEFAULT_START_KINSOKU[40];
  static char DEFAULT_END_KINSOKU[12];
  static char DEFAULT_SAVE_MENU_NAME[12];
  static char DEFAULT_LOAD_MENU_NAME[12];
  static char DEFAULT_SAVE_ITEM_NAME[8];
  static char MESSAGE_SAVE_EXIST[24];
  static char MESSAGE_SAVE_EMPTY[32];
  static char MESSAGE_SAVE_CONFIRM[40];
  static char MESSAGE_LOAD_CONFIRM[40];
  static char MESSAGE_RESET_CONFIRM[36];
  static char MESSAGE_END_CONFIRM[32];
  static char MESSAGE_YES[8];
  static char MESSAGE_NO[8];
  static char MESSAGE_OK[8];
  static char MESSAGE_CANCEL[12];
  virtual void init() = 0;
  virtual uint16_t conv2UTF16(uint16_t) const = 0;
  virtual uint16_t convUTF162Coding(uint16_t) const = 0;
  int convUTF16ToUTF8(unsigned char*,uint16_t) const;
  unsigned short convUTF8ToUTF16(const char**);
  virtual ~Coding2UTF16() {};
};

#endif

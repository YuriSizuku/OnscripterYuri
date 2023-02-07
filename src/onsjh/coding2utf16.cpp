/* -*- C++ -*-
*
*  coding2utf16.cpp
*
*  Copyright (C) 2014 jh10001 <jh10001@live.cn>
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

#include "coding2utf16.h"

char Coding2UTF16::space[4];
char Coding2UTF16::minus[4];
char Coding2UTF16::bracket[8];
char Coding2UTF16::num_str[24];
char Coding2UTF16::DEFAULT_START_KINSOKU[40];
char Coding2UTF16::DEFAULT_END_KINSOKU[12];
char Coding2UTF16::DEFAULT_SAVE_MENU_NAME[12];
char Coding2UTF16::DEFAULT_LOAD_MENU_NAME[12];
char Coding2UTF16::DEFAULT_SAVE_ITEM_NAME[8];
char Coding2UTF16::MESSAGE_SAVE_EXIST[24];
char Coding2UTF16::MESSAGE_SAVE_EMPTY[32];
char Coding2UTF16::MESSAGE_SAVE_CONFIRM[40];
char Coding2UTF16::MESSAGE_LOAD_CONFIRM[40];
char Coding2UTF16::MESSAGE_RESET_CONFIRM[36];
char Coding2UTF16::MESSAGE_END_CONFIRM[32];
char Coding2UTF16::MESSAGE_YES[8];
char Coding2UTF16::MESSAGE_NO[8];
char Coding2UTF16::MESSAGE_OK[8];
char Coding2UTF16::MESSAGE_CANCEL[12];

int Coding2UTF16::convUTF16ToUTF8(unsigned char dst[4], uint16_t src) const {
    if (src & 0xff80) {
        if (src & 0xf800) {
            // UCS-2 = U+0800 - U+FFFF -> UTF-8 (3 bytes)
            dst[0] = 0xe0 | (src >> 12);
            dst[1] = 0x80 | ((src >> 6) & 0x3f);
            dst[2] = 0x80 | (src & 0x3f);
            dst[3] = 0;

            return 3;
        }

        // UCS-2 = U+0080 - U+07FF -> UTF-8 (2 bytes)
        dst[0] = 0xc0 | (src >> 6);
        dst[1] = 0x80 | (src & 0x3f);
        dst[2] = 0;

        return 2;
    }

    // UCS-2 = U+0000 - U+007F -> UTF-8 (1 byte)
    dst[0] = src;
    dst[1] = 0;

    return 1;
}

unsigned short Coding2UTF16::convUTF8ToUTF16(const char **src) {
    unsigned short utf16=0;

    if (**src & 0x80) {
        if (**src & 0x20) {
            utf16 |= ((unsigned short)((*(*src)++)&0x0f)) << 12;
            utf16 |= ((unsigned short)((*(*src)++)&0x3f)) << 6;
            utf16 |= ((unsigned short)((*(*src)++)&0x3f));
        } else {
            utf16 |= ((unsigned short)((*(*src)++)&0x1f)) << 6;
            utf16 |=  (unsigned short)((*(*src)++)&0x3f);
        }
    } else {
        utf16 |= (unsigned short)(*(*src)++);
    }

    return utf16;
}

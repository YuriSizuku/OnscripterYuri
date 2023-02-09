/* -*- C++ -*-
 *
 *  LUAHandler.h - LUA handler for ONScripter
 *
 *  Copyright (c) 2001-2015 Ogapee. All rights reserved.
 *
 *  ogapee@aqua.dti2.ne.jp
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

#if !defined(__LUA_HANDLER_H__) && defined(USE_LUA)
#define __LUA_HANDLER_H__

#include <lua.hpp>

class ONScripter;
class ScriptHandler;

class LUAHandler{
public:
    enum { LUA_TAG,
           LUA_TEXT0,
           LUA_TEXT,
           LUA_ANIMATION,
           LUA_CLOSE,
           LUA_END,
           LUA_SAVEPOINT,
           LUA_SAVE,
           LUA_LOAD,
           LUA_RESET,
           MAX_CALLBACK
    };
           
    LUAHandler();
    ~LUAHandler();

    void init(ONScripter *ons, ScriptHandler *sh, 
              int screen_ratio1, int screen_ratio2);
    void loadInitScript();
    void addCallback(const char *label);

    int  callFunction(bool is_callback, const char *cmd, void *data=NULL);

    bool isCallbackEnabled(int val);

    bool is_animatable;
    int duration_time;
    int next_time;
    
    //private:
    ONScripter *ons;
    lua_State *state;
    ScriptHandler *sh;
    int screen_ratio1, screen_ratio2;

    char error_str[256];
    
    bool callback_state[MAX_CALLBACK];
};

#endif // __LUA_HANDLER_H__

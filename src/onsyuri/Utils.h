/* -*- C++ -*-
 *
 *  Utils.h
 *
 *  Copyright (c) 2014 jh10001 <jh10001@live.cn>
 *            (c) 2022-2023 yurisizuku <https://github.com/YuriSizuku>
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

#ifndef __UTILS_H__
#define __UTILS_H__

#if defined(__LIBRETRO__)
#undef ANDROID
#endif

#ifdef ANDROID
#include <android/log.h>
#elif defined(WINRT)
#include "debugapi.h"
#include "windows.h"
static BOOL MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize)
{
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, NULL, 0);
	if (dwSize < dwMinSize)
	{
		return FALSE;
	}
	MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);
	return TRUE;
}
#endif
#include <stdio.h>
#include <stdarg.h>
#include <utility>
#include <string>

extern std::string g_stdoutpath;
extern std::string g_stderrpath;

namespace utils{
	inline void printInfo(const char *format, ...){
		va_list ap;
		va_start(ap, format);
#ifdef USE_FILELOG
		static int isfirst = 1;
		FILE *fp = NULL;
		if(isfirst) {
			fp = fopen(g_stdoutpath.c_str(), "w");
			isfirst = 0;
		}
		else {
			fp = fopen(g_stdoutpath.c_str(), "a");
		}		
		if(fp) {
			va_list ap2;
			va_copy(ap2, ap);
			vfprintf(fp, format, ap2);
			va_end(ap2);
			fclose(fp);
		}
#endif
#ifdef ANDROID
		__android_log_vprint(ANDROID_LOG_INFO, "## onsyuri", format, ap);
#elif defined(WINRT)
		char *buf = new char[256];
		vsprintf(buf, format, ap);
		LPWSTR wstr = new WCHAR[128];
		MByteToWChar(buf, wstr, 256);
		OutputDebugString(wstr);
#else
		vfprintf(stdout, format, ap);
#endif
		va_end(ap);
	}

	inline void printError(const char *format, ...){
		va_list ap;
		va_start(ap, format);
#ifdef USE_FILELOG
		static int isfirst = 1;
		FILE *fp = NULL;
		if(isfirst) {
			fp = fopen(g_stderrpath.c_str(), "w");
			isfirst = 0;
		}
		else {
			fp = fopen(g_stderrpath.c_str(), "a");
		}		
		if(fp) {
			va_list ap2;
			va_copy(ap2, ap);
			vfprintf(fp, format, ap2);
			va_end(ap2);
			fclose(fp);
		}
#endif
#ifdef ANDROID
		__android_log_vprint(ANDROID_LOG_ERROR, "## onsyuri", format, ap);
#elif defined(WINRT)
		char *buf = new char[256];
		vsprintf(buf,format,ap);
		LPWSTR wstr = new WCHAR[128];
		MByteToWChar(buf, wstr, 256);
		OutputDebugString(wstr);
#else
		vfprintf(stderr, format, ap);
#endif
		va_end(ap);
	}

	template<class T> T min(T a, T b){
        return a < b ? a : b;
    }

	template<class T> T clamp(T x, T min, T max){
        return x < min ? min : (x > max ? max : x);
    }

	template <typename From>
	class auto_cast {
	public:
		explicit constexpr auto_cast(From const& t) noexcept
				: val { t }
		{}

		template <typename To>
		constexpr operator To() const noexcept(noexcept(static_cast<To>(std::declval<From>()))) {
			return static_cast<To>(val);
		}

	private:
		From const& val;
	};
}

#endif

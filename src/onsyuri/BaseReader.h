/* -*- C++ -*-
 *
 *  BaseReader.h - Base class of archive reader
 *
 *  Copyright (c) 2001-2018 Ogapee. All rights reserved.
 *            (C) 2014-2018 jh10001 <jh10001@live.cn>
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

#ifndef __BASE_READER_H__
#define __BASE_READER_H__

#include <stdio.h>
#ifdef ANDROID
extern "C" FILE *fopen_ons(const char *str, const char *mode);
#define fopen fopen_ons
extern "C" int mkdir_ons(const char *pathname, mode_t mode);
#define mkdir mkdir_ons
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif

#if defined(LINUX) || defined(MACOSX)
#define DELIMITER '/'
#elif defined(WIN32) || defined(_WIN32)
#define DELIMITER '\\'
#elif defined(MACOS9)
#define DELIMITER ':'
#define RELATIVEPATH ":"
#define RELATIVEPATHLENGTH 1
#else
#define DELIMITER '/'
#endif
#ifndef RELATIVEPATH
#define RELATIVEPATH ""
#define RELATIVEPATHLENGTH 0
#endif

struct BaseReader
{
    enum {
        NO_COMPRESSION   = 0,
        SPB_COMPRESSION  = 1,
        LZSS_COMPRESSION = 2,
        NBZ_COMPRESSION  = 4
    };
    
    enum {
        ARCHIVE_TYPE_NONE = 0,
        ARCHIVE_TYPE_SAR  = 1,
        ARCHIVE_TYPE_NSA  = 2,
        ARCHIVE_TYPE_NS2  = 4   //new format since NScr2.91, uses ext ".ns2"
    };

    struct FileInfo{
        char name[256];
        int  compression_type;
        size_t offset;
        size_t length;
        size_t original_length;
    };

    struct ArchiveInfo{
        ArchiveInfo *next;
        FILE *file_handle;
        int power_resume_number; // currently only for PSP
        char *file_name;
        FileInfo *fi_list;
        unsigned int num_of_files;
        unsigned long base_offset;

        ArchiveInfo(){
            next = NULL;
            file_handle = NULL;
            power_resume_number = 0;
            file_name = NULL;
            fi_list = NULL;
            num_of_files = 0;
        }
        ~ArchiveInfo(){
            if (file_handle) fclose( file_handle );
            if (file_name)   delete[] file_name;
            if (fi_list)     delete[] fi_list;
        }
    };

    virtual ~BaseReader(){};
    
    virtual int open( const char *name=NULL ) = 0;
    virtual int close() = 0;
    
    virtual const char *getArchiveName() const = 0;
    virtual int  getNumFiles() = 0;
    virtual void registerCompressionType( const char *ext, int type ) = 0;

    //virtual FileInfo getFileByIndex( unsigned int index ) = 0;
    virtual size_t getFileLength( const char *file_name ) = 0;
    virtual size_t getFile( const char *file_name, unsigned char *buffer, int *location=NULL ) = 0;
};

#endif // __BASE_READER_H__

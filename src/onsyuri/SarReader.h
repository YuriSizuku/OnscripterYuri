/* -*- C++ -*-
 *
 *  SarReader.h - Reader from a SAR archive
 *
 *  Copyright (c) 2001-2014 Ogapee. All rights reserved.
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

#ifndef __SAR_READER_H__
#define __SAR_READER_H__

#include "DirectReader.h"

class SarReader : public DirectReader
{
public:
    SarReader( const char *path=NULL, const unsigned char *key_table=NULL );
    ~SarReader();

    int open( const char *name=NULL );
    int close();
    const char *getArchiveName() const;
    int getNumFiles();
    
    size_t getFileLength( const char *file_name );
    size_t getFile( const char *file_name, unsigned char *buf, int *location=NULL );
    FileInfo getFileByIndex( unsigned int index );

    int writeHeader( FILE *fp );
    size_t putFile( FILE *fp, int no, size_t offset, size_t length, size_t original_length, bool modified_flag, unsigned char *buffer );
    
protected:
    ArchiveInfo archive_info;
    ArchiveInfo *root_archive_info, *last_archive_info;
    int num_of_sar_archives;

    void readArchive( ArchiveInfo *ai, int archive_type = ARCHIVE_TYPE_SAR, unsigned int offset=0 );
    int readArchiveSub( ArchiveInfo *ai, int archive_type = ARCHIVE_TYPE_SAR, bool check_size = true );
    int getIndexFromFile( ArchiveInfo *ai, const char *file_name );
    size_t getFileSub( ArchiveInfo *ai, const char *file_name, unsigned char *buf );

    int writeHeaderSub( ArchiveInfo *ai, FILE *fp, int archive_type = ARCHIVE_TYPE_SAR, int nsa_offset=0 );
    size_t putFileSub( ArchiveInfo *ai, FILE *fp, int no, size_t offset, size_t length, size_t original_length, int compression_type, bool modified_flag, unsigned char *buffer );
};

#endif // __SAR_READER_H__

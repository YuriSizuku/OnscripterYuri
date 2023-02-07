/* -*- C++ -*-
 *
 *  NsaReader.cpp - Reader from a NSA archive
 *
 *  Copyright (c) 2001-2014 Ogapee. All rights reserved.
 *            (C) 2014 jh10001 <jh10001@live.cn>
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

#include "NsaReader.h"
#include "Utils.h"
#include <string.h>
#define NSA_ARCHIVE_NAME "arc"
#define NSA_ARCHIVE_NAME2 "arc%d"

NsaReader::NsaReader( unsigned int nsa_offset, char *path, int archive_type, const unsigned char *key_table )
        :SarReader( path, key_table )
{
    sar_flag = true;
    this->nsa_offset = nsa_offset;
    this->archive_type = archive_type;
    num_of_nsa_archives = 0;
    num_of_ns2_archives = 0;

    if (key_table)
        nsa_archive_ext = "___";
    else
        nsa_archive_ext = "nsa";

    ns2_archive_ext = "ns2";
}

NsaReader::~NsaReader()
{
}

int NsaReader::open( const char *nsa_path )
{
    int i;
    bool archive_found = false;
    char archive_name[256], archive_name2[256];

    if ( !SarReader::open( "arc.sar" ) ) return 0;
    
    sar_flag = false;

    if (archive_type & ARCHIVE_TYPE_NS2){
        for ( i=0 ; i<MAX_NS2_ARCHIVE ; i++ ){
            sprintf( archive_name, "%s%02d.%s", nsa_path?nsa_path:"", i, ns2_archive_ext );
            if ( ( archive_info_ns2[i].file_handle = fopen( archive_name, "rb" ) ) == NULL ) break;
        
            archive_found = true;
            archive_info_ns2[i].file_name = new char[strlen(archive_name)+1];
            memcpy(archive_info_ns2[i].file_name, archive_name, strlen(archive_name)+1);
            readArchive( &archive_info_ns2[i], ARCHIVE_TYPE_NS2, nsa_offset );
            num_of_ns2_archives = i+1;
        }
    }

    if (num_of_ns2_archives ==0 && archive_type & ARCHIVE_TYPE_NSA){
        for ( i=-1 ; i<MAX_EXTRA_ARCHIVE ; i++ ){
            ArchiveInfo *ai;
        
            if (i == -1){
                sprintf( archive_name, "%s%s.%s", nsa_path?nsa_path:"", NSA_ARCHIVE_NAME, nsa_archive_ext );
                ai = &archive_info;
            }
            else{
                sprintf( archive_name2, NSA_ARCHIVE_NAME2, i+1 );
                sprintf( archive_name, "%s%s.%s", nsa_path?nsa_path:"", archive_name2, nsa_archive_ext );
                ai = &archive_info2[i];
            }
        
            if ( ( ai->file_handle = fopen( archive_name, "rb" ) ) == NULL ) break;

            archive_found = true;
            ai->file_name = new char[strlen(archive_name)+1];
            memcpy(ai->file_name, archive_name, strlen(archive_name)+1);
            readArchive( ai, ARCHIVE_TYPE_NSA, nsa_offset );
            num_of_nsa_archives = i+1;
        }
    }

    if (!archive_found) return -1;

    return 0;
}

int NsaReader::openForConvert( char *nsa_name, int archive_type, unsigned int nsa_offset )
{
    sar_flag = false;
    if ( ( archive_info.file_handle = ::fopen( nsa_name, "rb" ) ) == NULL ){
        utils::printError( "can't open file %s\n", nsa_name );
        return -1;
    }

    readArchive( &archive_info, archive_type, nsa_offset );

    return 0;
}

int NsaReader::writeHeader( FILE *fp, int archive_type, int nsa_offset )
{
    ArchiveInfo *ai = &archive_info;
    return writeHeaderSub( ai, fp, archive_type, nsa_offset );
}

size_t NsaReader::putFile( FILE *fp, int no, size_t offset, size_t length, size_t original_length, int compression_type, bool modified_flag, unsigned char *buffer )
{
    ArchiveInfo *ai = &archive_info;
    return putFileSub( ai, fp, no, offset, length, original_length , compression_type, modified_flag, buffer );
}

const char *NsaReader::getArchiveName() const
{
    return "nsa";
}

int NsaReader::getNumFiles(){
    int total = archive_info.num_of_files, i;

    for ( i=0 ; i<num_of_nsa_archives ; i++ ) total += archive_info2[i].num_of_files;
    for ( i=0 ; i<num_of_ns2_archives ; i++ ) total += archive_info_ns2[i].num_of_files;
    
    return total;
}

size_t NsaReader::getFileLengthSub( ArchiveInfo *ai, const char *file_name )
{
    unsigned int i = getIndexFromFile( ai, file_name );

    if ( i == ai->num_of_files ) return 0;

    if ( ai->fi_list[i].original_length != 0 )
        return ai->fi_list[i].original_length;

    int type = ai->fi_list[i].compression_type;
    if ( type == NO_COMPRESSION )
        type = getRegisteredCompressionType( file_name );
    if ( type == NBZ_COMPRESSION || type == SPB_COMPRESSION ) {
        ai->fi_list[i].original_length = getDecompressedFileLength( type, ai->file_handle, ai->fi_list[i].offset );
    }
    
    return ai->fi_list[i].original_length;
}

size_t NsaReader::getFileLength( const char *file_name )
{
    if ( sar_flag ) return SarReader::getFileLength( file_name );

    size_t ret;
    int i;
    
    if ( ( ret = DirectReader::getFileLength( file_name ) ) ) return ret;
    
    for ( i=0 ; i<num_of_ns2_archives ; i++ ){
        if ( (ret = getFileLengthSub( &archive_info_ns2[i], file_name )) ) return ret;
    }
    
    if ( ( ret = getFileLengthSub( &archive_info, file_name )) ) return ret;

    for ( i=0 ; i<num_of_nsa_archives ; i++ ){
        if ( (ret = getFileLengthSub( &archive_info2[i], file_name )) ) return ret;
    }

    return 0;
}

size_t NsaReader::getFile( const char *file_name, unsigned char *buffer, int *location )
{
    size_t ret;

    if ( sar_flag ) return SarReader::getFile( file_name, buffer, location );

    if ( ( ret = DirectReader::getFile( file_name, buffer, location ) ) ) return ret;

    for ( int i=0 ; i<num_of_ns2_archives ; i++ ){
        if ( (ret = getFileSub( &archive_info_ns2[i], file_name, buffer )) ){
            if ( location ) *location = ARCHIVE_TYPE_NS2;
            return ret;
        }
    }

    if ( (ret = getFileSub( &archive_info, file_name, buffer )) ){
        if ( location ) *location = ARCHIVE_TYPE_NSA;
        return ret;
    }

    for ( int i=0 ; i<num_of_nsa_archives ; i++ ){
        if ( (ret = getFileSub( &archive_info2[i], file_name, buffer )) ){
            if ( location ) *location = ARCHIVE_TYPE_NSA;
            return ret;
        }
    }

    return 0;
}

NsaReader::FileInfo NsaReader::getFileByIndex( unsigned int index )
{
    int i;
    
    for ( i=0 ; i<num_of_ns2_archives ; i++ ){
        if ( index < archive_info_ns2[i].num_of_files ) return archive_info_ns2[i].fi_list[index];
        index -= archive_info_ns2[i].num_of_files;
    }

    if ( index < archive_info.num_of_files ) return archive_info.fi_list[index];
    index -= archive_info.num_of_files;

    for ( i=0 ; i<num_of_nsa_archives ; i++ ){
        if ( index < archive_info2[i].num_of_files ) return archive_info2[i].fi_list[index];
        index -= archive_info2[i].num_of_files;
    }
    utils::printError("NsaReader::getFileByIndex  Index %d is out of range\n", index );

    return archive_info.fi_list[0];
}

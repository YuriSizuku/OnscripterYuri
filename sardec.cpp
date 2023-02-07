/* -*- C++ -*-
 * 
 *  sardec.cpp - SAR archive decoder
 *
 *  Copyright (c) 2001-2004 Ogapee. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "SarReader.h"
#ifdef _WIN32
#include <direct.h>
inline int mkdir(const char *pathname, int unused){
  return _mkdir(pathname);
}
#endif

extern int errno;

int main( int argc, char **argv )
{
    SarReader cSR;
    unsigned long length, buffer_length = 0;
    unsigned char *buffer = NULL;
    char file_name[256], dir_name[256];
    unsigned int i, j, count;
    FILE *fp;
    struct stat file_stat;

    if ( argc != 2 ){
        fprintf( stderr, "Usage: sardec arc_file\n");
        exit(-1);
    }
    if (cSR.open( argv[1] ) != 0){
        fprintf( stderr, "can't open file %s\n", argv[1] );
        exit(-1);
    }
    count = cSR.getNumFiles();

    SarReader::FileInfo sFI;
    
    for ( i=0 ; i<count ; i++ ){
        sFI = cSR.getFileByIndex( i );
        
        length = cSR.getFileLength( sFI.name );

        if ( length > buffer_length ){
            if ( buffer ) delete[] buffer;
            buffer = new unsigned char[length];
            buffer_length = length;
        }
        if ( cSR.getFile( sFI.name, buffer ) != length ){
            fprintf( stderr, "file %s can't be retrieved\n", sFI.name );
            continue;
        }
        
        strcpy( file_name, sFI.name );
        for ( j=0 ; j<strlen(file_name) ; j++ ){
            if ( file_name[j] == '\\' ){
                file_name[j] = '/';
                strncpy( dir_name, file_name, j );
                dir_name[j] = '\0';

                /* If the directory does'nt exist, create it */
                if ( stat ( dir_name, &file_stat ) == -1 && errno == ENOENT )
                    mkdir( dir_name, 00755 );
            }
        }
    
        printf("opening %s\n", file_name );
        if ( (fp = fopen( file_name, "wb" ) )){
            fwrite( buffer, 1, length, fp );
            fclose(fp);
        }
        else{
            printf(" ... falied\n");
        }
    }
    
    if ( buffer ) delete[] buffer;
    
    exit(0);
}

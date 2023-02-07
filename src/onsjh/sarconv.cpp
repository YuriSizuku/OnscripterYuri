/* -*- C++ -*-
 * 
 *  sarconv.cpp - Images in SAR archive are re-scaled to 320x240 size
 *
 *  Copyright (c) 2001-2006 Ogapee. All rights reserved.
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
#include "SarReader.h"

extern int scale_ratio_upper;
extern int scale_ratio_lower;

extern size_t rescaleJPEG( unsigned char *original_buffer, size_t length, unsigned char **rescaled_buffer,
                           int quality );
extern size_t rescaleBMP( unsigned char *original_buffer, unsigned char **rescaled_buffer,
                          bool output_jpeg_flag, int quality );

#ifdef main
#undef main
#endif

void help()
{
    fprintf(stderr, "Usage: sarconv [-j] [-q quality] src_width dst_width src_archive_file dst_archive_file\n");
    fprintf(stderr, "           quality   ... 0 to 100\n");
    fprintf(stderr, "           src_width ... 640 or 800\n");
    fprintf(stderr, "           dst_width ... 176, 220, 320, 360, 384, 640, etc.\n");
    exit(-1);
}

int main( int argc, char **argv )
{
    SarReader cSR;
    unsigned long length, offset = 0, buffer_length = 0;
    unsigned char *buffer = NULL, *rescaled_buffer = NULL;
    unsigned int i, count;
    bool bmp2jpeg_flag = false;
    int quality = 75;
    FILE *fp;

    argc--; // skip command name
    argv++;
    while (argc > 4){
        if      ( !strcmp( argv[0], "-j" ) )    bmp2jpeg_flag = true;
        else if ( !strcmp( argv[0], "-q" ) ){
            argc--;
            argv++;
            quality = atoi(argv[0]);
        }
        argc--;
        argv++;
    }
    if (argc != 4) help();

    scale_ratio_lower = atoi(argv[0]); // src width
    if (scale_ratio_lower!=640 && scale_ratio_lower!=800) help();
    
    scale_ratio_upper = atoi(argv[1]); // dst width
    
    if ( (fp = fopen( argv[3], "wb" ) ) == NULL ){
        fprintf( stderr, "can't open file %s for writing.\n", argv[3] );
        exit(-1);
    }
    if (cSR.open( argv[2] ) != 0){
        fprintf( stderr, "can't open file %s\n", argv[2] );
        exit(-1);
    }
    count = cSR.getNumFiles();

    SarReader::FileInfo sFI;
    
    for ( i=0 ; i<count ; i++ ){
        printf( "%d/%d\n", i, count );
        sFI = cSR.getFileByIndex( i );
        if ( i==0 ) offset = sFI.offset;
        length = cSR.getFileLength( sFI.name );
        if ( length > buffer_length ){
            if ( buffer ) delete[] buffer;
            buffer = new unsigned char[length];
            buffer_length = length;
        }

        sFI.offset = offset;
        if ( (strlen( sFI.name ) > 3 && !strcmp( sFI.name + strlen( sFI.name ) - 3, "JPG")) ||
             (strlen( sFI.name ) > 4 && !strcmp( sFI.name + strlen( sFI.name ) - 4, "JPEG")) ){
            if ( cSR.getFile( sFI.name, buffer ) != length ){
                fprintf( stderr, "file %s can't be retrieved %ld\n", sFI.name, length );
                continue;
            }
            sFI.length = rescaleJPEG( buffer, length, &rescaled_buffer, quality );
            cSR.putFile( fp, i, sFI.offset, sFI.length, sFI.length, true, rescaled_buffer );
        }
        else if ( strlen( sFI.name ) > 3 && !strcmp( sFI.name + strlen( sFI.name ) - 3, "BMP") ){
            if ( cSR.getFile( sFI.name, buffer ) != length ){
                fprintf( stderr, "file %s can't be retrieved %ld\n", sFI.name, length );
                continue;
            }
            sFI.length = rescaleBMP( buffer, &rescaled_buffer, bmp2jpeg_flag, quality );
            cSR.putFile( fp, i, sFI.offset, sFI.length, sFI.length, true, rescaled_buffer );
        }
        else{
            cSR.putFile( fp, i, sFI.offset, sFI.length, sFI.original_length, false, buffer );
        }
        
        offset += sFI.length;
    }
    cSR.writeHeader( fp );

    fclose(fp);

    if ( rescaled_buffer ) delete[] rescaled_buffer;
    if ( buffer ) delete[] buffer;
    
    return 0;
}

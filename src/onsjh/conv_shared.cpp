/* -*- C++ -*-
 * 
 *  conv_shared.cpp - Shared code of sarconv and nsaconv
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
extern "C"{
#include <jpeglib.h>
};
#include <bzlib.h>
#include "resize_image.h"

int scale_ratio_upper;
int scale_ratio_lower;

unsigned char *rescaled_tmp2_buffer = NULL;
size_t rescaled_tmp2_length = 0;
unsigned char *rescaled_tmp_buffer = NULL;
size_t rescaled_tmp_length = 0;

static unsigned char *restored_buffer = NULL;
static size_t restored_length = 0;

#define INPUT_BUFFER_SIZE       4096
typedef struct {
    struct jpeg_source_mgr pub;

    unsigned char *buf;
    size_t left;
} my_source_mgr;

typedef struct {
    struct jpeg_destination_mgr pub;

    unsigned char *buf;
    size_t left;
} my_destination_mgr;


void rescaleImage( unsigned char *original_buffer, int width, int height, int byte_per_pixel,
                   bool src_pad_flag, bool dst_pad_flag, bool palette_flag )
{
    size_t width_pad = 0;
    if ( src_pad_flag ) width_pad = (4 - width * byte_per_pixel % 4) % 4;
    
    size_t w = (int)(width  * scale_ratio_upper / scale_ratio_lower);
    size_t h = (int)(height * scale_ratio_upper / scale_ratio_lower);
    if ( w==0 ) w=1;
    if ( h==0 ) h=1;
    size_t w_pad = 0;
    if ( dst_pad_flag ) w_pad = (4 - w * byte_per_pixel % 4) % 4;

    if  ( (w * byte_per_pixel + w_pad) * h > rescaled_tmp_length ){
        int len = (w * byte_per_pixel + w_pad) * h;
        if ( rescaled_tmp_buffer ) delete[] rescaled_tmp_buffer;
        rescaled_tmp_buffer = new unsigned char[ len ];
        rescaled_tmp_length = len;
    }

    size_t len = (width * byte_per_pixel + width_pad) * (height+1) + byte_per_pixel;
    if ( len<16 ) len = 16;
    if ( len > rescaled_tmp2_length ){
        if ( rescaled_tmp2_buffer ) delete[] rescaled_tmp2_buffer;
        rescaled_tmp2_buffer = new unsigned char[ len ];
        rescaled_tmp2_length = len;
    }

    resizeImage( rescaled_tmp_buffer, w, h, w*byte_per_pixel+w_pad,
                 original_buffer, width, height, width*byte_per_pixel+width_pad,
                 byte_per_pixel, rescaled_tmp2_buffer, width*byte_per_pixel+width_pad, palette_flag );
}


void init_source (j_decompress_ptr cinfo)
{
}

boolean fill_input_buffer (j_decompress_ptr cinfo)
{
    my_source_mgr *src = (my_source_mgr *)cinfo->src;
    
    src->pub.next_input_byte = src->buf;
    src->pub.bytes_in_buffer = src->left;

    return TRUE;
}

void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    my_source_mgr *src = (my_source_mgr *)cinfo->src;
    
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
}

void term_source (j_decompress_ptr cinfo)
{
}

void init_destination (j_compress_ptr cinfo)
{
    my_destination_mgr * dest = (my_destination_mgr *) cinfo->dest;

    dest->pub.next_output_byte = dest->buf;
    dest->pub.free_in_buffer = dest->left;
}

boolean empty_output_buffer (j_compress_ptr cinfo)
{
    my_destination_mgr * dest = (my_destination_mgr *) cinfo->dest;

    dest->pub.next_output_byte = dest->buf;
    dest->pub.free_in_buffer = dest->left;
    
    return TRUE;
}

void term_destination (j_compress_ptr cinfo)
{
}

size_t rescaleJPEGWrite( unsigned int width, unsigned int height, int byte_per_pixel, unsigned char **rescaled_buffer,
                         int quality, bool bmp2jpeg_flag )
{
    jpeg_error_mgr jerr;
    struct jpeg_compress_struct cinfo2;
    JSAMPROW row_pointer[1];
    
    cinfo2.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo2);

    cinfo2.dest = (struct jpeg_destination_mgr *)
        (*cinfo2.mem->alloc_small) ((j_common_ptr) &cinfo2, JPOOL_PERMANENT,
                                    sizeof(my_destination_mgr));
    my_destination_mgr * dest = (my_destination_mgr *) cinfo2.dest;

    dest->buf = *rescaled_buffer;
    dest->left = restored_length;

    dest->pub.init_destination = init_destination;
    dest->pub.empty_output_buffer = empty_output_buffer;
    dest->pub.term_destination = term_destination;

    cinfo2.image_width = (int)(width * scale_ratio_upper / scale_ratio_lower);
    if ( cinfo2.image_width == 0 ) cinfo2.image_width = 1;
    cinfo2.image_height = (int)(height * scale_ratio_upper / scale_ratio_lower);
    if ( cinfo2.image_height == 0 ) cinfo2.image_height = 1;
    cinfo2.input_components = byte_per_pixel;
    if ( cinfo2.input_components == 1 )
        cinfo2.in_color_space = JCS_GRAYSCALE;
    else
        cinfo2.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo2);
    jpeg_set_quality(&cinfo2, quality, TRUE );
    cinfo2.optimize_coding = true;
    //jpeg_simple_progression (&cinfo2);
    jpeg_start_compress(&cinfo2, TRUE);

    int row_stride = cinfo2.image_width * byte_per_pixel;

    while (cinfo2.next_scanline < cinfo2.image_height) {
        if (bmp2jpeg_flag){
            unsigned char *src = row_pointer[0] = &rescaled_tmp_buffer[(cinfo2.image_height - 1 - cinfo2.next_scanline) * row_stride];
            for(unsigned int i=0 ; i<cinfo2.image_width ; i++, src+=3){
                unsigned char tmp = src[2];
                src[2] = src[0];
                src[0] = tmp;
            }
        }
        else{
            row_pointer[0] = &rescaled_tmp_buffer[cinfo2.next_scanline * row_stride];
        }
        jpeg_write_scanlines(&cinfo2, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo2);
    size_t datacount = dest->left - dest->pub.free_in_buffer;

    jpeg_destroy_compress(&cinfo2);

    return datacount;
}

size_t rescaleJPEG( unsigned char *original_buffer, size_t length, unsigned char **rescaled_buffer, int quality )
{
    struct jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    cinfo.src = (struct jpeg_source_mgr *)
        (*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
                                   sizeof(my_source_mgr));
    my_source_mgr * src = (my_source_mgr *) cinfo.src;
    
    src->buf = original_buffer;
    src->left = length;

    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart;
    src->pub.term_source = term_source;

    src->pub.bytes_in_buffer = 0;
    src->pub.next_input_byte = NULL;

    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    if ( cinfo.output_width * cinfo.output_height * cinfo.output_components + 0x400 > restored_length ){
        restored_length = cinfo.output_width * cinfo.output_height * cinfo.output_components + 0x400;
        if ( restored_buffer ) delete[] restored_buffer;
        restored_buffer = new unsigned char[ restored_length ];
        if ( *rescaled_buffer ) delete[] *rescaled_buffer;
        *rescaled_buffer = new unsigned char[ restored_length ];
    }
    int row_stride = cinfo.output_width * cinfo.output_components;

    JSAMPARRAY buf = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    unsigned char *buf_p = restored_buffer;
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buf, 1);
        memcpy( buf_p, buf[0], row_stride );
        buf_p += cinfo.output_width * cinfo.output_components;
    }

    rescaleImage( restored_buffer, cinfo.output_width, cinfo.output_height, cinfo.output_components, false, false, false );

    size_t datacount = rescaleJPEGWrite(cinfo.output_width, cinfo.output_height, cinfo.output_components, rescaled_buffer, quality, false);
    jpeg_destroy_decompress(&cinfo);

    return datacount;
}

void rescaleBMPWrite( unsigned char *original_buffer, size_t total_size, int width, int height, unsigned char **rescaled_buffer )
{
    int buffer_offset = original_buffer[10] + (original_buffer[11] << 8);
    memcpy( *rescaled_buffer, original_buffer, buffer_offset );
    memcpy( *rescaled_buffer + buffer_offset, rescaled_tmp_buffer, total_size - buffer_offset );

    *(*rescaled_buffer + 2) = total_size & 0xff;
    *(*rescaled_buffer + 3) = (total_size >>  8) & 0xff;
    *(*rescaled_buffer + 4) = (total_size >> 16) & 0xff;
    *(*rescaled_buffer + 5) = (total_size >> 24) & 0xff;
    *(*rescaled_buffer + 18) = width & 0xff;
    *(*rescaled_buffer + 19) = (width >>  8) & 0xff;
    *(*rescaled_buffer + 20) = (width >> 16) & 0xff;
    *(*rescaled_buffer + 21) = (width >> 24) & 0xff;
    *(*rescaled_buffer + 22) = height & 0xff;
    *(*rescaled_buffer + 23) = (height >>  8) & 0xff;
    *(*rescaled_buffer + 24) = (height >> 16) & 0xff;
    *(*rescaled_buffer + 25) = (height >> 24) & 0xff;
    *(*rescaled_buffer + 34) = 0;
    *(*rescaled_buffer + 35) = 0;
    *(*rescaled_buffer + 36) = 0;
    *(*rescaled_buffer + 37) = 0;

#if 0
    FILE *fp = fopen( "test.bmp", "wb" );
    fwrite( *rescaled_buffer, 1, width2 * height2 * byte_per_pixel + 54 + color_num*4, fp );
    fclose(fp);
    getchar();
#endif
}

size_t rescaleBMP( unsigned char *original_buffer, unsigned char **rescaled_buffer,
                   bool output_jpeg_flag, int quality )
{
    if (original_buffer[14] != 40){
        if (original_buffer[14] == 12)
            fprintf( stderr, " OS/2 format is not supported.\n");
        else
            fprintf( stderr, " this bitmap can't be handled.\n");
        exit(-1);
    }

    int buffer_offset = original_buffer[10] + (original_buffer[11] << 8);
    int width  = original_buffer[18] + (original_buffer[19] << 8);
    int height = original_buffer[22] + (original_buffer[23] << 8);

    int bit_per_pixel = original_buffer[28];
    if (bit_per_pixel == 1 || bit_per_pixel == 4){
        fprintf( stderr, " bit_per_pixel %d is not supported.\n", bit_per_pixel);
        exit(-1);
    }
    int byte_per_pixel = bit_per_pixel / 8;
    int color_num = original_buffer[46] + ((int)original_buffer[47] << 8) + (original_buffer[48] << 16) + (original_buffer[49] << 24);
    if (bit_per_pixel == 8 && color_num == 0) color_num = 256;

    bool palette_flag = false;
    if (bit_per_pixel == 8) palette_flag = true;
    if (palette_flag) output_jpeg_flag = false;

    size_t width2  = (int)(width * scale_ratio_upper / scale_ratio_lower);
    if ( width2 == 0 ) width2 = 1;
    size_t width2_pad = (4 - width2 * byte_per_pixel % 4) % 4;
    
    size_t height2 = (int)(height * scale_ratio_upper / scale_ratio_lower);
    if ( height2 == 0 ) height2 = 1;

    size_t total_size = (width2 * byte_per_pixel + width2_pad) * height2 + buffer_offset;
    if ( total_size+0x400 > restored_length ){
        restored_length = total_size+0x400;
        if ( restored_buffer ) delete[] restored_buffer;
        restored_buffer = new unsigned char[ restored_length ];
        if ( *rescaled_buffer ) delete[] *rescaled_buffer;
        *rescaled_buffer = new unsigned char[ restored_length ];
    }

    if (output_jpeg_flag){
        rescaleImage( original_buffer+buffer_offset, width, height, byte_per_pixel, true, false, palette_flag );
        total_size = rescaleJPEGWrite(width, height, byte_per_pixel, rescaled_buffer, quality, true);
    }
    else {
        rescaleImage( original_buffer+buffer_offset, width, height, byte_per_pixel, true, true, palette_flag );
        rescaleBMPWrite(original_buffer, total_size, width2, height2, rescaled_buffer);
    }

    return total_size;
}

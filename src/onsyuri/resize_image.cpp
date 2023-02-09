/* -*- C++ -*-
 * 
 *  resize_image.cpp - resize image using smoothing and resampling
 *
 *  Copyright (c) 2001-2012 Ogapee. All rights reserved.
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
#include <string.h>

static unsigned long *pixel_accum=NULL;
static unsigned long *pixel_accum_num=NULL;
static int pixel_accum_size=0;
static unsigned long tmp_acc[4];
static unsigned long tmp_acc_num[4];

static void calcWeightedSumColumnInit(unsigned char **src,
                                      int interpolation_height,
                                      int image_width, int image_height, int image_pixel_width, int byte_per_pixel)
{
    int y_end   = -interpolation_height/2+interpolation_height;

    memset(pixel_accum, 0, image_width*byte_per_pixel*sizeof(unsigned long));
    memset(pixel_accum_num, 0, image_width*byte_per_pixel*sizeof(unsigned long));
    for (int s=0 ; s<byte_per_pixel ; s++){
        for (int i=0 ; i<y_end-1 ; i++){
            if (i >= image_height) break;
            unsigned long *pa = pixel_accum + image_width*s;
            unsigned long *pan = pixel_accum_num + image_width*s;
            unsigned char *p = *src+image_pixel_width*i+s;
            for (int j=image_width ; j!=0 ; j--, p+=byte_per_pixel){
                *pa++ += *p;
                (*pan++)++;
            }
        }
    }
}

static void calcWeightedSumColumn(unsigned char **src, int y,
                                  int interpolation_height,
                                  int image_width, int image_height, int image_pixel_width, int byte_per_pixel)
{
    int y_start = y-interpolation_height/2;
    int y_end   = y-interpolation_height/2+interpolation_height;

    for (int s=0 ; s<byte_per_pixel ; s++){
        if ((y_start-1)>=0 && (y_start-1)<image_height){
            unsigned long *pa = pixel_accum + image_width*s;
            unsigned long *pan = pixel_accum_num + image_width*s;
            unsigned char *p = *src+image_pixel_width*(y_start-1)+s;
            for (int j=image_width ; j!=0 ; j--, p+=byte_per_pixel){
                *pa++ -= *p;
                (*pan++)--;
            }
        }
        
        if ((y_end-1)>=0 && (y_end-1)<image_height){
            unsigned long *pa = pixel_accum + image_width*s;
            unsigned long *pan = pixel_accum_num + image_width*s;
            unsigned char *p = *src+image_pixel_width*(y_end-1)+s;
            for (int j=image_width ; j!=0 ; j--, p+=byte_per_pixel){
                *pa++ += *p;
                (*pan++)++;
            }
        }
    }
}

static void calcWeightedSum(unsigned char **dst, unsigned char **src, int x,
                            int interpolation_width,
                            int image_width, int byte_per_pixel)
{
    int x_start = x-interpolation_width/2;
    int x_end   = x-interpolation_width/2+interpolation_width;
    
    for (int s=0 ; s<byte_per_pixel ; s++){
        if ((x_start-1)>=0 && (x_start-1)<image_width){
            tmp_acc[s] -= pixel_accum[image_width*s+x_start-1];
            tmp_acc_num[s] -= pixel_accum_num[image_width*s+x_start-1];
        }
        if ((x_end-1)>=0 && (x_end-1)<image_width){
            tmp_acc[s] += pixel_accum[image_width*s+x_end-1];
            tmp_acc_num[s] += pixel_accum_num[image_width*s+x_end-1];
        }
        *(*dst)++ = (unsigned char)(tmp_acc[s]/tmp_acc_num[s]);
    }
}

void resizeImage( unsigned char *dst_buffer, int dst_width, int dst_height, int dst_total_width,
                  unsigned char *src_buffer, int src_width, int src_height, int src_total_width,
                  int byte_per_pixel, unsigned char *tmp_buffer, int tmp_total_width, bool palette_flag )
{
    if (dst_width == 0 || dst_height == 0) return;
    
    unsigned char *tmp_buf = tmp_buffer;
    unsigned char *src_buf = src_buffer;

    int i, j, s;
    int tmp_offset = tmp_total_width - src_width * byte_per_pixel;

    unsigned int mx, my;

    if ( src_width  > 1 ) mx = 1;
    else                  mx = 0;
    if ( src_height > 1 ) my = 1;
    else                  my = 0;

    int interpolation_width = src_width / dst_width;
    if ( interpolation_width == 0 ) interpolation_width = 1;
    int interpolation_height = src_height / dst_height;
    if ( interpolation_height == 0 ) interpolation_height = 1;

    if (pixel_accum_size < src_width*byte_per_pixel){
        pixel_accum_size = src_width*byte_per_pixel;
        if (pixel_accum) delete[] pixel_accum;
        pixel_accum = new unsigned long[pixel_accum_size];
        if (pixel_accum_num) delete[] pixel_accum_num;
        pixel_accum_num = new unsigned long[pixel_accum_size];
    }
    /* smoothing */
    if ( byte_per_pixel >= 3 ){
        calcWeightedSumColumnInit(&src_buf, interpolation_height,
                                  src_width, src_height, src_total_width, byte_per_pixel );
        for ( i=0 ; i<src_height ; i++ ){
            calcWeightedSumColumn(&src_buf, i, interpolation_height,
                                  src_width, src_height, src_total_width, byte_per_pixel );
            for (s=0 ; s<byte_per_pixel ; s++){
                tmp_acc[s]=0;
                tmp_acc_num[s]=0;
                for (j=0 ; j<-interpolation_width/2+interpolation_width-1 ; j++){
                    if (j >= src_width) break;
                    tmp_acc[s] += pixel_accum[src_width*s+j];
                    tmp_acc_num[s] += pixel_accum_num[src_width*s+j];
                }
            }
            
            for ( j=0 ; j<src_width ; j++ )
                calcWeightedSum(&tmp_buf, &src_buf, j,
                                interpolation_width,
                                src_width, byte_per_pixel );
            tmp_buf += tmp_offset;
        }
    }
    else{
        tmp_buffer = src_buffer;
    }
    
    /* resampling */
    unsigned char *dst_buf = dst_buffer;
    int dh1 = dst_height-1; if (dh1==0) dh1 = 1;
    int dw1 = dst_width-1;  if (dw1==0) dw1 = 1;
    for ( i=0 ; i<dst_height ; i++ ){
        int y = (i<<3) * (src_height-1) / dh1;
        int dy = y & 0x7;
        y >>= 3;
        for ( j=0 ; j<dst_width ; j++ ){
            int x = (j<<3) * (src_width-1) / dw1;
            int dx = x & 0x7;
            x >>= 3;

            int k = tmp_total_width * y + x * byte_per_pixel;
            
            if (palette_flag){ //assuming byte_per_pixel=1
                *dst_buf++ = tmp_buffer[k];
            }
            else{
                for ( s=0 ; s<byte_per_pixel ; s++, k++ ){
                    unsigned int p;
                    p =  (8-dx)*(8-dy)*tmp_buffer[ k ];
                    p +=    dx *(8-dy)*tmp_buffer[ k+mx*byte_per_pixel ];
                    p += (8-dx)*   dy *tmp_buffer[ k+my*tmp_total_width ];
                    p +=    dx *   dy *tmp_buffer[ k+mx*byte_per_pixel+my*tmp_total_width ];
                    *dst_buf++ = (unsigned char)(p>>6);
                }
            }
        }
        for ( j=0 ; j<dst_total_width - dst_width*byte_per_pixel ; j++ )
            *dst_buf++ = 0;
    }

    /* pixels at the corners are preserved */
    for ( i=0 ; i<byte_per_pixel ; i++ ){
        dst_buffer[i] = src_buffer[i];
        dst_buffer[(dst_width-1)*byte_per_pixel+i] = src_buffer[(src_width-1)*byte_per_pixel+i];
        dst_buffer[(dst_height-1)*dst_total_width+i] = src_buffer[(src_height-1)*src_total_width+i];
        dst_buffer[(dst_height-1)*dst_total_width+(dst_width-1)*byte_per_pixel+i] =
            src_buffer[(src_height-1)*src_total_width+(src_width-1)*byte_per_pixel+i];
    }
}

/* -*- C++ -*-
*
*  layer_snow.cpp
*    - Emulation of Takashi Toyama's "snow.dll" and "hana.dll" NScripter plugin effect
*
*  Copyright (c) 2008-2012 "Uncle" Mion Sonozaki
*            (C) 2015 jh10001 <jh10001@live.cn>
*
*  UncleMion@gmail.com
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

#ifdef USE_BUILTIN_LAYER_EFFECTS
#include "../builtin_layer.h"
#include "../ONScripter.h"
#include <math.h>
#include <stdlib.h>
#include <SDL_image.h>

extern ONScripter ons;

static const float FURU_RATE_COEF = 0.2;
static float *base_disp_table = NULL;
static int furu_count = 0;
static const float fall_mult[N_FURU_ELEMENTS] = { 0.9, 0.7, 0.6 };

static void buildBaseDispTable()
{
  if (base_disp_table) return;

  base_disp_table = new float[FURU_AMP_TABLE_SIZE];
  // a = sin? * Z(cos?)
  // Z(z) = rate_z * z +1
  for (int i = 0; i<FURU_AMP_TABLE_SIZE; ++i) {
    float rad = (float)i * M_PI * 2 / FURU_AMP_TABLE_SIZE;
    base_disp_table[i] = sin(rad) * (FURU_RATE_COEF * cos(rad) + 1);
  }
}

FuruLayer::FuruLayer(int w, int h, bool animated, BaseReader *br)
{
  width = w;
  height = h;
  tumbling = animated;
  reader = br;

  interval = fall_velocity = wind = amplitude = freq = angle = 0;
  paused = halted = false;
  max_sp_w = 0;

  initialized = false;
}

FuruLayer::~FuruLayer(){
  if (initialized) {
    --furu_count;
    if (furu_count == 0) {
      delete[] base_disp_table;
      base_disp_table = NULL;
    }
  }
}

void FuruLayer::furu_init()
{
  for (int i = 0; i<N_FURU_ELEMENTS; i++) {
    elements[i].init();
  }
  angle = 0;
  halted = false;
  paused = false;

  buildBaseDispTable();

  ++furu_count;
  initialized = true;
}

static void setStr(char **dst, const char *src, int num = -1)
{
  if (*dst) delete[] * dst;
  *dst = NULL;

  if (src) {
    if (num >= 0) {
      *dst = new char[num + 1];
      memcpy(*dst, src, num);
      (*dst)[num] = '\0';
    } else {
      *dst = new char[strlen(src) + 1];
      strcpy(*dst, src);
    }
  }
}

static SDL_Surface *loadImage(char *file_name, bool *has_alpha, SDL_Surface *surface, BaseReader *br)
{
  if (!file_name) return NULL;
  unsigned long length = br->getFileLength(file_name);

  if (length == 0)
    return NULL;
  unsigned char *buffer = new unsigned char[length];
  int location;
  br->getFile(file_name, buffer, &location);
  SDL_Surface *tmp = IMG_Load_RW(SDL_RWFromMem(buffer, length), 1);

  char *ext = strrchr(file_name, '.');
  if (!tmp && ext && (!strcmp(ext + 1, "JPG") || !strcmp(ext + 1, "jpg"))) {
    fprintf(stderr, " *** force-loading a JPG image [%s]\n", file_name);
    SDL_RWops *src = SDL_RWFromMem(buffer, length);
    tmp = IMG_LoadJPG_RW(src);
    SDL_RWclose(src);
  }
  if (tmp && has_alpha) *has_alpha = tmp->format->Amask;

  delete[] buffer;
  if (!tmp) {
    fprintf(stderr, " *** can't load file [%s] ***\n", file_name);
    return NULL;
  }

  SDL_Surface *ret = SDL_ConvertSurface(tmp, surface->format, SDL_SWSURFACE);
  SDL_FreeSurface(tmp);
  return ret;
}

void FuruLayer::buildAmpTables()
{
  float amp[N_FURU_ELEMENTS];
  amp[0] = (float)amplitude;
  for (int i = 1; i<N_FURU_ELEMENTS; ++i)
    amp[i] = amp[i - 1] * 0.8;

  for (int i = 0; i<N_FURU_ELEMENTS; ++i) {
    Element *cur = &elements[i];
    if (!cur->amp_table)
      cur->amp_table = new int[FURU_AMP_TABLE_SIZE];
    for (int j = 0; j<FURU_AMP_TABLE_SIZE; ++j)
      cur->amp_table[j] = (int)(amp[i] * base_disp_table[j]);
  }
}

void FuruLayer::validate_params()
{
  const int half_wx = width / 2;

  if (interval < 1) interval = 1;
  else if (interval > 10000) interval = 10000;
  if (fall_velocity < 1) fall_velocity = 1;
  else if (fall_velocity > height) fall_velocity = height;
  for (int i = 0; i<N_FURU_ELEMENTS; i++)
    elements[i].fall_speed = (int)(fall_mult[i] * (fall_velocity + 1));
  if (wind < -half_wx) wind = -half_wx;
  else if (wind > half_wx) wind = half_wx;
  if (amplitude < 0) amplitude = 0;
  else if (amplitude > half_wx) amplitude = half_wx;
  if (amplitude != 0) buildAmpTables();
  if (freq < 0) freq = 0;
  else if (freq > 359) freq = 359;
  //adjust the freq to range 0-FURU_AMP_TABLE_SIZE-1
  freq = freq * FURU_AMP_TABLE_SIZE / 360;
}

char *FuruLayer::message(const char *message, int &ret_int)
{
  int num_cells[3], tmp[5];
  char buf[3][128];

  char *ret_str = NULL;
  ret_int = 0;

  if (!sprite)
    return NULL;

  //printf("FuruLayer: got message '%s'\n", message);
  //Image loading
  if (!strncmp(message, "i|", 2)) {
    max_sp_w = 0;
    SDL_Surface *ref_surface = AnimationInfo::alloc32bitSurface(1,1, ons.getTextureFormat());
    if (tumbling) {
      // "Hana"
      if (sscanf(message, "i|%d,%d,%d,%d,%d,%d",
        &tmp[0], &num_cells[0],
        &tmp[1], &num_cells[1],
        &tmp[2], &num_cells[2])) {
        for (int i = 0; i<3; i++) {
          elements[i].setSprite(new AnimationInfo(sprite_info[tmp[i]]));
          elements[i].sprite->num_of_cells = num_cells[i];
          if (elements[i].sprite->pos.w > max_sp_w)
            max_sp_w = elements[i].sprite->pos.w;
        }
      } else
        if (sscanf(message, "i|%120[^,],%d,%120[^,],%d,%120[^,],%d",
          &buf[0][0], &num_cells[0],
          &buf[1][0], &num_cells[1],
          &buf[2][0], &num_cells[2])) {
          for (int i = 0; i<3; i++) {
            bool has_alpha = false;
            SDL_Surface *img = loadImage(&buf[i][0], &has_alpha, ref_surface, reader);
            AnimationInfo *anim = new AnimationInfo();
            anim->num_of_cells = num_cells[i];
            anim->duration_list = new int[anim->num_of_cells];
            for (int j = anim->num_of_cells - 1; j >= 0; --j)
              anim->duration_list[j] = 0;
            anim->loop_mode = 3; // not animatable
            anim->trans_mode = AnimationInfo::TRANS_TOPLEFT;
            setStr(&anim->file_name, &buf[i][0]);
            anim->setImage(anim->setupImageAlpha(img, NULL, has_alpha), ons.getTextureFormat());
            elements[i].setSprite(anim);
            if (anim->pos.w > max_sp_w)
              max_sp_w = anim->pos.w;
          }
        }
    } else {
      // "Snow"
      if (sscanf(message, "i|%d,%d,%d",
        &tmp[0], &tmp[1], &tmp[2])) {
        for (int i = 0; i<3; i++) {
          elements[i].setSprite(new AnimationInfo(sprite_info[tmp[i]]));
          if (elements[i].sprite->pos.w > max_sp_w)
            max_sp_w = elements[i].sprite->pos.w;
        }
      } else if (sscanf(message, "i|%[^,],%[^,],%[^,]",
        &buf[0][0], &buf[1][0], &buf[2][0])) {
        for (int i = 0; i<3; i++) {
          Uint32 firstpix = 0;
          bool has_alpha = false;
          SDL_Surface *img = loadImage(&buf[i][0], &has_alpha, ref_surface, reader);
          AnimationInfo *anim = new AnimationInfo();
          anim->num_of_cells = 1;
          SDL_LockSurface(img);
          firstpix = *((Uint32*)img->pixels) & ~(img->format->Amask);
          if (firstpix > 0) {
            anim->trans_mode = AnimationInfo::TRANS_TOPLEFT;
          } else {
            // if first pix is black, this is an "additive" sprite
            anim->trans_mode = AnimationInfo::TRANS_COPY;
            anim->blending_mode = AnimationInfo::BLEND_ADD;
          }
          SDL_UnlockSurface(img);
          setStr(&anim->file_name, &buf[i][0]);
          anim->setImage(anim->setupImageAlpha(img, NULL, has_alpha), ons.getTextureFormat());
          elements[i].setSprite(anim);
          if (anim->pos.w > max_sp_w)
            max_sp_w = anim->pos.w;
        }
      }
    }
    SDL_FreeSurface(ref_surface);
    //Set Parameters
  } else if (sscanf(message, "s|%d,%d,%d,%d,%d",
    &interval, &fall_velocity, &wind,
    &amplitude, &freq)) {
    furu_init();
    validate_params();
    //Transition (adjust) Parameters
  } else if (sscanf(message, "t|%d,%d,%d,%d,%d",
    &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4])) {
    interval += tmp[0];
    fall_velocity += tmp[1];
    wind += tmp[2];
    amplitude += tmp[3];
    freq += tmp[4];
    validate_params();
    //Fill Screen w/Elements
  } else if (!strcmp(message, "f")) {
    if (initialized) {
      for (int j = 0; j<N_FURU_ELEMENTS; j++) {
        Element *cur = &elements[j];
        int y = 0;
        while (y < height) {
          const int tmp = (cur->pend + 1) % FURU_ELEMENT_BUFSIZE;
          if (tmp != cur->pstart) {
            // add a point for each element
            OscPt *item = &cur->points[cur->pend];
            item->pt.x = rand() % (width + max_sp_w);
            item->pt.y = y;
            item->pt.type = j;
            item->pt.cell = rand() % cur->sprite->num_of_cells;
            item->base_angle = rand() % FURU_AMP_TABLE_SIZE;
            cur->pend = tmp;
          }
          y += interval * cur->fall_speed;
        }
      }
    }
    //Get Parameters
  } else if (!strcmp(message, "g")) {
    ret_int = paused ? 1 : 0;
    sprintf(&buf[0][0], "s|%d,%d,%d,%d,%d", interval, fall_velocity,
      wind, amplitude, (freq * 360 / FURU_AMP_TABLE_SIZE));
    setStr(&ret_str, &buf[0][0]);
    //Halt adding new elements
  } else if (!strcmp(message, "h")) {
    halted = true;
    //Get number of elements displayed
  } else if (!strcmp(message, "n")) {
    for (int i = 0; i<N_FURU_ELEMENTS; i++)
      ret_int += (elements[i].pend - elements[i].pstart + FURU_ELEMENT_BUFSIZE)
      % FURU_ELEMENT_BUFSIZE;
    //Pause
  } else if (!strcmp(message, "p")) {
    paused = true;
    //Restart
  } else if (!strcmp(message, "r")) {
    paused = false;
    //eXtinguish
  } else if (!strcmp(message, "x")) {
    for (int i = 0; i<N_FURU_ELEMENTS; i++)
      elements[i].clear();
    initialized = false;
  }
  return ret_str;
}

void FuruLayer::update()
{
  if (initialized && !paused) {
    if (amplitude != 0)
      angle = (angle - freq + FURU_AMP_TABLE_SIZE) % FURU_AMP_TABLE_SIZE;
    for (int j = 0; j<N_FURU_ELEMENTS; ++j) {
      Element *cur = &elements[j];
      int i = cur->pstart;
      const int virt_w = width + max_sp_w;
      while (i != cur->pend) {
        cur->points[i].pt.x = (cur->points[i].pt.x + wind + virt_w) % virt_w;
        cur->points[i].pt.y += cur->fall_speed;
        ++(cur->points[i].pt.cell) %= cur->sprite->num_of_cells;
        ++i %= FURU_ELEMENT_BUFSIZE;
      }
      if (!halted) {
        if (--(cur->frame_cnt) <= 0) {
          const int tmp = (cur->pend + 1) % FURU_ELEMENT_BUFSIZE;
          cur->frame_cnt += interval;
          if (tmp != cur->pstart) {
            // add a point for this element
            OscPt *item = &cur->points[cur->pend];
            item->pt.x = rand() % virt_w;
            item->pt.y = -(cur->sprite->pos.h);
            item->pt.type = j;
            item->pt.cell = 0;
            item->base_angle = rand() % FURU_AMP_TABLE_SIZE;
            cur->pend = tmp;
          }
        }
      }
      while ((cur->pstart != cur->pend) &&
        (cur->points[cur->pstart].pt.y >= height))
        ++(cur->pstart) %= FURU_ELEMENT_BUFSIZE;
    }
  }
}

void FuruLayer::refresh(SDL_Surface *surface, SDL_Rect &clip)
{
  if (initialized) {
    const int virt_w = width + max_sp_w;
    for (int j = 0; j<N_FURU_ELEMENTS; j++) {
      Element *cur = &elements[j];
      if (cur->sprite) {
        cur->sprite->visible = true;
        const int n = (cur->pend - cur->pstart + FURU_ELEMENT_BUFSIZE) % FURU_ELEMENT_BUFSIZE;
        int p = cur->pstart;
        if (amplitude == 0) {
          //no need to mess with angles if no displacement
          for (int i = n; i>0; i--) {
            OscPt *curpt = &cur->points[p];
            ++p %= FURU_ELEMENT_BUFSIZE;
            cur->sprite->current_cell = curpt->pt.cell;
            cur->sprite->pos.x = ((curpt->pt.x + virt_w) % virt_w) - max_sp_w;
            cur->sprite->pos.y = curpt->pt.y;
            drawTaggedSurface(surface, cur->sprite, clip);
          }
        } else {
          for (int i = n; i>0; i--) {
            OscPt *curpt = &cur->points[p];
            ++p %= FURU_ELEMENT_BUFSIZE;
            const int disp_angle = (angle + curpt->base_angle + FURU_AMP_TABLE_SIZE) % FURU_AMP_TABLE_SIZE;
            cur->sprite->current_cell = curpt->pt.cell;
            cur->sprite->pos.x = ((curpt->pt.x + cur->amp_table[disp_angle] + virt_w) % virt_w) - max_sp_w;
            cur->sprite->pos.y = curpt->pt.y;
            drawTaggedSurface(surface, cur->sprite, clip);
          }
        }
      }
    }
  }
}
#endif
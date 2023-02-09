/* -*- C++ -*-
*
*  builtin_layer.h
*
*  Copyright (c) 2009 "Uncle" Mion Sonozaki
*            (C) 2015-2016 jh10001 <jh10001@live.cn>
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
#pragma once
#ifdef USE_BUILTIN_LAYER_EFFECTS
#include "BaseReader.h"
#include "AnimationInfo.h"

#define MAX_LAYER_NUM 32

struct Layer {
  BaseReader *reader;
  AnimationInfo *sprite_info, *sprite;
  int width, height;

  virtual ~Layer(){};

  void setSpriteInfo(AnimationInfo *sinfo, AnimationInfo *anim){
    sprite_info = sinfo;
    sprite = anim;
  };
  virtual void update() = 0;
  virtual char* message(const char *message, int &ret_int) = 0;
  virtual void refresh(SDL_Surface* surface, SDL_Rect &clip) = 0;
};

struct LayerInfo {
  Layer *handler;
  int sprite_num;
  Uint32 interval;
  Uint32 last_update;
  LayerInfo(){
    sprite_num = -1;
    interval = last_update = 0;
    handler = nullptr;
  }
  ~LayerInfo(){
    if (handler) {
      delete handler;
      handler = nullptr;
    }
  }
};
extern LayerInfo layer_info[MAX_LAYER_NUM];

class OldMovieLayer : public Layer {
public:
  OldMovieLayer(int w, int h);
  ~OldMovieLayer();
  void update();
  char* message(const char *message, int &ret_int);
  void refresh(SDL_Surface* surface, SDL_Rect &clip);

private:
  // message parameters
  int blur_level;
  int noise_level;
  int glow_level;
  int scratch_level;
  int dust_level;
  AnimationInfo *dust_sprite;
  AnimationInfo *dust;

  struct Pt { int x; int y; int type; int cell; } *dust_pts;
  int rx, ry, // Offset of blur (second copy of background image)
    ns;     // Current noise surface
  int gv, // Current glow level
    go; // Glow delta: flips between 1 and -1 to fade the glow in and out.
  bool initialized;

  void om_init();
};

static const int N_FURU_ELEMENTS = 3;
static const int FURU_ELEMENT_BUFSIZE = 512; // should be a power of 2
static const int FURU_AMP_TABLE_SIZE = 256; // should also be power of 2, it helps

class FuruLayer : public Layer {
public:
  FuruLayer(int w, int h, bool animated, BaseReader *br = NULL);
  ~FuruLayer();
  void update() final;
  char* message(const char *message, int &ret_int) final;
  void refresh(SDL_Surface* surface, SDL_Rect &clip) final;

private:
  bool tumbling; // true (hana) or false (snow)

  // message parameters
  int interval; // 1 ~ 10000; # frames between a new element release
  int fall_velocity; // 1 ~ screen_height; pix/frame
  int wind; // -screen_width/2 ~ screen_width/2; pix/frame
  int amplitude; // 0 ~ screen_width/2; pix/frame
  int freq; // 0 ~ 359; degree/frame
  int angle;
  bool paused, halted;

  struct OscPt { // point plus base oscillation angle
    int base_angle;
    struct {
      int x, y, type, cell;
    } pt;
  };
  struct Element {
    AnimationInfo *sprite;
    int *amp_table;
    // rolling buffer
    OscPt *points;
    int pstart, pend, frame_cnt, fall_speed;
    Element(){
      sprite = NULL;
      amp_table = NULL;
      points = NULL;
      pstart = pend = frame_cnt = fall_speed = 0;
    };
    ~Element(){
      if (sprite) delete sprite;
      if (amp_table) delete[] amp_table;
      if (points) delete[] points;
    };
    void init(){
      if (!points) points = new OscPt[FURU_ELEMENT_BUFSIZE];
      pstart = pend = frame_cnt = 0;
    };
    void clear(){
      if (sprite) delete sprite;
      sprite = NULL;
      if (amp_table) delete[] amp_table;
      amp_table = NULL;
      if (points) delete[] points;
      points = NULL;
      pstart = pend = frame_cnt = 0;
    };
    void setSprite(AnimationInfo *anim){
      if (sprite) delete sprite;
      sprite = anim;
    };
  } elements[N_FURU_ELEMENTS];
  int max_sp_w;

  bool initialized;

  void furu_init();
  void validate_params();
  void buildAmpTables();
};

inline void drawTaggedSurface(SDL_Surface *dst_surface, AnimationInfo *anim, SDL_Rect &clip) {
  anim->blendOnSurface(dst_surface, anim->pos.x, anim->pos.y, clip, anim->trans);
}
#endif

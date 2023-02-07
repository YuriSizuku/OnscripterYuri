/* -*- C++ -*-
*
*  layer_oldmovie.cpp
*    - Emulation of Takashi Toyama's "oldmovie.dll" NScripter plugin filters
*
*  Copyright (c) 2009-2011 "Uncle" Mion Sonozaki
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
#include <stdlib.h>

#define MAX_NOISE          8
#define MAX_GLOW          25
#define MAX_DUST_COUNT    10
#define MAX_SCRATCH_COUNT  6

extern ONScripter ons;

static int scratch_count;    // Number of scratches visible.

class Scratch {
private:
  int offs;   // Tint of the line: 64 for light, -64 for dark, 0 for no scratch.
  int x1, x2; // Horizontal position of top and bottom of the line.
  int dx;     // Distance by which the line moves each frame.
  int time;   // Number of frames remaining before reinitialisation.
  int width, height;
  void init(int level);
public:
  Scratch() : offs(0), time(1) {}
  void setwindow(int w, int h){ width = w; height = h; }
  void update(int level);
  void draw(SDL_Surface* surface, SDL_Rect clip);
};

// Create a new scratch.
void Scratch::init(int level)
{
  // If this scratch was visible, decrement the counter.
  if (offs) --scratch_count;
  offs = 0;

  // Each scratch object is reinitialised every 3-9 frames.
  time = rand() % 7 + 3;

  if ((rand() % 600) < level) {
    ++scratch_count;
    offs = rand() % 2 ? 64 : -64;
    x1 = rand() % (width - 20) + 10;
    dx = rand() % 12 - 6;
    x2 = x1 - dx; // The angle of the line is determined by the speed of motion.
  }
}

// Called each frame.
void Scratch::update(int level)
{
  if (--time == 0)
    init(level);
  else if (offs) {
    x1 += dx;
    x2 += dx;
  }
}

// Called each time the screen is refreshed.  Draws a simple line, without antialiasing.
void Scratch::draw(SDL_Surface* surface, SDL_Rect clip)
{
  // Don't draw unless this scratch is visible and likely to pass through the updated rectangle.
  if ((offs == 0) || (x1 < clip.x) || (x2 < clip.x) ||
    (x1 >= (clip.x + clip.w)) || (x2 >= (clip.x + clip.w)))
    return;

  const int sp = surface->pitch;
  float dx = (float)(x2 - x1) / width;
  float realx = (float)x1;
  int y = 0;
  while (y != clip.y) {
    // Skip all scanlines above the clipping rectangle.
    ++y;
    realx += dx;
  }
  while (y < clip.y + clip.h) {
    int lx = (int)floor(realx + 0.5);
    if (lx >= clip.x && lx < clip.x + clip.w) { // Only draw within the clipping rectangle.
      // Get pixel...
      Uint32* p = (Uint32*)((char*)surface->pixels + y * sp + lx * 4);
      const Uint32 c = *p;
      // ...add to or subtract from its colour...
      int c1 = (c & 0xff) + offs, c2 = ((c >> 8) & 0xff) + offs, c3 = ((c >> 16) & 0xff) + offs;
      if (c1 < 0) c1 = 0; else if (c1 > 255) c1 = 255;
      if (c2 < 0) c2 = 0; else if (c2 > 255) c2 = 255;
      if (c3 < 0) c3 = 0; else if (c3 > 255) c3 = 255;
      // ...and put it back.
      *p = c1 | c2 << 8 | c3 << 16;
    }
    ++y;
    realx += dx;
  }
}

static Scratch scratches[MAX_SCRATCH_COUNT];
static int om_count = 0;
static SDL_Surface* NoiseSurface[MAX_NOISE];
static SDL_Surface* GlowSurface;
static bool initialized_om_surfaces = false;

OldMovieLayer::OldMovieLayer(int w, int h)
{
  width = w;
  height = h;

  blur_level = noise_level = glow_level = scratch_level = dust_level = 0;
  dust_sprite = dust = NULL;
  dust_pts = NULL;

  initialized = false;
}

OldMovieLayer::~OldMovieLayer() {
  if (initialized) {
    --om_count;
    if (om_count == 0) {
      for (int i = 0; i<MAX_NOISE; i++) {
        SDL_FreeSurface(NoiseSurface[i]);
        NoiseSurface[i] = NULL;
      }
      SDL_FreeSurface(GlowSurface);
      GlowSurface = NULL;
      initialized_om_surfaces = false;
    }
    if (dust) delete dust;
    if (dust_pts) delete[] dust_pts;
  }
}

void OldMovieLayer::om_init()
{
  ++om_count;

  gv = 0;
  go = 1;
  rx = ry = 0;
  ns = 0;

  if (dust_sprite) {
    // Copy dust sprite to dust
    if (dust) delete dust;
    dust = new AnimationInfo(*dust_sprite);
    dust->visible = true;
  }
  if (dust_pts) delete[] dust_pts;
  dust_pts = new Pt[MAX_DUST_COUNT];

  initialized = true;

  //don't reinitialise existing noise and glow surfaces or scratches
  if (initialized_om_surfaces) return;

  // set up scratches
  for (int i = 0; i < MAX_SCRATCH_COUNT; i++)
    scratches[i].setwindow(width, height);

  // Generate screens of random noise.
  for (int i = 0; i < MAX_NOISE; i++) {
    NoiseSurface[i] = AnimationInfo::allocSurface(width, height, ons.getTextureFormat());
    SDL_LockSurface(NoiseSurface[i]);
    char* px = (char*)NoiseSurface[i]->pixels;
    const int pt = NoiseSurface[i]->pitch;
    for (int y = 0; y < height; ++y, px += pt) {
      Uint32* row = (Uint32*)px;
      for (int x = 0; x < width; ++x, ++row) {
        const int rm = (rand() % (noise_level + 1)) * 2;
        *row = 0 | (rm << 16) | (rm << 8) | rm;
      }
    }
    SDL_UnlockSurface(NoiseSurface[i]);
  }

  // Generate scanlines of solid greyscale, used for the glow effect.
  GlowSurface = AnimationInfo::allocSurface(width, MAX_GLOW, ons.getTextureFormat());
  for (SDL_Rect r = { 0, 0, width, 1 }; r.y < MAX_GLOW; r.y++) {
    const int ry = (r.y * 30 / MAX_GLOW) + 4;
    SDL_FillRect(GlowSurface, &r, SDL_MapRGB(GlowSurface->format, ry, ry, ry));
  }
}

// Called once each frame.  Updates effect parameters.
void OldMovieLayer::update()
{
  if (!initialized) return;

  const int last_x = rx, last_y = ry, last_n = ns;
  // Generate blur offset and noise screen randomly.
  // Ensure neither setting is the same two frames running.
  if (blur_level > 0) {
    do {
      rx = rand() % (blur_level + 1) - 1;
      ry = rand() % (blur_level + 1);
    } while (rx == last_x && ry == last_y);
  }
  do {
    ns = rand() % MAX_NOISE;
  } while (ns == last_n);

  // Increment glow; reverse direction if we've reached either limit.
  gv += go;
  if (gv >= 5) { gv = 3; go = -1; }
  if (gv < 0) { gv = 1; go = 1; }

  // Update scratches.
  for (int i = 0; i<MAX_SCRATCH_COUNT; i++)
    scratches[i].update(scratch_level);

  // Update dust
  if (dust->num_of_cells > 0) {
    for (int i = 0; i<MAX_DUST_COUNT; i++) {
      dust_pts[i].cell = rand() % (dust->num_of_cells);
      dust_pts[i].x = rand() % (width + 10) - 5;
      dust_pts[i].y = rand() % (height + 10) - 5;
    }
  }
}

char *OldMovieLayer::message(const char *message, int &ret_int)
{
  int sprite_no = 0;
  ret_int = 0;
  if (!sprite_info)
    return NULL;

  printf("OldMovieLayer: got message '%s'\n", message);
  if (sscanf(message, "s|%d,%d,%d,%d,%d,%d",
    &blur_level, &noise_level, &glow_level,
    &scratch_level, &dust_level, &sprite_no)) {
    if (blur_level < 0) blur_level = 0;
    else if (blur_level > 3) blur_level = 3;
    if (noise_level < 0) noise_level = 0;
    else if (noise_level > 24) noise_level = 24;
    if (glow_level < 0) glow_level = 0;
    else if (glow_level > 24) glow_level = 24;
    if (scratch_level < 0) scratch_level = 0;
    else if (scratch_level > 400) scratch_level = 400;
    if (dust_level < 0) dust_level = 0;
    else if (dust_level > 400) dust_level = 400;
    if ((sprite_no >= 0) && (sprite_no < MAX_SPRITE_NUM))
      dust_sprite = &sprite_info[sprite_no];
    om_init();
  }
  return NULL;
}

inline static void imageFilterMean(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length)
{
  int n = length + 1;
  while (--n > 0) {
    int result = ((int)(*src1) + (int)(*src2)) / 2;
    (*dst) = result;
    ++dst; ++src1; ++src2;
  }
}

// Apply blur effect by averaging two offset copies of a source surface together.
static void BlurOnSurface(SDL_Surface* src, SDL_Surface* dst, SDL_Rect clip, int rx, int ry, int width)
{
  // Calculate clipping bounds to avoid reading outside the source surface.
  const int srcx = clip.x - rx;
  const int srcy = clip.y - ry;
  const int length = ((srcx + clip.w > width) ? (width - srcx) : clip.w) * 4;
  int rows = clip.h;
  const int skipfirstrows = (srcy < 0) ? -srcy : 0;
  const int srcp = src->pitch;
  const int dstp = dst->pitch;

  SDL_LockSurface(src);
  SDL_LockSurface(dst);
  unsigned char* src1px = ((unsigned char*)src->pixels) + srcx * 4 + srcy * srcp;
  unsigned char* src2px = ((unsigned char*)src->pixels) + clip.x * 4 + clip.y * srcp;
  unsigned char* dstpx = ((unsigned char*)dst->pixels) + clip.x * 4 + clip.y * dstp;

  // If the vertical offset is positive, we are reading one copy from (x, -1), so we need to
  // skip the first scanline to avoid reading outside the source surface.
  for (int i = skipfirstrows; i; --i) {
    --rows;
    src1px += srcp;
    src2px += srcp;
    dstpx += dstp;
  }

  // Blend the remaining scanlines.
  while (rows--) {
    imageFilterMean(src1px, src2px, dstpx, length);
    src1px += srcp;
    src2px += srcp;
    dstpx += dstp;
  }

  // If the horizontal offset is -1, the rightmost column has not been written to.
  // Rectify that by copying it directly from the source image.
  if (rx && (clip.x + clip.w >= width)) {
    Uint32* r = ((Uint32*)src->pixels) + (width - 1) + clip.y * width;
    Uint32* d = ((Uint32*)dst->pixels) + (width - 1) + clip.y * width;
    while (clip.h--) {
      *d = *r;
      d += width;
      r += width;
    }
  }

  SDL_UnlockSurface(src);
  SDL_UnlockSurface(dst);

  // If we skipped the first scanlines, rectify that by copying directly from the source image.
  if (skipfirstrows) {
    clip.h = skipfirstrows;
    SDL_BlitSurface(src, &clip, dst, &clip);
  }
}

inline static void imageFilterSubFrom(unsigned char *dst, unsigned char *src, int length) {
  int n = length + 1;
  while (--n > 0) {
    int result = (*dst) - (*src);
    (*dst) = (result > 0) ? result : 0;
    ++dst, ++src;
  }
}

inline static void imageFilterAddTo(unsigned char *dst, unsigned char *src, int length) {
  int n = length + 1;
  while (--n > 0) {
    int result = (*dst) + (*src);
    (*dst) = (result < 255) ? result : 255;
    ++dst, ++src;
  }
}

// Called every time the screen is refreshed.
// Draws the background image with the old-movie effect applied, using the settings adopted at the
// last call to updateOldMovie().
void OldMovieLayer::refresh(SDL_Surface *surface, SDL_Rect &clip)
{
  if (!initialized) return;

  // Blur background.
  // If no offset is applied, we can just copy the given surface directly.
  // If an offset is present, we average the given surface with an offset version

  if ((rx != 0) || (ry != 0)) {
    SDL_BlitSurface(surface, &clip, sprite->image_surface, &clip);
    BlurOnSurface(sprite->image_surface, surface, clip, rx, ry, width);
  }

  // Add noise and glow.
  SDL_LockSurface(surface);
  SDL_LockSurface(NoiseSurface[ns]);
  SDL_LockSurface(GlowSurface);
  unsigned char* g = (unsigned char*)GlowSurface->pixels + (gv * glow_level / 4) * GlowSurface->pitch;
  const int sp = surface->pitch;
  if ((clip.x == 0) && (clip.y == 0) && (clip.w == width) && (clip.h == height)) {
    // If no clipping rectangle is defined, we can apply the noise in one go.
    unsigned char* s = (unsigned char*)surface->pixels;
    if (noise_level > 0)
      imageFilterSubFrom(s, (unsigned char*)NoiseSurface[ns]->pixels, sp * surface->h);
    // Since the glow is stored as a single scanline for each level, we always apply
    // the glow scanline by scanline.
    if (glow_level > 0) {
      for (int i = height; i; --i, s += sp)
        imageFilterAddTo(s, g, width * 4);
    }
  } else {
    // Otherwise we do everything scanline by scanline.
    const int length = clip.w * 4;
    if (noise_level > 0) {
      const int np = NoiseSurface[ns]->pitch;
      unsigned char* s = ((unsigned char*)surface->pixels) + clip.x * 4 + clip.y * sp;
      unsigned char* n = ((unsigned char*)NoiseSurface[ns]->pixels) + clip.x * 4 + clip.y * np;
      for (int i = clip.h; i; --i, s += sp, n += np)
        imageFilterSubFrom(s, n, length); // subtract noise
    }
    if (glow_level > 0) {
      unsigned char* s = ((unsigned char*)surface->pixels) + clip.x * 4 + clip.y * sp;
      for (int i = clip.h; i; --i, s += sp)
        imageFilterAddTo(s, g, length); // add glow
    }
  }
  SDL_UnlockSurface(NoiseSurface[ns]);
  SDL_UnlockSurface(GlowSurface);

  // Add scratches.
  if (scratch_level > 0)
    for (int i = 0; i < MAX_SCRATCH_COUNT; i++)
      scratches[i].draw(surface, clip);

  // Add dust specks.
  if (dust && (dust_level > 0)) {
    for (int i = 0; i<MAX_DUST_COUNT; i++) {
      if ((rand() & 1023) < dust_level) {
        dust->current_cell = dust_pts[i].cell;
        dust->pos.x = dust_pts[i].x;
        dust->pos.y = dust_pts[i].y;
        drawTaggedSurface(surface, dust, clip);
      }
    }
  }

  // And we're done.
  SDL_UnlockSurface(surface);

}
#endif

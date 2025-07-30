#ifndef SDL_LIBRETO_H
#define SDL_LIBRETO_H

#include "SDL.h"
#include "libretro.h"

#ifdef __cplusplus
extern "C" {
#endif

void SDL_libretro_RefreshVideo(retro_video_refresh_t video_cb);
void SDL_libretro_ProduceAudio(retro_audio_sample_batch_t audio_batch_t);
void SDL_libretro_KeyboardCallback(bool down, unsigned keycode,
                                   uint32_t character, uint16_t key_modifiers);
void SDL_libretro_SendMouseMotion(int relative, int x, int y);
void SDL_libretro_SendMouseButton(Uint8 state, Uint8 button);

int SDL_SendKeyboardKey(Uint8 state, SDL_Scancode scancode);

#ifdef __cplusplus
}
#endif

#endif

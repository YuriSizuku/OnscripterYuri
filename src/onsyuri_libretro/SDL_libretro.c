#include "SDL_libretro.h"
#include "SDL_image.h"
#include "./deps/SDL/src/audio/SDL_sysaudio.h"
#include "./deps/SDL/src/events/SDL_keyboard_c.h"
#include "./deps/SDL/src/events/SDL_mouse_c.h"
#include "./deps/SDL/src/video/SDL_sysvideo.h"
#include "cursor_png.h"

static SDL_Surface* _cursor = NULL;
static SDL_Surface* _surface_fb = NULL;         // backbuffer
static SDL_Surface* _surface_ons = NULL;        // updated screen
static SDL_Surface* _surface_retro = NULL;      // screen with mouse cursor
static SDL_AudioDevice* _audio = NULL;
static SDL_VideoDevice* _video = NULL;

static int
VideoInit(SDL_VideoDevice* device)
{
    SDL_DisplayMode mode;
    SDL_zero(mode);
    mode.format = SDL_PIXELFORMAT_RGB888;
    mode.w = 1920;
    mode.h = 1080;
    mode.refresh_rate = 60;
    mode.driverdata = NULL;
    SDL_AddBasicVideoDisplay(&mode);

    _cursor = IMG_Load_RW(SDL_RWFromConstMem(cursor_png, cursor_png_len), 1);
    if (_cursor == NULL) {
        return -1;
    }
    _video = device;
    return 0;
}

static void
VideoQuit(SDL_VideoDevice* device)
{
}

static void
VideoFree(SDL_VideoDevice* device)
{
    SDL_free(device);
    SDL_FreeSurface(_cursor);
}

static void
PumpEvents(SDL_VideoDevice* device)
{
}

void
DestroyWindowFramebuffer(SDL_VideoDevice* device, SDL_Window* window)
{
    SDL_FreeSurface(_surface_fb);
    SDL_FreeSurface(_surface_ons);
    SDL_FreeSurface(_surface_retro);
}

int
CreateWindowFramebuffer(SDL_VideoDevice* device,
                        SDL_Window* window,
                        Uint32* format,
                        void** pixels,
                        int* pitch)
{
    const Uint32 surface_format = SDL_PIXELFORMAT_RGB888;
    int w, h;

    /* Free the old framebuffer surface */
    DestroyWindowFramebuffer(device, window);

    /* Create a new one */
    SDL_GetWindowSizeInPixels(window, &w, &h);
    _surface_fb = SDL_CreateRGBSurfaceWithFormat(0, w, h, 0, surface_format);
    _surface_ons = SDL_CreateRGBSurfaceWithFormat(0, w, h, 0, surface_format);
    _surface_retro = SDL_CreateRGBSurfaceWithFormat(0, w, h, 0, surface_format);
    if (_surface_fb == NULL || _surface_ons == NULL || _surface_retro == NULL) {
        return -1;
    }

    /* Save the info and return! */
    *format = surface_format;
    *pixels = _surface_fb->pixels;
    *pitch = _surface_fb->pitch;

    return 0;
}

int
UpdateWindowFramebuffer(SDL_VideoDevice* device,
                        SDL_Window* window,
                        const SDL_Rect* rects,
                        int numrects)
{
    for (int i = 0; i < numrects; i += 1) {
        SDL_BlitSurface(_surface_fb, (SDL_Rect*)&rects[i], _surface_ons, (SDL_Rect*)&rects[i]);
    }
    return 0;
}

static SDL_VideoDevice*
VideoCreate(void)
{
    SDL_VideoDevice* device = SDL_malloc(sizeof(SDL_VideoDevice));
    if (device == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }
    SDL_zerop(device);
    device->is_dummy = SDL_FALSE;
    device->VideoInit = VideoInit;
    device->VideoQuit = VideoQuit;
    device->PumpEvents = PumpEvents;
    device->CreateWindowFramebuffer = CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = DestroyWindowFramebuffer;
    device->free = VideoFree;
    return device;
}

VideoBootStrap DUMMY_bootstrap = {
    "libretro",
    "SDL libretro video driver",
    VideoCreate,
    NULL,
};

static int
AudioOpen(SDL_AudioDevice* device, const char* devname)
{
    device->hidden = (void*)0x1;
    device->spec.freq = 44100;
    device->spec.format = AUDIO_S16SYS;
    device->spec.channels = 2;
    device->spec.silence = 0;
    device->spec.samples = 44100 / 60;
    SDL_CalculateAudioSpec(&device->spec);
    _audio = device;
    return 0;
}

static SDL_bool
AudioInit(SDL_AudioDriverImpl* impl)
{
    impl->OpenDevice = AudioOpen;
    impl->HasCaptureSupport = SDL_FALSE;
    impl->AllowsArbitraryDeviceNames = SDL_TRUE;
    impl->ProvidesOwnCallbackThread = SDL_TRUE;
    impl->OnlyHasDefaultOutputDevice = SDL_TRUE;
    impl->SupportsNonPow2Samples = SDL_TRUE;
    return SDL_TRUE;
}

AudioBootStrap DUMMYAUDIO_bootstrap = { "libretro",
                                        "SDL libretro audio driver",
                                        AudioInit,
                                        SDL_FALSE };

void
SDL_libretro_RefreshVideo(retro_video_refresh_t video_cb)
{
    if (_surface_retro == NULL)
        return;

    SDL_Mouse *mouse = SDL_GetMouse();

    if (mouse->cursor_shown) {
        SDL_Rect mouse_rect = { mouse->x, mouse->y, _cursor->w, _cursor->h };
        SDL_BlitSurface(_surface_ons, NULL, _surface_retro, NULL);
        SDL_BlitSurface(_cursor, NULL, _surface_retro, &mouse_rect);

        video_cb(_surface_retro->pixels,
                 _surface_retro->w,
                 _surface_retro->h,
                 _surface_retro->pitch);
    } else {
        video_cb(_surface_ons->pixels,
                 _surface_ons->w,
                 _surface_ons->h,
                 _surface_ons->pitch);
    }
}

void
SDL_libretro_ProduceAudio(retro_audio_sample_batch_t audio_batch_cb)
{
    if (_audio == NULL)
        return;
    _audio->spec.callback(_audio, _audio->work_buffer, _audio->spec.size);
    audio_batch_cb((const int16_t*)_audio->work_buffer, _audio->spec.samples);
}

void
SDL_libretro_KeyboardCallback(bool down,
                              unsigned keycode,
                              uint32_t character,
                              uint16_t key_modifiers)
{
    static const SDL_Keycode keycodes[RETROK_LAST] = {
        [RETROK_UNKNOWN] = SDLK_UNKNOWN,
        [RETROK_BACKSPACE] = SDLK_BACKSPACE,
        [RETROK_TAB] = SDLK_TAB,
        [RETROK_CLEAR] = SDLK_CLEAR,
        [RETROK_RETURN] = SDLK_RETURN,
        [RETROK_PAUSE] = SDLK_PAUSE,
        [RETROK_ESCAPE] = SDLK_ESCAPE,
        [RETROK_SPACE] = SDLK_SPACE,
        [RETROK_EXCLAIM] = SDLK_EXCLAIM,
        [RETROK_QUOTEDBL] = SDLK_QUOTEDBL,
        [RETROK_HASH] = SDLK_HASH,
        [RETROK_DOLLAR] = SDLK_DOLLAR,
        [RETROK_AMPERSAND] = SDLK_AMPERSAND,
        [RETROK_QUOTE] = SDLK_QUOTE,
        [RETROK_LEFTPAREN] = SDLK_LEFTPAREN,
        [RETROK_RIGHTPAREN] = SDLK_RIGHTPAREN,
        [RETROK_ASTERISK] = SDLK_ASTERISK,
        [RETROK_PLUS] = SDLK_PLUS,
        [RETROK_COMMA] = SDLK_COMMA,
        [RETROK_MINUS] = SDLK_MINUS,
        [RETROK_PERIOD] = SDLK_PERIOD,
        [RETROK_SLASH] = SDLK_SLASH,
        [RETROK_0] = SDLK_0,
        [RETROK_1] = SDLK_1,
        [RETROK_2] = SDLK_2,
        [RETROK_3] = SDLK_3,
        [RETROK_4] = SDLK_4,
        [RETROK_5] = SDLK_5,
        [RETROK_6] = SDLK_6,
        [RETROK_7] = SDLK_7,
        [RETROK_8] = SDLK_8,
        [RETROK_9] = SDLK_9,
        [RETROK_COLON] = SDLK_COLON,
        [RETROK_SEMICOLON] = SDLK_SEMICOLON,
        [RETROK_LESS] = SDLK_LESS,
        [RETROK_EQUALS] = SDLK_EQUALS,
        [RETROK_GREATER] = SDLK_GREATER,
        [RETROK_QUESTION] = SDLK_QUESTION,
        [RETROK_AT] = SDLK_AT,
        [RETROK_LEFTBRACKET] = SDLK_LEFTBRACKET,
        [RETROK_BACKSLASH] = SDLK_BACKSLASH,
        [RETROK_RIGHTBRACKET] = SDLK_RIGHTBRACKET,
        [RETROK_CARET] = SDLK_CARET,
        [RETROK_UNDERSCORE] = SDLK_UNDERSCORE,
        [RETROK_BACKQUOTE] = SDLK_BACKQUOTE,
        [RETROK_a] = SDLK_a,
        [RETROK_b] = SDLK_b,
        [RETROK_c] = SDLK_c,
        [RETROK_d] = SDLK_d,
        [RETROK_e] = SDLK_e,
        [RETROK_f] = SDLK_f,
        [RETROK_g] = SDLK_g,
        [RETROK_h] = SDLK_h,
        [RETROK_i] = SDLK_i,
        [RETROK_j] = SDLK_j,
        [RETROK_k] = SDLK_k,
        [RETROK_l] = SDLK_l,
        [RETROK_m] = SDLK_m,
        [RETROK_n] = SDLK_n,
        [RETROK_o] = SDLK_o,
        [RETROK_p] = SDLK_p,
        [RETROK_q] = SDLK_q,
        [RETROK_r] = SDLK_r,
        [RETROK_s] = SDLK_s,
        [RETROK_t] = SDLK_t,
        [RETROK_u] = SDLK_u,
        [RETROK_v] = SDLK_v,
        [RETROK_w] = SDLK_w,
        [RETROK_x] = SDLK_x,
        [RETROK_y] = SDLK_y,
        [RETROK_z] = SDLK_z,
        [RETROK_DELETE] = SDLK_DELETE,
        [RETROK_KP0] = SDLK_KP_0,
        [RETROK_KP1] = SDLK_KP_1,
        [RETROK_KP2] = SDLK_KP_2,
        [RETROK_KP3] = SDLK_KP_3,
        [RETROK_KP4] = SDLK_KP_4,
        [RETROK_KP5] = SDLK_KP_5,
        [RETROK_KP6] = SDLK_KP_6,
        [RETROK_KP7] = SDLK_KP_7,
        [RETROK_KP8] = SDLK_KP_8,
        [RETROK_KP9] = SDLK_KP_9,
        [RETROK_KP_PERIOD] = SDLK_KP_PERIOD,
        [RETROK_KP_DIVIDE] = SDLK_KP_DIVIDE,
        [RETROK_KP_MULTIPLY] = SDLK_KP_MULTIPLY,
        [RETROK_KP_MINUS] = SDLK_KP_MINUS,
        [RETROK_KP_PLUS] = SDLK_KP_PLUS,
        [RETROK_KP_ENTER] = SDLK_KP_ENTER,
        [RETROK_KP_EQUALS] = SDLK_KP_EQUALS,
        [RETROK_UP] = SDLK_UP,
        [RETROK_DOWN] = SDLK_DOWN,
        [RETROK_RIGHT] = SDLK_RIGHT,
        [RETROK_LEFT] = SDLK_LEFT,
        [RETROK_INSERT] = SDLK_INSERT,
        [RETROK_HOME] = SDLK_HOME,
        [RETROK_END] = SDLK_END,
        [RETROK_PAGEUP] = SDLK_PAGEUP,
        [RETROK_PAGEDOWN] = SDLK_PAGEDOWN,
        [RETROK_F1] = SDLK_F1,
        [RETROK_F2] = SDLK_F2,
        [RETROK_F3] = SDLK_F3,
        [RETROK_F4] = SDLK_F4,
        [RETROK_F5] = SDLK_F5,
        [RETROK_F6] = SDLK_F6,
        [RETROK_F7] = SDLK_F7,
        [RETROK_F8] = SDLK_F8,
        [RETROK_F9] = SDLK_F9,
        [RETROK_F10] = SDLK_F10,
        [RETROK_F11] = SDLK_F11,
        [RETROK_F12] = SDLK_F12,
        [RETROK_F13] = SDLK_F13,
        [RETROK_F14] = SDLK_F14,
        [RETROK_F15] = SDLK_F15,
        [RETROK_NUMLOCK] = SDLK_NUMLOCKCLEAR,
        [RETROK_CAPSLOCK] = SDLK_CAPSLOCK,
        [RETROK_SCROLLOCK] = SDLK_SCROLLLOCK,
        [RETROK_RSHIFT] = SDLK_RSHIFT,
        [RETROK_LSHIFT] = SDLK_LSHIFT,
        [RETROK_RCTRL] = SDLK_RCTRL,
        [RETROK_LCTRL] = SDLK_LCTRL,
        [RETROK_RALT] = SDLK_RALT,
        [RETROK_LALT] = SDLK_LALT,
        [RETROK_LSUPER] = SDLK_LGUI,
        [RETROK_RSUPER] = SDLK_RGUI,
        [RETROK_MODE] = SDLK_MODE,
        [RETROK_HELP] = SDLK_HELP,
        [RETROK_PRINT] = SDLK_PRINTSCREEN,
        [RETROK_SYSREQ] = SDLK_SYSREQ,
        [RETROK_BREAK] = SDLK_PAUSE,
        [RETROK_MENU] = SDLK_MENU,
        [RETROK_POWER] = SDLK_POWER,
        [RETROK_UNDO] = SDLK_UNDO,
        [RETROK_OEM_102] = SDLK_LESS,
    };
    SDL_Scancode scancode = SDL_GetScancodeFromKey(keycodes[keycode]);
    SDL_SendKeyboardKey(down ? SDL_PRESSED : SDL_RELEASED, scancode);
}

void
SDL_libretro_SendMouseMotion(int relative, int x, int y)
{
    SDL_SendMouseMotion(_video->grabbed_window, 0, relative, x, y);
}

void
SDL_libretro_SendMouseButton(Uint8 state, Uint8 button)
{
    SDL_SendMouseButton(_video->grabbed_window, 0, state, button);
}

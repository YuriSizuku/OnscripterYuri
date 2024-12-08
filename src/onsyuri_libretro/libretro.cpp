#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string>

#ifdef __ANDROID__
#define DEFAULT_MOUSE_MODE "Touch"
#else
#define DEFAULT_MOUSE_MODE "Classical"
#endif

#include "ONScripter.h"
#include "SDL_libretro.h"
#include "gbk2utf16.h"
#include "sjis2utf16.h"

static void fallback_log(enum retro_log_level level, const char* fmt, ...);
static retro_log_printf_t log_cb = fallback_log;
static retro_environment_t environ_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_audio_sample_batch_t audio_batch_cb;

static SDL_Thread* game_thread;
static bool classical_mouse = false;
static double mouse_sensitivity = 1.0;

ONScripter ons;
Coding2UTF16* coding2utf16 = NULL;
std::string g_stdoutpath = "stdout.txt";
std::string g_stderrpath = "stderr.txt";

static void
fallback_log(enum retro_log_level level, const char* fmt, ...)
{
    (void)level;
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

unsigned
retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void
retro_set_environment(retro_environment_t cb)
{
    static struct retro_log_callback log;

    static struct retro_core_option_definition opts[] = {
        {
            .key = "onsyuri_script_encoding",
            .desc = "Script Encoding",
            .info = NULL,
            .values = { { "GBK" }, { "SHIFTJIS" }, { NULL } },
            .default_value = "GBK",
        },
        {
            .key = "onsyuri_mouse_mode",
            .desc = "Mouse Mode",
            .info = NULL,
            .values = { { "Touch" }, { "Classical" }, { NULL } },
            .default_value = DEFAULT_MOUSE_MODE,
        },
        {
            .key = "onsyuri_mouse_sensitivity",
            .desc = "Classical Mouse Sensitivity",
            .info = NULL,
            .values = {
                { "0.4" }, { "0.6" }, { "0.8" }, { "1.0" }, { "1.2" }, { "1.4" },
                { "1.6" }, { "1.8" }, { "2.0" }, { "2.2" }, { "2.4" }, { "2.6 "},
            },
            .default_value = "1.0",
        },
        NULL,
    };

    environ_cb = cb;
    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
        log_cb = log.log;

    unsigned opts_ver = 0;
    environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &opts_ver);
    if (opts_ver >= 1) {
        environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, &opts);
    }
}

void
retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

void
retro_set_audio_sample(retro_audio_sample_t cb)
{
}

void
retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void
retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void
retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void
retro_get_system_info(struct retro_system_info* info)
{
    info->need_fullpath = true;
    info->valid_extensions = "txt|dat|___|nt2|nt3|ons|/";
    info->library_version = "0.7.4+2";
    info->library_name = "onsyuri";
    info->block_extract = false;
}

void
retro_get_system_av_info(struct retro_system_av_info* info)
{
    int width = ons.getWidth();
    int height = ons.getHeight();
    info->geometry.base_width = width;
    info->geometry.base_height = height;
    info->geometry.max_width = width;
    info->geometry.max_height = height;
    info->geometry.aspect_ratio = 0.0;
    info->timing.fps = 60.0;
    info->timing.sample_rate = 44100.0;
}

void
retro_init(void)
{
    enum retro_pixel_format pixfmt = RETRO_PIXEL_FORMAT_XRGB8888;
    environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixfmt);

    struct retro_variable var = { 0 };
    var.key = "onsyuri_script_encoding";
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var)) {
        if (strcmp(var.value, "SHIFTJIS") == 0) {
            coding2utf16 = new SJIS2UTF16();
        } else {
            coding2utf16 = new GBK2UTF16();
        }
    }
    var.key = "onsyuri_mouse_mode";
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var)) {
        if (strcmp(var.value, "Classical") == 0) {
            classical_mouse = true;
        } else {
            classical_mouse = false;
        }
    }
    var.key = "onsyuri_mouse_sensitivity";
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var)) {
        mouse_sensitivity = SDL_atof(var.value);
    }
}

static int
game_main(void* data)
{
    ons.executeLabel();
    return 0;
}

bool
retro_load_game(const struct retro_game_info* game)
{
    if (!game)
        return false;

    if (game->path[SDL_strlen(game->path) - 1] == '/') {
        chdir(game->path);
    } else {
        char* gamedir = dirname(SDL_strdup(game->path));
        chdir(gamedir);
    }

    // Ignore SDL_AUDIODRIVER and SDL_VIDEODRIVER.
    SDL_SetHintWithPriority(SDL_HINT_AUDIODRIVER, "libretro", SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_VIDEODRIVER, "libretro", SDL_HINT_OVERRIDE);

    if (ons.openScript() != 0)
        return false;

    if (ons.init() != 0) {
        log_cb(RETRO_LOG_ERROR, "Failed to initialize ONScripter.\n");
        return false;
    }

    SDL_CaptureMouse(SDL_TRUE);
    SDL_ShowCursor(classical_mouse ? SDL_ENABLE : SDL_DISABLE);

    struct retro_keyboard_callback keyboard = {
        .callback = SDL_libretro_KeyboardCallback,
    };
    environ_cb(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &keyboard);

    game_thread = SDL_CreateThread(game_main, "onsyuri", NULL);
    return true;
}

void
retro_set_controller_port_device(unsigned port, unsigned device)
{
}

void
retro_deinit(void)
{
}

void
retro_reset(void)
{
}

static void
PumpJoypadEvents(void)
{
    static int16_t buttons[16] = { 0 };
    static const int bkeys[16] = {
        [RETRO_DEVICE_ID_JOYPAD_B] = SDLK_SPACE,
        [RETRO_DEVICE_ID_JOYPAD_Y] = SDLK_RCTRL,
        [RETRO_DEVICE_ID_JOYPAD_SELECT] = SDLK_0,
        [RETRO_DEVICE_ID_JOYPAD_START] = SDLK_a,
        [RETRO_DEVICE_ID_JOYPAD_UP] = SDLK_UP,
        [RETRO_DEVICE_ID_JOYPAD_DOWN] = SDLK_DOWN,
        [RETRO_DEVICE_ID_JOYPAD_LEFT] = SDLK_LEFT,
        [RETRO_DEVICE_ID_JOYPAD_RIGHT] = SDLK_RIGHT,
        [RETRO_DEVICE_ID_JOYPAD_A] = SDLK_RETURN,
        [RETRO_DEVICE_ID_JOYPAD_X] = SDLK_ESCAPE,
        [RETRO_DEVICE_ID_JOYPAD_L] = SDLK_o,
        [RETRO_DEVICE_ID_JOYPAD_R] = SDLK_s,
        [RETRO_DEVICE_ID_JOYPAD_L2] = SDLK_PAGEUP,
        [RETRO_DEVICE_ID_JOYPAD_R2] = SDLK_PAGEDOWN,
        [RETRO_DEVICE_ID_JOYPAD_L3] = SDLK_TAB,
        [RETRO_DEVICE_ID_JOYPAD_R3] = SDLK_q,
    };
    for (int i = 0; i < 16; ++i) {
        int16_t state = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i);
        int k = bkeys[i];
        if (buttons[i] != state) {
            buttons[i] = state;
            SDL_SendKeyboardKey(state ? SDL_PRESSED : SDL_RELEASED,
                                SDL_GetScancodeFromKey(k));
        }
    }
}

static Uint8
pressed_to_button(int16_t pressed)
{
    if (pressed == 1)
        return SDL_BUTTON_LEFT;
    if (pressed == 2)
        return SDL_BUTTON_RIGHT;
    return 0;
}

static void
PumpMouseEvents(void)
{
#define MOUSE(X) input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_##X)
#define POINTER(X) input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_##X)
    if (classical_mouse) {
        static int16_t _left = 0;
        static int16_t _right = 0;
        int16_t dx     = MOUSE(X);
        int16_t dy     = MOUSE(Y);
        int16_t left  = MOUSE(LEFT);
        int16_t right = MOUSE(RIGHT);

        if (dx != 0 || dy != 0) {
            SDL_libretro_SendMouseMotion(1, dx * mouse_sensitivity, dy * mouse_sensitivity);
        }
        if (left != _left) {
            SDL_libretro_SendMouseButton(left ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_LEFT);
            _left = left;
        }
        if (right != _right) {
            SDL_libretro_SendMouseButton(right ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_RIGHT);
            _right = right;
        }

        // Keep mouse within the window.
        int x, y;
        SDL_GetMouseState(&x, &y);
        SDL_WarpMouseInWindow(NULL, x, y);
    } else {
        static int16_t _x = 0;
        static int16_t _y = 0;
        static int16_t _pressed = 0;

        int width = ons.getWidth();
        int height = ons.getHeight();
        int16_t x = POINTER(X);
        int16_t y = POINTER(Y);
        int16_t pressed = 0;
        while (input_state_cb(0, RETRO_DEVICE_POINTER, pressed,
                              RETRO_DEVICE_ID_POINTER_PRESSED))
            pressed += 1;
        if (MOUSE(LEFT))
            pressed = 1;
        if (MOUSE(RIGHT))
            pressed = 2;

        x = width * (x + 0x7fff) / 0xffff;
        y = height * (y + 0x7fff) / 0xffff;
        if (x != _x || y != _y) {
            SDL_libretro_SendMouseMotion(0, x, y);
            _x = x;
            _y = y;
        }
        if (pressed != _pressed) {
            if (_pressed)
                SDL_libretro_SendMouseButton(SDL_RELEASED, pressed_to_button(_pressed));
            else
                SDL_libretro_SendMouseButton(SDL_PRESSED, pressed_to_button(pressed));
            _pressed = pressed;
        }
    }
#undef MOUSE
#undef POINTER
}

void
retro_run(void)
{
    input_poll_cb();
    PumpJoypadEvents();
    PumpMouseEvents();

    SDL_libretro_RefreshVideo(video_cb);
    SDL_libretro_ProduceAudio(audio_batch_cb);
}

size_t
retro_serialize_size(void)
{
    return 0;
}

bool
retro_serialize(void* data, size_t size)
{
    return false;
}

bool
retro_unserialize(const void* data, size_t size)
{
    return false;
}

void
retro_cheat_reset(void)
{
}
void
retro_cheat_set(unsigned index, bool enabled, const char* code)
{
}

bool
retro_load_game_special(unsigned game_type,
                        const struct retro_game_info* info,
                        size_t num_info)
{
    return false;
}

void
retro_unload_game(void)
{
    SDL_Event event = { SDL_QUIT };
    SDL_PushEvent(&event);
    SDL_WaitThread(game_thread, NULL);
}

unsigned
retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

void*
retro_get_memory_data(unsigned id)
{
    return NULL;
}

size_t
retro_get_memory_size(unsigned id)
{
    return 0;
}

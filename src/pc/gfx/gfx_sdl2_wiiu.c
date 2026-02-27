#ifdef WAPI_WIIU

#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "gfx_window_manager_api.h"
#include "gfx_screen_config.h"

static SDL_Window *sWindow;
static uint32_t sWindowWidth = DESIRED_SCREEN_WIDTH;
static uint32_t sWindowHeight = DESIRED_SCREEN_HEIGHT;

static kb_callback_t sKbKeyDown;
static kb_callback_t sKbKeyUp;
static void (*sKbAllKeysUp)(void);
static void (*sKbTextInput)(char*);
static void (*sKbTextEditing)(char*, int);
static void (*sScrollCallback)(float, float);

static void gfx_wiiu_init(const char *window_title) {
    SDL_Init(SDL_INIT_VIDEO);

    sWindow = SDL_CreateWindow(
        window_title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        0, 0,
        SDL_WINDOW_FULLSCREEN_DESKTOP
    );

    if (sWindow) {
        SDL_DisplayMode mode;
        if (SDL_GetDesktopDisplayMode(0, &mode) == 0) {
            sWindowWidth = (uint32_t)mode.w;
            sWindowHeight = (uint32_t)mode.h;
        }
    }
}

static void gfx_wiiu_set_keyboard_callbacks(kb_callback_t on_key_down, kb_callback_t on_key_up,
                                           void (*on_all_keys_up)(void), void (*on_text_input)(char*),
                                           void (*on_text_editing)(char*, int)) {
    sKbKeyDown = on_key_down;
    sKbKeyUp = on_key_up;
    sKbAllKeysUp = on_all_keys_up;
    sKbTextInput = on_text_input;
    sKbTextEditing = on_text_editing;
}

static void gfx_wiiu_set_scroll_callback(void (*on_scroll)(float, float)) {
    sScrollCallback = on_scroll;
}

static void gfx_wiiu_main_loop(void (*run_one_game_iter)(void)) {
    run_one_game_iter();
}

static void gfx_wiiu_get_dimensions(uint32_t *width, uint32_t *height) {
    *width = sWindowWidth;
    *height = sWindowHeight;
}

static void gfx_wiiu_handle_events(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                if (sWindow) {
                    SDL_DestroyWindow(sWindow);
                    sWindow = NULL;
                }
                break;
            case SDL_KEYDOWN:
                if (sKbKeyDown) {
                    sKbKeyDown((int)event.key.keysym.scancode);
                }
                break;
            case SDL_KEYUP:
                if (sKbKeyUp) {
                    sKbKeyUp((int)event.key.keysym.scancode);
                }
                break;
            case SDL_TEXTINPUT:
                if (sKbTextInput) {
                    sKbTextInput(event.text.text);
                }
                break;
            case SDL_TEXTEDITING:
                if (sKbTextEditing) {
                    sKbTextEditing(event.edit.text, event.edit.start);
                }
                break;
            case SDL_MOUSEWHEEL:
                if (sScrollCallback) {
                    sScrollCallback((float)event.wheel.x, (float)event.wheel.y);
                }
                break;
            default:
                break;
        }
    }

    if (sKbAllKeysUp) {
        sKbAllKeysUp();
    }
}

static bool gfx_wiiu_start_frame(void) {
    return true;
}

static void gfx_wiiu_swap_buffers_begin(void) {
}

static void gfx_wiiu_swap_buffers_end(void) {
}

static double gfx_wiiu_get_time(void) {
    return 0.0;
}

static void gfx_wiiu_shutdown(void) {
    if (sWindow) {
        SDL_DestroyWindow(sWindow);
        sWindow = NULL;
    }
    SDL_Quit();
}

static void gfx_wiiu_start_text_input(void) {
    SDL_StartTextInput();
}

static void gfx_wiiu_stop_text_input(void) {
    SDL_StopTextInput();
}

static char* gfx_wiiu_get_clipboard_text(void) {
    return SDL_GetClipboardText();
}

static void gfx_wiiu_set_clipboard_text(const char* text) {
    SDL_SetClipboardText(text);
}

static void gfx_wiiu_set_cursor_visible(bool visible) {
    SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

static void gfx_wiiu_delay(unsigned int ms) {
    SDL_Delay((Uint32)ms);
}

static int gfx_wiiu_get_max_msaa(void) {
    return 0;
}

static void gfx_wiiu_set_window_title(const char* title) {
    if (sWindow) {
        SDL_SetWindowTitle(sWindow, title);
    }
}

static void gfx_wiiu_reset_window_title(void) {
}

static bool gfx_wiiu_has_focus(void) {
    return true;
}

struct GfxWindowManagerAPI gfx_wiiu = {
    gfx_wiiu_init,
    gfx_wiiu_set_keyboard_callbacks,
    gfx_wiiu_set_scroll_callback,
    gfx_wiiu_main_loop,
    gfx_wiiu_get_dimensions,
    gfx_wiiu_handle_events,
    gfx_wiiu_start_frame,
    gfx_wiiu_swap_buffers_begin,
    gfx_wiiu_swap_buffers_end,
    gfx_wiiu_get_time,
    gfx_wiiu_shutdown,
    gfx_wiiu_start_text_input,
    gfx_wiiu_stop_text_input,
    gfx_wiiu_get_clipboard_text,
    gfx_wiiu_set_clipboard_text,
    gfx_wiiu_set_cursor_visible,
    gfx_wiiu_delay,
    gfx_wiiu_get_max_msaa,
    gfx_wiiu_set_window_title,
    gfx_wiiu_reset_window_title,
    gfx_wiiu_has_focus,
};

#endif

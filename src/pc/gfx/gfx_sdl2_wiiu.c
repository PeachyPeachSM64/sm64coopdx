#ifdef WAPI_WIIU

#include <stdbool.h>
#include <stdint.h>

#include <coreinit/foreground.h>
#include <gx2/event.h>
#include <gx2/swap.h>
#include <macros.h>
#include <proc_ui/procui.h>
#include <whb/gfx.h>

#include "gfx_window_manager_api.h"
#include "gfx_screen_config.h"

static uint32_t sWindowWidth = DESIRED_SCREEN_WIDTH;
static uint32_t sWindowHeight = DESIRED_SCREEN_HEIGHT;
static bool sIsRunning;

static void gfx_wiiu_proc_ui_save_callback(void) {
    OSSavesDone_ReadyToRelease();
}

static uint32_t gfx_wiiu_proc_ui_exit_callback(UNUSED void* data) {
    sIsRunning = false;
    return 0;
}

static void gfx_wiiu_init(const char *window_title) {
    (void)window_title;

    ProcUIInit(&gfx_wiiu_proc_ui_save_callback);
    ProcUIRegisterCallback(PROCUI_CALLBACK_EXIT, &gfx_wiiu_proc_ui_exit_callback, NULL, 0);
    sIsRunning = true;

    GX2SetSwapInterval(2);
}

static void gfx_wiiu_set_keyboard_callbacks(kb_callback_t on_key_down, kb_callback_t on_key_up,
                                           void (*on_all_keys_up)(void), void (*on_text_input)(char*),
                                           void (*on_text_editing)(char*, int)) {
    (void)on_key_down;
    (void)on_key_up;
    (void)on_all_keys_up;
    (void)on_text_input;
    (void)on_text_editing;
}

static void gfx_wiiu_set_scroll_callback(void (*on_scroll)(float, float)) {
    (void)on_scroll;
}

static void gfx_wiiu_main_loop(void (*run_one_game_iter)(void)) {
    // Ensure we run at 30FPS
    // Fool-proof unless the Wii U is able to
    // execute `run_one_game_iter()` so fast
    // that it doesn't even stall for enough time for
    // the second `GX2WaitForVsync()` to register
    GX2WaitForVsync();
    run_one_game_iter();
    GX2WaitForVsync();
}

static void gfx_wiiu_get_dimensions(uint32_t *width, uint32_t *height) {
    *width = sWindowWidth;
    *height = sWindowHeight;
}

static void gfx_wiiu_handle_events(void) {
    if (!sIsRunning) {
        return;
    }

    ProcUIStatus status = ProcUIProcessMessages(true);
    switch (status) {
        case PROCUI_STATUS_EXITING:
            ProcUIShutdown();
            sIsRunning = false;
            break;
        case PROCUI_STATUS_RELEASE_FOREGROUND:
            ProcUIDrawDoneRelease();
            break;
        case PROCUI_STATUS_IN_BACKGROUND:
        case PROCUI_STATUS_IN_FOREGROUND:
            break;
    }
}

static bool gfx_wiiu_start_frame(void) {
    if (!sIsRunning) {
        return false;
    }
    GX2WaitForFlip();
    return true;
}

static void gfx_wiiu_swap_buffers_begin(void) {
    WHBGfxFinishRender();
}

static void gfx_wiiu_swap_buffers_end(void) {
}

static double gfx_wiiu_get_time(void) {
    return 0.0;
}

static void gfx_wiiu_shutdown(void) {
    if (sIsRunning) {
        ProcUIShutdown();
    }
    sIsRunning = false;
}

static void gfx_wiiu_start_text_input(void) {
}

static void gfx_wiiu_stop_text_input(void) {
}

static char* gfx_wiiu_get_clipboard_text(void) {
    return NULL;
}

static void gfx_wiiu_set_clipboard_text(const char* text) {
    (void)text;
}

static void gfx_wiiu_set_cursor_visible(bool visible) {
    (void)visible;
}

static void gfx_wiiu_delay(unsigned int ms) {
    (void)ms;
}

static int gfx_wiiu_get_max_msaa(void) {
    return 0;
}

static void gfx_wiiu_set_window_title(const char* title) {
    (void)title;
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

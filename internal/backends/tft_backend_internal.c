/*
Copyright (c) 2024 Yassine Ahmed Ali

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "TFT_eSPI.h"
#include <stdint.h>
#include "event/gooey_event_internal.h"
#include "logger/pico_logger_internal.h"

typedef struct {
    TFT_eSPI *tft;
    size_t active_window_count;
    bool inhibit_reset;
    unsigned int selected_color;
    uint16_t *palette;
    size_t palette_size;
} GooeyBackendContext;

static GooeyBackendContext ctx = {0};

void tft_setup_shared() {
    ctx.tft = new TFT_eSPI();
    ctx.tft->init();
    ctx.tft->setRotation(1);
    ctx.tft->fillScreen(TFT_BLACK);
}

void tft_setup_seperate_vao(int window_id) {
    // Not needed for TFT_eSPI
}

void tft_draw_rectangle(int x, int y, int width, int height,
                       long unsigned int color, float thickness,
                       int window_id, bool isRounded, float cornerRadius) {
    if (isRounded) {
        ctx.tft->drawRoundRect(x, y, width, height, cornerRadius, color);
    } else {
        ctx.tft->drawRect(x, y, width, height, color);
    }
}

void tft_set_foreground(long unsigned int color) {
    ctx.selected_color = color;
}

void tft_window_dim(int *width, int *height, int window_id) {
    *width = ctx.tft->width();
    *height = ctx.tft->height();
}

void tft_draw_line(int x1, int y1, int x2, int y2, long unsigned int color, int window_id) {
    ctx.tft->drawLine(x1, y1, x2, y2, color);
}

void tft_fill_arc(int x_center, int y_center, int width, int height,
                 int angle1, int angle2, int window_id) {
    // TFT_eSPI doesn't have direct arc support, implement using lines
    const int segments = 36;
    int radius = min(width, height) / 2;
    int prev_x = x_center + radius * cos(angle1 * DEG_TO_RAD);
    int prev_y = y_center + radius * sin(angle1 * DEG_TO_RAD);

    for (int i = 1; i <= segments; i++) {
        float angle = angle1 + (angle2 - angle1) * i / (float)segments;
        int x = x_center + radius * cos(angle * DEG_TO_RAD);
        int y = y_center + radius * sin(angle * DEG_TO_RAD);
        ctx.tft->drawLine(prev_x, prev_y, x_center, y_center, ctx.selected_color);
        ctx.tft->drawLine(prev_x, prev_y, x, y, ctx.selected_color);
        prev_x = x;
        prev_y = y;
    }
}

void tft_draw_image(unsigned int texture_id, int x, int y, int width, int height, int window_id) {
    // TFT_eSPI doesn't support texture IDs directly
    // You would need to implement your own image handling
}

void tft_fill_rectangle(int x, int y, int width, int height,
                       long unsigned int color, int window_id,
                       bool isRounded, float cornerRadius) {
    if (isRounded) {
        ctx.tft->fillRoundRect(x, y, width, height, cornerRadius, color);
    } else {
        ctx.tft->fillRect(x, y, width, height, color);
    }
}

void tft_set_projection(int window_id, int width, int height) {
    // Not needed for TFT_eSPI
}

static void keyboard_callback(size_t window_id, bool state, const char *value, unsigned long keycode,
                            void *data) {
    // Implement if you have keyboard input
}

static void mouse_click_callback(size_t window_id, bool state, void *data) {
    // Implement if you have touch input
}

void tft_request_redraw(GooeyWindow *win) {
    // Implement if needed
}

static void mouse_move_callback(size_t window_id, double posX, double posY, void *data) {
    // Implement if you have touch input
}

static void window_resize_callback(size_t window_id, int width, int height, void *data) {
    // Not applicable for TFT displays
}

static void window_close_callback(size_t window_id, void *data) {
    // Not applicable for TFT displays
}

int tft_init_ft() {
    // Font initialization for TFT_eSPI
    ctx.tft->setTextFont(1);
    ctx.tft->setTextColor(TFT_WHITE, TFT_BLACK);
    return 0;
}

int tft_init() {
    ctx.inhibit_reset = false;
    ctx.selected_color = TFT_WHITE;
    ctx.active_window_count = 1; // Only one screen for TFT_eSPI
    tft_setup_shared();
    return 0;
}

int tft_get_current_clicked_window(void) {
    return 0; // Only one window/screen
}

void tft_draw_text(int x, int y, const char *text, unsigned long color, float font_size, int window_id) {
    ctx.tft->setTextColor(color);
    ctx.tft->setCursor(x, y);
    ctx.tft->print(text);
}

void tft_unload_image(unsigned int texture_id) {
    // Implement if you have image support
}

unsigned int tft_load_image(const char *image_path) {
    // Implement if you have image support
    return 0;
}

unsigned int tft_load_image_from_bin(unsigned char *data, long unsigned binary_len) {
    // Implement if you have image support
    return 0;
}

GooeyWindow *tft_create_window(const char *title, int width, int height) {
    GooeyWindow *window = malloc(sizeof(GooeyWindow));
    window->creation_id = 0; // Only one screen
    ctx.active_window_count++;
    return window;
}

void tft_make_window_visible(int window_id, bool visibility) {
    // Not applicable
}

void tft_set_window_resizable(bool value, int window_id) {
    // Not applicable
}

void tft_hide_current_child(void) {
    // Not applicable
}

void tft_destroy_windows() {
    // Not applicable
}

void tft_clear(GooeyWindow *win) {
    ctx.tft->fillScreen(win->active_theme->base);
}

void tft_cleanup() {
    if (ctx.tft) {
        delete ctx.tft;
        ctx.tft = nullptr;
    }
}

void tft_update_background(GooeyWindow *win) {
    ctx.tft->fillScreen(win->active_theme->base);
}

void tft_render(GooeyWindow *win) {
    // TFT_eSPI renders immediately, no double buffering
}

float tft_get_text_width(const char *text, int length) {
    return ctx.tft->textWidth(text, length);
}

float tft_get_text_height(const char *text, int length) {
    return ctx.tft->fontHeight();
}

const char *tft_get_key_from_code(void *gooey_event) {
    // Implement if you have keyboard input
    return NULL;
}

void tft_set_cursor(GOOEY_CURSOR cursor) {
    // Not applicable
}

void tft_stop_cursor_reset(bool state) {
    ctx.inhibit_reset = state;
}

void tft_destroy_window_from_id(int window_id) {
    ctx.active_window_count--;
}

static void drag_n_drop_callback(size_t origin_window_id, char *mime, char *buff, int x, int y, void *data) {
    // Not applicable
}

void tft_setup_callbacks(void (*callback)(size_t window_id, void *data), void *data) {
    // Implement if you have input support
}

void tft_run() {
    // Main loop for TFT_eSPI
    while (1) {
        // Handle input if needed
        // Update display
    }
}

GooeyTimer *tft_create_timer() {
    // Implement if needed
    return NULL;
}

void tft_stop_timer(GooeyTimer *timer) {
    // Implement if needed
}

void tft_destroy_timer(GooeyTimer *gooey_timer) {
    // Implement if needed
}

void tft_set_callback_for_timer(uint64_t time, GooeyTimer *timer, void (*callback)(void *user_data), void *user_data) {
    // Implement if needed
}

void tft_window_toggle_decorations(GooeyWindow *win, bool enable) {
    // Not applicable
}

size_t tft_get_active_window_count() {
    return ctx.active_window_count;
}

size_t tft_get_total_window_count() {
    return 1; // Only one screen
}

void tft_reset_events(GooeyWindow *win) {
    // Implement if needed
}

void tft_set_viewport(size_t window_id, int width, int height) {
    // Not applicable
}

double tft_get_window_framerate(int window_id) {
    // Not easily measurable with TFT_eSPI
    return 0;
}

GooeyBackend tft_backend = {
    .Init = tft_init,
    .Run = tft_run,
    .CreateWindow = tft_create_window,
    .WindowToggleDecorations = tft_window_toggle_decorations,
    .GetWinFramerate = tft_get_window_framerate,
    .GetActiveWindowCount = tft_get_active_window_count,
    .GetTotalWindowCount = tft_get_total_window_count,
    .SetupCallbacks = tft_setup_callbacks,
    .RequestRedraw = tft_request_redraw,
    .SetViewport = tft_set_viewport,
    .GetWinDim = tft_window_dim,
    .DestroyWindows = tft_destroy_windows,
    .DestroyWindowFromId = tft_destroy_window_from_id,
    .MakeWindowResizable = tft_set_window_resizable,
    .GetCurrentClickedWindow = tft_get_current_clicked_window,
    .HideCurrentChild = tft_hide_current_child,
    .MakeWindowVisible = tft_make_window_visible,
    .UpdateBackground = tft_update_background,
    .Cleanup = tft_cleanup,
    .Render = tft_render,
    .ResetEvents = tft_reset_events,
    .DrawImage = tft_draw_image,
    .LoadImageFromBin = tft_load_image_from_bin,
    .LoadImage = tft_load_image,
    .UnloadImage = tft_unload_image,
    .FillArc = tft_fill_arc,
    .FillRectangle = tft_fill_rectangle,
    .DrawRectangle = tft_draw_rectangle,
    .DrawLine = tft_draw_line,
    .SetForeground = tft_set_foreground,
    .GetTextWidth = tft_get_text_width,
    .GetTextHeight = tft_get_text_height,
    .DrawText = tft_draw_text,
    .GetKeyFromCode = tft_get_key_from_code,
    .SetCursor = tft_set_cursor,
    .CreateTimer = tft_create_timer,
    .SetTimerCallback = tft_set_callback_for_timer,
    .DestroyTimer = tft_destroy_timer,
    .StopTimer = tft_stop_timer,
    .Clear = tft_clear,
    .CursorChange = tft_set_cursor,
    .StopCursorReset = tft_stop_cursor_reset,
};
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

#include <TFT_eSPI.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"
#include "logger/pico_logger_internal.h"
#include <EEPROM.h>
#include <string>

static uint16_t rgb888_to_rgb565(uint32_t color) {
    return ((color & 0xF80000) >> 8) | ((color & 0xFC00) >> 5) | ((color & 0xF8) >> 3);
}
uint16_t cal[5] = { 0, 0, 0, 0, 0 };
typedef struct {
    TFT_eSPI* tft;
    size_t active_window_count;
    bool inhibit_reset;
    uint32_t selected_color;
    uint16_t* palette;
    size_t palette_size;
    GooeyWindow** windows;
    void (*ReDrawCallback)(size_t window_id, void*data);

} GooeyBackendContext;

static GooeyBackendContext ctx = {0};
bool loadTouchCalibration(uint16_t *calData) {
    if (EEPROM.read(0) == 0xA5) { 
        for (int i = 0; i < 5; i++) {
            calData[i] = EEPROM.read(i * 2 + 1) << 8 | EEPROM.read(i * 2 + 2);
        }
        return true;
    }
    return false;
}

void saveTouchCalibration(uint16_t *calData) {
    EEPROM.write(0, 0xA5); 
    for (int i = 0; i < 5; i++) {
        EEPROM.write(i * 2 + 1, calData[i] >> 8);
        EEPROM.write(i * 2 + 2, calData[i] & 0xFF);
    }
    EEPROM.commit();
}

void tft_setup_shared() {
    
    ctx.tft = new TFT_eSPI();
    ctx.tft->init();
    ctx.tft->setRotation(TFT_SCREEN_ROTATION);
    ctx.tft->fillScreen(TFT_BLACK);
    ctx.tft->setFreeFont(&FreeMono9pt7b);    
    ctx.tft->setTextSize(1);
    if (!loadTouchCalibration(cal)) {
        ctx.tft->fillScreen(TFT_BLACK);
        ctx.tft->setCursor(20, 20);
        ctx.tft->setTextColor(TFT_WHITE, TFT_BLACK);
        ctx.tft->println("Touch the dots to calibrate");
        ctx.tft->calibrateTouch(cal, TFT_YELLOW, TFT_BLACK, 20);
        saveTouchCalibration(cal); 
    }
    ctx.tft->setTouch(cal);
}
void tft_setup_separate_vao(int window_id) {
}

void tft_draw_rectangle(int x, int y, int width, int height,
                       uint32_t color, float thickness,
                       int window_id, bool isRounded, float cornerRadius) {
    uint16_t rgb565 = rgb888_to_rgb565(color);
    if (isRounded) {
        ctx.tft->drawRoundRect(x, y, width, height, (int)cornerRadius, rgb565);
    } else {
        ctx.tft->drawRect(x, y, width, height, rgb565);
    }
}

void tft_set_foreground(uint32_t color) {
    ctx.selected_color = color;
}

void tft_window_dim(int* width, int* height, int window_id) {
    if (width) *width = ctx.tft->width();
    if (height) *height = ctx.tft->height();
}

void tft_draw_line(int x1, int y1, int x2, int y2, uint32_t color, int window_id) {
    ctx.tft->drawLine(x1, y1, x2, y2, rgb888_to_rgb565(color));
}

void tft_fill_arc(int x_center, int y_center, int width, int height,
                 int angle1, int angle2, int window_id) {
    const int segments = 36;
    int radius = (width < height ? width : height) / 2;
    uint16_t color = rgb888_to_rgb565(ctx.selected_color);
    
    int prev_x = x_center + radius * cos(angle1 * DEG_TO_RAD);
    int prev_y = y_center + radius * sin(angle1 * DEG_TO_RAD);

    for (int i = 1; i <= segments; i++) {
        float angle = angle1 + (angle2 - angle1) * i / (float)segments;
        int x = x_center + radius * cos(angle * DEG_TO_RAD);
        int y = y_center + radius * sin(angle * DEG_TO_RAD);
        ctx.tft->drawLine(prev_x, prev_y, x_center, y_center, color);
        ctx.tft->drawLine(prev_x, prev_y, x, y, color);
        prev_x = x;
        prev_y = y;
    }
}

void tft_draw_image(unsigned int texture_id, int x, int y, int width, int height, int window_id) {
}

void tft_fill_rectangle(int x, int y, int width, int height,
                       uint32_t color, int window_id,
                       bool isRounded, float cornerRadius) {
    uint16_t rgb565 = rgb888_to_rgb565(color);
    if (isRounded) {
        ctx.tft->fillRoundRect(x, y, width, height, (int)cornerRadius, rgb565);
    } else {
        ctx.tft->fillRect(x, y, width, height, rgb565);
    }
}

void tft_set_projection(int window_id, int width, int height) {
}

static void keyboard_callback(size_t window_id, bool state, const char* value, unsigned long keycode,
                            void* data) {
}

static void mouse_click_callback(size_t window_id, bool state, void* data) {
}

void tft_request_redraw(GooeyWindow* win) {
}

static void mouse_move_callback(size_t window_id, double posX, double posY, void* data) {
}

static void window_resize_callback(size_t window_id, int width, int height, void* data) {
}

static void window_close_callback(size_t window_id, void* data) {
}

int tft_init_ft() {
    ctx.tft->setTextFont(1);
    ctx.tft->setTextColor(TFT_WHITE, TFT_BLACK);
    return 0;
}

int tft_init() {
    ctx.inhibit_reset = false;
    ctx.selected_color = TFT_WHITE;
    ctx.active_window_count = 1;
    tft_setup_shared();

    return 0;
}

int tft_get_current_clicked_window(void) {
    return 0;
}

void tft_draw_text(int x, int y, const char* text, uint32_t color, float font_size, int window_id) {
    ctx.tft->setTextColor(rgb888_to_rgb565(color), TFT_BLACK);
    ctx.tft->setCursor(x, y);
    ctx.tft->print(text);
}

void tft_unload_image(unsigned int texture_id) {
}

unsigned int tft_load_image(const char* image_path) {
    return 0;
}

unsigned int tft_load_image_from_bin(unsigned char* data, size_t binary_len) {
    return 0;
}

GooeyWindow* tft_create_window(const char* title, int width, int height) {
    GooeyWindow* window = (GooeyWindow*)malloc(sizeof(GooeyWindow));
    if (window) {
        window->creation_id = 0;
        ctx.active_window_count++;
    }
    return window;
}

void tft_make_window_visible(int window_id, bool visibility) {
}

void tft_set_window_resizable(bool value, int window_id) {
}

void tft_hide_current_child(void) {

}

void tft_destroy_windows() {
 
}

void tft_clear(GooeyWindow* win) {
    if (win && win->active_theme) {
        ctx.tft->fillScreen(rgb888_to_rgb565(win->active_theme->base));
    }
}

void tft_cleanup() {
    if (ctx.tft) {
        delete ctx.tft;
        ctx.tft = nullptr;
    }
}

void tft_update_background(GooeyWindow* win) {
    if (win && win->active_theme) {
        ctx.tft->fillScreen(rgb888_to_rgb565(win->active_theme->base));
    }
}

void tft_render(GooeyWindow* win) {

}
float tft_get_text_width(const char* text, int length) {
    return (float)ctx.tft->textWidth((String) text);
}

float tft_get_text_height(const char* text, int length) {
    return (float) ctx.tft->fontHeight();
}

const char* tft_get_key_from_code(void* gooey_event) {
    return NULL;
}

void tft_set_cursor(GOOEY_CURSOR cursor) {

}

void tft_stop_cursor_reset(bool state) {
    ctx.inhibit_reset = state;
}

void tft_destroy_window_from_id(int window_id) {
    if (ctx.active_window_count > 0) {
        ctx.active_window_count--;
    }
}

static void drag_n_drop_callback(size_t origin_window_id, char* mime, char* buff, int x, int y, void* data) {

}

void tft_setup_callbacks(void (*callback)(size_t window_id, void* data), void* data) {
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyWindow *win = (GooeyWindow *)windows[0];
    ctx.windows = windows;
    ctx.ReDrawCallback = callback;
}   


void tft_run() {
    while (1) {     
        uint16_t x, y;
        if (ctx.tft->getTouch(&x, &y)) {
        Serial.printf("touch %d %d\n", x, y);
        GooeyEvent* event = (GooeyEvent*) ctx.windows[0]->current_event;
        event->click.x = x;
        event->click.y = y;
        event->type = GOOEY_EVENT_CLICK_PRESS;
        ctx.ReDrawCallback(0, ctx.windows);

        }
    }
}


GooeyTimer* tft_create_timer() {
    return NULL;
}

void tft_stop_timer(GooeyTimer* timer) {
 
}

void tft_destroy_timer(GooeyTimer* timer) {
    
}

void tft_set_callback_for_timer(uint64_t time, GooeyTimer* timer, 
                              void (*callback)(void* user_data), void* user_data) {
   
}

void tft_window_toggle_decorations(GooeyWindow* win, bool enable) {
  
}

size_t tft_get_active_window_count() {
    return ctx.active_window_count;
}

size_t tft_get_total_window_count() {
    return 1;
}

void tft_reset_events(GooeyWindow* win) {
   
}

void tft_set_viewport(size_t window_id, int width, int height) {
    
}

double tft_get_window_framerate(int window_id) {
    return 0.0;
}
extern "C" {
    __attribute__((used, visibility("default"))) GooeyBackend tft_backend = {
        .Init = tft_init,
        .Run = tft_run,
        .Cleanup = tft_cleanup,
        .SetupCallbacks = tft_setup_callbacks,
        .RequestRedraw = tft_request_redraw,
        .SetViewport = tft_set_viewport,
        .GetActiveWindowCount = tft_get_active_window_count,
        .GetTotalWindowCount = tft_get_total_window_count,
        .CreateWindow = tft_create_window,
        .SpawnWindow = NULL, 
        .MakeWindowVisible = tft_make_window_visible,
        .MakeWindowResizable = tft_set_window_resizable,
        .WindowToggleDecorations = tft_window_toggle_decorations,
        .GetCurrentClickedWindow = tft_get_current_clicked_window,
        .DestroyWindows = tft_destroy_windows,
        .DestroyWindowFromId = tft_destroy_window_from_id,
        .HideCurrentChild = tft_hide_current_child,
        .SetContext = NULL,  
        .UpdateBackground = tft_update_background,
        .Clear = tft_clear,
        .Render = tft_render,
        .SetForeground = tft_set_foreground,
        .DrawText = tft_draw_text,
        .LoadImage = tft_load_image,
        .LoadImageFromBin = tft_load_image_from_bin,
        .DrawImage = tft_draw_image,
        .FillRectangle = tft_fill_rectangle,
        .DrawRectangle = tft_draw_rectangle,
        .FillArc = tft_fill_arc,
        .GetKeyFromCode = tft_get_key_from_code,
        .HandleEvents = NULL,
        .ResetEvents = tft_reset_events,
        .GetWinDim = tft_window_dim,
        .GetWinFramerate = tft_get_window_framerate,
        .DrawLine = tft_draw_line,
        .GetTextWidth = tft_get_text_width,
        .GetTextHeight = tft_get_text_height,
        .SetCursor = tft_set_cursor,
        .UnloadImage = tft_unload_image,
        .CreateTimer = tft_create_timer,
        .SetTimerCallback = tft_set_callback_for_timer,
        .StopTimer = tft_stop_timer,
        .DestroyTimer = tft_destroy_timer,
        .CursorChange = NULL,
        .StopCursorReset = tft_stop_cursor_reset,
    };
}

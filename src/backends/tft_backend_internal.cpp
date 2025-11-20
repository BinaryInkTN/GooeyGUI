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
static int clamp(int val, int min_val, int max_val)
{
    if (val < min_val)
        return min_val;
    if (val > max_val)
        return max_val;
    return val;
}
static uint16_t rgb888_to_rgb565(uint32_t color)
{
    return ((color & 0xF80000) >> 8) | ((color & 0xFC00) >> 5) | ((color & 0xF8) >> 3);
}
uint16_t cal[5] = {0, 0, 0, 0, 0};
typedef struct
{
    TFT_eSPI *tft;
    size_t active_window_count;
    bool inhibit_reset;
    uint32_t selected_color;
    uint16_t *palette;
    size_t palette_size;
    GooeyWindow **windows;
    void (*ReDrawCallback)(size_t window_id, void *data);
    bool is_running;

} GooeyBackendContext;

static GooeyBackendContext ctx = {0};
bool loadTouchCalibration(uint16_t *calData)
{
    if (EEPROM.read(0) == 0xA5)
    {
        for (int i = 0; i < 5; i++)
        {
            calData[i] = EEPROM.read(i * 2 + 1) << 8 | EEPROM.read(i * 2 + 2);
        }
        return true;
    }
    return false;
}

void saveTouchCalibration(uint16_t *calData)
{
    EEPROM.write(0, 0xA5);
    for (int i = 0; i < 5; i++)
    {
        EEPROM.write(i * 2 + 1, calData[i] >> 8);
        EEPROM.write(i * 2 + 2, calData[i] & 0xFF);
    }
    EEPROM.commit();
}

void tft_setup_shared()
{

    ctx.tft = new TFT_eSPI();
    ctx.tft->init();
    ctx.tft->setRotation(TFT_SCREEN_ROTATION);
    ctx.tft->fillScreen(TFT_BLACK);
    ctx.tft->setFreeFont(&FreeMono9pt7b);
    ctx.tft->setTextSize(1);

    if (!loadTouchCalibration(cal))
    {
        ctx.tft->fillScreen(TFT_BLACK);
        ctx.tft->setCursor(20, 20);
        ctx.tft->setTextColor(TFT_WHITE, TFT_BLACK);
        ctx.tft->println("Touch the dots to calibrate");
        ctx.tft->calibrateTouch(cal, TFT_YELLOW, TFT_BLACK, 20);
        saveTouchCalibration(cal);
    }
    ctx.tft->setTouch(cal);
    ctx.tft->fillRect(0, 0, 480, 320, TFT_WHITE);
}
void tft_setup_separate_vao(int window_id)
{
}
void tft_draw_rectangle(int x, int y, int width, int height,
                        uint32_t color, float thickness,
                        int window_id, bool isRounded, float cornerRadius, GooeyTFT_Sprite *sprite)
{
    uint16_t rgb565 = rgb888_to_rgb565(color);

    if (sprite)
    {
        if (sprite->needs_redraw)
        {
            TFT_eSprite *tft_sprite = (TFT_eSprite *)sprite->sprite_ptr;
            tft_sprite->fillSprite(TFT_BLACK);

            if (isRounded)
            {
                tft_sprite->drawRoundRect(x - sprite->x, y - sprite->y, width, height, (int)cornerRadius, rgb565);
            }
            else
            {
                tft_sprite->drawRect(x - sprite->x, y - sprite->y, width, height, rgb565);
            }
            tft_sprite->pushSprite(sprite->x, sprite->y, TFT_BLACK);
        }
    }
    else
    {
        if (isRounded)
        {
            //     ctx.tft->drawRoundRect(x, y, width, height, (int)cornerRadius, rgb565);
        }
        else
        {
            //    ctx.tft->drawRect(x, y, width, height, rgb565);
        }
    }
}

void tft_set_foreground(uint32_t color)
{
    ctx.selected_color = color;
}

void tft_window_dim(int *width, int *height, int window_id)
{
    if (width)
        *width = ctx.tft->width();
    if (height)
        *height = ctx.tft->height();
}

void tft_draw_line(int x1, int y1, int x2, int y2, uint32_t color, int window_id, GooeyTFT_Sprite *sprite)
{
    uint16_t rgb565 = rgb888_to_rgb565(color);

    if (sprite)
    {
        if (sprite->needs_redraw)
        {
            TFT_eSprite *tft_sprite = (TFT_eSprite *)sprite->sprite_ptr;
            tft_sprite->fillSprite(TFT_BLACK);

            int sx1 = x1 - sprite->x;
            int sy1 = y1 - sprite->y;
            int sx2 = x2 - sprite->x;
            int sy2 = y2 - sprite->y;

            tft_sprite->drawLine(sx1, sy1, sx2, sy2, rgb565);
            tft_sprite->pushSprite(sprite->x, sprite->y, TFT_BLACK);
        }
    }
    else
    {
        //   ctx.tft->drawLine(x1, y1, x2, y2, rgb565);
    }
}
void tft_fill_arc(int x_center, int y_center, int width, int height,
                  int angle1, int angle2, int window_id, GooeyTFT_Sprite *sprite)
{
    if (!sprite || !sprite->sprite_ptr)
        return;

    TFT_eSprite *tft_sprite = (TFT_eSprite *)sprite->sprite_ptr;

    int radius = (width < height ? width : height) / 2;
    uint16_t color = rgb888_to_rgb565(ctx.selected_color);

    int cx = x_center - sprite->x;
    int cy = y_center - sprite->y;

    if (angle2 < angle1)
        angle2 += 360;

    if ((angle2 - angle1) >= 360)
    {
        tft_sprite->fillCircle(cx, cy, radius, color);
        sprite->needs_redraw = true;
        tft_sprite->pushSprite(sprite->x, sprite->y, TFT_TRANSPARENT);
        return;
    }

    int radius_sq = radius * radius;

    for (int y = -radius; y <= radius; y++)
    {
        int y_sq = y * y;
        int dx_limit = (int)sqrtf(radius_sq - y_sq);
        int last_start = -1, last_end = -1;

        for (int x = -dx_limit; x <= dx_limit; x++)
        {
            float angle = atan2f(-y, -x) * RAD_TO_DEG;
            if (angle < 0)
                angle += 360;

            if (angle >= angle1 && angle <= angle2)
            {
                if (last_start == -1)
                {
                    last_start = x;
                    last_end = x;
                }
                else if (x == last_end + 1)
                {
                    last_end = x;
                }
                else
                {
                    tft_sprite->drawFastHLine(cx + last_start,
                                              cy + y,
                                              last_end - last_start + 1,
                                              color);
                    last_start = x;
                    last_end = x;
                }
            }
        }

        if (last_start != -1)
        {
            tft_sprite->drawFastHLine(cx + last_start,
                                      cy + y,
                                      last_end - last_start + 1,
                                      color);
        }
    }

    sprite->needs_redraw = true;
    tft_sprite->pushSprite(sprite->x, sprite->y, TFT_TRANSPARENT);
}

void tft_draw_image(unsigned int texture_id, int x, int y, int width, int height, int window_id)
{
}
void tft_fill_rectangle(int x, int y, int width, int height,
                        uint32_t color, int window_id,
                        bool isRounded, float cornerRadius, GooeyTFT_Sprite *sprite)
{
    uint16_t rgb565 = rgb888_to_rgb565(color);

    if (sprite)
    {
        if (sprite->needs_redraw)
        {
            TFT_eSprite *tft_sprite = (TFT_eSprite *)sprite->sprite_ptr;
            tft_sprite->fillSprite(TFT_BLACK);

            if (isRounded)
            {
                tft_sprite->fillRoundRect(x - sprite->x, y - sprite->y, width, height, (int)cornerRadius, rgb565);

                Serial.printf("fillRect: sprite pos=(%d, %d), draw at x=%d y=%d\n",
                              sprite->x, sprite->y, x, y);
            }
            else
            {
                tft_sprite->fillRect(x - sprite->x, y - sprite->y, width, height, rgb565);
            }

            tft_sprite->pushSprite(sprite->x, sprite->y, TFT_BLACK);
            //  sprite->needs_redraw = true;
        }
        //  sprite->needs_redraw = false;
    }
    else
    {
        if (isRounded)
        {
            // ctx.tft->fillRoundRect(x, y, width, height, (int)cornerRadius, rgb565);
        }
        else
        {
            // ctx.tft->fillRect(x, y, width, height, rgb565);
        }
    }
}

void tft_destroy_sprite(GooeyTFT_Sprite *sprite)
{
    if (sprite && sprite->sprite_ptr)
    {
        TFT_eSprite *tft_sprite = (TFT_eSprite *)sprite->sprite_ptr;
        tft_sprite->deleteSprite();
        delete tft_sprite;
        free(sprite);
    }
}

void tft_set_projection(int window_id, int width, int height)
{
}

static void keyboard_callback(size_t window_id, bool state, const char *value, unsigned long keycode,
                              void *data)
{
}

static void mouse_click_callback(size_t window_id, bool state, void *data)
{
}

void tft_request_redraw(GooeyWindow *win)
{
}

static void mouse_move_callback(size_t window_id, double posX, double posY, void *data)
{
}

static void window_resize_callback(size_t window_id, int width, int height, void *data)
{
}

static void window_close_callback(size_t window_id, void *data)
{
}

int tft_init_ft()
{
    ctx.tft->setTextFont(1);
    ctx.tft->setTextColor(TFT_WHITE, TFT_BLACK);
    return 0;
}

int tft_init(int project_branch)
{
    (void)project_branch;
    Serial.begin(9600);
    ctx.inhibit_reset = false;
    ctx.selected_color = TFT_WHITE;
    ctx.active_window_count = 1;
    ctx.is_running = true;
    tft_setup_shared();

    return 0;
}

int tft_get_current_clicked_window(void)
{
    return 0;
}

void tft_draw_text(int x, int y, const char *text, uint32_t color, float font_size, int window_id, GooeyTFT_Sprite *sprite)
{

    TFT_eSprite* tft_sprite = (TFT_eSprite*) sprite;
    if (sprite == nullptr) {
        ctx.tft->setTextColor(rgb888_to_rgb565(color), TFT_BLACK);
        ctx.tft->setCursor(x, y);
        ctx.tft->print(text);
        return;
    }

    tft_sprite->setTextColor(rgb888_to_rgb565(color), TFT_BLACK);
    tft_sprite->setCursor(x, y);
    tft_sprite->print(text);
}
void tft_unload_image(unsigned int texture_id)
{
}

unsigned int tft_load_image(const char *image_path)
{
    return 0;
}

unsigned int tft_load_image_from_bin(unsigned char *data, size_t binary_len)
{
    return 0;
}

GooeyWindow *tft_create_window(const char *title, int width, int height)
{
    GooeyWindow *window = (GooeyWindow *)malloc(sizeof(GooeyWindow));
    if (window)
    {
        window->creation_id = 0;
        ctx.active_window_count++;
    }
    return window;
}

void tft_make_window_visible(int window_id, bool visibility)
{
}

void tft_set_window_resizable(bool value, int window_id)
{
}

void tft_hide_current_child(void)
{
}

void tft_destroy_windows()
{
}

void tft_clear(GooeyWindow *win)
{
    // if (win && win->active_theme)
    //  {
    ctx.tft->fillRect(0, 0, 480, 320, TFT_WHITE);
    // }
}

void tft_cleanup()
{
    if (ctx.tft)
    {
        delete ctx.tft;
        ctx.tft = nullptr;
    }
}

void tft_update_background(GooeyWindow *win)
{
    if (win && win->active_theme)
    {
        // ctx.tft->fillScreen(rgb888_to_rgb565(win->active_theme->base));
    }
}

void tft_render(GooeyWindow *win)
{
}
float tft_get_text_width(const char *text, int length)
{
    return (float)ctx.tft->textWidth((String)text);
}

float tft_get_text_height(const char *text, int length)
{
    return (float)ctx.tft->fontHeight();
}

const char *tft_get_key_from_code(void *gooey_event)
{
    return NULL;
}

void tft_set_cursor(GOOEY_CURSOR cursor)
{
}

void tft_stop_cursor_reset(bool state)
{
    ctx.inhibit_reset = state;
}

void tft_destroy_window_from_id(int window_id)
{
    if (ctx.active_window_count > 0)
    {
        ctx.active_window_count--;
    }
}

static void drag_n_drop_callback(size_t origin_window_id, char *mime, char *buff, int x, int y, void *data)
{
}

void tft_setup_callbacks(void (*callback)(size_t window_id, void *data), void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyWindow *win = (GooeyWindow *)windows[0];
    ctx.windows = windows;
    ctx.ReDrawCallback = callback;
}

void tft_run()
{
    while (ctx.is_running)
    {
        uint16_t x, y;
        if (ctx.tft->getTouch(&x, &y))
        {
            Serial.printf("touch %d %d\n", x, y);
            GooeyEvent *event = (GooeyEvent *)ctx.windows[0]->current_event;
            event->click.x = x;
            event->click.y = y;
            event->type = GOOEY_EVENT_CLICK_PRESS;
            ctx.ReDrawCallback(0, ctx.windows);

            // Simulate release
            event->type = GOOEY_EVENT_CLICK_RELEASE;
        }
    }
}

void tft_force_redraw()
{
    ctx.ReDrawCallback(0, ctx.windows);
}

GooeyTimer *tft_create_timer()
{
    return NULL;
}

void tft_stop_timer(GooeyTimer *timer)
{
}

void tft_destroy_timer(GooeyTimer *timer)
{
}

void tft_set_callback_for_timer(uint64_t time, GooeyTimer *timer,
                                void (*callback)(void *user_data), void *user_data)
{
}

void tft_window_toggle_decorations(GooeyWindow *win, bool enable)
{
}

size_t tft_get_active_window_count()
{
    return ctx.active_window_count;
}

size_t tft_get_total_window_count()
{
    return 1;
}

void tft_reset_events(GooeyWindow *win)
{
}

void tft_set_viewport(size_t window_id, int width, int height)
{
}

double tft_get_window_framerate(int window_id)
{
    return 0.0;
}
GooeyTFT_Sprite *tft_create_widget_sprite(int x, int y, int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        Serial.printf("INVALID SPRITE SIZE %d %d\n", width, height);
        return NULL;
    }

    x = clamp(x, 0, 480 - width);
    y = clamp(y, 0, 320 - height);

    TFT_eSprite *sprite = new TFT_eSprite(ctx.tft);
    if (!sprite)
    {
        Serial.println("Failed to allocate TFT_eSprite");
        return NULL;
    }

    GooeyTFT_Sprite *g_sprite = (GooeyTFT_Sprite *)calloc(1, sizeof(GooeyTFT_Sprite));
    if (!g_sprite)
    {
        Serial.println("Failed to allocate GooeyTFT_Sprite");
        delete sprite;
        return NULL;
    }

    if (!sprite->createSprite(width, height))
    {
        Serial.printf("INVALID SPRITE %d %d\n", width, height);
        delete sprite;
        free(g_sprite);
        return NULL;
    }

    g_sprite->sprite_ptr = sprite;
    g_sprite->x = x;
    g_sprite->y = y;
    g_sprite->width = width;
    g_sprite->height = height;
    g_sprite->needs_redraw = true;

    sprite->fillSprite(TFT_BLACK);
    sprite->setTextColor(TFT_WHITE, TFT_BLACK);

    return g_sprite;
}
void tft_redraw_sprite(GooeyTFT_Sprite *sprite)
{
    if (sprite && sprite->sprite_ptr)
    {
        TFT_eSprite *tft_sprite = (TFT_eSprite *)sprite->sprite_ptr;
        // clear old content
        tft_sprite->fillSprite(TFT_WHITE);
        delay(20);
        tft_sprite->pushSprite(sprite->x, sprite->y, TFT_BLACK);
        sprite->needs_redraw = true;
    }
}

void tft_reset_sprite_redraw(GooeyTFT_Sprite *sprite)
{
    sprite->needs_redraw = false;
}

void tft_clear_area(int x, int y, int width, int height)
{
    const int chunk_height = 64;
    int remaining_height = height;
    int current_y = y;

    while (remaining_height > 0)
    {
        int h = remaining_height > chunk_height ? chunk_height : remaining_height;
        ctx.tft->fillRect(x, current_y, width, h, TFT_WHITE);
        current_y += h;
        remaining_height -= h;
    }
}

void tft_clear_old_widget(GooeyTFT_Sprite* sprite)
{
    ctx.tft->fillRect(sprite->x, sprite->y, sprite->width, sprite->height, TFT_WHITE);
}

void tft_request_close()
{
    ctx.is_running = false;
}

extern "C"
{
    __attribute__((used, visibility("default"))) GooeyBackend tft_backend = {
        .Init = tft_init,
        .Run = tft_run,
        .Cleanup = tft_cleanup,
        .SetupCallbacks = tft_setup_callbacks,
        .RequestRedraw = tft_request_redraw,
        .SetViewport = tft_set_viewport,
        .GetActiveWindowCount = tft_get_active_window_count,
        .GetTotalWindowCount = tft_get_total_window_count,
        .GCreateWindow = tft_create_window,
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
        .ForceCallRedraw = tft_force_redraw,
        .RequestClose = tft_request_close,
        .CreateSpriteForWidget = tft_create_widget_sprite,
        .RedrawSprite = tft_redraw_sprite,
        .ResetRedrawSprite = tft_reset_sprite_redraw,
        .ClearArea = tft_clear_area,
        .ClearOldWidget = tft_clear_old_widget,
        .GetPlatformName = NULL
    };
}

#include "virtual/gooey_keyboard_internal.h"
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"
#include "logger/pico_logger_internal.h"
#include <string.h>

#if (ENABLE_VIRTUAL_KEYBOARD)
char vk_preview_buffer[128] = {0};

static const char *keyboard_layout_lower[][12] = {
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "DEL"},
    {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "ENT"},
    {"a", "s", "d", "f", "g", "h", "j", "k", "l"},
    {"z", "c", "v", "b", "n", "m", "ST"},

    {"123", "SPACE", "ENTER"}};
static const int keyboard_layout_lower_sizes[] = {11, 11, 9, 7, 3};

static const char *keyboard_layout_upper[][12] = {
    {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "DEL"},
    {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "ENT"},
    {"A", "S", "D", "F", "G", "H", "J", "K", "L"},
    {"Z", "C", "V", "B", "N", "M", "ST"},
    {"123", "SPACE", "ENTER"}};
static const int keyboard_layout_upper_sizes[] = {11, 11, 9, 7, 3};

static const char *keyboard_layout_symbols[][12] = {
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "DEL"},
    {"-", "/", ":", ";", "(", ")", "$", "&", "@", "\"", "ENT"},
    {"[", "]", "{", "}", "#", "%", "'", "=", "+"},
    {"", "", "", "", "", "", "ST"},

    {"ABC", "SPACE", "ENTER"}};
static const int keyboard_layout_symbols_sizes[] = {11, 11, 9, 7, 3};

static int vk_layout_state = 0;

static void render_preview_bar(GooeyWindow *win, int preview_height)
{
    active_backend->FillRectangle(0, 0, win->width, preview_height, win->active_theme->widget_base, win->creation_id, true, 4.0f, NULL);

    const float text_scale = 0.35f;
    int text_height = active_backend->GetTextHeight(vk_preview_buffer, text_scale);
    int text_x = 10;
    int text_y = (preview_height - text_height) / 2;

    active_backend->DrawText(text_x, text_y, vk_preview_buffer, win->active_theme->neutral, text_scale, win->creation_id);
}

static void append_to_preview(const char *key)
{
    size_t len = strlen(vk_preview_buffer);

    if (strcmp(key, "DEL") == 0)
    {
        if (len > 0)
            vk_preview_buffer[len - 1] = '\0';
        return;
    }

    if (strcmp(key, "ENTER") == 0 || strcmp(key, "ENT") == 0)
    {

        return;
    }

    if (strcmp(key, "SPACE") == 0 || strcmp(key, " ") == 0)
    {
        if (len + 1 < sizeof(vk_preview_buffer))
        {
            vk_preview_buffer[len] = ' ';
            vk_preview_buffer[len + 1] = '\0';
        }
        return;
    }

    if (strlen(key) == 1)
    {
        if (len + 1 < sizeof(vk_preview_buffer))
        {
            vk_preview_buffer[len] = key[0];
            vk_preview_buffer[len + 1] = '\0';
        }
    }
}

static void render_keyboard_buttons(GooeyWindow *win, int y_offset)
{
    if (!win)
        return;

    const int rows = 5;
    const int row_height = (win->height - y_offset) / rows;

    const char *(*layout)[12];
    const int *row_sizes;

    switch (vk_layout_state)
    {
    case 1:
        layout = keyboard_layout_upper;
        row_sizes = keyboard_layout_upper_sizes;
        break;
    case 2:
        layout = keyboard_layout_symbols;
        row_sizes = keyboard_layout_symbols_sizes;
        break;
    case 0:
    default:
        layout = keyboard_layout_lower;
        row_sizes = keyboard_layout_lower_sizes;
        break;
    }

    for (int row = 0; row < rows; ++row)
    {
        int cols = row_sizes[row];
        int button_width = win->width / cols;

        int x = 0;

        for (int col = 0; col < cols; ++col)
        {
            const char *label = layout[row][col];

            int current_button_width = button_width;

            if (row == rows - 1 && (strcmp(label, "SPACE") == 0))
            {
                current_button_width = button_width;
            }

            int draw_x = x + 2;
            int draw_y = y_offset + row * row_height + 2;

            active_backend->FillRectangle(draw_x, draw_y, current_button_width - 4, row_height - 4,
                                          win->active_theme->widget_base, win->creation_id, true, 5.0f, NULL);

            int label_x = draw_x + current_button_width / 2 - active_backend->GetTextWidth(label, 1) / 2;
            int label_y = draw_y + (row_height - 4) / 2 + active_backend->GetTextHeight(label, 1) / 2;

            active_backend->DrawText(label_x, label_y, label, win->active_theme->neutral, 0.27f, win->creation_id);

            x += current_button_width;
        }
    }
}

void GooeyVK_Internal_Draw(GooeyWindow *win)
{
    if (!win || !(win->vk && win->vk->is_shown))
        return;

    const int preview_height = win->height / 6;

    active_backend->FillRectangle(0, 0, win->width, win->height, win->active_theme->base, 0, false, 0.0f,NULL);

    render_preview_bar(win, preview_height);
    render_keyboard_buttons(win, preview_height);
}

void GooeyVK_Internal_HandleClick(GooeyWindow *win, int mouseX, int mouseY)
{
    if (!win || !(win->vk && win->vk->is_shown))
        return;

    const int preview_height = win->height / 6;
    if (mouseY < preview_height)
        return;

    int y_offset = preview_height;
    const int rows = 5;
    const int row_height = (win->height - y_offset) / rows;

    const char *(*layout)[12];
    const int *row_sizes;

    switch (vk_layout_state)
    {
    case 1:
        layout = keyboard_layout_upper;
        row_sizes = keyboard_layout_upper_sizes;
        break;
    case 2:
        layout = keyboard_layout_symbols;
        row_sizes = keyboard_layout_symbols_sizes;
        break;
    case 0:
    default:
        layout = keyboard_layout_lower;
        row_sizes = keyboard_layout_lower_sizes;
        break;
    }

    int y = y_offset;
    for (int row = 0; row < rows; ++row)
    {
        int cols = row_sizes[row];
        int button_width = win->width / cols;

        int x = 0;

        int row_y_start = y;
        int row_y_end = y + row_height;

        if (mouseY >= row_y_start && mouseY <= row_y_end)
        {
            for (int col = 0; col < cols; ++col)
            {
                const char *key = layout[row][col];

                int current_button_width = button_width;

                if (row == rows - 1 && (strcmp(key, "SPACE") == 0))
                    current_button_width = button_width;

                int x_start = x;
                int x_end = x + current_button_width;

                if (mouseX >= x_start && mouseX <= x_end)
                {
                    if (strcmp(key, "ENTER") == 0 || strcmp(key, "ENT") == 0)
                    {
                        win->vk->is_shown = false;
                        GooeyEvent *event = (GooeyEvent *)win->current_event;
                        event->type = GOOEY_EVENT_VK_ENTER;
                        active_backend->ForceCallRedraw();
                        return;
                    }

                    if (strcmp(key, "ST") == 0)
                    {

                        if (vk_layout_state == 0)
                            vk_layout_state = 1;
                        else if (vk_layout_state == 1)
                            vk_layout_state = 0;
                        return;
                    }

                    if (strcmp(key, "123") == 0)
                    {
                        vk_layout_state = 2;
                        return;
                    }

                    if (strcmp(key, "ABC") == 0)
                    {
                        vk_layout_state = 0;
                        return;
                    }

                    append_to_preview(key);
                    return;
                }

                x += current_button_width;
            }
        }

        y += row_height;
    }
}

void GooeyVK_Internal_Show(GooeyVK *vk, size_t textbox_id)
{
    vk->text_widget_id = textbox_id;
    vk->is_shown = true;
}

void GooeyVK_Internal_Hide(GooeyVK *vk)
{
    vk->is_shown = false;
}

char *GooeyVK_Internal_GetText(GooeyVK *vk)
{
    return vk_preview_buffer;
}

void GooeyVK_Internal_SetText(GooeyVK *vk, const char *text)
{
    strncpy(vk_preview_buffer, text, sizeof(vk_preview_buffer));
}

#endif

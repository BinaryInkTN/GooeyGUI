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

#include "widgets/gooey_textbox.h"
#include <math.h>
#include "core/gooey_backend.h"


GooeyTextbox *GooeyTextBox_Add(GooeyWindow *win, int x, int y, int width,
                               int height, char *placeholder, void (*onTextChanged)(char *text))
{
    win->textboxes[win->textboxes_count] = (GooeyTextbox){0};
    win->textboxes[win->textboxes_count].core.type = WIDGET_TEXTBOX;
    win->textboxes[win->textboxes_count].core.x = x;
    win->textboxes[win->textboxes_count].core.y = y;
    win->textboxes[win->textboxes_count].core.width = width;
    win->textboxes[win->textboxes_count].core.height = height;
    win->textboxes[win->textboxes_count].focused = false;
    win->textboxes[win->textboxes_count].callback = onTextChanged;
    win->textboxes[win->textboxes_count].scroll_offset = 0;
    win->textboxes[win->textboxes_count].text[0] = '\0';
    strcpy(win->textboxes[win->textboxes_count].placeholder, placeholder);

    GooeyWindow_RegisterWidget(win, (GooeyWidget *)&win->textboxes[win->textboxes_count].core);
    LOG_INFO("Textbox added with dimensions x=%d, y=%d, w=%d, h=%d", x, y, width, height);

    win->textboxes_count++;
    return &win->textboxes[win->textboxes_count - 1];
}

const char *GooeyTextbox_GetText(GooeyTextbox *textbox)
{
    if (!textbox)
    {
        LOG_ERROR("Widget<Textbox> cannot be null.");
        return NULL;
    }

    return (const char *)textbox->text;
}

void GooeyTextbox_setText(GooeyTextbox *textbox, const char *text)
{
    if (!textbox)
    {
        LOG_ERROR("Widget<Textbox> cannot be null.");
        return;
    }
    strcpy(textbox->text, text);
}

void GooeyTextbox_Draw(GooeyWindow *win)
{

    for (size_t index = 0; index < win->textboxes_count; ++index)
    {
        active_backend->FillRectangle(win->textboxes[index].core.x, win->textboxes[index].core.y,
                                      win->textboxes[index].core.width, win->textboxes[index].core.height, win->active_theme->base, win->creation_id);

        active_backend->DrawRectangle(win->textboxes[index].core.x, win->textboxes[index].core.y,
                                      win->textboxes[index].core.width, win->textboxes[index].core.height,
                                      win->textboxes[index].focused ? win->active_theme->primary : win->active_theme->neutral, win->creation_id);

        int text_x = win->textboxes[index].core.x + 5;
        int text_y = win->textboxes[index].core.y + (win->textboxes[index].core.height / 2) + 5;

        int max_text_width = win->textboxes[index].core.width - 10;
        size_t len = strlen(win->textboxes[index].text);
        size_t start_index = win->textboxes[index].scroll_offset;

        while (start_index < len &&
               active_backend->GetTextWidth(win->textboxes[index].text + start_index, len - start_index) > max_text_width)
        {
            start_index++;
        }

        char display_text[256];
        strncpy(display_text, win->textboxes[index].text + start_index, sizeof(display_text) - 1);
        display_text[sizeof(display_text) - 1] = '\0';

        active_backend->DrawText(text_x, text_y, display_text, win->active_theme->neutral, 0.25f, win->creation_id);

        if (win->textboxes[index].focused)
        {
            int cursor_x = text_x + active_backend->GetTextWidth(display_text, strlen(display_text));
            active_backend->DrawLine(cursor_x, win->textboxes[index].core.y + 5,
                                     cursor_x, win->textboxes[index].core.y + win->textboxes[index].core.height - 5, win->active_theme->neutral, win->creation_id);
        }
        else
        {

            if (strcmp(win->textboxes[index].placeholder, "") != 0 && strlen(win->textboxes[index].text) == 0)
                active_backend->DrawText(text_x, text_y, win->textboxes[index].placeholder, win->active_theme->neutral, 0.25f, win->creation_id);
        }
    }
}

void GooeyTextbox_HandleKeyPress(GooeyWindow *win, GooeyEvent *key_event)
{

    static bool is_capslock_on = false;
    static int ascii_offset = 'a' - 'A';
    printf("Key pressed\n");

    const char *buf = active_backend->GetKeyFromCode(key_event);
    if (buf == NULL)
    {
        return;
    }

    for (size_t i = 0; i < win->textboxes_count; i++)
    {
        if (!win->textboxes[i].focused)
            continue;

        size_t len = strlen(win->textboxes[i].text);

        if (strcmp(buf, "Backspace") == 0)
        {
            if (len > 0)
            {
                win->textboxes[i].text[len - 1] = '\0';

                if (win->textboxes[i].scroll_offset > 0)
                {
                    win->textboxes[i].scroll_offset--;
                }

                if (win->textboxes[i].callback)
                {
                    win->textboxes[i].callback(win->textboxes[i].text);
                }
            }
        }
        else if (strcmp(buf, "Return") == 0)
        {
            win->textboxes[i].focused = false;
        }
        else if (strcmp(buf, "Caps_Lock") == 0)
        {
            is_capslock_on = !is_capslock_on;
        }
        else if (strcmp(buf, "Space") == 0)
        {
            strcat(win->textboxes[i].text, " ");
        }
        else if (strcmp(buf, "Tab") == 0)
        {
        }
        else if (isprint(buf[0]) && len < sizeof(win->textboxes[i].text) - 1)
        {
            char ch = buf[0];
            if (is_capslock_on && ch >= 'a' && ch <= 'z')
            {
                ch -= ascii_offset;
            }
            win->textboxes[i].text[len] = ch;
            win->textboxes[i].text[len + 1] = '\0';

            if (win->textboxes[i].callback)
            {
                win->textboxes[i].callback(win->textboxes[i].text);
            }

            int text_width = active_backend->GetTextWidth(win->textboxes[i].text, len + 1);
            int max_text_width = win->textboxes[i].core.width - 10;

            if (text_width > max_text_width)
            {
                win->textboxes[i].scroll_offset++;
            }
        }
    }

    //
}

bool GooeyTextbox_HandleClick(GooeyWindow *win, int x, int y)
{
    for (size_t i = 0; i < win->textboxes_count; i++)
    {
        GooeyTextbox *textbox = &win->textboxes[i];
        if (x >= textbox->core.x &&
            x <= textbox->core.x + textbox->core.width &&
            y >= textbox->core.y &&
            y <= textbox->core.y + textbox->core.height)
        {
            // Focus the clicked textbox and unfocus others
            textbox->focused = true;
            for (size_t j = 0; j < win->textboxes_count; j++)
            {
                if (j != i)
                    win->textboxes[j].focused = false;
            }
            return true;
        }
    }
    return false;
}

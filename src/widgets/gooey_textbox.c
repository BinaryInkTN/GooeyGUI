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
#if (ENABLE_TEXTBOX)
#include <math.h>
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

GooeyTextbox *GooeyTextBox_Create(int x, int y, int width,
                                  int height, char *placeholder, bool is_password, void (*onTextChanged)(char *text))
{
    GooeyTextbox *textBox = calloc(1, sizeof(GooeyTextbox));
    if (textBox == NULL)
    {
        LOG_ERROR("Failed to allocate memory to textBox ");
        return NULL;
    }
    *textBox = (GooeyTextbox){0};
    textBox->core.type = WIDGET_TEXTBOX;
    textBox->core.x = x;
    textBox->core.y = y;
    textBox->core.width = width;
    textBox->cursor_pos = 0;
    textBox->core.height = height;
    textBox->core.is_visible = true;
    textBox->focused = false;
    textBox->callback = onTextChanged;
    textBox->is_password = is_password;
    textBox->scroll_offset = 0;
    textBox->text[0] = '\0';
    textBox->core.sprite = active_backend->CreateSpriteForWidget(x - 40, y - 40, width + 40, height + 40);

    strcpy(textBox->placeholder, placeholder);

    // GooeyWindow_RegisterWidget(win, (GooeyWidget *)&win->textboxes[win->textboxes_count].core);
    LOG_INFO("Textbox added with dimensions x=%d, y=%d, w=%d, h=%d", x, y, width, height);

    return textBox;
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
#endif
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

#include "widgets/gooey_label.h"
#if(ENABLE_LABEL)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

GooeyLabel *GooeyLabel_Create(const char *text, float font_size, int x, int y)
{
    GooeyLabel *label = (GooeyLabel *)calloc(1, sizeof(GooeyLabel));
    if (!label)
    {
        LOG_ERROR("Couldn't allocate memory for label.");
        return NULL;
    }

    *label = (GooeyLabel){0};

    label->core.type = WIDGET_LABEL;
    label->core.x = x;
    label->core.y = y;
    label->core.is_visible = true;
    label->font_size = font_size;
    label->is_using_custom_color = false;
    //label->core.sprite = active_backend->CreateSpriteForWidget(x, y, width, height);

    strncpy(label->text, text, sizeof(label->text) - 1);
    label->text[sizeof(label->text) - 1] = '\0';
    LOG_INFO("Label added with dimensions x=%d, y=%d", x, y);

    return label;
}

void GooeyLabel_SetColor(GooeyLabel *label, unsigned long color)
{
    label->color = color;
    label->is_using_custom_color = true;
}

void GooeyLabel_SetText(GooeyLabel *label, const char *text)
{
    if (label)
        strcpy(label->text, text);
}
#endif

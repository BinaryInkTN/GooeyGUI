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
#include "core/gooey_backend_internal.h"

GooeyLabel *GooeyLabel_Add(GooeyWindow *win, const char *text, float font_size, int x, int y)
{
    win->labels[win->label_count] = (GooeyLabel) {0};
    GooeyLabel *label = &win->labels[win->label_count++];
    label->core.type = WIDGET_LABEL;
    label->core.x = x;
    label->core.y = y;
    label->font_size = font_size;
    label->color = -1; // DEFAULTS TO NEUTRAL
    strcpy(label->text, text);
    GooeyWindow_RegisterWidget(win, (GooeyWidget *)&label->core);
    LOG_INFO("Label added with dimensions x=%d, y=%d", x, y);

    return label;
}

void GooeyLabel_SetColor(GooeyLabel *label, const char *color)
{
    unsigned long color_long = (unsigned long)strtol(color, NULL, 0);
    label->color = color_long;

}

void GooeyLabel_SetText(GooeyLabel *label, const char *text)
{
    if (label)
        strcpy(label->text, text);
}

void GooeyLabel_Draw(GooeyWindow *win)
{

    for (size_t i = 0; i < win->label_count; ++i)
    {

        active_backend->DrawText(win->labels[i].core.x, win->labels[i].core.y, win->labels[i].text, win->labels[i].color != (unsigned long) -1 ? win->labels[i].color : win->active_theme->neutral, win->labels[i].font_size, win->creation_id);
    }
}
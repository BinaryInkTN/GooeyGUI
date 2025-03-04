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

#include "widgets/gooey_checkbox.h"
#include "core/gooey_backend_internal.h"

GooeyCheckbox *GooeyCheckbox_Add(GooeyWindow *win, int x, int y, char *label,
                                 void (*callback)(bool checked))
{
    win->checkboxes[win->checkbox_count] = (GooeyCheckbox) {0};
    GooeyCheckbox *checkbox = &win->checkboxes[win->checkbox_count++];
    checkbox->core.type = WIDGET_CHECKBOX, checkbox->core.x = x;
    checkbox->core.y = y;
    checkbox->core.width = CHECKBOX_SIZE;
    checkbox->core.height = CHECKBOX_SIZE;
    if (label)
    {
        strcpy(checkbox->label, label);
    }
    else
    {
        sprintf(checkbox->label, "Checkbox %d", win->checkbox_count);
    }
    checkbox->checked = false;
    checkbox->callback = callback;
    GooeyWindow_RegisterWidget(win, (GooeyWidget *)&checkbox->core);
    LOG_INFO("Checkbox added with dimensions x=%d, y=%d", x, y);

    return checkbox;
}

void GooeyCheckbox_Draw(GooeyWindow *win)
{

    for (size_t i = 0; i < win->checkbox_count; ++i)
    {
        GooeyCheckbox *checkbox = &win->checkboxes[i];

        int label_width = active_backend->GetTextWidth(checkbox->label, strlen(checkbox->label));
        int label_x = checkbox->core.x + CHECKBOX_SIZE + 10;
        int label_y = checkbox->core.y + (CHECKBOX_SIZE / 2) + 5;
        active_backend->DrawText(label_x, label_y, checkbox->label, active_theme->neutral, 0.25f, win->creation_id);

        active_backend->DrawRectangle(checkbox->core.x, checkbox->core.y,
                                      checkbox->core.width, checkbox->core.height, active_theme->neutral, win->creation_id);
        active_backend->FillRectangle(checkbox->core.x + 1, checkbox->core.y + 1,
                                      checkbox->core.width - 2, checkbox->core.height - 2, active_theme->base, win->creation_id);

        if (checkbox->checked)
        {
            active_backend->FillRectangle(checkbox->core.x + 5, checkbox->core.y + 5,
                                          checkbox->core.width - 10, checkbox->core.height - 10, active_theme->primary, win->creation_id);
        }
    }
}

bool GooeyCheckbox_HandleClick(GooeyWindow *win, int x, int y)
{
    for (size_t i = 0; i < win->checkbox_count; ++i)
    {
        GooeyCheckbox *checkbox = &win->checkboxes[i];
        if (x >= checkbox->core.x && x <= checkbox->core.x + checkbox->core.width &&
            y >= checkbox->core.y &&
            y <= checkbox->core.y + checkbox->core.height)
        {
            checkbox->checked = !checkbox->checked;
            if (checkbox->callback)
                checkbox->callback(checkbox->checked);
            return true;
        }
    }
    return false;
}

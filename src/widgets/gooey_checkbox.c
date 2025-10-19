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
#if (ENABLE_CHECKBOX)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
#define CHECKBOX_SIZE 20 /** Size of a checkbox widget. */

GooeyCheckbox *GooeyCheckbox_Create(int x, int y, char *label,
                                    void (*callback)(bool checked, void *user_data), void *user_data)
{
    GooeyCheckbox *checkbox = (GooeyCheckbox *)calloc(1, sizeof(GooeyCheckbox));
    *checkbox = (GooeyCheckbox){0};

    if (!checkbox)
    {
        LOG_ERROR("Couldn't allocated memory for checkbox.");
        return NULL;
    }

    checkbox->core.type = WIDGET_CHECKBOX, checkbox->core.x = x;
    checkbox->core.y = y;
    checkbox->core.width = CHECKBOX_SIZE;
    checkbox->core.height = CHECKBOX_SIZE;
    checkbox->core.is_visible = true;
    checkbox->core.sprite = active_backend->CreateSpriteForWidget(x, y, CHECKBOX_SIZE, CHECKBOX_SIZE);
    checkbox->core.disable_input = false;

    if (label)
    {
        strncpy(checkbox->label, label, sizeof(checkbox->label) - 1);
    }
    else
    {
        strncpy(checkbox->label, "Checkbox", sizeof(checkbox->label) - 1);
    }

    checkbox->label[sizeof(checkbox->label) - 1] = '\0';
    checkbox->checked = false;
    checkbox->callback = callback;
    LOG_INFO("Checkbox added with dimensions x=%d, y=%d", x, y);

    return checkbox;
}
#endif

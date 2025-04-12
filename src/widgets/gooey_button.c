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

#include "widgets/gooey_button.h"
#include "backends/gooey_backend_internal.h"
#include "theme/gooey_theme.h"
#include "logger/pico_logger_internal.h"

void GooeyButton_SetText(GooeyButton *button, const char *text)
{
    if (!button)
    {
        LOG_ERROR("Widget<Button> Cannot be null \n");
        return;
    }

    strcpy(button->label, text);
}

GooeyButton *GooeyButton_Create(const char *label, int x, int y,
                                int width, int height, void (*callback)())
{
    GooeyButton *button = (GooeyButton*) malloc(sizeof(GooeyButton));
    
    if(!button) 
    {
        LOG_ERROR("Couldn't allocated memory for button");
        return NULL;
    }

    *button = (GooeyButton) {0};
    button->core.type = WIDGET_BUTTON;
    button->core.x = x;
    button->core.y = y;
    button->core.width = active_backend->GetTextWidth(label, strlen(label)) + 30;
    button->core.height = height;
    button->core.is_visible = true;
    strcpy(button->label, label);
    button->callback = callback;
    button->hover = false;
    button->clicked = false;
    button->is_highlighted = false;
    LOG_INFO("Button added with dimensions x=%d, y=%d, w=%d, h=%d.", x, y, width, height);
    return button;
}

void GooeyButton_SetHighlight(GooeyButton *button, bool is_highlighted)
{
    button->is_highlighted = true;
}

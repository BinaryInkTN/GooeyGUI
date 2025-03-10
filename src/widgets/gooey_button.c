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
#include "core/gooey_backend.h"
#include "theme/gooey_theme.h"

void GooeyButton_setText(GooeyButton *button, const char *text)
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

void GooeyButton_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->button_count; ++i)
    {
        GooeyButton *button = win->buttons[i];
        active_backend->FillRectangle(button->core.x,
                                      button->core.y, button->core.width, button->core.height, button->clicked ? win->active_theme->primary : win->active_theme->widget_base, win->creation_id);
        float text_width = active_backend->GetTextWidth(button->label, strlen(button->label));
        float text_height = active_backend->GetTextHeight(button->label, strlen(button->label));

        float text_x = button->core.x + (button->core.width - text_width) / 2;
        float text_y = button->core.y + (button->core.height + text_height) / 2;

        active_backend->DrawText(text_x,
                                 text_y, button->label, button->clicked ? win->active_theme->base : win->active_theme->neutral, 0.25f, win->creation_id);
        active_backend->SetForeground(win->active_theme->neutral);

        if (button->is_highlighted)
        {

            active_backend->DrawRectangle(button->core.x,
                                          button->core.y, button->core.width, button->core.height, win->active_theme->primary, win->creation_id);
        }
    }
}

bool GooeyButton_HandleClick(GooeyWindow *win, int x, int y)
{
    bool clicked_any_button = false;

    for (size_t i = 0; i < win->button_count; ++i)
    {
        GooeyButton *button = win->buttons[i];
        bool is_within_bounds = (x >= button->core.x && x <= button->core.x + button->core.width) &&
                                (y >= button->core.y && y <= button->core.y + button->core.height);

        if (is_within_bounds)
        {
            button->clicked = !button->clicked;
            clicked_any_button = true;
            if (button->callback)
            {
                button->callback();
            }
        }
        else
        {
            button->clicked = false;
        }
    }

    return clicked_any_button;
}

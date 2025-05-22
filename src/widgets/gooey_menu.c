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

#include "widgets/gooey_menu.h"
#if(ENABLE_MENU)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

GooeyMenu *GooeyMenu_Set(GooeyWindow *win)
{
    win->menu = (GooeyMenu *)calloc(1, sizeof(GooeyMenu));
    win->menu->children_count = 0;
    win->menu->is_busy = 0;
    LOG_INFO("Set menu for window.");

    return win->menu;
}

GooeyMenuChild *GooeyMenu_AddChild(GooeyWindow *win, char *title)
{
    if (!win->menu || win->menu->children_count >= MAX_MENU_CHILDREN)
    {
        LOG_ERROR("Unable to add child: Menu is full or not initialized.\n");
        return NULL;
    }

    win->menu->children[win->menu->children_count] = (GooeyMenuChild) {0};
    GooeyMenuChild *child = &win->menu->children[win->menu->children_count++];
    strcpy(child->title, title);
    child->menu_elements_count = 0;
    child->is_open = false;
    child->element_hovered_over = -1;
    LOG_INFO("Child added to menu with title=\"%s\"", title);

    return child;
}

void GooeyMenuChild_AddElement(GooeyMenuChild *child, char *title,
                               void (*callback)())
{
    child->menu_elements[child->menu_elements_count] = title;
    child->callbacks[child->menu_elements_count] = callback;
    child->menu_elements_count++;
    LOG_INFO("Element added to menu child with title=\"%s\"", title);
}
#endif

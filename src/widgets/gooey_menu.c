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
#if (ENABLE_MENU)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
#include "widgets/gooey_menu_internal.h"

GooeyMenu *GooeyMenu_Set(GooeyWindow *win)
{
    win->menu = (GooeyMenu *)calloc(1, sizeof(GooeyMenu));
    if (!win->menu)
    {
        LOG_ERROR("Failed to allocate memory for menu");
        return NULL;
    }
    
    win->menu->children_count = 0;
    win->menu->is_busy = false;
    LOG_INFO("Set menu for window.");

    return win->menu;
}

GooeyMenuChild *GooeyMenu_AddChild(GooeyWindow *win, char *title, void *user_data)
{
    if (!win->menu || win->menu->children_count >= MAX_MENU_CHILDREN)
    {
        LOG_ERROR("Unable to add child: Menu is full or not initialized.");
        return NULL;
    }

    // Initialize the child struct
    win->menu->children[win->menu->children_count] = (GooeyMenuChild){0};

    GooeyMenuChild *child = &win->menu->children[win->menu->children_count];
    child->child_user_data = user_data; // Store child-level user data
    strncpy(child->title, title, sizeof(child->title) - 1);
    child->title[sizeof(child->title) - 1] = '\0';
    child->menu_elements_count = 0;
    child->is_open = false;
    child->element_hovered_over = -1;
    child->is_animating = false;
    child->animation_height = 0;
    child->animation_step = 0;
    
    LOG_INFO("Child added to menu with title=\"%s\"", title);
    win->menu->children_count++;
    return child;
}

void GooeyMenuChild_AddElement(GooeyMenuChild *child, char *title,
                               void (*callback)(void *user_data), void *user_data)
{
    if (!child || child->menu_elements_count >= MAX_MENU_CHILDREN)
    {
        LOG_ERROR("Cannot add more elements to menu child: child is NULL or array is full");
        return;
    }
    
    int index = child->menu_elements_count;
    child->menu_elements[index] = title;
    child->callbacks[index] = callback;
    child->user_data[index] = user_data; // Store per-element user data

    child->menu_elements_count++;
    LOG_INFO("Element added to menu child with title=\"%s\"", title);
}
#endif
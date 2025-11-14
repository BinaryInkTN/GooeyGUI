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

#include "widgets/gooey_menu_internal.h"
#if(ENABLE_MENU)
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"
#include "core/gooey_timers_internal.h"
#include "animations/gooey_animations_internal.h"

static float ease_out_quad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

static void menu_animation_callback(void *user_data)
{
    GooeyMenuChild *child = (GooeyMenuChild *)user_data;
    if (!child)
        return;

    if (!child->is_animating)
        return;

    child->animation_step++;

    if (child->animation_step >= MENU_ANIMATION_STEPS)
    {
        child->animation_height = child->target_height;
        child->is_animating = false;

        if (child->animation_timer)
        {
            GooeyTimer_Stop_Internal(child->animation_timer);
        }
        return;
    }

    float progress = (float)child->animation_step / (float)MENU_ANIMATION_STEPS;
    float eased_progress = ease_out_quad(progress);

    int start_height = child->start_height;
    int target_height = child->target_height;
    int height_distance = target_height - start_height;
    child->animation_height = start_height + (int)(height_distance * eased_progress);

    if (child->animation_timer && child->is_animating)
    {
        GooeyTimer_SetCallback_Internal(MENU_ANIMATION_SPEED, child->animation_timer, menu_animation_callback, child);
    }
}

static void start_menu_animation(GooeyMenuChild *child, bool open)
{
    if (!child)
        return;

    child->target_height = open ? (25 * child->menu_elements_count) : 0;
    child->start_height = child->animation_height;
    child->animation_step = 0;
    child->is_animating = true;

    if (!child->animation_timer)
    {
        child->animation_timer = GooeyTimer_Create_Internal();
        if (!child->animation_timer)
        {
            child->animation_height = child->target_height;
            child->is_animating = false;
            return;
        }
    }

    GooeyTimer_SetCallback_Internal(MENU_ANIMATION_SPEED, child->animation_timer, menu_animation_callback, child);
}

void GooeyMenu_Draw(GooeyWindow *win)
{
    if (!win->menu)
        return;

    int window_width, window_height;
    active_backend->GetWinDim(&window_width, &window_height, win->creation_id);

    // Draw menu bar background
    active_backend->FillRectangle(0, 0, window_width, 20,
                                  win->active_theme->widget_base,
                                  win->creation_id, false, 0.0f, NULL);

    int x_offset = 10;

    for (int i = 0; i < win->menu->children_count; i++)
    {
        GooeyMenuChild *child = &win->menu->children[i];

        // Get width of the menu title text (scaled)
        int text_width = active_backend->GetTextWidth(child->title, strlen(child->title));

        // Draw menu bar item background if hovered/open/animating
        if (child->is_open || child->is_animating)
        {
            active_backend->FillRectangle(x_offset - 10, 0, text_width + 20, 20,
                                          win->active_theme->primary,
                                          win->creation_id, false, 0.0f, NULL);
        }

        // Draw menu title
        active_backend->DrawText(x_offset, 15, child->title,
                                 (child->is_open || child->is_animating) ?
                                 win->active_theme->base : win->active_theme->neutral,
                                 0.26f, win->creation_id, NULL);

        // Draw submenu if open or animating
        if (child->is_open || child->is_animating)
        {
            const int submenu_x = x_offset - 10;
            const int submenu_y = 20;

            // Compute submenu width as the widest element
            int submenu_width = text_width + 20;
            for (int j = 0; j < child->menu_elements_count; j++)
            {
                int element_width = active_backend->GetTextWidth(child->menu_elements[j], strlen(child->menu_elements[j])) + 20;
                if (element_width > submenu_width)
                    submenu_width = element_width;
            }

            // Compute current height (for animation)
            int current_height = child->is_animating ?
                                 child->animation_height :
                                 (child->is_open ? (25 * child->menu_elements_count) : 0);

            if (current_height > 0)
            {
                // Draw submenu background
                active_backend->FillRectangle(submenu_x, submenu_y,
                                              submenu_width, current_height,
                                              win->active_theme->widget_base,
                                              win->creation_id, true, 5.0f, NULL);

                // Draw submenu border
                active_backend->DrawRectangle(submenu_x, submenu_y,
                                              submenu_width, current_height,
                                              win->active_theme->primary, 0.5f,
                                              win->creation_id, true, 5.0f, NULL);

                // Draw each visible element
                int visible_elements = current_height / 25;
                if (visible_elements > child->menu_elements_count)
                    visible_elements = child->menu_elements_count;

                for (int j = 0; j < visible_elements; j++)
                {
                    const int element_y = submenu_y + (j * 25);
                    const bool is_hovered = (child->element_hovered_over == j);

                    // Highlight hovered element
                    if (is_hovered)
                    {
                        active_backend->FillRectangle(submenu_x, element_y,
                                                      submenu_width, 25,
                                                      win->active_theme->primary,
                                                      win->creation_id, true, 2.0f, NULL);
                    }

                    // Draw element text
                    active_backend->DrawText(submenu_x + 5, element_y + 18,
                                             child->menu_elements[j],
                                             is_hovered ? win->active_theme->base :
                                                          win->active_theme->neutral,
                                             0.26f, win->creation_id, NULL);

                    // Draw separator line between elements
                    if (j < visible_elements - 1 && j < child->menu_elements_count - 1)
                    {
                        active_backend->DrawLine(submenu_x, element_y + 24,
                                                 submenu_x + submenu_width, element_y + 24,
                                                 win->active_theme->neutral,
                                                 win->creation_id, NULL);
                    }
                }
            }
        }
        else
        {
            child->element_hovered_over = -1;
        }

        // Move to next menu child
        x_offset += text_width + 20;
    }
}

bool GooeyMenu_HandleHover(GooeyWindow *win)
{
    if (!win->menu)
        return false;

    GooeyEvent *event = (GooeyEvent *)win->current_event;
    const int x = event->mouse_move.x;
    const int y = event->mouse_move.y;
    bool is_any_element_hovered_over = false;
    static bool was_hovered_over = false;

    int x_offset = 10;
    for (int i = 0; i < win->menu->children_count; i++)
    {
        GooeyMenuChild *child = &win->menu->children[i];

        if (child->is_open || child->is_animating)
        {
            const int submenu_x = x_offset - 10;
            const int submenu_y = 20;
            const int submenu_width = 150;
            const int current_height = child->is_animating ? child->animation_height :
                                  (child->is_open ? (25 * child->menu_elements_count) : 0);

            child->element_hovered_over = -1;

            if (current_height > 0 &&
                x >= submenu_x && x <= submenu_x + submenu_width &&
                y >= submenu_y && y <= submenu_y + current_height)
            {
                int element_index = (y - submenu_y) / 25;
                if (element_index < child->menu_elements_count)
                {
                    child->element_hovered_over = element_index;
                    is_any_element_hovered_over = true;
                }
            }
        }

        x_offset += active_backend->GetTextWidth(child->title, strlen(child->title)) + 20;
    }

    if (is_any_element_hovered_over != was_hovered_over)
    {
        active_backend->CursorChange(is_any_element_hovered_over ? GOOEY_CURSOR_HAND : GOOEY_CURSOR_ARROW);
        was_hovered_over = is_any_element_hovered_over;
    }

    return is_any_element_hovered_over;
}

bool GooeyMenu_HandleClick(GooeyWindow *win, int x, int y)
{
    if (!win->menu)
        return false;

    bool handled = false;
    int x_offset = 10;

    // Check if click is on menu bar items
    for (int i = 0; i < win->menu->children_count && !handled; i++)
    {
        GooeyMenuChild *child = &win->menu->children[i];
        const int text_width = active_backend->GetTextWidth(child->title, strlen(child->title));

        // Check if click is on menu bar item
        if (y <= 20 && x >= x_offset && x <= x_offset + text_width)
        {
            bool new_open_state = !child->is_open;

            // Close all other menus
            for (int k = 0; k < win->menu->children_count; k++)
            {
                if (k != i)
                {
                    if (win->menu->children[k].is_open || win->menu->children[k].is_animating)
                    {
                        win->menu->children[k].is_open = false;
                        start_menu_animation(&win->menu->children[k], false);
                    }
                }
            }

            // Toggle current menu
            child->is_open = new_open_state;
            start_menu_animation(child, new_open_state);
            win->menu->is_busy = new_open_state;
            handled = true;
            break;
        }

        x_offset += text_width + 20;
    }

    // Check if click is on submenu items
    if (!handled)
    {
        x_offset = 10;
        for (int i = 0; i < win->menu->children_count && !handled; i++)
        {
            GooeyMenuChild *child = &win->menu->children[i];
            const int text_width = active_backend->GetTextWidth(child->title, strlen(child->title));

            if (child->is_open || child->is_animating)
            {
                const int submenu_x = x_offset - 10;
                const int submenu_y = 20;
                const int submenu_width = 150;
                const int current_height = child->is_animating ? child->animation_height :
                                      (child->is_open ? (25 * child->menu_elements_count) : 0);

                if (current_height > 0 &&
                    x >= submenu_x && x <= submenu_x + submenu_width &&
                    y >= submenu_y && y <= submenu_y + current_height)
                {
                    int element_index = (y - submenu_y) / 25;
                    if (element_index < child->menu_elements_count)
                    {
                        // FIXED: Properly call the callback with the correct user_data
                        if (child->callbacks[element_index])
                        {
                            child->callbacks[element_index](child->user_data[element_index]);
                        }

                        for (int k = 0; k < win->menu->children_count; k++)
                        {
                            if (win->menu->children[k].is_open || win->menu->children[k].is_animating)
                            {
                                win->menu->children[k].is_open = false;
                                start_menu_animation(&win->menu->children[k], false);
                            }
                        }
                        win->menu->is_busy = false;
                        handled = true;
                    }
                }
                else if (child->is_open && !child->is_animating)
                {
                    // Click outside open menu - close it
                    child->is_open = false;
                    start_menu_animation(child, false);
                    win->menu->is_busy = false;
                    handled = true;
                }
            }

            x_offset += text_width + 20;
        }
    }

    return handled;
}

void GooeyMenu_Cleanup(GooeyMenu *menu)
{
    if (!menu)
        return;

    for (int i = 0; i < menu->children_count; i++)
    {
        GooeyMenuChild *child = &menu->children[i];

        if (child->animation_timer)
        {
            GooeyTimer_Stop_Internal(child->animation_timer);
            GooeyTimer_Destroy_Internal(child->animation_timer);
            child->animation_timer = NULL;
        }

        child->is_animating = false;
    }
}
#endif
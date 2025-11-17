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

// Constants
#define MENU_BAR_HEIGHT 20
#define MENU_ITEM_PADDING 10
#define SUBMENU_ITEM_HEIGHT 25
#define SUBMENU_PADDING 5
#define MENU_ANIMATION_STEPS 10
#define MENU_ANIMATION_SPEED 16  // ~60 FPS

// Forward declarations
static void cleanup_menu_child(GooeyMenuChild *child);

static float ease_out_quad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

static void menu_animation_callback(void *user_data)
{
    GooeyMenuChild *child = (GooeyMenuChild *)user_data;
    if (!child || !child->is_animating) return;

    child->animation_step++;

    if (child->animation_step >= MENU_ANIMATION_STEPS)
    {
        // Animation complete
        child->animation_height = child->target_height;
        child->is_animating = false;
        
        if (child->animation_timer)
        {
            GooeyTimer_Stop_Internal(child->animation_timer);
        }
        return;
    }

    // Calculate animated height
    float progress = (float)child->animation_step / (float)MENU_ANIMATION_STEPS;
    float eased_progress = ease_out_quad(progress);
    int height_distance = child->target_height - child->start_height;
    child->animation_height = child->start_height + (int)(height_distance * eased_progress);

    // Continue animation if still active
    if (child->is_animating && child->animation_timer)
    {
        GooeyTimer_SetCallback_Internal(MENU_ANIMATION_SPEED, child->animation_timer, 
                                       menu_animation_callback, child);
    }
}

static void start_menu_animation(GooeyMenuChild *child, bool open)
{
    if (!child) return;

    child->target_height = open ? (SUBMENU_ITEM_HEIGHT * child->menu_elements_count) : 0;
    child->start_height = child->animation_height;
    child->animation_step = 0;
    child->is_animating = true;

    // Create timer if needed
    if (!child->animation_timer)
    {
        child->animation_timer = GooeyTimer_Create_Internal();
        if (!child->animation_timer)
        {
            // Fallback: set directly without animation
            child->animation_height = child->target_height;
            child->is_animating = false;
            return;
        }
    }

    GooeyTimer_SetCallback_Internal(MENU_ANIMATION_SPEED, child->animation_timer, 
                                   menu_animation_callback, child);
}

static int calculate_submenu_width(GooeyMenuChild *child, int base_width)
{
    int max_width = base_width;
    for (int j = 0; j < child->menu_elements_count; j++)
    {
        int element_width = active_backend->GetTextWidth(child->menu_elements[j], 
                                                        strlen(child->menu_elements[j])) 
                          + (MENU_ITEM_PADDING * 2);
        if (element_width > max_width)
            max_width = element_width;
    }
    return max_width;
}

static void draw_menu_bar_background(GooeyWindow *win, int width, int height)
{
    active_backend->FillRectangle(0, 0, width, MENU_BAR_HEIGHT,
                                  win->active_theme->widget_base,
                                  win->creation_id, false, 0.0f, NULL);
}

static void draw_menu_item(GooeyWindow *win, GooeyMenuChild *child, int x, int y, 
                          bool is_active, int text_width)
{
    // Draw background if active
    if (is_active)
    {
        active_backend->FillRectangle(x - MENU_ITEM_PADDING, y, 
                                      text_width + (MENU_ITEM_PADDING * 2), MENU_BAR_HEIGHT,
                                      win->active_theme->primary,
                                      win->creation_id, false, 0.0f, NULL);
    }

    // Draw menu title
    active_backend->DrawText(x, y + 15, child->title,
                             is_active ? win->active_theme->base : win->active_theme->neutral,
                             18.0f, win->creation_id, NULL);
}

static void draw_submenu(GooeyWindow *win, GooeyMenuChild *child, int x, int y, 
                        int base_width, int current_height)
{
    if (current_height <= 0) return;

    int submenu_width = calculate_submenu_width(child, base_width + (MENU_ITEM_PADDING * 2));
    int visible_elements = current_height / SUBMENU_ITEM_HEIGHT;
    if (visible_elements > child->menu_elements_count)
        visible_elements = child->menu_elements_count;

    // Draw submenu background and border
    active_backend->FillRectangle(x, y, submenu_width, current_height,
                                  win->active_theme->widget_base,
                                  win->creation_id, true, 5.0f, NULL);

    active_backend->DrawRectangle(x, y, submenu_width, current_height,
                                  win->active_theme->primary, 0.5f,
                                  win->creation_id, true, 5.0f, NULL);

    // Draw menu elements
    for (int j = 0; j < visible_elements; j++)
    {
        int element_y = y + (j * SUBMENU_ITEM_HEIGHT);
        bool is_hovered = (child->element_hovered_over == j);

        // Highlight hovered element
        if (is_hovered)
        {
            active_backend->FillRectangle(x, element_y, submenu_width, SUBMENU_ITEM_HEIGHT,
                                          win->active_theme->primary,
                                          win->creation_id, true, 2.0f, NULL);
        }

        // Draw element text
        active_backend->DrawText(x + MENU_ITEM_PADDING, element_y + 18,
                                 child->menu_elements[j],
                                 is_hovered ? win->active_theme->base : win->active_theme->neutral,
                                 18.0f, win->creation_id, NULL);

        // Draw separator (except for last element)
        if (j < visible_elements - 1 && j < child->menu_elements_count - 1)
        {
            active_backend->DrawLine(x, element_y + SUBMENU_ITEM_HEIGHT - 1,
                                     x + submenu_width, element_y + SUBMENU_ITEM_HEIGHT - 1,
                                     win->active_theme->neutral,
                                     win->creation_id, NULL);
        }
    }
}

void GooeyMenu_Draw(GooeyWindow *win)
{
    if (!win->menu) return;

    int window_width, window_height;
    active_backend->GetWinDim(&window_width, &window_height, win->creation_id);

    // Draw menu bar background
    draw_menu_bar_background(win, window_width, window_height);

    int x_offset = MENU_ITEM_PADDING;

    for (int i = 0; i < win->menu->children_count; i++)
    {
        GooeyMenuChild *child = &win->menu->children[i];
        int text_width = active_backend->GetTextWidth(child->title, strlen(child->title));
        bool is_active = (child->is_open || child->is_animating);

        // Draw menu item
        draw_menu_item(win, child, x_offset, 0, is_active, text_width);

        // Draw submenu if active
        if (is_active)
        {
            int current_height = child->is_animating ? child->animation_height :
                                (child->is_open ? (SUBMENU_ITEM_HEIGHT * child->menu_elements_count) : 0);
            
            draw_submenu(win, child, x_offset - MENU_ITEM_PADDING, MENU_BAR_HEIGHT, 
                        text_width, current_height);
        }
        else
        {
            child->element_hovered_over = -1;
        }

        x_offset += text_width + (MENU_ITEM_PADDING * 2);
    }
}

bool GooeyMenu_HandleHover(GooeyWindow *win)
{
    if (!win->menu) return false;

    GooeyEvent *event = (GooeyEvent *)win->current_event;
    int x = event->mouse_move.x;
    int y = event->mouse_move.y;
    bool is_any_element_hovered = false;
    static bool was_hovered = false;

    int x_offset = MENU_ITEM_PADDING;

    for (int i = 0; i < win->menu->children_count; i++)
    {
        GooeyMenuChild *child = &win->menu->children[i];
        child->element_hovered_over = -1;

        if (child->is_open || child->is_animating)
        {
            int text_width = active_backend->GetTextWidth(child->title, strlen(child->title));
            int submenu_x = x_offset - MENU_ITEM_PADDING;
            int submenu_y = MENU_BAR_HEIGHT;
            int submenu_width = calculate_submenu_width(child, text_width + (MENU_ITEM_PADDING * 2));
            int current_height = child->is_animating ? child->animation_height :
                                (child->is_open ? (SUBMENU_ITEM_HEIGHT * child->menu_elements_count) : 0);

            if (current_height > 0 &&
                x >= submenu_x && x <= submenu_x + submenu_width &&
                y >= submenu_y && y <= submenu_y + current_height)
            {
                int element_index = (y - submenu_y) / SUBMENU_ITEM_HEIGHT;
                if (element_index < child->menu_elements_count)
                {
                    child->element_hovered_over = element_index;
                    is_any_element_hovered = true;
                }
            }
        }

        x_offset += active_backend->GetTextWidth(child->title, strlen(child->title)) + (MENU_ITEM_PADDING * 2);
    }

    // Update cursor if hover state changed
    if (is_any_element_hovered != was_hovered)
    {
        active_backend->CursorChange(is_any_element_hovered ? GOOEY_CURSOR_HAND : GOOEY_CURSOR_ARROW);
        was_hovered = is_any_element_hovered;
    }

    return is_any_element_hovered;
}

bool GooeyMenu_HandleClick(GooeyWindow *win, int x, int y)
{
    if (!win->menu) return false;

    bool handled = false;
    int x_offset = MENU_ITEM_PADDING;

    // Check menu bar items first
    for (int i = 0; i < win->menu->children_count && !handled; i++)
    {
        GooeyMenuChild *child = &win->menu->children[i];
        int text_width = active_backend->GetTextWidth(child->title, strlen(child->title));

        // Check if click is on menu bar item
        if (y <= MENU_BAR_HEIGHT && x >= x_offset && x <= x_offset + text_width)
        {
            bool new_open_state = !child->is_open;

            // Close all other menus
            for (int k = 0; k < win->menu->children_count; k++)
            {
                if (k != i && (win->menu->children[k].is_open || win->menu->children[k].is_animating))
                {
                    win->menu->children[k].is_open = false;
                    start_menu_animation(&win->menu->children[k], false);
                }
            }

            // Toggle current menu
            child->is_open = new_open_state;
            start_menu_animation(child, new_open_state);
            win->menu->is_busy = new_open_state;
            handled = true;
            break;
        }

        x_offset += text_width + (MENU_ITEM_PADDING * 2);
    }

    // Check submenu items if no menu bar item was clicked
    if (!handled)
    {
        x_offset = MENU_ITEM_PADDING;
        for (int i = 0; i < win->menu->children_count && !handled; i++)
        {
            GooeyMenuChild *child = &win->menu->children[i];
            
            if (child->is_open || child->is_animating)
            {
                int text_width = active_backend->GetTextWidth(child->title, strlen(child->title));
                int submenu_x = x_offset - MENU_ITEM_PADDING;
                int submenu_y = MENU_BAR_HEIGHT;
                int submenu_width = calculate_submenu_width(child, text_width + (MENU_ITEM_PADDING * 2));
                int current_height = child->is_animating ? child->animation_height :
                                    (child->is_open ? (SUBMENU_ITEM_HEIGHT * child->menu_elements_count) : 0);

                if (current_height > 0 &&
                    x >= submenu_x && x <= submenu_x + submenu_width &&
                    y >= submenu_y && y <= submenu_y + current_height)
                {
                    // Click on submenu item
                    int element_index = (y - submenu_y) / SUBMENU_ITEM_HEIGHT;
                    if (element_index < child->menu_elements_count && child->callbacks[element_index])
                    {
                        child->callbacks[element_index](child->user_data[element_index]);
                    }

                    // Close all menus
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
                else if (child->is_open && !child->is_animating)
                {
                    // Click outside open menu - close it
                    child->is_open = false;
                    start_menu_animation(child, false);
                    win->menu->is_busy = false;
                    handled = true;
                }
            }

            x_offset += active_backend->GetTextWidth(child->title, strlen(child->title)) + (MENU_ITEM_PADDING * 2);
        }
    }

    return handled;
}

static void cleanup_menu_child(GooeyMenuChild *child)
{
    if (!child) return;

    if (child->animation_timer)
    {
        GooeyTimer_Stop_Internal(child->animation_timer);
        GooeyTimer_Destroy_Internal(child->animation_timer);
        child->animation_timer = NULL;
    }

    child->is_animating = false;
}

void GooeyMenu_Cleanup(GooeyMenu *menu)
{
    if (!menu) return;

    for (int i = 0; i < menu->children_count; i++)
    {
        cleanup_menu_child(&menu->children[i]);
    }
}
#endif
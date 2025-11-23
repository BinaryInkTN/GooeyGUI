/*
 Copyright (c) 2025 Yassine Ahmed Ali

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

#include "widgets/gooey_notifications_internal.h"

#if (ENABLE_NOTIFICATIONS)
#include "backends/gooey_backend_internal.h"
#include "widgets/gooey_window_internal.h"
#include "logger/pico_logger_internal.h"
#include "core/gooey_timers_internal.h"
#include <string.h>

static float ease_out_quad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

static void notification_animation_callback(void *user_data)
{
    GooeyNotification *notification = (GooeyNotification *)user_data;
    if (!notification || !notification->is_animating)
        return;

    notification->animation_step++;

    if (notification->animation_step >= NOTIFICATION_ANIMATION_STEPS)
    {
        // Animation complete
        notification->is_animating = false;
        
        if (notification->animation_type == NOTIFICATION_ANIMATION_OUT)
        {
            notification->should_remove = true;
        }
        
        // Stop the timer - don't re-register
        if (notification->animation_timer)
        {
            GooeyTimer_Stop_Internal(notification->animation_timer);
        }
        return;
    }
    
    // Timer will continue to fire periodically - no need to re-register
}

static void start_notification_animation(GooeyNotification *notification, GooeyNotificationAnimationType animation_type)
{
    if (!notification)
        return;

    // Stop any existing animation
    if (notification->animation_timer && notification->is_animating)
    {
        GooeyTimer_Stop_Internal(notification->animation_timer);
    }

    notification->animation_type = animation_type;
    notification->animation_step = 0;
    notification->is_animating = true;

    if (!notification->animation_timer)
    {
        notification->animation_timer = GooeyTimer_Create_Internal();
        if (!notification->animation_timer)
        {
            notification->is_animating = false;
            if (animation_type == NOTIFICATION_ANIMATION_OUT)
            {
                notification->should_remove = true;
            }
            return;
        }
    }

    // Start the periodic timer
    GooeyTimer_SetCallback_Internal(NOTIFICATION_ANIMATION_SPEED, notification->animation_timer, 
                                   notification_animation_callback, notification);
}

static unsigned long get_notification_color(GooeyNotificationType type, GooeyTheme *theme)
{
    switch (type)
    {
    case NOTIFICATION_INFO:
        return theme->info;
    case NOTIFICATION_SUCCESS:
        return theme->success;
    case NOTIFICATION_WARNING:
        return theme->primary;
    case NOTIFICATION_ERROR:
        return theme->danger;
    default:
        return theme->neutral;
    }
}

static void get_notification_position(GooeyNotificationPosition position, int window_width, int window_height,
                                      int notification_index, int *x, int *y)
{
    int spacing = NOTIFICATION_HEIGHT + NOTIFICATION_SPACING;
    int total_height = (NOTIFICATION_HEIGHT + NOTIFICATION_SPACING) * notification_index + NOTIFICATION_PADDING;

    switch (position)
    {
    case NOTIFICATION_POSITION_TOP_LEFT:
        *x = NOTIFICATION_PADDING;
        *y = total_height;
        break;
    case NOTIFICATION_POSITION_TOP_RIGHT:
        *x = window_width - NOTIFICATION_WIDTH - NOTIFICATION_PADDING;
        *y = total_height;
        break;
    case NOTIFICATION_POSITION_BOTTOM_RIGHT:
        *x = window_width - NOTIFICATION_WIDTH - NOTIFICATION_PADDING;
        *y = window_height - total_height - NOTIFICATION_HEIGHT;
        break;
    default:
        *x = NOTIFICATION_PADDING;
        *y = total_height;
        break;
    }
}

static void wrap_notification_text(const char *message, char *wrapped_text, size_t wrapped_size, int max_width)
{
    if (!message || !wrapped_text) return;
    
    size_t msg_len = strlen(message);
    if (msg_len == 0) {
        wrapped_text[0] = '\0';
        return;
    }

    char temp[512];
    strncpy(temp, message, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    char *words[64];
    int word_count = 0;
    
    char *token = strtok(temp, " ");
    while (token && word_count < 63) {
        words[word_count++] = token;
        token = strtok(NULL, " ");
    }

    wrapped_text[0] = '\0';
    char current_line[256] = "";
    
    for (int i = 0; i < word_count; i++) {
        char test_line[256];
        if (strlen(current_line) == 0) {
            strcpy(test_line, words[i]);
        } else {
            snprintf(test_line, sizeof(test_line), "%s %s", current_line, words[i]);
        }

        int line_width = active_backend->GetTextWidth(test_line, strlen(test_line));
        if (line_width > max_width && strlen(current_line) > 0) {
            if (strlen(wrapped_text) > 0) {
                strcat(wrapped_text, "\n");
            }
            strcat(wrapped_text, current_line);
            strcpy(current_line, words[i]);
        } else {
            strcpy(current_line, test_line);
        }
    }

    if (strlen(current_line) > 0) {
        if (strlen(wrapped_text) > 0) {
            strcat(wrapped_text, "\n");
        }
        strcat(wrapped_text, current_line);
    }
}

void GooeyNotification_Internal_Draw(GooeyWindow *window)
{
    if (!window || !window->notification_manager || window->notification_manager->notification_count == 0)
    {
        return;
    }

    GooeyNotificationManager *manager = window->notification_manager;
    int window_width = window->width;
    int window_height = window->height;

    for (size_t i = 0; i < manager->notification_count; i++)
    {
        GooeyNotification *notification = manager->notifications[i];
        if (!notification)
            continue;

        // Use fixed width for all notifications
        const int notification_width = NOTIFICATION_WIDTH;

        int base_x, base_y;
        get_notification_position(notification->position, window_width, window_height, (int)i, &base_x, &base_y);

        // Calculate animation progress
        float animation_progress = 0.0f;
        if (notification->is_animating)
        {
            animation_progress = (float)notification->animation_step / (float)NOTIFICATION_ANIMATION_STEPS;
            animation_progress = ease_out_quad(animation_progress);
            
            if (notification->animation_type == NOTIFICATION_ANIMATION_OUT)
            {
                animation_progress = 1.0f - animation_progress;
            }
        }
        else
        {
            animation_progress = (notification->animation_type == NOTIFICATION_ANIMATION_IN && !notification->is_animating) ? 1.0f : 0.0f;
        }

        int slide_offset = 0;
        int slide_distance = 100;
        
        switch (notification->position)
        {
        case NOTIFICATION_POSITION_TOP_LEFT:
        case NOTIFICATION_POSITION_BOTTOM_LEFT:
            slide_offset = (int)(-slide_distance * (1.0f - animation_progress));
            break;
        case NOTIFICATION_POSITION_TOP_RIGHT:
        case NOTIFICATION_POSITION_BOTTOM_RIGHT:
            slide_offset = (int)(slide_distance * (1.0f - animation_progress));
            break;
        }

        int animated_x = base_x + slide_offset;
        int animated_y = base_y;

        unsigned long bg_color = get_notification_color(notification->type, window->active_theme);
        unsigned long text_color = window->active_theme->neutral;

        active_backend->FillRectangle(
            animated_x, animated_y,
            notification_width, NOTIFICATION_HEIGHT,
            bg_color,
            window->creation_id,
            false, 0.0f, NULL);

        active_backend->DrawRectangle(
            animated_x, animated_y,
            notification_width, NOTIFICATION_HEIGHT,
            text_color,
            1.0f,
            window->creation_id, false, 0.0f, NULL);

        char wrapped_text[512];
        int text_area_width = notification_width - (NOTIFICATION_PADDING * 3);
        wrap_notification_text(notification->message, wrapped_text, sizeof(wrapped_text), text_area_width);

        int text_x = animated_x + NOTIFICATION_PADDING;
        int text_y = animated_y + NOTIFICATION_HEIGHT / 2;

        active_backend->DrawGooeyText(
            text_x, text_y,
            wrapped_text,
            text_color,
            18.0f,
            window->creation_id, NULL);

        int close_button_size = 10;
        int close_x = animated_x + notification_width - close_button_size - NOTIFICATION_PADDING;
        int close_y = animated_y + NOTIFICATION_PADDING;

        active_backend->DrawLine(
            close_x, close_y,
            close_x + close_button_size, close_y + close_button_size,
            text_color,
            window->creation_id, NULL);

        active_backend->DrawLine(
            close_x + close_button_size, close_y,
            close_x, close_y + close_button_size,
            text_color,
            window->creation_id, NULL);
    }
}

bool GooeyNotification_Internal_HandleClick(GooeyWindow *window, int mouse_x, int mouse_y)
{
    if (!window || !window->notification_manager || window->notification_manager->notification_count == 0)
    {
        return false;
    }

    GooeyNotificationManager *manager = window->notification_manager;
    int window_width = window->width;
    int window_height = window->height;

    for (int i = (int)manager->notification_count - 1; i >= 0; i--)
    {
        GooeyNotification *notification = manager->notifications[i];
        if (!notification || notification->is_animating)
            continue;

        const int notification_width = NOTIFICATION_WIDTH;

        int base_x, base_y;
        get_notification_position(notification->position, window_width, window_height, i, &base_x, &base_y);

        int final_x = base_x;
        int final_y = base_y;

        if (mouse_x >= final_x && mouse_x <= final_x + notification_width &&
            mouse_y >= final_y && mouse_y <= final_y + NOTIFICATION_HEIGHT)
        {
            int close_button_size = 10;
            int close_x = final_x + notification_width - close_button_size - NOTIFICATION_PADDING;
            int close_y = final_y + NOTIFICATION_PADDING;

            if (mouse_x >= close_x && mouse_x <= close_x + close_button_size &&
                mouse_y >= close_y && mouse_y <= close_y + close_button_size)
            {
                start_notification_animation(notification, NOTIFICATION_ANIMATION_OUT);
                active_backend->RequestRedraw(window);
                return true;
            }

            return true;
        }
    }

    return false;
}

void GooeyNotification_Internal_Update(GooeyWindow *window)
{
    if (!window || !window->notification_manager || window->notification_manager->notification_count == 0)
    {
        return;
    }

    GooeyNotificationManager *manager = window->notification_manager;
    bool needs_redraw = false;

    for (int i = (int)manager->notification_count - 1; i >= 0; i--)
    {
        GooeyNotification *notification = manager->notifications[i];
        if (!notification)
            continue;

        if (notification->auto_dismiss && !notification->is_animating && 
            notification->animation_type == NOTIFICATION_ANIMATION_IN)
        {
            notification->display_time++;
            if (notification->display_time >= NOTIFICATION_ANIMATION_DURATION / NOTIFICATION_ANIMATION_SPEED)
            {
                start_notification_animation(notification, NOTIFICATION_ANIMATION_OUT);
                needs_redraw = true;
            }
        }

        if (notification->should_remove)
        {
            free((void *)notification->message);
            free(notification);

            for (size_t j = i; j < manager->notification_count - 1; j++)
            {
                manager->notifications[j] = manager->notifications[j + 1];
            }
            manager->notification_count--;
            needs_redraw = true;
        }
    }

    if (needs_redraw)
    {
        active_backend->RequestRedraw(window);
    }
}
#endif
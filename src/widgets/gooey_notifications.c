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

#include "widgets/gooey_notifications.h"
#if (ENABLE_NOTIFICATIONS)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
#include "widgets/gooey_notifications_internal.h"
#include "widgets/gooey_window_internal.h"
#include "core/gooey_timers_internal.h"
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

#include "widgets/gooey_notifications.h"
#if (ENABLE_NOTIFICATIONS)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
#include "widgets/gooey_notifications_internal.h"
#include "widgets/gooey_window_internal.h"
#include "core/gooey_timers_internal.h"

void GooeyNotification_Init(GooeyWindow *window)
{
    if (!window)
    {
        LOG_ERROR("Cannot initialize notifications: window is NULL");
        return;
    }

    if (window->notification_manager)
    {
        return;
    }

    LOG_ERROR("Notification manager not allocated properly");
}

void GooeyNotifications_Run(GooeyWindow *window, const char *message, GooeyNotificationType type, GooeyNotificationPosition position)
{
    if (!window || !window->notification_manager || !message)
    {
        LOG_ERROR("Cannot run notification: invalid parameters");
        return;
    }

    GooeyNotificationManager *manager = window->notification_manager;

    if (manager->notification_count >= MAX_NOTIFICATIONS)
    {
        LOG_WARNING("Maximum notification limit reached");
        return;
    }

    GooeyNotification *notification = calloc(1, sizeof(GooeyNotification));
    if (!notification)
    {
        LOG_ERROR("Failed to allocate memory for notification");
        return;
    }

    notification->message = strdup(message);
    if (!notification->message)
    {
        LOG_ERROR("Failed to duplicate notification message");
        free(notification);
        return;
    }

    notification->type = type;
    notification->position = position;
    notification->display_time = 0;
    notification->auto_dismiss = true;
    notification->should_remove = false;
    notification->animation_type = NOTIFICATION_ANIMATION_IN;
    notification->is_animating = false;
    notification->animation_step = 0;
    notification->animation_timer = NULL;
    
    manager->notifications[manager->notification_count++] = notification;
    active_backend->RequestRedraw(window);
}

void GooeyNotification_Cleanup(GooeyWindow *window)
{
    if (!window || !window->notification_manager)
    {
        return;
    }

    GooeyNotificationManager *manager = window->notification_manager;
    
    for (size_t i = 0; i < manager->notification_count; i++)
    {
        GooeyNotification *notification = manager->notifications[i];
        if (notification)
        {
            if (notification->animation_timer)
            {
                GooeyTimer_Stop_Internal(notification->animation_timer);
                GooeyTimer_Destroy_Internal(notification->animation_timer);
                notification->animation_timer = NULL;
            }
            
            free((void *)notification->message);
            free(notification);
            manager->notifications[i] = NULL;
        }
    }
    
    manager->notification_count = 0;
    
}
#endif
#endif
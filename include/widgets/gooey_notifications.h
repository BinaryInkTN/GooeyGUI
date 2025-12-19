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

#ifndef GOOEY_NOTIFICATIONS_H
#define GOOEY_NOTIFICATIONS_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_NOTIFICATIONS)


void GooeyNotification_Init(GooeyWindow *window);
void GooeyNotifications_Run(GooeyWindow *window, const char *message, GooeyNotificationType type, GooeyNotificationPosition position);
void GooeyNotification_Cleanup(GooeyWindow *window);

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* GOOEY_NOTIFICATIONS_H */
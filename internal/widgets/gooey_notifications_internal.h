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

#ifndef GOOEY_NOTIFICATIONS_INTERNAL_H
#define GOOEY_NOTIFICATIONS_INTERNAL_H

#include "common/gooey_common.h"

#if (ENABLE_NOTIFICATIONS)

#include "widgets/gooey_window_internal.h"

void GooeyNotification_Internal_Draw(GooeyWindow *window);
bool GooeyNotification_Internal_HandleClick(GooeyWindow *window, int mouse_x, int mouse_y);
void GooeyNotification_Internal_Update(GooeyWindow *window);

#endif

#endif /* GOOEY_NOTIFICATIONS_INTERNAL_H */
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


#include "core/gooey_timers_internal.h"
#include "backends/gooey_backend_internal.h"

GooeyTimer *GooeyTimer_Create_Internal()
{
    GooeyTimer *timer = active_backend->CreateTimer();
    return timer;
}

void GooeyTimer_SetCallback_Internal(uint64_t time, GooeyTimer *timer, void (*callback)(void* user_data), void *user_data)
{
    active_backend->SetTimerCallback(time, timer, callback, user_data);
}

void GooeyTimer_Stop_Internal(GooeyTimer *timer)
{
    active_backend->StopTimer(timer);
}

void GooeyTimer_Destroy_Internal(GooeyTimer *timer)
{
    active_backend->DestroyTimer(timer);
}

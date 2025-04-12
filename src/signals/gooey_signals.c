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

#include "signals/gooey_signals.h"
#include "logger/pico_logger_internal.h"

#include <stdlib.h>

GooeySignal GooeySignal_Create(void)
{
    GooeySignal signal;
    signal.slots = NULL;
    return signal;
}

void GooeySignal_Link(GooeySignal *signal, GooeySignal_CallbackFunction callback, void *context)
{
    GooeySignal_Slot *slot = (GooeySignal_Slot *)malloc(sizeof(GooeySignal_Slot));
    if (!slot)
    {
        LOG_ERROR("Couldn't allocate memory to signal slot with context \"%s\".", context);
    }
    slot->callback = callback;
    slot->context = context;
    slot->next = signal->slots;
    signal->slots = slot;
}

void GooeySignal_Emit(GooeySignal *signal, void *data)
{
    if (signal->slots)
        for (GooeySignal_Slot *slot = signal->slots; slot != NULL; slot = slot->next)
        {
            if (slot->callback)
                slot->callback(slot->context, data);
        }
}

void GooeySignal_UnLinkAll(GooeySignal *signal)
{
    GooeySignal_Slot *slot = signal->slots;
    while (slot != NULL)
    {
        slot = slot->next;
    }
    free(slot);
    signal->slots = NULL;
}
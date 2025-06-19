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

#include "core/gooey_widget_internal.h"
#include "logger/pico_logger_internal.h"

void GooeyWidget_MakeVisible(void *widget, bool state)
{
    GooeyWidget_MakeVisible_Internal(widget, state);
}

void GooeyWidget_MoveTo(void *widget, int x, int y)
{
    GooeyWidget_MoveTo_Internal(widget, x, y);
}

void GooeyWidget_Resize(void *widget, int w, int h)
{
    GooeyWidget_Resize_Internal(widget, w, h);
}
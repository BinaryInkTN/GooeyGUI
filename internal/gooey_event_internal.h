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

#ifndef GOOEY_EVENT_INTERNAL_H
#define GOOEY_EVENT_INTERNAL_H

typedef enum
{
    GOOEY_EVENT_CLICK_PRESS,
    GOOEY_EVENT_CLICK_RELEASE,
    GOOEY_EVENT_MOUSE_MOVE,
    GOOEY_EVENT_MOUSE_SCROLL,
    GOOEY_EVENT_KEY_PRESS,
    GOOEY_EVENT_KEY_RELEASE,
    GOOEY_EVENT_WINDOW_CLOSE,
    GOOEY_EVENT_EXPOSE,
    GOOEY_EVENT_RESIZE,
    GOOEY_EVENT_REDRAWREQ
} GooeyEventType;

typedef struct
{
    int x, y;

} GooeyMouseData;

typedef struct
{
    unsigned int state;
    char value[20];

} GooeyKeyPressData;

typedef struct
{
    GooeyEventType type;
    int attached_window;

    GooeyMouseData click;
    GooeyMouseData mouse_move;
    GooeyMouseData mouse_scroll;
    GooeyKeyPressData key_press;

} GooeyEvent;



#endif
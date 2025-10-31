/*
Copyright (c) 2024 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef GOOEY_NODE_EDITOR_INTERNAL_H
#define GOOEY_NODE_EDITOR_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_NODE_EDITOR)

#include <stdbool.h>

    // Internal functions
    bool GooeyNodeEditor_HandleClick(GooeyWindow* win, int x, int y);
    bool GooeyNodeEditor_HandleRelease(GooeyWindow* win, int x, int y);
    bool GooeyNodeEditor_HandleHover(GooeyWindow* win, int x, int y);
    bool GooeyNodeEditor_HandleDrag(GooeyWindow* win, int x, int y, int dx, int dy);
    void GooeyNodeEditor_Draw(GooeyWindow* win);
    GooeyNodeConnection* GooeyNodeEditor_Internal_ConnectSockets(GooeyNodeEditor* editor, GooeyNodeSocket* from, GooeyNodeSocket* to);
    void GooeyNodeEditor_Internal_Clear(GooeyNodeEditor* editor);

    #endif // ENABLE_NODE_EDITOR

#ifdef __cplusplus
}
#endif

#endif // GOOEY_NODE_EDITOR_INTERNAL_H
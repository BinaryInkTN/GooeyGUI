/*
Copyright (c) 2024 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#ifndef GOOEY_NODE_EDITOR_H
#define GOOEY_NODE_EDITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_NODE_EDITOR)

#include <stdbool.h>

    // Public API
    GooeyNodeEditor* GooeyNodeEditor_Create(int x, int y, int width, int height, void (*callback)(void *user_data), void *user_data);
    void GooeyNodeEditor_AddNode(GooeyNodeEditor* editor, const char* title, int x, int y, int width, int height);
    void GooeyNodeEditor_RemoveNode(GooeyNodeEditor* editor, const char* node_id);
    void GooeyNodeEditor_Clear(GooeyNodeEditor* editor);
    void GooeyNodeEditor_SetGridSize(GooeyNodeEditor* editor, int grid_size);
    void GooeyNodeEditor_SetShowGrid(GooeyNodeEditor* editor, bool show_grid);
    GooeyNodeSocket* GooeyNode_AddSocket(GooeyNode* node, const char* name, GooeySocketType type, GooeyDataType data_type);
    GooeyNodeConnection* GooeyNodeEditor_ConnectSockets(GooeyNodeEditor* editor, GooeyNodeSocket* from, GooeyNodeSocket* to);

#endif // ENABLE_NODE_EDITOR

#ifdef __cplusplus
}
#endif

#endif // GOOEY_NODE_EDITOR_H
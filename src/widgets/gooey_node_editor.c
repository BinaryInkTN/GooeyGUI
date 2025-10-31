/*
 Copyright (c) 2024 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "widgets/gooey_node_editor.h"
#if (ENABLE_NODE_EDITOR)
#include "backends/gooey_backend_internal.h"
#include "widgets/gooey_node_editor_internal.h"
#include "theme/gooey_theme.h"
#include "logger/pico_logger_internal.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NODE_HEADER_HEIGHT 25
#define SOCKET_RADIUS 5
#define SOCKET_MARGIN 10

static int node_id_counter = 0;

GooeyNodeEditor* GooeyNodeEditor_Create(int x, int y, int width, int height, void (*callback)(void *user_data), void *user_data)
{
    GooeyNodeEditor* editor = (GooeyNodeEditor*)calloc(1, sizeof(GooeyNodeEditor));

    if (!editor)
    {
        LOG_ERROR("Couldn't allocate memory for node editor");
        return NULL;
    }

    *editor = (GooeyNodeEditor){0};
    editor->core.type = WIDGET_NODE_EDITOR;
    editor->core.x = x;
    editor->core.y = y;
    editor->core.width = width;
    editor->core.height = height;
    editor->core.is_visible = true;
    editor->grid_size = 20;
    editor->show_grid = true;
    editor->zoom_level = 1.0f;
    editor->pan_x = 0;
    editor->pan_y = 0;
    editor->callback = callback;
    editor->user_data = user_data;
    editor->core.disable_input = false;

    editor->core.sprite = active_backend->CreateSpriteForWidget(x, y, width, height);
    LOG_INFO("NodeEditor added with dimensions x=%d, y=%d, w=%d, h=%d.", x, y, width, height);
    return editor;
}

GooeyNodeSocket* GooeyNode_AddSocket(GooeyNode* node, const char* name, GooeySocketType type, GooeyDataType data_type)
{
    if (!node) return NULL;

    // Allocate or reallocate sockets array
    GooeyNodeSocket* new_sockets = (GooeyNodeSocket*)realloc(node->sockets, sizeof(GooeyNodeSocket) * (node->socket_count + 1));
    if (!new_sockets) return NULL;

    node->sockets = new_sockets;
    GooeyNodeSocket* socket = &node->sockets[node->socket_count];

    // Initialize socket
    snprintf(socket->id, sizeof(socket->id), "socket_%d", node->socket_count);
    strncpy(socket->name, name, sizeof(socket->name) - 1);
    socket->type = type;
    socket->data_type = data_type;
    socket->is_connected = false;
    socket->parent_node = node;

    // Position sockets vertically
    socket->x = (type == GOOEY_SOCKET_TYPE_INPUT) ? 0 : node->width;
    socket->y = NODE_HEADER_HEIGHT + (node->socket_count * 20) + 20;

    node->socket_count++;
    return socket;
}

void GooeyNodeEditor_AddNode(GooeyNodeEditor* editor, const char* title, int x, int y, int width, int height)
{
    if (!editor) return;

    GooeyNode* node = (GooeyNode*)calloc(1, sizeof(GooeyNode));
    if (!node) return;

    snprintf(node->node_id, sizeof(node->node_id), "node_%d", node_id_counter++);
    strncpy(node->title, title, sizeof(node->title) - 1);
    node->x = x;
    node->y = y;
    node->width = width;
    node->height = height;
    node->is_selected = false;
    node->is_dragging = false;
    node->sockets = NULL;
    node->socket_count = 0;

    // Add to editor
    editor->nodes = (GooeyNode**)realloc(editor->nodes, sizeof(GooeyNode*) * (editor->node_count + 1));
    editor->nodes[editor->node_count] = node;
    editor->node_count++;
}

GooeyNodeConnection* GooeyNodeEditor_ConnectSockets(GooeyNodeEditor* editor, GooeyNodeSocket* from, GooeyNodeSocket* to)
{

    return  GooeyNodeEditor_Internal_ConnectSockets(editor, from, to);
}

void GooeyNodeEditor_RemoveNode(GooeyNodeEditor* editor, const char* node_id)
{
    if (!editor || !node_id) return;

    for (int i = 0; i < editor->node_count; i++) {
        if (strcmp(editor->nodes[i]->node_id, node_id) == 0) {
            // Remove connections to this node
            for (int j = 0; j < editor->connection_count; j++) {
                GooeyNodeConnection* conn = editor->connections[j];
                if (conn->from_socket->parent_node == editor->nodes[i] ||
                    conn->to_socket->parent_node == editor->nodes[i]) {
                    free(conn);
                    // Shift remaining connections
                    for (int k = j; k < editor->connection_count - 1; k++) {
                        editor->connections[k] = editor->connections[k + 1];
                    }
                    editor->connection_count--;
                    j--;
                }
            }

            // Free node sockets
            free(editor->nodes[i]->sockets);
            free(editor->nodes[i]);

            // Shift remaining nodes
            for (int j = i; j < editor->node_count - 1; j++) {
                editor->nodes[j] = editor->nodes[j + 1];
            }
            editor->node_count--;
            break;
        }
    }
}

void GooeyNodeEditor_RemoveConnection(GooeyNodeEditor* editor, GooeyNodeConnection* connection)
{
    if (!editor || !connection) return;

    for (int i = 0; i < editor->connection_count; i++) {
        if (editor->connections[i] == connection) {
            // Mark sockets as disconnected
            connection->from_socket->is_connected = false;
            connection->to_socket->is_connected = false;

            free(connection);

            // Shift remaining connections
            for (int j = i; j < editor->connection_count - 1; j++) {
                editor->connections[j] = editor->connections[j + 1];
            }
            editor->connection_count--;
            break;
        }
    }
}

void GooeyNodeEditor_Clear(GooeyNodeEditor* editor)
{
   GooeyNodeEditor_Internal_Clear(editor);
}

void GooeyNodeEditor_SetGridSize(GooeyNodeEditor* editor, int grid_size)
{
    if (!editor) return;
    editor->grid_size = grid_size;
}

void GooeyNodeEditor_SetShowGrid(GooeyNodeEditor* editor, bool show_grid)
{
    if (!editor) return;
    editor->show_grid = show_grid;
}

#endif
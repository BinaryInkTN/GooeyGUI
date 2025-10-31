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
#define SOCKET_VERTICAL_SPACING 20
#define MIN_NODE_WIDTH 100
#define MIN_NODE_HEIGHT 80

static int node_id_counter = 0;

GooeyNodeEditor* GooeyNodeEditor_Create(int x, int y, int width, int height,
                                      void (*callback)(void *user_data), void *user_data)
{
    if (width <= 0 || height <= 0) {
        LOG_ERROR("Invalid node editor dimensions: %dx%d", width, height);
        return NULL;
    }

    GooeyNodeEditor* editor = (GooeyNodeEditor*)calloc(1, sizeof(GooeyNodeEditor));
    if (!editor) {
        LOG_ERROR("Couldn't allocate memory for node editor");
        return NULL;
    }

    // Initialize all fields
    editor->core.type = WIDGET_NODE_EDITOR;
    editor->core.x = x;
    editor->core.y = y;
    editor->core.width = width;
    editor->core.height = height;
    editor->core.is_visible = true;
    editor->core.disable_input = false;

    editor->grid_size = 20;
    editor->show_grid = true;
    editor->zoom_level = 1.0f;
    editor->pan_x = 0;
    editor->pan_y = 0;
    editor->callback = callback;
    editor->user_data = user_data;

    editor->nodes = NULL;
    editor->node_count = 0;
    editor->connections = NULL;
    editor->connection_count = 0;
    editor->dragging_socket = NULL;
    editor->is_panning = false;

    editor->core.sprite = active_backend->CreateSpriteForWidget(x, y, width, height);
    if (!editor->core.sprite) {
        LOG_WARNING("Failed to create sprite for node editor");
    }

    LOG_INFO("NodeEditor created at (%d, %d) with dimensions %dx%d", x, y, width, height);
    return editor;
}

GooeyNodeSocket* GooeyNode_AddSocket(GooeyNode* node, const char* name, GooeySocketType type, GooeyDataType data_type)
{
    if (!node || !name) {
        LOG_ERROR("Invalid parameters for AddSocket");
        return NULL;
    }

    // Calculate required height based on socket count
    const int required_height = NODE_HEADER_HEIGHT + ((node->socket_count + 1) * SOCKET_VERTICAL_SPACING) + 20;
    if (required_height > node->height) {
        node->height = required_height;
    }

    // Reallocate sockets array
    GooeyNodeSocket* new_sockets = (GooeyNodeSocket*)realloc(
        node->sockets,
        sizeof(GooeyNodeSocket) * (node->socket_count + 1)
    );

    if (!new_sockets) {
        LOG_ERROR("Failed to reallocate sockets array");
        return NULL;
    }

    node->sockets = new_sockets;
    GooeyNodeSocket* socket = &node->sockets[node->socket_count];

    // Initialize socket
    snprintf(socket->id, sizeof(socket->id), "socket_%d", node->socket_count);
    strncpy(socket->name, name, sizeof(socket->name) - 1);
    socket->name[sizeof(socket->name) - 1] = '\0'; // Ensure null termination

    socket->type = type;
    socket->data_type = data_type;
    socket->is_connected = false;
    socket->parent_node = node;

    // Position sockets - inputs on left, outputs on right
    socket->x = (type == GOOEY_SOCKET_TYPE_INPUT) ? 0 : node->width;
    socket->y = NODE_HEADER_HEIGHT + (node->socket_count * SOCKET_VERTICAL_SPACING) + 10;

    node->socket_count++;

    LOG_INFO("Added socket '%s' to node '%s'", name, node->title);
    return socket;
}

void GooeyNodeEditor_AddNode(GooeyNodeEditor* editor, const char* title, int x, int y, int width, int height)
{
    if (!editor || !title) {
        LOG_ERROR("Invalid parameters for AddNode");
        return;
    }

    // Validate dimensions
    if (width < MIN_NODE_WIDTH) width = MIN_NODE_WIDTH;
    if (height < MIN_NODE_HEIGHT) height = MIN_NODE_HEIGHT;

    GooeyNode* node = (GooeyNode*)calloc(1, sizeof(GooeyNode));
    if (!node) {
        LOG_ERROR("Failed to allocate memory for node");
        return;
    }

    // Initialize node
    snprintf(node->node_id, sizeof(node->node_id), "node_%d", node_id_counter++);
    strncpy(node->title, title, sizeof(node->title) - 1);
    node->title[sizeof(node->title) - 1] = '\0'; // Ensure null termination

    node->x = x;
    node->y = y;
    node->width = width;
    node->height = height;
    node->is_selected = false;
    node->is_dragging = false;
    node->sockets = NULL;
    node->socket_count = 0;

    // Add to editor's node array
    GooeyNode** new_nodes = (GooeyNode**)realloc(
        editor->nodes,
        sizeof(GooeyNode*) * (editor->node_count + 1)
    );

    if (!new_nodes) {
        LOG_ERROR("Failed to reallocate nodes array");
        free(node);
        return;
    }

    editor->nodes = new_nodes;
    editor->nodes[editor->node_count] = node;
    editor->node_count++;

    LOG_INFO("Added node '%s' at position (%d, %d)", title, x, y);
}

GooeyNodeConnection* GooeyNodeEditor_ConnectSockets(GooeyNodeEditor* editor, GooeyNodeSocket* from, GooeyNodeSocket* to)
{
    if (!editor || !from || !to) {
        LOG_ERROR("Invalid parameters for ConnectSockets");
        return NULL;
    }

    GooeyNodeConnection* connection = GooeyNodeEditor_Internal_ConnectSockets(editor, from, to);
    if (connection) {
        LOG_INFO("Connected socket '%s' to '%s'", from->name, to->name);
    } else {
        LOG_WARNING("Failed to connect sockets '%s' and '%s'", from->name, to->name);
    }

    return connection;
}

void GooeyNodeEditor_RemoveNode(GooeyNodeEditor* editor, const char* node_id)
{
    if (!editor || !node_id) {
        LOG_ERROR("Invalid parameters for RemoveNode");
        return;
    }

    for (int i = 0; i < editor->node_count; i++) {
        if (strcmp(editor->nodes[i]->node_id, node_id) == 0) {
            GooeyNode* node_to_remove = editor->nodes[i];

            LOG_INFO("Removing node '%s' (%s)", node_to_remove->title, node_id);

            // Remove all connections associated with this node
            for (int j = 0; j < editor->connection_count; j++) {
                GooeyNodeConnection* conn = editor->connections[j];
                if (conn->from_socket->parent_node == node_to_remove ||
                    conn->to_socket->parent_node == node_to_remove) {

                    // Mark sockets as disconnected
                    conn->from_socket->is_connected = false;
                    conn->to_socket->is_connected = false;

                    free(conn);

                    // Shift remaining connections
                    memmove(&editor->connections[j],
                           &editor->connections[j + 1],
                           sizeof(GooeyNodeConnection*) * (editor->connection_count - j - 1));

                    editor->connection_count--;
                    j--; // Recheck current position
                }
            }

            // Free node resources
            free(node_to_remove->sockets);
            free(node_to_remove);

            // Shift remaining nodes
            memmove(&editor->nodes[i],
                   &editor->nodes[i + 1],
                   sizeof(GooeyNode*) * (editor->node_count - i - 1));

            editor->node_count--;

            // Reallocate to free memory
            if (editor->node_count > 0) {
                GooeyNode** shrunk_nodes = (GooeyNode**)realloc(
                    editor->nodes,
                    sizeof(GooeyNode*) * editor->node_count
                );
                if (shrunk_nodes) {
                    editor->nodes = shrunk_nodes;
                }
            } else {
                free(editor->nodes);
                editor->nodes = NULL;
            }

            break;
        }
    }
}

void GooeyNodeEditor_RemoveConnection(GooeyNodeEditor* editor, GooeyNodeConnection* connection)
{
    if (!editor || !connection) {
        LOG_ERROR("Invalid parameters for RemoveConnection");
        return;
    }

    for (int i = 0; i < editor->connection_count; i++) {
        if (editor->connections[i] == connection) {
            LOG_INFO("Removing connection between '%s' and '%s'",
                     connection->from_socket->name, connection->to_socket->name);

            // Mark sockets as disconnected
            connection->from_socket->is_connected = false;
            connection->to_socket->is_connected = false;

            free(connection);

            // Shift remaining connections
            memmove(&editor->connections[i],
                   &editor->connections[i + 1],
                   sizeof(GooeyNodeConnection*) * (editor->connection_count - i - 1));

            editor->connection_count--;

            // Reallocate to free memory
            if (editor->connection_count > 0) {
                GooeyNodeConnection** shrunk_connections = (GooeyNodeConnection**)realloc(
                    editor->connections,
                    sizeof(GooeyNodeConnection*) * editor->connection_count
                );
                if (shrunk_connections) {
                    editor->connections = shrunk_connections;
                }
            } else {
                free(editor->connections);
                editor->connections = NULL;
            }

            break;
        }
    }
}

void GooeyNodeEditor_Clear(GooeyNodeEditor* editor)
{
    if (!editor) {
        LOG_ERROR("Invalid parameter for Clear");
        return;
    }

    LOG_INFO("Clearing node editor with %d nodes and %d connections",
             editor->node_count, editor->connection_count);

    GooeyNodeEditor_Internal_Clear(editor);
}

void GooeyNodeEditor_SetGridSize(GooeyNodeEditor* editor, int grid_size)
{
    if (!editor) return;

    if (grid_size < 5) {
        LOG_WARNING("Grid size too small, setting to minimum 5");
        grid_size = 5;
    }

    editor->grid_size = grid_size;
    LOG_INFO("Set grid size to %d", grid_size);
}

void GooeyNodeEditor_SetShowGrid(GooeyNodeEditor* editor, bool show_grid)
{
    if (!editor) return;

    editor->show_grid = show_grid;
    LOG_INFO("%s grid display", show_grid ? "Enabling" : "Disabling");
}

// Additional utility function
GooeyNode* GooeyNodeEditor_FindNodeById(GooeyNodeEditor* editor, const char* node_id)
{
    if (!editor || !node_id) return NULL;

    for (int i = 0; i < editor->node_count; i++) {
        if (strcmp(editor->nodes[i]->node_id, node_id) == 0) {
            return editor->nodes[i];
        }
    }
    return NULL;
}

#endif
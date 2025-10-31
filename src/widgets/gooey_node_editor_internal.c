/*
 Copyright (c) 2024 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "widgets/gooey_node_editor_internal.h"
#include <string.h>
#if (ENABLE_NODE_EDITOR)
#include "backends/gooey_backend_internal.h"
#include "theme/gooey_theme.h"
#include "logger/pico_logger_internal.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NODE_HEADER_HEIGHT 25
#define SOCKET_RADIUS 5

static GooeyNodeSocket* FindSocketAt(GooeyNodeEditor* editor, int x, int y)
{
    for (int i = 0; i < editor->node_count; i++) {
        GooeyNode* node = editor->nodes[i];
        for (int j = 0; j < node->socket_count; j++) {
            GooeyNodeSocket* socket = &node->sockets[j];
            const int confidence_interval = 80; // To make it easier to click on sockets
            int socket_x = node->x + socket->x;
            int socket_y = node->y + socket->y;

            int dx = x - socket_x;
            int dy = y - socket_y;
            if (dx * dx + dy * dy <= SOCKET_RADIUS * SOCKET_RADIUS + confidence_interval) {
                return socket;
            }
        }
    }
    return NULL;
}

static GooeyNode* FindNodeAt(GooeyNodeEditor* editor, int x, int y)
{

    for (int i = 0; i < editor->node_count; i++) {
        GooeyNode* node = editor->nodes[i];
        if (x >= node->x  && x <= node->x + node->width &&
            y >= node->y && y <= node->y + node->height) {
            return node;
        }
    }
    return NULL;
}

bool GooeyNodeEditor_HandleClick(GooeyWindow* win, int x, int y)
{
    if (!win || !win->node_editors) return false;

    for (size_t i = 0; i < win->node_editor_count; i++) {
        const int confidence_interval = 80; // To make it easier to click on nodes

        GooeyNodeEditor* editor = win->node_editors[i];
        if (!editor || !editor->core.is_visible || editor->core.disable_input) continue;

        // Check if click is within editor bounds
        if (x >= editor->core.x - confidence_interval && x <= editor->core.x +confidence_interval + editor->core.width &&
            y >= editor->core.y - confidence_interval && y <= editor->core.y + confidence_interval + editor->core.height) {

            int editor_x = x - editor->core.x;
            int editor_y = y - editor->core.y;

            // Check for node click
            GooeyNode* node = FindNodeAt(editor, editor_x, editor_y);
            if (node) {
                // Deselect all other nodes
                for (int j = 0; j < editor->node_count; j++) {
                    editor->nodes[j]->is_selected = false;
                }

                node->is_selected = true;
                                node->is_dragging = true;

                node->drag_offset_x = editor_x - node->x;
                node->drag_offset_y = editor_y - node->y;

                // Call callback if set
                if (editor->callback) {
                    editor->callback(editor->user_data);
                }
                return true;
            }

            // Check for socket click
            GooeyNodeSocket* socket = FindSocketAt(editor, editor_x, editor_y);
            if (socket) {
                if (!editor->dragging_socket) {
                    editor->dragging_socket = socket;
                } else {
                    // Create connection if valid
                    if (editor->dragging_socket->type == GOOEY_SOCKET_TYPE_OUTPUT &&
                        socket->type == GOOEY_SOCKET_TYPE_INPUT) {
                        GooeyNodeEditor_Internal_ConnectSockets(editor, editor->dragging_socket, socket);

                        // Call callback if set
                        if (editor->callback) {
                            editor->callback(editor->user_data);
                        }
                    }
                    editor->dragging_socket = NULL;
                }
                return true;
            }

            // Panning
            editor->is_panning = true;
            editor->pan_start_x = editor_x;
            editor->pan_start_y = editor_y;
            return true;
        }
    }
    return false;
}
GooeyNodeConnection* GooeyNodeEditor_Internal_ConnectSockets(GooeyNodeEditor* editor, GooeyNodeSocket* from, GooeyNodeSocket* to)
{
    if (!editor || !from || !to) return NULL;

    // Check if connection is valid
    if (from->type != GOOEY_SOCKET_TYPE_OUTPUT || to->type != GOOEY_SOCKET_TYPE_INPUT) {
        return NULL;
    }

    // Check if sockets are already connected
    for (int i = 0; i < editor->connection_count; i++) {
        if (editor->connections[i]->from_socket == from && editor->connections[i]->to_socket == to) {
            return editor->connections[i]; // Connection already exists
        }
    }

    GooeyNodeConnection* conn = (GooeyNodeConnection*)calloc(1, sizeof(GooeyNodeConnection));
    if (!conn) return NULL;

    conn->from_socket = from;
    conn->to_socket = to;
    conn->is_selected = false;

    // Add to editor
    editor->connections = (GooeyNodeConnection**)realloc(editor->connections, sizeof(GooeyNodeConnection*) * (editor->connection_count + 1));
    editor->connections[editor->connection_count] = conn;
    editor->connection_count++;

    // Mark sockets as connected
    from->is_connected = true;
    to->is_connected = true;

    return conn;
}

bool GooeyNodeEditor_HandleHover(GooeyWindow* win, int x, int y)
{
    if (!win || !win->node_editors) return false;

    for (size_t i = 0; i < win->node_editor_count; i++) {
        GooeyNodeEditor* editor = win->node_editors[i];
        if (!editor || !editor->core.is_visible || editor->core.disable_input) continue;

        if (x >= editor->core.x && x <= editor->core.x + editor->core.width &&
            y >= editor->core.y && y <= editor->core.y + editor->core.height) {

            // Check if hovering over socket or node
            int editor_x = x - editor->core.x;
            int editor_y = y - editor->core.y;

            if (FindSocketAt(editor, editor_x, editor_y) || FindNodeAt(editor, editor_x, editor_y)) {
                active_backend->SetCursor(GOOEY_CURSOR_CROSSHAIR);
            } else {
                active_backend->SetCursor(GOOEY_CURSOR_ARROW);
            }
            return true;
        }
    }
    return false;
}
bool GooeyNodeEditor_HandleRelease(GooeyWindow* win, int x, int y) {
    if (!win || !win->node_editors) return false;

    for (size_t i = 0; i < win->node_editor_count; i++) {
        GooeyNodeEditor* editor = win->node_editors[i];
        if (!editor || !editor->core.is_visible || editor->core.disable_input) continue;

        if (x >= editor->core.x && x <= editor->core.x + editor->core.width &&
            y >= editor->core.y && y <= editor->core.y + editor->core.height) {
            int editor_x = x - editor->core.x;
            int editor_y = y - editor->core.y;

            // Stop dragging nodes
            for (int j = 0; j < editor->node_count; j++) {
                GooeyNode* node = editor->nodes[j];
                if (node->is_dragging) {
                    node->is_dragging = false;


                    return true;

                }
            }

            // Stop panning
            if (editor->is_panning) {
                editor->is_panning = false;
                return true;
            }
        }
    }
    return false;
}


bool GooeyNodeEditor_HandleDrag(GooeyWindow* win, int x, int y, int dx, int dy)
{
    if (!win || !win->node_editors) return false;

    for (size_t i = 0; i < win->node_editor_count; i++) {
        GooeyNodeEditor* editor = win->node_editors[i];
        if (!editor || !editor->core.is_visible || editor->core.disable_input) continue;

        if (x >= editor->core.x && x <= editor->core.x + editor->core.width &&
            y >= editor->core.y && y <= editor->core.y + editor->core.height) {

            int editor_x = x - editor->core.x;
            int editor_y = y - editor->core.y;

            // Handle node dragging
            for (int j = 0; j < editor->node_count; j++) {
                GooeyNode* node = editor->nodes[j];
                if (node->is_dragging) {
                    node->x = editor_x - node->drag_offset_x;
                    node->y = editor_y - node->drag_offset_y;

                    return true;
                }
            }

            // Handle panning
            if (editor->is_panning) {
                editor->pan_x += dx;
                editor->pan_y += dy;
                return true;
            }
        }
    }
    return false;
}

void GooeyNodeEditor_Internal_Clear(GooeyNodeEditor* editor) {
    if (!editor) return;

    for (int i = 0; i < editor->node_count; i++) {
        free(editor->nodes[i]->sockets);
        free(editor->nodes[i]);
    }
    free(editor->nodes);

    for (int i = 0; i < editor->connection_count; i++) {
        free(editor->connections[i]);
    }
    free(editor->connections);

    editor->nodes = NULL;
    editor->connections = NULL;
    editor->node_count = 0;
    editor->connection_count = 0;
}

const char * GetTypeString(GooeyDataType type) {
    switch (type) {
        case GOOEY_DATA_TYPE_BOOL:
            return "<Bool>";
        case GOOEY_DATA_TYPE_INT:
            return "<Int>";
        case GOOEY_DATA_TYPE_FLOAT:
            return "<Float>";
        case GOOEY_DATA_TYPE_STRING:
            return "<String>";
        default:
            break;
    }
    return "Undefined";
}

void GooeyNodeEditor_Draw(GooeyWindow* win)
{
    if (!win || !win->node_editors) return;

    for (size_t i = 0; i < win->node_editor_count; i++) {
        GooeyNodeEditor* editor = win->node_editors[i];
        if (!editor || !editor->core.is_visible) continue;

        // Draw background
        active_backend->FillRectangle(
            editor->core.x, editor->core.y,
            editor->core.width, editor->core.height,
            win->active_theme->base, win->creation_id, false, 0.0f, NULL
        );

        // Draw grid
        if (editor->show_grid) {
            for (int x = editor->core.x; x < editor->core.x + editor->core.width; x += editor->grid_size) {
                active_backend->DrawLine(
                    x, editor->core.y,
                    x, editor->core.y + editor->core.height,
                    win->active_theme->neutral,  win->creation_id, NULL
                );
            }
            for (int y = editor->core.y; y < editor->core.y + editor->core.height; y += editor->grid_size) {
                active_backend->DrawLine(
                    editor->core.x, y,
                    editor->core.x + editor->core.width, y,
                    win->active_theme->neutral,  win->creation_id, NULL
                );
            }
        }

        // Draw connections
        for (int j = 0; j < editor->connection_count; j++) {
            GooeyNodeConnection* conn = editor->connections[j];
            if (conn->from_socket && conn->to_socket) {
                GooeyNode* from_node = (GooeyNode*)conn->from_socket->parent_node;
                GooeyNode* to_node = (GooeyNode*)conn->to_socket->parent_node;

                int x1 = editor->core.x + from_node->x + conn->from_socket->x;
                int y1 = editor->core.y + from_node->y + conn->from_socket->y;
                int x2 = editor->core.x + to_node->x + conn->to_socket->x;
                int y2 = editor->core.y + to_node->y + conn->to_socket->y;

                active_backend->DrawLine(x1, y1, x2, y2, win->active_theme->success,  win->creation_id, NULL);
            }
        }

        // Draw nodes
        for (int j = 0; j < editor->node_count; j++) {
            GooeyNode* node = editor->nodes[j];
            unsigned long node_color = win->active_theme->widget_base;

            // Draw node body
            active_backend->FillRectangle(
                editor->core.x + node->x,
                editor->core.y + node->y,
                node->width,
                node->height,
                node_color,
                win->creation_id,
                true,
                8.0f,
                NULL
            );

            // Draw node header
            active_backend->FillRectangle(
                editor->core.x + node->x,
                editor->core.y + node->y,
                node->width,
                NODE_HEADER_HEIGHT,
                win->active_theme->primary, win->creation_id, true, 8.0f,
                NULL

            );

            // Draw node title
            active_backend->DrawText(
                editor->core.x + node->x + 10,
                editor->core.y + node->y + NODE_HEADER_HEIGHT / 2 +5,
                node->title,
                win->active_theme->neutral,
                0.26f,
                win->creation_id,
                NULL
            );

            // Draw sockets
            for (int k = 0; k < node->socket_count; k++) {
                GooeyNodeSocket* socket = &node->sockets[k];
                unsigned long socket_color = socket->type == GOOEY_SOCKET_TYPE_INPUT ?
                                           win->active_theme->primary : win->active_theme->primary;


                // Draw socket as filled arc (circle)
                active_backend->FillArc(
                    editor->core.x + node->x + socket->x,
                    editor->core.y + node->y + socket->y,
                    SOCKET_RADIUS * 2,
                    SOCKET_RADIUS * 2,
                    0, 360, win->creation_id, NULL
                );




                active_backend->DrawText(editor->core.x + node->x + socket->x + 10,
                                        editor->core.y + node->y + socket->y,
                                        socket->name,
                                         win->active_theme->neutral,
                                         0.27f,
                                         win->creation_id,
                                         NULL);

                active_backend->DrawText(
                editor->core.x + node->x + socket->x + active_backend->GetTextWidth(socket->name, strlen(socket->name)) + 15,
                                                        editor->core.y + node->y + socket->y,
                                                         GetTypeString(socket->data_type),
                                                         win->active_theme->neutral,
                                                         0.27f,
                                                         win->creation_id,
                                                         NULL

                );

            }
        }
    }
}

#endif
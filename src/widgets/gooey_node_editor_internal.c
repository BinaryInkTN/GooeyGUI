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
#define SOCKET_CLICK_CONFIDENCE 10
#define NODE_CLICK_CONFIDENCE 10
#define CONNECTION_CURVE_STRENGTH 50
#define BEZIER_SEGMENTS 20
#define CONNECTION_HIT_TOLERANCE 8

static int global_mouse_x = 0;
static int global_mouse_y = 0;

static float bezier_interp(float t, float p0, float p1, float p2, float p3) {
    float u = 1.0f - t;
    float uu = u * u;
    float uuu = uu * u;
    float tt = t * t;
    float ttt = tt * t;
    return uuu * p0 + 3 * uu * t * p1 + 3 * u * tt * p2 + ttt * p3;
}

static bool point_near_curve(int px, int py, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
    for (int i = 0; i <= BEZIER_SEGMENTS; i++) {
        float t = (float)i / BEZIER_SEGMENTS;
        float bx = bezier_interp(t, x0, x1, x2, x3);
        float by = bezier_interp(t, y0, y1, y2, y3);
        float dx = px - bx;
        float dy = py - by;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist <= CONNECTION_HIT_TOLERANCE) {
            return true;
        }
    }
    return false;
}

static GooeyNodeSocket* FindSocketAt(const GooeyNodeEditor* editor, int x, int y) {
    if (!editor) return NULL;
    const int confidence_sq = SOCKET_CLICK_CONFIDENCE * SOCKET_CLICK_CONFIDENCE;
    for (int i = 0; i < editor->node_count; i++) {
        const GooeyNode* node = editor->nodes[i];
        if (!node) continue;
        for (int j = 0; j < node->socket_count; j++) {
            const GooeyNodeSocket* socket = &node->sockets[j];
            const int socket_x = node->x + socket->x;
            const int socket_y = node->y + socket->y;
            const int dx = x - socket_x;
            const int dy = y - socket_y;
            const int distance_sq = dx * dx + dy * dy;
            if (distance_sq <= confidence_sq) {
                return (GooeyNodeSocket*)socket;
            }
        }
    }
    return NULL;
}

static GooeyNode* FindNodeAt(const GooeyNodeEditor* editor, int x, int y) {
    if (!editor) return NULL;
    for (int i = 0; i < editor->node_count; i++) {
        GooeyNode* node = editor->nodes[i];
        if (!node) continue;
        if (x >= node->x - NODE_CLICK_CONFIDENCE &&
            x <= node->x + node->width + NODE_CLICK_CONFIDENCE &&
            y >= node->y - NODE_CLICK_CONFIDENCE &&
            y <= node->y + node->height + NODE_CLICK_CONFIDENCE) {
            return node;
        }
    }
    return NULL;
}

static GooeyNodeConnection* FindConnectionAt(const GooeyNodeEditor* editor, int x, int y) {
    if (!editor) return NULL;
    for (int i = 0; i < editor->connection_count; i++) {
        GooeyNodeConnection* conn = editor->connections[i];
        if (!conn->from_socket || !conn->to_socket) continue;
        GooeyNode* from_node = (GooeyNode*)conn->from_socket->parent_node;
        GooeyNode* to_node = (GooeyNode*)conn->to_socket->parent_node;
        int x1 = editor->core.x + from_node->x + conn->from_socket->x;
        int y1 = editor->core.y + from_node->y + conn->from_socket->y;
        int x2 = editor->core.x + to_node->x + conn->to_socket->x;
        int y2 = editor->core.y + to_node->y + conn->to_socket->y;
        int dx = abs(x2 - x1);
        int curve_strength = CONNECTION_CURVE_STRENGTH + (dx / 10);
        int cp1_x = x1 + curve_strength;
        int cp1_y = y1;
        int cp2_x = x2 - curve_strength;
        int cp2_y = y2;
        if (point_near_curve(x, y, x1, y1, cp1_x, cp1_y, cp2_x, cp2_y, x2, y2)) {
            return conn;
        }
    }
    return NULL;
}

static bool IsPointInEditor(const GooeyNodeEditor* editor, int x, int y) {
    return editor &&
           x >= editor->core.x &&
           x <= editor->core.x + editor->core.width &&
           y >= editor->core.y &&
           y <= editor->core.y + editor->core.height;
}

static void DeselectAllNodes(GooeyNodeEditor* editor) {
    if (!editor) return;
    for (int i = 0; i < editor->node_count; i++) {
        if (editor->nodes[i]) {
            editor->nodes[i]->is_selected = false;
        }
    }
}

static void DeselectAllConnections(GooeyNodeEditor* editor) {
    if (!editor) return;
    for (int i = 0; i < editor->connection_count; i++) {
        editor->connections[i]->is_selected = false;
    }
}

static void DrawBezierCurve(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, unsigned long color, int creation_id) {
    int prev_x = x0;
    int prev_y = y0;
    for (int i = 1; i <= BEZIER_SEGMENTS; i++) {
        float t = (float)i / BEZIER_SEGMENTS;
        int x = (int)bezier_interp(t, x0, x1, x2, x3);
        int y = (int)bezier_interp(t, y0, y1, y2, y3);
        active_backend->DrawLine(prev_x, prev_y, x, y, color, creation_id, NULL);
        prev_x = x;
        prev_y = y;
    }
}

static void DrawConnection(const GooeyNodeConnection* conn, const GooeyNodeEditor* editor, const GooeyWindow* win) {
    if (!conn->from_socket || !conn->to_socket) return;
    GooeyNode* from_node = (GooeyNode*)conn->from_socket->parent_node;
    GooeyNode* to_node = (GooeyNode*)conn->to_socket->parent_node;
    int x1 = editor->core.x + from_node->x + conn->from_socket->x;
    int y1 = editor->core.y + from_node->y + conn->from_socket->y;
    int x2 = editor->core.x + to_node->x + conn->to_socket->x;
    int y2 = editor->core.y + to_node->y + conn->to_socket->y;
    unsigned long color = conn->is_selected ? win->active_theme->danger : win->active_theme->success;
    int dx = abs(x2 - x1);
    int curve_strength = CONNECTION_CURVE_STRENGTH + (dx / 10);
    int cp1_x = x1 + curve_strength;
    int cp1_y = y1;
    int cp2_x = x2 - curve_strength;
    int cp2_y = y2;
    DrawBezierCurve(x1, y1, cp1_x, cp1_y, cp2_x, cp2_y, x2, y2, color, win->creation_id);
}

static void DrawDraggingConnection(const GooeyNodeEditor* editor, const GooeyWindow* win) {
    if (!editor->dragging_socket) return;
    GooeyNode* from_node = (GooeyNode*)editor->dragging_socket->parent_node;
    int start_x = editor->core.x + from_node->x + editor->dragging_socket->x;
    int start_y = editor->core.y + from_node->y + editor->dragging_socket->y;
    int end_x = global_mouse_x;
    int end_y = global_mouse_y;
    GooeyNodeSocket* target_socket = NULL;
    int editor_mouse_x = global_mouse_x - editor->core.x;
    int editor_mouse_y = global_mouse_y - editor->core.y;
    for (int i = 0; i < editor->node_count; i++) {
        const GooeyNode* node = editor->nodes[i];
        if (!node) continue;
        for (int j = 0; j < node->socket_count; j++) {
            const GooeyNodeSocket* socket = &node->sockets[j];
            if (socket->type != GOOEY_SOCKET_TYPE_INPUT) continue;
            const int socket_x = node->x + socket->x;
            const int socket_y = node->y + socket->y;
            const int dx = editor_mouse_x - socket_x;
            const int dy = editor_mouse_y - socket_y;
            const int distance_sq = dx * dx + dy * dy;
            if (distance_sq <= SOCKET_CLICK_CONFIDENCE * SOCKET_CLICK_CONFIDENCE) {
                target_socket = (GooeyNodeSocket*)socket;
                break;
            }
        }
        if (target_socket) break;
    }
    unsigned long color;
    if (target_socket && editor->dragging_socket->type == GOOEY_SOCKET_TYPE_OUTPUT) {
        color = win->active_theme->success;
        end_x = editor->core.x + ((GooeyNode*)target_socket->parent_node)->x + target_socket->x;
        end_y = editor->core.y + ((GooeyNode*)target_socket->parent_node)->y + target_socket->y;
    } else {
        color = win->active_theme->danger;
    }
    int dx = abs(end_x - start_x);
    int curve_strength = CONNECTION_CURVE_STRENGTH + (dx / 10);
    int cp1_x = start_x + curve_strength;
    int cp1_y = start_y;
    int cp2_x = end_x - curve_strength;
    int cp2_y = end_y;
    DrawBezierCurve(start_x, start_y, cp1_x, cp1_y, cp2_x, cp2_y, end_x, end_y, color, win->creation_id);
}

bool GooeyNodeEditor_HandleClick(GooeyWindow* win, int x, int y) {
    if (!win || !win->node_editors) return false;
    global_mouse_x = x;
    global_mouse_y = y;
    for (size_t i = 0; i < win->node_editor_count; i++) {
        GooeyNodeEditor* editor = win->node_editors[i];
        if (!editor || !editor->core.is_visible || editor->core.disable_input) continue;
        if (!IsPointInEditor(editor, x, y)) continue;
        int editor_x = x - editor->core.x;
        int editor_y = y - editor->core.y;
        GooeyNodeSocket* socket = FindSocketAt(editor, editor_x, editor_y);
        GooeyNode* node = FindNodeAt(editor, editor_x, editor_y);
        GooeyNodeConnection* connection = FindConnectionAt(editor, x, y);
        if (socket && socket->type == GOOEY_SOCKET_TYPE_OUTPUT) {
            editor->dragging_socket = socket;
            return true;
        }
        if (node) {
            bool clicked_on_socket = false;
            for (int j = 0; j < node->socket_count && !clicked_on_socket; j++) {
                const GooeyNodeSocket* node_socket = &node->sockets[j];
                const int socket_x = node->x + node_socket->x;
                const int socket_y = node->y + node_socket->y;
                const int dx = editor_x - socket_x;
                const int dy = editor_y - socket_y;
                const int distance_sq = dx * dx + dy * dy;
                if (distance_sq <= SOCKET_CLICK_CONFIDENCE * SOCKET_CLICK_CONFIDENCE) {
                    clicked_on_socket = true;
                }
            }
            if (!clicked_on_socket) {
                DeselectAllConnections(editor);
                DeselectAllNodes(editor);
                node->is_selected = true;
                node->is_dragging = true;
                node->drag_offset_x = editor_x - node->x;
                node->drag_offset_y = editor_y - node->y;
                if (editor->callback) {
                    editor->callback(editor->user_data);
                }
                return true;
            }
        }
        if (connection) {
            DeselectAllConnections(editor);
            DeselectAllNodes(editor);
            connection->is_selected = true;
            if (editor->callback) {
                editor->callback(editor->user_data);
            }
            return true;
        }
        DeselectAllConnections(editor);
        DeselectAllNodes(editor);
        editor->is_panning = true;
        editor->pan_start_x = editor_x;
        editor->pan_start_y = editor_y;
        return true;
    }
    return false;
}

GooeyNodeConnection* GooeyNodeEditor_Internal_ConnectSockets(GooeyNodeEditor* editor, GooeyNodeSocket* from, GooeyNodeSocket* to) {
    if (!editor || !from || !to) return NULL;
    if (from->type != GOOEY_SOCKET_TYPE_OUTPUT || to->type != GOOEY_SOCKET_TYPE_INPUT) {
        return NULL;
    }
    if (from->parent_node == to->parent_node) {
        return NULL;
    }
    for (int i = 0; i < editor->connection_count; i++) {
        if (editor->connections[i]->from_socket == from && editor->connections[i]->to_socket == to) {
            return editor->connections[i];
        }
        if (editor->connections[i]->to_socket == to) {
            GooeyNodeConnection* old_conn = editor->connections[i];
            old_conn->from_socket->is_connected = false;
            old_conn->from_socket = from;
            from->is_connected = true;
            return old_conn;
        }
    }
    GooeyNodeConnection* conn = (GooeyNodeConnection*)calloc(1, sizeof(GooeyNodeConnection));
    if (!conn) return NULL;
    conn->from_socket = from;
    conn->to_socket = to;
    conn->is_selected = false;
    GooeyNodeConnection** new_connections = (GooeyNodeConnection**)realloc(
        editor->connections,
        sizeof(GooeyNodeConnection*) * (editor->connection_count + 1)
    );
    if (!new_connections) {
        free(conn);
        return NULL;
    }
    editor->connections = new_connections;
    editor->connections[editor->connection_count] = conn;
    editor->connection_count++;
    from->is_connected = true;
    to->is_connected = true;
    return conn;
}

bool GooeyNodeEditor_HandleHover(GooeyWindow* win, int x, int y) {
    if (!win || !win->node_editors) return false;
    global_mouse_x = x;
    global_mouse_y = y;
    for (size_t i = 0; i < win->node_editor_count; i++) {
        GooeyNodeEditor* editor = win->node_editors[i];
        if (!editor || !editor->core.is_visible || editor->core.disable_input) continue;
        if (!IsPointInEditor(editor, x, y)) continue;
        int editor_x = x - editor->core.x;
        int editor_y = y - editor->core.y;
        if (FindSocketAt(editor, editor_x, editor_y) || editor->dragging_socket) {
            active_backend->SetCursor(GOOEY_CURSOR_CROSSHAIR);
        } else if (FindNodeAt(editor, editor_x, editor_y) || FindConnectionAt(editor, x, y)) {
            active_backend->SetCursor(GOOEY_CURSOR_HAND);
        } else {
            active_backend->SetCursor(GOOEY_CURSOR_ARROW);
        }
        return true;
    }
    return false;
}

bool GooeyNodeEditor_HandleRelease(GooeyWindow* win, int x, int y) {
    if (!win || !win->node_editors) return false;
    global_mouse_x = x;
    global_mouse_y = y;
    for (size_t i = 0; i < win->node_editor_count; i++) {
        GooeyNodeEditor* editor = win->node_editors[i];
        if (!editor || !editor->core.is_visible || editor->core.disable_input) continue;
        if (!IsPointInEditor(editor, x, y)) continue;
        int editor_x = x - editor->core.x;
        int editor_y = y - editor->core.y;
        bool handled = false;
        if (editor->dragging_socket) {
            GooeyNodeSocket* target_socket = FindSocketAt(editor, editor_x, editor_y);
            if (target_socket &&
                editor->dragging_socket->type == GOOEY_SOCKET_TYPE_OUTPUT &&
                target_socket->type == GOOEY_SOCKET_TYPE_INPUT &&
                editor->dragging_socket != target_socket &&
                editor->dragging_socket->parent_node != target_socket->parent_node) {
                GooeyNodeEditor_Internal_ConnectSockets(editor, editor->dragging_socket, target_socket);
                if (editor->callback) {
                    editor->callback(editor->user_data);
                }
            }
            editor->dragging_socket = NULL;
            handled = true;
        }
        for (int j = 0; j < editor->node_count; j++) {
            GooeyNode* node = editor->nodes[j];
            if (node && node->is_dragging) {
                node->is_dragging = false;
                handled = true;
            }
        }
        if (editor->is_panning) {
            editor->is_panning = false;
            handled = true;
        }
        if (handled) return true;
    }
    return false;
}

bool GooeyNodeEditor_HandleDrag(GooeyWindow* win, int x, int y, int dx, int dy) {
    if (!win || !win->node_editors) return false;
    global_mouse_x = x;
    global_mouse_y = y;
    for (size_t i = 0; i < win->node_editor_count; i++) {
        GooeyNodeEditor* editor = win->node_editors[i];
        if (!editor || !editor->core.is_visible || editor->core.disable_input) continue;
        if (!IsPointInEditor(editor, x, y)) continue;
        int editor_x = x - editor->core.x;
        int editor_y = y - editor->core.y;
        for (int j = 0; j < editor->node_count; j++) {
            GooeyNode* node = editor->nodes[j];
            if (node && node->is_dragging) {
                node->x = editor_x - node->drag_offset_x;
                node->y = editor_y - node->drag_offset_y;
                return true;
            }
        }
        if (editor->is_panning) {
            editor->pan_x += dx;
            editor->pan_y += dy;
            return true;
        }
    }
    return false;
}

void GooeyNodeEditor_Internal_Clear(GooeyNodeEditor* editor) {
    if (!editor) return;
    for (int i = 0; i < editor->node_count; i++) {
        if (editor->nodes[i]) {
            free(editor->nodes[i]->sockets);
            free(editor->nodes[i]);
        }
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
    editor->dragging_socket = NULL;
}

static const char* GetTypeString(GooeyDataType type) {
    switch (type) {
        case GOOEY_DATA_TYPE_BOOL:   return "<Bool>";
        case GOOEY_DATA_TYPE_INT:    return "<Int>";
        case GOOEY_DATA_TYPE_FLOAT:  return "<Float>";
        case GOOEY_DATA_TYPE_STRING: return "<String>";
        default:                     return "<Undefined>";
    }
}

static void DrawGrid(const GooeyNodeEditor* editor, const GooeyWindow* win) {
    if (!editor->show_grid) return;
    unsigned long grid_color = win->active_theme->neutral;
    int grid_size = editor->grid_size;
    for (int x = editor->core.x; x < editor->core.x + editor->core.width; x += grid_size) {
        active_backend->DrawLine(
            x, editor->core.y,
            x, editor->core.y + editor->core.height,
            grid_color, win->creation_id, NULL
        );
    }
    for (int y = editor->core.y; y < editor->core.y + editor->core.height; y += grid_size) {
        active_backend->DrawLine(
            editor->core.x, y,
            editor->core.x + editor->core.width, y,
            grid_color, win->creation_id, NULL
        );
    }
}

static void DrawConnections(const GooeyNodeEditor* editor, const GooeyWindow* win) {
    for (int j = 0; j < editor->connection_count; j++) {
        DrawConnection(editor->connections[j], editor, win);
    }
}

static void DrawNode(const GooeyNode* node, const GooeyNodeEditor* editor, const GooeyWindow* win) {
    unsigned long node_color = node->is_selected ? win->active_theme->danger : win->active_theme->widget_base;
    unsigned long header_color = win->active_theme->primary;
    unsigned long text_color = win->active_theme->neutral;
    int abs_x = editor->core.x + node->x;
    int abs_y = editor->core.y + node->y;
    active_backend->FillRectangle(
        abs_x, abs_y,
        node->width, node->height,
        node_color, win->creation_id, true, 8.0f, NULL
    );
    active_backend->FillRectangle(
        abs_x, abs_y,
        node->width, NODE_HEADER_HEIGHT,
        header_color, win->creation_id, true, 8.0f, NULL
    );
    active_backend->DrawText(
        abs_x + 10,
        abs_y + NODE_HEADER_HEIGHT / 2 + 5,
        node->title, text_color, 18.0f, win->creation_id, NULL
    );
    for (int k = 0; k < node->socket_count; k++) {
        const GooeyNodeSocket* socket = &node->sockets[k];
        unsigned long socket_color = socket->is_connected ? win->active_theme->success : win->active_theme->primary;
        int socket_x = abs_x + socket->x;
        int socket_y = abs_y + socket->y;
        active_backend->SetForeground(socket_color);
        active_backend->FillArc(
            socket_x, socket_y,
            SOCKET_RADIUS * 2, SOCKET_RADIUS * 2,
            0, 360, win->creation_id, NULL
        );
        if (socket->type == GOOEY_SOCKET_TYPE_INPUT) {
            active_backend->DrawText(
                socket_x + 10, socket_y + 10,
                socket->name, text_color, 18.0f, win->creation_id, NULL
            );
            const char* type_str = GetTypeString(socket->data_type);
            int name_width = active_backend->GetTextWidth(socket->name, strlen(socket->name));
            active_backend->DrawText(
                socket_x + name_width + 15, socket_y + 10,
                type_str, text_color, 18.0f, win->creation_id, NULL
            );
        } else {
            const char* type_str = GetTypeString(socket->data_type);
            int type_width = active_backend->GetTextWidth(type_str, strlen(type_str));
            active_backend->DrawText(
                socket_x - type_width - 15, socket_y + 10,
                type_str, text_color, 18.0f, win->creation_id, NULL
            );
            int name_width = active_backend->GetTextWidth(socket->name, strlen(socket->name));
            active_backend->DrawText(
                socket_x - name_width - type_width - 25, socket_y + 10,
                socket->name, text_color, 18.0f, win->creation_id, NULL
            );
        }
    }
}

void GooeyNodeEditor_Draw(GooeyWindow* win) {
    if (!win || !win->node_editors) return;
    for (size_t i = 0; i < win->node_editor_count; i++) {
        GooeyNodeEditor* editor = win->node_editors[i];
        if (!editor || !editor->core.is_visible) continue;
        active_backend->FillRectangle(
            editor->core.x, editor->core.y,
            editor->core.width, editor->core.height,
            win->active_theme->base, win->creation_id, false, 0.0f, NULL
        );
        DrawGrid(editor, win);
        DrawConnections(editor, win);
        if (editor->dragging_socket) {
            DrawDraggingConnection(editor, win);
        }
        for (int j = 0; j < editor->node_count; j++) {
            if (editor->nodes[j]) {
                DrawNode(editor->nodes[j], editor, win);
            }
        }
    }
}

#endif
#ifndef GOOEY_BACKEND_INTERNAL_H
#define GOOEY_BACKEND_INTERNAL_H
#include "core/gooey_window.h"

typedef enum GooeyBackends
{
    X11,  /**< Linux X11 native api */
    GLFW, /**< GLFW OpenGL Backend*/
    GLPS
} GooeyBackends;

/**
 * @brief The GooeyBackend structure contains function pointers
 *        for backend-specific operations.
 */
typedef struct GooeyBackend
{
    int (*Init)();
    void (*Run)();
    void (*Cleanup)();
    void (*SetupCallbacks)(void (*callback)(size_t window_id, void*data), void* data);
    void (*RequestRedraw)(GooeyWindow* win);
    size_t (*GetActiveWindowCount)(void);
    size_t (*GetTotalWindowCount)(void);
    GooeyWindow (*CreateWindow)(const char *title, int width, int height);
    GooeyWindow (*SpawnWindow)(const char *title, int width, int height, bool visibility);
    void (*MakeWindowVisible)(int window_id, bool visibility);
    void (*MakeWindowResizable)(bool value, int window_id);
    int (*GetCurrentClickedWindow)(void);
    void (*DestroyWindows)(void);
    void (*DestroyWindowFromId)(int window_id);
    void (*HideCurrentChild)(void);
    void (*SetContext)(GooeyWindow *win);
    void (*UpdateBackground)(void);
    void (*Clear)(int window_id);
    void (*Render)(int window_id);
    void (*SetForeground)(unsigned long color);
    void (*DrawText)(int x, int y, const char *text, unsigned long color, float font_size, int window_id);
    void (*FillRectangle)(int x, int y, int width, int height, unsigned long color, int window_id);
    void (*DrawRectangle)(int x, int y, int width, int height, unsigned long color, int window_id);
    void (*FillArc)(int x, int y, int width, int height, int angle1, int angle2, int window_id);
    const char *(*GetKeyFromCode)(GooeyEvent *gooey_event);
    GooeyEvent *(*HandleEvents)(void);
    void (*ResetEvents)(GooeyWindow* win);
    void (*GetWinDim)(int *width, int *height, int window_id);
    void (*DrawLine)(int x1, int y1, int x2, int y2, unsigned long color, int window_id);
    float (*GetTextWidth)(const char *text, int length);
    float (*GetTextHeight)(const char *text, int length);
    void (*SetCursor)(GOOEY_CURSOR cursor);
} GooeyBackend;

/**
 * @brief The currently active Gooey backend.
 */
extern GooeyBackend *active_backend;

/**
 * @brief The X11 backend for Gooey.
 */
extern GooeyBackend x11_backend;

/**
 * @brief The GLFW backend for Gooey.
 */

extern GooeyBackend glps_backend;

extern GooeyBackends ACTIVE_BACKEND;

#endif
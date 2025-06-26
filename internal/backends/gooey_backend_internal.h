/**
 * @file gooey_backend_internal.h
 * @brief Internal definitions for the Gooey GUI backend.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * This file contains the internal structures, enums, and function pointers
 * used by the Gooey GUI backend. It defines the backend interface and provides
 * the necessary abstractions for rendering, event handling, and window management.
 */

#ifndef GOOEY_BACKEND_INTERNAL_H
#define GOOEY_BACKEND_INTERNAL_H

#include "common/gooey_common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration for available backends in the Gooey framework.
 */
typedef enum GooeyBackends
{
    GLPS, /**< GLPS backend (e.g., OpenGL-based backend) */
    TFT   /**< TFT backend for embedded displays */
} GooeyBackends;


/**
 * @brief The GooeyBackend structure contains function pointers for backend-specific operations.
 *
 * This structure defines the interface for backend implementations, allowing the Gooey framework
 * to interact with different rendering and window management systems.
 */
typedef struct GooeyBackend
{
    int (*Init)();                                                                         /**< Initializes the backend. */
    void (*Run)();                                                                         /**< Starts the main loop of the backend. */
    void (*Cleanup)();                                                                     /**< Cleans up resources used by the backend. */
    void (*SetupCallbacks)(void (*callback)(size_t window_id, void *data), void *data);    /**< Sets up event callbacks. */
    void (*RequestRedraw)(GooeyWindow *win);                                               /**< Requests a redraw of the specified window. */
    void (*SetViewport)(size_t window_id, int width, int height);                          /**< Sets the viewport for the specified window. */
    size_t (*GetActiveWindowCount)(void);                                                  /**< Returns the number of active windows. */
    size_t (*GetTotalWindowCount)(void);                                                   /**< Returns the total number of windows. */
    GooeyWindow *(*CreateWindow)(const char *title, int width, int height);                /**< Creates a new window. */
    GooeyWindow (*SpawnWindow)(const char *title, int width, int height, bool visibility); /**< Spawns a new window with visibility control. */
    void (*MakeWindowVisible)(int window_id, bool visibility);                             /**< Sets the visibility of a window. */
    void (*MakeWindowResizable)(bool value, int window_id);                                /**< Makes a window resizable or non-resizable. */
    void (*WindowToggleDecorations)(GooeyWindow *win, bool enable);                        /**< Toggles window decorations. */
    int (*GetCurrentClickedWindow)(void);                                                  /**< Returns the ID of the currently clicked window. */
    void (*DestroyWindows)(void);                                                          /**< Destroys all windows. */
    void (*DestroyWindowFromId)(int window_id);                                            /**< Destroys a specific window by ID. */
    void (*HideCurrentChild)(void);                                                        /**< Hides the current child window. */
    void (*SetContext)(GooeyWindow *win);                                                  /**< Sets the rendering context for a window. */
    void (*UpdateBackground)(GooeyWindow *win);                                            /**< Updates the background of a window. */
    void (*Clear)(GooeyWindow *win);                                                       /**< Clears the contents of a window. */
    void (*Render)(GooeyWindow *win);                                                      /**< Renders the contents of a window. */
    void (*SetForeground)(uint32_t color);                                                 /**< Sets the foreground color for rendering. */
    void (*DrawText)(int x, int y, const char *text, uint32_t color, float font_size, int window_id); /**< Draws text on a window. */
    unsigned int (*LoadImage)(const char *image_path);                                      /**< Loads an image from a file. */
    unsigned int (*LoadImageFromBin)(unsigned char *data, size_t binary_len);               /**< Loads an image from binary data. */
    void (*DrawImage)(unsigned int texture_id, int x, int y, int width, int height, int window_id); /**< Draws an image on a window. */
    void (*FillRectangle)(int x, int y, int width, int height, uint32_t color, int window_id, bool isRounded, float cornerRadius); /**< Fills a rectangle on a window. */
    void (*DrawRectangle)(int x, int y, int width, int height, uint32_t color, float thickness, int window_id, bool isRounded, float cornerRadius); /**< Draws the outline of a rectangle on a window. */
    void (*FillArc)(int x, int y, int width, int height, int angle1, int angle2, int window_id); /**< Fills an arc on a window. */
    const char *(*GetKeyFromCode)(void *gooey_event);                                            /**< Converts a key code to a string representation. */
    void *(*HandleEvents)(void);                                                                 /**< Handles input events. */
    void (*ResetEvents)(GooeyWindow *win);                                                       /**< Resets the event state for a window. */
    void (*GetWinDim)(int *width, int *height, int window_id);                                   /**< Retrieves the dimensions of a window. */
    double (*GetWinFramerate)(int window_id);                                                    /**< Gets FPS for window. */
    void (*DrawLine)(int x1, int y1, int x2, int y2, uint32_t color, int window_id);            /**< Draws a line on a window. */
    float (*GetTextWidth)(const char *text, int length);                                         /**< Calculates the width of a text string. */
    float (*GetTextHeight)(const char *text, int length);                                        /**< Calculates the height of a text string. */
    void (*SetCursor)(GOOEY_CURSOR cursor);                                                      /**< Sets the cursor type for the window. */
    void (*UnloadImage)(unsigned int texture_id);                                                /**< Unloads an image texture. */
    GooeyTimer *(*CreateTimer)(void);                                                           /**< Creates a new timer. */
    void (*SetTimerCallback)(uint64_t time, GooeyTimer *timer, void (*callback)(void *user_data), void *user_data); /**< Sets a timer callback. */
    void (*StopTimer)(GooeyTimer* timer);                                                       /**< Stops a timer. */
    void (*DestroyTimer)(GooeyTimer *timer);                                                    /**< Destroys a timer. */
    void (*CursorChange)(GOOEY_CURSOR cursor);                                                  /**< Changes the cursor. */
    void (*StopCursorReset)(bool state);                                                        /**< Stops cursor reset. */

} GooeyBackend;

/**
 * @brief The currently active Gooey backend.
 *
 * This pointer points to the backend implementation currently in use by the Gooey framework.
 */
extern GooeyBackend *active_backend;

/**
 * @brief The GLPS backend for Gooey.
 */
extern GooeyBackend glps_backend;

/**
 * @brief The TFT backend for Gooey.
 */
extern GooeyBackend tft_backend __attribute__((visibility("default")));

/**
 * @brief The currently active backend type.
 *
 * This variable indicates which backend is currently active (e.g., GLPS, TFT).
 */
extern GooeyBackends ACTIVE_BACKEND;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GOOEY_BACKEND_INTERNAL_H
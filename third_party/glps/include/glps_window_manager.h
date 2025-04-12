/**
 * @file glps_window_manager.h
 * @brief Header file for the GLPS window manager, handling window creation,
 * rendering, and input events.
 */

#ifndef GLPS_WINDOW_MANAGER_H
#define GLPS_WINDOW_MANAGER_H

#include "glps_common.h"

/**
 * @brief Initializes the GLPS Window Manager.
 * @return Pointer to the initialized GLPS Window Manager.
 */
glps_WindowManager *glps_wm_init(void);

/**
 * @brief Creates a new window with the specified title and dimensions.
 * @param wm Pointer to the GLPS Window Manager.
 * @param title Title of the new window.
 * @param width Width of the new window in pixels.
 * @param height Height of the new window in pixels.
 * @return The ID of the created window.
 */
size_t glps_wm_window_create(glps_WindowManager *wm, const char *title,
                             int width, int height);

/**
 * @brief Gets dimensions of a window.
 * @param wm Pointer to the GLPS Window Manager.
 * @param window_id ID of the window to get dimensions of.
 * @param width Window width pointer.
 * @param height Window height pointer.
 */
void glps_wm_window_get_dimensions(glps_WindowManager *wm, size_t window_id,
                                   int *width, int *height);

/**
 * @brief Allows user to set callback to handle window resize.
 * @param wm Pointer to the GLPS Window Manager.
 * @param window_resize_callback user-set window resize callback.
 * @param data Additional data to pass to the callback.
 */
void glps_wm_window_set_resize_callback(
    glps_WindowManager *wm,
    void (*window_resize_callback)(size_t window_id, int width, int height,
                                   void *data),
    void *data);

/**
 * @brief Allows user to set callback to handle window frame update.
 * @param wm Pointer to the GLPS Window Manager.
 * @param window_frame_update_callback user-set window frame update callback.
 * @param data Additional data to pass to the callback.
 */
void glps_wm_window_set_frame_update_callback(
    glps_WindowManager *wm,
    void (*window_frame_update_callback)(size_t window_id, void *data),
    void *data);

/**
 * @brief Allows user to set callback to handle window resize.
 * @param wm Pointer to the GLPS Window Manager.
 * @param window_close_callback user-set window close callback.
 * @param data Additional data to pass to the callback.
 */
void glps_wm_window_set_close_callback(
    glps_WindowManager *wm,
    void (*window_close_callback)(size_t window_id, void *data), void *data);

/**
 * @brief Sets the OpenGL context of a specific window as the current context.
 * @param wm Pointer to the GLPS Window Manager.
 * @param window_id ID of the window to set the context for.
 */
void glps_wm_set_window_ctx_curr(glps_WindowManager *wm, size_t window_id);

/**
 * @brief Swaps the front and back buffers for the specified window.
 * @param wm Pointer to the GLPS Window Manager.
 * @param window_id ID of the window to swap buffers for.
 */
void glps_wm_swap_buffers(glps_WindowManager *wm, size_t window_id);

/**
 * @brief Sets the swap interval for buffer swaps.
 * @param wm Pointer to the GLPS Window Manager.
 * @param swap_interval Number of vertical refreshes between buffer swaps.
 */
void glps_wm_swap_interval(glps_WindowManager *wm, unsigned int swap_interval);

void glps_wm_window_update(glps_WindowManager *wm, size_t window_id);

/**
 * @brief Destroys the specified window.
 * @param wm Pointer to the GLPS Window Manager.
 * @param window_id ID of the window to destroy.
 */
void glps_wm_window_destroy(glps_WindowManager *wm, size_t window_id);

/**
 * @brief Cleans up and destroys the GLPS Window Manager.
 * @param wm Pointer to the GLPS Window Manager.
 */
void glps_wm_destroy(glps_WindowManager *wm);

/**
 * @brief Returns the total window count.
 * @param wm Pointer to the GLPS Window Manager.
 * @return total window count. 
 */
size_t glps_wm_get_window_count(glps_WindowManager *wm);


bool glps_wm_should_close(glps_WindowManager *wm);

/* ======= Events: I/O Devices ======= */

/**
 * @brief Sets the callback for keyboard focus gain events.
 * @param wm Pointer to the GLPS Window Manager.
 * @param keyboard_enter_callback Function to call on keyboard focus gain.
 */
void glps_wm_set_keyboard_enter_callback(
    glps_WindowManager *wm,
    void (*keyboard_enter_callback)(size_t window_id, void *data), void *data);

/**
 * @brief Sets the callback for keyboard focus loss events.
 * @param wm Pointer to the GLPS Window Manager.
 * @param keyboard_leave_callback Function to call on keyboard focus loss.
 */
void glps_wm_set_keyboard_leave_callback(
    glps_WindowManager *wm,
    void (*keyboard_leave_callback)(size_t window_id, void *data), void *data);

/**
 * @brief Sets the callback for key press and release events.
 * @param wm Pointer to the GLPS Window Manager.
 * @param keyboard_callback Function to call on key events.
 * @note There is a known issue with some keys not returning values.
 */
void glps_wm_set_keyboard_callback(glps_WindowManager *wm,
                                   void (*keyboard_callback)(size_t window_id,
                                                             bool state,
                                                             const char *value,
                                                             void *data),
                                   void *data);

/* ======= Mouse/Trackpad Events ======= */

/**
 * @brief Sets the callback for mouse enter events.
 * @param wm Pointer to the GLPS Window Manager.
 * @param mouse_enter_callback Function to call when the mouse enters a window.
 */
void glps_wm_set_mouse_enter_callback(
    glps_WindowManager *wm,
    void (*mouse_enter_callback)(size_t window_id, double mouse_x,
                                 double mouse_y, void *data),
    void *data);

/**
 * @brief Sets the callback for mouse leave events.
 * @param wm Pointer to the GLPS Window Manager.
 * @param mouse_leave_callback Function to call when the mouse leaves a window.
 */
void glps_wm_set_mouse_leave_callback(
    glps_WindowManager *wm,
    void (*mouse_leave_callback)(size_t window_id, void *data), void *data);

/**
 * @brief Sets the callback for mouse movement events.
 * @param wm Pointer to the GLPS Window Manager.
 * @param mouse_move_callback Function to call on mouse movement.
 */
void glps_wm_set_mouse_move_callback(
    glps_WindowManager *wm,
    void (*mouse_move_callback)(size_t window_id, double mouse_x,
                                double mouse_y, void *data),
    void *data);

/**
 * @brief Sets the callback for mouse button events.
 * @param wm Pointer to the GLPS Window Manager.
 * @param mouse_click_callback Function to call on mouse button press or
 * release.
 */
void glps_wm_set_mouse_click_callback(
    glps_WindowManager *wm,
    void (*mouse_click_callback)(size_t window_id, bool state, void *data),
    void *data);

/**
 * @brief Sets the callback for mouse scroll events.
 * @param wm Pointer to the GLPS Window Manager.
 * @param mouse_scroll_callback Function to call on mouse scroll events.
 */
void glps_wm_set_scroll_callback(
    glps_WindowManager *wm,
    void (*mouse_scroll_callback)(size_t window_id, GLPS_SCROLL_AXES axe,
                                  GLPS_SCROLL_SOURCE source, double value,
                                  int discrete, bool is_stopped, void *data),
    void *data);

/* ======= Touchscreen Events ======= */

/**
 * @brief Sets the callback for touch events.
 * @param wm Pointer to the GLPS Window Manager.
 * @param touch_callback Function to call on touch events.
 */
void glps_wm_set_touch_callback(
    glps_WindowManager *wm,
    void (*touch_callback)(size_t window_id, int id, double touch_x,
                           double touch_y, bool state, double major,
                           double minor, double orientation, void *data),
    void *data);

/* ======= Clipboard ======= */
/**
 * @brief Attaches data to Clipboard.
 * @param wm Pointer to the GLPS Window Manager.
 * @param mime The mime type.
 * @param data The data to attach to the Clipboard.
 */
void glps_wm_attach_to_clipboard(glps_WindowManager *wm, char *mime,
                                 char *data);
/**
 * @brief Gets data from clipboard.
 * @param wm Pointer to the GLPS Window Manager.
 * @param  data The data attached to the Clipboard.
 * @param data_size The size of the data buffer you're saving Clipboard content
 * to.
 */
void glps_wm_get_from_clipboard(glps_WindowManager *wm, char *data,
                                size_t data_size);

/* ======= Drag & Drop ======= */
/**
 * @brief Attaches data to Clipboard.
 * @param wm Pointer to the window manager struct.
 * @param origin_window The id of the window where the drag & drop operation is
 * initiated.
 * @param mime Mime type of data.
 * @param buff The data buffer.
 * @param drag_n_drop_callback The callback that gets called when a drag & drop
 * operation happens.
 * @param data Additional data set by the user.
 */
void glps_wm_start_drag_n_drop(
    glps_WindowManager *wm, size_t origin_window_id,
    void (*drag_n_drop_callback)(size_t origin_window_id, char *mime,
                                 char *buff, int x, int y, void *data),
    void *data);

/* ======= Utilities ======= */
double glps_wm_get_fps(glps_WindowManager *wm, size_t window_id);

void *glps_get_proc_addr(const char *name) ;

#endif // GLPS_WINDOW_MANAGER_H

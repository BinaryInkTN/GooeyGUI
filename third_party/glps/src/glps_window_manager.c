#include "glps_window_manager.h"
#include "utils/logger/pico_logger.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// *=========== WIN32 ===========* //
#ifdef GLPS_USE_WIN32
#include <glps_wgl_context.h>
#include <glps_win32.h>
#include <windows.h>
#endif

// *=========== WAYLAND ===========* //
#ifdef GLPS_USE_WAYLAND
#include "glps_wayland.h"
#include <EGL/eglplatform.h>
#include <glps_egl_context.h>
#include <glps_wgl_context.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <xkbcommon/xkbcommon.h>
#endif

// *=========== X11 ===========* //

#ifdef GLPS_USE_X11
#include "glps_x11.h"
#endif

void glps_wm_set_mouse_enter_callback(
    glps_WindowManager *wm,
    void (*mouse_enter_callback)(size_t window_id, double mouse_x,
                                 double mouse_y, void *data),
    void *data)
{

  if (wm == NULL || mouse_enter_callback == NULL)
  {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.mouse_enter_callback = mouse_enter_callback;
  wm->callbacks.mouse_move_data = data;
}

void glps_wm_set_mouse_leave_callback(
    glps_WindowManager *wm,
    void (*mouse_leave_callback)(size_t window_id, void *data), void *data)
{

  if (wm == NULL || mouse_leave_callback == NULL)
  {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.mouse_leave_callback = mouse_leave_callback;
  wm->callbacks.mouse_leave_data = data;
}

void glps_wm_set_mouse_move_callback(
    glps_WindowManager *wm,
    void (*mouse_move_callback)(size_t window_id, double mouse_x,
                                double mouse_y, void *data),
    void *data)
{

  if (wm == NULL || mouse_move_callback == NULL)
  {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.mouse_move_callback = mouse_move_callback;
  wm->callbacks.mouse_move_data = data;
}

void glps_wm_set_mouse_click_callback(
    glps_WindowManager *wm,
    void (*mouse_click_callback)(size_t window_id, bool state, void *data),
    void *data)
{

  if (wm == NULL || mouse_click_callback == NULL)
  {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.mouse_click_callback = mouse_click_callback;
  wm->callbacks.mouse_click_data = data;
}

void glps_wm_set_scroll_callback(
    glps_WindowManager *wm,
    void (*mouse_scroll_callback)(size_t window_id, GLPS_SCROLL_AXES axe,
                                  GLPS_SCROLL_SOURCE source, double value,
                                  int discrete, bool is_stopped, void *data),
    void *data)
{

  if (wm == NULL || mouse_scroll_callback == NULL)
  {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.mouse_scroll_callback = mouse_scroll_callback;
  wm->callbacks.mouse_scroll_data = data;
}

void glps_wm_set_keyboard_enter_callback(
    glps_WindowManager *wm,
    void (*keyboard_enter_callback)(size_t window_id, void *data), void *data)
{

  if (wm == NULL || keyboard_enter_callback == NULL)
  {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.keyboard_enter_callback = keyboard_enter_callback;
  wm->callbacks.keyboard_enter_data = data;
}

void glps_wm_set_keyboard_callback(glps_WindowManager *wm,
                                   void (*keyboard_callback)(size_t window_id,
                                                             bool state,
                                                             const char *value,
                                                             void *data),
                                   void *data)
{

  if (wm == NULL || keyboard_callback == NULL)
  {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.keyboard_callback = keyboard_callback;
  wm->callbacks.keyboard_data = data;
}

void glps_wm_set_keyboard_leave_callback(
    glps_WindowManager *wm,
    void (*keyboard_leave_callback)(size_t window_id, void *data), void *data)
{

  if (wm == NULL || keyboard_leave_callback == NULL)
  {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.keyboard_leave_callback = keyboard_leave_callback;
  wm->callbacks.keyboard_leave_data = data;
}

void glps_wm_set_touch_callback(
    glps_WindowManager *wm,
    void (*touch_callback)(size_t window_id, int id, double touch_x,
                           double touch_y, bool state, double major,
                           double minor, double orientation, void *data),
    void *data)
{

  if (wm == NULL || touch_callback == NULL)
  {
    LOG_ERROR("Window Manager and/or Touch Callback NULL");
    return;
  }

  wm->callbacks.touch_callback = touch_callback;
  wm->callbacks.touch_data = data;
}

void glps_wm_attach_to_clipboard(glps_WindowManager *wm, char *mime,
                                 char *data)
{

#ifdef GLPS_USE_WAYLAND
  glps_WaylandContext *context = NULL;
  if (wm == NULL || (context = __get_wl_context(wm)) == NULL)
  {
    LOG_ERROR("Couldn't attach data to clipboard, context is NULL.");
    return;
  }

  memset(&wm->clipboard, 0, sizeof(wm->clipboard));
  strcat(wm->clipboard.buff, data);
  strcat(wm->clipboard.mime_type, mime);
  context->data_src =
      wl_data_device_manager_create_data_source(context->data_dvc_manager);
  wl_data_source_add_listener(context->data_src, &data_source_listener, wm);
  wl_data_source_offer(context->data_src, "text/plain");
  wl_data_source_offer(context->data_src, "text/html");
  wl_data_device_set_selection(context->data_dvc, context->data_src,
                               context->keyboard_serial);

#endif

#ifdef GLPS_USE_WIN32

  glps_win32_attach_to_clipboard(wm, "unknown", data);

#endif
}

void glps_wm_get_from_clipboard(glps_WindowManager *wm, char *data,
                                size_t data_size)
{
#ifdef GLPS_USE_WAYLAND
  if (wm == NULL || data == NULL)
  {
    LOG_ERROR("Window Manager and/or data NULL.");
    return;
  }

  memset(data, 0, data_size);
  strncpy(data, wm->clipboard.buff, data_size - 1);
  data[data_size - 1] = '\0';
#endif
#ifdef GLPS_USE_WIN32
  glps_win32_get_from_clipboard(wm, data, data_size);
#endif
}

void glps_wm_start_drag_n_drop(
    glps_WindowManager *wm, size_t origin_window_id,
    void (*drag_n_drop_callback)(size_t origin_window_id, char *mime,
                                 char *buff, int x, int y, void *data),
    void *data)
{
  if (wm == NULL)
  {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

#ifdef GLPS_USE_WIN32
  glps_Win32Context *ctx = (glps_Win32Context *)wm->win32_ctx;
  if (ctx == NULL)
  {
    LOG_ERROR("Win32 context is NULL.");
    return;
  }

  wm->callbacks.drag_n_drop_callback = drag_n_drop_callback;
  wm->callbacks.drag_n_drop_data = data;
  // ctx->current_drag_n_drop_window = origin_window_id;

#endif

#ifdef GLPS_USE_WAYLAND

  glps_WaylandContext *ctx = (glps_WaylandContext *)__get_wl_context(wm);
  if (ctx == NULL)
  {
    LOG_ERROR("Wayland context is NULL.");
    return;
  }

  wm->callbacks.drag_n_drop_callback = drag_n_drop_callback;
  wm->callbacks.drag_n_drop_data = data;
  ctx->current_drag_n_drop_window = origin_window_id;

  struct wl_data_source *source =
      wl_data_device_manager_create_data_source(ctx->data_dvc_manager);
  wl_data_source_add_listener(source, &data_source_listener, wm);
  wl_data_source_offer(source, "text/plain");

  wl_data_source_set_actions(source,
                             WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE |
                                 WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);

  struct wl_surface *icon = NULL;
  wl_data_device_start_drag(ctx->data_dvc, source,
                            wm->windows[origin_window_id]->wl_surface, icon,
                            wm->pointer_event.serial);
#endif
}

void glps_wm_swap_interval(glps_WindowManager *wm, unsigned int swap_interval)
{
#ifdef GLPS_USE_WAYLAND
  eglSwapInterval(wm->egl_ctx->dpy, 0);
#endif
}

void glps_wm_swap_buffers(glps_WindowManager *wm, size_t window_id)
{
#ifdef GLPS_USE_WAYLAND
  glps_egl_swap_buffers(wm, window_id);
#endif

#ifdef GLPS_USE_WIN32
  glps_wgl_swap_buffers(wm, window_id);
#endif
}

void glps_wm_window_set_resize_callback(
    glps_WindowManager *wm,
    void (*window_resize_callback)(size_t window_id, int width, int height,
                                   void *data),
    void *data)
{

  if (wm == NULL)
  {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  wm->callbacks.window_resize_callback = window_resize_callback;
  wm->callbacks.window_resize_data = data;
}

void glps_wm_window_set_frame_update_callback(
    glps_WindowManager *wm,
    void (*window_frame_update_callback)(size_t window_id, void *data),
    void *data)
{

  if (wm == NULL)
  {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  wm->callbacks.window_frame_update_callback = window_frame_update_callback;
  wm->callbacks.window_frame_update_data = data;
}

void glps_wm_window_set_close_callback(
    glps_WindowManager *wm,
    void (*window_close_callback)(size_t window_id, void *data), void *data)
{

  if (wm == NULL)
  {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  wm->callbacks.window_close_callback = window_close_callback;
  wm->callbacks.window_close_data = data;
}

glps_WindowManager *glps_wm_init(void)
{

  glps_WindowManager *wm = malloc(sizeof(glps_WindowManager));
  *wm = (glps_WindowManager){0};
  if (!wm)
  {
    LOG_ERROR("Failed to allocate memory for glps_WindowManager");
    return NULL;
  }
#ifdef GLPS_USE_WAYLAND
  if (!glps_wl_init(wm))
  {
    LOG_ERROR("Wayland init failed. exiting...");
    exit(EXIT_FAILURE);
  }
  glps_egl_init(wm, wm->wayland_ctx->wl_display);

#elif defined(GLPS_USE_WIN32)
  glps_win32_init(wm);
#elif defined(GLPS_USE_X11)
  glps_x11_init(wm);
  glps_egl_init(wm, wm->x11_ctx->display);

#endif

  return wm;
}

void glps_wm_set_window_ctx_curr(glps_WindowManager *wm, size_t window_id)
{
#ifdef GLPS_USE_WAYLAND
  glps_egl_make_ctx_current(wm, window_id);
#endif

#ifdef GLPS_USE_WIN32
  glps_wgl_make_ctx_current(wm, window_id);
#endif
}

void glps_wm_window_get_dimensions(glps_WindowManager *wm, size_t window_id,
                                   int *width, int *height)
{

  if (wm == NULL)
  {
    LOG_ERROR("Couldn't get window dimensions. Window Manager NULL. ");
    return;
  }
#ifdef GLPS_USE_WAYLAND
  glps_WaylandWindow *window = (glps_WaylandWindow *)wm->windows[window_id];

  *width = window->properties.width;
  *height = window->properties.height;
#endif

#ifdef GLPS_USE_WIN32

  glps_win32_get_window_dimensions(wm, window_id, width, height);

#endif
}

void *glps_get_proc_addr(const char *name)
{
#ifdef GLPS_USE_WAYLAND
  return glps_egl_get_proc_addr(name);
#endif
#ifdef GLPS_USE_WIN32
  return glps_wgl_get_proc_addr(name);
#endif

  return NULL;
}

size_t glps_wm_window_create(glps_WindowManager *wm, const char *title,
                             int width, int height)
{

  ssize_t window_id;
#ifdef GLPS_USE_WAYLAND
  window_id = glps_wl_window_create(wm, title, width, height);
#endif

#ifdef GLPS_USE_WIN32
  window_id = glps_win32_window_create(wm, title, width, height);
#endif

#ifdef GLPS_USE_X11
  window_id = glps_x11_window_create(wm, title, width, height);
#endif

  if (window_id < 0)
  {
    LOG_ERROR("Window creation failed.");
  }
  return window_id;
}

void glps_wm_window_destroy(glps_WindowManager *wm, size_t window_id)
{
  if (wm == NULL || window_id >= wm->window_count ||
      wm->windows[window_id] == NULL)
  {
    LOG_ERROR("Invalid window ID or window manager is NULL.");
    return;
  }
#ifdef GLPS_USE_WAYLAND
  glps_wl_window_destroy(wm, window_id);
#endif

#ifdef GLPS_USE_WIN32

#endif
}

double glps_wm_get_fps(glps_WindowManager *wm, size_t window_id)
{
  if (!wm->windows[window_id]->fps_is_init)
  {
#ifdef GLPS_USE_WAYLAND
    clock_gettime(CLOCK_MONOTONIC, &wm->windows[window_id]->fps_start_time);
#endif

#ifdef GLPS_USE_WIN32
    QueryPerformanceCounter(&wm->windows[window_id]->fps_start_time);
    QueryPerformanceFrequency(&wm->windows[window_id]->fps_freq);
#endif

    wm->windows[window_id]->fps_is_init = true;
    return 0.0;
  }
  else
  {
#ifdef GLPS_USE_WAYLAND
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double seconds = (double)(end_time.tv_sec - wm->windows[window_id]->fps_start_time.tv_sec);
    double nanoseconds = (double)(end_time.tv_nsec - wm->windows[window_id]->fps_start_time.tv_nsec);

    if (nanoseconds < 0)
    {
      seconds -= 1.0;
      nanoseconds += 1e9;
    }

    wm->windows[window_id]->fps_start_time = end_time;
    return 1.0 / (seconds + nanoseconds / 1e9);
#endif

#ifdef GLPS_USE_WIN32
    LARGE_INTEGER end_time;
    QueryPerformanceCounter(&end_time);

    double time_taken = (double)(end_time.QuadPart - wm->windows[window_id]->fps_start_time.QuadPart) /
                        (double)wm->windows[window_id]->fps_freq.QuadPart;

    wm->windows[window_id]->fps_start_time = end_time;
    return 1.0 / time_taken;
#endif
  }

  return -1.0f;
}

bool glps_wm_should_close(glps_WindowManager *wm)
{
#ifdef GLPS_USE_WAYLAND
  return glps_wl_should_close(wm);
#endif
#ifdef GLPS_USE_WIN32
  MSG msg = {};
  if (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return false;
  }

  return true;
#endif
#ifdef GLPS_USE_X11
  return glps_x11_should_close(wm);
#endif
}

void glps_wm_destroy(glps_WindowManager *wm)
{
#ifdef GLPS_USE_WAYLAND

  glps_wl_destroy(wm);
#endif

#ifdef GLPS_USE_WIN32
  glps_win32_destroy(wm);
#endif

#ifdef GLPS_USE_X11
  glps_x11_destroy(wm);
#endif

  if (wm)
  {
    free(wm);
    wm = NULL;
  }
}

void glps_wm_window_update(glps_WindowManager *wm, size_t window_id)
{

#ifdef GLPS_USE_WAYLAND
  wl_update(wm, window_id);

#endif

#ifdef GLPS_USE_WIN32
  InvalidateRect(wm->windows[window_id]->hwnd, NULL, TRUE);
#endif

#ifdef GLPS_USE_X11
  glps_x11_window_update(wm, window_id);
#endif
}

size_t glps_wm_get_window_count(glps_WindowManager *wm)
{
  return wm->window_count;
}

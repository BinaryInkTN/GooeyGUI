
#include <glps_egl_context.h>
#include <glps_wayland.h>
#include "utils/logger/pico_logger.h"

void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                      uint32_t serial)
{
  xdg_wm_base_pong(xdg_wm_base, serial);
}

struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

glps_WaylandContext *__get_wl_context(glps_WindowManager *wm)
{
  if (wm == NULL || wm->wayland_ctx == NULL)
  {
    LOG_WARNING("Window manager and/or Wayland context is NULL.");
    return NULL;
  }
  return wm->wayland_ctx;
}

ssize_t __get_window_id_from_surface(glps_WindowManager *wm,
                                     struct wl_surface *surface)
{

  if (wm == NULL || surface == NULL)
  {
    LOG_ERROR("Couldn't get window id from surface, Window manager and/or "
              "Surface is NULL.");
    return -1;
  }

  for (size_t i = 0; i < wm->window_count; ++i)
  {
    if (wm->windows[i] == NULL)
      continue;
    if (surface == wm->windows[i]->wl_surface)
      return i;
  }

  return -1;
}

ssize_t __get_window_id_from_xdg_surface(glps_WindowManager *wm,
                                         struct xdg_surface *surface)
{

  if (wm == NULL || surface == NULL)
  {
    LOG_ERROR("Couldn't get window id from surface, Window manager and/or "
              "Surface is NULL.");
    return -1;
  }

  for (size_t i = 0; i < wm->window_count; ++i)
  {
    if (wm->windows[i] == NULL)
      continue;
    if (surface == wm->windows[i]->xdg_surface)
      return i;
  }

  return -1;
}

void __window_destroy(glps_WindowManager *wm, size_t window_id)
{

  glps_WaylandWindow *window = wm->windows[window_id];
  if (window->frame_args != NULL)
  {
    free(window->frame_args);
    window->frame_args = NULL;
  }

  if (window->zxdg_toplevel_decoration != NULL)
  {
    zxdg_toplevel_decoration_v1_destroy(window->zxdg_toplevel_decoration);
    window->zxdg_toplevel_decoration = NULL;
  }

  if (window->frame_callback != NULL)
  {
    wl_callback_destroy(window->frame_callback);
    window->frame_callback = NULL;
  }

  // eglDestroySurface(wm->egl_ctx->dpy, window->egl_surface);
  wl_egl_window_destroy(window->egl_window);

  xdg_toplevel_destroy(window->xdg_toplevel);
  xdg_surface_destroy(window->xdg_surface);
  wl_surface_destroy(window->wl_surface);

  free(window);

  wm->windows[window_id] = NULL;

  if (window_id == 0)
  {
    LOG_INFO("All windows destroyed. Exiting program.");

    wm->should_close = true; // trigger exit event
  }
}

void wl_update(glps_WindowManager *wm, size_t window_id)
{
  if (wm == NULL)
  {
    return;
  }

  int width = wm->windows[window_id]->properties.width,
      height = wm->windows[window_id]->properties.height;
  wl_surface_damage(wm->windows[window_id]->wl_surface, 0, 0, width, height);
  wl_surface_commit(wm->windows[window_id]->wl_surface);
}

ssize_t __get_window_id_from_xdg_toplevel(glps_WindowManager *wm,
                                          struct xdg_toplevel *toplevel)
{

  if (wm == NULL || toplevel == NULL)
  {
    LOG_ERROR("Couldn't get window id from toplevel, Window manager and/or "
              "toplevel is NULL.");
    return -1;
  }

  for (size_t i = 0; i < wm->window_count; ++i)
  {
    if (wm->windows[i] == NULL)
      continue;

    if (toplevel == wm->windows[i]->xdg_toplevel)
      return i;
  }

  return -1;
}

struct wl_callback_listener frame_callback_listener;
void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
                      uint32_t serial, struct wl_surface *surface,
                      wl_fixed_t surface_x, wl_fixed_t surface_y)
{
  glps_WindowManager *context = (glps_WindowManager *)data;

  ssize_t window_id = __get_window_id_from_surface(context, surface);

  if (window_id < 0)
  {
    LOG_ERROR("Origin window id is invalid.");
    return;
  }

  context->pointer_event.event_mask |= POINTER_EVENT_ENTER;
  context->pointer_event.serial = serial;
  context->pointer_event.surface_x = surface_x,
  context->pointer_event.surface_y = surface_y;

  glps_WaylandContext *wayland_context = __get_wl_context(context);

  if (wayland_context == NULL)
  {
    LOG_ERROR("Couldn't fetch wayland context.");
    return;
  }

  wayland_context->mouse_window_id = (size_t)window_id;
}

void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
                      uint32_t serial, struct wl_surface *surface)
{
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.serial = serial;
  context->pointer_event.event_mask |= POINTER_EVENT_LEAVE;
}

void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time,
                       wl_fixed_t surface_x, wl_fixed_t surface_y)
{
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.event_mask |= POINTER_EVENT_MOTION;
  context->pointer_event.time = time;
  context->pointer_event.surface_x = surface_x,
  context->pointer_event.surface_y = surface_y;
}

void wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
                       uint32_t serial, uint32_t time, uint32_t button,
                       uint32_t state)
{
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.event_mask |= POINTER_EVENT_BUTTON;
  context->pointer_event.time = time;
  context->pointer_event.serial = serial;
  context->pointer_event.button = button, context->pointer_event.state = state;
}

void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time,
                     uint32_t axis, wl_fixed_t value)
{
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.event_mask |= POINTER_EVENT_AXIS;
  context->pointer_event.time = time;
  context->pointer_event.axes[axis].valid = true;
  context->pointer_event.axes[axis].value = value;
}

void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
                            uint32_t axis_source)
{
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.event_mask |= POINTER_EVENT_AXIS_SOURCE;
  context->pointer_event.axis_source = axis_source;
}

void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
                          uint32_t time, uint32_t axis)
{
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.time = time;
  context->pointer_event.event_mask |= POINTER_EVENT_AXIS_STOP;
  context->pointer_event.axes[axis].valid = true;
}

void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
                              uint32_t axis, int32_t discrete)
{
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.event_mask |= POINTER_EVENT_AXIS_DISCRETE;
  context->pointer_event.axes[axis].valid = true;
  context->pointer_event.axes[axis].discrete = discrete;
}

void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer)
{
  glps_WindowManager *context = (glps_WindowManager *)data;
  struct pointer_event *event = &context->pointer_event;
  glps_WaylandContext *wayland_context = __get_wl_context(context);

  if (wayland_context == NULL)
  {
    LOG_ERROR("Couldn't fetch wayland context.");
    return;
  }
  if (event->event_mask & POINTER_EVENT_ENTER)
  {
    // Mouse enter callback
    if (context->callbacks.mouse_enter_callback)
    {
      context->callbacks.mouse_enter_callback(
          wayland_context->mouse_window_id,
          wl_fixed_to_double(event->surface_x),
          wl_fixed_to_double(event->surface_y),
          context->callbacks.mouse_enter_data);
    }
  }

  if (event->event_mask & POINTER_EVENT_LEAVE)
  {
    // Mouse leave callback
    if (context->callbacks.mouse_leave_callback)
    {
      context->callbacks.mouse_leave_callback(
          wayland_context->mouse_window_id,
          context->callbacks.mouse_leave_data);
    }
  }

  if (event->event_mask & POINTER_EVENT_MOTION)
  {
    // Mouse move callback
    if (context->callbacks.mouse_move_callback)
    {
      context->callbacks.mouse_move_callback(
          wayland_context->mouse_window_id,
          wl_fixed_to_double(event->surface_x),
          wl_fixed_to_double(event->surface_y),
          context->callbacks.mouse_move_data);
    }
  }

  if (event->event_mask & POINTER_EVENT_BUTTON)
  {
    char *state = event->state == WL_POINTER_BUTTON_STATE_RELEASED ? "released"
                                                                   : "pressed";

    // Mouse click callback
    if (context->callbacks.mouse_click_callback)
    {
      context->callbacks.mouse_click_callback(
          wayland_context->mouse_window_id,
          event->state == WL_POINTER_BUTTON_STATE_RELEASED ? false : true,
          context->callbacks.mouse_click_data);
    }
  }

  uint32_t axis_events = POINTER_EVENT_AXIS | POINTER_EVENT_AXIS_SOURCE |
                         POINTER_EVENT_AXIS_STOP | POINTER_EVENT_AXIS_DISCRETE;

  if (event->event_mask & axis_events)
  {
    for (size_t i = 0; i < 2; ++i)
    {
      if (!event->axes[i].valid)
      {
        continue;
      }
      // Mouse scroll callback.
      if (context->callbacks.mouse_scroll_callback)
      {
        GLPS_SCROLL_AXES axis_name[2] = {
            [WL_POINTER_AXIS_VERTICAL_SCROLL] = GLPS_SCROLL_V_AXIS,
            [WL_POINTER_AXIS_HORIZONTAL_SCROLL] = GLPS_SCROLL_H_AXIS,
        };

        GLPS_SCROLL_SOURCE axis_source[4] = {
            [WL_POINTER_AXIS_SOURCE_WHEEL] = GLPS_SCROLL_SOURCE_WHEEL,
            [WL_POINTER_AXIS_SOURCE_FINGER] = GLPS_SCROLL_SOURCE_FINGER,
            [WL_POINTER_AXIS_SOURCE_CONTINUOUS] = GLPS_SCROLL_SOURCE_CONTINUOUS,
            [WL_POINTER_AXIS_SOURCE_WHEEL_TILT] = GLPS_SCROLL_SOURCE_WHEEL_TILT,
        };

        GLPS_SCROLL_AXES axe = axis_name[i];
        GLPS_SCROLL_SOURCE source =
            event->event_mask & POINTER_EVENT_AXIS_SOURCE
                ? axis_source[event->axis_source]
                : GLPS_SCROLL_SOURCE_OTHER;
        double value = event->event_mask & POINTER_EVENT_AXIS
                           ? wl_fixed_to_double(event->axes[i].value)
                           : 0.0f;
        int discrete = event->event_mask & POINTER_EVENT_AXIS_DISCRETE
                           ? event->axes[i].discrete
                           : -1;
        bool is_stopped = event->event_mask & POINTER_EVENT_AXIS_STOP;

        context->callbacks.mouse_scroll_callback(
            wayland_context->mouse_window_id,

            axe, source, value, discrete, is_stopped,
            context->callbacks.mouse_scroll_data);
      }
    }
  }
  memset(event, 0, sizeof(*event));
}

struct wl_pointer_listener wl_pointer_listener = {
    .enter = wl_pointer_enter,
    .leave = wl_pointer_leave,
    .motion = wl_pointer_motion,
    .button = wl_pointer_button,
    .axis = wl_pointer_axis,
    .frame = wl_pointer_frame,
    .axis_source = wl_pointer_axis_source,
    .axis_stop = wl_pointer_axis_stop,
    .axis_discrete = wl_pointer_axis_discrete,
};

void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                        uint32_t format, int32_t fd, uint32_t size)
{
  glps_WaylandContext *context = __get_wl_context(data);
  if (context == NULL)
    return;
  assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

  char *map_shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  assert(map_shm != MAP_FAILED);
  struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
      context->xkb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
      XKB_KEYMAP_COMPILE_NO_FLAGS);

  munmap(map_shm, size);
  close(fd);

  struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
  xkb_keymap_unref(context->xkb_keymap);
  xkb_state_unref(context->xkb_state);
  context->xkb_keymap = xkb_keymap;
  context->xkb_state = xkb_state;
}

void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                       uint32_t serial, struct wl_surface *surface,
                       struct wl_array *keys)
{
  glps_WaylandContext *context = __get_wl_context(data);
  if (context == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;
  if (wm->callbacks.keyboard_enter_callback != NULL)
  {
    wm->callbacks.keyboard_enter_callback(context->keyboard_window_id,
                                          wm->callbacks.keyboard_enter_data);
  }
  ssize_t window_id = __get_window_id_from_surface(wm, surface);

  if (window_id < 0)
  {

    LOG_ERROR("Origin window id is invalid.");
    return;
  }
  context->keyboard_serial = serial;
  context->keyboard_window_id = (size_t)window_id;

  uint32_t *key;
  wl_array_for_each(key, keys)
  {
    char buf[128];
    xkb_keysym_t sym = xkb_state_key_get_one_sym(context->xkb_state, *key + 8);
    xkb_keysym_get_name(sym, buf, sizeof(buf));
    fprintf(stderr, "sym: %-12s (%d), ", buf, sym);
    xkb_state_key_get_utf8(context->xkb_state, *key + 8, buf, sizeof(buf));
    fprintf(stderr, "utf8: '%s'\n", buf);
  }
}
void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                     uint32_t serial, uint32_t time, uint32_t key,
                     uint32_t state)
{
  glps_WaylandContext *context = __get_wl_context(data);
  if (context == NULL)
    return;
  char utf8[128] = "";
  char name[128] = "";
  uint32_t keycode = key + 8;
  xkb_keysym_t sym = xkb_state_key_get_one_sym(context->xkb_state, keycode);
  if (sym == XKB_KEY_NoSymbol)
    return;
  if (xkb_keysym_get_name(sym, name, sizeof(name)) <= 0)
  {
    name[0] = '\0';
  }
  ssize_t utf8_len =
      xkb_state_key_get_utf8(context->xkb_state, keycode, utf8, sizeof(utf8));
  if (utf8_len <= 0 || utf8[0] == '\0')
  {
    utf8[0] = '\0';
  }
  glps_WindowManager *wm = (glps_WindowManager *)data;
  if (wm->callbacks.keyboard_callback != NULL)
  {
    wm->callbacks.keyboard_callback(
        context->keyboard_window_id,
        state == WL_KEYBOARD_KEY_STATE_PRESSED ? true : false,
        (utf8[0] != '\0' ? utf8 : name), wm->callbacks.keyboard_data);
  }
}

void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                       uint32_t serial, struct wl_surface *surface)
{
  glps_WindowManager *wm = (glps_WindowManager *)data;
  if (wm->callbacks.keyboard_leave_callback != NULL)
  {
    wm->callbacks.keyboard_leave_callback(wm->wayland_ctx->keyboard_window_id,
                                          wm->callbacks.keyboard_leave_data);
  }
}
void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, uint32_t mods_depressed,
                           uint32_t mods_latched, uint32_t mods_locked,
                           uint32_t group)
{

  glps_WaylandContext *context = __get_wl_context(data);
  if (context == NULL)
    return;
  xkb_state_update_mask(context->xkb_state, mods_depressed, mods_latched,
                        mods_locked, 0, 0, group);
}
void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                             int32_t rate, int32_t delay) {}

struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = wl_keyboard_keymap,
    .enter = wl_keyboard_enter,
    .leave = wl_keyboard_leave,
    .key = wl_keyboard_key,
    .modifiers = wl_keyboard_modifiers,
    .repeat_info = wl_keyboard_repeat_info,
};

struct touch_point *get_touch_point(void *data, int32_t id)
{

  if (data == NULL)
    return NULL;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_event *touch = &wm->touch_event;
  const size_t nmemb = sizeof(touch->points) / sizeof(struct touch_point);
  int invalid = -1;
  for (size_t i = 0; i < nmemb; ++i)
  {
    if (touch->points[i].id == id)
    {
      return &touch->points[i];
    }
    if (invalid == -1 && !touch->points[i].valid)
    {
      invalid = i;
    }
  }
  if (invalid == -1)
  {
    return NULL;
  }
  touch->points[invalid].valid = true;
  touch->points[invalid].id = id;
  return &touch->points[invalid];
}

void wl_touch_down(void *data, struct wl_touch *wl_touch, uint32_t serial,
                   uint32_t time, struct wl_surface *surface, int32_t id,
                   wl_fixed_t x, wl_fixed_t y)
{

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  ssize_t window_id = __get_window_id_from_surface(wm, surface);
  if (window_id < 0)
  {
    LOG_ERROR("Window id is invalid.");
    return;
  }

  struct touch_point *point = get_touch_point(wm, id);
  if (point == NULL)
  {
    return;
  }
  point->event_mask |= TOUCH_EVENT_UP;
  point->surface_x = wl_fixed_to_double(x),
  point->surface_y = wl_fixed_to_double(y);
  wm->touch_event.time = time;
  wm->touch_event.serial = serial;

  glps_WaylandContext *context = __get_wl_context(wm);
  if (context == NULL)
  {
    LOG_ERROR("Couldn't fetch wayland context.");
    return;
  }

  context->touch_window_id = (size_t)window_id;
}

void wl_touch_up(void *data, struct wl_touch *wl_touch, uint32_t serial,
                 uint32_t time, int32_t id)
{

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_point *point = get_touch_point(wm, id);
  if (point == NULL)
  {
    return;
  }
  point->event_mask |= TOUCH_EVENT_UP;
}

void wl_touch_motion(void *data, struct wl_touch *wl_touch, uint32_t time,
                     int32_t id, wl_fixed_t x, wl_fixed_t y)
{

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_point *point = get_touch_point(wm, id);
  if (point == NULL)
  {
    return;
  }
  point->event_mask |= TOUCH_EVENT_MOTION;
  point->surface_x = x, point->surface_y = y;
  wm->touch_event.time = time;
}

void wl_touch_cancel(void *data, struct wl_touch *wl_touch)
{

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  wm->touch_event.event_mask |= TOUCH_EVENT_CANCEL;
}

void wl_touch_shape(void *data, struct wl_touch *wl_touch, int32_t id,
                    wl_fixed_t major, wl_fixed_t minor)
{

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_point *point = get_touch_point(wm, id);
  if (point == NULL)
  {
    return;
  }
  point->event_mask |= TOUCH_EVENT_SHAPE;
  point->major = major, point->minor = minor;
}

void wl_touch_orientation(void *data, struct wl_touch *wl_touch, int32_t id,
                          wl_fixed_t orientation)
{

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_point *point = get_touch_point(wm, id);
  if (point == NULL)
  {
    return;
  }
  point->event_mask |= TOUCH_EVENT_ORIENTATION;
  point->orientation = orientation;
}

void wl_touch_frame(void *data, struct wl_touch *wl_touch)
{

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_event *touch = &wm->touch_event;
  const size_t nmemb = sizeof(touch->points) / sizeof(struct touch_point);
  fprintf(stderr, "touch event @ %d:\n", touch->time);

  for (size_t i = 0; i < nmemb; ++i)
  {
    struct touch_point *point = &touch->points[i];
    if (!point->valid)
    {
      continue;
    }
    if (wm->callbacks.touch_callback)
    {
      wm->callbacks.touch_callback(
          touch->window_id,
          touch->points[i].id,                  // id
          wl_fixed_to_double(point->surface_x), // touch_x
          wl_fixed_to_double(point->surface_y), // touch_y
          (point->event_mask & (TOUCH_EVENT_DOWN | TOUCH_EVENT_UP))
              ? true
              : false,                            // state (down/up)
          wl_fixed_to_double(point->major),       // major
          wl_fixed_to_double(point->minor),       // minor
          wl_fixed_to_double(point->orientation), // orientation
          wm->callbacks.touch_data);
    }
    point->valid = false;
  }
}

struct wl_touch_listener wl_touch_listener = {
    .down = wl_touch_down,
    .up = wl_touch_up,
    .motion = wl_touch_motion,
    .frame = wl_touch_frame,
    .cancel = wl_touch_cancel,
    .shape = wl_touch_shape,
    .orientation = wl_touch_orientation,
};

void wl_seat_capabilities(void *data, struct wl_seat *wl_seat,
                          uint32_t capabilities)
{

  glps_WindowManager *context = (glps_WindowManager *)data;
  bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;
  if (have_pointer && context->wayland_ctx->wl_pointer == NULL)
  {
    context->wayland_ctx->wl_pointer =
        wl_seat_get_pointer(context->wayland_ctx->wl_seat);
    wl_pointer_add_listener(context->wayland_ctx->wl_pointer,
                            &wl_pointer_listener, data);
  }
  else if (!have_pointer && context->wayland_ctx->wl_pointer != NULL)
  {
    wl_pointer_release(context->wayland_ctx->wl_pointer);
    context->wayland_ctx->wl_pointer = NULL;
  }

  bool have_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

  if (have_keyboard && context->wayland_ctx->wl_keyboard == NULL)
  {
    context->wayland_ctx->wl_keyboard =
        wl_seat_get_keyboard(context->wayland_ctx->wl_seat);
    wl_keyboard_add_listener(context->wayland_ctx->wl_keyboard,
                             &wl_keyboard_listener, data);
  }
  else if (!have_keyboard && context->wayland_ctx->wl_keyboard != NULL)
  {
    wl_keyboard_release(context->wayland_ctx->wl_keyboard);
    context->wayland_ctx->wl_keyboard = NULL;
  }

  bool have_touch = capabilities & WL_SEAT_CAPABILITY_TOUCH;

  if (have_touch && context->wayland_ctx->wl_touch == NULL)
  {
    context->wayland_ctx->wl_touch =
        wl_seat_get_touch(context->wayland_ctx->wl_seat);
    wl_touch_add_listener(context->wayland_ctx->wl_touch, &wl_touch_listener,
                          data);
  }
  else if (!have_touch && context->wayland_ctx->wl_touch != NULL)
  {
    wl_touch_release(context->wayland_ctx->wl_touch);
    context->wayland_ctx->wl_touch = NULL;
  }
}

void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name) {}

struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name,
};

void data_source_handle_send(void *data, struct wl_data_source *source,
                             const char *mime_type, int fd)
{
  glps_WindowManager *wm = (glps_WindowManager *)data;
  glps_WaylandContext *context = NULL;

  if (wm == NULL)
  {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  if ((context = __get_wl_context(wm)) == NULL)
  {
    LOG_ERROR("Failed to get Wayland context from Window Manager.");
    return;
  }

  if (fd < 0)
  {
    LOG_ERROR("Invalid file descriptor: %d", fd);
    return;
  }

  LOG_INFO("Copying to clipboard: MIME type=%s, Data Preview=%s", mime_type,
           wm->clipboard.buff);

  if (strcmp(mime_type, wm->clipboard.mime_type) == 0)
  {
    if (write(fd, wm->clipboard.buff, strlen(wm->clipboard.buff)) < 0)
    {
      LOG_ERROR("Error writing data to clipboard pipe.");
    }
  }
  else
  {
    LOG_WARNING("Unsupported MIME type: %s", mime_type);
  }

  if (close(fd) < 0)
  {
    LOG_ERROR("Error closing file descriptor.");
  }
}

void data_source_handle_cancelled(void *data, struct wl_data_source *source)
{
  wl_data_source_destroy(source);
}

void data_source_handle_target(void *data, struct wl_data_source *source,
                               const char *mime_type)
{
  if (mime_type != NULL)
  {
    printf("Destination would accept MIME type if dropped: %s\n", mime_type);
  }
  else
  {
    printf("Destination would reject if dropped\n");
  }
}

enum wl_data_device_manager_dnd_action last_dnd_action =
    WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;

void data_source_handle_action(void *data, struct wl_data_source *source,
                               uint32_t dnd_action)
{
  last_dnd_action = dnd_action;
  switch (dnd_action)
  {
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE:
    printf("Destination would perform a move action if dropped\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY:
  {
    printf("Destination would perform a copy action if dropped\n");
    break;
  }
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE:
    printf("Destination would reject the drag if dropped\n");
    break;
  }
}

void data_source_handle_dnd_drop_performed(void *data,
                                           struct wl_data_source *source)
{
  printf("Drop performed\n");
}

void data_source_handle_dnd_finished(void *data,
                                     struct wl_data_source *source)
{
  switch (last_dnd_action)
  {
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE:
    printf("Destination has accepted the drop with a move action\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY:
    printf("Destination has accepted the drop with a copy action\n");
    break;
  }
}

struct wl_data_source_listener data_source_listener = {
    .send = data_source_handle_send,
    .cancelled = data_source_handle_cancelled,
    .target = data_source_handle_target,
    .action = data_source_handle_action,
    .dnd_drop_performed = data_source_handle_dnd_drop_performed,
    .dnd_finished = data_source_handle_dnd_finished,
};

void data_offer_handle_offer(void *data, struct wl_data_offer *offer,
                             const char *mime_type)
{
  glps_WindowManager *wm = (glps_WindowManager *)data;
  glps_WaylandContext *context = NULL;

  if (wm == NULL)
  {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  if ((context = __get_wl_context(wm)) == NULL)
  {
    LOG_ERROR("Failed to get Wayland context from Window Manager.");
    return;
  }

  //  LOG_INFO("Offered MIME type: %s", mime_type);

  if (strcmp(mime_type, "text/plain") == 0)
  {
    wl_data_offer_accept(offer, context->current_serial, "text/plain");
  }
}
void data_offer_handle_source_actions(void *data, struct wl_data_offer *offer,
                                      uint32_t actions)
{
  if (actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE)
  {
    printf("Drag supports the move action\n");
  }
  if (actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY)
  {
    printf("Drag supports the copy action\n");
  }
}

struct wl_data_offer_listener data_offer_listener = {
    .offer = data_offer_handle_offer,
    .source_actions = data_offer_handle_source_actions,
    .action = data_offer_handle_action,
};

void data_device_handle_drop(void *data, struct wl_data_device *data_device) {
  glps_WindowManager *wm = (glps_WindowManager *)data;
  glps_WaylandContext *context = __get_wl_context(wm);

  if (wm == NULL) {
      LOG_ERROR("Window Manager is NULL.");
      return;
  }

  if ((context = __get_wl_context(wm)) == NULL) {
      LOG_ERROR("Failed to get Wayland context from Window Manager.");
      return;
  }

  if (context->current_drag_offer == NULL) {
      LOG_ERROR("Drag offer is null.");
      return;
  }

  assert(context->current_drag_offer != NULL);

  int fds[2];
  pipe(fds);
  wl_data_offer_receive(context->current_drag_offer, "text/plain", fds[1]);
  close(fds[1]);

  wl_display_roundtrip(context->wl_display);

  char buffer[4096];
  ssize_t bytes_read = read(fds[0], buffer, sizeof(buffer));

  if (wm->callbacks.drag_n_drop_callback) {
      // Pass the drop coordinates to the callback
      wm->callbacks.drag_n_drop_callback(
          context->mouse_window_id, "text/plain", buffer,
          context->drop_coordinates.x, context->drop_coordinates.y,
          wm->callbacks.drag_n_drop_data
      );
  }

  close(fds[0]);

  wl_data_offer_finish(context->current_drag_offer);
  wl_data_offer_destroy(context->current_drag_offer);
  context->current_drag_offer = NULL;
}

void data_offer_handle_action(void *data, struct wl_data_offer *offer,
                              uint32_t dnd_action)
{
  switch (dnd_action)
  {
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE:
    // printf("A move action would be performed if dropped\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY:
    // printf("A copy action would be performed if dropped\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE:
    // printf("The drag would be rejected if dropped\n");
    break;
  }
}

void data_device_handle_data_offer(void *data,
                                   struct wl_data_device *data_device,
                                   struct wl_data_offer *offer)
{
  glps_WindowManager *wm = (glps_WindowManager *)data;

  if (wm == NULL)
  {
    LOG_ERROR("Window Manager is NULL during clipboard selection.");
    return;
  }

  wl_data_offer_add_listener(offer, &data_offer_listener, data);
}
void data_device_handle_selection(void *data,
                                  struct wl_data_device *data_device,
                                  struct wl_data_offer *offer)
{
  glps_WindowManager *wm = (glps_WindowManager *)data;

  if (wm == NULL)
  {
    LOG_ERROR("Window Manager is NULL during clipboard selection.");
    return;
  }

  if (offer == NULL)
  {
    LOG_INFO("Clipboard is empty.");
    memset(&wm->clipboard, 0, sizeof(wm->clipboard));
    return;
  }

  int fds[2];
  if (pipe(fds) < 0)
  {
    perror("Failed to create pipe for clipboard data");
    return;
  }

  wl_data_offer_receive(offer, "text/plain", fds[1]);
  close(fds[1]);

  glps_WaylandContext *context = __get_wl_context(wm);
  if (context == NULL)
  {
    LOG_ERROR("Failed to get Wayland context.");
    close(fds[0]);
    return;
  }

  wl_display_roundtrip(context->wl_display);
  char buf[1024];
  size_t buff_size = sizeof(wm->clipboard.buff) - 1;

  ssize_t n;
  wm->clipboard.buff[0] = '\0';

  while ((n = read(fds[0], buf, sizeof(buf) - 1)) > 0)
  {
    buf[n] = '\0';

    if (buff_size > 0)
    {
      size_t to_copy = ((size_t)n < buff_size) ? (size_t)n : buff_size;
      strncat(wm->clipboard.buff, buf, to_copy);
      buff_size -= to_copy;
    }
  }

  if (n < 0)
  {
    LOG_ERROR("Error reading clipboard data.");
  }

  wl_data_offer_destroy(offer);
}
void data_device_handle_enter(void *data, struct wl_data_device *data_device,
                              uint32_t serial, struct wl_surface *surface,
                              wl_fixed_t x, wl_fixed_t y,
                              struct wl_data_offer *offer)
{
  printf("Drag entered surface: %fx%f\n", wl_fixed_to_double(x),
         wl_fixed_to_double(y));
  // Set the current offer
  glps_WaylandContext *ctx = __get_wl_context((glps_WindowManager *)data);
  ctx->current_drag_offer = offer;
  ctx->current_serial = serial;
  wl_data_offer_set_actions(offer, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY,
                            WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
}

void data_device_handle_motion(void *data, struct wl_data_device *data_device,
                                      uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
  glps_WindowManager *wm = (glps_WindowManager *)data;
  glps_WaylandContext *context = __get_wl_context(wm);

  if (context && context->current_drag_offer)
  {
    // Store the coordinates
    context->drop_coordinates.x = wl_fixed_to_int(x);
    context->drop_coordinates.y = wl_fixed_to_int(y);
  }
}

void data_device_handle_leave(void *data, struct wl_data_device *data_device)
{

  glps_WindowManager *wm = (glps_WindowManager *)data;
  if (wm == NULL)
  {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }
  glps_WaylandContext *ctx = (glps_WaylandContext *)__get_wl_context(wm);
  if (ctx == NULL)
  {
    LOG_ERROR("Wayland Context is NULL.");
    return;
  }
  printf("Drag left our surface\n");
  ctx->current_drag_offer = NULL;
}

struct wl_data_device_listener data_device_listener = {
    .data_offer = data_device_handle_data_offer,
    .selection = data_device_handle_selection,
    .enter = data_device_handle_enter,
    .motion = data_device_handle_motion,
    .leave = data_device_handle_leave,
    .drop = data_device_handle_drop,

};

void handle_global(void *data, struct wl_registry *registry, uint32_t id,
                   const char *interface, uint32_t version)
{
  glps_WindowManager *context = (glps_WindowManager *)data;
  glps_WaylandContext *s = (glps_WaylandContext *)context->wayland_ctx;

  LOG_INFO("Attempting to bind interface: %s (id=%u, version=%u)", interface,
           id, version);

  if (strcmp(interface, "wl_compositor") == 0)
  {
    s->wl_compositor =
        wl_registry_bind(registry, id, &wl_compositor_interface, 1);
    if (!s->wl_compositor)
    {
      LOG_ERROR("Failed to bind wl_compositor.");
    }
    else
    {
      LOG_INFO("Successfully bound wl_compositor.");
    }
  }
  else if (strcmp(interface, "xdg_wm_base") == 0)
  {
    s->xdg_wm_base = wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
    if (!s->xdg_wm_base)
    {
      LOG_ERROR("Failed to bind xdg_wm_base.");
    }
    else
    {
      LOG_INFO("Successfully bound xdg_wm_base.");
    }
  }
  else if (strcmp(interface, "zxdg_decoration_manager_v1") == 0)
  {
    s->decoration_manager = wl_registry_bind(
        registry, id, &zxdg_decoration_manager_v1_interface, version);
    if (!s->decoration_manager)
    {
      LOG_ERROR("Failed to bind zxdg_decoration_manager_v1.");
    }
    else
    {
      LOG_INFO("Successfully bound zxdg_decoration_manager_v1.");
    }
  }
  else if (strcmp(interface, wl_seat_interface.name) == 0)
  {
    s->wl_seat = wl_registry_bind(registry, id, &wl_seat_interface, version);
    if (s->wl_seat)
    {
      wl_seat_add_listener(s->wl_seat, &wl_seat_listener, data);
      LOG_INFO("Successfully bound wl_seat and added listener.");
    }
    else
    {
      LOG_ERROR("Failed to bind wl_seat.");
    }
  }
  else if (strcmp(interface, wl_data_device_manager_interface.name) == 0)
  {
    s->data_dvc_manager =
        wl_registry_bind(registry, id, &wl_data_device_manager_interface, 3);
    if (s->data_dvc_manager)
    {
      if (s->wl_seat)
      {
        s->data_dvc = wl_data_device_manager_get_data_device(
            s->data_dvc_manager, s->wl_seat);
        if (s->data_dvc == NULL)
        {
          LOG_ERROR("Failed to get data device.");
        }
        else
        {
          wl_data_device_add_listener(s->data_dvc, &data_device_listener,
                                      context);
        }
      }
      else
      {
        LOG_ERROR("Failed to bind data device manager: No wl_seat found.");
      }
    }
    else
    {
      LOG_ERROR("Failed to bind wl_data_device_manager_interface.");
    }
  }
  else
  {
    LOG_WARNING("Unhandled interface: %s", interface);
  }
}

void handle_global_remove(void *data, struct wl_registry *registry,
                          uint32_t name) {}

struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};

void frame_callback_done(void *data, struct wl_callback *callback,
                         uint32_t time)
{
  frame_callback_args *args = (frame_callback_args *)data;
  glps_WaylandWindow *window =
      (glps_WaylandWindow *)args->wm->windows[args->window_id];

  if (window == NULL)
  {
    return;
  }

  if (callback)
  {
    wl_callback_destroy(callback);
  }

  if (window->wl_surface)
  {
    window->frame_callback = wl_surface_frame(window->wl_surface);
    if (window->frame_callback)
    {
      wl_callback_add_listener(window->frame_callback, &frame_callback_listener,
                               args);
    }
  }

  if (args->wm->callbacks.window_frame_update_callback)
  {
    args->wm->callbacks.window_frame_update_callback(
        args->window_id, args->wm->callbacks.window_frame_update_data);
  }
}

struct wl_callback_listener frame_callback_listener = {.done =
                                                           frame_callback_done};

void handle_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
                               int32_t width, int32_t height,
                               struct wl_array *states)
{
  glps_WindowManager *wm = (glps_WindowManager *)data;

  ssize_t window_id = __get_window_id_from_xdg_toplevel(wm, toplevel);
  if (window_id < 0)
    return;

  glps_WaylandWindow *window = wm->windows[window_id];

  if (width != 0 && height != 0)
  {
    window->properties.height = height;
    window->properties.width = width;
  }

  wl_egl_window_resize(window->egl_window, width, height, 0, 0);

  if (wm->callbacks.window_resize_callback)
  {
    wm->callbacks.window_resize_callback(window_id, window->properties.width,
                                         window->properties.height,
                                         wm->callbacks.window_resize_data);
  }
  wl_update(wm, window_id);
}

void handle_toplevel_close(void *data, struct xdg_toplevel *toplevel)
{
  glps_WindowManager *wm = (glps_WindowManager *)data;
  if (wm == NULL)
  {
    LOG_ERROR("Window Manager is NULL. Can't close window.");
    return;
  }

  ssize_t window_id = __get_window_id_from_xdg_toplevel(wm, toplevel);
  if (window_id < 0)
  {
    LOG_ERROR("Invalid Window ID. Can't close window.");
    return;
  }

  if (wm->callbacks.window_close_callback)
  {
    wm->callbacks.window_close_callback((size_t)window_id,
                                        wm->callbacks.window_close_data);
  }
}

struct xdg_toplevel_listener toplevel_listener = {
    .configure = handle_toplevel_configure,
    .close = handle_toplevel_close,
};

void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                           uint32_t serial)
{
  glps_WindowManager *wm = (glps_WindowManager *)data;

  if (wm == NULL)
  {
    LOG_ERROR("Couldn't configure XDG Surface. Window Manager is NULL.");
    return;
  }

  glps_WaylandContext *ctx = __get_wl_context(wm);
  if (ctx == NULL)
  {
    LOG_ERROR("Couldn't configure XDG Surface. Window Manager is NULL.");
    return;
  }

  xdg_surface_ack_configure(xdg_surface, serial);

  ssize_t window_id = __get_window_id_from_xdg_surface(wm, xdg_surface);
  if (window_id < 0)
  {
    return;
  }

  wm->windows[(size_t)window_id]->serial = serial;
}

struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void _cleanup_wl(glps_WindowManager *wm)
{
  for (size_t i = 0; i < wm->window_count; ++i)
  {
    if (wm->windows[i])
    {
      __window_destroy(wm, i);
    }
  }
  free(wm->windows);
  wm->windows = NULL;

  if (wm->wayland_ctx != NULL)
  {
    if (wm->wayland_ctx->wl_seat != NULL)
    {
      wl_seat_destroy(wm->wayland_ctx->wl_seat);
    }
    if (wm->wayland_ctx->xdg_wm_base != NULL)
    {
      xdg_wm_base_destroy(wm->wayland_ctx->xdg_wm_base);
    }
    if (wm->wayland_ctx->decoration_manager != NULL)
    {
      zxdg_decoration_manager_v1_destroy(wm->wayland_ctx->decoration_manager);
    }

    if (wm->wayland_ctx->wl_compositor != NULL)
    {
      wl_compositor_destroy(wm->wayland_ctx->wl_compositor);
      wm->wayland_ctx->wl_compositor = NULL;
    }
    if (wm->wayland_ctx->wl_registry != NULL)
    {
      wl_registry_destroy(wm->wayland_ctx->wl_registry);
      wm->wayland_ctx->wl_registry = NULL;
    }
    if (wm->wayland_ctx->data_dvc != NULL)
    {
      wl_data_device_destroy(wm->wayland_ctx->data_dvc);
      wm->wayland_ctx->data_dvc = NULL;
    }
    if (wm->wayland_ctx->wl_keyboard != NULL)
    {
      wl_keyboard_destroy(wm->wayland_ctx->wl_keyboard);
      wm->wayland_ctx->wl_keyboard = NULL;
    }

    if (wm->wayland_ctx->wl_touch != NULL)
    {
      wl_touch_destroy(wm->wayland_ctx->wl_touch);
      wm->wayland_ctx->wl_touch = NULL;
    }

    if (wm->wayland_ctx->wl_pointer != NULL)
    {
      wl_pointer_destroy(wm->wayland_ctx->wl_pointer);
      wm->wayland_ctx->wl_pointer = NULL;
    }

    if (wm->wayland_ctx->xkb_keymap != NULL)
    {
      xkb_keymap_unref(wm->wayland_ctx->xkb_keymap);
      wm->wayland_ctx->xkb_keymap = NULL;
    }

    if (wm->wayland_ctx->xkb_state != NULL)
    {
      xkb_state_unref(wm->wayland_ctx->xkb_state);
      wm->wayland_ctx->xkb_state = NULL;
    }

    if (wm->wayland_ctx->xkb_context != NULL)
    {
      xkb_context_unref(wm->wayland_ctx->xkb_context);
      wm->wayland_ctx->xkb_context = NULL;
    }

    if (wm->wayland_ctx->data_dvc_manager != NULL)
    {
      wl_data_device_manager_destroy(wm->wayland_ctx->data_dvc_manager);
      wm->wayland_ctx->data_dvc_manager = NULL;
    }
    if (wm->wayland_ctx->data_src != NULL)
    {
      wl_data_source_destroy(wm->wayland_ctx->data_src);
      wm->wayland_ctx->data_src = NULL;
    }

    wl_display_disconnect(wm->wayland_ctx->wl_display);
    free(wm->wayland_ctx);
    wm->wayland_ctx = NULL;
  }
}

ssize_t glps_wl_window_create(glps_WindowManager *wm, const char *title,
                              int width, int height)
{
  glps_WaylandWindow *window = malloc(sizeof(glps_WaylandWindow));
  if (window == NULL)
  {
    LOG_ERROR("Wayland window allocation failed.");
    return -1;
  }

  window->wl_surface =
      wl_compositor_create_surface(wm->wayland_ctx->wl_compositor);
  if (!window->wl_surface)
  {
    LOG_ERROR("Failed to create wayland surface");
    exit(EXIT_FAILURE);
  }

  window->properties.width = width;
  window->properties.height = height;

  window->fps_start_time = (struct timespec){0};
  window->fps_is_init = false;

  window->xdg_surface = xdg_wm_base_get_xdg_surface(
      wm->wayland_ctx->xdg_wm_base, window->wl_surface);

  if (!window->xdg_surface)
  {
    LOG_ERROR("Failed to create XDG surface");
    exit(EXIT_FAILURE);
  }

  if (xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener,
                               wm) == -1)
  {
    LOG_ERROR("Failed to add XDG surface listener");
    exit(EXIT_FAILURE);
  }

  window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
  if (!window->xdg_toplevel)
  {
    LOG_ERROR("Failed to create toplevel");
    exit(EXIT_FAILURE);
  }

  xdg_toplevel_set_title(window->xdg_toplevel, title);
  strcpy(window->properties.title, title);
  xdg_toplevel_add_listener(window->xdg_toplevel, &toplevel_listener, wm);
  if (wm->wayland_ctx->decoration_manager != NULL)
  {

    window->zxdg_toplevel_decoration =
        zxdg_decoration_manager_v1_get_toplevel_decoration(
            wm->wayland_ctx->decoration_manager, window->xdg_toplevel);
    zxdg_toplevel_decoration_v1_set_mode(
        window->zxdg_toplevel_decoration,
        ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
  }

  wl_surface_commit(window->wl_surface);

  wl_display_roundtrip(wm->wayland_ctx->wl_display);

  window->egl_window = wl_egl_window_create(
      window->wl_surface, window->properties.width, window->properties.height);
  if (!window->egl_window)
  {
    LOG_ERROR("Failed to create EGL window");
    exit(EXIT_FAILURE);
  }

  window->egl_surface =
      eglCreateWindowSurface(wm->egl_ctx->dpy, wm->egl_ctx->conf,
                             (NativeWindowType)window->egl_window, NULL);
  if (window->egl_surface == EGL_NO_SURFACE)
  {
    LOG_ERROR("Failed to create EGL surface");
    exit(EXIT_FAILURE);
  }

  wm->windows[wm->window_count] = window;

  if (wm->window_count == 0)
  {
    glps_egl_create_ctx(wm);
    glps_egl_make_ctx_current(wm, 0);
  }

  // setup frame callback
  frame_callback_args *frame_args =
      (frame_callback_args *)malloc(sizeof(frame_callback_args));
  window->frame_callback = wl_surface_frame(window->wl_surface);
  frame_args->wm = wm;
  frame_args->window_id = wm->window_count;
  window->frame_args = (void *)frame_args;

  wl_callback_add_listener(window->frame_callback, &frame_callback_listener,
                           frame_args);

  return wm->window_count++;
}

bool glps_wl_should_close(glps_WindowManager *wm)
{
  if (wl_display_dispatch(wm->wayland_ctx->wl_display) == -1)
    return true;
  else if (wm->should_close)
    return true;

  return false;
}

void glps_wl_destroy(glps_WindowManager *wm)
{
  if (wm == NULL)
  {
    return;
  }
  glps_egl_destroy(wm);
  _cleanup_wl(wm);
}

void glps_wl_window_destroy(glps_WindowManager *wm, size_t window_id)
{
  __window_destroy(wm, window_id);
}

bool glps_wl_init(glps_WindowManager *wm)
{

  wm->windows = malloc(sizeof(glps_WaylandWindow *) * MAX_WINDOWS);
  if (!wm->windows)
  {
    LOG_ERROR("Failed to allocate memory for windows array");
    free(wm);
    return false;
  }

  wm->wayland_ctx = malloc(sizeof(glps_WaylandContext));
  *wm->wayland_ctx = (glps_WaylandContext){0};
  if (!wm->wayland_ctx)
  {
    LOG_ERROR("Failed to allocate memory for Wayland context");
    free(wm->windows);
    free(wm);
    return false;
  }

  wm->window_count = 0;
  wm->wayland_ctx->wl_touch = NULL;
  wm->wayland_ctx->wl_pointer = NULL;
  wm->wayland_ctx->wl_keyboard = NULL;
  wm->wayland_ctx->xkb_state = NULL;
  wm->wayland_ctx->xkb_keymap = NULL;
  wm->wayland_ctx->xkb_context = NULL;
  wm->wayland_ctx->decoration_manager = NULL;
  wm->wayland_ctx->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

  wm->wayland_ctx->wl_display = wl_display_connect(NULL);
  if (!wm->wayland_ctx->wl_display)
  {
    LOG_ERROR("Failed to connect to Wayland display");
    free(wm->wayland_ctx);
    free(wm->windows);
    free(wm);
    return false;
  }

  wm->wayland_ctx->wl_registry =
      wl_display_get_registry(wm->wayland_ctx->wl_display);
  if (!wm->wayland_ctx->wl_registry)
  {
    LOG_ERROR("Failed to get Wayland registry");
    wl_display_disconnect(wm->wayland_ctx->wl_display);
    free(wm->wayland_ctx);
    free(wm->windows);
    free(wm);
    return false;
  }

  wl_registry_add_listener(wm->wayland_ctx->wl_registry, &registry_listener,
                           wm);

  wl_display_roundtrip(wm->wayland_ctx->wl_display);

  if (wm->wayland_ctx->xdg_wm_base)
  {
    xdg_wm_base_add_listener(wm->wayland_ctx->xdg_wm_base,
                             &xdg_wm_base_listener, NULL);
  }
  else
  {
    LOG_WARNING("xdg_wm_base protocol not supported by compositor");
  }

  if (!wm->wayland_ctx->decoration_manager)
  {
    LOG_WARNING("xdg-decoration protocol not supported by compositor");
  }

  if (!wm->wayland_ctx->wl_compositor || !wm->wayland_ctx->xdg_wm_base)
  {
    LOG_ERROR("Failed to retrieve Wayland compositor or xdg_wm_base");
    wl_registry_destroy(wm->wayland_ctx->wl_registry);
    wl_display_disconnect(wm->wayland_ctx->wl_display);
    free(wm->wayland_ctx);
    free(wm->windows);
    free(wm);
    return false;
  }

  return true;
}

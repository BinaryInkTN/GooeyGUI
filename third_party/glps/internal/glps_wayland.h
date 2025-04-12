#ifndef GLPS_WAYLAND_H
#define GLPS_WAYLAND_H

#ifdef GLPS_USE_WAYLAND

#include "glps_common.h"

// Wayland core functions
void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                      uint32_t serial);
glps_WaylandContext *__get_wl_context(glps_WindowManager *wm);
ssize_t __get_window_id_from_surface(glps_WindowManager *wm,
                                     struct wl_surface *surface);
ssize_t __get_window_id_from_xdg_surface(glps_WindowManager *wm,
                                         struct xdg_surface *surface);
ssize_t __get_window_id_from_xdg_toplevel(glps_WindowManager *wm,
                                          struct xdg_toplevel *toplevel);

/**
 * @brief Updates the specified window, handling rendering and events.
 * @param wm Pointer to the GLPS Window Manager.
 * @param window_id ID of the window to update.
 */
 void wl_update(glps_WindowManager *wm, size_t window_id);

// Pointer event handlers
void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
                      uint32_t serial, struct wl_surface *surface,
                      wl_fixed_t surface_x, wl_fixed_t surface_y);
void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
                      uint32_t serial, struct wl_surface *surface);
void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time,
                       wl_fixed_t surface_x, wl_fixed_t surface_y);
void wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
                       uint32_t serial, uint32_t time, uint32_t button,
                       uint32_t state);
void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time,
                     uint32_t axis, wl_fixed_t value);
void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
                            uint32_t axis_source);
void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
                          uint32_t time, uint32_t axis);
void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
                              uint32_t axis, int32_t discrete);
void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer);

// Keyboard event handlers
void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                        uint32_t format, int32_t fd, uint32_t size);
void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                       uint32_t serial, struct wl_surface *surface,
                       struct wl_array *keys);
void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                     uint32_t serial, uint32_t time, uint32_t key,
                     uint32_t state);
void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                       uint32_t serial, struct wl_surface *surface);
void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, uint32_t mods_depressed,
                           uint32_t mods_latched, uint32_t mods_locked,
                           uint32_t group);
void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                             int32_t rate, int32_t delay);

// Touch event handlers
struct touch_point *get_touch_point(void *data, int32_t id);
void wl_touch_down(void *data, struct wl_touch *wl_touch, uint32_t serial,
                   uint32_t time, struct wl_surface *surface, int32_t id,
                   wl_fixed_t x, wl_fixed_t y);
void wl_touch_up(void *data, struct wl_touch *wl_touch, uint32_t serial,
                 uint32_t time, int32_t id);
void wl_touch_motion(void *data, struct wl_touch *wl_touch, uint32_t time,
                     int32_t id, wl_fixed_t x, wl_fixed_t y);
void wl_touch_cancel(void *data, struct wl_touch *wl_touch);
void wl_touch_shape(void *data, struct wl_touch *wl_touch, int32_t id,
                    wl_fixed_t major, wl_fixed_t minor);
void wl_touch_orientation(void *data, struct wl_touch *wl_touch, int32_t id,
                          wl_fixed_t orientation);
void wl_touch_frame(void *data, struct wl_touch *wl_touch);

// Seat event handlers
void wl_seat_capabilities(void *data, struct wl_seat *wl_seat,
                          uint32_t capabilities);
void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name);

// Data device and clipboard handlers
void data_source_handle_send(void *data, struct wl_data_source *source,
                             const char *mime_type, int fd);
void data_source_handle_cancelled(void *data, struct wl_data_source *source);
void data_source_handle_target(void *data, struct wl_data_source *source,
                               const char *mime_type);
void data_source_handle_action(void *data, struct wl_data_source *source,
                               uint32_t dnd_action);
void data_source_handle_dnd_drop_performed(void *data,
                                           struct wl_data_source *source);
void data_source_handle_dnd_finished(void *data, struct wl_data_source *source);

void data_offer_handle_offer(void *data, struct wl_data_offer *offer,
                             const char *mime_type);
void data_offer_handle_source_actions(void *data, struct wl_data_offer *offer,
                                      uint32_t actions);
void data_offer_handle_action(void *data, struct wl_data_offer *offer,
                              uint32_t dnd_action);

void data_device_handle_data_offer(void *data,
                                   struct wl_data_device *data_device,
                                   struct wl_data_offer *offer);
void data_device_handle_selection(void *data,
                                  struct wl_data_device *data_device,
                                  struct wl_data_offer *offer);
void data_device_handle_enter(void *data, struct wl_data_device *data_device,
                              uint32_t serial, struct wl_surface *surface,
                              wl_fixed_t x, wl_fixed_t y,
                              struct wl_data_offer *offer);
void data_device_handle_motion(void *data, struct wl_data_device *data_device,
                               uint32_t time, wl_fixed_t x, wl_fixed_t y);
void data_device_handle_leave(void *data, struct wl_data_device *data_device);
void data_device_handle_drop(void *data, struct wl_data_device *data_device);

// Registry and global object handlers
void handle_global(void *data, struct wl_registry *registry, uint32_t id,
                   const char *interface, uint32_t version);
void handle_global_remove(void *data, struct wl_registry *registry,
                          uint32_t name);

// Frame callback and window management
void frame_callback_done(void *data, struct wl_callback *callback,
                         uint32_t time);
void handle_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
                               int32_t width, int32_t height,
                               struct wl_array *states);
void handle_toplevel_close(void *data, struct xdg_toplevel *toplevel);
void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                           uint32_t serial);

bool glps_wl_init(glps_WindowManager* wm);

ssize_t glps_wl_window_create(glps_WindowManager *wm, const char *title,
                             int width, int height);

bool glps_wl_should_close(glps_WindowManager *wm);

void glps_wl_window_destroy(glps_WindowManager *wm, size_t window_id);

void glps_wl_destroy();

extern struct xdg_wm_base_listener xdg_wm_base_listener;

extern struct wl_seat_listener wl_seat_listener;

extern struct xdg_toplevel_listener toplevel_listener;

extern struct xdg_surface_listener xdg_surface_listener;

extern struct wl_pointer_listener wl_pointer_listener;

extern struct wl_keyboard_listener wl_keyboard_listener;

extern struct wl_touch_listener wl_touch_listener;

extern struct wl_data_source_listener data_source_listener;

extern struct wl_data_offer_listener data_offer_listener;

extern struct wl_data_device_listener data_device_listener;

extern struct wl_registry_listener registry_listener;

extern struct wl_callback_listener frame_callback_listener;

#endif

#endif

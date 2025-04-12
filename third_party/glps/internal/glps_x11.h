#ifndef GLPS_X11_H
#define GLPS_X11_H

#include "glps_common.h"

void glps_x11_init(glps_WindowManager *wm);

ssize_t glps_x11_window_create(glps_WindowManager *wm, const char *title,
                               int width, int height);

void glps_x11_destroy(glps_WindowManager *wm);
void glps_x11_get_window_dimensions(glps_WindowManager *wm, size_t window_id,
                                    int *width, int *height);

void glps_x11_attach_to_clipboard(glps_WindowManager *wm, char *mime,
                                  char *data);

void glps_x11_get_from_clipboard(glps_WindowManager *wm, char *data,
                                 size_t data_size);

bool glps_x11_should_close(glps_WindowManager *wm);
void glps_x11_window_update(glps_WindowManager *wm, size_t window_id);

#endif
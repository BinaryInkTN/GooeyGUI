#ifndef GLPS_WIN32_H
#define GLPS_WIN32_H

#include "glps_common.h"

void glps_win32_init(glps_WindowManager *wm);

ssize_t glps_win32_window_create(glps_WindowManager *wm, const char *title,
                                 int width, int height);

void glps_win32_destroy(glps_WindowManager *wm);
void glps_win32_get_window_dimensions(glps_WindowManager *wm, size_t window_id,
                                      int *width, int *height);

HDC glps_win32_get_window_hdc(glps_WindowManager *wm, size_t window_id);

void glps_win32_attach_to_clipboard(glps_WindowManager *wm, char *mime,
                                 char *data);

void glps_win32_get_from_clipboard(glps_WindowManager *wm, char *data,
                                size_t data_size);

#endif

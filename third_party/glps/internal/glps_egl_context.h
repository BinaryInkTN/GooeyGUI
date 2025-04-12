#ifndef GLPS_EGL_CONTEXT_H
#define GLPS_EGL_CONTEXT_H

#include <glps_common.h>


void glps_egl_init(glps_WindowManager *wm, EGLNativeDisplayType display);
void glps_egl_create_ctx(glps_WindowManager *wm);
void glps_egl_make_ctx_current(glps_WindowManager *wm, size_t window_id);
void *glps_egl_get_proc_addr(const char *name);
void glps_egl_swap_buffers(glps_WindowManager *wm, size_t window_id);
void glps_egl_destroy(glps_WindowManager *wm);

#endif

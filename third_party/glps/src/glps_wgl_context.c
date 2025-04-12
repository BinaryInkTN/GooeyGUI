#include <glps_wgl_context.h>

#include "utils/logger/pico_logger.h"

void glps_wgl_make_ctx_current(glps_WindowManager *wm, size_t window_id) {
  wglMakeCurrent(wm->windows[window_id]->hdc, wm->win32_ctx->hglrc);
}
void *glps_wgl_get_proc_addr(const char *name) {
    return (void *)wglGetProcAddress(name);
}
void glps_wgl_swap_buffers(glps_WindowManager *wm, size_t window_id) {
  SwapBuffers(wm->windows[window_id]->hdc);
}
void glps_wgl_destroy(glps_WindowManager *wm);

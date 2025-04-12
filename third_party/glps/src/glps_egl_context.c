

#include <glps_egl_context.h>
#include "utils/logger/pico_logger.h"

void glps_egl_init(glps_WindowManager *wm, EGLNativeDisplayType display) {


  wm->egl_ctx = malloc(sizeof(glps_EGLContext));

  EGLint config_attribs[] = {EGL_SURFACE_TYPE,
                             EGL_WINDOW_BIT,
                             EGL_RED_SIZE,
                             8,
                             EGL_GREEN_SIZE,
                             8,
                             EGL_BLUE_SIZE,
                             8,
                             EGL_ALPHA_SIZE,
                             8,
                             EGL_RENDERABLE_TYPE,
                             EGL_OPENGL_BIT,
                             EGL_NONE};

  EGLint major, minor, n;

  wm->egl_ctx->dpy =
      eglGetDisplay((EGLNativeDisplayType)display);
  assert(wm->egl_ctx->dpy);

  if (!eglInitialize(wm->egl_ctx->dpy, &major, &minor)) {
    LOG_ERROR("Failed to initialize EGL");
    exit(EXIT_FAILURE);
  }


  if (!eglChooseConfig(wm->egl_ctx->dpy, config_attribs, &wm->egl_ctx->conf, 1,
                       &n) ||
      n != 1) {
    LOG_ERROR("Failed to choose a valid EGL config");
    exit(EXIT_FAILURE);
  }

  if (!eglBindAPI(EGL_OPENGL_API)) {
    LOG_ERROR("Failed to bind OpenGL API");
    exit(EXIT_FAILURE);
  }
  EGLint error = eglGetError();
  if (error != EGL_SUCCESS) {
    LOG_ERROR("EGL error: %x", error);
  }
  LOG_INFO("EGL initialized successfully (version %d.%d)", major, minor);

}



void glps_egl_create_ctx(glps_WindowManager *wm) {
  static const EGLint context_attribs[] = {
      EGL_CONTEXT_MAJOR_VERSION,
      4,
      EGL_CONTEXT_MINOR_VERSION,
      5,
      EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,
      EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
      EGL_NONE};

  wm->egl_ctx->ctx = eglCreateContext(wm->egl_ctx->dpy, wm->egl_ctx->conf,
                                      EGL_NO_CONTEXT, context_attribs);
  if (wm->egl_ctx->ctx == EGL_NO_CONTEXT) {
    fprintf(stderr, "Failed to create EGL context\n");
    exit(EXIT_FAILURE);
  }
}

void glps_egl_make_ctx_current(glps_WindowManager *wm, size_t window_id) {
  if (!eglMakeCurrent(wm->egl_ctx->dpy, wm->windows[window_id]->egl_surface,
                      wm->windows[window_id]->egl_surface, wm->egl_ctx->ctx)) {
    EGLint error = eglGetError();
    LOG_ERROR("eglMakeCurrent failed: 0x%x", error);
    if (error == EGL_BAD_DISPLAY)
      LOG_ERROR("Invalid EGL display");
    if (error == EGL_BAD_SURFACE)
      LOG_ERROR("Invalid draw or read surface");
    if (error == EGL_BAD_CONTEXT)
      LOG_ERROR("Invalid EGL context");
    if (error == EGL_BAD_MATCH)
      LOG_ERROR("Context or surface attributes mismatch");
    exit(EXIT_FAILURE);
  }
}

void *glps_egl_get_proc_addr(const char* name) { return eglGetProcAddress; }

void glps_egl_destroy(glps_WindowManager *wm) {

  if (wm->egl_ctx->ctx) {
    eglDestroyContext(wm->egl_ctx->dpy, wm->egl_ctx->ctx);
    wm->egl_ctx->ctx = EGL_NO_CONTEXT;
  }
  if (wm->egl_ctx->dpy) {
    eglTerminate(wm->egl_ctx->dpy);
    wm->egl_ctx->dpy = EGL_NO_DISPLAY;
  }
  free(wm->egl_ctx);
  wm->egl_ctx = NULL;
}

void glps_egl_swap_buffers(glps_WindowManager *wm, size_t window_id) {
  eglSwapBuffers(wm->egl_ctx->dpy, wm->windows[window_id]->egl_surface);
}


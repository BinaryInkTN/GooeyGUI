/*
 Copyright 2025 Google LLC

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      https://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include "glps_x11.h"
#include "glps_egl_context.h"
#include "utils/logger/pico_logger.h"

void glps_x11_init(glps_WindowManager *wm)
{
    if (wm == NULL)
    {
        LOG_CRITICAL("Window Manager is NULL. Exiting..");
        exit(EXIT_FAILURE);
    }

    wm->x11_ctx = (glps_X11Context *)malloc(sizeof(glps_X11Context));

    wm->windows = (glps_X11Window **)malloc(sizeof(glps_X11Window *) * MAX_WINDOWS);

    wm->x11_ctx->display = XOpenDisplay(NULL);
    if (!wm->x11_ctx->display)
    {
        LOG_CRITICAL("Failed to open X display\n");
        exit(EXIT_FAILURE);
    }

    wm->x11_ctx->font = XLoadQueryFont(wm->x11_ctx->display, "fixed");
    if (!wm->x11_ctx->font)
    {
        LOG_CRITICAL("Failed to load system font\n");
        XCloseDisplay(wm->x11_ctx->display);
        exit(EXIT_FAILURE);
    }

}

ssize_t glps_x11_window_create(glps_WindowManager *wm, const char *title,
                               int width, int height)
{

    if (wm == NULL || wm->x11_ctx->display == NULL)
    {
        LOG_CRITICAL("Failed to create X11 window. Window manager and/or Display NULL.");
        exit(EXIT_FAILURE);
    }

    int screen = DefaultScreen(wm->x11_ctx->display);
    wm->windows[wm->window_count] = (glps_X11Window *)malloc(sizeof(glps_X11Window));
    wm->windows[wm->window_count]->window = XCreateSimpleWindow(
        wm->x11_ctx->display,
        RootWindow(wm->x11_ctx->display, screen),
        10, 10, width, height, 1,
        BlackPixel(wm->x11_ctx->display, screen),
        WhitePixel(wm->x11_ctx->display, screen));
    XSetWindowBackground(wm->x11_ctx->display, wm->windows[wm->window_count]->window, 0xFFFFFF);
    XStoreName(wm->x11_ctx->display, wm->windows[wm->window_count]->window, title);

    wm->x11_ctx->gc = XCreateGC(wm->x11_ctx->display, wm->windows[wm->window_count]->window, 0, NULL);

    wm->x11_ctx->wm_delete_window = XInternAtom(wm->x11_ctx->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(wm->x11_ctx->display, wm->windows[wm->window_count]->window, &wm->x11_ctx->wm_delete_window, 1);

    XSelectInput(wm->x11_ctx->display, wm->windows[wm->window_count]->window,
                 ExposureMask | ButtonPressMask | KeyPressMask | StructureNotifyMask);

    XMapWindow(wm->x11_ctx->display, wm->windows[wm->window_count]->window);
  
    wm->windows[wm->window_count]->egl_surface =
        eglCreateWindowSurface(wm->egl_ctx->dpy, wm->egl_ctx->conf,
                               (NativeWindowType)wm->windows[wm->window_count]->window, NULL);
    if (wm->windows[wm->window_count]->egl_surface == EGL_NO_SURFACE) {
      LOG_ERROR("Failed to create EGL surface");
      exit(EXIT_FAILURE);
    }
  
    return wm->window_count++;
}

bool glps_x11_should_close(glps_WindowManager *wm)
{
    if (wm == NULL)
    {
        LOG_CRITICAL("Window Manager is NULL. Exiting..");
        exit(EXIT_FAILURE);
    }

    XEvent event;

    if (XPending(wm->x11_ctx->display) > 0)
    {
        XNextEvent(wm->x11_ctx->display, &event);

        if (event.type == ClientMessage && event.xclient.data.l[0] == wm->x11_ctx->wm_delete_window)
        {
            return true;
        }

        return false;
    }
}

void glps_x11_window_update(glps_WindowManager *wm, size_t window_id)
{
    XFlush(wm->x11_ctx->display);

    XClearWindow(wm->x11_ctx->display, wm->windows[window_id]->window);
}

void glps_x11_destroy(glps_WindowManager *wm)
{

    if (wm->windows)
    {
        for (size_t i = 0; i < wm->window_count; ++i)
        {
            if (wm->windows[i]->window)
            {
                XDestroyWindow(wm->x11_ctx->display, wm->windows[i]->window);
            }
            free(wm->windows[i]);
            wm->windows[i] = NULL;
        }
        free(wm->windows);
        wm->windows = NULL;
    }

    if (wm->x11_ctx)
    {
        if (wm->x11_ctx->font)
        {
            XFreeFont(wm->x11_ctx->display, wm->x11_ctx->font);
        }
        if (wm->x11_ctx->display)
        {
            XCloseDisplay(wm->x11_ctx->display);
        }
        if(wm->x11_ctx->gc)
        {
            XFreeGC(wm->x11_ctx->display, wm->x11_ctx->gc);
        }

        free(wm->x11_ctx);
        wm->x11_ctx=NULL;
    }
}
void glps_x11_get_window_dimensions(glps_WindowManager *wm, size_t window_id,
                                    int *width, int *height);

void glps_x11_attach_to_clipboard(glps_WindowManager *wm, char *mime,
                                  char *data);

void glps_x11_get_from_clipboard(glps_WindowManager *wm, char *data,
                                 size_t data_size);

/*
 Copyright (c) 2024 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "gooey_common.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    Display *display;      /**< X11 display connection. */
    Window window;         /**< X11 window identifier. */
    GC gc;                 /**< Graphics context for rendering. */
    Atom wm_delete_window; /**< Atom for handling window close events. */
    XFontStruct *font;     /**< X11 font structure for text rendering. */
} GooeyBackendContext;

static GooeyBackendContext ctx = {0};

int x11_init()
{
    ctx.display = XOpenDisplay(NULL);
    if (!ctx.display)
    {
        LOG_ERROR("Failed to open X display\n");
        return -1;
    }

    ctx.font = XLoadQueryFont(ctx.display, "fixed");
    if (!ctx.font)
    {
        LOG_ERROR("Failed to load system font\n");
        XCloseDisplay(ctx.display);
        return -1;
    }
    return 0;
}

void x11_cleanup()
{
    if (ctx.font)
    {
        XFreeFont(ctx.display, ctx.font);
    }
    if (ctx.display)
    {
        XCloseDisplay(ctx.display);
    }

    memset(&ctx, 0, sizeof(ctx));
}

void x11_update_background()
{
    XSetWindowBackground(ctx.display, ctx.window, active_theme->base);
}

GooeyWindow x11_create_window(const char *title, int width, int height)
{
    GooeyWindow window;

    int screen = DefaultScreen(ctx.display);

    ctx.window = XCreateSimpleWindow(
        ctx.display,
        RootWindow(ctx.display, screen),
        10, 10, width, height, 1,
        BlackPixel(ctx.display, screen),
        WhitePixel(ctx.display, screen));
    XSetWindowBackground(ctx.display, ctx.window, active_theme->base);
    XStoreName(ctx.display, ctx.window, title);

    ctx.gc = XCreateGC(ctx.display, ctx.window, 0, NULL);

    ctx.wm_delete_window = XInternAtom(ctx.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(ctx.display, ctx.window, &ctx.wm_delete_window, 1);

    XSelectInput(ctx.display, ctx.window,
                 ExposureMask | ButtonPressMask | KeyPressMask | StructureNotifyMask);

    XMapWindow(ctx.display, ctx.window);

    return window;
}

void x11_destroy_window()
{

    if (ctx.window)
    {
        XDestroyWindow(ctx.display, ctx.window);
    }
}

void x11_render()
{
    XFlush(ctx.display);
}

void x11_clear()
{
    XClearWindow(ctx.display, ctx.window);
}
char *x11_get_key_from_code(GooeyEvent *gooey_event)
{
    if (gooey_event->type != GOOEY_EVENT_KEY_PRESS)
    {
        return NULL;
    }

    int keycode = gooey_event->data.key_press.keycode;
    unsigned int modifiers = gooey_event->data.key_press.state;

    XKeyEvent key_event;
    key_event.display = ctx.display;
    key_event.window = DefaultRootWindow(ctx.display);
    key_event.keycode = keycode;
    key_event.state = modifiers;

    char buf[32];
    KeySym keysym;

    int len = XLookupString(&key_event, buf, sizeof(buf), &keysym, NULL);

    if (len > 0)
    {

        if (keysym == XK_BackSpace)
        {
            return "Backspace";
        }
        else if (keysym == XK_Return)
        {
            return "Return";
        }
        else if (keysym == XK_Tab)
        {
            return "Tab";
        }
        else
        {
            buf[len] = '\0';
            return strdup(buf);
        }
    }

    return NULL;
}

void x11_set_foreground(unsigned long color)
{

    XSetForeground(ctx.display, ctx.gc,
                   color);
}

void x11_draw_text(int x, int y, const char *text, unsigned long color)
{

    XSetForeground(ctx.display, ctx.gc, color);
    XDrawString(ctx.display, ctx.window, ctx.gc, x, y, text, strlen(text));
}

void x11_fill_rectangle(int x, int y, int width, int height, unsigned long color)
{

    XSetForeground(ctx.display, ctx.gc, color);
    XFillRectangle(ctx.display, ctx.window, ctx.gc, x, y, width, height);
}

void x11_draw_rectangle(int x, int y, int width, int height, unsigned long color)
{

    XSetForeground(ctx.display, ctx.gc, color);
    XDrawRectangle(ctx.display, ctx.window, ctx.gc, x, y,
                   width, height);
}

void x11_get_win_dim(int *width, int *height)
{
    XWindowAttributes attr;
    XGetWindowAttributes(ctx.display, ctx.window, &attr);
    *width = attr.width;
    *height = attr.height;
}

void x11_draw_line(int x1, int y1, int x2, int y2, unsigned long color)
{
    XSetForeground(ctx.display, ctx.gc, color);
    XDrawLine(ctx.display, ctx.window, ctx.gc, x1,
              y1, x2,
              y2);
}

float x11_get_text_width(const char *text, int length)
{
    return XTextWidth(ctx.font, text, length);
}

void x11_fill_arc(int x, int y, int width, int height, int angle1, int angle2)
{
    XFillArc(ctx.display, ctx.window, ctx.gc, x, y, width, height, angle1, angle2);
}

GooeyWindow x11_spawn_window(const char* window, int width, int height) {
    return (GooeyWindow) {0};
}

GooeyEvent x11_handle_events()
{
    XEvent event;
    GooeyEvent gooey_event = {0};

    if (XPending(ctx.display) > 0)
    {
        XNextEvent(ctx.display, &event);

        if (event.type == ClientMessage && event.xclient.data.l[0] == ctx.wm_delete_window)
        {
            gooey_event.type = GOOEY_EVENT_WINDOW_CLOSE;
            return gooey_event;
        }

        if (event.type == ButtonPress)
        {
            gooey_event.type = GOOEY_EVENT_CLICK_PRESS;
            gooey_event.data.click.x = event.xbutton.x;
            gooey_event.data.click.y = event.xbutton.y;
            return gooey_event;
        }

        if (event.type == KeyPress)
        {
            gooey_event.type = GOOEY_EVENT_KEY_PRESS;
            gooey_event.data.key_press.keycode = event.xkey.keycode;
            return gooey_event;
        }

        if (event.type == MotionNotify)
        {
            gooey_event.type = GOOEY_EVENT_MOUSE_MOVE;
            gooey_event.data.mouse_move.x = event.xmotion.x;
            gooey_event.data.mouse_move.y = event.xmotion.y;
            return gooey_event;
        }

        if (event.type == Expose)
        {
            gooey_event.type = GOOEY_EVENT_EXPOSE;
            return gooey_event;
        }
    }

    return gooey_event;
}

GooeyBackend x11_backend = {
    .CreateWindow = x11_create_window,
    .SpawnWindow = x11_spawn_window,
    .HandleEvents = x11_handle_events,
    .DestroyWindow = x11_destroy_window,
    .UpdateBackground = x11_update_background,
    .Init = x11_init,
    .Clear = x11_clear,
    .Cleanup = x11_cleanup,
    .Render = x11_render,
    .SetForeground = x11_set_foreground,
    .GetKeyFromCode = x11_get_key_from_code,
    .DrawText = x11_draw_text,
    .DrawRectangle = x11_draw_rectangle,
    .FillRectangle = x11_fill_rectangle,
    .FillArc = x11_fill_arc,
    .GetWinDim = x11_get_win_dim,
    .DrawLine = x11_draw_line,
    .GetTextWidth = x11_get_text_width

};

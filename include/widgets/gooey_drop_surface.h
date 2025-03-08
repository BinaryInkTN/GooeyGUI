#ifndef GOOEY_DROP_SURFACE_H
#define GOOEY_DROP_SURFACE_H

#include "common/gooey_common.h"

GooeyDropSurface* GooeyDropSurface_Add(GooeyWindow* win, int x, int y, int width, int height, char* default_message, void (*callback)(char* mime, char* file_path));
void GooeyDropSurface_Clear(GooeyDropSurface *drop_surface);
bool GooeyDropSurface_HandleFileDrop(GooeyWindow *win, int mouseX, int mouseY);
void GooeyDropSurface_Draw(GooeyWindow *win);

#endif
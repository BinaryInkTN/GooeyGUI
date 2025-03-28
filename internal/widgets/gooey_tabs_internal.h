#ifndef GOOEY_TABS_INTERNAL_H
#define GOOEY_TABS_INTERNAL_H

#include "common/gooey_common.h"

#define TAB_HEIGHT 40
bool GooeyTabs_HandleClick(GooeyWindow *win, int mouse_x, int mouse_y);
void GooeyTabs_Draw(GooeyWindow *win);

#endif
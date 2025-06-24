#ifndef GOOEY_TABS_INTERNAL_H
#define GOOEY_TABS_INTERNAL_H

#include "common/gooey_common.h"
#if(ENABLE_TABS)
#define TAB_HEIGHT 40
#define TAB_WIDTH 150
#define TAB_ELEMENT_HEIGHT 40
#define TAB_TEXT_PADDING 10
#define TAB_TEXT_SCALE 0.28f
#define TAB_HIGHLIGHT_ALPHA 0.1f
bool GooeyTabs_HandleClick(GooeyWindow *win, int mouse_x, int mouse_y);
void GooeyTabs_Draw(GooeyWindow *win);
#endif

#endif
#ifndef GOOEY_CTXMENU_INTERNAL_H
#define GOOEY_CTXMENU_INTERNAL_H
#include "common/gooey_common.h"

#if (ENABLE_CTXMENU)
void GooeyCtxMenu_Internal_Draw(GooeyWindow *window);
void GooeyCtxMenu_Internal_HandleClick(GooeyWindow *window, int x, int y);
#endif // ENABLE_CTXMENU

#endif // GOOEY_CTXMENU_INTERNAL_H
#ifndef GOOEY_CTXMENU_H
#define GOOEY_CTXMENU_H
#include "common/gooey_common.h"

#ifdef ENABLE_CTXMENU
void GooeyCtxMenu_Set(GooeyWindow* window);
void GooeyCtxMenu_Show(GooeyWindow* window, int x, int y);
void GooeyCtxMenu_Hide(GooeyWindow* window);

#endif


#endif // GOOEY_CTXMENU_H  
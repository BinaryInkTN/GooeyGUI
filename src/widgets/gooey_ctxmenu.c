#include "widgets/gooey_ctxmenu.h"
#include "common/gooey_common.h"
#if (ENABLE_CTXMENU)

void GooeyCtxMenu_Set(GooeyWindow* window)
{
    if (!window || window->ctx_menu)
        return;

    window->ctx_menu->core.type = WIDGET_CTXMENU;
    window->ctx_menu->core.is_visible = true;
    window->ctx_menu->is_open = false;
    window->ctx_menu->menu_item_count = 0;
}

void GooeyCtxMenu_Show(GooeyWindow* window, int x, int y)
{
    if (!window || !window->ctx_menu || window->ctx_menu->menu_item_count == 0)
        return;

    window->ctx_menu->core.x = x;
    window->ctx_menu->core.y = y;
    window->ctx_menu->is_open = true;
    
}

void GooeyCtxMenu_Hide(GooeyWindow* window)
{
    if (!window || !window->ctx_menu)
        return;

    window->ctx_menu->is_open = false;
}
#endif
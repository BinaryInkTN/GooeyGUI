#include "widgets/gooey_ctxmenu.h"
#include "common/gooey_common.h"
#if (ENABLE_CTXMENU)

void GooeyCtxMenu_Set(GooeyWindow* window)
{
    if(!window) return ;
    window->ctx_menu->core.type = WIDGET_CTXMENU;
    window->ctx_menu->core.width = 150; 
    window->ctx_menu->core.height = 200; 
    window->ctx_menu->core.x = 20;
    window->ctx_menu->core.y = 20;
    window->ctx_menu->core.is_visible = true;
    window->ctx_menu->is_open = false;
    window->ctx_menu->menu_item_count = 2;

    // Default menu items
    window->ctx_menu->menu_items[0].callback = NULL;
    window->ctx_menu->menu_items[1].callback = NULL;
    strncpy(window->ctx_menu->menu_items[0].label, "Copy", sizeof(window->ctx_menu->menu_items[0].label));
    strncpy(window->ctx_menu->menu_items[1].label, "Paste", sizeof(window->ctx_menu->menu_items[1].label));

}

void GooeyCtxMenu_Show(GooeyWindow* window, int x, int y)
{
    if (!window || !window->ctx_menu)
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
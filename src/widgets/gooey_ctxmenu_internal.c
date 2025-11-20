#include "common/gooey_common.h"
#include "widgets/gooey_ctxmenu_internal.h"

#if (ENABLE_CTXMENU)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

void GooeyCtxMenu_Internal_Draw(GooeyWindow *window)
{
    if (!window || !window->ctx_menu || !window->ctx_menu->is_open)
        return;

    GooeyCtxMenu *ctx_menu = window->ctx_menu;

    int menu_x = ctx_menu->core.x;
    int menu_y = ctx_menu->core.y;
    int item_height = 30;
    int menu_width = 160;
    int menu_height = ctx_menu->menu_item_count * item_height;

    active_backend->FillRectangle(
        menu_x, menu_y,
        menu_width, menu_height,
        window->active_theme->widget_base, window->creation_id, false, 0.0f, NULL);
  
    for (int i = 0; i < ctx_menu->menu_item_count; ++i)
    {
        int item_y = menu_y + i * item_height;
        active_backend->DrawGooeyText(
            menu_x + 10, item_y + 20,
            ctx_menu->menu_items[i].label,
            window->active_theme->neutral, 0.25f, window->creation_id, NULL);
        if(i < ctx_menu->menu_item_count-1) active_backend->DrawLine(
            menu_x, item_y + item_height - 1,
            menu_x + menu_width, item_y + item_height - 1,
            window->active_theme->neutral, window->creation_id, NULL);
    }
}

void GooeyCtxMenu_Internal_HandleClick(GooeyWindow *window, int x, int y)
{
    if (!window || !window->ctx_menu)
        return;

    GooeyCtxMenu *ctx_menu = window->ctx_menu;
    ctx_menu->is_open = !ctx_menu->is_open;
    ctx_menu->core.x = x;
    ctx_menu->core.y = y;
}

#endif

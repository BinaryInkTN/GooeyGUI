#include "common/gooey_common.h"

#if (ENABLE_CTXMENU)
#include "widgets/gooey_ctxmenu_internal.h"
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
void GooeyCtxMenu_Internal_Draw(GooeyWindow *window)
{
    if (!window || !window->ctx_menu || !window->ctx_menu->is_open)
        return;

    GooeyCtxMenu *ctx_menu = window->ctx_menu;
    if (!ctx_menu->menu_items || ctx_menu->menu_item_count == 0)
    {
        LOG_ERROR("Context menu is empty or not initialized");
        return;
    }

    if (ctx_menu->is_open)
        active_backend->FillRectangle(
            ctx_menu->core.x, ctx_menu->core.y,
            ctx_menu->core.width, ctx_menu->core.height,
            window->active_theme->widget_base, window->creation_id, false, 0.0f, NULL);
}

void GooeyCtxMenu_Internal_HandleClick(GooeyWindow *window, int x, int y)
{
    if (!window || !window->ctx_menu || !window->ctx_menu->is_open)
        return;

    GooeyCtxMenu *ctx_menu = window->ctx_menu;
    ctx_menu->is_open = true;
}

#endif
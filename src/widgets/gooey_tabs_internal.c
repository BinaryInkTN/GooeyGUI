#include "widgets/gooey_tabs_internal.h"
#if (ENABLE_TABS)
#include "backends/gooey_backend_internal.h"
#include "widgets/gooey_window_internal.h"



bool GooeyTabs_HandleClick(GooeyWindow *win, int mouse_x, int mouse_y)
{
    if (!win || !win->tabs) return false;

    for (size_t i = 0; i < win->tab_count; ++i)
    {
        GooeyTabs *tabs = win->tabs[i];
        if (!tabs) continue;

        int tab_width = tabs->core.width / (int)tabs->tab_count;
        int tab_height = tabs->is_sidebar ? TAB_ELEMENT_HEIGHT : TAB_HEIGHT;

        for (size_t j = 0; j < tabs->tab_count; ++j)
        {
            int tab_x = tabs->core.x;
            int tab_y = tabs->core.y;

            if (!tabs->is_sidebar)
            {
                tab_x += tab_width * (int)j;
            }
            else
            {
                tab_y += TAB_ELEMENT_HEIGHT * (int)j;
                tab_width = TAB_WIDTH;
            }

            if (mouse_x >= tab_x && mouse_x < tab_x + tab_width &&
                mouse_y >= tab_y && mouse_y < tab_y + tab_height &&
                tabs->is_open)
            {
                tabs->active_tab_id = tabs->tabs[j].tab_id;
                return true;
            }
        }
    }
    return false;
}

void GooeyTabs_Draw(GooeyWindow *win)
{
    if (!win || !win->tabs) return;

    for (size_t i = 0; i < win->tab_count; ++i)
    {
        GooeyTabs *tabs = win->tabs[i];
        if (!tabs) continue;

        active_backend->DrawRectangle(
            tabs->core.x,
            tabs->core.y,
            tabs->core.width,
            tabs->core.height,
            win->active_theme->widget_base, 1.0f,
            win->creation_id, false, 0.0f);

        if (!tabs->is_sidebar)
        {
            // Horizontal tabs
            const int visible_area_y = tabs->core.y + TAB_HEIGHT;
            const int visible_area_x = tabs->core.x;
            const int visible_area_w = tabs->core.width;
            const int visible_area_h = tabs->core.height - TAB_HEIGHT;

            const int tab_width = tabs->core.width / (int)tabs->tab_count;

            for (size_t j = 0; j < tabs->tab_count; ++j)
            {
                GooeyTab *tab = &tabs->tabs[j];
                const int tab_x = tabs->core.x + tab_width * (int)j;
                const int tab_y = tabs->core.y;

                // Tab underline
                active_backend->DrawLine(
                    tab_x,
                    tab_y + TAB_HEIGHT,
                    tab_x + tab_width,
                    tab_y + TAB_HEIGHT,
                    (tabs->active_tab_id != tab->tab_id) ?
                        win->active_theme->widget_base : win->active_theme->primary,
                    win->creation_id);

                // Tab text
                const int tab_name_x = tab_x + (tab_width / 2) -
                    active_backend->GetTextWidth(tab->tab_name, strlen(tab->tab_name)) / 2;
                const int text_height = active_backend->GetTextHeight(tab->tab_name, strlen(tab->tab_name));

                active_backend->DrawText(
                    tab_name_x,
                    tab_y + (TAB_HEIGHT / 2) + (text_height / 2),
                    tab->tab_name,
                    win->active_theme->neutral,
                    TAB_TEXT_SCALE,
                    win->creation_id);

                // Handle widget visibility
                for (size_t k = 0; k < tab->widget_count; ++k)
                {
                    if (!tab->widgets[k]) continue;

                    GooeyWidget *core = (GooeyWidget *)tab->widgets[k];
                    core->is_visible = (tabs->active_tab_id == tab->tab_id) &&
                        (core->x >= visible_area_x) && (core->x < visible_area_x + visible_area_w) &&
                        (core->y >= visible_area_y) && (core->y < visible_area_y + visible_area_h);
                }
            }
        }
        else
        {
            // Sidebar
            const int visible_area_y = tabs->core.y;
            const int visible_area_x = tabs->is_open ? tabs->core.x + TAB_WIDTH : tabs->core.x;
            const int visible_area_w = tabs->is_open ? tabs->core.width - TAB_WIDTH : tabs->core.width;
            const int visible_area_h = tabs->core.height;

            if (tabs->is_open)
            {
                active_backend->FillRectangle(
                    tabs->core.x,
                    tabs->core.y,
                    TAB_WIDTH,
                    tabs->core.height,
                    win->active_theme->widget_base,
                    win->creation_id,
                    true,
                    2.0f);
            }

            for (size_t j = 0; j < tabs->tab_count; ++j)
            {
                GooeyTab *tab = &tabs->tabs[j];
                const int tab_y = tabs->core.y + TAB_ELEMENT_HEIGHT * (int)j;

                if (tabs->is_open)
                {

                     // Active tab highlight
                    if (tabs->active_tab_id == tab->tab_id)
                    {
                        active_backend->FillRectangle(
                            tabs->core.x,
                            tab_y,
                            TAB_WIDTH,
                            TAB_ELEMENT_HEIGHT,
                            win->active_theme->primary,
                            win->creation_id,
                            false,
                            0.0f);
                    }

                    // Tab text
                    const int text_height = active_backend->GetTextHeight(tab->tab_name, strlen(tab->tab_name));
                    active_backend->DrawText(
                        tabs->core.x + TAB_TEXT_PADDING,
                        tab_y + TAB_ELEMENT_HEIGHT / 2 + text_height / 2,
                        tab->tab_name,
                        win->active_theme->neutral,
                        TAB_TEXT_SCALE,
                        win->creation_id);

                   
                }

                // Handle widget visibility
                for (size_t k = 0; k < tab->widget_count; ++k)
                {
                    if (!tab->widgets[k]) continue;

                    GooeyWidget *core = (GooeyWidget *)tab->widgets[k];
                    core->is_visible = (tabs->active_tab_id == tab->tab_id) &&
                        (core->x >= visible_area_x) && (core->x < visible_area_x + visible_area_w) &&
                        (core->y >= visible_area_y) && (core->y < visible_area_y + visible_area_h);
                }
            }
        }
    }
}
#endif
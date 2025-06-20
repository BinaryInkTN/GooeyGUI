#include "gooey_tabs_internal.h"
#if (ENABLE_TABS)
#include "backends/gooey_backend_internal.h"
#include "gooey_window_internal.h"

bool GooeyTabs_HandleClick(GooeyWindow *win, int mouse_x, int mouse_y)
{
    for (size_t i = 0; i < win->tab_count; ++i)
    {
        GooeyTabs *tabs = win->tabs[i];
        const int tab_height = 40;
        int tab_width = tabs->core.width / tabs->tab_count;

        for (size_t j = 0; j < tabs->tab_count; ++j)
        {
            int tab_x = tabs->core.x + tab_width * j;
            int tab_y = tabs->core.y;

            if (tabs->is_sidebar)
            {
                tab_x = tabs->core.x;
                tab_y = tabs->core.y + TAB_ELEMENT_HEIGHT * j;
                tab_width = TAB_WIDTH;
            }

            if (mouse_x >= tab_x && mouse_x < tab_x + tab_width &&
                mouse_y >= tab_y && mouse_y < tab_y + tab_height)
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
    for (size_t i = 0; i < win->tab_count; ++i)
    {
        GooeyTabs *tabs = win->tabs[i];
        active_backend->DrawRectangle(
            tabs->core.x,
            tabs->core.y,
            tabs->core.width,
            tabs->core.height,
            win->active_theme->widget_base, 1.0f,
            win->creation_id, false, 0.0f);

        if (!tabs->is_sidebar)
        {
            const int visible_area_y = tabs->core.y + TAB_HEIGHT;
            const int visible_area_x = tabs->core.x;
            const int visible_area_w = tabs->core.width;
            const int visible_area_h = tabs->core.height - TAB_HEIGHT;

            const size_t tab_count = tabs->tab_count;
            const int tab_width = (int)((float)tabs->core.width / tab_count);
            for (size_t j = 0; j < tab_count; ++j)
            {
                GooeyTab *tab = &tabs->tabs[j];
                const int tab_x = tabs->core.x + tab_width * j;
                const int tab_y = tabs->core.y;
                // tabs
                active_backend->DrawLine(
                    tab_x,
                    tab_y + TAB_HEIGHT,
                    tab_x + tab_width,
                    tab_y + TAB_HEIGHT,
                    tabs->active_tab_id != tab->tab_id ? win->active_theme->widget_base : win->active_theme->primary,
                    win->creation_id);

                const int tab_name_x = tab_x + ((float)tab_width / 2) - ((float)active_backend->GetTextWidth(tab->tab_name, strlen(tab->tab_name)) / 2);

                active_backend->DrawText(
                    tab_name_x,
                    tab_y + ((float)TAB_HEIGHT / 2) + ((float)active_backend->GetTextHeight(tab->tab_name, strlen(tab->tab_name)) / 2),
                    tab->tab_name,
                    win->active_theme->neutral,
                    0.28f,
                    win->creation_id);

                for (size_t k = 0; k < tab->widget_count; ++k)
                {
                    void *widget = tab->widgets[k];
                    GooeyWidget *core = (GooeyWidget *)widget;

                    // Widgets
                    if (tabs->active_tab_id == tab->tab_id)
                    {
                        if (core->x > visible_area_x && core->x < visible_area_x + visible_area_w &&
                            core->y > visible_area_y && core->y < visible_area_y + visible_area_h)
                        {
                            core->is_visible = true;
                        }
                    }
                    else
                    {
                        core->is_visible = false;
                    }
                }
            }
        }
        else
        {
            // Sidebar
            const int visible_area_y = tabs->core.y;
            const int visible_area_x = tabs->core.x + TAB_WIDTH;
            const int visible_area_w = tabs->core.width - TAB_WIDTH;
            const int visible_area_h = tabs->core.height;

            const size_t tab_count = tabs->tab_count;
            active_backend->FillRectangle(
                tabs->core.x,
                tabs->core.y,
                TAB_WIDTH,
                tabs->core.height,
                win->active_theme->widget_base,
                win->creation_id,
                false,
                0.0f);

            for (size_t j = 0; j < tab_count; ++j)
            {
                GooeyTab *tab = &tabs->tabs[j];

                const int tab_x = tabs->core.x;
                const int tab_y = tabs->core.y + TAB_ELEMENT_HEIGHT * j;
                const int text_height = active_backend->GetTextHeight(tab->tab_name, strlen(tab->tab_name));
                const int tab_name_y = tab_y + TAB_ELEMENT_HEIGHT / 2 + text_height / 2;

                active_backend->DrawText(
                    tabs->core.x + 10,
                    tab_name_y,
                    tab->tab_name,
                    win->active_theme->neutral,
                    0.28f,
                    win->creation_id);

                if (tabs->active_tab_id == j)
                {
                    active_backend->DrawRectangle(
                        tabs->core.x,
                        tab_y,
                        TAB_WIDTH,
                        TAB_ELEMENT_HEIGHT + 5,
                        win->active_theme->primary,
                        0.1f,
                        win->creation_id,
                        false,
                        0.0f);
                }

                for (size_t k = 0; k < tab->widget_count; ++k)
                {
                    void *widget = tab->widgets[k];
                    GooeyWidget *core = (GooeyWidget *)widget;

                    // Widgets
                    if (tabs->active_tab_id == tab->tab_id)
                    {
                        if (core->x >= visible_area_x && core->x < visible_area_x + visible_area_w &&
                            core->y >= visible_area_y && core->y < visible_area_y + visible_area_h)
                        {
                            core->is_visible = true;
                        }
                    }
                    else
                    { /*  */
                        core->is_visible = false;
                    }
                }
            }
        }
    }
}
#endif
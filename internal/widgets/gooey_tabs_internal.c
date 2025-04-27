#include "gooey_tabs_internal.h"
#include "backends/gooey_backend_internal.h"
#include "gooey_window_internal.h"

bool GooeyTabs_HandleClick(GooeyWindow *win, int mouse_x, int mouse_y)
{
    for (size_t i = 0; i < win->tab_count; ++i)
    {
        GooeyTabs *tabs = win->tabs[i];
        const int tab_height = 40;
        const int tab_width = tabs->core.width / tabs->tab_count;

        for (size_t j = 0; j < tabs->tab_count; ++j)
        {
            const int tab_x = tabs->core.x + tab_width * j;
            const int tab_y = tabs->core.y;

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
 
        const int visible_area_y = tabs->core.y + TAB_HEIGHT;
        const int visible_area_x = tabs->core.x;
        const int visible_area_w = tabs->core.width;
        const int visible_area_h = tabs->core.height - TAB_HEIGHT;

        active_backend->DrawRectangle(
            tabs->core.x,
            tabs->core.y,
            tabs->core.width,
            tabs->core.height,
            win->active_theme->widget_base, 1.0f,
            win->creation_id, false, 0.0f);

        const size_t tab_count = tabs->tab_count;
        const int tab_width = (int)((float)tabs->core.width / tab_count);
        for (size_t j = 0; j < tab_count; ++j)
        {
            GooeyTab *tab = &tabs->tabs[j];
            const int tab_x = tabs->core.x + tab_width * j;
            const int tab_y = tabs->core.y;
            // tabs
            active_backend->FillRectangle(
                tab_x,
                tab_y,
                tab_width,
                TAB_HEIGHT,
                tabs->active_tab_id != tab->tab_id ? win->active_theme->widget_base : win->active_theme->primary,
                win->creation_id, false, 0.0f);

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
                    if (core->x > visible_area_x && core->x < visible_area_x + visible_area_w && core->y > visible_area_y && core->y < visible_area_y + visible_area_h)
                        core->is_visible = true;
                }
                else
                    core->is_visible = false;
            }
        }
    }
}
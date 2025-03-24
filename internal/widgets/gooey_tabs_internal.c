#include "gooey_tabs_internal.h"
#include "backends/gooey_backend_internal.h"


void GooeyTabs_Draw(GooeyWindow *win)
{
    const int tab_height = 40;

    for (size_t i = 0; i < win->tab_count; ++i)
    {
        GooeyTabs *tabs = win->tabs[i];

        const int visible_area_y = tabs->core.y + tab_height;
        const int visible_area_x = tabs->core.x;
        const int visible_area_w = tabs->core.width;
        const int visible_area_h = tabs->core.height - tab_height;

        active_backend->DrawRectangle(
            tabs->core.x,
            tabs->core.y,
            tabs->core.width,
            tabs->core.height,
            win->active_theme->widget_base,
            win->creation_id);

        const size_t tab_count = tabs->tab_count;
        const int tab_width = (int)((float)tabs->core.width / tab_count);
        for (size_t j = 0; j < tab_count; ++j)
        {
            GooeyTab *tab = &tabs->tabs[j];
            const int tab_x = tabs->core.x + tab_width * j;
            const int tab_y = tabs->core.y;
            // tabs
            if (tabs->active_tab_id != tab->tab_id)
                active_backend->DrawRectangle(
                    tab_x,
                    tab_y,
                    tab_width,
                    tab_height,
                    win->active_theme->widget_base,
                    win->creation_id);
            else
                active_backend->FillRectangle(
                    tab_x,
                    tab_y,
                    tab_width,
                    tab_height,
                    win->active_theme->primary,
                    win->creation_id);

            const int tab_name_x = tab_x + ((float)tab_width / 2) - ((float)active_backend->GetTextWidth(tab->tab_name, strlen(tab->tab_name)) / 2);

            active_backend->DrawText(
                tab_name_x,
                tab_y + ((float)tab_height / 2) + ((float)active_backend->GetTextHeight(tab->tab_name, strlen(tab->tab_name)) / 2),
                tab->tab_name,
                win->active_theme->neutral,
                0.26f,
                win->creation_id);

            for (size_t k = 0; k < tab->widget_count; ++k)
            {
                // Widgets
                void *widget = tab->widgets[k];
                GooeyWidget* core = (GooeyWidget*) widget;

                core->y += tab_height;
                core->x += tabs->core.x;

            }
        }
    }
}
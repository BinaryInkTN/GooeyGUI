/*
 Copyright (c) 2025 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "widgets/gooey_tabs.h"
#include "core/gooey_backend.h"

GooeyTabs *GooeyTabs_Add( int x, int y, int width, int height)
{
  

    GooeyTabs *tabs_widget = malloc(sizeof(GooeyTabs));
    if (tabs_widget == NULL ){
        LOG_ERROR("Unable to allocate memory to tabs widget");
        return NULL ; 
    }
    *tabs_widget = (GooeyTabs){0};
    tabs_widget->core.type = WIDGET_TABS;
    tabs_widget->core.x = x;
    tabs_widget->core.y = y;
    tabs_widget->core.width = width;
    tabs_widget->core.height = height;
    tabs_widget->tabs = malloc(sizeof(void *[MAX_TABS][MAX_WIDGETS]));
    tabs_widget->tab_count = 0;
    tabs_widget->active_tab_id = 0; // default active tab is the first one.

    return tabs_widget;
}

GooeyTab *GooeyTabs_InsertTab(GooeyTabs *tab_widget, char *tab_name)
{

    if (!tab_widget)
    {
        LOG_ERROR("Couldn't insert tab, tab widget is invalid.");
        return NULL;
    }

    size_t tab_id = tab_widget->tab_count;
    GooeyTab *tab = &tab_widget->tabs[tab_widget->tab_count++];
    tab->tab_id = tab_id;
    tab->widgets = (void **)malloc(sizeof(void *) * MAX_WIDGETS);
    tab->widget_count = 0;

    if (tab_name)
    {
        strncpy(tab->tab_name, tab_name, sizeof(tab->tab_name) - 1);
        tab->tab_name[sizeof(tab->tab_name) - 1] = '\0';
    }
    else
    {
        LOG_WARNING("Invalid tab name, sticking to default.");
        snprintf(tab->tab_name, sizeof(tab->tab_name), "Tab %ld", tab_id);
    }

    return tab;
}

void GooeyTabs_AddWidget(GooeyTab *tab, void *widget)
{
    if (!tab || !widget)
    {
        LOG_ERROR("Couldn't add widget.");
        return;
    }

    tab->widgets[tab->widget_count++] = widget;
}

void GooeyTabs_SetActiveTab(GooeyTabs *tabs, GooeyTab *active_tab)
{
    if (!tabs || !active_tab)
    {
        LOG_ERROR("Couldn't set active tab.");
        return;
    }

    tabs->active_tab_id = active_tab->tab_id;
}

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
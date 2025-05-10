/*
 Copyright (c) 2024 Yassine Ahmed Ali

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

#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"

#include "widgets/gooey_tabs_internal.h"
#include "widgets/gooey_drop_surface_internal.h"
#include "widgets/gooey_tabs_internal.h"
#include "widgets/gooey_image_internal.h"
#include "widgets/gooey_button_internal.h"
#include "widgets/gooey_canvas_internal.h"
#include "widgets/gooey_checkbox_internal.h"
#include "widgets/gooey_dropdown_internal.h"
#include "widgets/gooey_label_internal.h"
#include "widgets/gooey_layout.h"
#include "widgets/gooey_list_internal.h"
#include "widgets/gooey_menu_internal.h"
#include "widgets/gooey_messagebox.h"
#include "widgets/gooey_radiobutton_internal.h"
#include "widgets/gooey_slider_internal.h"
#include "widgets/gooey_textbox_internal.h"
#include "widgets/gooey_plot_internal.h"
#include "widgets/gooey_debug_overlay_internal.h"
#include "widgets/gooey_progressbar_internal.h"
#include "signals/gooey_signals.h"
#include "logger/pico_logger_internal.h"
#include "widgets/gooey_window_internal.h"
#include "widgets/gooey_meter_internal.h"
#include "widgets/gooey_layout_internal.h"
#include <stdarg.h>
#include <string.h>

#include <sys/resource.h>

GooeyBackend *active_backend = NULL;
GooeyBackends ACTIVE_BACKEND = -1;

/* Theme Management Functions */

static GooeyTheme *__default_theme()
{
    GooeyTheme *theme = malloc(sizeof(GooeyTheme));
    if (!theme)
    {
        LOG_ERROR("Failed to allocate memory for default theme");
        return NULL;
    }

    *theme = (GooeyTheme){
        .base = 0xFFFFFF,
        .neutral = 0x000000,
        .primary = 0x2196F3,
        .widget_base = 0xD3D3D3,
        .danger = 0xE91E63,
        .info = 0x2196F3,
        .success = 0x00A152};

    return theme;
}

GooeyTheme *GooeyTheme_LoadFromFile(const char *theme_path)
{
    if (!theme_path)
    {
        LOG_ERROR("Invalid theme path");
        return NULL;
    }

    bool is_loaded = false;
    GooeyTheme *theme = malloc(sizeof(GooeyTheme));
    if (!theme)
    {
        LOG_ERROR("Failed to allocate memory for theme");
        return NULL;
    }

    *theme = parser_load_theme_from_file(theme_path, &is_loaded);

    if (!is_loaded)
    {
        LOG_WARNING("Failed to load theme from %s", theme_path);
        free(theme);
        return NULL;
    }

    return theme;
}

static void __destroy_theme(GooeyTheme *theme)
{
    if (theme)
    {
        free(theme);
    }
}

void GooeyTheme_Destroy(GooeyTheme *theme)
{
    __destroy_theme(theme);
}

/* Window Functions */

void GooeyWindow_RegisterWidget(GooeyWindow *win, void *widget)
{
    GooeyWindow_Internal_RegisterWidget(win, widget);
}

void GooeyWindow_MakeVisible(GooeyWindow *win, bool visibility)
{
    active_backend->MakeWindowVisible(win->creation_id, visibility);
}

void GooeyWindow_MakeResizable(GooeyWindow *msgBoxWindow, bool is_resizable)
{
    active_backend->MakeWindowResizable(is_resizable, msgBoxWindow->creation_id);
}

bool GooeyWindow_HandleCursorChange(GooeyWindow *win, GOOEY_CURSOR *cursor, int x, int y)
{
    for (size_t i = 0; i < win->widget_count; ++i)
    {
        if (x >= win->widgets[i]->x && x <= win->widgets[i]->x + win->widgets[i]->width &&
            y >= win->widgets[i]->y && y <= win->widgets[i]->y + win->widgets[i]->height)
        {
            switch (win->widgets[i]->type)
            {
            case WIDGET_TEXTBOX:
                *cursor = GOOEY_CURSOR_TEXT;
                break;

            case WIDGET_LABEL:
                *cursor = GOOEY_CURSOR_ARROW;
                break;
            default:
                *cursor = GOOEY_CURSOR_HAND;
                break;
            }
            return true;
        }
    }
    return false;
}

void GooeyWindow_SetTheme(GooeyWindow *win, GooeyTheme *theme)
{
    if (!win)
    {
        LOG_ERROR("Invalid window pointer");
        return;
    }

    if (!theme)
    {
        LOG_WARNING("NULL theme provided, using default");
        if (win->default_theme)
        {
            win->active_theme = win->default_theme;
        }
        return;
    }

    if (win->active_theme && win->active_theme != win->default_theme)
    {
        __destroy_theme(win->active_theme);
    }

    win->active_theme = theme;
    active_backend->RequestRedraw(win);
}

bool GooeyWindow_AllocateResources(GooeyWindow *win)
{
    if (!(win->tabs = calloc(MAX_WIDGETS, sizeof(GooeyTabs *))) ||
        !(win->drop_surface = calloc(MAX_WIDGETS, sizeof(GooeyDropSurface *))) ||
        !(win->images = calloc(MAX_WIDGETS, sizeof(GooeyImage *))) ||
        !(win->buttons = calloc(MAX_WIDGETS, sizeof(GooeyButton *))) ||
        !(win->current_event = calloc(1, sizeof(GooeyEvent))) ||
        !(win->labels = calloc(MAX_WIDGETS, sizeof(GooeyLabel *))) ||
        !(win->checkboxes = calloc(MAX_WIDGETS, sizeof(GooeyCheckbox *))) ||
        !(win->radio_buttons = calloc(MAX_WIDGETS, sizeof(GooeyRadioButton *))) ||
        !(win->radio_button_groups = calloc(MAX_WIDGETS, sizeof(GooeyRadioButtonGroup *))) ||
        !(win->sliders = calloc(MAX_WIDGETS, sizeof(GooeySlider *))) ||
        !(win->dropdowns = calloc(MAX_WIDGETS, sizeof(GooeyDropdown *))) ||
        !(win->textboxes = calloc(MAX_WIDGETS, sizeof(GooeyTextbox *))) ||
        !(win->layouts = calloc(MAX_WIDGETS, sizeof(GooeyLayout *))) ||
        !(win->lists = calloc(MAX_WIDGETS, sizeof(GooeyList *))) ||
        !(win->canvas = calloc(MAX_WIDGETS, sizeof(GooeyCanvas *))) ||
        !(win->widgets = calloc(MAX_WIDGETS, sizeof(GooeyWidget *))) ||
        !(win->plots = calloc(MAX_PLOT_COUNT, sizeof(GooeyPlot *))) ||
        !(win->progressbars = calloc(MAX_PLOT_COUNT, sizeof(GooeyProgressBar *))) ||
        !(win->meters = calloc(MAX_WIDGETS, sizeof(GooeyMeter *))))
    {
        return false;
    }

    win->default_theme = __default_theme();
    if (!win->default_theme)
    {
        LOG_ERROR("Failed to allocate memory for default theme");
        return false;
    }

    win->active_theme = win->default_theme;
    return true;
}

void GooeyWindow_FreeResources(GooeyWindow *win)
{
    // Free active theme if it's not the default theme
    if (win->active_theme && win->active_theme != win->default_theme)
    {
        __destroy_theme(win->active_theme);
        win->active_theme = NULL;
    }

    // Free default theme
    if (win->default_theme)
    {
        __destroy_theme(win->default_theme);
        win->default_theme = NULL;
    }

    // Rest of the resource freeing remains the same
    for (size_t i = 0; i < win->canvas_count; ++i)
    {
        if (!win->canvas[i]->elements)
            continue;

        for (int j = 0; j < win->canvas[i]->element_count; ++j)
        {
            CanvaElement *element = &win->canvas[i]->elements[j];
            if (element->args)
            {
                free(element->args);
                element->args = NULL;
            }
        }

        free(win->canvas[i]->elements);
        win->canvas[i]->elements = NULL;

        if (win->canvas[i])
        {
            free(win->canvas[i]);
            win->canvas[i] = NULL;
        }
    }

    if (win->canvas)
    {
        free(win->canvas);
        win->canvas = NULL;
    }

    if (win->meters)
    {
        free(win->meters);
        win->meters = NULL;
    }

    if (win->tabs)
    {
        for (size_t tab_container_index = 0; tab_container_index < win->tab_count; ++tab_container_index)
        {
            GooeyTabs *current_tab_container = win->tabs[tab_container_index];

            if (current_tab_container)
            {
                if (current_tab_container->tabs)
                {
                    for (size_t tab_index = 0; tab_index < current_tab_container->tab_count; ++tab_index)
                    {
                        GooeyTab *current_tab = &current_tab_container->tabs[tab_index];

                        if (current_tab->widgets)
                        {
                            free(current_tab->widgets);
                            current_tab->widgets = NULL;
                        }
                    }

                    free(current_tab_container->tabs);
                    current_tab_container->tabs = NULL;
                }

                free(win->tabs[tab_container_index]);
                win->tabs[tab_container_index] = NULL;
            }
        }

        free(win->tabs);
        win->tabs = NULL;
    }

    if (win->drop_surface)
    {
        for (size_t i = 0; i < win->drop_surface_count; ++i)
        {
            if (win->drop_surface[i])
            {
                free(win->drop_surface[i]);
                win->drop_surface[i] = NULL;
            }
        }
        free(win->drop_surface);
        win->drop_surface = NULL;
    }

    if (win->images)
    {
        for (size_t i = 0; i < win->image_count; ++i)
        {
            if (win->images[i])
            {
                free(win->images[i]);
                win->images[i] = NULL;
            }
        }
        free(win->images);
        win->images = NULL;
    }

    if (win->progressbars)
    {
        for (size_t i = 0; i < win->progressbar_count; ++i)
        {
            if (win->progressbars[i])
            {
                free(win->progressbars[i]);
                win->progressbars[i] = NULL;
            }
        }

        free(win->progressbars);
        win->progressbars = NULL;
    }

    if (win->current_event)
    {
        free(win->current_event);
        win->current_event = NULL;
    }

    if (win->buttons)
    {
        for (size_t i = 0; i < win->button_count; ++i)
        {
            if (win->buttons[i])
            {
                free(win->buttons[i]);
                win->buttons[i] = NULL;
            }
        }
        free(win->buttons);
        win->buttons = NULL;
    }
    if (win->labels)
    {
        for (size_t i = 0; i < win->label_count; ++i)
        {
            if (win->labels[i])
            {
                free(win->labels[i]);
                win->labels[i] = NULL;
            }
        }
        free(win->labels);
        win->labels = NULL;
    }
    if (win->checkboxes)
    {
        for (size_t i = 0; i < win->checkbox_count; ++i)
        {
            if (win->checkboxes[i])
            {
                free(win->checkboxes[i]);
                win->checkboxes[i] = NULL;
            }
        }
        free(win->checkboxes);
        win->checkboxes = NULL;
    }
    if (win->radio_buttons)
    {
        for (size_t i = 0; i < win->radio_button_count; ++i)
        {
            if (win->radio_buttons[i])
            {
                free(win->radio_buttons[i]);
                win->radio_buttons[i] = NULL;
            }
        }
        free(win->radio_buttons);
        win->radio_buttons = NULL;
    }
    if (win->radio_button_groups)
    {
        for (size_t i = 0; i < win->radio_button_group_count; ++i)
        {
            if (win->radio_button_groups[i])
            {
                free(win->radio_button_groups[i]);
                win->radio_button_groups[i] = NULL;
            }
        }
        free(win->radio_button_groups);
        win->radio_button_groups = NULL;
    }
    if (win->menu)
    {
        free(win->menu);
        win->menu = NULL;
    }
    if (win->sliders)
    {
        for (size_t i = 0; i < win->slider_count; ++i)
        {
            if (win->sliders[i])
            {
                free(win->sliders[i]);
                win->sliders[i] = NULL;
            }
        }
        free(win->sliders);
        win->sliders = NULL;
    }
    if (win->dropdowns)
    {
        for (size_t i = 0; i < win->dropdown_count; ++i)
        {
            if (win->dropdowns[i])
            {
                free(win->dropdowns[i]);
                win->dropdowns[i] = NULL;
            }
        }
        free(win->dropdowns);
        win->dropdowns = NULL;
    }
    if (win->textboxes)
    {
        for (size_t i = 0; i < win->textboxes_count; ++i)
        {
            if (win->textboxes[i])
            {
                free(win->textboxes[i]);
                win->textboxes[i] = NULL;
            }
        }
        free(win->textboxes);
        win->textboxes = NULL;
    }
    if (win->layouts)
    {
        for (size_t i = 0; i < win->layout_count; ++i)
        {
            if (win->layouts[i])
            {
                free(win->layouts[i]);
                win->layouts[i] = NULL;
            }
        }
        free(win->layouts);
        win->layouts = NULL;
    }

    if (win->lists)
    {
        for (size_t j = 0; j < win->list_count; j++)
        {
            if (win->lists[j])
            {
                if (win->lists[j]->items)
                {
                    free(win->lists[j]->items);
                    win->lists[j]->items = NULL;
                }
            }
        }

        free(win->lists);
        win->lists = NULL;
    }

    if (win->plots)
    {
        for (size_t i = 0; i < win->plot_count; ++i)
        {
            GooeyPlotData *data = win->plots[i]->data;
            if (data->x_data)
            {
                free(data->x_data);
                data->x_data = NULL;
            }

            if (data->y_data)
            {
                free(data->y_data);
                data->y_data = NULL;
            }
        }

        free(win->plots);
        win->plots = NULL;
    }

    if (win->widgets)
    {
        free(win->widgets);
        win->widgets = NULL;
    }
}

GooeyWindow *GooeyWindow_Create(const char *title, int width, int height, bool visibility)
{
    GooeyWindow *win = active_backend->CreateWindow(title, width, height);
    if (!win)
    {
        LOG_CRITICAL("Failed to create window");
        return NULL;
    }

    win->type = WINDOW_REGULAR;
    if (!GooeyWindow_AllocateResources(win))
    {
        GooeyWindow_FreeResources(win);
        LOG_CRITICAL("Failed to allocate memory for GooeyWindow.");
        exit(EXIT_FAILURE);
    }

    win->menu = NULL;
    win->enable_debug_overlay = false;
    win->tab_count = 0;
    win->visibility = visibility;
    win->image_count = 0;
    win->drop_surface_count = 0;
    win->canvas_count = 0;
    win->button_count = 0;
    win->label_count = 0;
    win->plot_count = 0;
    win->checkbox_count = 0;
    win->radio_button_count = 0;
    win->radio_button_group_count = 0;
    win->slider_count = 0;
    win->dropdown_count = 0;
    win->textboxes_count = 0;
    win->layout_count = 0;
    win->list_count = 0;
    win->progressbar_count = 0;
    win->meter_count = 0;
    win->widget_count = 0;
    win->continuous_redraw = false;

    LOG_INFO("Window created with dimensions (%d, %d).", width, height);
    return win;
}

GooeyWindow GooeyWindow_CreateChild(const char *title, int width, int height, bool visibility)
{
    GooeyWindow win = active_backend->SpawnWindow(title, width, height, visibility);

    win.type = WINDOW_REGULAR;
    if (!GooeyWindow_AllocateResources(&win))
    {
        GooeyWindow_FreeResources(&win);
        active_backend->DestroyWindowFromId(win.creation_id);
        LOG_CRITICAL("Failed to allocate memory for GooeyWindow.");
        return (GooeyWindow){0};
    }

    win.menu = NULL;
    win.visibility = visibility;
    win.canvas_count = 0;
    win.button_count = 0;
    win.label_count = 0;
    win.checkbox_count = 0;
    win.radio_button_count = 0;
    win.radio_button_group_count = 0;
    win.slider_count = 0;
    win.dropdown_count = 0;
    win.textboxes_count = 0;
    win.layout_count = 0;
    win.list_count = 0;
    win.widget_count = 0;
    LOG_INFO("Window created with dimensions (%d, %d).", width, height);
    return win;
}

void GooeyWindow_DrawUIElements(GooeyWindow *win)
{
    if (win == NULL)
        return;

    active_backend->Clear(win);

    // Draw all UI components
    for (size_t i = 0; i < win->layout_count; ++i)
        GooeyLayout_Build(win->layouts[i]);
    GooeyTabs_Draw(win);
    GooeyCanvas_Draw(win);
    GooeyMeter_Draw(win);
    GooeyProgressBar_Draw(win);
    GooeyDropSurface_Draw(win);
    GooeyImage_Draw(win);
    GooeyList_Draw(win);
    GooeyButton_Draw(win);
    GooeyTextbox_Draw(win);
    GooeyCheckbox_Draw(win);
    GooeyRadioButtonGroup_Draw(win);
    GooeySlider_Draw(win);
    GooeyPlot_Draw(win);
    GooeyDropdown_Draw(win);
    GooeyLabel_Draw(win);
    GooeyMenu_Draw(win);
    GooeyDebugOverlay_Draw(win);

    active_backend->Render(win);
}

void GooeyWindow_Redraw(size_t window_id, void *data)
{
    bool needs_redraw = true; // Keep true for now, we need to work on pending events for this to work properly.

    if (!data || !active_backend)
    {
        LOG_CRITICAL("Invalid data or backend in Redraw callback");
        return;
    }

    GooeyWindow **windows = (GooeyWindow **)data;
    if (window_id > active_backend->GetTotalWindowCount() || !windows[window_id])
    {
        LOG_CRITICAL("Invalid window ID or window is NULL");
        return;
    }

    GooeyWindow *window = windows[window_id];
    GooeyEvent *event = (GooeyEvent *)window->current_event;

    int width, height;
    active_backend->GetWinDim(&width, &height, window_id);
    active_backend->SetViewport(window_id, width, height);
    active_backend->UpdateBackground(window);

    needs_redraw |= GooeySlider_HandleDrag(window, event);
    GooeyButton_HandleHover(window, event->mouse_move.x, event->mouse_move.y);
    GooeyMenu_HandleHover(window);
    GooeyDropdown_HandleHover(window, event->mouse_move.x, event->mouse_move.y);
    needs_redraw |= GooeyList_HandleThumbScroll(window, event);

    switch (event->type)
    {

    case GOOEY_EVENT_MOUSE_SCROLL:
        needs_redraw |= GooeyList_HandleScroll(window, event);
        break;

    case GOOEY_EVENT_RESIZE:
        needs_redraw = true;
        break;

    case GOOEY_EVENT_REDRAWREQ:
        needs_redraw = true;
        break;

    case GOOEY_EVENT_KEY_PRESS:
        GooeyTextbox_HandleKeyPress(window, event);
        needs_redraw = true;
        break;

    case GOOEY_EVENT_CLICK_PRESS:
    {
        int mouse_click_x = event->click.x, mouse_click_y = event->click.y;
        needs_redraw |= GooeyList_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyButton_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyDropdown_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyCheckbox_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyRadioButtonGroup_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyTextbox_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyMenu_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyList_HandleThumbScroll(window, event);
        needs_redraw |= GooeyImage_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyTabs_HandleClick(window, mouse_click_x, mouse_click_y);

        break;
    }

    case GOOEY_EVENT_CLICK_RELEASE:

        break;

    case GOOEY_EVENT_DROP:
        needs_redraw |= GooeyDropSurface_HandleFileDrop(window, event->drop_data.drop_x, event->drop_data.drop_y);
        break;

    case GOOEY_EVENT_WINDOW_CLOSE:
        active_backend->DestroyWindowFromId(window_id);
        windows[window_id] = NULL;
        break;

    case GOOEY_EVENT_RESET:
        break;

    default:
        LOG_INFO("Unhandled event type: %d", event->type);
        break;
    }

    if (needs_redraw)
    {
        GooeyWindow_DrawUIElements(window);
        active_backend->ResetEvents(window);
    }
}

void GooeyWindow_Cleanup(int num_windows, GooeyWindow *first_win, ...)
{
    if (!active_backend)
    {
        LOG_CRITICAL("Backend is not initialized");
        return;
    }

    va_list args;
    GooeyWindow *windows[num_windows];

    va_start(args, first_win);
    windows[0] = first_win;

    for (int i = 1; i < num_windows; ++i)
    {
        GooeyWindow *win = va_arg(args, GooeyWindow *);
        windows[i] = win;
    }
    va_end(args);

    for (int i = 0; i < num_windows; ++i)
    {
        if (windows[i])
        {
            GooeyWindow_FreeResources(windows[i]);
            free(windows[i]);
            windows[i] = NULL;
        }
    }

    active_backend->Cleanup();
}

void GooeyWindow_Run(int num_windows, GooeyWindow *first_win, ...)
{
    if (!active_backend)
    {
        LOG_CRITICAL("Backend is not initialized");
        return;
    }

    va_list args;
    GooeyWindow *windows[num_windows];
    va_start(args, first_win);

    LOG_INFO("Starting application");

    windows[0] = first_win;
    GooeyWindow_DrawUIElements(first_win);

    for (int i = 1; i < num_windows; ++i)
    {
        GooeyWindow *win = va_arg(args, GooeyWindow *);
        windows[i] = win;
        GooeyWindow_DrawUIElements(win);
    }
    va_end(args);

    active_backend->SetupCallbacks(GooeyWindow_Redraw, windows);
    active_backend->Run();
}

void GooeyWindow_RequestRedraw(GooeyWindow *win)
{
    LOG_CRITICAL("Window redraw req %ld", win->creation_id);
    active_backend->RequestRedraw(win);
}

void GooeyWindow_SetContinuousRedraw(GooeyWindow *win)
{
    //  win->continuous_redraw = true;
    // removed for now.
}

void GooeyWindow_EnableDebugOverlay(GooeyWindow *win, bool is_enabled)
{
    win->enable_debug_overlay = is_enabled;
}

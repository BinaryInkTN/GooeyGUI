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

#include "core/gooey_backend.h"
#include "event/gooey_event.h"

#include "widgets/gooey_drop_surface.h"
#include "widgets/gooey_image.h"
#include "widgets/gooey_button.h"
#include "widgets/gooey_canvas.h"
#include "widgets/gooey_checkbox.h"
#include "widgets/gooey_dropdown.h"
#include "widgets/gooey_label.h"
#include "widgets/gooey_layout.h"
#include "widgets/gooey_list.h"
#include "widgets/gooey_menu.h"
#include "widgets/gooey_messagebox.h"
#include "widgets/gooey_radiobutton.h"
#include "widgets/gooey_slider.h"
#include "widgets/gooey_textbox.h"
#include "widgets/gooey_plot.h"
#include "signals/gooey_signals.h"
#include <stdarg.h>

#include <sys/resource.h>

GooeyBackend *active_backend = NULL;
GooeyBackends ACTIVE_BACKEND = -1;

void GooeyWindow_RegisterWidget(GooeyWindow *win, GooeyWidget *widget)
{
    //  if (win && win->widgets)
    //        win->widgets[win->widget_count++] = widget;
}

void GooeyWindow_MakeVisible(GooeyWindow *win, bool visibility)
{
    active_backend->MakeWindowVisible(win->creation_id, visibility);
    //
}

void GooeyWindow_MakeResizable(GooeyWindow *msgBoxWindow, bool is_resizable)
{
    active_backend->MakeWindowResizable(0, msgBoxWindow->creation_id);
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

GooeyTheme *GooeyWindow_LoadTheme(const char *theme_path)
{
    return parser_load_theme_from_file(theme_path);
}

void GooeyWindow_SetTheme(GooeyWindow *win, GooeyTheme *theme)
{
    if (!win || !theme)
    {
        LOG_CRITICAL("Couldn't load theme, falling back to default.");
        return;
    }

    win->active_theme = theme;
    active_backend->RequestRedraw(win);
}

bool GooeyWindow_AllocateResources(GooeyWindow *win)
{
    if (!(win->drop_surface = malloc(sizeof(GooeyDropSurface) * MAX_WIDGETS)) ||
        !(win->images = malloc(sizeof(GooeyImage) * MAX_WIDGETS)) ||
        !(win->buttons = malloc(sizeof(GooeyButton) * MAX_WIDGETS)) ||
        !(win->active_theme = malloc(sizeof(GooeyTheme))) ||
        !(win->current_event = malloc(sizeof(GooeyEvent))) ||
        !(win->labels = malloc(sizeof(GooeyLabel) * MAX_WIDGETS)) ||
        !(win->checkboxes = malloc(sizeof(GooeyCheckbox) * MAX_WIDGETS)) ||
        !(win->radio_buttons = malloc(sizeof(GooeyRadioButton) * MAX_WIDGETS)) ||
        !(win->radio_button_groups = malloc(sizeof(GooeyRadioButtonGroup) * MAX_WIDGETS)) ||
        !(win->sliders = malloc(sizeof(GooeySlider) * MAX_WIDGETS)) ||
        !(win->dropdowns = malloc(sizeof(GooeyDropdown) * MAX_WIDGETS)) ||
        !(win->textboxes = malloc(sizeof(GooeyTextbox) * MAX_WIDGETS)) ||
        !(win->layouts = malloc(sizeof(GooeyLayout) * MAX_WIDGETS)) ||
        !(win->lists = malloc(sizeof(GooeyList) * MAX_WIDGETS)) ||
        !(win->canvas = malloc(sizeof(GooeyCanvas) * MAX_WIDGETS)) ||
        !(win->widgets = malloc(sizeof(GooeyWidget *) * MAX_WIDGETS)) ||
        !(win->plots = malloc(sizeof(GooeyPlot) * MAX_PLOT_COUNT)))
    {
        return false;
    }

    return true;
}

void GooeyWindow_FreeResources(GooeyWindow *win)
{
    for (size_t i = 0; i < win->canvas_count; ++i)
    {
        if (!win->canvas[i].elements)
            continue;

        for (int j = 0; j < win->canvas[i].element_count; ++j)
        {
            CanvaElement *element = &win->canvas[i].elements[j];
            if (element->args)
            {
                free(element->args);
                element->args = NULL;
            }
        }

        free(win->canvas[i].elements);
        win->canvas[i].elements = NULL;
    }

    if (win->drop_surface)
    {
        free(win->drop_surface);
        win->drop_surface = NULL;
    }

    if (win->images)
    {
        free(win->images);
        win->images = NULL;
    }

    if (win->current_event)
    {
        free(win->current_event);
        win->current_event = NULL;
    }

    if (win->active_theme)
    {
        free(win->active_theme);
        win->active_theme = NULL;
    }

    if (win->canvas)
    {
        free(win->canvas);
        win->canvas = NULL;
        win->canvas_count = 0;
    }

    if (win->buttons)
    {
        free(win->buttons);
        win->buttons = NULL;
    }
    if (win->labels)
    {
        free(win->labels);
        win->labels = NULL;
    }
    if (win->checkboxes)
    {
        free(win->checkboxes);
        win->checkboxes = NULL;
    }
    if (win->radio_buttons)
    {
        free(win->radio_buttons);
        win->radio_buttons = NULL;
    }
    if (win->radio_button_groups)
    {
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
        free(win->sliders);
        win->sliders = NULL;
    }
    if (win->dropdowns)
    {
        free(win->dropdowns);
        win->dropdowns = NULL;
    }
    if (win->textboxes)
    {
        free(win->textboxes);
        win->textboxes = NULL;
    }
    if (win->layouts)
    {
        free(win->layouts);
        win->layouts = NULL;
    }

    if (win->lists)
    {
        for (size_t i = 0; i < win->list_count; ++i)
        {
            if (win->lists[i].items)
            {
                free(win->lists[i].items);
                win->lists[i].items = NULL;
            }
        }
        free(win->lists);
        win->lists = NULL;
    }

    if (win->plots)
    {
        for (size_t i = 0; i < win->plot_count; ++i)
        {
            GooeyPlotData *data = win->plots->data;
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

GooeyWindow *GooeyWindow_Create(const char *title, int width, int height, bool visibilty)
{
    GooeyWindow *win = active_backend->CreateWindow(title, width, height);
    win->type = WINDOW_REGULAR;
    if (!GooeyWindow_AllocateResources(win))
    {
        GooeyWindow_Cleanup(1, win);
        LOG_CRITICAL("Failed to allocate memory for GooeyWindow.");
        exit(EXIT_FAILURE);
    }

    const unsigned long primaryColor = 0x2196F3;
    const unsigned long baseColor = 0xFFFFFF;
    const unsigned long neutralColor = 0x000000;
    const unsigned long widgetBaseColor = 0xD3D3D3;
    const unsigned long dangerColor = 0xE91E63;
    const unsigned long infoColor = 0x2196F3;
    const unsigned long successColor = 0x00A152;

    win->menu = NULL;

    *win->active_theme = (GooeyTheme){.base = baseColor, .neutral = neutralColor, .primary = primaryColor, .widget_base = widgetBaseColor, .danger = dangerColor, .info = infoColor, .success = successColor};
    win->visibility = visibilty;
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
    win->widget_count = 0;
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
        return (GooeyWindow){0}; // TODO: Need to handle this in all window-dependant functions.
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
    GooeyDropSurface_Draw(win);
    GooeyImage_Draw(win);
    GooeyList_Draw(win);
    GooeyLabel_Draw(win);
    GooeyCanvas_Draw(win);
    GooeyButton_Draw(win);
    GooeyTextbox_Draw(win);
    GooeyCheckbox_Draw(win);
    GooeyRadioButtonGroup_Draw(win);
    GooeyDropdown_Draw(win);
    GooeySlider_Draw(win);
    GooeyPlot_Draw(win);
    GooeyMenu_Draw(win);
    active_backend->Render(win);

    active_backend->ResetEvents(win);
}

void GooeyWindow_Redraw(size_t window_id, void *data)
{
    bool needs_redraw = false;

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

    int width, height;
    active_backend->GetWinDim(&width, &height, window_id);
    active_backend->SetViewport(window_id, width, height);
    active_backend->UpdateBackground(window);
    needs_redraw |= GooeySlider_HandleDrag(window, window->current_event);
    needs_redraw |= GooeyList_HandleThumbScroll(window, window->current_event);

    switch (window->current_event->type)
    {
    case GOOEY_EVENT_RESIZE:
        needs_redraw = true;
        break;

    case GOOEY_EVENT_REDRAWREQ:
        needs_redraw = true;
        break;

    case GOOEY_EVENT_KEY_PRESS:
        // Handle key press event
        GooeyTextbox_HandleKeyPress(window, window->current_event);
        needs_redraw = true;
        break;

    case GOOEY_EVENT_CLICK_PRESS:
        // Handle mouse click press event
        int mouse_click_x = window->current_event->click.x, mouse_click_y = window->current_event->click.y;
        needs_redraw |= GooeyList_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyButton_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyDropdown_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyCheckbox_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyRadioButtonGroup_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyTextbox_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyMenu_HandleClick(window, mouse_click_x, mouse_click_y);
        needs_redraw |= GooeyList_HandleThumbScroll(window, window->current_event);
        needs_redraw |= GooeyImage_HandleClick(window, mouse_click_x, mouse_click_y);
        break;

    case GOOEY_EVENT_CLICK_RELEASE:
        // Handle mouse click release event

        break;

    case GOOEY_EVENT_DROP:
        LOG_INFO("GOT DROP EVET %d %d", window->current_event->drop_data.drop_x, window->current_event->drop_data.drop_y);
        needs_redraw |= GooeyDropSurface_HandleFileDrop(window, window->current_event->drop_data.drop_x, window->current_event->drop_data.drop_y);
        break;

    case GOOEY_EVENT_WINDOW_CLOSE:
        active_backend->DestroyWindowFromId(window_id);
        windows[window_id] = NULL;
        break;

    case -1:
        break;

    default:
        // Handle any other unhandled events
        LOG_INFO("Unhandled event type: %d", window->current_event->type);
        break;
    }

    if (needs_redraw)
        GooeyWindow_DrawUIElements(window);
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
        }
        if (windows[i])
        {
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

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

#include "core/gooey_backend_internal.h"
#include "gooey_event_internal.h"

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

GooeyBackend *active_backend = NULL;
GooeyTheme *active_theme;
GooeyBackends ACTIVE_BACKEND = -1;

void GooeyWindow_RegisterWidget(GooeyWindow *win, GooeyWidget *widget)
{
    if (win && win->widgets)
        win->widgets[win->widget_count++] = widget;
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

void GooeyWindow_SetTheme(const char *fontPath)
{

    if (active_backend)
    {
        *active_theme = parser_load_theme_from_file(fontPath);
        active_backend->UpdateBackground();
    }
}

bool GooeyWindow_AllocateResources(GooeyWindow *win)
{
    if (!(win->buttons = malloc(sizeof(GooeyButton) * MAX_WIDGETS)) ||
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
        free(win->plots);
        win->plots = NULL;
    }

    if (win->widgets)
    {
        free(win->widgets);
        win->widgets = NULL;
    }
}

GooeyWindow GooeyWindow_Create(const char *title, int width, int height, bool visibilty)
{
    GooeyWindow win = active_backend->CreateWindow(title, width, height);
    win.type = WINDOW_REGULAR;
    if (!GooeyWindow_AllocateResources(&win))
    {
        GooeyWindow_Cleanup(1, &win);
        LOG_CRITICAL("Failed to allocate memory for GooeyWindow.");
        exit(EXIT_FAILURE);
    }

    win.menu = NULL;
    win.visibility = visibilty;
    win.canvas_count = 0;
    win.button_count = 0;
    win.label_count = 0;
    win.plot_count = 0;
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
    active_backend->Clear(win->creation_id);

    // Draw all UI components
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
    active_backend->Render(win->creation_id);
}

void GooeyWindow_Redraw(size_t window_id, void *data)
{

    if (!data || !active_backend)
    {
        LOG_CRITICAL("Invalid data or backend in Redraw callback");
        return;
    }

    GooeyWindow **windows = (GooeyWindow **)data;
    if (window_id >= active_backend->GetWindowCount() || !windows[window_id])
    {
        LOG_CRITICAL("Invalid window ID or window is NULL");
        return;
    }

    LOG_CRITICAL("Redraw callback");

    GooeyEvent *event = active_backend->HandleEvents();

    if (!event)
    {
        LOG_CRITICAL("Failed to handle events");
        return;
    }

    if (window_id == event->attached_window)
    {
        GooeySlider_HandleDrag(windows[window_id], event);

        switch (event->type)
        {
        case GOOEY_EVENT_WINDOW_CLOSE:
            // Handle window close event
            LOG_INFO("Window close event");
            active_backend->DestroyWindowFromId(window_id);
            return;
            break;

        case GOOEY_EVENT_KEY_PRESS:
            // Handle key press event
            GooeyTextbox_HandleKeyPress(windows[window_id], event);
            break;

        case GOOEY_EVENT_CLICK_PRESS:
            // Handle mouse click press event
            int mouse_click_x = event->click.x, mouse_click_y = event->click.y;
            GooeyList_HandleClick(windows[window_id], mouse_click_x, mouse_click_y);
            GooeyButton_HandleClick(windows[window_id], mouse_click_x, mouse_click_y);
            GooeyDropdown_HandleClick(windows[window_id], mouse_click_x, mouse_click_y);
            GooeyCheckbox_HandleClick(windows[window_id], mouse_click_x, mouse_click_y);
            GooeyRadioButtonGroup_HandleClick(windows[window_id], mouse_click_x, mouse_click_y);
            GooeyTextbox_HandleClick(windows[window_id], mouse_click_x, mouse_click_y);
            GooeyMenu_HandleClick(windows[window_id], mouse_click_x, mouse_click_y);
            break;

        case GOOEY_EVENT_CLICK_RELEASE:
            // Handle mouse click release event
            LOG_INFO("Click release event at pos: %d %d", event->mouse_move.x, event->mouse_move.y);
            break;

        case -1:
            // Handle custom or default event
            LOG_INFO("Default event");
            break;

        default:
            // Handle any other unhandled events
            LOG_INFO("Unhandled event type: %d", event->type);
            break;
        }
        GooeyWindow_DrawUIElements(windows[window_id]);
    }

    usleep(16667); // Simulate 60 FPS
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
    }

    active_backend->DestroyWindows();
    active_backend->Cleanup();

    LOG_INFO("Cleanup completed.");
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
    active_backend->RequestRedraw(win->creation_id);
}

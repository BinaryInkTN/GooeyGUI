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

#include "event/gooey_event_internal.h"
#include "logger/pico_logger_internal.h"
#include "signals/gooey_signals.h"
#include "widgets/gooey_appbar_internal.h"
#include "widgets/gooey_button_internal.h"
#include "widgets/gooey_canvas_internal.h"
#include "widgets/gooey_checkbox_internal.h"
#include "widgets/gooey_container_internal.h"
#include "widgets/gooey_debug_overlay_internal.h"
#include "widgets/gooey_drop_surface_internal.h"
#include "widgets/gooey_dropdown_internal.h"
#include "widgets/gooey_image_internal.h"
#include "widgets/gooey_label_internal.h"
#include "widgets/gooey_layout.h"
#include "widgets/gooey_layout_internal.h"
#include "widgets/gooey_list_internal.h"
#include "widgets/gooey_menu_internal.h"
#include "widgets/gooey_messagebox.h"
#include "widgets/gooey_meter_internal.h"
#include "widgets/gooey_plot_internal.h"
#include "widgets/gooey_progressbar_internal.h"
#include "widgets/gooey_radiobutton_internal.h"
#include "widgets/gooey_slider_internal.h"
#include "widgets/gooey_switch_internal.h"
#include "widgets/gooey_tabs_internal.h"
#include "widgets/gooey_textbox_internal.h"
#include "widgets/gooey_window_internal.h"
#include "virtual/gooey_keyboard_internal.h"
#include "widgets/gooey_webview_internal.h"
#include "backends/gooey_backend_internal.h"
#include "widgets/gooey_ctxmenu_internal.h"
#include <stdarg.h>
#include <string.h>

#ifndef WIN32
#include <sys/resource.h>
#endif

GooeyBackend *active_backend = NULL;

/* Theme Management Functions */

static GooeyTheme *__default_theme(void)
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

GooeyTheme *GooeyTheme_LoadFromString(const char *styling)
{
    bool is_loaded = false;
    GooeyTheme *theme = malloc(sizeof(GooeyTheme));
    if (!theme)
    {
        LOG_ERROR("Failed to allocate memory for theme");
        return NULL;
    }

    *theme = parser_load_theme_from_string(styling, &is_loaded);

    if (!is_loaded)
    {
        LOG_WARNING("Failed to load theme from string");
        free(theme);
        return NULL;
    }

    return theme;
}

static void __destroy_theme(GooeyTheme *theme)
{
    free(theme);
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
    const size_t widget_size = MAX_WIDGETS * sizeof(void *);
    const size_t plot_size = MAX_PLOT_COUNT * sizeof(void *);
    const size_t switch_size = MAX_SWITCHES * sizeof(void *);

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
        !(win->meters = calloc(MAX_WIDGETS, sizeof(GooeyMeter *))) ||
        !(win->containers = calloc(MAX_WIDGETS, sizeof(GooeyContainer *))) ||
        !(win->switches = calloc(MAX_SWITCHES, sizeof(GooeySwitch *))) ||
        !(win->vk = calloc(1, sizeof(GooeyVK))) ||
        !(win->ctx_menu = calloc(1, sizeof(GooeyCtxMenu))))
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

static void __free_widget_array(void **array, size_t count)
{
    if (!array)
        return;

    for (size_t i = 0; i < count; ++i)
    {
        free(array[i]);
    }
    free(array);
}

static void __free_canvas_elements(GooeyWindow *win)
{
    if (!win->canvas)
        return;

    for (size_t i = 0; i < win->canvas_count; ++i)
    {
        if (!win->canvas[i] || !win->canvas[i]->elements)
            continue;

        for (int j = 0; j < win->canvas[i]->element_count; ++j)
        {
            CanvaElement *element = &win->canvas[i]->elements[j];
            free(element->args);
        }
        free(win->canvas[i]->elements);
        free(win->canvas[i]);
    }
    free(win->canvas);
    win->canvas = NULL;
}

static void __free_containers(GooeyWindow *win)
{
    if (!win->containers)
        return;

    for (size_t i = 0; i < win->container_count; ++i)
    {
        GooeyContainers *container = win->containers[i];
        if (!container)
            continue;

        for (size_t j = 0; j < container->container_count; ++j)
        {
            GooeyContainer *cont = &container->container[j];
            free(cont->widgets);
        }
        free(container->container);
        free(container);
    }
    free(win->containers);
    win->containers = NULL;
}

static void __free_tabs(GooeyWindow *win)
{
    if (!win->tabs)
        return;

    for (size_t i = 0; i < win->tab_count; ++i)
    {
        GooeyTabs *tab_container = win->tabs[i];
        if (!tab_container)
            continue;

        for (size_t j = 0; j < tab_container->tab_count; ++j)
        {
            free(tab_container->tabs[j].widgets);
        }
        free(tab_container->tabs);
        free(tab_container);
    }
    free(win->tabs);
    win->tabs = NULL;
}

static void __free_plots(GooeyWindow *win)
{
    if (!win->plots)
        return;

    for (size_t i = 0; i < win->plot_count; ++i)
    {
        GooeyPlotData *data = win->plots[i]->data;
        
    }
    free(win->plots);
    win->plots = NULL;
}

static void __free_lists(GooeyWindow *win)
{
    if (!win->lists)
        return;

    for (size_t i = 0; i < win->list_count; ++i)
    {
        free(win->lists[i]->items);
    }
    free(win->lists);
    win->lists = NULL;
}

void GooeyWindow_FreeResources(GooeyWindow *win)
{
    if (!win)
        return;

    /* Free themes */
    if (win->active_theme && win->active_theme != win->default_theme)
    {
        __destroy_theme(win->active_theme);
    }
    if (win->default_theme)
    {
        __destroy_theme(win->default_theme);
    }

    /* Free individual resources */
    free(win->appbar);
    free(win->vk);
    free(win->current_event);
    free(win->menu);
    free(win->ctx_menu);

    /* Free widget arrays */
    __free_canvas_elements(win);
    __free_containers(win);
    __free_tabs(win);
    __free_plots(win);
    __free_lists(win);
    __free_widget_array((void **)win->drop_surface, win->drop_surface_count);
    __free_widget_array((void **)win->images, win->image_count);
    __free_widget_array((void **)win->progressbars, win->progressbar_count);
    __free_widget_array((void **)win->switches, win->switch_count);
    __free_widget_array((void **)win->buttons, win->button_count);
    __free_widget_array((void **)win->labels, win->label_count);
    __free_widget_array((void **)win->checkboxes, win->checkbox_count);
    __free_widget_array((void **)win->radio_buttons, win->radio_button_count);
    __free_widget_array((void **)win->radio_button_groups, win->radio_button_group_count);
    __free_widget_array((void **)win->sliders, win->slider_count);
    __free_widget_array((void **)win->dropdowns, win->dropdown_count);
    __free_widget_array((void **)win->textboxes, win->textboxes_count);
    __free_widget_array((void **)win->layouts, win->layout_count);
    __free_widget_array((void **)win->meters, win->meter_count);

    free(win->widgets);

    /* Reset counts */
    win->tab_count = 0;
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
    win->container_count = 0;
    win->widget_count = 0;
    win->switch_count = 0;
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

    /* Initialize window fields */
    win->menu = NULL;
    win->appbar = NULL;
    win->width = width;
    win->height = height;
    win->enable_debug_overlay = false;
    win->visibility = visibility;
    win->continuous_redraw = false;

    if (win->vk)
        win->vk->is_shown = false;

    /* Initialize all counts to zero */
    win->tab_count = 0;
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
    win->container_count = 0;
    win->widget_count = 0;
    win->switch_count = 0;

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

    /* Initialize counts to zero */
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

#define DRAW_WIDGET_IF_ENABLED(feature, draw_func) \
    do                                             \
    {                                              \
        if (feature)                               \
            draw_func(win);                        \
    } while (0)

void GooeyWindow_DrawUIElements(GooeyWindow *win)
{
    if (!win)
        return;

#if (!TFT_ESPI_ENABLED)
    active_backend->Clear(win);
#endif

#if (ENABLE_VIRTUAL_KEYBOARD)
    GooeyVK_Internal_Draw(win);
#endif

    if (win->vk && win->vk->is_shown)
    {
        active_backend->Render(win);
        return;
    }

    /* Draw layouts first */
    for (size_t i = 0; i < win->layout_count; ++i)
    {
#if (ENABLE_LAYOUT)
        GooeyLayout_Build(win->layouts[i]);
#endif
    }

    /* Draw all UI components */
    DRAW_WIDGET_IF_ENABLED(ENABLE_CANVAS, GooeyCanvas_Draw);

    DRAW_WIDGET_IF_ENABLED(ENABLE_CONTAINER, GooeyContainer_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_TABS, GooeyTabs_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_METER, GooeyMeter_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_PROGRESSBAR, GooeyProgressBar_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_DROP_SURFACE, GooeyDropSurface_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_IMAGE, GooeyImage_Draw);

    DRAW_WIDGET_IF_ENABLED(ENABLE_LIST, GooeyList_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_BUTTON, GooeyButton_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_SWITCH, GooeySwitch_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_TEXTBOX, GooeyTextbox_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_CHECKBOX, GooeyCheckbox_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_RADIOBUTTON, GooeyRadioButtonGroup_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_SLIDER, GooeySlider_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_PLOT, GooeyPlot_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_LABEL, GooeyLabel_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_MENU, GooeyMenu_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_DEBUG_OVERLAY, GooeyDebugOverlay_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_DROPDOWN, GooeyDropdown_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_APPBAR, GooeyAppbar_Internal_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_CTXMENU, GooeyCtxMenu_Internal_Draw);

    active_backend->Render(win);
}

/* Fixed macros for event handling */
#define HANDLE_EVENT_IF_ENABLED_BOOL(feature, handler, ...) \
    do                                                      \
    {                                                       \
        if (feature)                                        \
            needs_redraw |= handler(__VA_ARGS__);           \
    } while (0)

#define HANDLE_EVENT_IF_ENABLED_VOID(feature, handler, ...) \
    do                                                      \
    {                                                       \
        if (feature)                                        \
        {                                                   \
            handler(__VA_ARGS__);                           \
            needs_redraw = true;                            \
        }                                                   \
    } while (0)

#define HANDLE_HOVER_IF_ENABLED(feature, handler, ...) \
    do                                                 \
    {                                                  \
        if (feature)                                   \
            handler(__VA_ARGS__);                      \
    } while (0)

void GooeyWindow_Redraw(size_t window_id, void *data)
{
    bool needs_redraw = true;

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

    /* Handle continuous updates */
    HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_SLIDER, GooeySlider_HandleDrag, window, event);
    HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_LIST, GooeyList_HandleThumbScroll, window, event);

#if (!TFT_ESPI_ENABLED)
    /* Handle hover effects */
    HANDLE_HOVER_IF_ENABLED(ENABLE_BUTTON, GooeyButton_HandleHover, window, event->mouse_move.x, event->mouse_move.y);
    HANDLE_HOVER_IF_ENABLED(ENABLE_MENU, GooeyMenu_HandleHover, window);
    HANDLE_HOVER_IF_ENABLED(ENABLE_DROPDOWN, GooeyDropdown_HandleHover, window, event->mouse_move.x, event->mouse_move.y);
    HANDLE_HOVER_IF_ENABLED(ENABLE_TEXTBOX, GooeyTextbox_HandleHover, window, event->mouse_move.x, event->mouse_move.y);
#endif

    switch (event->type)
    {
    case GOOEY_EVENT_MOUSE_SCROLL:
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_LIST, GooeyList_HandleScroll, window, event);
        break;

    case GOOEY_EVENT_RESIZE:
    case GOOEY_EVENT_REDRAWREQ:
        needs_redraw = true;
        break;

    case GOOEY_EVENT_KEY_PRESS:
        HANDLE_EVENT_IF_ENABLED_VOID(ENABLE_TEXTBOX, GooeyTextbox_HandleKeyPress, window, event);
        break;

    case GOOEY_EVENT_CLICK_PRESS:
    {
        int mouse_click_x = event->click.x, mouse_click_y = event->click.y;

        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_BUTTON, GooeyButton_HandleClick, window, mouse_click_x, mouse_click_y);
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_SWITCH, GooeySwitch_HandleClick, window, mouse_click_x, mouse_click_y);
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_DROPDOWN, GooeyDropdown_HandleClick, window, mouse_click_x, mouse_click_y);
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_CHECKBOX, GooeyCheckbox_HandleClick, window, mouse_click_x, mouse_click_y);
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_RADIOBUTTON, GooeyRadioButtonGroup_HandleClick, window, mouse_click_x, mouse_click_y);
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_TEXTBOX, GooeyTextbox_HandleClick, window, mouse_click_x, mouse_click_y);
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_MENU, GooeyMenu_HandleClick, window, mouse_click_x, mouse_click_y);
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_LIST, GooeyList_HandleClick, window, mouse_click_x, mouse_click_y);
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_IMAGE, GooeyImage_HandleClick, window, mouse_click_x, mouse_click_y);
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_TABS, GooeyTabs_HandleClick, window, mouse_click_x, mouse_click_y);

#if (ENABLE_CTXMENU)
        GooeyCtxMenu_Internal_HandleClick(window, mouse_click_x, mouse_click_y);
#endif

#if (ENABLE_CANVAS)
        GooeyCanvas_HandleClick(window, mouse_click_x, mouse_click_y);
#endif
#if (ENABLE_VIRTUAL_KEYBOARD)
        GooeyVK_Internal_HandleClick(window, mouse_click_x, mouse_click_y);
#endif
        break;
    }

    case GOOEY_EVENT_VK_ENTER:
        LOG_CRITICAL("Clicked enter");
#if (ENABLE_TEXTBOX)
        GooeyTextbox_Internal_HandleVK(window);
#endif
        break;

    case GOOEY_EVENT_DROP:
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_DROP_SURFACE, GooeyDropSurface_HandleFileDrop,
                                     window, event->drop_data.drop_x, event->drop_data.drop_y);
        break;

    case GOOEY_EVENT_WINDOW_CLOSE:
        active_backend->DestroyWindowFromId(window_id);
        windows[window_id] = NULL;
        break;

    default:
        // LOG_INFO("Unhandled event type: %d", event->type);
        break;
    }

    if (needs_redraw)
    {
        GooeyWindow_DrawUIElements(window);
        active_backend->ResetEvents(window);
    }
}

void GooeyWindow_ToggleDecorations(GooeyWindow *win, bool enable)
{
    active_backend->WindowToggleDecorations(win, enable);
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
        windows[i] = va_arg(args, GooeyWindow *);
    }
    va_end(args);

    for (int i = 0; i < num_windows; ++i)
    {
        if (windows[i])
        {
            GooeyWindow_FreeResources(windows[i]);
            free(windows[i]);
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

#if (ENABLE_WEBVIEW)
    GooeyWebview_Internal_Draw(NULL);
#endif

    va_list args;
    GooeyWindow *windows[num_windows];

    va_start(args, first_win);
    LOG_INFO("Starting application");

    windows[0] = first_win;
    GooeyWindow_DrawUIElements(first_win);

    for (int i = 1; i < num_windows; ++i)
    {
        windows[i] = va_arg(args, GooeyWindow *);
        GooeyWindow_DrawUIElements(windows[i]);
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

void GooeyWindow_UnRegisterWidget(GooeyWindow *win, void *widget)
{
    if (!win || !widget)
    {
        LOG_CRITICAL("Window and/or widget NULL.");
        return;
    }

    GooeyWidget *core = (GooeyWidget *)widget;
    WIDGET_TYPE type = core->type;
    
    switch (type)
    {
    case WIDGET_LABEL:
    {
        for (int i = 0; i < win->label_count; i++)
        {
            if (win->labels[i] == (GooeyLabel *)widget)
            {
                // Shift remaining elements left
                for (int j = i; j < win->label_count - 1; j++)
                {
                    win->labels[j] = win->labels[j + 1];
                }
                win->label_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_SLIDER:
    {
        for (int i = 0; i < win->slider_count; i++)
        {
            if (win->sliders[i] == (GooeySlider *)widget)
            {
                for (int j = i; j < win->slider_count - 1; j++)
                {
                    win->sliders[j] = win->sliders[j + 1];
                }
                win->slider_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_SWITCH:
    {
        for (int i = 0; i < win->switch_count; i++)
        {
            if (win->switches[i] == (GooeySwitch *)widget)
            {
                for (int j = i; j < win->switch_count - 1; j++)
                {
                    win->switches[j] = win->switches[j + 1];
                }
                win->switch_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_RADIOBUTTON:
    {
        for (int i = 0; i < win->radio_button_count; i++)
        {
            if (win->radio_button_groups[i] == (GooeyRadioButtonGroup *)widget)
            {
                for (int j = i; j < win->radio_button_count - 1; j++)
                {
                    win->radio_button_groups[j] = win->radio_button_groups[j + 1];
                }
                win->radio_button_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_CHECKBOX:
    {
        for (int i = 0; i < win->checkbox_count; i++)
        {
            if (win->checkboxes[i] == (GooeyCheckbox *)widget)
            {
                for (int j = i; j < win->checkbox_count - 1; j++)
                {
                    win->checkboxes[j] = win->checkboxes[j + 1];
                }
                win->checkbox_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_BUTTON:
    {
        for (int i = 0; i < win->button_count; i++)
        {
            if (win->buttons[i] == (GooeyButton *)widget)
            {
                for (int j = i; j < win->button_count - 1; j++)
                {
                    win->buttons[j] = win->buttons[j + 1];
                }
                win->button_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_TEXTBOX:
    {
        for (int i = 0; i < win->textboxes_count; i++)
        {
            if (win->textboxes[i] == (GooeyTextbox *)widget)
            {
                for (int j = i; j < win->textboxes_count - 1; j++)
                {
                    win->textboxes[j] = win->textboxes[j + 1];
                }
                win->textboxes_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_WEBVIEW:
    {
        for (int i = 0; i < win->webview_count; i++)
        {
            if (win->webviews[i] == (GooeyWebview *)widget)
            {
                for (int j = i; j < win->webview_count - 1; j++)
                {
                    win->webviews[j] = win->webviews[j + 1];
                }
                win->webview_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_DROPDOWN:
    {
        for (int i = 0; i < win->dropdown_count; i++)
        {
            if (win->dropdowns[i] == (GooeyDropdown *)widget)
            {
                for (int j = i; j < win->dropdown_count - 1; j++)
                {
                    win->dropdowns[j] = win->dropdowns[j + 1];
                }
                win->dropdown_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_CANVAS:
    {
        for (int i = 0; i < win->canvas_count; i++)
        {
            if (win->canvas[i] == (GooeyCanvas *)widget)
            {
                for (int j = i; j < win->canvas_count - 1; j++)
                {
                    win->canvas[j] = win->canvas[j + 1];
                }
                win->canvas_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_LAYOUT:
    {
        for (int i = 0; i < win->layout_count; i++)
        {
            if (win->layouts[i] == (GooeyLayout *)widget)
            {
                for (int j = i; j < win->layout_count - 1; j++)
                {
                    win->layouts[j] = win->layouts[j + 1];
                }
                win->layout_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_PLOT:
    {
        for (int i = 0; i < win->plot_count; i++)
        {
            if (win->plots[i] == (GooeyPlot *)widget)
            {
                for (int j = i; j < win->plot_count - 1; j++)
                {
                    win->plots[j] = win->plots[j + 1];
                }
                win->plot_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_DROP_SURFACE:
    {
        for (int i = 0; i < win->drop_surface_count; i++)
        {
            if (win->drop_surface[i] == (GooeyDropSurface *)widget)
            {
                for (int j = i; j < win->drop_surface_count - 1; j++)
                {
                    win->drop_surface[j] = win->drop_surface[j + 1];
                }
                win->drop_surface_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_IMAGE:
    {
        for (int i = 0; i < win->image_count; i++)
        {
            if (win->images[i] == (GooeyImage *)widget)
            {
                for (int j = i; j < win->image_count - 1; j++)
                {
                    win->images[j] = win->images[j + 1];
                }
                win->image_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_LIST:
    {
        for (int i = 0; i < win->list_count; i++)
        {
            if (win->lists[i] == (GooeyList *)widget)
            {
                for (int j = i; j < win->list_count - 1; j++)
                {
                    win->lists[j] = win->lists[j + 1];
                }
                win->list_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_PROGRESSBAR:
    {
        for (int i = 0; i < win->progressbar_count; i++)
        {
            if (win->progressbars[i] == (GooeyProgressBar *)widget)
            {
                for (int j = i; j < win->progressbar_count - 1; j++)
                {
                    win->progressbars[j] = win->progressbars[j + 1];
                }
                win->progressbar_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_METER:
    {
        for (int i = 0; i < win->meter_count; i++)
        {
            if (win->meters[i] == (GooeyMeter *)widget)
            {
                for (int j = i; j < win->meter_count - 1; j++)
                {
                    win->meters[j] = win->meters[j + 1];
                }
                win->meter_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_CONTAINER:
    {
        for (int i = 0; i < win->container_count; i++)
        {
            if (win->containers[i] == (GooeyContainers *)widget)
            {
                for (int j = i; j < win->container_count - 1; j++)
                {
                    win->containers[j] = win->containers[j + 1];
                }
                win->container_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_TABS:
    {
        for (int i = 0; i < win->tab_count; i++)
        {
            if (win->tabs[i] == (GooeyTabs *)widget)
            {
                for (int j = i; j < win->tab_count - 1; j++)
                {
                    win->tabs[j] = win->tabs[j + 1];
                }
                win->tab_count--;
                break;
            }
        }
        break;
    }
    default:
        LOG_ERROR("Invalid widget type.");
        break;
    }

    active_backend->RequestRedraw(win);
}

void GooeyWindow_SetContinuousRedraw(GooeyWindow *win)
{
    // win->continuous_redraw = true;
    // removed for now
}

void GooeyWindow_EnableDebugOverlay(GooeyWindow *win, bool is_enabled)
{
    win->enable_debug_overlay = is_enabled;
}

void GooeyWindow_RequestCleanup(GooeyWindow *win)
{
    if (!win || !active_backend)
    {
        LOG_ERROR("Invalid window or backend");
        return;
    }

    active_backend->RequestClose(win->creation_id);
}
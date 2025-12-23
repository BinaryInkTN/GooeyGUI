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
#include "widgets/gooey_node_editor_internal.h"
#include "widgets/gooey_notifications_internal.h"
#include "widgets/gooey_notifications.h"
#include <stdarg.h>
#include <string.h>

#ifndef WIN32
#include <sys/resource.h>
#endif

GooeyBackend *active_backend = NULL;

static GooeyTheme *cached_default_theme = NULL;

static GooeyTheme *__default_theme(void)
{
    if (cached_default_theme)
    {
        return cached_default_theme;
    }

    GooeyTheme *theme = malloc(sizeof(GooeyTheme));
    if (!theme)
    {
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

    cached_default_theme = theme;
    return theme;
}

GooeyTheme *GooeyTheme_LoadFromFile(const char *theme_path)
{
    if (!theme_path)
    {
        return NULL;
    }

    bool is_loaded = false;
    GooeyTheme *theme = malloc(sizeof(GooeyTheme));
    if (!theme)
    {
        return NULL;
    }

    *theme = parser_load_theme_from_file(theme_path, &is_loaded);

    if (!is_loaded)
    {
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
        return NULL;
    }

    *theme = parser_load_theme_from_string(styling, &is_loaded);

    if (!is_loaded)
    {
        free(theme);
        return NULL;
    }

    return theme;
}

static void __destroy_theme(GooeyTheme *theme)
{
    if (theme == cached_default_theme)
    {
        cached_default_theme = NULL;
    }
    free(theme);
}

void GooeyTheme_Destroy(GooeyTheme *theme)
{
    __destroy_theme(theme);
}

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
    for (int i = (int)win->widget_count - 1; i >= 0; --i)
    {
        GooeyWidget *widget = win->widgets[i];
        if ((x >= widget->x) & (x <= widget->x + widget->width) &
            (y >= widget->y) & (y <= widget->y + widget->height))
        {
            switch (widget->type)
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
        return;
    }

    if (!theme)
    {
        if (win->default_theme)
        {
            win->active_theme = win->default_theme;
        }
        return;
    }

    if (win->active_theme != theme)
    {
        if (win->active_theme && win->active_theme != win->default_theme)
        {
            __destroy_theme(win->active_theme);
        }
        win->active_theme = theme;
        active_backend->RequestRedraw(win);
    }
}

bool GooeyWindow_AllocateResources(GooeyWindow *win)
{
    const size_t total_widget_ptrs = MAX_WIDGETS * 20;
    const size_t total_byte_size = (total_widget_ptrs * sizeof(void *)) +
                                   (MAX_PLOT_COUNT * sizeof(GooeyPlot *)) +
                                   (MAX_SWITCHES * sizeof(GooeySwitch *)) +
                                   sizeof(GooeyVK) + sizeof(GooeyEvent) + sizeof(GooeyCtxMenu) +
                                   sizeof(GooeyNotificationManager);

    void *memory_pool = malloc(total_byte_size);
    if (!memory_pool)
    {
        return false;
    }

    char *pool_ptr = (char *)memory_pool;

    win->tabs = (GooeyTabs **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyTabs *);
    win->drop_surface = (GooeyDropSurface **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyDropSurface *);
    win->images = (GooeyImage **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyImage *);
    win->buttons = (GooeyButton **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyButton *);
    win->labels = (GooeyLabel **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyLabel *);
    win->checkboxes = (GooeyCheckbox **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyCheckbox *);
    win->radio_buttons = (GooeyRadioButton **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyRadioButton *);
    win->radio_button_groups = (GooeyRadioButtonGroup **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyRadioButtonGroup *);
    win->sliders = (GooeySlider **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeySlider *);
    win->dropdowns = (GooeyDropdown **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyDropdown *);
    win->textboxes = (GooeyTextbox **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyTextbox *);
    win->layouts = (GooeyLayout **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyLayout *);
    win->lists = (GooeyList **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyList *);
    win->canvas = (GooeyCanvas **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyCanvas *);
    win->widgets = (GooeyWidget **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyWidget *);
    win->plots = (GooeyPlot **)pool_ptr;
    pool_ptr += MAX_PLOT_COUNT * sizeof(GooeyPlot *);
    win->progressbars = (GooeyProgressBar **)pool_ptr;
    pool_ptr += MAX_PLOT_COUNT * sizeof(GooeyProgressBar *);
    win->meters = (GooeyMeter **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyMeter *);
    win->containers = (GooeyContainers **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyContainers *);
    win->switches = (GooeySwitch **)pool_ptr;
    pool_ptr += MAX_SWITCHES * sizeof(GooeySwitch *);
    win->node_editors = (GooeyNodeEditor **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyNodeEditor *);
    win->webviews = (GooeyWebview **)pool_ptr;
    pool_ptr += MAX_WIDGETS * sizeof(GooeyWebview *);

    win->current_event = (GooeyEvent *)pool_ptr;
    pool_ptr += sizeof(GooeyEvent);
    win->vk = (GooeyVK *)pool_ptr;
    pool_ptr += sizeof(GooeyVK);
    win->ctx_menu = (GooeyCtxMenu *)pool_ptr;
    pool_ptr += sizeof(GooeyCtxMenu);
    win->notification_manager = (GooeyNotificationManager *)pool_ptr;
    pool_ptr += sizeof(GooeyNotificationManager);

    memset(memory_pool, 0, total_byte_size);

    // Initialize notification manager
    if (win->notification_manager)
    {
        win->notification_manager->notifications = calloc(MAX_NOTIFICATIONS, sizeof(GooeyNotification *));
        if (!win->notification_manager->notifications)
        {
            free(memory_pool);
            return false;
        }
        win->notification_manager->notification_count = 0;

        // Initialize notification system
        GooeyNotification_Init(win);
    }

    win->default_theme = __default_theme();
    if (!win->default_theme)
    {
        free(memory_pool);
        return false;
    }

    win->active_theme = win->default_theme;
    win->memory_pool = memory_pool;
    return true;
}

static void __free_widget_array(void **array, size_t count)
{
    if (!array)
        return;

    for (size_t i = 0; i < count; ++i)
    {
        if (array[i])
        {
            free(array[i]);
        }
    }
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
            if (element->args)
            {
                free(element->args);
            }
        }
        free(win->canvas[i]->elements);
        free(win->canvas[i]);
    }
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

        if (container->container)
        {
            for (size_t j = 0; j < container->container_count; ++j)
            {
                GooeyContainer *cont = &container->container[j];
                if (cont->widgets)
                {
                    free(cont->widgets);
                }
            }
            free(container->container);
        }
        free(container);
    }
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

        if (tab_container->tabs)
        {
            for (size_t j = 0; j < tab_container->tab_count; ++j)
            {
                if (tab_container->tabs[j].widgets)
                {
                    free(tab_container->tabs[j].widgets);
                }
            }
            free(tab_container->tabs);
        }
        free(tab_container);
    }
}

static void __free_plots(GooeyWindow *win)
{
    if (!win->plots)
        return;

    for (size_t i = 0; i < win->plot_count; ++i)
    {
        GooeyPlot *plot = win->plots[i];
        if (!plot)
            continue;
        free(plot);
    }
}

static void __free_lists(GooeyWindow *win)
{
    if (!win->lists)
        return;

    for (size_t i = 0; i < win->list_count; ++i)
    {
        GooeyList *list = win->lists[i];
        if (!list)
            continue;

        if (list->items)
        {
            free(list->items);
        }
        free(list);
    }
}

static void __free_dropdowns(GooeyWindow *win)
{
    if (!win->dropdowns)
        return;

    for (size_t i = 0; i < win->dropdown_count; ++i)
    {
        GooeyDropdown *dropdown = win->dropdowns[i];
        if (!dropdown)
            continue;

        free(dropdown);
    }
}

static void __free_textboxes(GooeyWindow *win)
{
    if (!win->textboxes)
        return;

    for (size_t i = 0; i < win->textboxes_count; ++i)
    {
        GooeyTextbox *textbox = win->textboxes[i];
        if (!textbox)
            continue;

        if (textbox->text)
        {
            free(textbox->text);
        }
        free(textbox);
    }
}

static void __free_node_editors(GooeyWindow *win)
{
    if (!win->node_editors)
        return;

    for (size_t i = 0; i < win->node_editor_count; ++i)
    {
        GooeyNodeEditor *editor = win->node_editors[i];
        if (!editor)
            continue;

        GooeyNodeEditor_Internal_Clear(editor);
        free(editor);
    }
}

static void __free_webviews(GooeyWindow *win)
{
    if (!win->webviews)
        return;

    for (size_t i = 0; i < win->webview_count; ++i)
    {
        GooeyWebview *webview = win->webviews[i];
        if (!webview)
            continue;

        free(webview);
    }
}

static void __free_notifications(GooeyWindow *win)
{
    if (!win || !win->notification_manager)
        return;

    // Use the proper cleanup function
    GooeyNotification_Cleanup(win);
}

void GooeyWindow_FreeResources(GooeyWindow *win)
{
    if (!win)
        return;

    if (win->active_theme && win->active_theme != win->default_theme)
    {
        __destroy_theme(win->active_theme);
        win->active_theme = NULL;
    }
    if (win->default_theme)
    {
        __destroy_theme(win->default_theme);
        win->default_theme = NULL;
    }

    if (win->appbar)
    {
        free(win->appbar);
        win->appbar = NULL;
    }
    if (win->menu)
    {
        free(win->menu);
        win->menu = NULL;
    }

    __free_canvas_elements(win);
    __free_containers(win);
    __free_tabs(win);
    __free_plots(win);
    __free_lists(win);
    __free_dropdowns(win);
    __free_textboxes(win);
    __free_node_editors(win);
    __free_webviews(win);
    __free_notifications(win);

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
    __free_widget_array((void **)win->meters, win->meter_count);
    __free_widget_array((void **)win->layouts, win->layout_count);

    if (win->memory_pool)
    {
        free(win->memory_pool);
        win->memory_pool = NULL;
    }

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
    win->node_editor_count = 0;
    win->webview_count = 0;
    win->notification_count = 0;
}

GooeyWindow *GooeyWindow_Create(const char *title, int x, int y, int width, int height, bool visibility)
{
    GooeyWindow *win = active_backend->CreateGooeyWindow(title, x, y, width, height);
    if (!win)
    {
        return NULL;
    }

    win->type = WINDOW_REGULAR;
    if (!GooeyWindow_AllocateResources(win))
    {
        GooeyWindow_FreeResources(win);
        exit(EXIT_FAILURE);
    }

    win->menu = NULL;
    win->appbar = NULL;
    win->width = width;
    win->height = height;
    win->enable_debug_overlay = false;
    win->visibility = visibility;
    win->continuous_redraw = false;

    if (win->vk)
        win->vk->is_shown = false;

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
    win->node_editor_count = 0;
    win->webview_count = 0;
    win->notification_count = 0;

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
    win.node_editor_count = 0;
    win.webview_count = 0;
    win.notification_count = 0;

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

    for (size_t i = 0; i < win->layout_count; ++i)
    {
#if (ENABLE_LAYOUT)
        GooeyLayout_Build(win->layouts[i]);
#endif
    }

    DRAW_WIDGET_IF_ENABLED(ENABLE_CANVAS, GooeyCanvas_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_CONTAINER, GooeyContainer_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_DROP_SURFACE, GooeyDropSurface_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_METER, GooeyMeter_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_PROGRESSBAR, GooeyProgressBar_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_PLOT, GooeyPlot_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_IMAGE, GooeyImage_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_LABEL, GooeyLabel_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_LIST, GooeyList_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_SLIDER, GooeySlider_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_CHECKBOX, GooeyCheckbox_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_RADIOBUTTON, GooeyRadioButtonGroup_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_SWITCH, GooeySwitch_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_TEXTBOX, GooeyTextbox_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_BUTTON, GooeyButton_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_TABS, GooeyTabs_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_APPBAR, GooeyAppbar_Internal_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_MENU, GooeyMenu_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_DROPDOWN, GooeyDropdown_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_CTXMENU, GooeyCtxMenu_Internal_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_DEBUG_OVERLAY, GooeyDebugOverlay_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_NODE_EDITOR, GooeyNodeEditor_Draw);
    DRAW_WIDGET_IF_ENABLED(ENABLE_NOTIFICATIONS, GooeyNotification_Internal_Draw);

    active_backend->Render(win);
}

#define HANDLE_EVENT_IF_ENABLED_BOOL(feature, handler, ...) \
    (feature ? (needs_redraw |= handler(__VA_ARGS__)) : 0)

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
        return;
    }

    GooeyWindow **windows = (GooeyWindow **)data;
    if (window_id > active_backend->GetTotalWindowCount() || !windows[window_id])
    {
        return;
    }

    GooeyWindow *window = windows[window_id];
    GooeyEvent *event = (GooeyEvent *)window->current_event;

    int width, height;
    active_backend->GetWinDim(&width, &height, window_id);
    active_backend->SetViewport(window_id, width, height);
    active_backend->UpdateBackground(window);

    HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_SLIDER, GooeySlider_HandleDrag, window, event);
    HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_LIST, GooeyList_HandleThumbScroll, window, event);

    HANDLE_EVENT_IF_ENABLED_VOID(ENABLE_NOTIFICATIONS, GooeyNotification_Internal_Update, window);

#if (!TFT_ESPI_ENABLED)
    HANDLE_HOVER_IF_ENABLED(ENABLE_BUTTON, GooeyButton_HandleHover, window, event->mouse_move.x, event->mouse_move.y);
    HANDLE_HOVER_IF_ENABLED(ENABLE_MENU, GooeyMenu_HandleHover, window);
    HANDLE_HOVER_IF_ENABLED(ENABLE_DROPDOWN, GooeyDropdown_HandleHover, window, event->mouse_move.x, event->mouse_move.y);
    HANDLE_HOVER_IF_ENABLED(ENABLE_TEXTBOX, GooeyTextbox_HandleHover, window, event->mouse_move.x, event->mouse_move.y);
    HANDLE_HOVER_IF_ENABLED(ENABLE_NODE_EDITOR, GooeyNodeEditor_HandleHover, window, event->mouse_move.x, event->mouse_move.y);
    HANDLE_HOVER_IF_ENABLED(ENABLE_NODE_EDITOR, GooeyNodeEditor_HandleDrag, window, event->mouse_move.x, event->mouse_move.y, event->mouse_move.x, event->mouse_move.y);
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

    case GOOEY_EVENT_CLICK_RELEASE:
        HANDLE_HOVER_IF_ENABLED(ENABLE_NODE_EDITOR, GooeyNodeEditor_HandleRelease, window, event->mouse_move.x, event->mouse_move.y);
        break;

    case GOOEY_EVENT_KEY_PRESS:
        HANDLE_EVENT_IF_ENABLED_VOID(ENABLE_TEXTBOX, GooeyTextbox_HandleKeyPress, window, event);
        break;

    case GOOEY_EVENT_CLICK_PRESS:
    {
        int mouse_click_x = event->click.x, mouse_click_y = event->click.y;

        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_NOTIFICATIONS, GooeyNotification_Internal_HandleClick, window, mouse_click_x, mouse_click_y);

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
        HANDLE_EVENT_IF_ENABLED_BOOL(ENABLE_NODE_EDITOR, GooeyNodeEditor_HandleClick, window, mouse_click_x, mouse_click_y);

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
        return;
    }

#if (ENABLE_WEBVIEW)
    GooeyWebview_Internal_Draw(NULL);
#endif

    va_list args;
    GooeyWindow *windows[num_windows];

    va_start(args, first_win);

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
    active_backend->RequestRedraw(win);
}

void GooeyWindow_UnRegisterWidget(GooeyWindow *win, void *widget)
{
    if (!win || !widget)
    {
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
    case WIDGET_NODE_EDITOR:
    {
        for (int i = 0; i < win->node_editor_count; i++)
        {
            if (win->node_editors[i] == (GooeyNodeEditor *)widget)
            {
                for (int j = i; j < win->node_editor_count - 1; j++)
                {
                    win->node_editors[j] = win->node_editors[j + 1];
                }
                win->node_editor_count--;
                break;
            }
        }
        break;
    }
    case WIDGET_NOTIFICATIONS:
    {
        break;
    }
    default:
        break;
    }

    active_backend->RequestRedraw(win);
}

void GooeyWindow_SetContinuousRedraw(GooeyWindow *win)
{
}

void GooeyWindow_EnableDebugOverlay(GooeyWindow *win, bool is_enabled)
{
    win->enable_debug_overlay = is_enabled;
}

void GooeyWindow_RequestCleanup(GooeyWindow *win)
{
    if (!win || !active_backend)
    {
        return;
    }

    active_backend->RequestClose(win->creation_id);
}

void GooeyWindow_MakeTransparent(GooeyWindow *win, int blur_radius, float opacity)
{
    active_backend->MakeWindowTransparent(win, blur_radius, opacity);
}

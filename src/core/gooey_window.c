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
#include <stdarg.h>
#include <string.h>

#ifndef WIN32
#include <sys/resource.h>
#endif

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

  *theme = (GooeyTheme){.base = 0xFFFFFF,
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
    LOG_WARNING("Failed to load theme from ");
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

void GooeyTheme_Destroy(GooeyTheme *theme) { __destroy_theme(theme); }

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

bool GooeyWindow_HandleCursorChange(GooeyWindow *win, GOOEY_CURSOR *cursor,
                                    int x, int y)
{
  for (size_t i = 0; i < win->widget_count; ++i)
  {
    if (x >= win->widgets[i]->x &&
        x <= win->widgets[i]->x + win->widgets[i]->width &&
        y >= win->widgets[i]->y &&
        y <= win->widgets[i]->y + win->widgets[i]->height)
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
      !(win->radio_button_groups =
            calloc(MAX_WIDGETS, sizeof(GooeyRadioButtonGroup *))) ||
      !(win->sliders = calloc(MAX_WIDGETS, sizeof(GooeySlider *))) ||
      !(win->dropdowns = calloc(MAX_WIDGETS, sizeof(GooeyDropdown *))) ||
      !(win->textboxes = calloc(MAX_WIDGETS, sizeof(GooeyTextbox *))) ||
      !(win->layouts = calloc(MAX_WIDGETS, sizeof(GooeyLayout *))) ||
      !(win->lists = calloc(MAX_WIDGETS, sizeof(GooeyList *))) ||
      !(win->canvas = calloc(MAX_WIDGETS, sizeof(GooeyCanvas *))) ||
      !(win->widgets = calloc(MAX_WIDGETS, sizeof(GooeyWidget *))) ||
      !(win->plots = calloc(MAX_PLOT_COUNT, sizeof(GooeyPlot *))) ||
      !(win->progressbars =
            calloc(MAX_PLOT_COUNT, sizeof(GooeyProgressBar *))) ||
      !(win->meters = calloc(MAX_WIDGETS, sizeof(GooeyMeter *))) ||
      !(win->containers = calloc(MAX_WIDGETS, sizeof(GooeyContainer *))) ||
      !(win->switches = calloc(MAX_SWITCHES, sizeof(GooeySwitch *))) ||
      !(win->vk = calloc(1, sizeof(GooeyVK))))
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
  if (win->active_theme && win->active_theme != win->default_theme)
  {
    __destroy_theme(win->active_theme);
    win->active_theme = NULL;
  }
  if (win->appbar)
  {
    free(win->appbar);
    win->appbar = NULL;
  }
  if (win->default_theme)
  {
    __destroy_theme(win->default_theme);
    win->default_theme = NULL;
  }
  if (win->vk)
  {
    free(win->vk);
    win->vk = NULL;
  }
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
if (win->containers)
{
  for (size_t container_index = 0; container_index < win->container_count;
       ++container_index)
  {
    GooeyContainers *container = win->containers[container_index];
    if (container && container->container)
    {
      for (size_t cont_index = 0; cont_index < container->container_count;
           ++cont_index)
      {
        GooeyContainer *cont = &container->container[cont_index];
        if (cont && cont->widgets)
        {
          free(cont->widgets);
          cont->widgets = NULL;
          cont->widget_count = 0;
        }
      }

      free(container->container);
      container->container = NULL;
      container->container_count = 0;
    }

    free(container);
    win->containers[container_index] = NULL;
  }

  free(win->containers);
  win->containers = NULL;
  win->container_count = 0;
}
  if (win->tabs)
  {
    for (size_t tab_container_index = 0; tab_container_index < win->tab_count;
         ++tab_container_index)
    {
      GooeyTabs *current_tab_container = win->tabs[tab_container_index];

      if (current_tab_container)
      {
        if (current_tab_container->tabs)
        {
          for (size_t tab_index = 0;
               tab_index < current_tab_container->tab_count; ++tab_index)
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
  if (win->switches)
  {
    for (size_t i = 0; i < win->switch_count; ++i)
    {
      if (win->switches[i])
      {
        free(win->switches[i]);
        win->switches[i] = NULL;
      }
    }
    free(win->switches);
    win->switches = NULL;
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

GooeyWindow *GooeyWindow_Create(const char *title, int width, int height,
                                bool visibility)
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

  if (win->vk)
    win->vk->is_shown = false;

  win->menu = NULL;
  win->appbar = NULL;
  win->width = width;
  win->height = height;
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
  win->container_count = 0;
  win->widget_count = 0;
  win->switch_count = 0;
  win->continuous_redraw = false;

  LOG_INFO("Window created with dimensions (%d, %d).", width, height);
  return win;
}

GooeyWindow GooeyWindow_CreateChild(const char *title, int width, int height,
                                    bool visibility)
{
  GooeyWindow win =
      active_backend->SpawnWindow(title, width, height, visibility);

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
#if (!TFT_ESPI_ENABLED)
  active_backend->Clear(win);
#endif
#if (ENABLE_VIRTUAL_KEYBOARD)
  GooeyVK_Internal_Draw(win);
#endif
#if (ENABLE_CONTAINER)
  GooeyContainer_Draw(win);
#endif
  if (win->vk->is_shown)
  {
    active_backend->Render(win);
    return;
  }
  // Draw all UI components
  for (size_t i = 0; i < win->layout_count; ++i)
  {
#if (ENABLE_LAYOUT)
    GooeyLayout_Build(win->layouts[i]);
#endif
  }
#if (ENABLE_TABS)
  GooeyTabs_Draw(win);
#endif
#if (ENABLE_CANVAS)
  GooeyCanvas_Draw(win);
#endif
#if (ENABLE_METER)
  GooeyMeter_Draw(win);
#endif
#if (ENABLE_PROGRESSBAR)
  GooeyProgressBar_Draw(win);
#endif
#if (ENABLE_DROP_SURFACE)
  GooeyDropSurface_Draw(win);
#endif
#if (ENABLE_IMAGE)  
GooeyImage_Draw(win);
#endif
#if (ENABLE_LIST)
  GooeyList_Draw(win);
#endif
#if (ENABLE_BUTTON)
  GooeyButton_Draw(win);
#endif
#if (ENABLE_SWITCH)
  GooeySwitch_Draw(win);
#endif
#if (ENABLE_TEXTBOX)
  GooeyTextbox_Draw(win);
#endif
#if (ENABLE_CHECKBOX)
  GooeyCheckbox_Draw(win);
#endif
#if (ENABLE_RADIOBUTTON)
  GooeyRadioButtonGroup_Draw(win);
#endif
#if (ENABLE_SLIDER)
  GooeySlider_Draw(win);
#endif
#if (ENABLE_PLOT)
  GooeyPlot_Draw(win);
#endif
#if (ENABLE_LABEL)
  GooeyLabel_Draw(win);
#endif
#if (ENABLE_MENU)
  GooeyMenu_Draw(win);
#endif
#if (ENABLE_DEBUG_OVERLAY)
  GooeyDebugOverlay_Draw(win);
#endif
#if (ENABLE_DROPDOWN)
  GooeyDropdown_Draw(win);
#endif
#if (ENABLE_APPBAR) 
  GooeyAppbar_Internal_Draw(win);
#endif

  active_backend->Render(win);
}

void GooeyWindow_Redraw(size_t window_id, void *data)
{
  bool needs_redraw = true; // Keep true for now, we need to work on pending
                            // events for this to work properly.

  if (!data || !active_backend)
  {
    LOG_CRITICAL("Invalid data or backend in Redraw callback");
    return;
  }

  GooeyWindow **windows = (GooeyWindow **)data;
  if (window_id > active_backend->GetTotalWindowCount() ||
      !windows[window_id])
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
#if (ENABLE_SLIDER)
  needs_redraw |= GooeySlider_HandleDrag(window, event);
#endif

#if (ENABLE_BUTTON && !TFT_ESPI_ENABLED)
  GooeyButton_HandleHover(window, event->mouse_move.x, event->mouse_move.y);
#endif
#if (ENABLE_MENU && !TFT_ESPI_ENABLED)
  GooeyMenu_HandleHover(window);
#endif
#if (ENABLE_DROPDOWN && !TFT_ESPI_ENABLED)
  GooeyDropdown_HandleHover(window, event->mouse_move.x, event->mouse_move.y);
#endif
#if (ENABLE_LIST)
  needs_redraw |= GooeyList_HandleThumbScroll(window, event);
#endif
#if (ENABLE_TEXTBOX && !TFT_ESPI_ENABLED)
  GooeyTextbox_HandleHover(window, event->mouse_move.x, event->mouse_move.y);
#endif
  switch (event->type)
  {

  case GOOEY_EVENT_MOUSE_SCROLL:
#if (ENABLE_LIST)
    needs_redraw |= GooeyList_HandleScroll(window, event);
#endif
    break;

  case GOOEY_EVENT_RESIZE:
    needs_redraw = true;
    break;

  case GOOEY_EVENT_REDRAWREQ:
    needs_redraw = true;
    break;

  case GOOEY_EVENT_KEY_PRESS:
#if (ENABLE_TEXTBOX)
    GooeyTextbox_HandleKeyPress(window, event);
#endif
    needs_redraw = true;
    break;

  case GOOEY_EVENT_CLICK_PRESS:
  {
    int mouse_click_x = event->click.x, mouse_click_y = event->click.y;

#if (ENABLE_BUTTON)
    needs_redraw |=
        GooeyButton_HandleClick(window, mouse_click_x, mouse_click_y);
#endif
#if (ENABLE_SWITCH)
    needs_redraw |=
        GooeySwitch_HandleClick(window, mouse_click_x, mouse_click_y);
#endif
#if (ENABLE_DROPDOWN)
    needs_redraw |=
        GooeyDropdown_HandleClick(window, mouse_click_x, mouse_click_y);
#endif
#if (ENABLE_CHECKBOX)
    needs_redraw |=
        GooeyCheckbox_HandleClick(window, mouse_click_x, mouse_click_y);
#endif
#if (ENABLE_RADIOBUTTON)
    needs_redraw |=
        GooeyRadioButtonGroup_HandleClick(window, mouse_click_x, mouse_click_y);
#endif
#if (ENABLE_TEXTBOX)
    needs_redraw |=
        GooeyTextbox_HandleClick(window, mouse_click_x, mouse_click_y);
#endif
#if (ENABLE_MENU)
    needs_redraw |= GooeyMenu_HandleClick(window, mouse_click_x, mouse_click_y);
#endif
#if (ENABLE_LIST)
    needs_redraw |= GooeyList_HandleThumbScroll(window, event);
    needs_redraw |= GooeyList_HandleClick(window, mouse_click_x, mouse_click_y);
#endif
#if (ENABLE_IMAGE)
    needs_redraw |=
        GooeyImage_HandleClick(window, mouse_click_x, mouse_click_y);
#endif
#if (ENABLE_TABS)
    needs_redraw |= GooeyTabs_HandleClick(window, mouse_click_x, mouse_click_y);
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
  {
    LOG_CRITICAL("Clicked enter");
#if (ENABLE_TEXTBOX)
    GooeyTextbox_Internal_HandleVK(window);
#endif
    break;
  }
  case GOOEY_EVENT_CLICK_RELEASE:

    break;

  case GOOEY_EVENT_DROP:
#if (ENABLE_DROP_SURFACE)
    needs_redraw |= GooeyDropSurface_HandleFileDrop(
        window, event->drop_data.drop_x, event->drop_data.drop_y);
#endif
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

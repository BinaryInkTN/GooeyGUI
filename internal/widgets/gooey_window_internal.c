#include "gooey_window_internal.h"
#include "common/gooey_common.h"
#include "logger/pico_logger_internal.h"

void GooeyWindow_Internal_RegisterWidget(GooeyWindow *win, void *widget)
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
        win->labels[win->label_count++] = (GooeyLabel *)widget;
        break;
    }
    case WIDGET_SLIDER:
    {
        win->sliders[win->slider_count++] = (GooeySlider *)widget;
        break;
    }
    case WIDGET_RADIOBUTTON:
    {
        win->radio_button_groups[win->radio_button_count++] = (GooeyRadioButtonGroup *)widget;
        break;
    }
    case WIDGET_CHECKBOX:
    {
        win->checkboxes[win->checkbox_count++] = (GooeyCheckbox *)widget;
        break;
    }
    case WIDGET_BUTTON:
    {
        win->buttons[win->button_count++] = (GooeyButton *)widget;
        break;
    }
    case WIDGET_TEXTBOX:
    {
        win->textboxes[win->textboxes_count++] = (GooeyTextbox *)widget;
        break;
    }
    case WIDGET_DROPDOWN:
    {
        win->dropdowns[win->dropdown_count++] = (GooeyDropdown *)widget;
        break;
    }
    case WIDGET_CANVAS:
    {
        win->canvas[win->canvas_count++] = (GooeyCanvas *)widget;
        break;
    }
    case WIDGET_LAYOUT:
    {
        win->layouts[win->layout_count++] = (GooeyLayout *)widget;
        break;
    }
    case WIDGET_PLOT:
    {
        win->plots[win->plot_count++] = (GooeyPlot *)widget;
        break;
    }
    case WIDGET_DROP_SURFACE:
    {
        win->drop_surface[win->drop_surface_count++] = (GooeyDropSurface *)widget;
        break;
    }
    case WIDGET_IMAGE:
    {
        win->images[win->image_count++] = (GooeyImage *)widget;
        break;
    }
    case WIDGET_LIST:
    {
        win->lists[win->list_count++] = (GooeyList *)widget;
        break;
    }
    case WIDGET_PROGRESSBAR:
    {
        win->progressbars[win->progressbar_count++] = (GooeyProgressBar *)widget;
        break;
    }
    case WIDGET_METER:
    {
        win->meters[win->meter_count++] = (GooeyMeter *)widget;
        break;
    }
    case WIDGET_TABS:
    {
        win->tabs[win->tab_count++] = (GooeyTabs *)widget;
        break;
    }

    default:
        LOG_ERROR("Invalid widget type.");
        break;
    }
}
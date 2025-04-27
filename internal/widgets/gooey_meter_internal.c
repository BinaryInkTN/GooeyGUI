#include "gooey_meter_internal.h"
#include "common/gooey_common.h"
#include "backends/gooey_backend_internal.h"

#define MIN_SIZE 80
#define ASPECT_RATIO 1.0f
#define PADDING_PERCENT 0.1f
#define INNER_CIRCLE_SCALE 0.85f
#define BORDER_WIDTH 1.0f
#define FONT_SCALE 0.27f
#define LABEL_SPACING_PERCENT 0.08f
#define VALUE_SPACING_PERCENT 0.15f
#define VALUE_TEXT_Y_OFFSET 0.25f

void GooeyMeter_RecalculateLayout(GooeyMeter *meter)
{
    meter->core.width = (meter->core.width < MIN_SIZE) ? MIN_SIZE : meter->core.width;
    meter->core.height = (meter->core.height < MIN_SIZE) ? MIN_SIZE : meter->core.height;

    if (meter->core.width > meter->core.height * ASPECT_RATIO)
    {
        meter->core.width = meter->core.height * ASPECT_RATIO;
    }
    else
    {
        meter->core.height = meter->core.width / ASPECT_RATIO;
    }
}

void GooeyMeter_Draw(GooeyWindow *win)
{
    if (!win)
        return;

    for (size_t i = 0; i < win->meter_count; ++i)
    {
        GooeyMeter *meter = win->meters[i];
        if (!meter || !meter->core.is_visible)
            continue;

        GooeyMeter_RecalculateLayout(meter);

        const int padding = meter->core.width * PADDING_PERCENT;
        const int label_spacing = meter->core.height * LABEL_SPACING_PERCENT;
        const int value_spacing = meter->core.height * VALUE_SPACING_PERCENT;

        const int card_width = meter->core.width + padding * 2;
        const int card_height = meter->core.height + padding * 2;
        const int arc_center_x = meter->core.x + padding + meter->core.width / 2;
        const int arc_center_y = meter->core.y + padding * 2 + meter->core.height / 2;


        active_backend->FillRectangle(
            meter->core.x,
            meter->core.y,
            card_width,
            card_height,
            win->active_theme->widget_base,
            win->creation_id, true, 10.0f);

       

        const char *label_text = meter->label;
        const int label_text_width = active_backend->GetTextWidth(
            label_text,
            strlen(label_text));
        const int label_text_x = meter->core.x + (card_width - label_text_width) / 2;
        const int label_text_y = meter->core.y + padding + meter->core.height / 2;

        active_backend->SetForeground(win->active_theme->base);
        active_backend->FillArc(
            arc_center_x,
            arc_center_y,
            meter->core.width,
            meter->core.height,
            0,
            180,
            win->creation_id);

        active_backend->SetForeground(win->active_theme->primary);
        active_backend->FillArc(
            arc_center_x,
            arc_center_y,
            meter->core.width,
            meter->core.height,
            0,
            180 * ((float) meter->value/100),
            win->creation_id);

        active_backend->SetForeground(win->active_theme->widget_base);
        active_backend->FillArc(
            arc_center_x,
            arc_center_y,
            meter->core.width * INNER_CIRCLE_SCALE,
            meter->core.height * INNER_CIRCLE_SCALE,
            0,
            180,
            win->creation_id);

        active_backend->DrawText(
            label_text_x,
            label_text_y,
            label_text,
            win->active_theme->neutral,
            FONT_SCALE,
            win->creation_id);
        char value_text[20];
        snprintf(value_text, sizeof(value_text), "%d%%", (int) meter->value);
        const int value_text_width = active_backend->GetTextWidth(
            value_text,
            strlen(value_text));
        const int value_text_x = arc_center_x - value_text_width / 2;
        const int value_text_y = arc_center_y + meter->core.height * VALUE_TEXT_Y_OFFSET;

        active_backend->DrawText(
            value_text_x,
            value_text_y,
            value_text,
            win->active_theme->neutral,
            FONT_SCALE,
            win->creation_id);
        
        active_backend->DrawImage(meter->texture_id, meter->core.x + meter->core.width - 10, meter->core.y + meter->core.height - 10, 28, 28, win->creation_id);

        }
}
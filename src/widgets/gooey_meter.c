#include "widgets/gooey_meter.h"
#if (ENABLE_METER)
#include "logger/pico_logger_internal.h"
#include "backends/gooey_backend_internal.h"

GooeyMeter *GooeyMeter_Create(int x, int y, int width, int height, long initial_value, const char *label, const char *icon_path)
{
    GooeyMeter *meter = (GooeyMeter *)calloc(1, sizeof(GooeyMeter));

    if (!meter)
    {
        LOG_ERROR("Couldn't allocate memory for meter widget.");
        return NULL;
    }

    *meter = (GooeyMeter){
        .core = {
            .type = WIDGET_METER,
            .sprite = active_backend->CreateSpriteForWidget(x - 20, y - 20, width + 40, height + 40),
            .is_visible = true,
            .x = x,
            .y = y,
            .width = width,
            .height = height,
        },
        .value = initial_value,
        .label = label,
        .texture_id = active_backend->LoadImage(icon_path)};

    return meter;
}

void GooeyMeter_Update(GooeyMeter *meter, long new_value)
{
    if (!meter || new_value < 0 || new_value > 100)
    {
        LOG_ERROR("Couldn't update meter value.");
        return;
    }

    meter->value = new_value;
}
#endif
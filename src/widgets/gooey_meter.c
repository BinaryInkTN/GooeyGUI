#include "widgets/gooey_meter.h"
#include "logger/pico_logger_internal.h"

GooeyMeter *GooeyMeter_Create(int x, int y, int width, int height, long initial_value, const char *label)
{
    GooeyMeter *meter = (GooeyMeter *)malloc(sizeof(GooeyMeter));

    if (!meter)
    {
        LOG_ERROR("Couldn't allocate memory for meter widget.");
        return NULL;
    }

    *meter = (GooeyMeter){
        .core = {
            .type = WIDGET_METER,
            .is_visible = true,
            .x = x,
            .y = y,
            .width = width,
            .height = height,
        },
        .value = initial_value,
        .label = label
    };

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
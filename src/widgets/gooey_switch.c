
#include "widgets/gooey_switch.h"
#if (ENABLE_SWITCH)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

#define SWITCH_WIDTH 68
#define SWITCH_HEIGHT 33
GooeySwitch *GooeySwitch_Create(int x, int y, bool IsToggled,
                                bool show_hints,
                                void (*callback)(bool value, void *user_data), void *user_data)
{

    GooeySwitch *gswitch = calloc(1, sizeof(GooeySwitch));

    *gswitch = (GooeySwitch){0};
    gswitch->core.type = WIDGET_SWITCH;
    gswitch->core.x = x;
    gswitch->core.y = y;
    gswitch->core.width = SWITCH_WIDTH;
    gswitch->core.height = SWITCH_HEIGHT;
    gswitch->core.is_visible = true;
    gswitch->core.disable_input = false;

    gswitch->is_toggled = IsToggled;
    gswitch->show_hints = show_hints;
    gswitch->callback = callback;
    gswitch->core.sprite = active_backend->CreateSpriteForWidget(x - 20, y - 20, SWITCH_WIDTH + 40, SWITCH_HEIGHT + 40);
    gswitch->user_data = user_data;
    return gswitch;
}

bool GooeySwitch_GetState(GooeySwitch *gswitch)
{
    if (!gswitch)
    {
        LOG_ERROR("Widget<Switch> cannot be NULL. \n");
        return -1;
    }

    return gswitch->is_toggled;
}

void GooeySwitch_Toggle(GooeySwitch *gswitch)
{
    if (!gswitch)
    {
        LOG_ERROR("Widget<switch> cannot be NULL. \n");
        return;
    }

    gswitch->is_toggled = !gswitch->is_toggled;
}
#endif
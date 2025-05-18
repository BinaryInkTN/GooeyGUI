#ifndef GOOEY_ANIMATIONS_H
#define GOOEY_ANIMATIONS_H

#include "core/gooey_widget_internal.h"
#include "core/gooey_timers_internal.h"
#include "logger/pico_logger_internal.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct
{
    GooeyWidget *widget;
    GooeyTimer *timer;
    int start;
    int end;
    int current_step;
    int total_steps;
    bool is_x_axis;
    bool is_active;
} AnimationData;


static void animation_cleanup(AnimationData *data)
{

    if (!data)
        return;

    if (data->timer)
    {
        GooeyTimer_Stop_Internal(data->timer);
        GooeyTimer_Destroy_Internal(data->timer);
        data->timer = NULL;
    }

    data->widget = NULL;
    data->is_active = false;
    free(data);
}


static void animation_callback(void *user_data)
{
    if (!user_data)
        return;

    AnimationData *data = (AnimationData *)user_data;

    if (!data->is_active || !data->widget || !data->timer)
    {
        animation_cleanup(data);
        return;
    }

    if (data->current_step >= data->total_steps)
    {
        int final_pos = data->end;
        if (data->is_x_axis)
        {
            GooeyWidget_MoveTo_Internal(data->widget, final_pos, -1);
        }
        else
        {
            GooeyWidget_MoveTo_Internal(data->widget, -1, final_pos);
        }
        animation_cleanup(data);
        return;
    }

    float progress = (float)data->current_step / (float)data->total_steps;
    int current_pos = data->start + (int)((data->end - data->start) * progress);

    if (data->is_x_axis)
    {
        GooeyWidget_MoveTo_Internal(data->widget, current_pos, -1);
    }
    else
    {
        GooeyWidget_MoveTo_Internal(data->widget, -1, current_pos);
    }

    data->current_step++;
}


static bool setup_animation(GooeyWidget *widget, int start, int end,
                            float speed, bool is_x_axis)
{
    if (!widget)
    {
        LOG_ERROR("Animation setup failed: null widget\n");
        return false;
    }

    if (start == end)
    {
        LOG_ERROR("Animation setup failed: start equals end position\n");
        return false;
    }

    if (speed <= 0.0f)
    {
        LOG_ERROR("Animation setup failed: invalid speed %f\n", speed);
        return false;
    }

    const uint32_t frame_delay = 20; 
    AnimationData *data = calloc(1, sizeof(AnimationData));
    if (!data)
    {
        LOG_ERROR("Animation setup failed: memory allocation error\n");
        return false;
    }

    data->timer = GooeyTimer_Create_Internal();
    if (!data->timer)
    {
        LOG_ERROR("Animation setup failed: timer creation error\n");
        free(data);
        return false;
    }

    *data = (AnimationData){
        .widget = widget,
        .timer = data->timer,
        .start = start,
        .end = end,
        .current_step = 0,
        .total_steps = (int)(frame_delay / speed),
        .is_x_axis = is_x_axis,
        .is_active = true};

    if (data->total_steps <= 0)
    {
        data->total_steps = 1;
    }

    if (is_x_axis)
    {
        GooeyWidget_MoveTo_Internal(widget, start, -1);
    }
    else
    {
        GooeyWidget_MoveTo_Internal(widget, -1, start);
    }

    GooeyTimer_SetCallback_Internal(frame_delay, data->timer, animation_callback, data);

    return true;
}


void GooeyAnimation_TranslateX(GooeyWidget *widget, int start, int end, float speed)
{
    if (!setup_animation(widget, start, end, speed, true))
    {
        LOG_ERROR("X-axis translation animation failed to initialize\n");
    }
}

void GooeyAnimation_TranslateY(GooeyWidget *widget, int start, int end, float speed)
{
    if (!setup_animation(widget, start, end, speed, false))
    {
        LOG_ERROR("Y-axis translation animation failed to initialize\n");
    }
}

#endif // GOOEY_ANIMATIONS_H
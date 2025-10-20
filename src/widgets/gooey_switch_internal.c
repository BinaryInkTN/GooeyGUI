#include "widgets/gooey_switch_internal.h"
#if (ENABLE_SWITCH)
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"
#include "core/gooey_timers_internal.h"
#include "animations/gooey_animations_internal.h"

#define SWITCH_ON_TEXT "ON"
#define SWITCH_OFF_TEXT "OFF"

#define GOOEY_SWITCH_DEFAULT_RADIUS 17.0f

static float ease_out_quad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

static float ease_in_out_quad(float t)
{
    return t < 0.5f ? 2.0f * t * t : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
}

static void switch_animation_callback(void *user_data)
{
    GooeySwitch *gswitch = (GooeySwitch *)user_data;
    if (!gswitch)
        return;

    if (!gswitch->is_animating)
        return;

    gswitch->animation_step++;

    if (gswitch->animation_step >= SWITCH_ANIMATION_STEPS)
    {
        gswitch->thumb_position = gswitch->target_position;
        gswitch->is_animating = false;

        if (gswitch->animation_timer)
        {
            GooeyTimer_Stop_Internal(gswitch->animation_timer);
        }
        return;
    }

    float progress = (float)gswitch->animation_step / (float)SWITCH_ANIMATION_STEPS;
    float eased_progress = ease_out_quad(progress);

    int start_pos = gswitch->start_position;
    int target_pos = gswitch->target_position;
    int position_distance = target_pos - start_pos;
    gswitch->thumb_position = start_pos + (int)(position_distance * eased_progress);

    if (gswitch->animation_timer && gswitch->is_animating)
    {
        GooeyTimer_SetCallback_Internal(SWITCH_ANIMATION_SPEED, gswitch->animation_timer, switch_animation_callback, gswitch);
    }
}

static void start_switch_animation(GooeySwitch *gswitch, bool toggled)
{
    if (!gswitch)
        return;

    const int thumb_padding = 20;
    const int track_x = gswitch->core.x;
    const int track_width = gswitch->core.width;

    gswitch->target_position = toggled
                                   ? (track_x + track_width - thumb_padding)
                                   : (track_x + thumb_padding);

    gswitch->start_position = gswitch->thumb_position;
    gswitch->animation_step = 0;
    gswitch->is_animating = true;

    if (!gswitch->animation_timer)
    {
        gswitch->animation_timer = GooeyTimer_Create_Internal();
        if (!gswitch->animation_timer)
        {

            gswitch->thumb_position = gswitch->target_position;
            gswitch->is_animating = false;
            return;
        }
    }

    GooeyTimer_SetCallback_Internal(SWITCH_ANIMATION_SPEED, gswitch->animation_timer, switch_animation_callback, gswitch);
}

bool GooeySwitch_HandleClick(GooeyWindow *win, int x, int y)
{
    bool clicked_any_gswitch = false;

    for (size_t i = 0; i < win->switch_count; ++i)
    {
        GooeySwitch *gswitch = win->switches[i];
        if (!gswitch || !gswitch->core.is_visible || gswitch->core.disable_input)
            continue;

        bool is_within_bounds = (x >= gswitch->core.x && x <= gswitch->core.x + gswitch->core.width) &&
                                (y >= gswitch->core.y && y <= gswitch->core.y + gswitch->core.height);

        if (is_within_bounds)
        {
            bool new_toggle_state = !gswitch->is_toggled;
            gswitch->is_toggled = new_toggle_state;
            clicked_any_gswitch = true;
            active_backend->RedrawSprite(gswitch->core.sprite);

            start_switch_animation(gswitch, new_toggle_state);

            if (gswitch->callback)
            {
                gswitch->callback(gswitch->is_toggled, gswitch->user_data);
            }
        }
    }

    return clicked_any_gswitch;
}

void GooeySwitch_Draw(GooeyWindow *win)
{
    const int thumb_scaling_factor = 5;
    const int thumb_padding = 20;

    for (size_t i = 0; i < win->switch_count; ++i)
    {
        GooeySwitch *gswitch = win->switches[i];
        if (!gswitch->core.is_visible)
            continue;

        const int track_x = gswitch->core.x;
        const int track_y = gswitch->core.y;
        const int track_width = gswitch->core.width;
        const int track_height = gswitch->core.height;

        const int thumb_diameter = track_height - 2 * thumb_scaling_factor;
        const int thumb_radius = thumb_diameter / 2;

        unsigned long target_track_color = gswitch->is_toggled ? win->active_theme->primary : win->active_theme->widget_base;
        unsigned long current_track_color = target_track_color;

        if (gswitch->is_animating)
        {
            float progress = (float)gswitch->animation_step / (float)SWITCH_ANIMATION_STEPS;
            float eased_progress = ease_out_quad(progress);

            unsigned long start_track_color = gswitch->is_toggled ? win->active_theme->widget_base : win->active_theme->primary;

            if (eased_progress > 0.5f)
            {
                current_track_color = target_track_color;
            }
            else
            {
                current_track_color = start_track_color;
            }
        }

        active_backend->FillRectangle(
            track_x,
            track_y,
            track_width,
            track_height,
            current_track_color,
            win->creation_id,
            true,
            GOOEY_SWITCH_DEFAULT_RADIUS, gswitch->core.sprite);

        const int thumb_y = (track_y + track_height) - track_height / 2;

        int thumb_x;
        if (gswitch->is_animating)
        {
            thumb_x = gswitch->thumb_position;
        }
        else
        {
            thumb_x = gswitch->is_toggled
                          ? (track_x + track_width - thumb_padding)
                          : (track_x + thumb_padding);

            gswitch->thumb_position = thumb_x;
        }

        active_backend->SetForeground(0xFFFFFF);
        active_backend->FillArc(
            thumb_x,
            thumb_y,
            thumb_diameter,
            thumb_diameter,
            0,
            360,
            win->creation_id, gswitch->core.sprite);

        active_backend->SetForeground(0xF0F0F0);
        active_backend->FillArc(
            thumb_x,
            thumb_y - 1,
            thumb_diameter - 4,
            thumb_diameter - 4,
            0,
            360,
            win->creation_id, gswitch->core.sprite);

        if (gswitch->is_animating)
        {
            float progress = (float)gswitch->animation_step / (float)SWITCH_ANIMATION_STEPS;

            float on_alpha = gswitch->is_toggled ? progress : (1.0f - progress);
            if (on_alpha > 0.1f)
            {
                active_backend->DrawText(
                    track_x + 8,
                    thumb_y + 5,
                    SWITCH_ON_TEXT,
                    win->active_theme->neutral,
                    0.26f * on_alpha,
                    win->creation_id, gswitch->core.sprite);
            }

            float off_alpha = gswitch->is_toggled ? (1.0f - progress) : progress;
            if (off_alpha > 0.1f)
            {
                active_backend->DrawText(
                    track_x + track_width - 32,
                    thumb_y + 4,
                    SWITCH_OFF_TEXT,
                    win->active_theme->neutral,
                    0.26f * off_alpha,
                    win->creation_id, gswitch->core.sprite);
            }
        }
        else
        {

            if (gswitch->is_toggled)
            {
                active_backend->DrawText(
                    track_x + 8,
                    thumb_y + 4,
                    SWITCH_ON_TEXT,
                    win->active_theme->neutral,
                    0.26f,
                    win->creation_id, gswitch->core.sprite);
            }
            else
            {
                active_backend->DrawText(
                    track_x + track_width - 32,
                    thumb_y + 4,
                    SWITCH_OFF_TEXT,
                    win->active_theme->neutral,
                    0.26f,
                    win->creation_id, gswitch->core.sprite);
            }
        }
    }
}

void GooeySwitch_Cleanup(GooeySwitch *gswitch)
{
    if (!gswitch)
        return;

    if (gswitch->animation_timer)
    {
        GooeyTimer_Stop_Internal(gswitch->animation_timer);
        GooeyTimer_Destroy_Internal(gswitch->animation_timer);
        gswitch->animation_timer = NULL;
    }

    gswitch->is_animating = false;
}

#endif
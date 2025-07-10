#include "widgets/gooey_switch_internal.h"
#if (ENABLE_SWITCH)
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"

#define SWITCH_ON_TEXT "ON"
#define SWITCH_OFF_TEXT "OFF"

#define GOOEY_SWITCH_DEFAULT_RADIUS 17.0f
bool GooeySwitch_HandleClick(GooeyWindow *win, int x, int y)
{
    bool clicked_any_gswitch = false;

    for (size_t i = 0; i < win->switch_count; ++i)
    {
        GooeySwitch *gswitch = win->switches[i];
        if (!gswitch || !gswitch->core.is_visible)
            continue;
        bool is_within_bounds = (x >= gswitch->core.x && x <= gswitch->core.x + gswitch->core.width) &&
                                (y >= gswitch->core.y && y <= gswitch->core.y + gswitch->core.height);

        if (is_within_bounds)
        {

            gswitch->is_toggled = !gswitch->is_toggled;
            clicked_any_gswitch = true;
            active_backend->RedrawSprite(gswitch->core.sprite);

            if (gswitch->callback)
            {
                gswitch->callback(gswitch->is_toggled);
            }
        }
        else
        {
            clicked_any_gswitch = false;
        }
    }

    return clicked_any_gswitch;
}

/**
 * @brief Draws all checkboxes within the specified window.
 *
 * This function renders all checkboxes that belong to the given window,
 * ensuring they are displayed with their correct state (checked/unchecked).
 *
 * @param win The window containing the checkboxes.
 */
void GooeySwitch_Draw(GooeyWindow *win)
{
    const int thumb_scaling_factor = 5; // consistent horizontal padding
    const int thumb_padding = 20;
    for (size_t i = 0; i < win->switch_count; ++i)
    {
        GooeySwitch *gswitch = win->switches[i];
        if (!gswitch->core.is_visible)
            continue;

        // Track dimensions
        const int track_x = gswitch->core.x;
        const int track_y = gswitch->core.y;
        const int track_width = gswitch->core.width;
        const int track_height = gswitch->core.height;

        // Thumb dimensions
        const int thumb_diameter = track_height - 2 * thumb_scaling_factor; // slightly smaller than track
        const int thumb_radius = thumb_diameter / 2;

        // Draw the track
        unsigned long gswitch_color = gswitch->is_toggled ? win->active_theme->primary : win->active_theme->widget_base;
        active_backend->FillRectangle(
            track_x,
            track_y,
            track_width,
            track_height,
            gswitch_color,
            win->creation_id,
            true,
            GOOEY_SWITCH_DEFAULT_RADIUS, gswitch->core.sprite);

        const int thumb_y = (track_y + track_height) - track_height / 2;

        int thumb_x = gswitch->is_toggled
                          ? (track_x + track_width - thumb_padding)
                          : (track_x + thumb_padding);
        active_backend->SetForeground(0xFFFFFF);

        active_backend->FillArc(
            thumb_x,
            thumb_y,
            thumb_diameter,
            thumb_diameter,
            0,
            360,
            win->creation_id, gswitch->core.sprite);
    }
}

#endif
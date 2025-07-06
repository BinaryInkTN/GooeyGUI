#include "widgets/gooey_switch_internal.h"
#if(ENABLE_SWITCH)
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"

#define SWITCH_ON_TEXT  "ON"
#define SWITCH_OFF_TEXT  "OFF"

#define GOOEY_SWITCH_DEFAULT_RADIUS 5.0f
bool GooeyCheckbox_HandleClick(GooeyWindow *win, int x, int y){ 
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

            if (gswitch->callback)
            {
                gswitch->callback(gswitch->is_toggled);
            }
        }else { 
            clicked_any_gswitch = false ; 
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
void GooeyCheckbox_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->switch_count; ++i)
    {
        GooeySwitch *gswitch = win->switches[i];
        if (!gswitch->core.is_visible)
            continue;
        const int track_height = gswitch->core.height;
        const int thumb_diameter = track_height; // Make thumb height match track
        const int thumb_radius = thumb_diameter / 2;
        const int padding = 2;
        // int thumb_x = gswitch->is_on ? (gswitch->core.x + gswitch->core.width - thumb_diameter - padding) : (gswitch->core.x + padding);

        // Draw the thumb
        // active_backend->FillArc(
        //     thumb_x,
        //     gswitch->core.y + (track_height - thumb_diameter) / 2, // Center vertically
        //     thumb_diameter,
        //     thumb_diameter,
        //     0,
        //     360 * 64, // Full circle
        //     win->creation_id);
        unsigned long gswitch_color = win->active_theme->primary;

        if (gswitch->is_toggled == false)
        {
            int thumb_x = (gswitch->core.x + padding);
            active_backend->FillRectangle(gswitch->core.x,
                                          gswitch->core.y, gswitch->core.width, gswitch->core.height, gswitch_color, win->creation_id, true, GOOEY_SWITCH_DEFAULT_RADIUS);
            active_backend->FillArc(
                thumb_x,
                gswitch->core.y + (track_height - thumb_diameter) / 2, // Center vertically
                thumb_diameter,
                thumb_diameter,
                0,
                360 * 64, // Full circle
                win->creation_id);
        }
        else
        {
            int thumb_x = (gswitch->core.x + gswitch->core.width - thumb_diameter - padding);
            gswitch_color = win->active_theme->primary;
            active_backend->FillRectangle(gswitch->core.x,
                                          gswitch->core.y, gswitch->core.width, gswitch->core.height, gswitch_color, win->creation_id, true, GOOEY_SWITCH_DEFAULT_RADIUS);
            active_backend->FillArc(
                thumb_x,
                gswitch->core.y + (track_height - thumb_diameter) / 2, // Center vertically
                thumb_diameter,
                thumb_diameter,
                0,
                360 * 64, // Full circle
                win->creation_id);
        }
        // float text_width = active_backend->GetTextWidth(gswitch->label, strlen(gswitch->label));
        // float text_height = active_backend->GetTextHeight(gswitch->label, strlen(gswitch->label));

        // float text_x = gswitch->core.x + (gswitch->core.width - text_width) / 2;
        // float text_y = gswitch->core.y + (gswitch->core.height + text_height) / 2;

        // active_backend->DrawText(text_x,
        //                          text_y, gswitch->label, win->active_theme->neutral, 0.27f, win->creation_id);
        // active_backend->SetForeground(win->active_theme->neutral);

        // if (gswitch->is_highlighted)
        // {

        //     active_backend->DrawRectangle(gswitch->core.x,
        //                                   gswitch->core.y, gswitch->core.width, gswitch->core.height, win->active_theme->primary, 1.0f, win->creation_id, true, GOOEY_gswitch_DEFAULT_RADIUS);
        // }
    }
}
#endif
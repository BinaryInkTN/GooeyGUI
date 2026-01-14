#include "widgets/gooey_dropdown_internal.h"
#if (ENABLE_DROPDOWN)
#include "backends/gooey_backend_internal.h"
#include "core/gooey_timers_internal.h"
#include "animations/gooey_animations_internal.h"

struct user_data {
    GooeyWindow *window;
    GooeyDropdown *dropdown;
};
static float ease_out_quad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

static float ease_in_out_quad(float t)
{
    return t < 0.5f ? 2.0f * t * t : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
}

static void dropdown_animation_callback(void *user_data)
{
    struct user_data *data = (struct user_data *)user_data;
    GooeyWindow *win = data->window;

    GooeyDropdown *dropdown = data->dropdown;
    if (!dropdown || !dropdown->is_animating)
        return;

    dropdown->animation_step++;
    active_backend->RequestRedraw(win);
    if (dropdown->animation_step >= DROPDOWN_ANIMATION_STEPS)
    {
        // Animation complete
        dropdown->animation_height = dropdown->target_height;
        dropdown->is_animating = false;

        // Stop the timer
        if (dropdown->animation_timer)
        {
            GooeyTimer_Stop_Internal(dropdown->animation_timer);
        }

        if(data)
        {
            free(data);
            data = NULL;
        }
        return;
    }

    float progress = (float)dropdown->animation_step / (float)DROPDOWN_ANIMATION_STEPS;
    float eased_progress = ease_out_quad(progress);

    int start_height = dropdown->start_height;
    int target_height = dropdown->target_height;
    int height_distance = target_height - start_height;
    dropdown->animation_height = start_height + (int)(height_distance * eased_progress);

}

static void start_dropdown_animation(GooeyWindow *win, GooeyDropdown *dropdown, bool open)
{
    if (!dropdown)
        return;

    // Stop any existing animation
    if (dropdown->animation_timer && dropdown->is_animating)
    {
        GooeyTimer_Stop_Internal(dropdown->animation_timer);
    }

    dropdown->target_height = open ? (25 * dropdown->num_options) : 0;
    dropdown->start_height = dropdown->animation_height;
    dropdown->animation_step = 0;
    dropdown->is_animating = true;

    if (!dropdown->animation_timer)
    {
        dropdown->animation_timer = GooeyTimer_Create_Internal();
        if (!dropdown->animation_timer)
        {
            // Fallback: set position immediately
            dropdown->animation_height = dropdown->target_height;
            dropdown->is_animating = false;
            return;
        }
    }

    struct user_data* data = malloc(sizeof(struct user_data));
    if (!data)
        return;

    data->window = win;
    data->dropdown = dropdown;

    GooeyTimer_SetCallback_Internal(DROPDOWN_ANIMATION_SPEED, dropdown->animation_timer, dropdown_animation_callback, data);
}

void GooeyDropdown_Draw(GooeyWindow *win)
{
    if (!win || !win->dropdowns)
        return;

    for (size_t i = 0; i < win->dropdown_count; i++)
    {
        GooeyDropdown *dropdown = win->dropdowns[i];
        if (!dropdown || !dropdown->core.is_visible)
            continue;

        // Draw dropdown background
        active_backend->FillRectangle(dropdown->core.x, dropdown->core.y,
                                      dropdown->core.width, dropdown->core.height,
                                      win->active_theme->widget_base, win->creation_id, true, 2.0f, dropdown->core.sprite);

        // Draw selected text
        const char *selected_text = dropdown->options[dropdown->selected_index];
        active_backend->DrawGooeyText(dropdown->core.x + 5, dropdown->core.y + 20,
                                 selected_text, win->active_theme->neutral,
                                 18.0f, win->creation_id, dropdown->core.sprite);

        // Draw arrow with animation
        int arrow_x = dropdown->core.x + dropdown->core.width - 15;
        int arrow_y = dropdown->core.y + dropdown->core.height / 2;

        float arrow_progress = 0.0f;
        if (dropdown->is_animating)
        {
            arrow_progress = (float)dropdown->animation_step / (float)DROPDOWN_ANIMATION_STEPS;
            if (!dropdown->is_open)
                arrow_progress = 1.0f - arrow_progress;
        }
        else
        {
            arrow_progress = dropdown->is_open ? 1.0f : 0.0f;
        }

        int arrow_offset = (int)(5.0f * arrow_progress);

        if (arrow_progress > 0.5f)
        {
            // Arrow pointing up
            active_backend->DrawLine(arrow_x - 3, arrow_y + 2 - arrow_offset, arrow_x, arrow_y - 3 + arrow_offset,
                                     win->active_theme->neutral, win->creation_id, dropdown->core.sprite);
            active_backend->DrawLine(arrow_x, arrow_y - 3 + arrow_offset, arrow_x + 3, arrow_y + 2 - arrow_offset,
                                     win->active_theme->neutral, win->creation_id, dropdown->core.sprite);
        }
        else
        {
            // Arrow pointing down
            active_backend->DrawLine(arrow_x - 3, arrow_y - 2 + arrow_offset, arrow_x, arrow_y + 3 - arrow_offset,
                                     win->active_theme->neutral, win->creation_id, dropdown->core.sprite);
            active_backend->DrawLine(arrow_x, arrow_y + 3 - arrow_offset, arrow_x + 3, arrow_y - 2 + arrow_offset,
                                     win->active_theme->neutral, win->creation_id, dropdown->core.sprite);
        }

        // Draw dropdown options if open or animating
        if (dropdown->is_animating || dropdown->is_open)
        {
            const int submenu_x = dropdown->core.x;
            const int submenu_y = dropdown->core.y + dropdown->core.height;
            const int submenu_width = dropdown->core.width;
            const int current_height = dropdown->is_animating ? dropdown->animation_height : (dropdown->is_open ? (25 * dropdown->num_options) : 0);

            if (current_height > 0)
            {
                // Draw options background
                active_backend->FillRectangle(submenu_x, submenu_y,
                                              submenu_width, current_height,
                                              win->active_theme->widget_base, win->creation_id, true, 2.0f, dropdown->core.sprite);
                active_backend->DrawRectangle(submenu_x, submenu_y,
                                              submenu_width, current_height,
                                              win->active_theme->primary, 0.5f, win->creation_id, true, 2.0f, dropdown->core.sprite);

                int visible_options = current_height / 25;
                if (visible_options > dropdown->num_options)
                    visible_options = dropdown->num_options;

                for (int j = 0; j < visible_options; j++)
                {
                    const int element_y = submenu_y + (j * 25);
                    const bool is_hovered = (dropdown->element_hovered_over == j);
                    const bool is_selected = (j == dropdown->selected_index);

                    // Highlight hovered or selected option
                    if (is_hovered || is_selected)
                    {
                        active_backend->FillRectangle(submenu_x, element_y,
                                                      submenu_width, 25,
                                                      win->active_theme->primary, win->creation_id, false, 0.0f, dropdown->core.sprite);
                    }

                    // Draw option text
                    active_backend->DrawGooeyText(submenu_x + 5, element_y + 18,
                                             dropdown->options[j],
                                             (is_hovered || is_selected) ? win->active_theme->base : win->active_theme->neutral,
                                             18.0f, win->creation_id, dropdown->core.sprite);

                    // Draw separator (except for last option)
                    if (j < visible_options - 1 && j < dropdown->num_options - 1)
                    {
                        active_backend->DrawLine(submenu_x, element_y + 24,
                                                 submenu_x + submenu_width, element_y + 24,
                                                 win->active_theme->neutral, win->creation_id, dropdown->core.sprite);
                    }
                }
            }
        }

        // Reset sprite redraw flag if needed
        if (dropdown->core.sprite && dropdown->core.sprite->needs_redraw)
            active_backend->ResetRedrawSprite(dropdown->core.sprite);
    }
}

bool GooeyDropdown_HandleHover(GooeyWindow *win, int x, int y)
{
    if (!win || !win->dropdowns)
        return false;

    bool hover_handled = false;

    for (size_t i = 0; i < win->dropdown_count; i++)
    {
        GooeyDropdown *dropdown = win->dropdowns[i];
        if (!dropdown || !dropdown->core.is_visible || dropdown->core.disable_input)
            continue;

        // Check if hovering over main dropdown
        if (x >= dropdown->core.x && x <= dropdown->core.x + dropdown->core.width &&
            y >= dropdown->core.y && y <= dropdown->core.y + dropdown->core.height)
        {
            hover_handled = true;
            continue;
        }

        // Check if hovering over dropdown options
        if (dropdown->is_open || dropdown->is_animating)
        {
            const int submenu_x = dropdown->core.x;
            const int submenu_y = dropdown->core.y + dropdown->core.height;
            const int submenu_width = dropdown->core.width;
            const int current_height = dropdown->is_animating ? dropdown->animation_height : (dropdown->is_open ? (25 * dropdown->num_options) : 0);

            dropdown->element_hovered_over = -1;

            if (x >= submenu_x && x <= submenu_x + submenu_width &&
                y >= submenu_y && y <= submenu_y + current_height)
            {
                int option_index = (y - submenu_y) / 25;
                if (option_index < dropdown->num_options)
                {
                    dropdown->element_hovered_over = option_index;
                    hover_handled = true;
                }
            }
        }
        else
        {
            dropdown->element_hovered_over = -1;
        }
    }

    return hover_handled;
}

bool GooeyDropdown_HandleClick(GooeyWindow *win, int x, int y)
{
    if (!win || !win->dropdowns)
        return false;

    bool click_handled = false;

    for (size_t i = 0; i < win->dropdown_count; i++)
    {
        GooeyDropdown *dropdown = win->dropdowns[i];
        if (!dropdown || !dropdown->core.is_visible || dropdown->core.disable_input)
            continue;

        // Check click on main dropdown
        if (x >= dropdown->core.x && x <= dropdown->core.x + dropdown->core.width &&
            y >= dropdown->core.y && y <= dropdown->core.y + dropdown->core.height)
        {
            active_backend->RedrawSprite(dropdown->core.sprite);

            bool new_open_state = !dropdown->is_open;
            dropdown->is_open = new_open_state;
            start_dropdown_animation(win, dropdown, new_open_state);

            click_handled = true;
            continue;
        }

        // Check click on dropdown options
        if (dropdown->is_open || dropdown->is_animating)
        {
            const int submenu_x = dropdown->core.x;
            const int submenu_y = dropdown->core.y + dropdown->core.height;
            const int submenu_width = dropdown->core.width;
            const int current_height = dropdown->is_animating ? dropdown->animation_height : (dropdown->is_open ? (25 * dropdown->num_options) : 0);

            if (x >= submenu_x && x <= submenu_x + submenu_width &&
                y >= submenu_y && y <= submenu_y + current_height)
            {
                // Click on an option
                int option_index = (y - submenu_y) / 25;
                if (option_index < dropdown->num_options)
                {
                    active_backend->RedrawSprite(dropdown->core.sprite);

                    dropdown->selected_index = option_index;
                    if (dropdown->callback)
                    {
                        dropdown->callback(option_index, dropdown->user_data);
                    }

                    dropdown->is_open = false;
                    start_dropdown_animation(win, dropdown, false);

                    click_handled = true;
                }
            }
            else
            {
                // Click outside open dropdown - close it
                if (dropdown->is_open && !dropdown->is_animating)
                {
                    active_backend->RedrawSprite(dropdown->core.sprite);
                    dropdown->is_open = false;
                    start_dropdown_animation(win, dropdown, false);
                    click_handled = true;
                }
            }
        }
    }

    return click_handled;
}

void GooeyDropdown_Cleanup(GooeyDropdown *dropdown)
{
    if (!dropdown)
        return;

    if (dropdown->animation_timer)
    {
        GooeyTimer_Stop_Internal(dropdown->animation_timer);
        GooeyTimer_Destroy_Internal(dropdown->animation_timer);
        dropdown->animation_timer = NULL;
    }

    // Reset all animation state
    dropdown->is_animating = false;
    dropdown->animation_step = 0;
    dropdown->animation_height = 0;
}
#endif

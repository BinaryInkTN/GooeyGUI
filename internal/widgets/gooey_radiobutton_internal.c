#include "gooey_radiobutton_internal.h"
#if(ENABLE_RADIOBUTTON)
#include "backends/gooey_backend_internal.h"
#define RADIO_BUTTON_RADIUS 10 /** Radius of the radio button widget. */
void GooeyRadioButtonGroup_Draw(GooeyWindow *win)
{

    for (size_t i = 0; i < win->radio_button_group_count; ++i)
    {
        GooeyRadioButtonGroup *group = win->radio_button_groups[i];
        for (int j = 0; j < group->button_count; ++j)
        {
            GooeyRadioButton *button = &group->buttons[j];
            if (!button->core.is_visible)
                continue;
            int button_center_x = button->core.x + RADIO_BUTTON_RADIUS;
            int button_center_y = button->core.y + RADIO_BUTTON_RADIUS;

            int label_width = active_backend->GetTextWidth(button->label, strlen(button->label));

            int label_x = button->core.x + RADIO_BUTTON_RADIUS * 2;
            int label_y = button->core.y + RADIO_BUTTON_RADIUS / 2;
            active_backend->DrawText(label_x, label_y, button->label, win->active_theme->neutral, 0.27f, win->creation_id);
            active_backend->SetForeground(win->active_theme->neutral);
            active_backend->FillArc(button->core.x, button->core.y, RADIO_BUTTON_RADIUS * 2, RADIO_BUTTON_RADIUS * 2, 0, 360 * 64, win->creation_id);
            if (button->selected)
            {
                active_backend->SetForeground(win->active_theme->primary);
                active_backend->FillArc(button->core.x, button->core.y, RADIO_BUTTON_RADIUS * 1.5, RADIO_BUTTON_RADIUS * 1.5, 0, 360 * 64, win->creation_id);
            }
            else
            {
                active_backend->SetForeground(win->active_theme->base);

                active_backend->FillArc(button->core.x, button->core.y, RADIO_BUTTON_RADIUS * 1.5, RADIO_BUTTON_RADIUS * 1.5, 0, 360 * 64, win->creation_id);
            }
        }
    }
}

bool GooeyRadioButtonGroup_HandleClick(GooeyWindow *win, int x, int y)
{
    for (size_t i = 0; i < win->radio_button_group_count; ++i)
    {
        GooeyRadioButtonGroup *group = win->radio_button_groups[i];
        for (int j = 0; j < group->button_count; ++j)
        {
            GooeyRadioButton *button = &group->buttons[j];
            if (!button || !button->core.is_visible)
                continue;
            int dx = x - (button->core.x + RADIO_BUTTON_RADIUS);
            int dy = y - (button->core.y + RADIO_BUTTON_RADIUS);

            if (dx * dx + dy * dy <= (RADIO_BUTTON_RADIUS + 10) * (RADIO_BUTTON_RADIUS + 10))
            {
                for (int k = 0; k < group->button_count; ++k)
                {
                    group->buttons[k].selected = false;
                }

                button->selected = true;

                if (button->callback)
                {
                    button->callback(true);
                }
                return true;
            }
        }
    }
    return false;
}

bool GooeyRadioButton_HandleClick(GooeyWindow *win, int x, int y)
{
    int state = false;
    for (size_t i = 0; i < win->radio_button_count; ++i)
    {
        GooeyRadioButton *radio_button = win->radio_buttons[i];
        int dx = x - radio_button->core.x;
        int dy = y - radio_button->core.y;
        if (dx * dx + dy * dy <= radio_button->radius * radio_button->radius)
        {
            radio_button->selected = !radio_button->selected;
            if (radio_button->callback)
                radio_button->callback(radio_button->selected);
            state = true;
        }
        else
        {
            if (radio_button->selected)
            {
                radio_button->selected = !radio_button->selected;
            }
        }
    }
    return state;
}
#endif
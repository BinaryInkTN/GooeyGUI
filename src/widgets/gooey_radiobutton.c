/*
 Copyright (c) 2024 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "widgets/gooey_radiobutton.h"
#include "core/gooey_backend_internal.h"

GooeyRadioButtonGroup *GooeyRadioButtonGroup_Create(GooeyWindow *win)
{
    if (win->radio_button_group_count >= MAX_WIDGETS)
    {
        LOG_ERROR("Cannot create more radio button groups. Maximum limit reached.\n");
        return NULL;
    }
    GooeyRadioButtonGroup *group = &win->radio_button_groups[win->radio_button_group_count++];
    group->button_count = 0;
    LOG_INFO("Radio button group created and added to window.");

    return group;
}

GooeyRadioButton *GooeyRadioButtonGroup_AddChild(GooeyWindow *win, GooeyRadioButtonGroup *group, int x, int y, const char *label, void (*callback)(bool))
{
    if (group->button_count >= MAX_RADIO_BUTTONS)
    {
        LOG_ERROR("Cannot add more radio buttons to the group. Maximum limit reached.\n");
        return NULL;
    }
    group->buttons[group->button_count] = (GooeyRadioButton) {0};
    GooeyRadioButton *button = &group->buttons[group->button_count++];
    button->core.x = x;
    button->core.y = y;

    button->core.type = WIDGET_RADIOBUTTON;
    button->selected = false;
    if (label)
        strncpy(button->label, label, sizeof(button->label) - 1);
    else
        sprintf(button->label, "Radio button %d", group->button_count);

    LOG_INFO("Added child to radio button group at x=%d, y=%d.", x, y);

    return button;
}

GooeyRadioButton *GooeyRadioButton_Add(GooeyWindow *win, int x, int y,
                                       char *label,
                                       void (*callback)(bool selected))
{
    GooeyRadioButton *radio_button =
        &win->radio_buttons[win->radio_button_count++];

    radio_button->core.type = WIDGET_RADIOBUTTON;
    radio_button->core.x = x;
    radio_button->core.y = y;
    if (label)
        strcpy(radio_button->label, label);
    else
    {
        sprintf(radio_button->label, "Radio button %d", win->radio_button_count);
    }

    radio_button->radius = RADIO_BUTTON_RADIUS;
    radio_button->selected = false;
    radio_button->callback = callback;
    GooeyWindow_RegisterWidget(win, (GooeyWidget *)&radio_button->core);

    return radio_button;
}

void GooeyRadioButtonGroup_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->radio_button_group_count; ++i)
    {
        GooeyRadioButtonGroup *group = &win->radio_button_groups[i];
        for (int j = 0; j < group->button_count; ++j)
        {
            GooeyRadioButton *button = &group->buttons[j];
            int button_center_x = button->core.x + RADIO_BUTTON_RADIUS;
            int button_center_y = button->core.y + RADIO_BUTTON_RADIUS;

            int label_width = active_backend->GetTextWidth(button->label, strlen(button->label));

            int label_x = ACTIVE_BACKEND == X11 ? button->core.x + RADIO_BUTTON_RADIUS * 2 + 10 : button->core.x + RADIO_BUTTON_RADIUS * 2;
            int label_y = ACTIVE_BACKEND == X11 ? button->core.y + RADIO_BUTTON_RADIUS + 5 : button->core.y + RADIO_BUTTON_RADIUS / 2;
            active_backend->DrawText(label_x, label_y, button->label, active_theme->neutral, 0.25f, win->creation_id);
            active_backend->SetForeground(active_theme->neutral);
            active_backend->FillArc(button->core.x, button->core.y, RADIO_BUTTON_RADIUS * 2, RADIO_BUTTON_RADIUS * 2, 0, 360 * 64, win->creation_id);
            if (button->selected)
            {
                active_backend->SetForeground(active_theme->primary);
                active_backend->FillArc(ACTIVE_BACKEND == X11 ? button->core.x + 2 : button->core.x, ACTIVE_BACKEND == X11 ? button->core.y + 2 : button->core.y, RADIO_BUTTON_RADIUS * 1.5, RADIO_BUTTON_RADIUS * 1.5, 0, 360 * 64, win->creation_id);
            }
            else
            {
                active_backend->SetForeground(active_theme->base);

                active_backend->FillArc(ACTIVE_BACKEND == X11 ? button->core.x + 2 : button->core.x, ACTIVE_BACKEND == X11 ? button->core.y + 2 : button->core.y, RADIO_BUTTON_RADIUS * 1.5, RADIO_BUTTON_RADIUS * 1.5, 0, 360 * 64, win->creation_id);
            }
        }
    }
}

bool GooeyRadioButtonGroup_HandleClick(GooeyWindow *win, int x, int y)
{
    for (size_t i = 0; i < win->radio_button_group_count; ++i)
    {
        GooeyRadioButtonGroup *group = &win->radio_button_groups[i];
        for (int j = 0; j < group->button_count; ++j)
        {
            GooeyRadioButton *button = &group->buttons[j];
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
        GooeyRadioButton *radio_button = &win->radio_buttons[i];
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
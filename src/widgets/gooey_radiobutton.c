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
#if(ENABLE_RADIOBUTTON)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
#define RADIO_BUTTON_RADIUS 10 /** Radius of the radio button widget. */

GooeyRadioButtonGroup *GooeyRadioButtonGroup_Create()
{

    //     LOG_ERROR("Cannot create more radio button groups. Maximum limit reached.\n");
    //     return NULL;
    // }
    GooeyRadioButtonGroup *group = calloc(1, sizeof(GooeyRadioButtonGroup));
    if (group == NULL)
    {
        LOG_ERROR("Error allocating memory for radio button group");
        return NULL;
    }
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
    group->buttons[group->button_count] = (GooeyRadioButton){0};
    GooeyRadioButton *button = &group->buttons[group->button_count++];
    button->core.x = x;
    button->core.y = y;
    button->core.is_visible = true;
    button->core.type = WIDGET_RADIOBUTTON;
    button->selected = false;
    if (label)
        strncpy(button->label, label, sizeof(button->label) - 1);
    else
        sprintf(button->label, "Radio button %d", group->button_count);

    LOG_INFO("Added child to radio button group at x=%d, y=%d.", x, y);

    return button;
}

GooeyRadioButton *GooeyRadioButton_Create(int x, int y,
                                          char *label,
                                          void (*callback)(bool selected))
{
    GooeyRadioButton *radio_button = (GooeyRadioButton *)calloc(1, sizeof(GooeyRadioButton));

    if (!radio_button)
    {
        LOG_ERROR("Couldn't allocate memory for radio button.");
        return NULL;
    }

    *radio_button = (GooeyRadioButton){0};

    radio_button->core.type = WIDGET_RADIOBUTTON;
    
    radio_button->core.x = x;
    radio_button->core.y = y;
    if (label)
    {
        strncpy(radio_button->label, label, sizeof(radio_button->label) - 1);
        radio_button->label[sizeof(radio_button->label) - 1] = '\0';
    }

    else
    {
        strncpy(radio_button->label, "Radio button", sizeof(radio_button->label) - 1);
        radio_button->label[sizeof(radio_button->label) - 1] = '\0';
    }

    radio_button->radius = RADIO_BUTTON_RADIUS;
    radio_button->selected = false;
    radio_button->callback = callback;

    return radio_button;
}
#endif

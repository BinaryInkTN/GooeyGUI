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

#include "widgets/gooey_dropdown.h"
#if(ENABLE_DROPDOWN)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

GooeyDropdown *GooeyDropdown_Create(int x, int y, int width,
                                    int height, const char **options,
                                    int num_options,
                                    void (*callback)(int selected_index, void* user_data), void* user_data)
{
    GooeyDropdown *dropdown = (GooeyDropdown *)calloc(1, sizeof(GooeyDropdown));
    
    if(!dropdown)
    {
        LOG_ERROR("Couldn't allocate memory for dropdown.");
        return NULL;
    }

    *dropdown = (GooeyDropdown) {0};

    dropdown->core.type = WIDGET_DROPDOWN;
    dropdown->core.x = x;
    dropdown->core.y = y;
    dropdown->core.width = width;
    dropdown->core.height = height;
    dropdown->core.is_visible = true;
    dropdown->options = options;
    dropdown->num_options = num_options;
    dropdown->selected_index = 0;
    dropdown->element_hovered_over = -1;
    dropdown->callback = callback;
    dropdown->core.sprite = active_backend->CreateSpriteForWidget(x, y, width, height + 200); // todo make it dynamic
    dropdown->user_data = user_data;
    dropdown->core.disable_input = false;
    LOG_INFO("Dropdown added with dimensions x=%d, y=%d, w=%d, h=%d", x, y, width, height);

    dropdown->is_open = false;
    return dropdown;
}
void GooeyDropdown_Update(GooeyDropdown *dropdown, const char **new_options, int new_num_options)
{
    if (!dropdown)
        return;

    dropdown->options = new_options;
    dropdown->num_options = new_num_options;
    dropdown->selected_index = 0;
    dropdown->element_hovered_over = -1;
    dropdown->is_open = false;

    //if (dropdown->core.sprite)
   // {
    //    active_backend->DestroySprite(dropdown->core.sprite);
    //}
    //dropdown->core.sprite = active_backend->CreateSpriteForWidget(dropdown->core.x, dropdown->core.y, dropdown->core.width, dropdown->core.height + 200); // todo make it dynamic

    LOG_INFO("Dropdown updated with new number of options: %d", new_num_options);
}

#endif


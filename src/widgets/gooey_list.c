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

#include "widgets/gooey_list.h"
#if(ENABLE_LIST)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

#define DEFAULT_THUMB_WIDTH 10
#define DEFAULT_ITEM_SPACING 40
#define DEFAULT_SCROLL_OFFSET 1

GooeyList *GooeyList_Create(int x, int y, int width, int height, void (*callback)(int index))
{
    GooeyList *list = (GooeyList *)calloc(1, sizeof(GooeyList));
    if (!list)
    {
        LOG_ERROR("Couldn't allocate memory for list.");
        return NULL;
    }
    *list = (GooeyList){0};
    list->core.type = WIDGET_LIST;
    list->core.x = x;
    list->core.y = y;
    list->core.width = width;
    list->core.height = height;
    list->core.is_visible = true;
    list->items = (GooeyListItem *)calloc(1024, sizeof(GooeyListItem));
    list->item_count = 0;
    list->scroll_offset = DEFAULT_SCROLL_OFFSET;
    list->thumb_y = y;
    list->thumb_height = -1;
    list->thumb_width = DEFAULT_THUMB_WIDTH;
    list->item_spacing = DEFAULT_ITEM_SPACING;
    list->callback = callback;
    list->show_separator = true;
    list->element_hovered_over = -1;

    return list;
}

void GooeyList_AddItem(GooeyList *list, const char *title, const char *description)
{
    if (!list || !title || !description)
    {
        LOG_ERROR("Couldn't add item.");
        return;
    }

    GooeyListItem item = {0};
    strncpy(item.title, title, sizeof(item.title) - 1);
    item.title[sizeof(item.title) - 1] = '\0';
    strncpy(item.description, description, sizeof(item.description) - 1);
    item.description[sizeof(item.description) - 1] = '\0';
    list->items[list->item_count++] = item;
}

void GooeyList_UpdateItem(GooeyList *list, size_t item_index, const char *title, const char *description)
{
    if (!list || !title || !description)
    {
        LOG_ERROR("Couldn't update item.");
        return;
    }

    GooeyListItem *item = &list->items[item_index];

    if (!item)
    {
        LOG_ERROR("Couldn't update item with index %ld", item_index);
        return;
    }
    strncpy(item->title, title, sizeof(item->title) - 1);
    item->title[sizeof(item->title) - 1] = '\0';
    strncpy(item->description, description, sizeof(item->description) - 1);
    item->description[sizeof(item->description) - 1] = '\0';
}

void GooeyList_ClearItems(GooeyList *list)
{
    memset(list->items, 0, sizeof(*list->items));
    list->item_count = 0;
}

void GooeyList_ShowSeparator(GooeyList *list, bool state)
{
    list->show_separator = state;
}
#endif

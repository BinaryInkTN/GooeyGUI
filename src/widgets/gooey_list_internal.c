#include "widgets/gooey_list_internal.h"
#if(ENABLE_LIST)
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"
#include "logger/pico_logger_internal.h"
#include <string.h>

void GooeyList_Draw(GooeyWindow *win)
{
    const int title_description_spacing = 15;

    for (size_t i = 0; i < win->list_count; ++i)
    {
        GooeyList *list = win->lists[i];
        if(!list->core.is_visible)
            continue;
        active_backend->FillRectangle(
            list->core.x, list->core.y,
            list->core.width, list->core.height,
            win->active_theme->widget_base, win->creation_id, false, 0.0f);

        active_backend->DrawRectangle(
            list->core.x, list->core.y,
            list->core.width, list->core.height,
            win->active_theme->neutral, 0.1f, win->creation_id, false, 0.0f);

        active_backend->FillRectangle(
            list->core.x + list->core.width, list->core.y,
            list->thumb_width, list->core.height,
            win->active_theme->widget_base, win->creation_id, false, 0.0f);

        active_backend->DrawRectangle(
            list->core.x + list->core.width, list->core.y,
            list->thumb_width, list->core.height,
            win->active_theme->neutral, 0.1f, win->creation_id, false, 0.0f);

        int total_content_height = list->item_count * list->item_spacing;
        int visible_height = list->core.height;
        int max_scroll_offset = (total_content_height > visible_height)
                                    ? total_content_height - visible_height
                                    : 0;

        if (list->scroll_offset < -max_scroll_offset)
            list->scroll_offset = -max_scroll_offset;
        if (list->scroll_offset > 0)
            list->scroll_offset = 0;

        int current_y_offset = list->core.y + list->scroll_offset + 10;
        list->thumb_height = (total_content_height <= visible_height)
                                 ? list->core.height
                                 : (int)((float)visible_height * visible_height / total_content_height);
        if (total_content_height > 0)
        {
            list->thumb_y = list->core.y - (int)((float)list->scroll_offset * visible_height / total_content_height);

            active_backend->FillRectangle(
                list->core.x + list->core.width, list->thumb_y,
                list->thumb_width, list->thumb_height,
                win->active_theme->primary, win->creation_id, true, 2.0f);
        }

        for (size_t j = 0; j < list->item_count; ++j)
        {
            GooeyListItem item = list->items[j];

            int title_y = current_y_offset + active_backend->GetTextHeight(item.title, strlen(item.title));
            int description_y = title_y + title_description_spacing;

            if (title_y < list->core.y + list->core.height && title_y > list->core.y + 5)
            {
                active_backend->DrawText(
                    list->core.x + 10, title_y,
                    item.title, win->active_theme->neutral,
                    0.27f, win->creation_id);
            }

            if (description_y < list->core.y + list->core.height && description_y > list->core.y + 5)
            {
                active_backend->DrawText(
                    list->core.x + 10, description_y,
                    item.description, win->active_theme->neutral,
                    0.27f, win->creation_id);
            }

            int line_separator_y = current_y_offset + list->item_spacing - 10;
            if (j < list->item_count - 1 &&
                line_separator_y < list->core.y + list->core.height - 10 &&
                line_separator_y > list->core.y + 5)
            {
                if (list->show_separator)
                    active_backend->DrawLine(
                        list->core.x, line_separator_y,
                        list->core.x + list->core.width,
                        line_separator_y, win->active_theme->neutral,
                        win->creation_id);
            }

            current_y_offset += list->item_spacing;
        }
    }
}

bool GooeyList_HandleScroll(GooeyWindow *window, void *scroll_event)
{
    const int scroll_speed_multiplier = 20;
    GooeyEvent *event = (GooeyEvent *)scroll_event;

    for (size_t i = 0; i < window->list_count; ++i)
    {
        GooeyList *list = window->lists[i];
        {
            if (!list->core.is_visible)
                continue;
            int mouse_x = event->mouse_move.x;
            int mouse_y = event->mouse_move.y;

            int total_content_height = list->item_count * list->item_spacing;
            int visible_height = list->core.height;

            if (mouse_x >= list->core.x && mouse_x <= list->core.x + list->core.width &&
                mouse_y >= list->core.y && mouse_y <= list->core.y + list->core.height)
            {
                if (event->type == GOOEY_EVENT_MOUSE_SCROLL)
                {
                    int scroll_offset_amount = event->mouse_scroll.y *
                                               (total_content_height / visible_height) *
                                               scroll_speed_multiplier;
                    list->scroll_offset += scroll_offset_amount;

                    return true;
                }
                else if (event->type == GOOEY_EVENT_KEY_PRESS)
                {
                    const char *key = active_backend->GetKeyFromCode(event);
                    LOG_ERROR("%s", key);

                    if (strcmp(key, "Up") == 0)
                        list->scroll_offset += (total_content_height / visible_height) * scroll_speed_multiplier;
                    else if (strcmp(key, "Down") == 0)
                        list->scroll_offset -= (total_content_height / visible_height) * scroll_speed_multiplier;
                }
            }
        }
    }

    return false;
}

bool GooeyList_HandleClick(GooeyWindow *window, int mouse_x, int mouse_y)
{
    for (size_t i = 0; i < window->list_count; ++i)
    {
        GooeyList *list = window->lists[i];
        if (!list || !list->core.is_visible)
            continue;
        if (mouse_x >= list->core.x && mouse_x <= list->core.x + list->core.width &&
            mouse_y >= list->core.y && mouse_y <= list->core.y + list->core.height)
        {
            int scroll_offset = list->scroll_offset;
            int mouse_y_relative = mouse_y - list->core.y;
            int adjusted_y = mouse_y_relative + scroll_offset;

            if (list->item_spacing <= 0)
                return false;

            int selected_index = adjusted_y / list->item_spacing;

            if (selected_index >= 0 && (unsigned long) selected_index < list->item_count)
            {
                if (list->callback)
                {
                    list->callback(selected_index);
                }

                return true;
            }
        }
    }

    return false;
}

bool GooeyList_HandleThumbScroll(GooeyWindow *window, void *scroll_event)
{
    static int mouse_prev = -1;
    static bool is_dragging = false;
    GooeyEvent *event = (GooeyEvent *)scroll_event;

    for (size_t i = 0; i < window->list_count; ++i)
    {
        GooeyList *list = window->lists[i];

        int mouse_x = event->mouse_move.x;
        int mouse_y = event->mouse_move.y;

        int thumb_width = list->thumb_width;
        int thumb_height = list->thumb_height;
        int thumb_x = list->core.x + list->core.width;
        int thumb_y = list->thumb_y;
        int total_content_height = list->item_count * list->item_spacing;
        int visible_height = list->core.height;

        if (mouse_x >= thumb_x && mouse_x <= thumb_x + thumb_width &&
            mouse_y >= thumb_y && mouse_y <= thumb_y + thumb_height &&
            event->type == GOOEY_EVENT_CLICK_PRESS)
        {
            is_dragging = true;
        }

        if (is_dragging)
        {
            if (event->type != GOOEY_EVENT_CLICK_RELEASE)
            {
                if (mouse_prev != -1)
                    list->scroll_offset -= (mouse_y - mouse_prev) * (total_content_height / visible_height);
                mouse_prev = mouse_y;
            }
            else
            {
                is_dragging = false;
                mouse_prev = -1;
            }

            return true;
        }
    }
    return false;
}
#endif
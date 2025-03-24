#include "gooey_list_internal.h"
#include "backends/gooey_backend_internal.h"
#include "event/gooey_event_internal.h"
#include "logger/pico_logger_internal.h"

bool GooeyList_HandleScroll(GooeyWindow *window, void *scroll_event)
{
    const int scroll_speed_multiplier = 2;
    GooeyEvent* event = (GooeyEvent*) scroll_event;

    for (size_t i = 0; i < window->list_count; ++i)
    {
        GooeyList *list = window->lists[i];

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
    return false;
}

bool GooeyList_HandleClick(GooeyWindow *window, int mouse_x, int mouse_y)
{
    for (size_t i = 0; i < window->list_count; ++i)
    {
        GooeyList *list = window->lists[i];

        if (mouse_x >= list->core.x && mouse_x <= list->core.x + list->core.width &&
            mouse_y >= list->core.y && mouse_y <= list->core.y + list->core.height)
        {
            int scroll_offset = list->scroll_offset;
            int mouse_y_relative = mouse_y - list->core.y;
            int adjusted_y = mouse_y_relative + scroll_offset;

            if (list->item_spacing <= 0)
                return false;

            int selected_index = adjusted_y / list->item_spacing;

            if (selected_index >= 0 && selected_index < list->item_count)
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
    GooeyEvent* event = (GooeyEvent*) scroll_event;

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
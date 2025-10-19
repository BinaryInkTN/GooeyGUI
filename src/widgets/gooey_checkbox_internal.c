#include "widgets/gooey_checkbox_internal.h"
#if (ENABLE_CHECKBOX)
#include "backends/gooey_backend_internal.h"
#define CHECKBOX_SIZE 20 /** Size of a checkbox widget. */

void GooeyCheckbox_Draw(GooeyWindow *win)
{

    for (size_t i = 0; i < win->checkbox_count; ++i)
    {
        GooeyCheckbox *checkbox = win->checkboxes[i];
        if (!checkbox->core.is_visible)
            continue;
        int label_width = active_backend->GetTextWidth(checkbox->label, strlen(checkbox->label));
        int label_x = checkbox->core.x + CHECKBOX_SIZE + 10;
        int label_y = checkbox->core.y + (CHECKBOX_SIZE / 2) + 5;
        active_backend->DrawText(label_x, label_y, checkbox->label, win->active_theme->neutral, 0.27f, win->creation_id, checkbox->core.sprite);

        active_backend->DrawRectangle(checkbox->core.x, checkbox->core.y,
                                      checkbox->core.width, checkbox->core.height, win->active_theme->neutral,1.0f, win->creation_id, false, 0.0f, checkbox->core.sprite);
        active_backend->FillRectangle(checkbox->core.x + 1, checkbox->core.y + 1,
                                      checkbox->core.width - 2, checkbox->core.height - 2, win->active_theme->base, win->creation_id, false, 0.0f, checkbox->core.sprite);

        if (checkbox->checked)
        {
            active_backend->FillRectangle(checkbox->core.x + 5, checkbox->core.y + 5,
                                          checkbox->core.width - 10, checkbox->core.height - 10, win->active_theme->primary, win->creation_id, false, 0.0f, checkbox->core.sprite);
        }
    }
}

bool GooeyCheckbox_HandleClick(GooeyWindow *win, int x, int y)
{
    for (size_t i = 0; i < win->checkbox_count; ++i)
    {
        GooeyCheckbox *checkbox = win->checkboxes[i];
                if(!checkbox || !checkbox->core.is_visible || checkbox->core.disable_input) continue;

        if (x >= checkbox->core.x && x <= checkbox->core.x + checkbox->core.width &&
            y >= checkbox->core.y &&
            y <= checkbox->core.y + checkbox->core.height)
        {
            checkbox->checked = !checkbox->checked;
            if (checkbox->callback)
                checkbox->callback(checkbox->checked, checkbox->user_data);
            return true;
        }
    }
    return false;
}
#endif
#include "widgets/gooey_tabs_internal.h"
#if (ENABLE_TABS)
#include "backends/gooey_backend_internal.h"
#include "widgets/gooey_window_internal.h"
#include "core/gooey_timers_internal.h"
#include "logger/pico_logger_internal.h"
#include "animations/gooey_animations_internal.h"

static float ease_out_quad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

static float ease_in_out_quad(float t)
{
    return t < 0.5f ? 2.0f * t * t : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
}

static void update_sidebar_widget_visibility(GooeyTabs *tabs)
{
    if (!tabs || !tabs->is_sidebar)
        return;

    int current_sidebar_width = tabs->is_animating ? tabs->sidebar_offset : (tabs->is_open ? TAB_WIDTH : 0);

    for (size_t j = 0; j < tabs->tab_count; ++j)
    {
        GooeyTab *tab = &tabs->tabs[j];
        for (size_t k = 0; k < tab->widget_count; ++k)
        {
            if (!tab->widgets[k])
                continue;

            GooeyWidget *widget = (GooeyWidget *)tab->widgets[k];
            widget->is_visible = (tabs->active_tab_id == tab->tab_id);
            widget->disable_input = tabs->is_open;
        }
    }
}

static void sidebar_animation_callback(void *user_data)
{
    GooeyTabs *tabs = (GooeyTabs *)user_data;
    if (!tabs || !tabs->is_animating)
        return;

    tabs->current_step++;

    if (tabs->current_step >= SIDEBAR_ANIMATION_STEPS)
    {
        tabs->sidebar_offset = tabs->target_offset;
        tabs->is_animating = false;
        tabs->is_open = (tabs->target_offset == TAB_WIDTH);

        if (tabs->animation_timer)
        {
            GooeyTimer_Stop_Internal(tabs->animation_timer);
        }

        update_sidebar_widget_visibility(tabs);
        return;
    }

    float progress = (float)tabs->current_step / (float)SIDEBAR_ANIMATION_STEPS;
    float eased_progress = ease_out_quad(progress);
    int start_offset = tabs->target_offset == TAB_WIDTH ? 0 : TAB_WIDTH;
    int distance = tabs->target_offset - start_offset;
    tabs->sidebar_offset = start_offset + (int)(distance * eased_progress);

    update_sidebar_widget_visibility(tabs);

    if (tabs->animation_timer && tabs->is_animating)
    {
        GooeyTimer_SetCallback_Internal(SIDEBAR_ANIMATION_SPEED, tabs->animation_timer, sidebar_animation_callback, tabs);
    }
}

static void tab_line_animation_callback(void *user_data)
{
    GooeyTabs *tabs = (GooeyTabs *)user_data;
    if (!tabs)
        return;

    if (!tabs->is_sidebar && tabs->is_animating)
    {
        tabs->current_step++;

        if (tabs->current_step >= TABLINE_ANIMATION_STEPS)
        {
            tabs->sidebar_offset = tabs->target_offset;
            tabs->is_animating = false;

            if (tabs->animation_timer)
            {
                GooeyTimer_Stop_Internal(tabs->animation_timer);
            }
            return;
        }

        float progress = (float)tabs->current_step / (float)TABLINE_ANIMATION_STEPS;
        float eased_progress = ease_in_out_quad(progress);

        int start_x = tabs->is_open ? tabs->sidebar_offset : tabs->target_offset;
        int target_x = tabs->target_offset;
        int x_distance = target_x - start_x;
        tabs->sidebar_offset = start_x + (int)(x_distance * eased_progress);

        if (tabs->animation_timer && tabs->is_animating)
        {
            GooeyTimer_SetCallback_Internal(TABLINE_ANIMATION_SPEED, tabs->animation_timer, tab_line_animation_callback, tabs);
        }
    }
}

static void start_tab_line_animation(GooeyTabs *tabs, int target_tab_index)
{
    if (!tabs || tabs->is_sidebar)
        return;

    int tab_width = tabs->core.width / (int)tabs->tab_count;
    int target_x = tabs->core.x + tab_width * target_tab_index;

    tabs->target_offset = target_x;
    tabs->current_step = 0;
    tabs->is_animating = true;

    if (!tabs->is_open)
    {
        tabs->is_open = true;
    }

    if (!tabs->animation_timer)
    {
        tabs->animation_timer = GooeyTimer_Create_Internal();
        if (!tabs->animation_timer)
        {

            tabs->sidebar_offset = target_x;
            tabs->is_animating = false;
            return;
        }
    }

    GooeyTimer_SetCallback_Internal(TABLINE_ANIMATION_SPEED, tabs->animation_timer, tab_line_animation_callback, tabs);
}

void GooeyTabs_ToggleSidebar(GooeyTabs *tabs)
{
    if (!tabs || !tabs->is_sidebar)
        return;

    if (tabs->is_animating && tabs->target_offset == (tabs->is_open ? 0 : TAB_WIDTH))
        return;

    tabs->target_offset = tabs->is_open ? 0 : TAB_WIDTH;
    tabs->current_step = 0;

    if (!tabs->animation_timer)
    {
        tabs->animation_timer = GooeyTimer_Create_Internal();
        if (!tabs->animation_timer)
            return;
    }

    tabs->is_animating = true;
    update_sidebar_widget_visibility(tabs);

    GooeyTimer_SetCallback_Internal(SIDEBAR_ANIMATION_SPEED, tabs->animation_timer, sidebar_animation_callback, tabs);
}

void GooeyTabs_Cleanup(GooeyTabs *tabs)
{
    if (!tabs)
        return;

    if (tabs->animation_timer)
    {
        GooeyTimer_Stop_Internal(tabs->animation_timer);
        GooeyTimer_Destroy_Internal(tabs->animation_timer);
        tabs->animation_timer = NULL;
    }

    tabs->is_animating = false;
}

bool GooeyTabs_HandleClick(GooeyWindow *win, int mouse_x, int mouse_y)
{
    if (!win || !win->tabs)
        return false;

    for (size_t i = 0; i < win->tab_count; ++i)
    {
        GooeyTabs *tabs = win->tabs[i];
        if (!tabs || !tabs->is_sidebar)
            continue;

        int toggle_width = 15;
        int current_sidebar_width = tabs->is_animating ? tabs->sidebar_offset : (tabs->is_open ? TAB_WIDTH : 0);
        if (mouse_x >= tabs->core.x &&
            mouse_x < tabs->core.x + current_sidebar_width + toggle_width &&
            mouse_y >= tabs->core.y &&
            mouse_y < tabs->core.y + tabs->core.height)
        {
            bool tab_clicked = false;
            if (current_sidebar_width > 0)
            {
                for (size_t j = 0; j < tabs->tab_count; ++j)
                {
                    int tab_y = tabs->core.y + TAB_ELEMENT_HEIGHT * (int)j;

                    if (mouse_y >= tab_y &&
                        mouse_y < tab_y + TAB_ELEMENT_HEIGHT &&
                        mouse_x >= tabs->core.x &&
                        mouse_x < tabs->core.x + current_sidebar_width)
                    {
                        tabs->active_tab_id = tabs->tabs[j].tab_id;
                        tab_clicked = true;

                        update_sidebar_widget_visibility(tabs);
                        break;
                    }
                }
            }

            if (!tab_clicked)
            {
                GooeyTabs_ToggleSidebar(tabs);
            }
            return true;
        }
    }

    for (size_t i = 0; i < win->tab_count; ++i)
    {
        GooeyTabs *tabs = win->tabs[i];
        if (!tabs || tabs->is_sidebar)
            continue;

        int tab_width = tabs->core.width / (int)tabs->tab_count;
        int tab_height = TAB_HEIGHT;

        for (size_t j = 0; j < tabs->tab_count; ++j)
        {
            int tab_x = tabs->core.x + tab_width * (int)j;
            int tab_y = tabs->core.y;

            if (mouse_x >= tab_x && mouse_x < tab_x + tab_width &&
                mouse_y >= tab_y && mouse_y < tab_y + tab_height)
            {
                tabs->active_tab_id = tabs->tabs[j].tab_id;
                start_tab_line_animation(tabs, (int)j);
                return true;
            }
        }
    }
    return false;
}

void GooeyTabs_Draw(GooeyWindow *win)
{
    if (!win || !win->tabs)
        return;

    for (size_t i = 0; i < win->tab_count; ++i)
    {
        GooeyTabs *tabs = win->tabs[i];
        if (!tabs || tabs->is_sidebar)
            continue;

        active_backend->DrawRectangle(
            tabs->core.x,
            tabs->core.y,
            tabs->core.width,
            tabs->core.height,
            win->active_theme->widget_base, 1.0f,
            win->creation_id, false, 0.0f, tabs->core.sprite);

        const int visible_area_x = tabs->core.x;
        const int visible_area_y = tabs->core.y + TAB_HEIGHT;
        const int visible_area_w = tabs->core.width;
        const int visible_area_h = tabs->core.height - TAB_HEIGHT;

        const int tab_width = tabs->core.width / (int)tabs->tab_count;

        int line_x;

        if (tabs->is_animating)
        {

            line_x = tabs->sidebar_offset;
        }
        else
        {

            for (size_t j = 0; j < tabs->tab_count; ++j)
            {
                if (tabs->tabs[j].tab_id == tabs->active_tab_id)
                {
                    line_x = tabs->core.x + tab_width * (int)j;

                    tabs->sidebar_offset = line_x;
                    tabs->target_offset = line_x;
                    break;
                }
            }
        }

        active_backend->DrawLine(
            line_x,
            tabs->core.y + TAB_HEIGHT,
            line_x + tab_width,
            tabs->core.y + TAB_HEIGHT,
            win->active_theme->primary,
            win->creation_id, tabs->core.sprite);

        for (size_t j = 0; j < tabs->tab_count; ++j)
        {
            GooeyTab *tab = &tabs->tabs[j];
            const int tab_x = tabs->core.x + tab_width * (int)j;
            const int tab_y = tabs->core.y;

            const int text_width = active_backend->GetTextWidth(tab->tab_name, strlen(tab->tab_name));
            const int text_height = active_backend->GetTextHeight(tab->tab_name, strlen(tab->tab_name));
            const int tab_name_x = tab_x + (tab_width / 2) - text_width / 2;

            active_backend->DrawText(
                tab_name_x,
                tab_y + (TAB_HEIGHT / 2) + (text_height / 2),
                tab->tab_name,
                win->active_theme->neutral,
                TAB_TEXT_SCALE,
                win->creation_id, tabs->core.sprite);

            for (size_t k = 0; k < tab->widget_count; ++k)
            {
                if (!tab->widgets[k])
                    continue;

                GooeyWidget *widget = (GooeyWidget *)tab->widgets[k];
                widget->is_visible = (tabs->active_tab_id == tab->tab_id) &&
                                     (widget->x >= visible_area_x) &&
                                     (widget->x < visible_area_x + visible_area_w) &&
                                     (widget->y >= visible_area_y) &&
                                     (widget->y < visible_area_y + visible_area_h);
            }
        }
    }

    for (size_t i = 0; i < win->tab_count; ++i)
    {
        GooeyTabs *tabs = win->tabs[i];
        if (!tabs || !tabs->is_sidebar)
            continue;
        update_sidebar_widget_visibility(tabs);

        int current_sidebar_width = tabs->is_animating ? tabs->sidebar_offset : (tabs->is_open ? TAB_WIDTH : 0);

        if (current_sidebar_width > 0)
        {
            active_backend->FillRectangle(
                tabs->core.x,
                tabs->core.y,
                current_sidebar_width,
                tabs->core.height,
                win->active_theme->widget_base,
                win->creation_id,
                true,
                2.0f, tabs->core.sprite);
        }

        for (size_t j = 0; j < tabs->tab_count; ++j)
        {
            GooeyTab *tab = &tabs->tabs[j];
            const int tab_y = tabs->core.y + TAB_ELEMENT_HEIGHT * (int)j;

            if (current_sidebar_width > 0)
            {
                if (tabs->active_tab_id == tab->tab_id)
                {
                    int highlight_width = current_sidebar_width;
                    active_backend->FillRectangle(
                        tabs->core.x,
                        tab_y,
                        highlight_width,
                        TAB_ELEMENT_HEIGHT,
                        win->active_theme->primary,
                        win->creation_id,
                        false,
                        0.0f, tabs->core.sprite);
                }

                if (current_sidebar_width >= TAB_TEXT_PADDING * 2)
                {
                    const int text_height = active_backend->GetTextHeight(tab->tab_name, strlen(tab->tab_name));
                    active_backend->DrawText(
                        tabs->core.x + TAB_TEXT_PADDING,
                        tab_y + TAB_ELEMENT_HEIGHT / 2 + text_height / 2,
                        tab->tab_name,
                        win->active_theme->neutral,
                        TAB_TEXT_SCALE,
                        win->creation_id, tabs->core.sprite);
                }
            }
        }

        if (current_sidebar_width < TAB_WIDTH)
        {
            active_backend->FillRectangle(
                tabs->core.x + current_sidebar_width,
                tabs->core.y,
                2,
                tabs->core.height,
                win->active_theme->primary,
                win->creation_id,
                false,
                0.0f, tabs->core.sprite);
        }
    }
}
#endif
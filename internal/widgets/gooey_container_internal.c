//
// Created by overflow on 6/10/2025.
//

#include "gooey_container_internal.h"
#include "logger/pico_logger_internal.h"
#if (ENABLE_CONTAINER)

void GooeyContainer_Draw(GooeyWindow *window)
{
    if (!window)
    {
        LOG_ERROR("Couldn't draw container.");
        return;
    }

    for (size_t i = 0; i < window->container_count; ++i)
    {
        GooeyContainers *container = window->containers[i];
        if (!container || !container->core.is_visible)
        {
            continue;
        }

        for (size_t j = 0; j < container->container_count; j++)
        {
            GooeyContainer *cont = &container->container[j];
            for (size_t k = 0; k < cont->widget_count; ++k)
            {

                void *widget = cont->widgets[k];
                GooeyWidget *core = (GooeyWidget *)widget;

                // Widgets
                if (container->active_container_id == cont->id)
                {

                    core->is_visible = true;
                }
                else
                {
                    core->is_visible = false;
                }
            }
        }
    }
}
#endif
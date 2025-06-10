//
// Created by overflow on 6/10/2025.
//

#include "gooey_container_internal.h"
#include "logger/pico_logger_internal.h"
#if (ENABLE_CONTAINER)

void GooeyContainer_Draw(GooeyWindow* window) {
    if (!window) {
        LOG_ERROR("Couldn't draw container.");
        return;
    }

    for (size_t i=0; i<window->container_count; ++i) {
        GooeyContainer *container = window->containers[i];
        if (!container || !container->core.is_visible) {
            continue;
        }

        // Manage children visibility
        for (size_t j=0; j<container->widget_count; ++j) {
            container->widgets[j]->is_visible = container->core.is_visible;
        }
    }
}

#endif
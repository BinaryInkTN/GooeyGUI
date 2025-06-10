//
// Created by overflow on 6/10/2025.
//
#include "widgets/gooey_container.h"
#include "logger/pico_logger_internal.h"

GooeyContainer* GooeyContainer_Create(int x, int y, int width, int height) {
    if (x < 0 || y <0 || width < 0 || height < 0) {
        LOG_ERROR("Invalid arguments");
        return NULL;
    }

    GooeyContainer* container = (GooeyContainer*)calloc(1, sizeof(GooeyContainer));
    *container = (GooeyContainer) {
    .core = {
        .is_visible = true,
        .type = WIDGET_CONTAINER,
    },
        .widget_count = 0,
        .widgets = calloc(MAX_CONTAINER_CHILDREN, sizeof(GooeyContainer*))
    };

    return container;
}

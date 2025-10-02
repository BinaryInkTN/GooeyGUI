//
// Created by overflow on 6/10/2025.
//

#include "widgets/gooey_container.h"
#include "logger/pico_logger_internal.h"
#include "widgets/gooey_window_internal.h"
#include "backends/gooey_backend_internal.h"
GooeyContainers *GooeyContainer_Create(int x, int y, int width, int height)
{
    GooeyContainers *container_widget = calloc(1, sizeof(GooeyContainers));
    if (container_widget == NULL)
    {
        LOG_ERROR("Unable to allocate memory to tabs widget");
        return NULL;
    }
    container_widget->core.type = WIDGET_CONTAINER;
    container_widget->core.x = x;
    container_widget->core.y = y;
    container_widget->core.width = width;
    container_widget->core.height = height;
    container_widget->container = calloc(MAX_CONTAINER, sizeof(GooeyContainer));
    if (!container_widget->container)
    {
        LOG_ERROR("Unable to allocate container array");
        free(container_widget);
        return NULL;
    }
    container_widget->container_count = 0;
    container_widget->active_container_id = 0;
    container_widget->core.is_visible = true;
    container_widget->core.sprite = NULL;
    return container_widget;
}

void GooeyContainer_InsertContainer(GooeyContainers *container)
{
    if (!container)
    {
        LOG_ERROR("Couldn't insert tab, tab widget is invalid.");
        return;
    }

    if (container->container_count >= MAX_CONTAINER)
    {
        LOG_ERROR("Maximum container limit reached: %d", MAX_CONTAINER);
        return;
    }

    size_t container_id = container->container_count;
    GooeyContainer *cont = &container->container[container_id];
    cont->id = container_id;
    cont->widgets = calloc(MAX_WIDGETS, sizeof(void *));
    if (!cont->widgets)
    {
        LOG_ERROR("Unable to allocate widgets array for container %zu", container_id);
        return;
    }
    cont->widget_count = 0;
    container->container_count++;
}

void GooeyContainer_AddWidget(GooeyWindow *window, GooeyContainers *container, size_t container_id, void *widget)
{
    if (!window || !container || !widget)
    {
        LOG_ERROR("Invalid parameters passed to GooeyContainer_AddWidget");
        return;
    }

    if (container_id >= container->container_count)
    {
        LOG_ERROR("Container ID %zu out of bounds (max: %zu)", container_id, container->container_count - 1);
        return;
    }

    GooeyContainer *selected_cont = &container->container[container_id];
    if (!selected_cont || !selected_cont->widgets)
    {
        LOG_ERROR("Selected container or its widgets array is invalid");
        return;
    }

    if (selected_cont->widget_count >= MAX_WIDGETS)
    {
        LOG_ERROR("Maximum widget limit reached for container %zu", container_id);
        return;
    }

    GooeyWidget *core = (GooeyWidget *)widget;
    core->x = core->x + container->core.x;
    core->y = core->y + container->core.y;

    selected_cont->widgets[selected_cont->widget_count] = widget;
    selected_cont->widget_count++;

    // Register widget to window implicitly
    GooeyWindow_Internal_RegisterWidget(window, widget);
}

void GooeyContainer_SetActiveContainer(GooeyContainers *container, size_t container_id)
{
    if (!container)
    {
        LOG_ERROR("Container is NULL");
        return;
    }

    if (container_id >= container->container_count)
    {
        LOG_ERROR("Container ID %zu out of bounds (max: %zu)", container_id, container->container_count - 1);
        return;
    }

    container->active_container_id = container_id;
}
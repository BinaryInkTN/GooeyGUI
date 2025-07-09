//
// Created by overflow on 6/10/2025.
//

#include "widgets/gooey_container.h"
#include "logger/pico_logger_internal.h"
#include "widgets/gooey_window_internal.h"
#include "backends/gooey_backend_internal.h"


GooeyContainers* GooeyContainer_Create(int x, int y, int width, int height)
{
  

    GooeyContainers *container_widget = calloc(1, sizeof(GooeyContainers));
    if (container_widget == NULL ){
        LOG_ERROR("Unable to allocate memory to tabs widget");
        return NULL ; 
    }
    *container_widget = (GooeyContainers){0};
    container_widget->core.type = WIDGET_CONTAINER;
    container_widget->core.x = x;
    container_widget->core.y = y;
    container_widget->core.width = width;
    container_widget->core.height = height;
    container_widget->container = malloc(sizeof(GooeyContainer)* MAX_CONTAINER);
    container_widget->container_count = 0;
    container_widget->active_container_id = 0; // default active tab is the first one.
    container_widget->core.is_visible = true;
    container_widget->core.sprite = active_backend->CreateSpriteForWidget(x, y, width, height);
    return container_widget;
}

void GooeyContainer_InsertContainer(GooeyContainers *container)
{

    if (!container)
    {
        LOG_ERROR("Couldn't insert tab, tab widget is invalid.");
        return;
    }

    size_t container_id = container->container_count;
    GooeyContainer *cont = &container->container[container->container_count++];
    cont->id = container_id;
    cont->widgets = (void **)calloc(MAX_WIDGETS, sizeof(void *));
    cont->widget_count = 0;
}

void GooeyContainer_AddWidget(GooeyWindow* window, GooeyContainers* container, size_t container_id, void *widget)
{
    GooeyContainer* selected_cont = (GooeyContainer*) &container->container[container_id];
    if (!container || !selected_cont || !widget)
    {
        LOG_ERROR("Couldn't add widget.");
        return;
    }


    GooeyWidget *core = (GooeyWidget*) widget;
    core->x = core->x + container->core.x;
    core->y = core->y + container->core.y ;

    selected_cont->widgets[selected_cont->widget_count++] = widget;

    // Register widget to window implicitly
    GooeyWindow_Internal_RegisterWidget(window, widget);
    
}

void GooeyContainer_SetActiveContainer(GooeyContainers *container, size_t container_id)
{
    if (!container || container_id > container->container_count)
    {
        LOG_ERROR("Couldn't set active Container.");
        // container->active_container_id = 0;

        return;
    }

    container->active_container_id = container_id;
}

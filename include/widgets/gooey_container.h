//
// Created by overflow on 6/10/2025.
//

#ifndef GOOEY_CONTAINER_H
#define GOOEY_CONTAINER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_CONTAINER)

/**
 * @brief Creates a new container with the given position and dimensions.
 *
 * @param x X-coordinate of the container.
 * @param y Y-coordinate of the container.
 * @param width Width of the container.
 * @param height Height of the container.
 * @return Pointer to the newly created GooeyContainers structure.
 */
GooeyContainers* GooeyContainer_Create(int x, int y, int width, int height);

/**
 * @brief Inserts a sub-container into the given container.
 *
 * @param container Pointer to the parent GooeyContainers.
 */
void GooeyContainer_InsertContainer(GooeyContainers *container);

/**
 * @brief Adds a widget to a specific container within the window.
 *
 * @param window The target GooeyWindow.
 * @param container The container to add the widget into.
 * @param container_id The ID of the target sub-container.
 * @param widget Pointer to the widget to add.
 */
void GooeyContainer_AddWidget(GooeyWindow* window, GooeyContainers* container, size_t container_id, void *widget);

/**
 * @brief Sets which container is currently active/visible.
 *
 * @param container Pointer to the container set.
 * @param container_id ID of the container to activate.
 */
void GooeyContainer_SetActiveContainer(GooeyContainers *container, size_t container_id);

#endif // ENABLE_CONTAINER

#ifdef __cplusplus
}
#endif

#endif // GOOEY_CONTAINER_H

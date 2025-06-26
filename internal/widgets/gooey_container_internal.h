/**
 * @file gooey_container_internal.h
 * @brief Internal header for Gooey container widget rendering logic.
 *        Handles drawing of container widgets inside Gooey windows.
 * @author Yassine Ahmed Ali
 * @date 2025-06-10
 */

#ifndef GOOEY_CONTAINER_INTERNAL_H
#define GOOEY_CONTAINER_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_CONTAINER)

/**
 * @brief Draws all containers within the specified window.
 *
 * @param window The window that holds the containers.
 */
void GooeyContainer_Draw(GooeyWindow* window);

#endif // ENABLE_CONTAINER

#ifdef __cplusplus
}
#endif

#endif // GOOEY_CONTAINER_INTERNAL_H

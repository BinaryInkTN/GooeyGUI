/**
 * @file gooey_drop_surface_internal.h
 * @brief Header file for the GooeyDropSurface module.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * Provides functionality for creating file drop surfaces, handling file drops,
 * and rendering drop surfaces within a GooeyWindow.
 */

#ifndef GOOEY_DROP_SURFACE_INTERNAL_H
#define GOOEY_DROP_SURFACE_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_DROP_SURFACE)

/**
 * @brief Handles file drop events for drop surfaces within the specified window.
 *
 * Checks whether a file has been dropped onto a drop surface and
 * triggers the associated callback function.
 *
 * @param win The window containing the drop surface.
 * @param mouseX The x-coordinate of the mouse when the file was dropped.
 * @param mouseY The y-coordinate of the mouse when the file was dropped.
 * @return True if a file was dropped onto a valid drop surface, false otherwise.
 */
bool GooeyDropSurface_HandleFileDrop(GooeyWindow *win, int mouseX, int mouseY);

/**
 * @brief Draws all drop surfaces within the specified window.
 *
 * This function renders all GooeyDropSurface elements that belong to
 * the given window, ensuring they display the correct message and state.
 *
 * @param win The window containing the drop surfaces.
 */
void GooeyDropSurface_Draw(GooeyWindow *win);

#endif // ENABLE_DROP_SURFACE

#ifdef __cplusplus
}
#endif

#endif // GOOEY_DROP_SURFACE_INTERNAL_H

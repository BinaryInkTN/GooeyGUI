/**
 * @file gooey_drop_surface.h
 * @brief Header file for the GooeyDropSurface module.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * Provides functionality for creating file drop surfaces, handling file drops,
 * and rendering drop surfaces within a GooeyWindow.
 */

#ifndef GOOEY_DROP_SURFACE_H
#define GOOEY_DROP_SURFACE_H

#include "common/gooey_common.h"

/**
 * @brief Adds a file drop surface to the specified window.
 *
 * Creates a new GooeyDropSurface that allows users to drop files onto it.
 * When a file is dropped, the provided callback function is triggered with
 * the MIME type and file path.
 *
 * @param win The window to add the drop surface to.
 * @param x The x-coordinate of the drop surface's position.
 * @param y The y-coordinate of the drop surface's position.
 * @param width The width of the drop surface.
 * @param height The height of the drop surface.
 * @param default_message The default text displayed on the drop surface.
 * @param callback The function to call when a file is dropped.
 *                 It receives the file's MIME type and path.
 * @return A pointer to the newly created GooeyDropSurface.
 */
GooeyDropSurface *GooeyDropSurface_Add(GooeyWindow *win, int x, int y, int width,
                                       int height, char *default_message,
                                       void (*callback)(char *mime, char *file_path));

/**
 * @brief Clears the content of a drop surface.
 *
 * Resets the drop surface, removing any dropped file information
 * and restoring the default message.
 *
 * @param drop_surface The drop surface to clear.
 */
void GooeyDropSurface_Clear(GooeyDropSurface *drop_surface);

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

#endif // GOOEY_DROP_SURFACE_H

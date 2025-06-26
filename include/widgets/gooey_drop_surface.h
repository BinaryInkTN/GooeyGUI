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

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_DROP_SURFACE)

/**
 * @brief Adds a file drop surface.
 *
 * Creates a new GooeyDropSurface that allows users to drop files onto it.
 * When a file is dropped, the provided callback function is triggered with
 * the MIME type and file path.
 *
 * @param x The x-coordinate of the drop surface's position.
 * @param y The y-coordinate of the drop surface's position.
 * @param width The width of the drop surface.
 * @param height The height of the drop surface.
 * @param default_message The default text displayed on the drop surface.
 * @param callback The function to call when a file is dropped.
 *                 It receives the file's MIME type and path.
 * @return A pointer to the newly created GooeyDropSurface.
 */
GooeyDropSurface *GooeyDropSurface_Create(
    int x, int y, int width, int height,
    char *default_message,
    void (*callback)(char *mime, char *file_path)
);

/**
 * @brief Clears the content of a drop surface.
 *
 * Resets the drop surface, removing any dropped file information
 * and restoring the default message.
 *
 * @param drop_surface The drop surface to clear.
 */
void GooeyDropSurface_Clear(GooeyDropSurface *drop_surface);

#endif // ENABLE_DROP_SURFACE

#ifdef __cplusplus
}
#endif

#endif // GOOEY_DROP_SURFACE_H

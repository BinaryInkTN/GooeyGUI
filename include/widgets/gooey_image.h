/**
 * @file gooey_image.h
 * @brief Image handling functions for the Gooey GUI library.
 * 
 * This file contains functions for adding and drawing images in a Gooey window.
 * 
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 */

#ifndef GOOEY_IMAGE_H
#define GOOEY_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_IMAGE)

/**
 * @brief Adds an image to a Gooey window.
 *
 * This function loads an image from the specified path and adds it to the given window
 * at the specified position and dimensions. A callback function can be provided to handle
 * interactions with the image.
 *
 * @param image_path The file path of the image to load.
 * @param x The x-coordinate of the image's position.
 * @param y The y-coordinate of the image's position.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param callback A callback function to handle interactions with the image (e.g., clicks).
 * @return A pointer to the newly created GooeyImage.
 */
GooeyImage *GooeyImage_Create(
    const char *image_path, int x, int y, int width, int height,
    void (*callback)(void)
);

/**
 * @brief Sets a new image path for the given image widget.
 *
 * @param image The image widget to update.
 * @param image_path The new image file path.
 */
void GooeyImage_SetImage(GooeyImage *image, const char *image_path);

/**
 * @brief Marks the image as needing redrawing (damaged).
 *
 * @param image The image widget to damage.
 */
void GooeyImage_Damage(GooeyImage *image);

#endif // ENABLE_IMAGE

#ifdef __cplusplus
}
#endif

#endif // GOOEY_IMAGE_H

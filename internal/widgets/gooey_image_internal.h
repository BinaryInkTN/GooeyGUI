/**
 * @file gooey_image_internal.h
 * @brief Internal image handling functions for the Gooey GUI library.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * This file contains internal functions for adding and drawing images in a Gooey window.
 */

#ifndef GOOEY_IMAGE_INTERNAL_H
#define GOOEY_IMAGE_INTERNAL_H

#include <stdbool.h>
#include "common/gooey_common.h"

#if (ENABLE_IMAGE)

bool GooeyImage_HandleClick(GooeyWindow *win, int mouseX, int mouseY);

/**
 * @brief Draws all images in a Gooey window.
 *
 * This function renders all images that have been added to the specified window.
 *
 * @param win The Gooey window containing the images to draw.
 */
void GooeyImage_Draw(GooeyWindow* win);

#endif // ENABLE_IMAGE

#endif // GOOEY_IMAGE_INTERNAL_H

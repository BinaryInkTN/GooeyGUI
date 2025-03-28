/**
 * @file gooey_image.h
 * @brief Image handling functions for the Gooey GUI library.
 * @author Yassine Ahmed Ali
 * @copyright GNU General Public License v3.0
 *
 * This file contains functions for adding and drawing images in a Gooey window.
 */

 #ifndef GOOEY_IMAGE_H
 #define GOOEY_IMAGE_H
 
 #include "common/gooey_common.h"
 
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
  */
 GooeyImage* GooeyImage_Create(const char *image_path, int x, int y, int width, int height, void (*callback)(void));
 

 #endif
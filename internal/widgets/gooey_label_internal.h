/**
 * @file gooey_label_internal.h
 * @brief Internal header for the GooeyLabel module.
 *
 * Provides internal functions for creating, modifying, and rendering text labels
 * within a GooeyWindow.
 *
 * @author Yassine Ahmed Ali
 * @license GPL-3.0
 */

#ifndef GOOEY_LABEL_INTERNAL_H
#define GOOEY_LABEL_INTERNAL_H

#include "common/gooey_common.h"

#if (ENABLE_LABEL)

/**
 * @brief Draws all labels within the specified window.
 *
 * This function renders all GooeyLabel elements associated with 
 * the given window.
 *
 * @param win The window containing the labels.
 */
void GooeyLabel_Draw(GooeyWindow *win);

#endif // ENABLE_LABEL

#endif // GOOEY_LABEL_INTERNAL_H

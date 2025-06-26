/**
 * @file gooey_contextmenu_internal.h
 * @brief Internal functions for drawing and handling context menus in Gooey GUI library.
 *        These functions are not part of the public API and are meant for internal rendering
 *        and interaction processing of context menus within a GooeyWindow.
 * @author
 * Yassine Ahmed Ali
 * @copyright
 * GNU General Public License v3.0
 */

#ifndef GOOEY_CONTEXTMENU_INTERNAL_H
#define GOOEY_CONTEXTMENU_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_CONTEXTMENU)

/**
 * @brief Draws the context menu on the specified window.
 *
 * @param win Pointer to the GooeyWindow containing the context menu.
 */
void GooeyContextMenu_Draw(GooeyWindow* win);

/**
 * @brief Handles a click event related to the context menu.
 *
 * This function determines whether the click intersects with any item in
 * the context menu and performs the appropriate action.
 *
 * @param win Pointer to the GooeyWindow containing the context menu.
 */
void GooeyContextMenu_HandleClick(GooeyWindow* win);

#endif // ENABLE_CONTEXTMENU

#ifdef __cplusplus
}
#endif

#endif // GOOEY_CONTEXTMENU_INTERNAL_H

/**
 * @file gooey_debug_internal_overlay.h
 * @brief Internal drawing support for Gooey's debug overlay system.
 *        Used to render debugging visuals within a Gooey window (e.g., widget bounds, IDs).
 * @author
 * Yassine Ahmed Ali
 * @copyright
 * GNU General Public License v3.0
 */

#ifndef GOOEY_DEBUG_INTERNAL_OVERLAY_H
#define GOOEY_DEBUG_INTERNAL_OVERLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_DEBUG_OVERLAY)

/**
 * @brief Draws the debug overlay on the specified window.
 *
 * This function renders debug-specific visual elements such as bounding boxes,
 * widget IDs, or layout guides to aid in GUI development and troubleshooting.
 *
 * @param win Pointer to the GooeyWindow to render the overlay onto.
 */
void GooeyDebugOverlay_Draw(GooeyWindow* win);

#endif // ENABLE_DEBUG_OVERLAY

#ifdef __cplusplus
}
#endif

#endif // GOOEY_DEBUG_INTERNAL_OVERLAY_H

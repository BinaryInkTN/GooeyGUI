#ifndef GOOEY_APPBAR_INTERNAL_H
#define GOOEY_APPBAR_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_APPBAR)

/**
 * @brief Internal function to draw the AppBar widget.
 *
 * This function renders the application bar (AppBar) on the given window.
 * It is intended for internal use only.
 *
 * @param window Pointer to the GooeyWindow where the AppBar should be drawn.
 */
void GooeyAppbar_Internal_Draw(GooeyWindow* window);

#endif // ENABLE_APPBAR

#ifdef __cplusplus
}
#endif

#endif // GOOEY_APPBAR_INTERNAL_H

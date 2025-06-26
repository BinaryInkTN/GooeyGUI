#ifndef GOOEY_WINDOW_INTERNAL_H
#define GOOEY_WINDOW_INTERNAL_H

#include "common/gooey_common.h"

/**
 * @brief Registers a widget internally to the specified GooeyWindow.
 *
 * This function adds a widget to the internal management system of the window,
 * allowing it to be tracked and managed during rendering and events.
 *
 * @param win Pointer to the GooeyWindow.
 * @param widget Pointer to the widget to register.
 */
void GooeyWindow_Internal_RegisterWidget(GooeyWindow *win, void *widget);

#endif /* GOOEY_WINDOW_INTERNAL_H */

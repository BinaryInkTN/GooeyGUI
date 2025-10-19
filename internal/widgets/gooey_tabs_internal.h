#ifndef GOOEY_TABS_INTERNAL_H
#define GOOEY_TABS_INTERNAL_H

#include "common/gooey_common.h"

#if (ENABLE_TABS)

#define TABLINE_ANIMATION_SPEED 1 // ms per frame
#define TABLINE_ANIMATION_STEPS 100 // total animation frames


#define SIDEBAR_ANIMATION_SPEED 1 // ms per frame
#define SIDEBAR_ANIMATION_STEPS 10 // total animation frames

/* Tab UI element dimensions */
#define TAB_HEIGHT           40
#define TAB_WIDTH            150
#define TAB_ELEMENT_HEIGHT   40

/* Padding and text scale for tab labels */
#define TAB_TEXT_PADDING     10
#define TAB_TEXT_SCALE       0.28f

/* Alpha transparency for the tab highlight */
#define TAB_HIGHLIGHT_ALPHA  0.1f

/**
 * @brief Handles click events for tabs within the given window.
 * 
 * @param win The Gooey window containing the tabs.
 * @param mouse_x The x-coordinate of the mouse click.
 * @param mouse_y The y-coordinate of the mouse click.
 * @return true if a tab was clicked, false otherwise.
 */
bool GooeyTabs_HandleClick(GooeyWindow *win, int mouse_x, int mouse_y);

/**
 * @brief Draws all tabs in the specified window.
 *
 * @param win The Gooey window to draw tabs in.
 */
void GooeyTabs_Draw(GooeyWindow *win);

#endif // ENABLE_TABS

#endif /* GOOEY_TABS_INTERNAL_H */

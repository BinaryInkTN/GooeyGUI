#ifndef GOOEY_RADIOBUTTON_INTERNAL_H
#define GOOEY_RADIOBUTTON_INTERNAL_H

#include "common/gooey_common.h"
#if(ENABLE_RADIOBUTTON)
/**
 * @brief Handles radio button click events.
 *
 * This function detects if a click event occurred on a radio button in the
 * given window and invokes the appropriate action.
 *
 * @param win The window containing the radio button.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 *
 * @return True if the radio button was clicked, false otherwise.
 */
bool GooeyRadioButton_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Handles click events for a group of radio buttons.
 *
 * This function allows handling click events in a group of radio buttons,
 * ensuring that only one radio button is selected at a time.
 *
 * @param win The window containing the radio button group.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 *
 * @return True if a radio button in the group was clicked, false otherwise.
 */
bool GooeyRadioButtonGroup_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Draws the radio button group on the window.
 *
 * This function renders all the radio buttons within a group onto the window.
 * It ensures that the visual representation reflects the selected state of each button.
 *
 * @param win The window to draw the radio button group on.
 */
void GooeyRadioButtonGroup_Draw(GooeyWindow *win);
#endif
#endif /* GOOEY_RADIOBUTTON_H */

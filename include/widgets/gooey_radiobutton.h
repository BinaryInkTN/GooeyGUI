#ifndef GOOEY_RADIOBUTTON_H
#define GOOEY_RADIOBUTTON_H

#include "common/gooey_common.h"

/**
 * @brief Adds a radio button to the window.
 *
 * This function adds a radio button with a label to the specified window at a
 * given position. A callback function is invoked when the radio button is selected.
 *
 * @param win The window to add the radio button to.
 * @param x The x-coordinate of the radio button's position.
 * @param y The y-coordinate of the radio button's position.
 * @param label The label of the radio button.
 * @param callback The callback function to call when the radio button is selected.
 *
 * @return A new GooeyRadioButton object.
 */
GooeyRadioButton *GooeyRadioButton_Add(GooeyWindow *win, int x, int y,
                                       char *label,
                                       void (*callback)(bool selected));

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
 * @brief Creates a radio button group widget.
 *
 * This function creates a new radio button group, allowing multiple radio buttons
 * to be grouped together, ensuring that only one button is selected at a time.
 *
 * @param win The window containing the radio button group.
 *
 * @return A pointer to the created GooeyRadioButtonGroup object.
 */
GooeyRadioButtonGroup *GooeyRadioButtonGroup_Create(GooeyWindow *win);

/**
 * @brief Adds a radio button to a radio button group.
 *
 * This function adds a new radio button to a specified radio button group within
 * the given window. A callback is executed when the radio button is selected.
 *
 * @param win The window containing the radio button group.
 * @param group The radio button group to add the button to.
 * @param x The x-coordinate of the radio button.
 * @param y The y-coordinate of the radio button.
 * @param label The label for the radio button.
 * @param callback The callback to execute when the radio button is selected.
 *
 * @return The newly created GooeyRadioButton object.
 */
GooeyRadioButton *GooeyRadioButtonGroup_AddChild(GooeyWindow *win, GooeyRadioButtonGroup *group, int x, int y, const char *label, void (*callback)(bool));

/**
 * @brief Draws the radio button group on the window.
 *
 * This function renders all the radio buttons within a group onto the window.
 * It ensures that the visual representation reflects the selected state of each button.
 *
 * @param win The window to draw the radio button group on.
 */
void GooeyRadioButtonGroup_Draw(GooeyWindow *win);

#endif /* GOOEY_RADIOBUTTON_H */

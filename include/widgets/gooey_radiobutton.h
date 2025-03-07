#ifndef GOOEY_RADIOBUTTON_H
#define GOOEY_RADIOBUTTON_H

#include "common/gooey_common.h"

/**
 * @brief Adds a radio button to the window.
 *
 * @param win The window to add the radio button to.
 * @param x The x-coordinate of the radio button's position.
 * @param y The y-coordinate of the radio button's position.
 * @param label The label of the radio button.
 * @param callback The callback function to call when the radio button is selected.
 * @return A new GooeyRadioButton object.
 */
GooeyRadioButton *GooeyRadioButton_Add(GooeyWindow *win, int x, int y,
                                       char *label,
                                       void (*callback)(bool selected));

/**
 * @brief Handles radio button click events.
 *
 * @param win The window containing the radio button.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 * @return True if the radio button was clicked, false otherwise.
 */
bool GooeyRadioButton_HandleClick(GooeyWindow *win, int x, int y);

bool GooeyRadioButtonGroup_HandleClick(GooeyWindow *win, int x, int y);


/**
 * @brief Creates a radio button group widget.
 *
 * @param win The window containing the radio button group.
 * @return GooeyRadioButtonGroup widget pointer.
 */
GooeyRadioButtonGroup *GooeyRadioButtonGroup_Create(GooeyWindow *win);

/**
 * @brief Adds a radio button to a radio button group.
 *
 * @param group The radio button group to add the button to.
 * @param x The x-coordinate of the radio button.
 * @param y The y-coordinate of the radio button.
 * @param label The label for the radio button.
 * @param callback The callback to execute when the radio button is selected.
 * @return The GooeyRadioButton object.
 */
GooeyRadioButton *GooeyRadioButtonGroup_AddChild(GooeyWindow *win, GooeyRadioButtonGroup *group, int x, int y, const char *label, void (*callback)(bool));

/**
 * @brief Draws the radio button group on the window.
 *
 * @param win The window to draw the radio button group on.
 */
void GooeyRadioButtonGroup_Draw(GooeyWindow *win);



#endif
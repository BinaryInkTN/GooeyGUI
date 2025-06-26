#ifndef GOOEY_RADIOBUTTON_H
#define GOOEY_RADIOBUTTON_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_RADIOBUTTON)

/**
 * @brief Adds a radio button to the window.
 *
 * Creates a radio button with a label at the specified position.
 * The callback is called when the radio button is selected.
 *
 * @param x The x-coordinate of the radio button.
 * @param y The y-coordinate of the radio button.
 * @param label The label of the radio button.
 * @param callback Function called when the button is selected, receiving the selection state.
 * @return Pointer to the newly created GooeyRadioButton.
 */
GooeyRadioButton *GooeyRadioButton_Add(int x, int y,
                                       const char *label,
                                       void (*callback)(bool selected));

/**
 * @brief Handles a click event on radio buttons within a window.
 *
 * Checks if a radio button was clicked and triggers the appropriate callback.
 *
 * @param win The window containing the radio buttons.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 * @return true if a radio button was clicked, false otherwise.
 */
bool GooeyRadioButton_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Handles click events for a group of radio buttons.
 *
 * Ensures that only one radio button in the group is selected at a time.
 *
 * @param win The window containing the radio button group.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 * @return true if a radio button in the group was clicked, false otherwise.
 */
bool GooeyRadioButtonGroup_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Creates a new radio button group.
 *
 * Allows grouping of multiple radio buttons, ensuring single selection.
 *
 * @return Pointer to the created GooeyRadioButtonGroup.
 */
GooeyRadioButtonGroup *GooeyRadioButtonGroup_Create();

/**
 * @brief Adds a radio button to an existing radio button group.
 *
 * Adds a labeled radio button to the group with a callback on selection.
 *
 * @param win The window containing the group.
 * @param group The radio button group to add the button to.
 * @param x The x-coordinate of the radio button.
 * @param y The y-coordinate of the radio button.
 * @param label The label of the radio button.
 * @param callback Function to call when the button is selected.
 * @return Pointer to the newly created GooeyRadioButton.
 */
GooeyRadioButton *GooeyRadioButtonGroup_AddChild(GooeyWindow *win, GooeyRadioButtonGroup *group, int x, int y, const char *label, void (*callback)(bool));

/**
 * @brief Draws all radio buttons in the group on the window.
 *
 * Renders the visual state of each radio button, reflecting selection.
 *
 * @param win The window on which to draw the group.
 */
void GooeyRadioButtonGroup_Draw(GooeyWindow *win);

#endif // ENABLE_RADIOBUTTON

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* GOOEY_RADIOBUTTON_H */

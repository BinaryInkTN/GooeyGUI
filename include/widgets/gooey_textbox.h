#ifndef GOOEY_TEXTBOX_H
#define GOOEY_TEXTBOX_H

#include "common/gooey_common.h"
#include <stdbool.h>

/**
 * @brief Adds a textbox to the window.
 *
 * This function creates and adds a textbox to the specified window at the given position
 * with a defined size. The textbox can display placeholder text, and a callback function
 * is triggered whenever the text inside the textbox changes.
 *
 * @param x The x-coordinate of the textbox's position.
 * @param y The y-coordinate of the textbox's position.
 * @param width The width of the textbox.
 * @param height The height of the textbox.
 * @param placeholder The placeholder text for the textbox.
 * @param onTextChanged The callback function to call when the text changes.
 *
 * @return A new GooeyTextbox object.
 */
GooeyTextbox *GooeyTextBox_Create( int x, int y, int width,
                               int height, char *placeholder, bool is_password, void (*onTextChanged)(char *text));

/**
 * @brief Draws the textbox on the window.
 *
 * This function renders the textbox on the specified window. It should be called after
 * the textbox's state has been updated to reflect the changes visually.
 *
 * @param win The window to draw the textbox on.
 */
void GooeyTextbox_Draw(GooeyWindow *win);

/**
 * @brief Handles textbox click events.
 *
 * This function is responsible for detecting and processing click events on the textbox.
 * If the textbox is clicked, it will enter an active state.
 *
 * @param win The window containing the textbox.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 *
 * @return True if the textbox was clicked, false otherwise.
 */
bool GooeyTextbox_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Handles key press events for the textbox.
 *
 * This function processes key events, allowing the user to type text inside the textbox.
 * The textbox's content is updated in real-time as the user types.
 *
 * @param win The window containing the textbox.
 * @param event The key event to handle.
 */
void GooeyTextbox_HandleKeyPress(GooeyWindow *win, void *event);

/**
 * @brief Gets the text of the textbox.
 *
 * This function retrieves the current content of the textbox.
 *
 * @param textbox The textbox to retrieve text from.
 *
 * @return The current text of the textbox.
 */
const char *GooeyTextbox_GetText(GooeyTextbox *textbox);

/**
 * @brief Sets the text of the textbox.
 *
 * This function sets the textbox's content to the specified text.
 *
 * @param textbox The textbox to set the text for.
 * @param text The new text to set in the textbox.
 */
void GooeyTextbox_SetText(GooeyTextbox *textbox, const char *text);

#endif /* GOOEY_TEXTBOX_H */

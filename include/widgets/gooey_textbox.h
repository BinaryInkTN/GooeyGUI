#ifndef GOOEY_TEXTBOX_H
#define GOOEY_TEXTBOX_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_TEXTBOX)
#include <stdbool.h>

/**
 * @brief Creates and adds a textbox widget.
 *
 * Creates a textbox at the specified position with given dimensions. The textbox
 * can optionally mask input for password fields. A callback is triggered when
 * the text changes.
 *
 * @param x The x-coordinate of the textbox.
 * @param y The y-coordinate of the textbox.
 * @param width The width of the textbox.
 * @param height The height of the textbox.
 * @param placeholder The placeholder text to show when empty.
 * @param is_password Whether the textbox input should be masked (e.g., password).
 * @param onTextChanged Callback invoked when the text changes; receives the new text.
 *
 * @return Pointer to the created GooeyTextbox object.
 */
GooeyTextbox *GooeyTextBox_Create(int x, int y, int width, int height,
                                  char *placeholder, bool is_password,
                                  void (*onTextChanged)(char *text));

/**
 * @brief Draws the textbox on the specified window.
 *
 * Renders the textbox UI, reflecting its current state and contents.
 *
 * @param win The window to draw the textbox on.
 */
void GooeyTextbox_Draw(GooeyWindow *win);

/**
 * @brief Handles click events on the textbox.
 *
 * Detects clicks on the textbox area and activates the textbox for input if clicked.
 *
 * @param win The window containing the textbox.
 * @param x The x-coordinate of the click.
 * @param y The y-coordinate of the click.
 *
 * @return True if the textbox was clicked and activated, false otherwise.
 */
bool GooeyTextbox_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Processes key press events for the textbox.
 *
 * Updates the textbox content based on keyboard input.
 *
 * @param win The window containing the textbox.
 * @param event Pointer to the key event to handle.
 */
void GooeyTextbox_HandleKeyPress(GooeyWindow *win, void *event);

/**
 * @brief Gets the current text from the textbox.
 *
 * @param textbox The textbox to retrieve text from.
 *
 * @return Pointer to the current text content.
 */
const char *GooeyTextbox_GetText(GooeyTextbox *textbox);

/**
 * @brief Sets the text content of the textbox.
 *
 * @param textbox The textbox to update.
 * @param text The new text to set.
 */
void GooeyTextbox_SetText(GooeyTextbox *textbox, const char *text);

#endif // ENABLE_TEXTBOX

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* GOOEY_TEXTBOX_H */

#ifndef GOOEY_TEXTBOX_H
#define GOOEY_TEXTBOX_H


#include "gooey_event_internal.h"
#include "gooey_widgets_internal.h"
#include <stdbool.h>

/**
 * @brief Adds a textbox to the window.
 *
 * @param win The window to add the textbox to.
 * @param x The x-coordinate of the textbox's position.
 * @param y The y-coordinate of the textbox's position.
 * @param width The width of the textbox.
 * @param height The height of the textbox.
 * @param placeholder The placeholder text for the textbox.
 * @param onTextChanged The callback function to call when the text changes.
 * @return A new GooeyTextbox object.
 */
GooeyTextbox *GooeyTextBox_Add(GooeyWindow *win, int x, int y, int width,
                               int height, char *placeholder, void (*onTextChanged)(char *text));

/**
 * @brief Draws the textbox on the window.
 *
 * @param win The window to draw the textbox on.
 */
void GooeyTextbox_Draw(GooeyWindow *win);

/**
 * @brief Handles textbox click events.
 *
 * @param win The window containing the textbox.
 * @param x The x-coordinate of the click event.
 * @param y The y-coordinate of the click event.
 * @return True if the textbox was clicked, false otherwise.
 */
bool GooeyTextbox_HandleClick(GooeyWindow *win, int x, int y);

/**
 * @brief Handles key press events for the textbox.
 *
 * @param win The window containing the textbox.
 * @param key_event The key event to handle.
 */
void GooeyTextbox_HandleKeyPress(GooeyWindow *win, GooeyEvent *event);

/**
 * @brief Gets the text of the textbox.
 *
 * @param textbox The textbox to retrieve text from.
 * @return The current text of the textbox.
 */
const char *GooeyTextbox_GetText(GooeyTextbox *textbox);

/**
 * @brief Sets the text of the textbox.
 *
 * @param textbox The textbox to set the text for.
 * @param text The new text to set in the textbox.
 */
void GooeyTextbox_SetText(GooeyTextbox *textbox, const char *text);


#endif
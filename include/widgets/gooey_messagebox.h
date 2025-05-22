/**
 * @file gooey_messagebox.h
 * @brief Header file for creating and managing message boxes in Gooey.
 *
 * Provides functions to create, configure, and display modal message boxes
 * with various types of alerts and a callback to handle user input.
 *
 * @author Yassine Ahmed Ali
 * @license GPL-3.0
 */

 #ifndef GOOEY_MESSAGEBOX_H
 #define GOOEY_MESSAGEBOX_H
 
 #include "common/gooey_common.h"

 /**
  * @brief Creates a message box window.
  *
  * This function creates a modal message box with a specified title, message,
  * type, and a callback function to handle user interaction.
  *
  * @param title The title of the message box.
  * @param message The message to display in the message box.
  * @param type The type of the message box (e.g., INFO, WARNING, ERROR, QUESTION).
  * @param callback A function pointer to handle user input (e.g., button click).
  *                 The callback receives an integer representing the selected option.
  * @return A pointer to the created message box window.
  */
 GooeyWindow *GooeyMessageBox_Create(const char *title, const char *message, MSGBOX_TYPE type, void (*callback)(int option));
 
 /**
  * @brief Displays the specified message box window.
  *
  * This function makes the message box visible on the screen. The message box must
  * have been created using `GooeyMessageBox_Create`.
  *
  * @param msgBoxWindow A pointer to the message box window to display.
  */
 void GooeyMessageBox_Show(GooeyWindow *msgBoxWindow);
 
 #endif // GOOEY_MESSAGEBOX_H
 
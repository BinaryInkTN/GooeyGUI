/*
 Copyright (c) 2024 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

 #ifndef GOOEY_EVENT_INTERNAL_H
 #define GOOEY_EVENT_INTERNAL_H

 /**
  * @brief Enumeration of event types.
  *
  * This enum defines the types of events that can be handled in the Gooey
  * event system. Each event corresponds to a different action, such as mouse
  * interactions, key presses, window events, etc.
  */
 typedef enum
 {
     GOOEY_EVENT_CLICK_PRESS,  /**< Mouse button press event */
     GOOEY_EVENT_CLICK_RELEASE, /**< Mouse button release event */
     GOOEY_EVENT_MOUSE_MOVE, /**< Mouse movement event */
     GOOEY_EVENT_MOUSE_SCROLL, /**< Mouse scroll event */
     GOOEY_EVENT_KEY_PRESS, /**< Keyboard key press event */
     GOOEY_EVENT_KEY_RELEASE, /**< Keyboard key release event */
     GOOEY_EVENT_WINDOW_CLOSE, /**< Window close event */
     GOOEY_EVENT_EXPOSE, /**< Redraw request event (window needs to be redrawn) */
     GOOEY_EVENT_RESIZE, /**< Window resize event */
     GOOEY_EVENT_REDRAWREQ, /**< Redraw request event for widget or area */
     GOOEY_EVENT_DROP, /**< File drop event (drag and drop) */
     GOOEY_EVENT_RESET
 } GooeyEventType;

 /**
  * @brief Data structure for mouse events.
  *
  * This struct holds the x and y coordinates for mouse-related events such as
  * mouse clicks, movement, and scrolling.
  */
 typedef struct
 {
     int x; /**< The x-coordinate of the mouse position */
     int y; /**< The y-coordinate of the mouse position */
 } GooeyMouseData;

 /**
  * @brief Data structure for key press events.
  *
  * This struct holds information about key press events, including the key
  * state and the key value (typically a string representation of the key).
  */
 typedef struct
 {
     unsigned int state; /**< The state of the key (e.g., pressed or released) */
     char value[20]; /**< The value of the key (e.g., the key name or symbol) */
     unsigned long keycode; /**< The keycode of the key */
 } GooeyKeyPressData;

 /**
  * @brief Data structure for file drop events.
  *
  * This struct holds information about file drop events, including the
  * state of the drop, the MIME type, the file path, and the position
  * where the file was dropped.
  */
 typedef struct
 {
     unsigned int state; /**< The state of the drop (e.g., start or end) */
     char mime[20]; /**< The MIME type of the dropped file */
     char file_path[1024]; /**< The file path of the dropped file */
     int drop_x; /**< The x-coordinate where the file was dropped */
     int drop_y; /**< The y-coordinate where the file was dropped */
 } GooeyDropData;

 /**
  * @brief Event structure holding various event data.
  *
  * This struct represents a generic event and can hold data for multiple
  * event types, such as mouse events, key events, and file drop events.
  */
 typedef struct
 {
     GooeyEventType type; /**< The type of event (e.g., click, mouse move, etc.) */

     // Mouse-related event data
     GooeyMouseData click; /**< Mouse click event data */
     GooeyMouseData mouse_move; /**< Mouse movement event data */
     GooeyMouseData mouse_scroll; /**< Mouse scroll event data */

     // Key press event data
     GooeyKeyPressData key_press; /**< Key press event data */

     // File drop event data
     GooeyDropData drop_data; /**< File drop event data */
 } GooeyEvent;

 #endif /* GOOEY_EVENT_INTERNAL_H */

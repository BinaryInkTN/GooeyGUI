/**
 * @file gooey_dropdown.h
 * @brief Header file for the GooeyDropdown module.
 *
 * Provides functions to create, handle, and render dropdown menus 
 * within a GooeyWindow.
 *
 * @author Yassine Ahmed Ali
 * @license GPL-3.0
 */

#ifndef GOOEY_DROPDOWN_H
#define GOOEY_DROPDOWN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_DROPDOWN)

/**
 * @brief Adds a dropdown menu to the specified window.
 *
 * This function creates a new dropdown menu with the given list of options.
 *
 * @param x The x-coordinate of the dropdown's position.
 * @param y The y-coordinate of the dropdown's position.
 * @param width The width of the dropdown menu.
 * @param height The height of the dropdown menu.
 * @param options The array of strings representing the dropdown menu options.
 * @param num_options The total number of options in the dropdown.
 * @param callback The function to call when an option is selected. 
 *                 It receives the index of the selected option.
 * @return A pointer to the newly created GooeyDropdown object.
 */
GooeyDropdown *GooeyDropdown_Create(
    int x, int y, int width, int height,
    const char **options, int num_options,
    void (*callback)(int selected_index)
);

void GooeyDropdown_Update(GooeyDropdown *dropdown, const char **new_options, int new_num_options);

#endif // ENABLE_DROPDOWN

#ifdef __cplusplus
}
#endif

#endif // GOOEY_DROPDOWN_H

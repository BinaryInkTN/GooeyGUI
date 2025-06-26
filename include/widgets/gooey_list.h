#ifndef GOOEY_LIST_H
#define GOOEY_LIST_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_LIST)

/**
 * @brief Creates a list widget.
 *
 * Creates a new list widget and attaches it to the given window.
 *
 * @param x The x-coordinate of the list widget's position.
 * @param y The y-coordinate of the list widget's position.
 * @param width The width of the list widget.
 * @param height The height of the list widget.
 * @param callback The function to be called when an item is selected.
 *                 It receives the index of the selected item.
 * @return A pointer to the newly created GooeyList object.
 */
GooeyList *GooeyList_Create(int x, int y, int width, int height, void (*callback)(int index));

/**
 * @brief Adds an item to the specified list widget.
 *
 * @param list The list widget to which the item will be added.
 * @param title The title of the list item.
 * @param description The description of the list item.
 */
void GooeyList_AddItem(GooeyList *list, const char *title, const char *description);

/**
 * @brief Clears all items from the specified list widget.
 *
 * Removes all list items from the provided list widget.
 *
 * @param list The list widget to be cleared.
 */
void GooeyList_ClearItems(GooeyList *list);

/**
 * @brief Toggles the visibility of the separator in a list widget.
 *
 * Enables or disables the visual separator between list items.
 *
 * @param list The list widget.
 * @param state `true` to show the separator, `false` to hide it.
 */
void GooeyList_ShowSeparator(GooeyList *list, bool state);

/**
 * @brief Updates the title and description of a specific item in the list.
 *
 * @param list The list widget.
 * @param item_index The index of the item to update.
 * @param title The new title for the list item.
 * @param description The new description for the list item.
 */
void GooeyList_UpdateItem(GooeyList *list, size_t item_index, const char *title, const char *description);

#endif // ENABLE_LIST

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GOOEY_LIST_H

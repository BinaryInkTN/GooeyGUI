#ifndef GOOEY_TABS_H
#define GOOEY_TABS_H

#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_TABS)

/**
 * @brief Creates a tab widget.
 *
 * Creates a GooeyTabs widget at the specified position with given dimensions.
 * If `is_sidebar` is true, the tabs behave as a sidebar.
 *
 * @param x The x-coordinate of the tabs widget.
 * @param y The y-coordinate of the tabs widget.
 * @param width The width of the tabs widget.
 * @param height The height of the tabs widget.
 * @param is_sidebar Whether the tabs should behave as a sidebar.
 *
 * @return Pointer to the created GooeyTabs widget.
 */
GooeyTabs *GooeyTabs_Create(int x, int y, int width, int height, bool is_sidebar);

/**
 * @brief Inserts a new tab with the specified name.
 *
 * Adds a new tab to the tab widget.
 *
 * @param tab_widget The tab widget to add a tab to.
 * @param tab_name The name/title of the new tab.
 */
void GooeyTabs_InsertTab(GooeyTabs *tab_widget, char *tab_name);

/**
 * @brief Adds a widget to a specified tab.
 *
 * Associates a widget with a tab identified by tab_id.
 *
 * @param window The parent window containing the tabs.
 * @param tabs The tabs widget.
 * @param tab_id The index of the tab to add the widget to.
 * @param widget Pointer to the widget to add.
 */
void GooeyTabs_AddWidget(GooeyWindow *window, GooeyTabs *tabs, size_t tab_id, void *widget);

/**
 * @brief Sets the active tab.
 *
 * Switches the active tab to the one specified by tab_id.
 *
 * @param tabs The tabs widget.
 * @param tab_id The index of the tab to activate.
 */
void GooeyTabs_SetActiveTab(GooeyTabs *tabs, size_t tab_id);

/**
 * @brief Opens the sidebar tabs (if tabs are in sidebar mode).
 *
 * @param tabs_widget The tabs widget.
 */
void GooeyTabs_Sidebar_Open(GooeyTabs *tabs_widget);

/**
 * @brief Closes the sidebar tabs (if tabs are in sidebar mode).
 *
 * @param tabs_widget The tabs widget.
 */
void GooeyTabs_Sidebar_Close(GooeyTabs *tabs_widget);

#endif // ENABLE_TABS

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* GOOEY_TABS_H */

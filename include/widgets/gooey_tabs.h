#ifndef GOOEY_TABS_H
#define GOOEY_TABS_H

#include "common/gooey_common.h"
#if(ENABLE_TABS)
GooeyTabs *GooeyTabs_Create( int x, int y, int width, int height, bool is_sidebar);
void GooeyTabs_InsertTab(GooeyTabs *tab_widget, char *tab_name);
void GooeyTabs_AddWidget(GooeyWindow* window, GooeyTabs* tabs, size_t tab_id, void *widget);
void GooeyTabs_SetActiveTab(GooeyTabs* tabs, size_t tab_id);
void GooeyTabs_Sidebar_Open(GooeyTabs *tabs_widget);
void GooeyTabs_Sidebar_Close(GooeyTabs *tabs_widget);
#endif
#endif
#ifndef GOOEY_TABS_H
#define GOOEY_TABS_H

#include "common/gooey_common.h"
#if(ENABLE_TABS)
GooeyTabs *GooeyTabs_Create( int x, int y, int width, int height);
void GooeyTabs_InsertTab(GooeyTabs *tab_widget, char *tab_name);
void GooeyTabs_AddWidget(GooeyTabs* tabs, size_t tab_id, void *widget);
void GooeyTabs_SetActiveTab(GooeyTabs* tabs, size_t tab_id);
#endif
#endif
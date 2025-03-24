#ifndef GOOEY_TABS_H
#define GOOEY_TABS_H

#include "common/gooey_common.h"

GooeyTabs *GooeyTabs_Add( int x, int y, int width, int height);

GooeyTab *GooeyTabs_InsertTab(GooeyTabs *tab_widget, char *tab_name);

void GooeyTabs_AddWidget(GooeyTab *tab, void *widget);

void GooeyTabs_SetActiveTab(GooeyTabs* tabs, GooeyTab *active_tab);

#endif
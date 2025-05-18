#ifndef GOOEY_WIDGET_H
#define GOOEY_WIDGET_H

#include "common/gooey_common.h"

void GooeyWidget_MakeVisible(void* widget, bool state);
void GooeyWidget_MoveTo(void* widget, int x, int y);
void GooeyWidget_Resize(void* widget, int w, int h);

#endif
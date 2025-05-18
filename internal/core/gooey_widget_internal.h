#ifndef GOOEY_WIDGET_INTERNAL
#define GOOEY_WIDGET_INTERNAL

#include <stdbool.h>

void GooeyWidget_MakeVisible_Internal(void* widget, bool state);
void GooeyWidget_MoveTo_Internal(void* widget, int x, int y);
void GooeyWidget_Resize_Internal(void* widget, int w, int h);


#endif
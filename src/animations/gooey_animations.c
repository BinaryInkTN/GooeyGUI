#include "animations/gooey_animations.h"
#include "animations/gooey_animations_internal.h"
void GooeyAnimation_TranslateX(GooeyWidget *widget, int start, int end, float speed)
{
    GooeyAnimation_TranslateX_Internal(widget, start, end, speed);
}
void GooeyAnimation_TranslateY(GooeyWidget *widget, int start, int end, float speed)
{
    GooeyAnimation_TranslateY_Internal(widget, start, end, speed);
}
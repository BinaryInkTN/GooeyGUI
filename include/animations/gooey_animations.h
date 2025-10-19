#ifndef GOOEY_ANIMATIONS_H
#define GOOEY_ANIMATIONS_H

#include "core/gooey_widget_internal.h"
#include "core/gooey_timers_internal.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void GooeyAnimation_TranslateX(GooeyWidget *widget, int start, int end, float speed);
void GooeyAnimation_TranslateY(GooeyWidget *widget, int start, int end, float speed);

#ifdef __cplusplus
}
#endif

#endif // GOOEY_ANIMATIONS_H
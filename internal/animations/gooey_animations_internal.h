#ifndef GOOEY_ANIMATIONS_INTERNAL_H
#define GOOEY_ANIMATIONS_INTERNAL_H

#include "core/gooey_widget_internal.h"
#include "core/gooey_timers_internal.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void GooeyAnimation_TranslateX_Internal(GooeyWidget *widget, int start, int end, float speed);
void GooeyAnimation_TranslateY_Internal(GooeyWidget *widget, int start, int end, float speed);

#ifdef __cplusplus
}
#endif

#endif 
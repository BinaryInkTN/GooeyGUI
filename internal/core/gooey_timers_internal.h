#ifndef GOOEY_TIMERS_INTERNAL_H
#define GOOEY_TIMERS_INTERNAL_H

#include "common/gooey_common.h"
#include <stdint.h>

GooeyTimer *GooeyTimer_Create_Internal(void);
void GooeyTimer_SetCallback_Internal(uint64_t time, GooeyTimer *timer, void (*callback)(void *user_data), void *user_data);
void GooeyTimer_Destroy_Internal(GooeyTimer *timer);
void GooeyTimer_Stop_Internal(GooeyTimer *timer);
#endif
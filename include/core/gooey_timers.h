#ifndef GOOEY_TIMERS_H
#define GOOEY_TIMERS_H

#include "common/gooey_common.h"
#include <stdint.h>

GooeyTimer *GooeyTimer_Create(void);
void GooeyTimer_SetCallback(uint64_t time, GooeyTimer *timer, void (*callback)(void *user_data), void *user_data);
void GooeyTimer_Destroy(GooeyTimer *timer);
void GooeyTimer_Stop(GooeyTimer *timer);

#endif
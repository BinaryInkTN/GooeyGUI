#include "core/gooey_timers.h"
#include "core/gooey_timers_internal.h"
#include "backends/gooey_backend_internal.h"

GooeyTimer *GooeyTimer_Create()
{
    return GooeyTimer_Create_Internal();
}

void GooeyTimer_SetCallback(uint64_t time, GooeyTimer *timer, void (*callback)(void *user_data), void *user_data)
{
    GooeyTimer_SetCallback_Internal(time, timer, callback, user_data);
}
void GooeyTimer_Destroy(GooeyTimer *timer)
{
    GooeyTimer_Destroy_Internal(timer);
}

void GooeyTimer_Stop(GooeyTimer *timer)
{
    GooeyTimer_Stop_Internal(timer);
}

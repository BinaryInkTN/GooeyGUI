#include "core/gooey_timers.h"
#include "backends/gooey_backend_internal.h"

GooeyTimer *GooeyTimer_Create()
{
    GooeyTimer *timer = active_backend->CreateTimer();
    return timer;
}

void GooeyTimer_SetCallback(uint64_t time, GooeyTimer *timer, void (*callback)(void* user_data), void *user_data)
{
    active_backend->SetTimerCallback(time, timer, callback, user_data);
}
void GooeyTimer_Destroy(GooeyTimer *timer)
{
    active_backend->DestroyTimer(timer);
}

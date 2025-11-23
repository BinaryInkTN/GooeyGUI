#include "core/gooey_event.h"
#include "backends/gooey_backend_internal.h"
GooeyEvent* GooeyEvents_GetEvents(GooeyWindow* win)
{
    return (GooeyEvent*) active_backend->GetEvents(win);
}
#include "widgets/gooey_appbar_internal.h"
#include "logger/pico_logger_internal.h"
#include "backends/gooey_backend_internal.h"
#if(ENABLE_APPBAR)

void GooeyAppbar_Internal_Draw(GooeyWindow* window)
{
    if(!window || !window->appbar) return;

    
    //active_backend->FillRectangle(0, 0, window->width, APPBAR_HEIGHT, window->active_theme->primary, window->creation_id, false, 0.0f);
    active_backend->DrawText(10, APPBAR_HEIGHT/2 + active_backend->GetTextHeight(window->appbar->appname, strlen(window->appbar->appname)) / 2, window->appbar->appname, window->active_theme->neutral, 0.4f, window->creation_id, NULL);
}

#endif
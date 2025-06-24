#include "widgets/gooey_appbar.h"
#include "logger/pico_logger_internal.h"
#if(ENABLE_APPBAR)

void GooeyAppbar_Set(GooeyWindow* window, const char* app_name)
{
    if(!window)
    {
        LOG_ERROR("Couldn't set Appbar, window is invalid.");
        return;
    }


    window->appbar = (GooeyAppbar*) calloc(1, sizeof(GooeyAppbar));
    if(!window->appbar)
    {
        LOG_ERROR("Couldn't allocate memory for appbar.");
        return;
    }

    window->appbar->appname = app_name;
}
#endif
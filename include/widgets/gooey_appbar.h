#ifndef GOOEY_APPBAR_H
#define GOOEY_APPBAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gooey_common.h"

#if (ENABLE_APPBAR)
void GooeyAppbar_Set(GooeyWindow* window, const char* app_name);
#endif

#ifdef __cplusplus
}
#endif

#endif // GOOEY_APPBAR_H

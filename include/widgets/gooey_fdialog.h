#ifndef GOOEY_FDIALOG_H
#define GOOEY_FDIALOG_H

#include "common/gooey_common.h"
#if (ENABLE_FDIALOG)
#ifdef __cplusplus

extern "C"
{

#endif

    void GooeyFDialog_Open(
        const char *path,
        const char **filters,
        int filter_count,
        void (*callback)(const char *));
    void GooeyFDialog_Close(GooeyWindow *win);


#ifdef __cplusplus
}
#endif
#endif
#endif

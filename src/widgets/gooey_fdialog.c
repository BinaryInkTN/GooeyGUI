#include "backends/gooey_backend_internal.h"
#include "widgets/gooey_fdialog.h"
#if (ENABLE_FDIALOG)

void GooeyFDialog_Open(
    const char *path,
    const char **filters,
    int filter_count,
    void (*callback)(const char *))
{
    active_backend->OpenFileDialog(path, filters, filter_count, callback);
}

#endif
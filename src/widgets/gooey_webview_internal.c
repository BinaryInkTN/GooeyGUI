#include "widgets/gooey_webview_internal.h"
#include "backends/gooey_backend_internal.h"


#if (ENABLE_WEBVIEW)

void GooeyWebview_Internal_Draw(GooeyWindow* window) {
    if (!window) {
        return;
    }

    if (active_backend) {
        active_backend->DrawWebview(0, 0, 0, 0, 0, NULL);
    }
}   

#endif
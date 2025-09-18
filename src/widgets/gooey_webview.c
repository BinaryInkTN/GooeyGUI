#include "widgets/gooey_webview.h"
#include "logger/pico_logger_internal.h"

#if (ENABLE_WEBVIEW)

GooeyWebview* GooeyWebview_Create(GooeyWindow* window, const char* url, int x, int y, int width, int height)
{
    if(!window)
    {
        LOG_ERROR("GooeyWebview_Create: window is NULL");
        return NULL;
    }   

    if(window->webview_count >= MAX_WIDGETS)
    {
        LOG_ERROR("GooeyWebview_Create: Maximum number of webviews reached");
        return;
    }

    GooeyWebview* webview = (GooeyWebview*)calloc(1, sizeof(GooeyWebview));

    if(!webview)
    {
        LOG_ERROR("GooeyWebview_Create: Failed to allocate memory for webview");
        return NULL;
    }

    webview->core.type = WIDGET_WEBVIEW;
    webview->core.x = x;
    webview->core.y = y;
    webview->core.width = width;
    webview->core.height = height;
    webview->core.is_visible = true;
    strncpy(webview->url, url, sizeof(webview->url) - 1);
    webview->needs_refresh = true;
    webview->core.sprite = NULL; // Initialize sprite to NULL, will be created during drawing

    return webview;
}
#endif

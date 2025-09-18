#ifndef GOOEY_WEBVIEW_H
#define GOOEY_WEBVIEW_H
#include "common/gooey_common.h"

#ifdef __cplusplus
extern "C" {
#endif


GooeyWebview* GooeyWebview_Create(GooeyWindow* window, const char* url, int x, int y, int width, int height);


#endif
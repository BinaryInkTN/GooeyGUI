#ifndef GOOEY_KEYBOARD_INTERNAL_H
#define GOOEY_KEYBOARD_INTERNAL_H

#include "common/gooey_common.h"
#ifdef __cplusplus
extern "C" {
#endif

#if (ENABLE_VIRTUAL_KEYBOARD)
void GooeyVK_Internal_Draw(GooeyWindow* win);
void GooeyVK_Internal_Show(GooeyVK *vk, size_t textbox_id);
void GooeyVK_Internal_Hide(GooeyVK* vk);
void GooeyVK_Internal_HandleClick(GooeyWindow *win, int mouseX, int mouseY);
char* GooeyVK_Internal_GetText(GooeyVK *vk);
void GooeyVK_Internal_SetText(GooeyVK* vk, const char* text);
#endif
#ifdef __cplusplus
} // extern "C"
#endif
#endif
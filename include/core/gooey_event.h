
#ifndef GOOEY_EVENT_H
#define GOOEY_EVENT_H

typedef struct GooeyEvent GooeyEvent;
typedef struct GooeyWindow GooeyWindow;

GooeyEvent* GooeyEvents_GetEvents(GooeyWindow* window);

#endif
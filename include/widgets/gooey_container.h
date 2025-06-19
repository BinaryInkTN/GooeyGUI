//
// Created by overflow on 6/10/2025.
//

#ifndef GOOEY_CONTAINER_H
#define GOOEY_CONTAINER_H
#include "common/gooey_common.h"

#if(ENABLE_CONTAINER)
GooeyContainers* GooeyContainer_Create(int x, int y, int width, int height);
void GooeyContainer_InsertContainer(GooeyContainers *container);
void GooeyContainer_AddWidget(GooeyWindow* window, GooeyContainers* container, size_t container_id, void *widget);
void GooeyContainer_SetActiveContainer(GooeyContainers *container, size_t container_id);


#endif
#endif //GOOEY_CONTAINER_H

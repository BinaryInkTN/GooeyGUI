#ifndef GOOEY_PROGRESSBAR_H
#define GOOEY_PROGRESSBAR_H

#include "common/gooey_common.h"
#if(ENABLE_PROGRESSBAR)
GooeyProgressBar *GooeyProgressBar_Create(int x, int y, int width, int height, long initial_value);

void GooeyProgressBar_Update(GooeyProgressBar *progressbar, long new_value);
#endif
#endif
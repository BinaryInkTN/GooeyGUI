#ifndef GOOEY_METER_H
#define GOOEY_METER_H

#include "common/gooey_common.h"

GooeyMeter *GooeyMeter_Create(int x, int y, int width, int height, long initial_value, const char *label);
void GooeyMeter_Update(GooeyMeter *meter, long new_value);

#endif
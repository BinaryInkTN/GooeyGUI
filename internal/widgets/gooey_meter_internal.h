#ifndef GOOEY_METER_INTERNAL_H
#define GOOEY_METER_INTERNAL_H

#include "common/gooey_common.h"

#if (ENABLE_METER)

/**
 * @brief Draws all meter widgets in the specified window.
 *
 * @param win The Gooey window containing meter widgets to draw.
 */
void GooeyMeter_Draw(GooeyWindow *win);

#endif // ENABLE_METER

#endif // GOOEY_METER_INTERNAL_H

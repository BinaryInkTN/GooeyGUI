#ifndef GOOEY_PROGRESSBAR_INTERNAL_H
#define GOOEY_PROGRESSBAR_INTERNAL_H

#include "common/gooey_common.h"

#if (ENABLE_PROGRESSBAR)

/**
 * @brief Draws all progress bars in the specified Gooey window.
 *
 * @param win Pointer to the Gooey window containing the progress bars.
 */
void GooeyProgressBar_Draw(GooeyWindow *win);

#endif // ENABLE_PROGRESSBAR

#endif // GOOEY_PROGRESSBAR_INTERNAL_H

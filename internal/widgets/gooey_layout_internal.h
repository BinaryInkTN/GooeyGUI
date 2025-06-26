#ifndef GOOEY_LAYOUT_INTERNAL_H
#define GOOEY_LAYOUT_INTERNAL_H

#include "common/gooey_common.h"

#if (ENABLE_LAYOUT)

/**
 * @brief Builds or updates the given layout.
 * 
 * This function arranges the components or widgets inside the layout
 * according to the layout rules.
 *
 * @param layout Pointer to the GooeyLayout to build.
 */
void GooeyLayout_Build(GooeyLayout *layout);

#endif // ENABLE_LAYOUT

#endif // GOOEY_LAYOUT_INTERNAL_H

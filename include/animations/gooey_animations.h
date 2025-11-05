#ifndef GOOEY_ANIMATIONS_H
#define GOOEY_ANIMATIONS_H

#include "core/gooey_widget_internal.h"
#include "core/gooey_timers_internal.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Animates a widget's horizontal position
 *
 * Creates a smooth translation animation along the X-axis from start to end
 * position. The animation speed controls the duration and easing of the movement.
 *
 * @param[in,out] widget Pointer to the widget to animate
 * @param[in] start Starting X position in pixels
 * @param[in] end Target X position in pixels
 * @param[in] speed Animation speed (pixels per second or normalized 0.0-1.0)
 *
 * @note The widget must be properly initialized and part of the widget tree.
 * @warning Avoid calling this function multiple times on the same widget
 *          before previous animations complete.
 *
 * @see GooeyAnimation_TranslateY
 */
void GooeyAnimation_TranslateX(GooeyWidget *widget, int start, int end, float speed);

/**
 * @brief Animates a widget's vertical position
 *
 * Creates a smooth translation animation along the Y-axis from start to end
 * position. The animation speed controls the duration and easing of the movement.
 *
 * @param[in,out] widget Pointer to the widget to animate
 * @param[in] start Starting Y position in pixels
 * @param[in] end Target Y position in pixels
 * @param[in] speed Animation speed (pixels per second or normalized 0.0-1.0)
 *
 * @note The widget must be properly initialized and part of the widget tree.
 * @warning Avoid calling this function multiple times on the same widget
 *          before previous animations complete.
 *
 * @see GooeyAnimation_TranslateX
 */
void GooeyAnimation_TranslateY(GooeyWidget *widget, int start, int end, float speed);
#ifdef __cplusplus
}
#endif

#endif // GOOEY_ANIMATIONS_H

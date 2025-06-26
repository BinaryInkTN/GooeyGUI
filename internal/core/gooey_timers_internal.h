#ifndef GOOEY_TIMERS_INTERNAL_H
#define GOOEY_TIMERS_INTERNAL_H

#include "common/gooey_common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates an internal timer instance.
 *
 * @return Pointer to a newly allocated GooeyTimer object.
 */
GooeyTimer *GooeyTimer_Create_Internal(void);

/**
 * @brief Sets a timer callback to be triggered after a specific time.
 *
 * @param time The delay in milliseconds before the callback is invoked.
 * @param timer Pointer to the GooeyTimer.
 * @param callback Function to call when the timer elapses.
 * @param user_data User data passed to the callback.
 */
void GooeyTimer_SetCallback_Internal(uint64_t time, GooeyTimer *timer,
                                     void (*callback)(void *user_data),
                                     void *user_data);

/**
 * @brief Destroys the internal timer and releases associated resources.
 *
 * @param timer Pointer to the timer to destroy.
 */
void GooeyTimer_Destroy_Internal(GooeyTimer *timer);

/**
 * @brief Stops the internal timer without destroying it.
 *
 * @param timer Pointer to the timer to stop.
 */
void GooeyTimer_Stop_Internal(GooeyTimer *timer);

#ifdef __cplusplus
}
#endif

#endif /* GOOEY_TIMERS_INTERNAL_H */

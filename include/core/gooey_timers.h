#ifndef GOOEY_TIMERS_H
#define GOOEY_TIMERS_H

#include "common/gooey_common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates a new timer object.
 *
 * @return Pointer to the created GooeyTimer.
 */
GooeyTimer *GooeyTimer_Create(void);

/**
 * @brief Sets a callback to be called after a specified time.
 *
 * @param time The delay in milliseconds before the callback is executed.
 * @param timer Pointer to the timer to configure.
 * @param callback The function to call when the timer expires.
 * @param user_data User data passed to the callback.
 */
void GooeyTimer_SetCallback(uint64_t time, GooeyTimer *timer, void (*callback)(void *user_data), void *user_data);

/**
 * @brief Destroys the timer and frees its resources.
 *
 * @param timer Pointer to the timer to destroy.
 */
void GooeyTimer_Destroy(GooeyTimer *timer);

/**
 * @brief Stops the timer if it is running.
 *
 * @param timer Pointer to the timer to stop.
 */
void GooeyTimer_Stop(GooeyTimer *timer);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* GOOEY_TIMERS_H */

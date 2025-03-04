#ifndef GOOEY_SIGNALS_H
#define GOOEY_SIGNALS_H

#include "utils/logger/gooey_logger_internal.h"

/* ============ Signaling system ==============*/

typedef void (*GooeySignal_CallbackFunction)(void *context, void *data);

typedef struct GooeySignal_Slot
{
    GooeySignal_CallbackFunction callback;
    void *context;
    struct GooeySignal_Slot *next;
} GooeySignal_Slot;

typedef struct
{
    GooeySignal_Slot *slots;
} GooeySignal;

/**
 * @brief Creates a new signal object.
 *
 * Signals are used to manage event handling and callbacks. This function initializes
 * a new signal object.
 *
 * @return GooeySignal The created signal object.
 */
GooeySignal GooeySignal_Create(void);

/**
 * @brief Links a callback function to a signal.
 *
 * This function connects a signal to a specific callback function. When the signal
 * is emitted, the callback function will be executed.
 *
 * @param signal A pointer to the signal to which the callback is linked.
 * @param callback The callback function to execute when the signal is emitted.
 * @param context A user-defined context pointer passed to the callback.
 */
void GooeySignal_Link(GooeySignal *signal, GooeySignal_CallbackFunction callback, void *context);

/**
 * @brief Emits a signal.
 *
 * This function triggers the signal, invoking all linked callbacks in the order
 * they were added.
 *
 * @param signal A pointer to the signal to emit.
 * @param data A void pointer to the data.
 */
void GooeySignal_Emit(GooeySignal *signal, void *data);

/**
 * @brief Unlinks all callbacks from a signal.
 *
 * Removes all callback functions linked to the specified signal, effectively
 * clearing its event listeners.
 *
 * @param signal A pointer to the signal to modify.
 */
void GooeySignal_UnLinkAll(GooeySignal *signal);

#endif
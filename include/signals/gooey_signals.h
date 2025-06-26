#ifndef GOOEY_SIGNALS_H
#define GOOEY_SIGNALS_H

/* ============ Signaling system ==============*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback function type for signal events.
 *
 * This function type is used for the callback that is executed when a signal
 * is emitted. The callback receives a context and data passed from the emitter.
 *
 * @param context The user-defined context passed when linking the callback.
 * @param data The data passed when emitting the signal.
 */
typedef void (*GooeySignal_CallbackFunction)(void *context, void *data);

/**
 * @brief A slot representing a linked callback in the signal system.
 *
 * A slot contains a callback function, its associated context, and a link
 * to the next slot in a potentially linked list of callbacks.
 */
typedef struct GooeySignal_Slot
{
    GooeySignal_CallbackFunction callback; /**< The callback function to be executed */
    void *context;                         /**< The user-defined context to be passed to the callback */
    struct GooeySignal_Slot *next;        /**< Pointer to the next slot in the list */
} GooeySignal_Slot;

/**
 * @brief A signal that can emit events to all linked slots.
 *
 * A signal contains a list of slots. Each slot corresponds to a callback
 * function that will be executed when the signal is emitted.
 */
typedef struct
{
    GooeySignal_Slot *slots; /**< List of slots (callbacks) associated with the signal */
} GooeySignal;

/**
 * @brief Creates a new signal object.
 *
 * Signals are used to manage event handling and callbacks. This function initializes
 * a new signal object with no slots.
 *
 * @return GooeySignal The created signal object with no linked callbacks.
 */
GooeySignal GooeySignal_Create(void);

/**
 * @brief Links a callback function to a signal.
 *
 * This function connects a signal to a specific callback function. When the signal
 * is emitted, the callback function will be executed. The callback is linked
 * to the signal and can be triggered when the signal is emitted.
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
 * they were added. Each callback receives the specified data.
 *
 * @param signal A pointer to the signal to emit.
 * @param data A void pointer to the data passed to all linked callbacks.
 */
void GooeySignal_Emit(GooeySignal *signal, void *data);

/**
 * @brief Unlinks all callbacks from a signal.
 *
 * Removes all callback functions linked to the specified signal, effectively
 * clearing its event listeners.
 *
 * @param signal A pointer to the signal from which callbacks should be unlinked.
 */
void GooeySignal_UnLinkAll(GooeySignal *signal);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* GOOEY_SIGNALS_H */

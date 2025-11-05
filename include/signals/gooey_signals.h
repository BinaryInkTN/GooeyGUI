#ifndef GOOEY_SIGNALS_H
#define GOOEY_SIGNALS_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Creation
#define GooeySignal_var(type, value) GooeySignal_Create(sizeof(type), &(type){value})
#define GooeySignal_string(value) GooeySignal_CreateString(value)

// Access
#define GooeySignal_get(s, type) (*(type*)GooeySignal_GetValue(s))
#define GooeySignal_set(s, value) GooeySignal_SetValue(s, &(typeof(value)){value})

// Reactivity
#define GooeySignal_watch(s, callback) GooeySignal_Watch(s, (GooeySignal_Callback)callback, NULL)
#define GooeySignal_unwatch(s, callback) GooeySignal_Unwatch(s, (GooeySignal_Callback)callback, NULL)

// Effects
#define GooeyReact_effect(fn) GooeyReact_CreateEffect((GooeyReact_EffectFn)fn, NULL)

// Batch updates
#define GooeySignal_batch(...) do { \
    GooeySignal_BatchBegin(); \
    __VA_ARGS__; \
    GooeySignal_BatchEnd(); \
} while(0)

// Cleanup
#define GooeySignal_free(s) GooeySignal_Destroy(s)

/* ============ PUBLIC TYPES ============ */

typedef struct GooeySignal GooeySignal;
typedef struct GooeyReactEffect GooeyReactEffect;

// Callback types
typedef void (*GooeySignal_Callback)(void* context, void* new_value);
typedef void (*GooeyReact_EffectFn)(void* context);

/* ============ PUBLIC API ============ */

// Core signal system
GooeySignal* GooeySignal_Create(size_t size, const void* initial_value);
GooeySignal* GooeySignal_CreateString(const char* initial_value);
void* GooeySignal_GetValue(GooeySignal* signal);
bool GooeySignal_SetValue(GooeySignal* signal, const void* new_value);
void GooeySignal_Destroy(GooeySignal* signal);

// Watch system
bool GooeySignal_Watch(GooeySignal* signal, GooeySignal_Callback callback, void* context);
bool GooeySignal_Unwatch(GooeySignal* signal, GooeySignal_Callback callback, void* context);

// Effects system
GooeyReactEffect* GooeyReact_CreateEffect(GooeyReact_EffectFn effect_fn, void* context);
void GooeyReact_DestroyEffect(GooeyReactEffect* effect);

// Batch updates
void GooeySignal_BatchBegin(void);
void GooeySignal_BatchEnd(void);

#ifdef __cplusplus
}
#endif

#endif // GOOEY_SIGNALS_H

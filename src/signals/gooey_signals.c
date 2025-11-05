#include "signals/gooey_signals.h"
#include "logger/pico_logger_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============ INTERNAL STRUCTURES ============

typedef struct GooeySignalWatch {
    GooeySignal_Callback callback;
    void* context;
    struct GooeySignalWatch* next;
} GooeySignalWatch;

typedef struct GooeyEffectDependency {
    GooeySignal* signal;
    struct GooeyEffectDependency* next;
} GooeyEffectDependency;

struct GooeySignal {
    void* value;
    size_t size;
    GooeySignalWatch* watches;
    bool is_string;
};

struct GooeyReactEffect {
    GooeyReact_EffectFn effect_fn;
    void* context;
    GooeyEffectDependency* dependencies;
    bool is_running;
};

// ============ GLOBAL STATE ============

static int batch_depth = 0;
static GooeyReactEffect* current_effect = NULL;

// Batch system
static struct {
    GooeySignal** signals;
    void** data;
    size_t count;
    size_t capacity;
} batch_queue = {0};

// ============ FORWARD DECLARATIONS ============

static void effect_signal_changed(void* context, void* new_value);
static void effect_track_dependency(GooeySignal* signal);
static void effect_cleanup_dependencies(GooeyReactEffect* effect);

// ============ CORE SIGNAL IMPLEMENTATION ============

GooeySignal* GooeySignal_Create(size_t size, const void* initial_value) {
    GooeySignal* signal = calloc(1, sizeof(GooeySignal));
    if (!signal) return NULL;

    signal->value = malloc(size);
    if (!signal->value) {
        free(signal);
        return NULL;
    }

    if (initial_value) {
        memcpy(signal->value, initial_value, size);
    } else {
        memset(signal->value, 0, size);
    }

    signal->size = size;
    signal->is_string = false;
    signal->watches = NULL;

    return signal;
}

GooeySignal* GooeySignal_CreateString(const char* initial_value) {
    if (!initial_value) return NULL;

    size_t len = strlen(initial_value) + 1;
    GooeySignal* signal = GooeySignal_Create(len, initial_value);
    if (signal) {
        signal->is_string = true;
    }
    return signal;
}

void* GooeySignal_GetValue(GooeySignal* signal) {
    if (!signal) return NULL;

    if (current_effect) {
        effect_track_dependency(signal);
    }

    return signal->value;
}

static void signal_emit(GooeySignal* signal, void* data) {
    if (!signal || !signal->watches) return;

    if (batch_depth > 0) {
        if (batch_queue.count >= batch_queue.capacity) {
            batch_queue.capacity = batch_queue.capacity == 0 ? 32 : batch_queue.capacity * 2;
            batch_queue.signals = realloc(batch_queue.signals, sizeof(GooeySignal*) * batch_queue.capacity);
            batch_queue.data = realloc(batch_queue.data, sizeof(void*) * batch_queue.capacity);
        }

        for (size_t i = 0; i < batch_queue.count; i++) {
            if (batch_queue.signals[i] == signal) {
                batch_queue.data[i] = data; // Update data
                return;
            }
        }

        batch_queue.signals[batch_queue.count] = signal;
        batch_queue.data[batch_queue.count] = data;
        batch_queue.count++;
        return;
    }

    GooeySignalWatch* watch = signal->watches;
    while (watch) {
        if (watch->callback) {
            watch->callback(watch->context, data);
        }
        watch = watch->next;
    }
}

bool GooeySignal_SetValue(GooeySignal* signal, const void* new_value) {
    if (!signal || !new_value) return false;

    bool changed = false;

    if (signal->is_string) {
        const char* current_str = (const char*)signal->value;
        const char* new_str = (const char*)new_value;
        if (strcmp(current_str, new_str) != 0) {
            changed = true;
        }
    } else {
        changed = true;
    }

    if (changed) {
        if (signal->is_string) {
            size_t new_len = strlen((const char*)new_value) + 1;
            if (new_len > signal->size) {
                void* new_ptr = realloc(signal->value, new_len);
                if (!new_ptr) return false;
                signal->value = new_ptr;
                signal->size = new_len;
            }
            strcpy((char*)signal->value, (const char*)new_value);
        } else {
            memcpy(signal->value, new_value, signal->size);
        }

        signal_emit(signal, signal->value);
    }

    return changed;
}

void GooeySignal_Destroy(GooeySignal* signal) {
    if (!signal) return;

    GooeySignalWatch* watch = signal->watches;
    while (watch) {
        GooeySignalWatch* next = watch->next;
        free(watch);
        watch = next;
    }

    free(signal->value);
    free(signal);
}

// ============ WATCH SYSTEM ============

bool GooeySignal_Watch(GooeySignal* signal, GooeySignal_Callback callback, void* context) {
    if (!signal || !callback) return false;

    GooeySignalWatch* watch = malloc(sizeof(GooeySignalWatch));
    if (!watch) return false;

    watch->callback = callback;
    watch->context = context;
    watch->next = signal->watches;
    signal->watches = watch;

    return true;
}

bool GooeySignal_Unwatch(GooeySignal* signal, GooeySignal_Callback callback, void* context) {
    if (!signal || !callback) return false;

    GooeySignalWatch** prev = &signal->watches;
    GooeySignalWatch* current = signal->watches;

    while (current) {
        if (current->callback == callback && current->context == context) {
            *prev = current->next;
            free(current);
            return true;
        }
        prev = &current->next;
        current = current->next;
    }

    return false;
}

// ============ EFFECT DEPENDENCY TRACKING ============

static void effect_track_dependency(GooeySignal* signal) {
    if (!current_effect || !signal) return;

    GooeyEffectDependency* dep = current_effect->dependencies;
    while (dep) {
        if (dep->signal == signal) return;
        dep = dep->next;
    }

    dep = malloc(sizeof(GooeyEffectDependency));
    dep->signal = signal;
    dep->next = current_effect->dependencies;
    current_effect->dependencies = dep;

    GooeySignal_Watch(signal, effect_signal_changed, current_effect);
}

static void effect_cleanup_dependencies(GooeyReactEffect* effect) {
    if (!effect) return;

    GooeyEffectDependency* dep = effect->dependencies;
    while (dep) {
        GooeyEffectDependency* next = dep->next;
        GooeySignal_Unwatch(dep->signal, effect_signal_changed, effect);
        free(dep);
        dep = next;
    }
    effect->dependencies = NULL;
}

static void effect_signal_changed(void* context, void* new_value) {
    GooeyReactEffect* effect = (GooeyReactEffect*)context;
    if (effect->is_running) {
        return;
    }

    LOG_INFO("DEBUG: Effect dependency changed, re-running effect\n");

    GooeyReactEffect* prev_effect = current_effect;
    current_effect = effect;
    effect->is_running = true;

    effect_cleanup_dependencies(effect);

    if (effect->effect_fn) {
        effect->effect_fn(effect->context);
    }

    effect->is_running = false;
    current_effect = prev_effect;
}

// ============ EFFECT SYSTEM ============

GooeyReactEffect* GooeyReact_CreateEffect(GooeyReact_EffectFn effect_fn, void* context) {
    if (!effect_fn) return NULL;

    GooeyReactEffect* effect = malloc(sizeof(GooeyReactEffect));
    if (!effect) return NULL;

    effect->effect_fn = effect_fn;
    effect->context = context;
    effect->dependencies = NULL;
    effect->is_running = false;

    GooeyReactEffect* prev_effect = current_effect;
    current_effect = effect;
    effect->is_running = true;

    effect_fn(context);

    effect->is_running = false;
    current_effect = prev_effect;

    return effect;
}

void GooeyReact_DestroyEffect(GooeyReactEffect* effect) {
    if (!effect) return;

    effect_cleanup_dependencies(effect);
    free(effect);
}

// ============ BATCH SYSTEM ============

static void batch_process_all(void) {
    for (size_t i = 0; i < batch_queue.count; i++) {
        GooeySignal* signal = batch_queue.signals[i];
        void* data = batch_queue.data[i];

        GooeySignalWatch* watch = signal->watches;
        while (watch) {
            if (watch->callback) {
                watch->callback(watch->context, data);
            }
            watch = watch->next;
        }
    }

    batch_queue.count = 0;
}

void GooeySignal_BatchBegin(void) {
    batch_depth++;
}

void GooeySignal_BatchEnd(void) {
    if (batch_depth > 0) {
        batch_depth--;
    }

    if (batch_depth == 0) {
        batch_process_all();
    }
}
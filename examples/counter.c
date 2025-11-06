#include <Gooey/gooey.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void increment_fn(void *user_data);
void update_ui(void *context);

typedef struct
{
    GooeyLabel *label;
    GooeySignal *signal;
} AppData;

void increment_fn(void *user_data)
{
    AppData *app = (AppData *)user_data;

    int *count = (int *)GooeySignal_GetValue(app->signal);

    (*count)++;

    GooeySignal_SetValue(app->signal, count);
}

void update_ui(void *context)
{
    AppData *app = (AppData *)context;
    int *count = (int *)GooeySignal_GetValue(app->signal);

    char text[64];
    snprintf(text, sizeof(text), "Counter: %d", *count);

    GooeyLabel_SetText(app->label, text);
}

int main()
{
    Gooey_Init();

    GooeyWindow *window = GooeyWindow_Create("Simple Counter", 400, 300, true);

    GooeyLayout *vertical_layout = GooeyLayout_Create(LAYOUT_GRID, 0, 0, 400, 300);

    GooeySignal *signal = GooeySignal_Create(sizeof(int), 0);
    int initial_value = 10;
    GooeySignal_SetValue(signal, &initial_value);
    GooeyLabel *label = GooeyLabel_Create("Counter: 0", 0.3f, 0, 0);

    AppData *app = malloc(sizeof(AppData));
    app->label = label;
    app->signal = signal;

    GooeyButton *button = GooeyButton_Create("Click Me!", 0, 0, 100, 30, increment_fn, app);

    GooeyReactEffect *effect_hook = GooeyReact_CreateEffect(update_ui, app);

    GooeyLayout_AddChild(window, vertical_layout, label);
    GooeyLayout_AddChild(window, vertical_layout, button);

    GooeyWindow_RegisterWidget(window, vertical_layout);

    GooeyWindow_Run(1, window);

    free(app);
    GooeyWindow_Cleanup(1, window);
    GooeySignal_Destroy(signal);
    GooeyReact_DestroyEffect(effect_hook);
    return 0;
}

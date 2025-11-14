#include <Gooey/gooey.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../internal/backends/gooey_backend_internal.h"

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
  /*



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

te)
      GooeySignal_Destroy(signal);
  GooeyReact_DestroyEffect(effect_hook);
      free(app);

  */

  GooeyWindow *window = GooeyWindow_Create("Simple Counter", 800, 600, true);

  active_backend->DrawRectangle(10, 10, 200, 200, 0xFF0000, 10.0f, 0, false, 0.0f, NULL);
 // active_backend->FillRectangle(100, 100, 200, 200, 0xFF0000, 0, false, 0.0f, NULL);
  //active_backend->FillArc(10, 10, 100, 100, 0, 180, 0, NULL);
  
  //  active_backend->StopTimer(NULL);
   //active_backend->DrawRectangle(100, 100, 100, 100, 0x00FF00, 1.0f, 0, false, 0.0f, NULL);
  //active_backend->DrawText(10, 10, "test", 0x0000FF, 0.3f, 0, NULL);
  active_backend->RenderBatch(0);
  GooeyWindow_Run(1, window);

  GooeyWindow_Cleanup(1, window);

  return 0;
}

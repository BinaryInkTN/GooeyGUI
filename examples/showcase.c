

#include "gooey.h"
#include <stdio.h>

GooeyWindow *win;
bool state = false;
GooeyProgressBar* progressbar;
int progress_percentage = 0;
void enable_overlay()
{
  state = !state;
  GooeyWindow_EnableDebugOverlay(win, state);
}

void progress()
{
  progress_percentage += 10;
  GooeyProgressBar_Update(progressbar, progress_percentage);

}

int main()
{
  Gooey_Init();

  win = GooeyWindow_Create("window", 800, 600, false);

  //GooeyWindow_SetContinuousRedraw(win);
  //GooeyWindow_SetTheme(win, GooeyWindow_LoadTheme("dark.json"));

  //GooeyMenu *menu = GooeyMenu_Set(win);

//  GooeyMenuChild *file_child = GooeyMenu_AddChild(win, "File");

//  GooeyMenuChild_AddElement(file_child, "New", progress);
 // GooeyMenuChild_AddElement(file_child, "Open", NULL);
//  GooeyMenuChild_AddElement(file_child, "Save", NULL);
  //GooeyMenuChild_AddElement(file_child, "Exit", NULL);

////  GooeyMenuChild *settings_child = GooeyMenu_AddChild(win, "Settings");

 // GooeyMenuChild_AddElement(settings_child, "New", NULL);
//  GooeyMenuChild_AddElement(settings_child, "Open", NULL);
//  GooeyMenuChild_AddElement(settings_child, "Save", NULL);
 // GooeyMenuChild_AddElement(settings_child, "Exit", NULL);

 // GooeyMenuChild *about_child = GooeyMenu_AddChild(win, "Debug");

 // GooeyMenuChild_AddElement(about_child, "Enable Overlay", enable_overlay);

  //GooeyWindow_SetContinuousRedraw(win);
//  GooeyWindow_MakeVisible(win, false);
//  GooeyWindow_MakeResizable(win, false);
GooeyImage *test_img = GooeyImage_Create("", 0, 0, 20, 20, NULL);
  GooeyButton *button_0 = GooeyButton_Create("Button", 198, 92, 100, 30, NULL);
  GooeyList *list_1 = GooeyList_Create(58, 186, 200, 200, NULL);
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  GooeyList_AddItem(list_1, "test", "test");
  const char *options_dropdown_2[2] = {"test", "test"};
  GooeyDropdown *dropdown_2 = GooeyDropdown_Create(69, 58, 100, 30, options_dropdown_2, 2, NULL);
  GooeySlider *slider_3 = GooeySlider_Create(231, 40, 150, 0, 100, true, NULL);
  progressbar = GooeyProgressBar_Create(40, 40, 100, 30, progress_percentage);
  GooeyMeter *meter = GooeyMeter_Create( 20, 450, 100, 100, 50, "Meter", "cube.png");
  float x_data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
  float y_data[4] = {-3.0f, 4.0f, 5.0f, 6.0f};
 
  GooeyPlotData plot_data = {
    .x_data = x_data,
    .y_data = y_data,
    .data_count = 4,
    .x_step = 1.0f,
    .y_step = 1.0f,
  };
  GooeyPlot *plot = GooeyPlot_Create(GOOEY_PLOT_LINE, &plot_data, 300, 100, 400, 400);
  GooeyProgressBar* progressbar = GooeyProgressBar_Create(300, 550, 200, 40, progress_percentage);
  GooeyProgressBar_Update(progressbar, 50);

//  GooeyWindow_RegisterWidget(win, progressbar);
 //GooeyWindow_RegisterWidget(win, test_img);
 //GooeyWindow_RegisterWidget(win, progressbar);
 //GooeyWindow_RegisterWidget(win, meter);
  GooeyWindow_RegisterWidget(win, button_0);
  GooeyWindow_RegisterWidget(win, list_1);
  GooeyWindow_RegisterWidget(win, dropdown_2);
  GooeyWindow_RegisterWidget(win, slider_3);

  GooeyWindow_Run(1, win);

  GooeyWindow_Cleanup(1, win);

  return 0;
}
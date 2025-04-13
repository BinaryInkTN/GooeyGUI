#include "gooey.h"
#include <stdio.h>
#include <stdbool.h>

GooeyWindow *dashboard;
GooeyMeter *light_meter, *garbage_meter;
GooeyPlot *light_plot;
GooeyButton *toggle_light;
GooeyList *alert_list;

bool light_on = false;

float hours[24] = {
    0, 0, 0, 0, 0,
    10, 30, 70, 80, 85,
    90, 95, 100, 100, 100,
    90, 80, 70, 60, 50,
    30, 10, 0, 0};
float light_levels[24] = {0};

GooeyPlotData plot_data = {
    .x_data = hours,
    .y_data = light_levels,
    .data_count = 4,
    .x_step = 1.0f,
    .y_step = 1.0f,
    .title = "24h Light Levels"};
void toggle_light_callback()
{
    light_on = !light_on;
}
gthread_t thread_id;

void *update_meter()
{

    while (true)
    {
        GooeyMeter_Update(light_meter, rand() % 101);
        printf("UPDATE  %d \n", rand() % 101);
        sleep(1);
    }
}

void initialize_dashboard()
{

    srand(time(NULL));

    dashboard = GooeyWindow_Create("Smart Lighting Dashboard", 500, 450, true);
    GooeyWindow_SetTheme(dashboard, GooeyWindow_LoadTheme("dark.json"));
   // GooeyWindow_EnableDebugOverlay(dashboard, true);
    GooeyWindow_SetContinuousRedraw(dashboard);

    for (int i = 0; i < 24; i++)
    {
        hours[i] = (float)i;
    }

    const int left_col = 20;
    const int meter_size = 120;

    light_meter = GooeyMeter_Create(left_col, 60, meter_size, meter_size, 80, "light");

    toggle_light = GooeyButton_Create("Toggle Light", left_col, 160, meter_size, 30, toggle_light_callback);

    garbage_meter = GooeyMeter_Create(left_col, 250, meter_size, meter_size, 30, "garbage");

    const int middle_col = 180;
    const int plot_width = 300;
    const int plot_height = 150;

    light_plot = GooeyPlot_Create(GOOEY_PLOT_LINE, &plot_data,
                                  middle_col, 60, plot_width, plot_height);

    const int right_col = 500;
    const int alert_width = 120;
    const int alert_height = 350;

    alert_list = GooeyList_Create(middle_col, 240, plot_width, 180, NULL);
    GooeyList_AddItem(alert_list, "Light Status", light_on ? "ON" : "OFF");
    GooeyList_AddItem(alert_list, "Garbage Level", "35% full");
    GooeyList_AddItem(alert_list, "Last Check", "2 mins ago");
    GooeyList_AddItem(alert_list, "System Status", "All normal");

    // Top bar
    GooeyCanvas *canvas = GooeyCanvas_Create(0, 0, 650, 40);
    GooeyCanvas_DrawRectangle(canvas, 0, 0, 650, 40, dashboard->active_theme->primary, true);
    // Title
    GooeyLabel *title = GooeyLabel_Create("Smart Lighting", 0.3f, 25, 25);
    //    GooeyLabel_SetColor(title, 0xFFFFFF);

    GooeyWindow_RegisterWidget(dashboard, title);

    GooeyWindow_RegisterWidget(dashboard, canvas);
    GooeyWindow_RegisterWidget(dashboard, light_meter);
    GooeyWindow_RegisterWidget(dashboard, garbage_meter);

    GooeyWindow_RegisterWidget(dashboard, light_plot);
    GooeyWindow_RegisterWidget(dashboard, alert_list);
    glps_thread_create(&thread_id, NULL, update_meter, NULL);
}

int main()
{
    Gooey_Init();

    initialize_dashboard();
    GooeyWindow_Run(1, dashboard);
    glps_thread_join(thread_id, NULL);
    GooeyWindow_Cleanup(1, dashboard);
    return 0;
}
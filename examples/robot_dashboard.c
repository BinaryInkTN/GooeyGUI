#include "gooey.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

GooeyWindow *dashboard;
GooeyProgressBar *battery_bar;
GooeyButton *theme_toggle;
GooeyList *alert_list;
GooeyTheme *dark_theme;
GooeyImage *camera_feed;
gthread_t thread_camera_feed;
GooeySignal frame_update_call;
bool dark_mode = false;

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
    .data_count = 5,
    .x_step = 1.0f,
    .y_step = 1.0f,
    .title = "24h Light Levels"};

void toggle_dark_mode()
{
    dark_mode = !dark_mode;
    GooeyWindow_SetTheme(dashboard, dark_mode ? dark_theme : NULL);
    GooeyButton_SetText(theme_toggle, dark_mode ? "Dark Mode ON" : "Dark Mode OFF");
}

void robot_control_callback(const char *action)
{
    printf("[Robot Action] %s\n", action);
  //  GooeyList_UpdateItem(alert_list, 3, "Last Command", action);
}

void cb_gripper_open() { robot_control_callback("Gripper Open"); }
void cb_gripper_close() { robot_control_callback("Gripper Close"); }
void cb_wrist_tilt_up() { robot_control_callback("Wrist Tilt Up"); }
void cb_wrist_tilt_down() { robot_control_callback("Wrist Tilt Down"); }
void cb_wrist_rot_left() { robot_control_callback("Wrist Rotate Left"); }
void cb_wrist_rot_right() { robot_control_callback("Wrist Rotate Right"); }
void cb_elbow_up() { robot_control_callback("Elbow Up"); }
void cb_elbow_down() { robot_control_callback("Elbow Down"); }
void cb_shoulder_up() { robot_control_callback("Shoulder Up"); }
void cb_shoulder_down() { robot_control_callback("Shoulder Down"); }
void cb_base_left() { robot_control_callback("Base Rotate Left"); }
void cb_base_right() { robot_control_callback("Base Rotate Right"); }

void *camera_feed_thread(void *data)
{
    while (1)
    {
       // GooeySignal_Emit(&frame_update_call, "camera");
       if(camera_feed) GooeyImage_Damage(camera_feed);
  
       sleep(1);
    }
    return NULL;
}


void signal_camera_feed()
{
    GooeyImage_Damage(camera_feed);
}

void initialize_dashboard()
{
    srand(time(NULL));

    dark_theme = GooeyTheme_LoadFromFile("dark.json");
    if (!dark_theme)
    {
        fprintf(stderr, "Failed to load theme\n");
        exit(1);
    }

    dashboard = GooeyWindow_Create("Robot Dashboard", 520, 660, true);
    if (!dashboard)
    {
        fprintf(stderr, "Failed to create dashboard\n");
        exit(1);
    }

    frame_update_call = GooeySignal_Create();
    GooeySignal_Link(&frame_update_call, signal_camera_feed, "camera");

    for (int i = 0; i < 24; i++)
        hours[i] = (float)i;

    const int left_col = 20;
    const int meter_size = 120;

    battery_bar = GooeyProgressBar_Create(left_col + 130, 350, meter_size, 20, 75);
    if (!battery_bar)
    {
        fprintf(stderr, "Failed to create battery progress bar\n");
        exit(1);
    }

    GooeyLabel *battery_label = GooeyLabel_Create("Battery Level:", 0.3f, left_col + 20, 365);
    if (!battery_label)
    {
        fprintf(stderr, "Failed to create battery label\n");
        exit(1);
    }

    const int middle_col = 180;

    camera_feed = GooeyImage_Create("camera.jpg", left_col, 60, 480, 270, NULL);
    if (!camera_feed)
    {
        fprintf(stderr, "Failed to create camera feed\n");
        exit(1);
    }

    theme_toggle = GooeyButton_Create("Dark Mode OFF", 370, 5, 120, 30, toggle_dark_mode);
    if (!theme_toggle)
    {
        fprintf(stderr, "Failed to create theme toggle button\n");
        exit(1);
    }

    int x = 40, y = 400, w = 250, h = 30, gap = 40;

    GooeyButton *btns[] = {
        GooeyButton_Create("Gripper Open", x, y, w + 80, h, cb_gripper_open),
        GooeyButton_Create("Gripper Close", x + w + 10, y, w + 80, h, cb_gripper_close),

        GooeyButton_Create("Wrist Up", x, y + gap, w + 80, h, cb_wrist_tilt_up),
        GooeyButton_Create("Wrist Down", x + w + 10, y + gap, w + 80, h, cb_wrist_tilt_down),

        GooeyButton_Create("Wrist Left", x, y + 2 * gap, w + 80, h, cb_wrist_rot_left),
        GooeyButton_Create("Wrist Right", x + w + 10, y + 2 * gap, w + 80, h, cb_wrist_rot_right),

        GooeyButton_Create("Elbow Up", x, y + 3 * gap, w + 80, h, cb_elbow_up),
        GooeyButton_Create("Elbow Down", x + w + 10, y + 3 * gap, w + 80, h, cb_elbow_down),

        GooeyButton_Create("Shoulder Up", x, y + 4 * gap, w + 80, h, cb_shoulder_up),
        GooeyButton_Create("Shoulder Down", x + w + 10, y + 4 * gap, w + 80, h, cb_shoulder_down),

        GooeyButton_Create("Base Left", x, y + 5 * gap, w + 80, h, cb_base_left),
        GooeyButton_Create("Base Right", x + w + 10, y + 5 * gap, w + 80, h, cb_base_right),
    };

    for (int i = 0; i < sizeof(btns) / sizeof(btns[0]); i++)
    {
        if (!btns[i])
        {
            fprintf(stderr, "Button creation failed\n");
            exit(1);
        }
    }

    dark_theme->primary = 0x1877F2;
    dashboard->active_theme->primary = dark_theme->primary;

    GooeyCanvas *canvas = GooeyCanvas_Create(0, 0, 520, 600);
    if (!canvas)
    {
        fprintf(stderr, "Failed to create canvas\n");
        exit(1);
    }

    GooeyCanvas_DrawRectangle(canvas, 0, 0, 520, 40, dashboard->active_theme->primary, true);
    GooeyLabel *title = GooeyLabel_Create("Robot Dashboard", 0.3f, 25, 25);
    if (!title)
    {
        fprintf(stderr, "Failed to create title label\n");
        exit(1);
    }

    GooeyWindow_RegisterWidget(dashboard, canvas);
    GooeyWindow_RegisterWidget(dashboard, title);
    GooeyWindow_RegisterWidget(dashboard, theme_toggle);
    GooeyWindow_RegisterWidget(dashboard, battery_bar);
    GooeyWindow_RegisterWidget(dashboard, battery_label);
    GooeyWindow_RegisterWidget(dashboard, camera_feed);

    for (int i = 0; i < sizeof(btns) / sizeof(btns[0]); i++)
        GooeyWindow_RegisterWidget(dashboard, btns[i]);
}

int main()
{
    Gooey_Init();

    int ret = glps_thread_create(&thread_camera_feed, NULL, camera_feed_thread, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to create camera feed thread\n");
        exit(1);
    }

    initialize_dashboard();

    if (dashboard)
    {
        GooeyWindow_MakeResizable(dashboard, false);
        GooeyWindow_Run(1, dashboard);
        glps_thread_join(thread_camera_feed, NULL);

        GooeyWindow_Cleanup(1, dashboard);
    }

    return 0;
}

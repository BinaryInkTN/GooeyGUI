#include "gooey.h"

void dropdown_callback(int selected_index);
void button_callback();
void close_callback();
void switch_callback(bool state);

GooeyTabs *mainTabs;
GooeyMeter *meter;
bool systemEnabled = true;

void dropdown_callback(int selected_index)
{
    printf("Selection changed to index: %d\n", selected_index);
}

void button_callback()
{
    printf("Button pressed");

    static int meterValue = 0;
    meterValue = (meterValue + 10) % 100;
    GooeyMeter_Update(meter, meterValue);
}

void close_callback()
{
    printf("Close requested");
}

void switch_callback(bool state)
{
    systemEnabled = state;
printf("System %s\n", state ? "enabled" : "disabled");
}

void setup()
{
    

    Gooey_Init();

    GooeyWindow *win = GooeyWindow_Create("TFT Control Panel", 480, 320, false);
    win->vk->is_shown = true;

    GooeyMenu *menu = GooeyMenu_Set(win);
    GooeyMenuChild *file_menu = GooeyMenu_AddChild(win, "File");
    GooeyMenuChild_AddElement(file_menu, "Save", NULL);
    GooeyMenuChild_AddElement(file_menu, "Open", NULL);
    GooeyMenuChild_AddElement(file_menu, "Close", close_callback);

    mainTabs = GooeyTabs_Create(10, 35, 460, 275, false);
    mainTabs->is_open = true;

    GooeyTabs_InsertTab(mainTabs, "Dashboard");

    meter = GooeyMeter_Create(20, 20, 180, 180, 0, "System Load", NULL);
    GooeyLabel *sysLabel = GooeyLabel_Create("System Status:", 0.28f, 250, 30);
    GooeySwitch *sysSwitch = GooeySwitch_Create(250, 60, true, false, switch_callback);
    GooeyButton *refreshBtn = GooeyButton_Create("Refresh Data", 250, 120, 150, 40, button_callback);

    GooeyTabs_InsertTab(mainTabs, "Settings");

    const char *quality_options[] = {"Low", "Medium", "High", "Ultra"};
    const char *color_options[] = {"Red", "Green", "Blue", "Yellow"};

    GooeyLabel *qualityLabel = GooeyLabel_Create("Quality Preset:", 0.28f, 30, 30);
    GooeyDropdown *qualityDropdown = GooeyDropdown_Create(30, 55, 180, 35, quality_options, 4, dropdown_callback);

    GooeyLabel *brightnessLabel = GooeyLabel_Create("Brightness:", 0.28f, 30, 110);
    GooeySlider *brightnessSlider = GooeySlider_Create(30, 135, 180, 0, 100, true, NULL);

    GooeyLabel *colorLabel = GooeyLabel_Create("UI Theme:", 0.28f, 250, 30);
    GooeyDropdown *colorDropdown = GooeyDropdown_Create(250, 55, 180, 35, color_options, 4, dropdown_callback);

    GooeyTabs_InsertTab(mainTabs, "Data");

    GooeyLabel *nameLabel = GooeyLabel_Create("Sensor ID:", 0.28f, 30, 30);
    GooeyTextbox *idTextbox = GooeyTextBox_Create(160, 30, 200, 35, "SENSOR-001", false, NULL);

    GooeyLabel *valueLabel = GooeyLabel_Create("Threshold:", 0.28f, 30, 80);
    GooeyTextbox *valueTextbox = GooeyTextBox_Create(160, 80, 200, 35, "75.5", false, NULL);

    GooeyCheckbox *logData = GooeyCheckbox_Create(30, 130, "Enable Data Logging", NULL);
    GooeyButton *saveBtn = GooeyButton_Create("Save Configuration", 150, 180, 180, 40, button_callback);

    GooeyTabs_AddWidget(win, mainTabs, 0, meter);
    GooeyTabs_AddWidget(win, mainTabs, 0, sysLabel);
    GooeyTabs_AddWidget(win, mainTabs, 0, sysSwitch);
    GooeyTabs_AddWidget(win, mainTabs, 0, refreshBtn);

    GooeyTabs_AddWidget(win, mainTabs, 1, qualityLabel);
    GooeyTabs_AddWidget(win, mainTabs, 1, qualityDropdown);
    GooeyTabs_AddWidget(win, mainTabs, 1, brightnessLabel);
    GooeyTabs_AddWidget(win, mainTabs, 1, brightnessSlider);
    GooeyTabs_AddWidget(win, mainTabs, 1, colorLabel);
    GooeyTabs_AddWidget(win, mainTabs, 1, colorDropdown);

    GooeyTabs_AddWidget(win, mainTabs, 2, nameLabel);
    GooeyTabs_AddWidget(win, mainTabs, 2, idTextbox);
    GooeyTabs_AddWidget(win, mainTabs, 2, valueLabel);
    GooeyTabs_AddWidget(win, mainTabs, 2, valueTextbox);
    GooeyTabs_AddWidget(win, mainTabs, 2, logData);
    GooeyTabs_AddWidget(win, mainTabs, 2, saveBtn);

    GooeyWindow_RegisterWidget(win, mainTabs);
    GooeyTabs_SetActiveTab(mainTabs, 0);
    GooeyWindow_Run(1, win);
}

void main()
{
    setup();
}
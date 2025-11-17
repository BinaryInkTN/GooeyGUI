#include <gooey.h>

// Callback functions
void dropdown_callback(int selected_index)
{
    printf("Dropdown selection changed to index: %d\n", selected_index);
}
bool state = true;
GooeyTabs *mainTabs;
void button_callback(void *user_data)
{
    GooeyMeter *meter = (GooeyMeter *)user_data;
    GooeyMeter_Update(meter, 90);
}

int main()
{
    Gooey_Init();
    GooeyWindow *win = GooeyWindow_Create("Advanced Tabs Example", 800, 600, true);
    GooeyWindow_EnableDebugOverlay(win, 1);
    GooeyTheme *dark_mode = GooeyTheme_LoadFromFile("dark.json");
   // GooeyWindow_SetTheme(win, dark_mode);
    win->vk = NULL;
    // Create main tabs container
    mainTabs = GooeyTabs_Create(0, 0, 780, 580, true);
    mainTabs->is_sidebar = 0;
    // Tab 1: Controls Demo
    GooeyTabs_InsertTab(mainTabs, "Controls");

    // Widgets for Controls tab
    GooeyMeter *meter = GooeyMeter_Create(30, 200, 100, 100, 50, "test", "logo.png");
    GooeyLabel *label1 = GooeyLabel_Create("test", 0.6f, 200, 100);
    GooeyButton *btn1 = GooeyButton_Create("Click Me", 30, 340, 100, 30, button_callback, meter);
    GooeyCheckbox *checkbox = GooeyCheckbox_Create(30, 80, "Enable Feature", NULL, NULL);
    GooeySlider *slider = GooeySlider_Create(30, 130, 200, 0, 100, true, NULL, NULL);
    GooeySwitch *switch_ = GooeySwitch_Create(30, 150, false, true, NULL, NULL);
    // Tab 2: Dropdowns and Options
    GooeyTabs_InsertTab(mainTabs, "Options");

    // Dropdown options
    const char *quality_options[] = {"Low", "Medium", "High", "Ultra"};
    const char *color_options[] = {"Red", "Green", "Blue", "Yellow", "Cyan", "Magenta"};

    // Widgets for Options tab
    GooeyDropdown *qualityDropdown = GooeyDropdown_Create(30, 30, 150, 30,
                                                          quality_options, 4, dropdown_callback, NULL);
    GooeyDropdown *colorDropdown = GooeyDropdown_Create(30, 80, 150, 30,
                                                        color_options, 6, dropdown_callback, NULL);
    GooeyLabel *dropdownLabel = GooeyLabel_Create("Select Quality:", 0.28f, 30, 10);
    GooeyLabel *colorLabel = GooeyLabel_Create("Select Color:", 0.28f, 30, 60);

    // Tab 3: Form Inputs
    GooeyTabs_InsertTab(mainTabs, "Forms");

    // Widgets for Forms tab
    GooeyLabel *nameLabel = GooeyLabel_Create("Name:", 0.28f, 30, 30);
    GooeyTextbox *nameTextbox = GooeyTextBox_Create(100, 30, 200, 25, "Name:", false, NULL, NULL);

    GooeyLabel *emailLabel = GooeyLabel_Create("Password:", 0.28f, 30, 70);
    GooeyTextbox *emailTextbox = GooeyTextBox_Create(100, 70, 200, 25, "Password:", true, NULL, NULL);

    GooeyButton *submitBtn = GooeyButton_Create("Submit", 150, 120, 100, 30, button_callback, NULL);

    // Add widgets to their respective tabs
    // Tab 0: Controls
        GooeyTabs_AddWidget(win, mainTabs, 0, label1);

    GooeyTabs_AddWidget(win, mainTabs, 0, btn1);
    GooeyTabs_AddWidget(win, mainTabs, 0, checkbox);
    GooeyTabs_AddWidget(win, mainTabs, 0, slider);
    GooeyTabs_AddWidget(win, mainTabs, 0, switch_);
    GooeyTabs_AddWidget(win, mainTabs, 0, meter);

    GooeyButton *button_test = GooeyButton_Create("test", 30, 130, 40, 20, NULL, NULL);

    // Tab 1: Options
    GooeyTabs_AddWidget(win, mainTabs, 1, dropdownLabel);
    GooeyTabs_AddWidget(win, mainTabs, 1, qualityDropdown);
    GooeyTabs_AddWidget(win, mainTabs, 1, button_test);
    GooeyTabs_AddWidget(win, mainTabs, 1, colorLabel);
    GooeyTabs_AddWidget(win, mainTabs, 1, colorDropdown);

    // Tab 2: Forms
    GooeyTabs_AddWidget(win, mainTabs, 2, nameLabel);
    GooeyTabs_AddWidget(win, mainTabs, 2, nameTextbox);
    GooeyTabs_AddWidget(win, mainTabs, 2, emailLabel);
    GooeyTabs_AddWidget(win, mainTabs, 2, emailTextbox);
    GooeyTabs_AddWidget(win, mainTabs, 2, submitBtn);

    GooeyWindow_RegisterWidget(win, mainTabs);

    // Set initial active tab
    GooeyTabs_SetActiveTab(mainTabs, 0);

    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}
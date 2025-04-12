#include <Gooey/gooey.h>

// Callback functions
void dropdown_callback(int selected_index)
{
    printf("Dropdown selection changed to index: %d\n", selected_index);
}

void button_callback()
{
    printf("Button clicked!\n");
}

int main()
{
    Gooey_Init();
    GooeyWindow *win = GooeyWindow_Create("Advanced Tabs Example", 800, 600, true);
    GooeyTheme *dark_mode = GooeyWindow_LoadTheme("dark.json");
    GooeyWindow_SetTheme(win, dark_mode);

    // Create main tabs container
    GooeyTabs *mainTabs = GooeyTabs_Create(10, 10, 780, 580);

    // Tab 1: Controls Demo
    GooeyTabs_InsertTab(mainTabs, "Controls");

    // Widgets for Controls tab
    GooeyButton *btn1 = GooeyButton_Create("Click Me", 30, 30, 100, 30, button_callback);
    GooeyCheckbox *checkbox = GooeyCheckbox_Create(30, 80,  "Enable Feature", NULL);
    GooeySlider *slider = GooeySlider_Create(30, 130, 200, 0, 100, true, NULL);

    // Tab 2: Dropdowns and Options
    GooeyTabs_InsertTab(mainTabs, "Options");

    // Dropdown options
    const char *quality_options[] = {"Low", "Medium", "High", "Ultra"};
    const char *color_options[] = {"Red", "Green", "Blue", "Yellow", "Cyan", "Magenta"};

    // Widgets for Options tab
    GooeyDropdown *qualityDropdown = GooeyDropdown_Create(30, 30, 150, 30,
                                                          quality_options, 4, dropdown_callback);
    GooeyDropdown *colorDropdown = GooeyDropdown_Create(30, 80, 150, 30,
                                                        color_options, 6, dropdown_callback);
    GooeyLabel *dropdownLabel = GooeyLabel_Create("Select Quality:", 0.28f, 30, 10);
    GooeyLabel *colorLabel = GooeyLabel_Create("Select Color:", 0.28f, 30, 60);

    // Tab 3: Form Inputs
    GooeyTabs_InsertTab(mainTabs, "Forms");

    // Widgets for Forms tab
    GooeyLabel *nameLabel = GooeyLabel_Create("Name:", 0.28f, 30, 30);
    GooeyTextbox *nameTextbox = GooeyTextBox_Create(100, 30, 200, 25, "Name:", NULL);

    GooeyLabel *emailLabel = GooeyLabel_Create("Email:", 0.28f, 30, 70);
    GooeyTextbox *emailTextbox = GooeyTextBox_Create(100, 70, 200, 25,"Email:", NULL);

    GooeyButton *submitBtn = GooeyButton_Create("Submit", 150, 120, 100, 30, button_callback);

    // Add widgets to their respective tabs
    // Tab 0: Controls
    GooeyTabs_AddWidget(mainTabs, 0, btn1);
    GooeyTabs_AddWidget(mainTabs, 0, checkbox);
    GooeyTabs_AddWidget(mainTabs, 0, slider);
    

    GooeyButton* button_test = GooeyButton_Create("test", 30, 130, 40, 20, NULL);

    // Tab 1: Options
    GooeyTabs_AddWidget(mainTabs, 1, dropdownLabel);
    GooeyTabs_AddWidget(mainTabs, 1, qualityDropdown);
    GooeyTabs_AddWidget(mainTabs, 1, button_test);
    GooeyTabs_AddWidget(mainTabs, 1, colorLabel);
    GooeyTabs_AddWidget(mainTabs, 1, colorDropdown);

    // Tab 2: Forms
    GooeyTabs_AddWidget(mainTabs, 2, nameLabel);
    GooeyTabs_AddWidget(mainTabs, 2, nameTextbox);
    GooeyTabs_AddWidget(mainTabs, 2, emailLabel);
    GooeyTabs_AddWidget(mainTabs, 2, emailTextbox);
    GooeyTabs_AddWidget(mainTabs, 2, submitBtn);

    // Register all widgets with the window
    GooeyWindow_RegisterWidget(win, btn1);
    GooeyWindow_RegisterWidget(win, checkbox);
    GooeyWindow_RegisterWidget(win, slider);
    GooeyWindow_RegisterWidget(win, dropdownLabel);
    GooeyWindow_RegisterWidget(win, qualityDropdown);
    GooeyWindow_RegisterWidget(win, colorLabel);
    GooeyWindow_RegisterWidget(win, colorDropdown);
    GooeyWindow_RegisterWidget(win, button_test);

    GooeyWindow_RegisterWidget(win, nameLabel);
    GooeyWindow_RegisterWidget(win, nameTextbox);
    GooeyWindow_RegisterWidget(win, emailLabel);
    GooeyWindow_RegisterWidget(win, emailTextbox);
    GooeyWindow_RegisterWidget(win, submitBtn);
    GooeyWindow_RegisterWidget(win, mainTabs);

    // Set initial active tab
    GooeyTabs_SetActiveTab(mainTabs, 0);

    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}
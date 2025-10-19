#include <gooey.h>

// Callback functions
void dropdown_callback(int selected_index)
{
    printf("Dropdown selection changed to index: %d\n", selected_index);
}
bool state = true;
GooeyTabs *mainTabs;
void button_callback()
{
    if (state)
    {
        GooeyTabs_Sidebar_Open(mainTabs);
    } else {
        GooeyTabs_Sidebar_Close(mainTabs);
    }

    state = !state;
        
}

int main()
{
    Gooey_Init();
    GooeyWindow *win = GooeyWindow_Create("Advanced Tabs Example", 800, 600, true);
    GooeyAppbar_Set(win, "test");
    GooeyTheme *dark_mode = GooeyTheme_LoadFromFile("dark.json");
    GooeyWindow_SetTheme(win, dark_mode);

    // Create main tabs container
    mainTabs = GooeyTabs_Create(10, 10, 780, 580, true);
    mainTabs->is_sidebar = 1;
    // Tab 1: Controls Demo
    GooeyTabs_InsertTab(mainTabs, "Controls");

    // Widgets for Controls tab
    GooeyButton *btn1 = GooeyButton_Create("Click Me", 30, 30, 100, 30, button_callback, NULL);
    GooeyCheckbox *checkbox = GooeyCheckbox_Create(30, 80, "Enable Feature", NULL, NULL);
    GooeySlider *slider = GooeySlider_Create(30, 130, 200, 0, 100, true, NULL, NULL);

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
    GooeyTabs_AddWidget(win, mainTabs, 0, btn1);
    GooeyTabs_AddWidget(win, mainTabs, 0, checkbox);
    GooeyTabs_AddWidget(win, mainTabs, 0, slider);

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
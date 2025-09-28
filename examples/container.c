#include <gooey.h>
GooeyContainers *mainContainer; 
int i =0 ; 
// Callback functions
void dropdown_callback(int selected_index)
{
    printf("Dropdown selection changed to index: %d\n", selected_index);
}

void button_callback()
{
    printf("Button clicked!\n");
}
void bt_clbk(){ 
    if ( i >= 2 ){ 
        i = 0 ; 
    }else { 
        i++; 
    }
    GooeyContainer_SetActiveContainer(mainContainer, i);
    printf("Gooey containr %d \n",i);
}
int main()
{
    Gooey_Init();
    GooeyWindow *win = GooeyWindow_Create("Advanced Container Example", 800, 600, true);
    GooeyTheme *dark_mode = GooeyTheme_LoadFromFile("dark.json");
    GooeyWindow_SetTheme(win, dark_mode);
    GooeyButton* bt = GooeyButton_Create("bt",5, 5, 8, 8,bt_clbk);
    GooeyWindow_RegisterWidget(win,bt);
    // Create main tabs container
    mainContainer =GooeyContainer_Create(10, 10, 780, 580);

    // Tab 1: Controls Demo
    GooeyContainer_InsertContainer(mainContainer);

    // Widgets for Controls tab
    GooeyButton *btn1 = GooeyButton_Create("Click Me", 30, 30, 100, 30, button_callback);
    GooeyCheckbox *checkbox = GooeyCheckbox_Create(30, 80,  "Enable Feature", NULL);
    GooeySlider *slider = GooeySlider_Create(30, 130, 200, 0, 100, true, NULL);

    // Tab 2: Dropdowns and Options
    GooeyContainer_InsertContainer(mainContainer);

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
    GooeyContainer_InsertContainer(mainContainer);

    // Widgets for Forms tab
    GooeyLabel *nameLabel = GooeyLabel_Create("Name:", 0.28f, 30, 30);
    // GooeyTextbox *nameTextbox = GooeyTextBox_Create(100, 30, 200, 25, "Name:", NULL);

    GooeyLabel *emailLabel = GooeyLabel_Create("Email:", 0.28f, 30, 70);
    // GooeyTextbox *emailTextbox = GooeyTextBox_Create(100, 70, 200, 25,"Email:", NULL);

    GooeyButton *submitBtn = GooeyButton_Create("Submit", 150, 120, 100, 30, button_callback);

    // Add widgets to their respective tabs
    // Tab 0: Controls
    GooeyContainer_AddWidget(win, mainContainer, 0, btn1);
    GooeyContainer_AddWidget(win, mainContainer, 0, checkbox);
    GooeyContainer_AddWidget(win, mainContainer, 0, slider);
    

    GooeyButton* button_test = GooeyButton_Create("test", 30, 130, 40, 20, NULL);

    // Tab 1: Options
    GooeyContainer_AddWidget(win, mainContainer, 1, dropdownLabel);
    GooeyContainer_AddWidget(win, mainContainer, 1, qualityDropdown);
    GooeyContainer_AddWidget(win, mainContainer, 1, button_test);
    GooeyContainer_AddWidget(win, mainContainer, 1, colorLabel);
    GooeyContainer_AddWidget(win, mainContainer, 1, colorDropdown);

    // Tab 2: Forms
    GooeyContainer_AddWidget(win, mainContainer, 2, nameLabel);
    // GooeyContainer_AddWidget(mainContainer, 2, nameTextbox);
    GooeyContainer_AddWidget(win, mainContainer, 2, emailLabel);
    // GooeyContainer_AddWidget(mainContainer, 2, emailTextbox);
    GooeyContainer_AddWidget(win, mainContainer, 2, submitBtn);

    
    GooeyWindow_RegisterWidget(win, mainContainer);

    // Set initial active tab
    GooeyContainer_SetActiveContainer(mainContainer, 0);

    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}
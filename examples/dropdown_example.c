#include <Gooey/gooey.h>

void dropdown_callback(int selected_index)
{
    printf("Dropdown selection changed to index: %d\n", selected_index);
}

int main()
{

    Gooey_Init();
    GooeyWindow *win = GooeyWindow_Create("DropDown", 800, 800, true);

    const char *fruits_options[] = {"apple", "banana", "manga", "melon"};
    const char *quality_options[] = {"Low", "Medium", "High", "Ultra"};
    const char *color_options[] = {"Red", "Green", "Blue", "Yellow", "Cyan", "Magenta"};

    // Widgets for Options tab
    GooeyDropdown *fruitsDropdown = GooeyDropdown_Create(130, 30, 150, 30,fruits_options, 4, dropdown_callback);
    GooeyDropdown *qualityDropdown = GooeyDropdown_Create(30, 130, 150, 30, quality_options, 4, dropdown_callback);
    GooeyDropdown *colorDropdown = GooeyDropdown_Create(30, 180, 150, 30, color_options, 6, dropdown_callback);

    GooeyWindow_RegisterWidget(win, colorDropdown);

    GooeyWindow_RegisterWidget(win, qualityDropdown);
    GooeyWindow_RegisterWidget(win, fruitsDropdown);
    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);
    return 0;
}
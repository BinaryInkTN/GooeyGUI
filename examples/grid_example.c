#include <gooey.h>

int main()
{
    Gooey_Init("Grid Example", 800, 600);

    GooeyWindow *main_window = GooeyWindow_Create("Main Window", 700, 500, true);
    if (!main_window)
    {
        return -1;
    }

    GooeyLayout *grid_layout = GooeyLayout_Create(LAYOUT_GRID, 20, 20, 660, 460);
    GooeyLayout_SetColumns(grid_layout, 3);
    GooeyButton *button1 = GooeyButton_Create("Button 1", 0, 0, 100, 40, NULL,NULL);
    GooeyButton *button2 = GooeyButton_Create("Button 2", 0, 0, 100, 40, NULL,NULL);
    GooeyButton *button3 = GooeyButton_Create("Button 3", 0, 0, 100, 40, NULL,NULL);
    GooeyButton *button4 = GooeyButton_Create("Button 4", 0, 0, 100, 40, NULL,NULL);
    GooeyButton *button5 = GooeyButton_Create("Button 5", 0, 0, 100, 40, NULL,NULL);
    GooeyButton *button6 = GooeyButton_Create("Button 6", 0, 0, 100, 40, NULL,NULL);

    GooeyLayout_AddChild(main_window, grid_layout, (GooeyWidget *)button6);
    GooeyLayout_AddChild(main_window, grid_layout, (GooeyWidget *)button5);
    GooeyLayout_AddChild(main_window, grid_layout, (GooeyWidget *)button4);
    GooeyLayout_AddChild(main_window, grid_layout, (GooeyWidget *)button3);
    GooeyLayout_AddChild(main_window, grid_layout, (GooeyWidget *)button1);
    GooeyLayout_AddChild(main_window, grid_layout, (GooeyWidget *)button2);
    GooeyLayout_Build(grid_layout);

    GooeyWindow_RegisterWidget(main_window, (GooeyWidget *)grid_layout);

    GooeyWindow_Run(1, main_window);

    return 0;
}
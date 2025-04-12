#include <Gooey/gooey.h>

int main()
{
    Gooey_Init();
    GooeyWindow *win = GooeyWindow_Create("My Window", 800, 600, true);

    GooeyWindow_SetContinuousRedraw(win);
    GooeyWindow_MakeVisible(win, false);
    GooeyWindow_MakeResizable(win, false);
    GooeyButton *button_0 = GooeyButton_Create("Button", 286, 149, 100, 30, NULL);
    const char* options_dropdown_1[4] = {"test1","test2","test3","test4"};
    GooeyDropdown *dropdown_1 = GooeyDropdown_Create(102, 71, 100, 30, options_dropdown_1, 4, NULL);
    GooeyImage *image_2 = GooeyImage_Create("gooey.png", 94, 222, 150, 150, NULL);

    GooeyWindow_RegisterWidget(win, button_0);
    GooeyWindow_RegisterWidget(win, dropdown_1);
    GooeyWindow_RegisterWidget(win, image_2);

    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}

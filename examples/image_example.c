#include <Gooey/gooey.h>


int main()

{   

    Gooey_Init();
    GooeyWindow* win = GooeyWindow_Create("test image", 200, 200, true);


    GooeyImage *image = GooeyImage_Create("gooey.png", 20, 20, 120, 120, NULL);

    GooeyWindow_RegisterWidget(win, image);
    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}
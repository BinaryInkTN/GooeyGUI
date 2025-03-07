#include <Gooey/gooey.h>


int main()

{   

    Gooey_Init(GLPS);
    GooeyWindow* win = GooeyWindow_Create("test image", 100, 100, true);


    GooeyImage_Add(win, "gooey.png", 20, 20, 64, 64);


    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}
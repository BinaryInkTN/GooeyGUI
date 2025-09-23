#include <Gooey/gooey.h>
void canvClbk(int x ,int y ){
printf("hello world \n"); 
}
int main()
{
    Gooey_Init();
    GooeyWindow *win = GooeyWindow_Create("Raw fill test", 800, 600, true);
    GooeyCanvas *can = GooeyCanvas_Create(0, 0, 800, 600, canvClbk);
    GooeyCanvas_DrawRectangle(can, 10, 10, 100, 20, 0xFF0000, true, 0.1f, false, 0.0f);
    GooeyWindow_RegisterWidget(win, can);
    GooeyWindow_Run(1, win);

    GooeyWindow_Cleanup(1, win);
    return 0;
}
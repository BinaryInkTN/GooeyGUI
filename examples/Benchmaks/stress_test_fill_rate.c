#include <Gooey/gooey.h>
#include <Gooey/utils/logger/gooey_logger_internal.h>

#define MAX_NUMBER 300
void canvClbk(int x ,int y ){
printf("hello world \n"); 
}
int main()
{
    Gooey_Init();
    GooeyWindow *win = GooeyWindow_Create("Raw fill test", 800, 600, true);
    GooeyWindow_EnableDebugOverlay(win,true);
    GooeyCanvas *can = GooeyCanvas_Create(0, 0, 700, 500, canvClbk);
    GooeyWindow_RegisterWidget(win, can);
    LOG_PERFORMANCE(NULL); 
    for (int i = 0; i < MAX_NUMBER; i++)
    {
        GooeyCanvas_DrawRectangle(can, 10, 10, 100, 20, 0xFF0000, true, 0.1f, false, 0.0f);
    }
    LOG_PERFORMANCE("Performance: ");
    GooeyWindow_Run(1, win);
    
    GooeyWindow_Cleanup(1, win);
    return 0;
}
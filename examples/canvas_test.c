#include <gooey.h>
#include "../internal/backends/gooey_backend_internal.h"

#define MAX_NUMBER  20
void canvClbk(int x ,int y ){
printf("hello world \n"); 
        active_backend->FillRectangle(10, 10, 50, 50, 0xFF0000, 0, false, 0.0f, NULL);

}
int main()
{
    Gooey_Init();
    GooeyWindow* win = GooeyWindow_Create("Raw fill test", 800, 600, true);
    GooeyWindow_EnableDebugOverlay(win, true);
    GooeyCanvas* can = GooeyCanvas_Create(0, 0, 700, 500, canvClbk);
    GooeyCanvas_DrawRectangle(can, 10, 10, 50, 50, 0xFF0000, 0, false, true, 1.0f);
    GooeyWindow_RegisterWidget(win, can);
    GooeyButton* btn = GooeyButton_Create( "Click me", 10, 520, 100, 50, NULL);
    GooeyWindow_RegisterWidget(win, btn);
    //LOG_PERFORMANCE(NULL); 
  
   
    //LOG_PERFORMANCE("Performance: ");
    GooeyWindow_Run(1, win);

    GooeyWindow_Cleanup(1, win);
    return 0;
}
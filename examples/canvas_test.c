#include <gooey.h>
#include "../internal/backends/gooey_backend_internal.h"

#define MAX_NUMBER  20
void canvClbk(int x ,int y ){
printf("hello world \n"); 

}

void on_file_selected(const char* path)
{
    printf("Selected file: %s\n", path);
}

void open_fd()
{
    const char* filters[2][2] = {{"Image Files", "*.png;*.jpg;*.jpeg"}, {"All Files", "*.*"}};
    GooeyFDialog_Open("/", (void*) filters, 2, on_file_selected);
}

int main()
{
    Gooey_Init();
    GooeyWindow* win = GooeyWindow_Create("Raw fill test", 800, 1000, true);
    GooeyContainers* container = GooeyContainer_Create(0, 0, 800, 600);
    GooeyWindow_RegisterWidget(win, container);
    GooeyContainer_InsertContainer(container);

    GooeyWindow_EnableDebugOverlay(win, true);
    GooeyCanvas* can = GooeyCanvas_Create(0, 0, 700, 500, canvClbk);
    GooeyCanvas_DrawRectangle(can, 10, 10, 50, 50, 0xFF0000, 0, false, true, 1.0f);
    GooeyButton* btn = GooeyButton_Create( "Click me", 10, 520, 100, 50, open_fd);
    GooeyContainer_AddWidget(win, container, 0, can);
    GooeyContainer_AddWidget(win, container, 0, btn);
    //LOG_PERFORMANCE(NULL);

    const char* filters[2][2] = {{"c File", "*.png"}, {"*.c", "*.jpeg"}};

    //LOG_PERFORMANCE("Performance: ");
    GooeyWindow_Run(1, win);

    GooeyWindow_Cleanup(1, win);
    return 0;
}
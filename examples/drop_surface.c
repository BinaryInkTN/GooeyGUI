#include "gooey.h"

GooeyDropSurface* drop_surface;

void clear_callback()
{
    GooeyDropSurface_Clear(drop_surface);
}

void drag_n_drop(char* mime, char* file_path)
{
   // LOG_INFO("%s %s", mime, file_path);
}

int main()
{  

    Gooey_Init();
    GooeyWindow *win = GooeyWindow_Create("Drag n Drop example", 400, 400, true);

    GooeyTheme* dark_mode = GooeyWindow_LoadTheme("dark.json");
    GooeyWindow_SetTheme(win, dark_mode);
    drop_surface = GooeyDropSurface_Create(25, 25, 350, 300, "Drop files here", drag_n_drop);


    GooeyButton* clear_button = GooeyButton_Create("Clear drop surface", 25, 340, 25, 40, clear_callback);
    
    GooeyWindow_RegisterWidget(win, drop_surface);
    GooeyWindow_RegisterWidget(win, clear_button);
    
    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}
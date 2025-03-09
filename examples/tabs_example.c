#include <Gooey/gooey.h>


int main()
{  

    Gooey_Init(GLPS);
    GooeyWindow *win = GooeyWindow_Create("Tabs example", 400, 400, true);
    GooeyTheme* dark_mode = GooeyWindow_LoadTheme("dark.json");
    GooeyWindow_SetTheme(win, dark_mode);
    GooeyButton *button = GooeyButton_Add(win, "test", 20, 20, 50, 50, NULL);
    
   // GooeyTabs* tabs = GooeyTabs_Add(win, 25, 25, 350, 350);

   // GooeyTab* test_tab = GooeyTabs_InsertTab(tabs, "mokh");
    //GooeyTab* test_tab2 = GooeyTabs_InsertTab(tabs, "yassine");

    //GooeyTabs_AddWidget(test_tab2, button);

    //GooeyTabs_SetActiveTab(tabs, test_tab2);

    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}
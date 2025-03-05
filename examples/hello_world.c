
#include "gooey.h"
#include <stdio.h>

bool state = 0;
GooeyWindow msgBox;

void messageBoxCallback(int option)
{
    LOG_INFO("MessageBox option: %d", option);
}

void onButtonClick()
{
    state = !state;
    GooeyWindow_MakeVisible(&msgBox, state);
}

int main()
{
    set_logging_enabled(true);
    set_minimum_log_level(DEBUG_LEVEL_INFO);

    Gooey_Init(GLPS);

    GooeyWindow win = GooeyWindow_Create("Hello World", 400, 300, 1);

    msgBox = GooeyMessageBox_Create("Hello", "Welcome to Gooey!", MSGBOX_INFO, messageBoxCallback);

    GooeyMessageBox_Show(&msgBox);

    GooeyButton_Add(&win, "Click Me", 150, 100, 100, 40, onButtonClick);

    GooeyWindow_Run(2, &win, &msgBox);

    //GooeyWindow_Cleanup(2, &win, &msgBox);

    return 0;
}

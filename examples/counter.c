#include "gooey.h"
#include <stdio.h>

int counter = 0;
GooeyLabel *counterLabel;

void onIncrementClick()
{
    counter++;
    char text[20];
    snprintf(text, sizeof(text), "Counter: %d", counter);
    GooeyLabel_SetText(counterLabel, text);
}

int main()
{
    Gooey_Init(GLPS);

    GooeyWindow win = GooeyWindow_Create("Counter App", 300, 130, true);

    GooeyLayout *layout = GooeyLayout_Create(&win, LAYOUT_VERTICAL, 10, 50, 280, 180);

    counterLabel = GooeyLabel_Add(&win, "Counter: 0", 0.25f, 0, 0);
    GooeyLayout_AddChild(layout, counterLabel);

    GooeyButton *incrementButton = GooeyButton_Add(&win, "Increment", 0, 0, 100, 30, onIncrementClick);
    GooeyLayout_AddChild(layout, incrementButton);

    GooeyLayout_Build(layout);

    GooeyWindow_Run(1, &win);
    GooeyWindow_Cleanup(1,&win);

    return 0;
}

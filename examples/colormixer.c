#include "gooey.h"
#include <stdio.h>

int red = 0, green = 0, blue = 0;
GooeyCanvas *canvas;
GooeyWindow childWindow;

void updateColor()
{
    LOG_INFO("r=%d g=%d b=%d", red, green, blue);
    unsigned long color = (red << 16) | (green  << 8) | blue;
    GooeyCanvas_DrawRectangle(canvas, 0, 0, 200, 200, color, true);
    LOG_WARNING("%ld", childWindow.creation_id);
    GooeyWindow_RequestRedraw(&childWindow);
}

void onRedChange(long value)
{
    red = value;
    updateColor();
}
void onGreenChange(long value)
{
    green = value;
    updateColor();
}
void onBlueChange(long value)
{
    blue = value;
    updateColor();
}

int main()
{
    Gooey_Init(GLPS);

    GooeyWindow win = GooeyWindow_Create("RGB Mixer", 420, 130, true);
    GooeyLayout *layout = GooeyLayout_Create(&win, LAYOUT_VERTICAL, 10, 30, 380, 380);

    GooeySlider *redSlider = GooeySlider_Add(&win, 0, 0, 200, 0, 255, true, onRedChange);
    GooeyLayout_AddChild(layout, redSlider);

    GooeySlider *greenSlider = GooeySlider_Add(&win, 0, 0, 200, 0, 255, true, onGreenChange);
    GooeyLayout_AddChild(layout, greenSlider);

    GooeySlider *blueSlider = GooeySlider_Add(&win, 0, 0, 200, 0, 255, true, onBlueChange);
    GooeyLayout_AddChild(layout, blueSlider);

    GooeyLayout_Build(layout);

    childWindow = GooeyWindow_Create("Color Preview", 220, 220, true);
    canvas = GooeyCanvas_Add(&childWindow, 10, 10, 200, 200);

    GooeyWindow_Run(2, &win, &childWindow);
    GooeyWindow_Cleanup(2, &win, &childWindow);

    return 0;
}

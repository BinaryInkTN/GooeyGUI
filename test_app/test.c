#include "build/include/gooey.h"
#include <stdio.h>

int red = 0, green = 0, blue = 0;
GooeyCanvas *canvas;
GooeyWindow childWindow;

void updateColor()
{
    LOG_CRITICAL("r=%d g=%d b=%d", red, green, blue);
    char color[9];
    snprintf(color, sizeof(color), "0x%02X%02X%02X", red, green, blue);
    GooeyCanvas_DrawRectangle(canvas, 0, 0, 200, 200, color, true);
    GooeyWindow_Redraw(&childWindow);
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
    Gooey_Init(GLFW);

    GooeyWindow win = GooeyWindow_Create("RGB Mixer", 420, 130, true);
    GooeyLayout *layout = GooeyLayout_Create(&win, LAYOUT_VERTICAL, 10, 30, 380, 380);

    GooeySlider *redSlider = GooeySlider_Add(&win, 0, 0, 200, 0, 255, true, onRedChange);
    GooeyLayout_AddChild(layout, redSlider);

    GooeySlider *greenSlider = GooeySlider_Add(&win, 0, 0, 200, 0, 255, true, onGreenChange);
    GooeyLayout_AddChild(layout, greenSlider);

    GooeySlider *blueSlider = GooeySlider_Add(&win, 0, 0, 200, 0, 255, true, onBlueChange);
    GooeyLayout_AddChild(layout, blueSlider);

    GooeyLayout_Build(layout);

    childWindow = GooeyWindow_CreateChild("Color Preview", 220, 220, true);
    canvas = GooeyCanvas_Add(&childWindow, 10, 10, 200, 200);

    GooeyWindow_Run(2, &win, &childWindow);
    GooeyWindow_Cleanup(2, &win, &childWindow);

    return 0;
}
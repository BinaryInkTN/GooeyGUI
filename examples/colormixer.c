#include "gooey.h"
#include <stdio.h>

int red = 0, green = 0, blue = 0;
GooeyCanvas *canvas;
GooeyWindow* childWindow;

void updateColor()
{
    //LOG_INFO("r=%d g=%d b=%d", red, green, blue);
    unsigned long color = (red << 16) | (green  << 8) | blue;
    GooeyCanvas_DrawRectangle(canvas, 0, 0, 200, 200, color, true, 1.0f, true, 1.0f);
   // LOG_WARNING("%ld", childWindow->creation_id);
    GooeyWindow_RequestRedraw(childWindow);
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
    Gooey_Init();

    GooeyWindow* win = GooeyWindow_Create("RGB Mixer", 420, 130, true);
    GooeyLayout *layout = GooeyLayout_Create(LAYOUT_VERTICAL, 10, 30, 380, 380);

    GooeySlider *redSlider = GooeySlider_Create( 0, 0, 200, 0, 255, true, NULL);
    GooeyLayout_AddChild(win, layout, redSlider);

    GooeySlider *greenSlider = GooeySlider_Create(0, 0, 200, 0, 255, true, NULL);
    GooeyLayout_AddChild(win, layout, greenSlider);

    GooeySlider *blueSlider = GooeySlider_Create(0, 0, 200, 0, 255, true, NULL);
    GooeyLayout_AddChild(win, layout, blueSlider);

    GooeyLayout_Build(layout);

    childWindow = GooeyWindow_Create("Color Preview", 220, 220, true);
    canvas = GooeyCanvas_Create(10, 10, 200, 200, NULL);

   // GooeyLayout_AddChild(childWindow, layout, canvas);

    GooeyWindow_RegisterWidget(win, layout);

    GooeyWindow_RegisterWidget(childWindow, canvas);

    GooeyWindow_Run(2, win, childWindow);
//GooeyWindow_Cleanup(2, win, childWindow);

    return 0;
}

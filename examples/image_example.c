#include <Gooey/gooey.h>
#include <GLPS/glps_thread.h>
#include <Gooey/animations/gooey_animations.h>


int main()
{
    Gooey_Init();
    GooeyWindow *win = GooeyWindow_Create("test image", 200, 200, true);

    GooeyImage *image = GooeyImage_Create("gooey.png", 0, 0, 120, 120, NULL);
    GooeyWindow_RegisterWidget(win, image);

    GooeyAnimation_TranslateX(image, 0, 120, 0.2f);
    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}

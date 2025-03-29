#include <Gooey/gooey.h>

// Callback functions
void button_callback()
{
    printf("Button clicked!\n");
}

void slider_callback(int value)
{
    printf("Slider value: %d\n", value);
}

void checkbox_callback(bool checked)
{
    printf("Checkbox %s\n", checked ? "checked" : "unchecked");
}

void input_callback(const char* text)
{
    printf("Input changed: %s\n", text);
}

int main()
{
    Gooey_Init();
    GooeyWindow *win = GooeyWindow_Create("Hello World", 800, 600, true);

    GooeyCanvas *canvas_0 = GooeyCanvas_Create(134, 90, 100, 30);

    // Register all widgets with the window
    GooeyWindow_RegisterWidget(win, canvas_0);

    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}

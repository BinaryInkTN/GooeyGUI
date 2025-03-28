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

    GooeyButton *button_0 = GooeyButton_Create("Mokhtar", 277, 113, 100, 30, button_callback);
    GooeyTextbox *input_1 = GooeyTextBox_Create(105, 114, 100, 30, "Type here", input_callback);

    // Register all widgets with the window
    GooeyWindow_RegisterWidget(win, button_0);
    GooeyWindow_RegisterWidget(win, input_1);

    GooeyWindow_Run(1, win);
    GooeyWindow_Cleanup(1, win);

    return 0;
}

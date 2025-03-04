#include <Gooey/gooey.h>
#include <stdio.h>
#include <string.h>
GooeyWindow win;
GooeyLayout *layout;
GooeyTextbox *inputBox;
GooeyList *list;
int taskCount = 0;

void onAddTask(void)
{
    char title[20];
    sprintf(title, "Task %d", taskCount + 1);
    GooeyList_AddItem(list, title, GooeyTextbox_GetText(inputBox));
    taskCount++;
}

void test_idx(int index){
    LOG_INFO("%d", index);
}

int main()
{
    Gooey_Init(GLPS);

    win = GooeyWindow_Create("To-Do List", 400, 300, true);
    //GooeyWindow_MakeResizable(&win, 0);
    layout = GooeyLayout_Create(&win, LAYOUT_VERTICAL, 10, 20, 380, 280);
    GooeyLayout *input_layout = GooeyLayout_Create(&win, LAYOUT_HORIZONTAL, 0, 0, 0, 0);

    inputBox = GooeyTextBox_Add(&win, 0, 0, 300, 30, "Add task", NULL);
    GooeyButton *addButton = GooeyButton_Add(&win, "Add Task", 0, 0, 20, 30, onAddTask);
    list = GooeyList_Add(&win, 0, 0, 380, 250, test_idx);

    GooeyLayout_AddChild(input_layout, inputBox);

    GooeyLayout_AddChild(input_layout, addButton);

    GooeyLayout_AddChild(layout, input_layout);

    GooeyLayout_AddChild(layout, list);

    GooeyLayout_Build(layout);

    GooeyWindow_Run(1, &win);
    GooeyWindow_Cleanup(1, &win);

    return 0;
}

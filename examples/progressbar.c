#include "gooey.h"
#include <stdio.h>
#include <unistd.h>

GooeyWindow *win;
GooeyProgressBar *progressbar;
volatile int progress_percentage = 0;
volatile bool running = true;

void *progress_thread(void *arg)
{
    while (running)
    {
        if (progress_percentage < 100)
        {
            progress_percentage += 15;
            GooeyProgressBar_Update(progressbar, progress_percentage);
        }
        else
        {
            progress_percentage = 0;
            GooeyProgressBar_Update(progressbar, progress_percentage);
        }

        sleep(1);
    }
    return NULL;
}

void toggle_progress()
{
    static gthread_t thread;
    static bool thread_running = false;

    if (!thread_running)
    {
        running = true;
        glps_thread_create(&thread, NULL, progress_thread, NULL);
        thread_running = true;
    }
    else
    {
        running = false;
        glps_thread_join(thread, NULL);
        thread_running = false;
    }
}

void reset_progress()
{
    progress_percentage = 0;
    GooeyProgressBar_Update(progressbar, progress_percentage);
}

int main()
{

    Gooey_Init();
    win = GooeyWindow_Create("Progress Demo", 800, 600, true);
    //GooeyWindow_SetContinuousRedraw(win);
    //GooeyWindow_EnableDebugOverlay(win, 1);

    GooeyMenu *menu = GooeyMenu_Set(win);
    GooeyMenuChild *control_child = GooeyMenu_AddChild(win, "Control");
    GooeyMenuChild_AddElement(control_child, "Start/Stop", toggle_progress);
    GooeyMenuChild_AddElement(control_child, "Reset", reset_progress);

    progressbar = GooeyProgressBar_Create(300, 250, 200, 30, progress_percentage);
    GooeyButton *start_btn = GooeyButton_Create("Start/Stop", 300, 300, 200, 40, toggle_progress);
    GooeyButton *reset_btn = GooeyButton_Create("Reset", 300, 350, 200, 40, reset_progress);

    GooeyWindow_RegisterWidget(win, progressbar);
    GooeyWindow_RegisterWidget(win, start_btn);
    GooeyWindow_RegisterWidget(win, reset_btn);

    GooeyLabel *title = GooeyLabel_Create("Threaded Progress Demo", 0.27f, 300, 200);

    GooeyWindow_RegisterWidget(win, title);

    GooeyWindow_Run(1, win);

    running = false;
    GooeyWindow_Cleanup(1, win);
    return 0;
}
#include "widgets/gooey_image_internal.h"
#if(ENABLE_IMAGE)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
#include <fcntl.h>
#include <unistd.h>

bool GooeyImage_HandleClick(GooeyWindow *win, int mouseX, int mouseY)
{
    for (size_t i = 0; i < win->image_count; ++i)
    {
        GooeyImage *image = win->images[i];
        if (!image || !image->core.is_visible)
            continue;
        if (mouseX > image->core.x && mouseX < image->core.x + image->core.width && mouseY > image->core.y && mouseY < image->core.y + image->core.height)
        {
            if (image->callback)
            {
                image->callback();
            }

            return true;
        }
    }

    return false;
}

void GooeyImage_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->image_count; ++i)
    {
        GooeyImage *image = win->images[i];
        if (!image->core.is_visible)
            continue;

        if (image->needs_refresh)
        {
            active_backend->UnloadImage(image->texture_id);
            active_backend->LoadImage(image->image_path);
            image->needs_refresh = false;
        }
        LOG_PERFORMANCE(NULL);
        active_backend->DrawImage(image->texture_id, image->core.x, image->core.y, image->core.width, image->core.height, win->creation_id);
        LOG_PERFORMANCE("Image Draw: ");
    }
}
#endif
#include "gooey_image_internal.h"
#include "backends/gooey_backend_internal.h"

bool GooeyImage_HandleClick(GooeyWindow *win, int mouseX, int mouseY)
{
    for (size_t i = 0; i < win->image_count; ++i)
    {
        GooeyImage *image = win->images[i];
        if (mouseX > image->core.x && mouseX < image->core.x + image->core.width && mouseY > image->core.y && mouseY < image->core.y + image->core.height)
        {
            if (image->callback)
                image->callback();
        }
    }
}

void GooeyImage_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->image_count; ++i)
    {
        GooeyImage *image = win->images[i];
        active_backend->DrawImage(image->texture_id, image->core.x, image->core.y, image->core.width, image->core.height, win->creation_id);
    }
}
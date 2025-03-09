#include "widgets/gooey_image.h"
#include "core/gooey_backend.h"

void GooeyImage_Add(GooeyWindow *win, const char *image_path, int x, int y, int width, int height, void (*callback)(void))
{
    if (!win)
    {
        LOG_ERROR("Couldn't add image, window is invalid");
        return;
    }
    GooeyImage *image = &win->images[win->image_count++];
    *image = (GooeyImage){0};

    image->texture_id = active_backend->LoadImage(image_path);
    image->core.type = WIDGET_IMAGE;
    image->core.x = x;
    image->core.y = y;
    image->core.width = width;
    image->core.height = height;
    image->callback = callback;
}

bool GooeyImage_HandleClick(GooeyWindow *win, int mouseX, int mouseY)
{
    for (size_t i = 0; i < win->image_count; ++i)
    {
        GooeyImage *image = &win->images[i];
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
        GooeyImage *image = &win->images[i];

        //  if (image->is_image_drawn == false) {
        active_backend->DrawImage(image->texture_id, image->core.x, image->core.y, image->core.width, image->core.height, win->creation_id);
        // image->is_image_drawn = true;
        //  }
    }
}
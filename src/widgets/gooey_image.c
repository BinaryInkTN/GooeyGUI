#include "widgets/gooey_image.h"
#include "core/gooey_backend.h"

void GooeyImage_Create(const char *image_path, int x, int y, int width, int height, void (*callback)(void))
{
    GooeyImage *image = (GooeyImage *) malloc(sizeof(GooeyImage));

    if(!image)
    {
        LOG_ERROR("Couldn't allocate memory for image.");
        return NULL;
    }

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
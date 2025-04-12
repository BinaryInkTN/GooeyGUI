#include "widgets/gooey_image.h"
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"

GooeyImage *GooeyImage_Create(const char *image_path, int x, int y, int width, int height, void (*callback)(void))
{
    GooeyImage *image = (GooeyImage *)malloc(sizeof(GooeyImage));

    if (!image)
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
    image->core.is_visible = true;
    image->callback = callback;

    return image;
}

void GooeyImage_SetImage(GooeyImage *image, const char *image_path)
{
    if (!image)
    {
        LOG_ERROR("Image widget is NULL");
        return;
    }

    image->texture_id = active_backend->LoadImage(image_path);
}
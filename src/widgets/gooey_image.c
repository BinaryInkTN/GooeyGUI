#include "widgets/gooey_image.h"
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
#include <fcntl.h>
#include <unistd.h>

GooeyImage *GooeyImage_Create(const char *image_path, int x, int y, int width, int height, void (*callback)(void))
{
    GooeyImage *image = (GooeyImage *)calloc(1, sizeof(GooeyImage));

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
    image->needs_refresh = false;
    image->image_path = image_path;
    return image;
}

void GooeyImage_SetImage(GooeyImage *image, const char *image_path)
{
    if (!image)
    {
        LOG_ERROR("Image widget is NULL");
        return;
    }
    printf("%u \n", image->texture_id);

    if (image->texture_id != 0)
    {
        active_backend->UnloadImage(image->texture_id);
        image->texture_id = 0;
    }

    image->texture_id = active_backend->LoadImage(image_path);
    image->image_path = image_path;
}

void GooeyImage_Damage(GooeyImage *image)
{
    image->needs_refresh = access(image->image_path, F_OK) == 0;
}

#include "widgets/gooey_image.h"
#if (ENABLE_IMAGE)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
#include <fcntl.h>
#include <unistd.h>

GooeyImage *GooeyImage_Create(const char *image_path, int x, int y, int width, int height, void (*callback)(void *user_data), void *user_data)
{
    GooeyImage *image = (GooeyImage *)calloc(1, sizeof(GooeyImage));

    if (!image)
    {
        LOG_ERROR("Couldn't allocate memory for image.");
        return NULL;
    }

    *image = (GooeyImage){0};
    image->texture_id = 0;
    image->core.type = WIDGET_IMAGE;
    image->is_loaded = false;
    image->core.x = x;
    image->core.y = y;
    image->core.width = width;
    image->core.height = height;
    image->core.is_visible = true;
    image->callback = callback;
    image->needs_refresh = false;
    image->image_path = image_path;
    image->user_data = user_data;
    image->core.sprite = active_backend->CreateSpriteForWidget(x, y, width, height);
    image->core.disable_input = false;
    return image;
}

void GooeyImage_SetImage(GooeyImage *image, const char *image_path)
{
    if (!image)
    {
        LOG_ERROR("Image widget is NULL");
        return;
    }

    image->image_path = image_path;
}

void GooeyImage_Damage(GooeyImage *image)
{
    image->needs_refresh = true;
}
#endif

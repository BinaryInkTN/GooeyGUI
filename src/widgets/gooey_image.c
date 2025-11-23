#include "widgets/gooey_image.h"
#if (ENABLE_IMAGE)
#include "backends/gooey_backend_internal.h"
#include "logger/pico_logger_internal.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

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
    image->user_data = user_data;
    image->core.disable_input = false;

    // COPY the image path string instead of storing the pointer
    if (image_path)
    {
        image->image_path = strdup(image_path);
        if (!image->image_path)
        {
            LOG_ERROR("Failed to allocate memory for image path");
            free(image);
            return NULL;
        }
    }
    else
    {
        image->image_path = NULL;
        LOG_WARNING("Image path is NULL");
    }

    image->core.sprite = active_backend->CreateSpriteForWidget(x, y, width, height);
    
    LOG_INFO("Created image widget with path: %s", image_path ? image_path : "NULL");
    
    return image;
}

void GooeyImage_SetImage(GooeyImage *image, const char *image_path)
{
    if (!image)
    {
        LOG_ERROR("Image widget is NULL");
        return;
    }

    // Free the old path if it exists
    if (image->image_path)
    {
        free((void*)image->image_path);
    }

    // Copy the new path
    if (image_path)
    {
        image->image_path = strdup(image_path);
        if (!image->image_path)
        {
            LOG_ERROR("Failed to allocate memory for new image path");
            return;
        }
    }
    else
    {
        image->image_path = NULL;
    }

    image->needs_refresh = true;
    LOG_INFO("Set new image path: %s", image_path ? image_path : "NULL");
}

void GooeyImage_Damage(GooeyImage *image)
{
    image->needs_refresh = true;
}

void GooeyImage_Destroy(GooeyImage *image)
{
    if (!image) return;
    
    // Free the copied image path
    if (image->image_path)
    {
        free((void*)image->image_path);
        image->image_path = NULL;
    }
    
    // Free any other resources...
    free(image);
}
#endif
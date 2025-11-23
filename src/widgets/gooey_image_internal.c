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
        if (!image || !image->core.is_visible || image->core.disable_input)
            continue;
        if (mouseX > image->core.x && mouseX < image->core.x + image->core.width && mouseY > image->core.y && mouseY < image->core.y + image->core.height)
        {
            if (image->callback)
            {
                image->callback(image->user_data);
            }
        }
    }

    return true;
}

void GooeyImage_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->image_count; ++i)
    {
        GooeyImage *image = win->images[i];
        if (!image || !image->core.is_visible)
            continue;
        
        if(!image->is_loaded)
        {
            if (image->image_path)
            {
                LOG_INFO("Loading image: %s", image->image_path);
                image->texture_id = active_backend->LoadGooeyImage(image->image_path);
                if (image->texture_id == 0)
                {
                    LOG_ERROR("Failed to load image: %s", image->image_path);
                    // You could set a fallback texture here
                }
                image->is_loaded = true;
            }
            else
            {
                LOG_ERROR("Image path is NULL for image widget");
            }
        }

        if (image->needs_refresh && image->is_loaded)
        {
            LOG_INFO("Refreshing image: %s", image->image_path);
            active_backend->UnloadImage(image->texture_id);
            if (image->image_path)
            {
                image->texture_id = active_backend->LoadGooeyImage(image->image_path);
            }
            image->needs_refresh = false;
            active_backend->RequestRedraw(win);
        }
        
        // Only draw if we have a valid texture
        if (image->texture_id != 0)
        {
            active_backend->DrawImage(image->texture_id, image->core.x, image->core.y, image->core.width, image->core.height, win->creation_id);
        }
        else if (image->is_loaded)
        {
            // Draw a placeholder rectangle if image failed to load
            active_backend->FillRectangle(image->core.x, image->core.y, image->core.width, image->core.height, 
                                        0xFF0000, win->creation_id, false, 0.0f, image->core.sprite);
        }
    }
}
#endif
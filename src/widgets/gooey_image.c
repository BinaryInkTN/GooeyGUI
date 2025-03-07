#include "widgets/gooey_image.h"
#include "core/gooey_backend.h"

void GooeyImage_Add(GooeyWindow *win, const char *image_path, int x, int y, int width, int height)
{
    if (!win)
    {
        LOG_ERROR("Couldn't add image, window is invalid");
        return;
    }
    GooeyImage *image = &win->images[win->image_count++];
    *image = (GooeyImage){0};

    image->texture_id = active_backend->LoadImage(image_path);

    image->core.x = x;
    image->core.y = y;
    image->core.width = width;
    image->core.height = height;
   
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
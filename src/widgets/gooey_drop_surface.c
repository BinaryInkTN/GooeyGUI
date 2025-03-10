#include "widgets/gooey_drop_surface.h"
#include "core/gooey_backend.h"
#include "assets/drop_surface_image.h"

static void __get_filename_from_path(char *file_path, char *filename, size_t filename_size)
{
    char *basename = strrchr(file_path, '/');
    if (basename != NULL)
    {
        basename++; // Move past the '/'
        strncpy(filename, basename, filename_size - 1);
        filename[filename_size - 1] = '\0';
    }
    else
    {
        strncpy(filename, "Couldn't fetch filename", filename_size - 1);
        filename[filename_size - 1] = '\0';
    }
}
GooeyDropSurface *GooeyDropSurface_Create(int x, int y, int width, int height, char *default_message, void (*callback)(char *mime, char *file_path))
{
    GooeyDropSurface *drop_surface = (GooeyDropSurface *) malloc(sizeof(GooeyDropSurface));
    
    if(!drop_surface)
    {
        LOG_ERROR("Couldn't allocate memory for drop surface.");
        return NULL;
    }
    
    *drop_surface = (GooeyDropSurface){0};
    drop_surface->core.type = WIDGET_DROP_SURFACE;
    drop_surface->core.x = x;
    drop_surface->core.y = y;
    drop_surface->core.width = width;
    drop_surface->core.height = height;
    drop_surface->callback = callback;
    drop_surface->is_file_dropped = false;
    drop_surface->file_icon_texture_id = active_backend->LoadImageFromBin(file_png, file_png_len);
    strncpy(drop_surface->default_message, default_message, sizeof(drop_surface->default_message) - 1);
    drop_surface->default_message[sizeof(drop_surface->default_message) - 1] = '\0';

    return drop_surface;
}

void GooeyDropSurface_Clear(GooeyDropSurface *drop_surface)
{
    drop_surface->is_file_dropped = false;
}

bool GooeyDropSurface_HandleFileDrop(GooeyWindow *win, int mouseX, int mouseY)
{
    for (size_t i = 0; i < win->drop_surface_count; ++i)
    {
        GooeyDropSurface *drop_surface = win->drop_surface[i];

        if (mouseX > drop_surface->core.x && mouseX < drop_surface->core.x + drop_surface->core.width && mouseY > drop_surface->core.y && mouseY < drop_surface->core.y + drop_surface->core.height)
        {
            drop_surface->is_file_dropped = true;

            if (drop_surface->callback)
            {
                drop_surface->callback(win->current_event->drop_data.mime, win->current_event->drop_data.file_path);
            }

            return true;
        }
    }

    return false;
}
void GooeyDropSurface_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->drop_surface_count; ++i)
    {
        GooeyDropSurface *drop_surface = win->drop_surface[i];
        unsigned long surface_color = win->active_theme->widget_base;
        bool show_image = true;
        char filename[64];

        if (drop_surface->is_file_dropped)
        {
            surface_color = win->active_theme->primary;
            __get_filename_from_path(win->current_event->drop_data.file_path, filename, sizeof(filename));
            show_image = false;
        }
        else
        {
            strncpy(filename, drop_surface->default_message, sizeof(filename) - 1);
            filename[sizeof(filename) - 1] = '\0';
        }

        active_backend->DrawRectangle(
            drop_surface->core.x, drop_surface->core.y,
            drop_surface->core.width, drop_surface->core.height,
            surface_color, win->creation_id);

        int available_width = drop_surface->core.width - 20; // Padding
        int text_width = active_backend->GetTextWidth(filename, strlen(filename));

        if (text_width > available_width)
        {
            size_t len = strlen(filename);
            if (len > 3)
            {
                size_t new_len = len;
                do
                {
                    new_len--;
                    filename[new_len] = '\0';
                    text_width = active_backend->GetTextWidth(filename, new_len);
                } while (new_len > 3 && text_width > available_width);

                strcpy(filename + new_len - 3, "...");
            }
        }
        int img_height = 64, img_width = 64;
    
        if(show_image) 
        {
            int icon_x = drop_surface->core.x + (float)(drop_surface->core.width - img_width) / 2 ;
            int icon_y = drop_surface->core.y + (float)(drop_surface->core.height -  img_width) / 2;
            active_backend->DrawImage(drop_surface->file_icon_texture_id, icon_x, icon_y, img_width, img_height, win->creation_id);
        } else {
            img_height = 0;
        }
        int text_height = active_backend->GetTextHeight(filename, strlen(filename));
        int text_x = drop_surface->core.x + (float)(drop_surface->core.width - text_width) / 2;
        int text_y = drop_surface->core.y + (float)(drop_surface->core.height - text_height) / 2 + img_height;
      
        active_backend->DrawText(text_x, text_y, filename, win->active_theme->neutral, 0.26f, win->creation_id);
    }
}

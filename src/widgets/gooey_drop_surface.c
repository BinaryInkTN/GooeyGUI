#include "widgets/gooey_drop_surface.h"
#include "core/gooey_backend.h"

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
GooeyDropSurface* GooeyDropSurface_Add(GooeyWindow *win, int x, int y, int width, int height, char *default_message, void (*callback)(char *mime, char *file_path))
{
    if (!win)
    {
        LOG_ERROR("Couldn't add drop surface, window is invalid");
        return;
    }
    GooeyDropSurface *drop_surface = &win->drop_surface[win->drop_surface_count++];
    *drop_surface = (GooeyDropSurface){0};
    drop_surface->core.x = x;
    drop_surface->core.y = y;
    drop_surface->core.width = width;
    drop_surface->core.height = height;
    drop_surface->callback = callback;
    drop_surface->is_file_dropped = false;
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
        GooeyDropSurface *drop_surface = &win->drop_surface[i];

        if (mouseX > drop_surface->core.x && mouseX < drop_surface->core.x + drop_surface->core.width && mouseY > drop_surface->core.y && mouseY < drop_surface->core.y + drop_surface->core.height)
        {
            drop_surface->is_file_dropped = true;
            return true;
        }
    }

    return false;
}

void GooeyDropSurface_Draw(GooeyWindow *win)
{
    for (size_t i = 0; i < win->drop_surface_count; ++i)
    {
        GooeyDropSurface *drop_surface = &win->drop_surface[i];
        unsigned long surface_color = win->active_theme->widget_base;
        const char *shown_text = drop_surface->default_message;
        char filename[64];

        if (drop_surface->is_file_dropped)
        {
            surface_color = win->active_theme->primary;
            __get_filename_from_path(win->current_event->drop_data.file_path, filename, sizeof(filename));
            shown_text = filename;
        }

        active_backend->DrawRectangle(
            drop_surface->core.x, drop_surface->core.y,
            drop_surface->core.width, drop_surface->core.height,
            surface_color, win->creation_id
        );

        int text_width = active_backend->GetTextWidth(shown_text, strlen(shown_text));
        int text_height = active_backend->GetTextHeight(shown_text, strlen(shown_text));

        int text_x = drop_surface->core.x + (drop_surface->core.width - text_width) / 2;
        int text_y = drop_surface->core.y + (drop_surface->core.height + text_height) / 2;

        active_backend->DrawText(text_x, text_y, shown_text, win->active_theme->neutral, 0.26f, win->creation_id);
    }
}

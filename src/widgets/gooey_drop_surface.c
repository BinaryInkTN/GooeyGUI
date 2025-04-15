#include "widgets/gooey_drop_surface.h"
#include "backends/gooey_backend_internal.h"
#include "assets/drop_surface_image.h"
#include "logger/pico_logger_internal.h"

GooeyDropSurface *GooeyDropSurface_Create(int x, int y, int width, int height, char *default_message, void (*callback)(char *mime, char *file_path))
{
    GooeyDropSurface *drop_surface = (GooeyDropSurface *) calloc(1, sizeof(GooeyDropSurface));
    
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
    drop_surface->core.is_visible = true;
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

/*
 Copyright (c) 2025 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "gooey_debug_overlay_internal.h"
#include "backends/gooey_backend_internal.h"

#define OVERLAY_POS 20

void GooeyDebugOverlay_Draw(GooeyWindow *win)
{

    int window_width, window_height;
    active_backend->GetWinDim(&window_width, &window_height, win->creation_id);
    char overlay_text[1024];
    snprintf(overlay_text, sizeof(overlay_text), "FPS %.0lf", active_backend->GetWinFramerate(win->creation_id));
    active_backend->DrawText(OVERLAY_POS, OVERLAY_POS, overlay_text, win->active_theme->primary, 0.3f, win->creation_id);
    
}
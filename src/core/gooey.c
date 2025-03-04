/*
 Copyright (c) 2024 Yassine Ahmed Ali

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

#include "gooey.h"
#include <ctype.h>

GooeyTheme default_theme;
GOOEY_CURSOR currently_set_cursor = GOOEY_CURSOR_ARROW;
int called = 0;

int Gooey_Init(GooeyBackends backend)
{
    unsigned long primaryColor = 0x2196F3;
    unsigned long baseColor = 0xFFFFFF;
    unsigned long neutralColor = 0x000000;
    unsigned long widgetBaseColor = 0xD3D3D3;
    unsigned long dangerColor = 0xE91E63;
    unsigned long infoColor = 0x2196F3;
    unsigned long successColor = 0x00A152;

    default_theme = (GooeyTheme){.base = baseColor, .neutral = neutralColor, .primary = primaryColor, .widget_base = widgetBaseColor, .danger = dangerColor, .info = infoColor, .success = successColor};

    active_theme = &default_theme;

    switch (backend)
    {


    case GLPS:
        LOG_INFO("using GLPS backend.");
        active_backend = &glps_backend;
        default:
        break;
    }
    
    if (active_backend->Init() < 0)
    {
        LOG_CRITICAL("Backend initialization failed.");
        return -1;
    }

    ACTIVE_BACKEND = backend;
    LOG_INFO("Gooey initialized successfully.");

    return 0;
}


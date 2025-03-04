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
#ifndef GOOEY_THEME_INTERNAL_H
#define GOOEY_THEME_INTERNAL_H

#include <cjson/cJSON.h>
#include <stdio.h> 
#include <stdlib.h>

typedef struct
{
    unsigned long base; /* Base window background color */

    unsigned long neutral; /* Neutral color for text */

    unsigned long widget_base; /* Base widget color */

    unsigned long primary; /* Primary color */

    unsigned long danger; /* Danger color */

    unsigned long info; /* Info color */

    unsigned long success; /* Success color */

} GooeyTheme;

GooeyTheme parser_load_theme_from_file(const char* filePath);

#endif
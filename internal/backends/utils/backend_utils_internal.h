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
#ifndef BACKEND_UTILS_INTERNAL_H
#define BACKEND_UTILS_INTERNAL_H

#include "backends/gooey_backend_internal.h"
#include "backends/utils/glad/glad.h"
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include <GLPS/glps_window_manager.h>
#include "backends/utils/linmath/linmath.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    GLuint textureID;
    int width;
    int height;
    int bearingX;
    int bearingY;
    int advance;
} Character;

typedef struct Vertex
{
    vec2 pos;
    vec3 col;
    vec2 texCoord;   // Texture coordinates (u, v)

} Vertex;

static const char *rectangle_vertex_shader =
    "#version 330 core\n"
    "layout(location = 0) in vec2 pos;\n"
    "layout(location = 1) in vec3 col;\n"
    "layout(location = 2) in vec2 texCoord;\n"
    "out vec3 color;\n"
    "out vec2 TexCoord;\n"
    "void main() {\n"
    "    gl_Position = vec4(pos, 0.0, 1.0);\n"
    "    color = col;\n"
    "    TexCoord = texCoord;\n"
    "}\n";

    static const char *rectangle_fragment_shader =
    "#version 330 core\n"
    "in vec3 color;\n"
    "in vec2 TexCoord;\n"
    "out vec4 fragment;\n"
    "uniform sampler2D tex;\n"
    "uniform bool useTexture;\n"  

    "void main() {\n"
    "    if (useTexture) {\n"
    "        fragment = texture(tex, TexCoord) * vec4(color, 1.0);\n"
    "    } else {\n"
    "        fragment = vec4(color, 1.0);\n"
    "    }\n"
    "}\n";


static const char *text_vertex_shader_source = "#version 330 core\n"
                                               "layout(location = 0) in vec4 vertex;\n"
                                               "out vec2 TexCoords;\n"
                                               "uniform mat4 projection;\n"
                                               "void main() {\n"
                                               "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
                                               "    TexCoords = vec2(vertex.z, 1.0-vertex.w);\n"
                                               "}\n";

static const char *text_fragment_shader_source = "#version 330 core\n"
                                                 "in vec2 TexCoords;\n"
                                                 "out vec4 color;\n"
                                                 "uniform sampler2D text;\n"
                                                 "uniform vec3 textColor;\n"
                                                 "void main() {\n"
                                                 "    float alpha = texture(text, TexCoords).r;\n"
                                                 "    color = vec4(textColor, alpha);\n"
                                                 "}\n";
void check_shader_link(GLuint program);
void check_shader_compile(GLuint shader);
void get_window_size(glps_WindowManager* wm, size_t window_id, int *window_width, int *window_height);
void convert_coords_to_ndc(glps_WindowManager* wm, size_t window_id,float *ndc_x, float *ndc_y, int x, int y);
void convert_dimension_to_ndc(glps_WindowManager* wm, size_t window_id, float *ndc_w, float *ndc_h, int width, int height);
void convert_hex_to_rgb(vec3 *rgb, unsigned int color_hex);
const char *LookupString(int keycode);



#endif // BACKEND_UTILS_H

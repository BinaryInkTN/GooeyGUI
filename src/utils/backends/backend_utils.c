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

#include "utils/backends/backend_utils.h"


void check_shader_compile(GLuint shader)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        LOG_ERROR("ERROR: Shader compilation failed\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }
}

void check_shader_link(GLuint program)
{
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        LOG_ERROR("ERROR: Program linking failed\n%s\n", infoLog);
        exit(EXIT_FAILURE);
    }
}

void get_window_size(glps_WindowManager *wm, size_t window_id, int *window_width, int *window_height)
{
    glps_wm_window_get_dimensions(wm, window_id, window_width, window_height);
}
void convert_coords_to_ndc(glps_WindowManager *wm, size_t window_id, float *ndc_x, float *ndc_y, int x, int y)
{
    int window_width, window_height;
    get_window_size(wm, window_id, &window_width, &window_height);

    *ndc_x = (2.0f * x / window_width) - 1.0f;
    *ndc_y = 1.0f - (2.0f * y / window_height);
}

void convert_dimension_to_ndc(glps_WindowManager *wm, size_t window_id, float *ndc_w, float *ndc_h, int width, int height)
{
    int window_width, window_height;
    get_window_size(wm, window_id, &window_width, &window_height);

    *ndc_w = (2.0f * width) / window_width;
    *ndc_h = -(2.0f * height) / window_height;
}

void convert_hex_to_rgb(vec3 *rgb, unsigned int color_hex)
{
    (*rgb)[0] = ((color_hex >> 16) & 0xFF) / 255.0f;
    (*rgb)[1] = ((color_hex >> 8) & 0xFF) / 255.0f;
    (*rgb)[2] = ((color_hex) & 0xFF) / 255.0f;
}

const char *LookupString(int keycode)
{
    switch (keycode)
    {
    case -1:
        return "Unknown";
    case 32:
        return "Space";
    case 39:
        return "'";
    case 44:
        return ",";
    case 45:
        return "-";
    case 46:
        return ".";
    case 47:
        return "/";
    case 48:
        return "0";
    case 49:
        return "1";
    case 50:
        return "2";
    case 51:
        return "3";
    case 52:
        return "4";
    case 53:
        return "5";
    case 54:
        return "6";
    case 55:
        return "7";
    case 56:
        return "8";
    case 57:
        return "9";
    case 59:
        return ";";
    case 61:
        return "=";
    case 65:
        return "a";
    case 66:
        return "b";
    case 67:
        return "c";
    case 68:
        return "d";
    case 69:
        return "e";
    case 70:
        return "f";
    case 71:
        return "g";
    case 72:
        return "h";
    case 73:
        return "i";
    case 74:
        return "j";
    case 75:
        return "k";
    case 76:
        return "l";
    case 77:
        return "m";
    case 78:
        return "n";
    case 79:
        return "o";
    case 80:
        return "p";
    case 81:
        return "q";
    case 82:
        return "r";
    case 83:
        return "s";
    case 84:
        return "t";
    case 85:
        return "u";
    case 86:
        return "v";
    case 87:
        return "w";
    case 88:
        return "x";
    case 89:
        return "y";
    case 90:
        return "z";
    case 91:
        return "[";
    case 92:
        return "\\";
    case 93:
        return "]";
    case 96:
        return "`";
    case 161:
        return "World1";
    case 162:
        return "World2";
    case 256:
        return "Escape";
    case 257:
        return "Enter";
    case 258:
        return "Tab";
    case 259:
        return "Backspace";
    case 260:
        return "Insert";
    case 261:
        return "Delete";
    case 262:
        return "Right";
    case 263:
        return "Left";
    case 264:
        return "Down";
    case 265:
        return "Up";
    case 266:
        return "PageUp";
    case 267:
        return "PageDown";
    case 268:
        return "Home";
    case 269:
        return "End";
    case 280:
        return "CapsLock";
    case 281:
        return "ScrollLock";
    case 282:
        return "NumLock";
    case 283:
        return "PrintScreen";
    case 284:
        return "Pause";
    case 290:
        return "F1";
    case 291:
        return "F2";
    case 292:
        return "F3";
    case 293:
        return "F4";
    case 294:
        return "F5";
    case 295:
        return "F6";
    case 296:
        return "F7";
    case 297:
        return "F8";
    case 298:
        return "F9";
    case 299:
        return "F10";
    case 300:
        return "F11";
    case 301:
        return "F12";
    case 302:
        return "F13";
    case 303:
        return "F14";
    case 304:
        return "F15";
    case 305:
        return "F16";
    case 306:
        return "F17";
    case 307:
        return "F18";
    case 308:
        return "F19";
    case 309:
        return "F20";
    case 310:
        return "F21";
    case 311:
        return "F22";
    case 312:
        return "F23";
    case 313:
        return "F24";
    case 314:
        return "F25";
    case 320:
        return "0";
    case 321:
        return "1";
    case 322:
        return "2";
    case 323:
        return "3";
    case 324:
        return "4";
    case 325:
        return "5";
    case 326:
        return "6";
    case 327:
        return "7";
    case 328:
        return "8";
    case 329:
        return "9";
    case 330:
        return ".";
    case 331:
        return "/";
    case 332:
        return "*";
    case 333:
        return "-";
    case 334:
        return "+";
    case 335:
        return "Enter";
    case 336:
        return "=";
    case 340:
        return "LeftShift";
    case 341:
        return "LeftControl";
    case 342:
        return "LeftAlt";
    case 343:
        return "LeftSuper";
    case 344:
        return "RightShift";
    case 345:
        return "RightControl";
    case 346:
        return "RightAlt";
    case 347:
        return "RightSuper";
    case 348:
        return "Menu";
    default:
        return "Unknown";
    }
}

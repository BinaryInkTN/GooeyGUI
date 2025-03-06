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

typedef struct
{
    //  glpscursor *cursor;
    GLuint *text_programs;
    GLuint shape_program;
    GLuint text_vbo;
    GLuint shape_vbo;
    GLuint *text_vaos;
    GLuint *shape_vaos;
    mat4x4 projection;
    GLuint text_fragment_shader;
    glps_WindowManager *wm;
    GLuint text_vertex_shader;
    Character characters[128];
    char font_path[256];
    size_t active_window_count;
    bool inhibit_reset; /**< useful for continuesly happening events like dragging a slider. */
    unsigned int selected_color;
} GooeyBackendContext;
;

static GooeyBackendContext ctx = {0};

void glps_setup_shared()
{

    glGenBuffers(1, &ctx.text_vbo);

    ctx.text_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(ctx.text_vertex_shader, 1, &text_vertex_shader_source, NULL);
    glCompileShader(ctx.text_vertex_shader);
    check_shader_compile(ctx.text_vertex_shader);

    ctx.text_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(ctx.text_fragment_shader, 1, &text_fragment_shader_source, NULL);
    glCompileShader(ctx.text_fragment_shader);
    check_shader_compile(ctx.text_fragment_shader);

    glGenBuffers(1, &ctx.shape_vbo);

    GLuint shape_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shape_vertex_shader, 1, &rectangle_vertex_shader, NULL);
    glCompileShader(shape_vertex_shader);
    check_shader_compile(shape_vertex_shader);

    GLuint shape_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shape_fragment_shader, 1, &rectangle_fragment_shader, NULL);
    glCompileShader(shape_fragment_shader);
    check_shader_compile(shape_fragment_shader);

    ctx.shape_program = glCreateProgram();
    glAttachShader(ctx.shape_program, shape_vertex_shader);
    glAttachShader(ctx.shape_program, shape_fragment_shader);
    glLinkProgram(ctx.shape_program);
    check_shader_link(ctx.shape_program);

    glDeleteShader(shape_vertex_shader);
    glDeleteShader(shape_fragment_shader);
}

void glps_setup_seperate_vao(int window_id)
{

    ctx.text_programs[window_id] = glCreateProgram();
    glAttachShader(ctx.text_programs[window_id], ctx.text_vertex_shader);
    glAttachShader(ctx.text_programs[window_id], ctx.text_fragment_shader);
    glLinkProgram(ctx.text_programs[window_id]);
    check_shader_link(ctx.text_programs[window_id]);

    GLuint text_vao;
    glGenVertexArrays(1, &text_vao);

    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.text_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    ctx.text_vaos[window_id] = text_vao;
    glBindVertexArray(0);

    GLuint shape_vao;
    glGenVertexArrays(1, &shape_vao);
    glBindVertexArray(shape_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.shape_vbo);

    GLint position_attrib = glGetAttribLocation(ctx.shape_program, "pos");
    glEnableVertexAttribArray(position_attrib);
    glVertexAttribPointer(position_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

    GLint col_attrib = glGetAttribLocation(ctx.shape_program, "col");
    glEnableVertexAttribArray(col_attrib);
    glVertexAttribPointer(col_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, col));
    ctx.shape_vaos[window_id] = shape_vao;
}

void glps_fill_rectangle(int x, int y, int width, int height, long unsigned int color, int window_id)
{

    glps_wm_set_window_ctx_curr(ctx.wm, window_id);
    float ndc_x, ndc_y;
    float ndc_width, ndc_height;
    vec3 color_rgb;

    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x, &ndc_y, x, y);
    convert_dimension_to_ndc(ctx.wm, window_id, &ndc_width, &ndc_height, width, height);
    convert_hex_to_rgb(&color_rgb, color);

    Vertex vertices[6];

    for (int i = 0; i < 6; i++)
    {
        vertices[i].col[0] = color_rgb[0];
        vertices[i].col[1] = color_rgb[1];
        vertices[i].col[2] = color_rgb[2];
    }

    vertices[0].pos[0] = ndc_x;
    vertices[0].pos[1] = ndc_y;
    vertices[1].pos[0] = ndc_x + ndc_width;
    vertices[1].pos[1] = ndc_y;
    vertices[2].pos[0] = ndc_x;
    vertices[2].pos[1] = ndc_y + ndc_height;

    vertices[3].pos[0] = ndc_x + ndc_width;
    vertices[3].pos[1] = ndc_y;
    vertices[4].pos[0] = ndc_x + ndc_width;
    vertices[4].pos[1] = ndc_y + ndc_height;
    vertices[5].pos[0] = ndc_x;
    vertices[5].pos[1] = ndc_y + ndc_height;

    glUseProgram(ctx.shape_program);

    glBindBuffer(GL_ARRAY_BUFFER, ctx.shape_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(ctx.shape_vaos[window_id]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void glps_set_foreground(long unsigned int color)
{
    ctx.selected_color = color;
}

void glps_draw_rectangle(int x, int y, int width, int height, long unsigned int color, int window_id)
{
    glps_wm_set_window_ctx_curr(ctx.wm, window_id);
    float ndc_x, ndc_y;
    float ndc_width, ndc_height;
    vec3 color_rgb;

    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x, &ndc_y, x, y);
    convert_dimension_to_ndc(ctx.wm, window_id, &ndc_width, &ndc_height, width, height);
    convert_hex_to_rgb(&color_rgb, color);

    Vertex vertices[4];

    for (int i = 0; i < 4; i++)
    {
        vertices[i].col[0] = color_rgb[0];
        vertices[i].col[1] = color_rgb[1];
        vertices[i].col[2] = color_rgb[2];
    }

    vertices[0].pos[0] = ndc_x;
    vertices[0].pos[1] = ndc_y;
    vertices[1].pos[0] = ndc_x + ndc_width;
    vertices[1].pos[1] = ndc_y;
    vertices[2].pos[0] = ndc_x + ndc_width;
    vertices[2].pos[1] = ndc_y + ndc_height;

    vertices[3].pos[0] = ndc_x;
    vertices[3].pos[1] = ndc_y + ndc_height;
    glUseProgram(ctx.shape_program);

    glBindBuffer(GL_ARRAY_BUFFER, ctx.shape_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(ctx.shape_vaos[window_id]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void glps_draw_line(int x1, int y1, int x2, int y2, long unsigned int color, int window_id)
{

    glps_wm_set_window_ctx_curr(ctx.wm, window_id);
    float ndc_x1, ndc_y1;
    float ndc_x2, ndc_y2;
    vec3 color_rgb;

    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x1, &ndc_y1, x1, y1);
    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x2, &ndc_y2, x2, y2);

    convert_hex_to_rgb(&color_rgb, color);

    Vertex vertices[2];

    for (int i = 0; i < 2; i++)
    {
        vertices[i].col[0] = color_rgb[0];
        vertices[i].col[1] = color_rgb[1];
        vertices[i].col[2] = color_rgb[2];
    }

    vertices[0].pos[0] = ndc_x1;
    vertices[0].pos[1] = ndc_y1;
    vertices[1].pos[0] = ndc_x2;
    vertices[1].pos[1] = ndc_y2;
    glUseProgram(ctx.shape_program);

    glBindBuffer(GL_ARRAY_BUFFER, ctx.shape_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(ctx.shape_vaos[window_id]);
    glDrawArrays(GL_LINES, 0, 2);
}

void glps_fill_arc(int x_center, int y_center, int width, int height, int angle1, int angle2, int window_id)
{

    glps_wm_set_window_ctx_curr(ctx.wm, window_id);
    const int segments = 10;

    float ndc_x_center, ndc_y_center;
    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x_center, &ndc_y_center, x_center, y_center);

    vec3 color_rgb;
    convert_hex_to_rgb(&color_rgb, ctx.selected_color);

    Vertex vertices[segments + 2];

    vertices[0].pos[0] = ndc_x_center;
    vertices[0].pos[1] = ndc_y_center;
    vertices[0].col[0] = color_rgb[0];
    vertices[0].col[1] = color_rgb[1];
    vertices[0].col[2] = color_rgb[2];

    for (int i = 0; i <= segments; ++i)
    {
        float angle = (float)i / segments * 2.0f * M_PI;
        float x = x_center + (width * 0.5f * cosf(angle));
        float y = y_center + (height * 0.5f * sinf(angle));

        float ndc_x, ndc_y;
        convert_coords_to_ndc(ctx.wm, window_id, &ndc_x, &ndc_y, x, y);

        vertices[i + 1].pos[0] = ndc_x;
        vertices[i + 1].pos[1] = ndc_y;
        vertices[i + 1].col[0] = color_rgb[0];
        vertices[i + 1].col[1] = color_rgb[1];
        vertices[i + 1].col[2] = color_rgb[2];
    }

    glUseProgram(ctx.shape_program);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.shape_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(ctx.shape_vaos[window_id]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);
}

void glps_set_projection(int window_id, int width, int height)
{
    mat4x4 projection;
    mat4x4_ortho(projection, 0.0f, width, height, 0.0f, -1.0f, 1.0f);
    glUseProgram(ctx.text_programs[window_id]);
    glUniformMatrix4fv(glGetUniformLocation(ctx.text_programs[window_id], "projection"), 1, GL_FALSE, (const GLfloat *)projection);
    glBindVertexArray(ctx.text_vaos[window_id]);
}
static void error_callback(int error, const char *description)
{
    LOG_ERROR("Error: %s\n", description);
}

static void keyboard_callback(size_t window_id, bool state, const char *value,
                              void *data)
{

    GooeyWindow **windows = (GooeyWindow**)data;

    windows[window_id]->current_event->type = state ? GOOEY_EVENT_KEY_PRESS : GOOEY_EVENT_KEY_RELEASE;
    windows[window_id]->current_event->key_press.state = state;
    LOG_INFO("%s", value);
    strncpy(windows[window_id]->current_event->key_press.value, value, sizeof(windows[window_id]->current_event->key_press.value));
    windows[window_id]->current_event->attached_window = window_id;
    glps_wm_window_update(ctx.wm, window_id);
}

static void mouse_scroll_callback(size_t window_id, GLPS_SCROLL_AXES axe,
                                  GLPS_SCROLL_SOURCE source, double value,
                                  int discrete, bool is_stopped, void *data)
{
    GooeyWindow **windows = (GooeyWindow**)data;

    windows[window_id]->current_event->attached_window = window_id;
    windows[window_id]->current_event->type = GOOEY_EVENT_MOUSE_SCROLL;
    if (axe == GLPS_SCROLL_H_AXIS)
        windows[window_id]->current_event->mouse_scroll.x = value;
    else
        windows[window_id]->current_event->mouse_scroll.y = value;
}

void mouse_click_callback(size_t window_id, bool state, void *data)
{
    GooeyWindow **windows = (GooeyWindow**)data;
    windows[window_id]->current_event->type = state ? GOOEY_EVENT_CLICK_PRESS : GOOEY_EVENT_CLICK_RELEASE;
    windows[window_id]->current_event->click.x = windows[window_id]->current_event->mouse_move.x;
    windows[window_id]->current_event->click.y = windows[window_id]->current_event->mouse_move.y;
    windows[window_id]->current_event->attached_window = window_id;
    glps_wm_window_update(ctx.wm, window_id);
}

void glps_request_redraw(GooeyWindow* win)
{
    win->current_event->type = GOOEY_EVENT_REDRAWREQ;
    glps_wm_window_update(ctx.wm, win->creation_id);
}

static void mouse_move_callback(size_t window_id, double posX, double posY, void *data)
{
    GooeyWindow **windows = (GooeyWindow**)data;

    windows[window_id]->current_event->attached_window = window_id;
    windows[window_id]->current_event->mouse_move.x = posX;
    windows[window_id]->current_event->mouse_move.y = posY;
}

void window_resize_callback(size_t window_id, int width, int height, void *data)
{
    GooeyWindow **windows = (GooeyWindow**)data;
    LOG_INFO("%d %d %ld", width, height, window_id);
    LOG_INFO("resize %ld %d %d", window_id, width, height);
    glps_wm_set_window_ctx_curr(ctx.wm, window_id);
    glps_set_projection(window_id, width, height); // Update projection matrix
    glViewport(0, 0, width, height);
    windows[window_id]->current_event->attached_window = window_id;
    windows[window_id]->current_event->type = GOOEY_EVENT_RESIZE;
    glps_wm_window_update(ctx.wm, window_id);
}

void window_close_callback(size_t window_id, void *data)
{
    GooeyWindow **windows = (GooeyWindow**)data;

    windows[window_id]->current_event->attached_window = window_id;
    windows[window_id]->current_event->type = GOOEY_EVENT_WINDOW_CLOSE;
    glps_wm_window_update(ctx.wm, window_id);
}

int glps_init_ft()
{

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        LOG_ERROR("Could not initialize FreeType library\n");
        return -1;
    }

    FT_Face face;

    if (FT_New_Face(ft, "roboto.ttf", 0, &face))
    {
        LOG_ERROR("Failed to load font: %s\n", "Roboto");
        FT_Done_FreeType(ft);
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            LOG_ERROR("Failed to load Glyph for character: '%c'\n", c);
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            face->glyph->bitmap_left,
            face->glyph->bitmap_top,
            (int)face->glyph->advance.x};
        ctx.characters[c] = character;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 0;
}

int glps_init()
{

    ctx.inhibit_reset = 0;
    ctx.selected_color = 0x000000;
    ctx.active_window_count = 0;
    ctx.text_vaos = (GLuint *)malloc(sizeof(GLuint) * 100);
    ctx.shape_vaos = (GLuint *)malloc(sizeof(GLuint) * 100);
    ctx.text_programs = (GLuint *)malloc(sizeof(GLuint) * 100);
  
    // glpsSetErrorCallback(error_callback);
    ctx.wm = glps_wm_init();

    return 0;
}

void glps_window_dim(int *width, int *height, int window_id)
{

    get_window_size(ctx.wm, window_id,
                    width, height);
}

int glps_get_current_clicked_window(void)
{
    return -1;
}

void glps_draw_text(int x, int y, const char *text, unsigned long color, float font_size, int window_id)
{
    glps_wm_set_window_ctx_curr(ctx.wm, window_id);
    vec3 color_rgb;
    float ndc_x, ndc_y;
    int window_width, window_height;

    glps_window_dim(&window_width, &window_height, window_id);
    int initial_x = x;

    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x, &ndc_y, x, y);
    convert_hex_to_rgb(&color_rgb, color);
    glUseProgram(ctx.text_programs[window_id]);
    glUniform3f(glGetUniformLocation(ctx.text_programs[window_id], "textColor"), color_rgb[0], color_rgb[1], color_rgb[2]);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(ctx.text_vaos[window_id]);

    int line_count = 0;
    int last_space_pos = -1;
    int split_pos[128];
    int line_height = 20;
    float current_x = x;

    for (size_t i = 0; i < strlen(text); i++)
    {
        Character ch = ctx.characters[(unsigned char)text[i]];
        if (ch.textureID == 0)
            continue;

        float xpos = current_x + ch.bearingX * font_size;

        if (text[i] == ' ')
        {
            last_space_pos = current_x;
        }

        if (xpos + ch.width * font_size > window_width)
        {
            if (last_space_pos != -1)
            {
                split_pos[line_count] = last_space_pos;
                line_count++;
            }
            current_x = initial_x;
            y += (ch.height + ch.bearingY) * font_size;
        }
        else
        {
            current_x += (ch.advance >> 6) * font_size;
        }
    }

    int current_rendering_line = 0;
    current_x = initial_x;

    for (size_t i = 0; i < strlen(text); i++)
    {
        Character ch = ctx.characters[(unsigned char)text[i]];
        if (ch.textureID == 0)
            continue;

        float xpos = current_x + ch.bearingX * font_size;
        float ypos = y - ch.bearingY * font_size;
        float w = ch.width * font_size;
        float h = ch.height * font_size;

        if (current_rendering_line < line_count && xpos >= split_pos[current_rendering_line])
        {
            current_x = initial_x;
            y += (ch.height + ch.bearingY) * font_size + line_height;
            xpos = current_x + ch.bearingX * font_size;
            ypos = y - ch.bearingY * font_size;
            current_rendering_line++;
        }

        if (y + h > window_height)
        {
            break;
        }

        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, ctx.text_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        current_x += (ch.advance >> 6) * font_size;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GooeyWindow glps_create_window(const char *title, int width, int height)

{

    GooeyWindow window = (GooeyWindow){0};

    size_t window_id = glps_wm_window_create(ctx.wm, title, width, height);
    window.creation_id = window_id;

    if (!gladLoadGL())
    {
        LOG_ERROR("Failed to initialize GLAD");
        exit(EXIT_FAILURE);
    }

    glps_init_ft();

    if (window.creation_id == 0)
        glps_setup_shared();

    glps_setup_seperate_vao(window.creation_id);
    glps_set_projection(window.creation_id, width, height);

    ctx.active_window_count++;

    return window;
}

void glps_make_window_visible(int window_id, bool visibility)
{
}

void glps_set_window_resizable(bool value, int window_id)
{
    // glps_wm_window(value, window_id);
}

GooeyWindow glps_spawn_window(const char *title, int width, int height, bool visibility)
{
    /*
    GooeyWindow window;

    //    glpsWindowHint(glps_VISIBLE, is_visible);

    glpswindow *glps_window = glpsCreateWindow(width, height, title, NULL, ctx.window);
    //  glpsSetWindowSizeLimits(glps_window, width, height, glps_DONT_CARE, glps_DONT_CARE);

    // c/tx.child_windows[ctx.active_window_count] = glps_window;
    // window.creation_id = ctx.active_window_count + 1;
    // ctx.active_window_count++;
    glpsMakeContextCurrent(window_id);
    glViewport(0, 0, width, height);
    setup_seperate_vao(window.creation_id);

    set_projection(width, height, window.creation_id);

    glpsSetKeyCallback(glps_window, key_callback);
    glpsSetMouseButtonCallback(glps_window, click_callback);
    glpsSetCursorPosCallback(glps_window, cursor_callback);
    glpsSetWindowRefreshCallback(glps_window,  refresh_callback);
    glpsSetScrollCallback(glps_window, scroll_callback);
    glpsSetFramebufferSizeCallback(glps_window, framebuffer_size_callback);

    // ctx.user_ptrs[window.creation_id] = (userPtr){.id = window.creation_id};

    // glpsSetWindowUserPointer(glps_window, &ctx.user_ptrs[window.creation_id]);

    glpsSwapInterval(1);
    return window;
    */
}

GooeyEvent *glps_handle_events()
{
 
}

void glps_hide_current_child(void)
{

}

void glps_destroy_windows()
{

    /* if (ctx.window)
    {
        glpsDestroyWindow(ctx.window);
        ctx.window = NULL;
    }

    if (ctx.child_windows)
    {

        for (int i = 0; i < ctx.active_window_count; ++i)
        {
            glpsDestroyWindow(ctx.child_windows[i]);
            ctx.child_windows[i] = NULL;
        }
        free(ctx.child_windows);
        ctx.child_windows = NULL;
    }*/
}

void glps_clear(int window_id)
{

    glps_wm_set_window_ctx_curr(ctx.wm, window_id);
    glps_wm_swap_interval(ctx.wm, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    vec3 color;
    convert_hex_to_rgb(&color, active_theme->base);
    glClearColor(color[0], color[1], color[2], 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void glps_cleanup()
{

    /*if (ctx.cursor)
    {
        glpsDestroyCursor(ctx.cursor);
        ctx.cursor = NULL;
    }
*/
    if (ctx.text_vaos)
    {
        free(ctx.text_vaos);
        ctx.text_vaos = NULL;
    }

    if (ctx.shape_vaos)
    {
        free(ctx.shape_vaos);
        ctx.shape_vaos = NULL;
    }

    /*
    if (ctx.user_ptrs)
    {
        free(ctx.user_ptrs);
        ctx.user_ptrs = NULL;
    }
    */

    if (ctx.text_programs)
    {
        free(ctx.text_programs);
        ctx.text_programs = NULL;
    }

    glDeleteShader(ctx.text_vertex_shader);
    glDeleteShader(ctx.text_fragment_shader);

    //   glpsTerminate();
}

void glps_update_background()
{

    for (size_t i = 0; i < ctx.wm->window_count; ++i)
    {
        glps_wm_set_window_ctx_curr(ctx.wm, i);

        vec3 color;
        convert_hex_to_rgb(&color, active_theme->base);
        glClearColor(color[0], color[1], color[2], 1.0f);
    }
}

void glps_render(int window_id)
{
    glps_wm_swap_buffers(ctx.wm, window_id);
}

float glps_get_text_width(const char *text, int length)
{
    float total_width = 0.0f;
    for (int i = 0; i < length; ++i)
    {
        total_width += (ctx.characters[text[i]].advance / 64.0f) * 0.25f;
    }
    return total_width;
}

float glps_get_text_height(const char *text, int length)
{
    float max_height = 0;

    for (int i = 0; i < length; ++i)
    {
        float char_height = ctx.characters[text[i]].height * 0.25f;
        if (char_height > max_height)
        {
            max_height = char_height;
        }
    }
    return max_height;
}

const char *glps_get_key_from_code(GooeyEvent *gooey_event)
{
    return gooey_event->key_press.value;
}

void glps_set_cursor(GOOEY_CURSOR cursor)
{
}

void glps_destroy_window_from_id(int window_id)
{
    glps_wm_window_destroy(ctx.wm, window_id);
    ctx.active_window_count--;
}

void test(size_t window_id, void *data)
{
    LOG_ERROR("RENDER CALLBACK CALLED");
}

void glps_setup_callbacks(void (*callback)(size_t window_id, void *data), void *data)
{

    glps_wm_set_keyboard_callback(ctx.wm, keyboard_callback, data);
    glps_wm_set_mouse_move_callback(ctx.wm, mouse_move_callback, data);
    glps_wm_set_mouse_click_callback(ctx.wm, mouse_click_callback, data);
    glps_wm_set_scroll_callback(ctx.wm, mouse_scroll_callback, data);
    glps_wm_window_set_resize_callback(ctx.wm, window_resize_callback,
                                       data);
    glps_wm_window_set_close_callback(ctx.wm, window_close_callback, data);

    glps_wm_window_set_frame_update_callback(ctx.wm, callback, data);
    LOG_INFO("Set render callback GLPS");
}

void glps_run()
{
    while (!glps_wm_should_close(ctx.wm))
    {
        // glps_wm_window_update(ctx.wm, 0);
    }
}

size_t glps_get_active_window_count()
{
    return ctx.active_window_count;
}

size_t glps_get_total_window_count()
{
    return glps_wm_get_window_count(ctx.wm);
}

void glps_reset_events(GooeyWindow* win)
{
    // Allow only one event at a time, so it doesn't cause redraws.
    win->current_event->type = -1;
}

GooeyBackend glps_backend = {
    .Init = glps_init,
    .Run = glps_run,
    .CreateWindow = glps_create_window,
    .GetActiveWindowCount = glps_get_active_window_count,
    .GetTotalWindowCount = glps_get_total_window_count,
    .SetupCallbacks = glps_setup_callbacks,
    .SpawnWindow = glps_spawn_window,
    .RequestRedraw = glps_request_redraw,
    .GetWinDim = glps_window_dim,
    .DestroyWindows = glps_destroy_windows,
    .DestroyWindowFromId = glps_destroy_window_from_id,
    .MakeWindowResizable = glps_set_window_resizable,
    .GetCurrentClickedWindow = glps_get_current_clicked_window,
    .HideCurrentChild = glps_hide_current_child,
    .MakeWindowVisible = glps_make_window_visible,
    .UpdateBackground = glps_update_background,
    .Cleanup = glps_cleanup,
    .Render = glps_render,
    .HandleEvents = glps_handle_events,
    .ResetEvents = glps_reset_events,
    .FillArc = glps_fill_arc,
    .FillRectangle = glps_fill_rectangle,
    .DrawRectangle = glps_draw_rectangle,
    .DrawLine = glps_draw_line,
    .SetForeground = glps_set_foreground,
    .GetTextWidth = glps_get_text_width,
    .GetTextHeight = glps_get_text_height,
    .DrawText = glps_draw_text,
    .GetKeyFromCode = glps_get_key_from_code,
    .SetCursor = glps_set_cursor,
    .Clear = glps_clear};


#define STB_IMAGE_IMPLEMENTATION
#define NANOSVG_IMPLEMENTATION
#include "backends/utils/nanosvg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "backends/utils/nanosvg/nanosvgrast.h"

#include "backends/utils/backend_utils_internal.h"
#if (TFT_ESPI_ENABLED == 0)
#include "backends/utils/stb_image/stb_image.h"
#include "backends/fonts/roboto.h"
#include "logger/pico_logger_internal.h"
#include <time.h>
#include <nfd.h>
typedef struct
{
    GLuint textureID;
    int width, height;
    int bearingX, bearingY;
    int advance;
} Glyph;
typedef struct
{
    GLuint *text_programs;
    GLuint shape_program;
    GLuint text_vbo;
    GLuint shape_vbo;
    GLuint *text_vaos;
    GLuint *shape_vaos;
    mat4x4 projection;
    GLuint text_fragment_shader;
    glps_WindowManager *wm;
    glps_timer **timers;
    GLuint text_vertex_shader;
    Character characters[128];
    char font_path[256];
    size_t active_window_count;
    size_t timer_count;
    bool inhibit_reset;
    unsigned int selected_color;
    bool is_running;
    FT_Face face;
    Glyph glyph_cache[128]; // simple ASCII cache

} GooeyBackendContext;

static GooeyBackendContext ctx = {0};

static bool validate_window_id(int window_id)
{
    return (window_id >= 0 && window_id < MAX_WINDOWS);
}
void glps_generate_glyphs(int pixel_height)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        LOG_ERROR("Could not initialize FreeType\n");
        return;
    }

    if (FT_New_Memory_Face(ft, roboto_ttf, roboto_ttf_len, 0, &ctx.face))
    {
        LOG_ERROR("Failed to load Roboto font\n");
        FT_Done_FreeType(ft);
        return;
    }

    FT_Set_Pixel_Sizes(ctx.face, 0, pixel_height);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(ctx.face, c, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL))
        {
            LOG_ERROR("Failed to load glyph '%c'\n", c);
            continue;
        }

        FT_Bitmap *bmp = &ctx.face->glyph->bitmap;

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bmp->width, bmp->rows, 0,
                     GL_RED, GL_UNSIGNED_BYTE, bmp->buffer);

        ctx.glyph_cache[c] = (Glyph){
            tex,
            bmp->width,
            bmp->rows,
            ctx.face->glyph->bitmap_left,
            ctx.face->glyph->bitmap_top,
            (ctx.face->glyph->advance.x >> 6)};
    }

    FT_Done_Face(ctx.face);
    FT_Done_FreeType(ft);
}
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
void glps_set_projection(int window_id, int width, int height)
{
    mat4x4 projection;
    mat4x4_ortho(projection, 0.0f, width, height, 0.0f, -1.0f, 1.0f);
    glUseProgram(ctx.text_programs[window_id]);
    glUniformMatrix4fv(glGetUniformLocation(ctx.text_programs[window_id], "projection"), 1, GL_FALSE, (const GLfloat *)projection);
    glBindVertexArray(ctx.text_vaos[window_id]);
    glViewport(0, 0, width, height);
}
void glps_set_viewport(size_t window_id, int width, int height)
{
    glps_wm_set_window_ctx_curr(ctx.wm, window_id);
    glps_set_projection(window_id, width, height);
}
void glps_draw_rectangle(int x, int y, int width, int height,
                         uint32_t color, float thickness,
                         int window_id, bool isRounded, float cornerRadius, GooeyTFT_Sprite *sprite)
{
    if (!validate_window_id(window_id))
        return;

    glps_wm_set_window_ctx_curr(ctx.wm, window_id);

    int win_width, win_height;
    get_window_size(ctx.wm, window_id, &win_width, &win_height);

    float ndc_x, ndc_y;
    float ndc_width, ndc_height;
    vec3 color_rgb;

    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x, &ndc_y, x, y);
    convert_dimension_to_ndc(ctx.wm, window_id, &ndc_width, &ndc_height, width, height);
    convert_hex_to_rgb(&color_rgb, color);

    Vertex vertices[6] = {
        {{ndc_x, ndc_y}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0, 0.0}},
        {{ndc_x + ndc_width, ndc_y}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0, 0.0}},
        {{ndc_x, ndc_y + ndc_height}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0, 1.0}},
        {{ndc_x + ndc_width, ndc_y}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0, 0.0}},
        {{ndc_x + ndc_width, ndc_y + ndc_height}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {1.0, 1.0}},
        {{ndc_x, ndc_y + ndc_height}, {color_rgb[0], color_rgb[1], color_rgb[2]}, {0.0, 1.0}}};

    glUseProgram(ctx.shape_program);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "useTexture"), 0);
    glUniform2f(glGetUniformLocation(ctx.shape_program, "size"), width, height);
    glUniform1f(glGetUniformLocation(ctx.shape_program, "radius"), cornerRadius);
    glUniform1f(glGetUniformLocation(ctx.shape_program, "borderWidth"), thickness);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "isRounded"), isRounded ? 1 : 0);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "isHollow"), 1);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "shapeType"), 0);

    glBindBuffer(GL_ARRAY_BUFFER, ctx.shape_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(ctx.shape_vaos[window_id]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, col));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glps_set_foreground(uint32_t color)
{
    ctx.selected_color = color;
}

void glps_window_dim(int *width, int *height, int window_id)
{
    get_window_size(ctx.wm, window_id, width, height);
}

void glps_draw_line(int x1, int y1, int x2, int y2, uint32_t color, int window_id, GooeyTFT_Sprite *sprite)
{
    if (!validate_window_id(window_id))
        return;

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
    glUniform1i(glGetUniformLocation(ctx.shape_program, "useTexture"), 0);
    glUniform1f(glGetUniformLocation(ctx.shape_program, "borderWidth"), 1.0f);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "isRounded"), 0);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "isHollow"), 1);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "shapeType"), 1);

    glBindBuffer(GL_ARRAY_BUFFER, ctx.shape_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(ctx.shape_vaos[window_id]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, col));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
    glDrawArrays(GL_LINES, 0, 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glps_fill_arc(int x_center, int y_center, int width, int height,
                   int angle1, int angle2, int window_id, GooeyTFT_Sprite *sprite)
{
    if (!validate_window_id(window_id))
        return;

    glps_wm_set_window_ctx_curr(ctx.wm, window_id);
    int win_width, win_height;
    get_window_size(ctx.wm, window_id, &win_width, &win_height);
    const int segments = 80;

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

    float angle1_rad = ((float)angle1) * M_PI / 180.0f;
    float angle2_rad = (float)angle2 * M_PI / 180.0f;
    float angle_range = angle2_rad - angle1_rad;

    float aspect_ratio = (float)height / (float)width;

    for (int i = 0; i <= segments; ++i)
    {
        float t = (float)i / segments;
        float angle = angle1_rad + t * angle_range;

        float x = ndc_x_center - (cos(angle) * 2.0 / win_width) * (width / 2.0);
        float y = ndc_y_center + (sin(angle) * 2.0 / win_height) * (height / 2.0) * aspect_ratio;

        vertices[i + 1].pos[0] = x;
        vertices[i + 1].pos[1] = y;
        vertices[i + 1].col[0] = color_rgb[0];
        vertices[i + 1].col[1] = color_rgb[1];
        vertices[i + 1].col[2] = color_rgb[2];
    }

    glUseProgram(ctx.shape_program);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "useTexture"), GL_FALSE);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "shapeType"), 2);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "isRounded"), GL_FALSE);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "isHollow"), GL_FALSE);

    glBindBuffer(GL_ARRAY_BUFFER, ctx.shape_vbo);
    glBufferData(GL_ARRAY_BUFFER, (segments + 2) * sizeof(Vertex), vertices, GL_DYNAMIC_DRAW);

    glBindVertexArray(ctx.shape_vaos[window_id]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, col));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void glps_draw_image(unsigned int texture_id, int x, int y, int width, int height, int window_id)
{
    if (!validate_window_id(window_id))
        return;

    glps_wm_set_window_ctx_curr(ctx.wm, window_id);

    float ndc_x, ndc_y, ndc_width, ndc_height;
    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x, &ndc_y, x, y);
    convert_dimension_to_ndc(ctx.wm, window_id, &ndc_width, &ndc_height, width, height);

    Vertex vertices[6] = {
        {{ndc_x, ndc_y}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        {{ndc_x + ndc_width, ndc_y}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{ndc_x, ndc_y + ndc_height}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ndc_x + ndc_width, ndc_y}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{ndc_x + ndc_width, ndc_y + ndc_height}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{ndc_x, ndc_y + ndc_height}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}};

    glBindBuffer(GL_ARRAY_BUFFER, ctx.shape_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glUseProgram(ctx.shape_program);

    GLint isRoundedLocation = glGetUniformLocation(ctx.shape_program, "isRounded");
    if (isRoundedLocation != -1)
    {
        glUniform1i(isRoundedLocation, GL_FALSE);
    }

    GLint useTextureLocation = glGetUniformLocation(ctx.shape_program, "useTexture");
    if (useTextureLocation == -1)
    {
        LOG_ERROR("Failed to find useTexture uniform location");
    }
    glUniform1i(useTextureLocation, GL_TRUE);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    GLint textureSamplerLocation = glGetUniformLocation(ctx.shape_program, "tex");
    if (textureSamplerLocation == -1)
    {
        LOG_ERROR("Failed to find tex uniform location");
    }
    glUniform1i(textureSamplerLocation, 1);

    glBindVertexArray(ctx.shape_vaos[window_id]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, col));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void glps_fill_rectangle(int x, int y, int width, int height,
                         uint32_t color, int window_id,
                         bool isRounded, float cornerRadius, GooeyTFT_Sprite *sprite)
{
    if (!validate_window_id(window_id))
        return;

    glps_wm_set_window_ctx_curr(ctx.wm, window_id);
    float ndc_x, ndc_y;
    float ndc_width, ndc_height;
    vec3 color_rgb;

    convert_coords_to_ndc(ctx.wm, window_id, &ndc_x, &ndc_y, x, y);
    convert_dimension_to_ndc(ctx.wm, window_id, &ndc_width, &ndc_height, width, height);
    convert_hex_to_rgb(&color_rgb, color);

    Vertex vertices[6];
    vec2 texCoords[6] = {
        {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

    for (int i = 0; i < 6; i++)
    {
        vertices[i].col[0] = color_rgb[0];
        vertices[i].col[1] = color_rgb[1];
        vertices[i].col[2] = color_rgb[2];
        vertices[i].texCoord[0] = texCoords[i][0];
        vertices[i].texCoord[1] = texCoords[i][1];
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

    glBindBuffer(GL_ARRAY_BUFFER, ctx.shape_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glUseProgram(ctx.shape_program);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "useTexture"), 0);
    glUniform2f(glGetUniformLocation(ctx.shape_program, "size"), width, height);
    glUniform1f(glGetUniformLocation(ctx.shape_program, "radius"), cornerRadius);
    glUniform1f(glGetUniformLocation(ctx.shape_program, "borderWidth"), 1.0f);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "isRounded"), isRounded ? 1 : 0);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "isHollow"), 0);
    glUniform1i(glGetUniformLocation(ctx.shape_program, "shapeType"), 0);

    glBindVertexArray(ctx.shape_vaos[window_id]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, col));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void keyboard_callback(size_t window_id, bool state, const char *value, unsigned long keycode,
                              void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;

    event->type = state ? GOOEY_EVENT_KEY_PRESS : GOOEY_EVENT_KEY_RELEASE;
    event->key_press.state = state;
    LOG_INFO("%s", value);
    strncpy(event->key_press.value, value, sizeof(event->key_press.value));
    event->key_press.keycode = keycode;
}

static void mouse_scroll_callback(size_t window_id, GLPS_SCROLL_AXES axe,
                                  GLPS_SCROLL_SOURCE source, double value,
                                  int discrete, bool is_stopped, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;

    event->type = GOOEY_EVENT_MOUSE_SCROLL;

    if (axe == GLPS_SCROLL_H_AXIS)
        event->mouse_scroll.x = value;
    else
        event->mouse_scroll.y = value;
}

static void mouse_click_callback(size_t window_id, bool state, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;
    GooeyWindow *win = (GooeyWindow *)windows[window_id];
    event->type = state ? GOOEY_EVENT_CLICK_PRESS : GOOEY_EVENT_CLICK_RELEASE;
    event->click.x = event->mouse_move.x;
    event->click.y = event->mouse_move.y;
}

void glps_request_redraw(GooeyWindow *win)
{
    GooeyEvent *event = (GooeyEvent *)win->current_event;
    event->type = GOOEY_EVENT_REDRAWREQ;
}

void glps_force_redraw()
{
}

static void mouse_move_callback(size_t window_id, double posX, double posY, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;
    GooeyWindow *win = (GooeyWindow *)windows[window_id];
    event->mouse_move.x = posX;
    event->mouse_move.y = posY;
}

static void window_resize_callback(size_t window_id, int width, int height, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;
    GooeyWindow *win = (GooeyWindow *)windows[window_id];
    event->type = GOOEY_EVENT_RESIZE;
    win->width = width;
    win->height = height;
    glps_set_viewport(window_id, width, height);
}

static void window_close_callback(size_t window_id, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyEvent *event = (GooeyEvent *)windows[window_id]->current_event;
    GooeyWindow *win = (GooeyWindow *)windows[window_id];
    event->type = GOOEY_EVENT_WINDOW_CLOSE;
}

int glps_init_ft()
{
    gladLoadGL();
    return 0;
}
int glps_init(int project_branch)
{
    NFD_Init();
    set_logging_enabled(project_branch);
    ctx.inhibit_reset = 0;
    ctx.selected_color = 0x000000;
    ctx.active_window_count = 0;
    ctx.text_vaos = (GLuint *)calloc(MAX_WINDOWS, sizeof(GLuint));
    ctx.shape_vaos = (GLuint *)calloc(MAX_WINDOWS, sizeof(GLuint));
    ctx.text_programs = (GLuint *)calloc(MAX_WINDOWS, sizeof(GLuint));
    ctx.wm = glps_wm_init();
    ctx.timers = (glps_timer **)calloc(MAX_TIMERS, sizeof(glps_timer *));
    ctx.timer_count = 0;
    ctx.is_running = true;
    return 0;
}

int glps_get_current_clicked_window(void)
{
    return -1;
}
void glps_draw_text(int x, int y, const char *text, uint32_t color, float font_size, int window_id)
{
    if (!validate_window_id(window_id) || !text)
        return;

    glps_wm_set_window_ctx_curr(ctx.wm, window_id);

    int window_width, window_height;
    glps_window_dim(&window_width, &window_height, window_id);

    vec3 color_rgb;
    convert_hex_to_rgb(&color_rgb, color);

    glUseProgram(ctx.text_programs[window_id]);
    glUniform3f(glGetUniformLocation(ctx.text_programs[window_id], "textColor"),
                color_rgb[0], color_rgb[1], color_rgb[2]);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(ctx.text_vaos[window_id]);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float cursor_x = (float)x;
    float cursor_y = (float)y;
    float scale = font_size / 28.0f;
    float baseline_y = cursor_y;

    for (size_t i = 0; i < strlen(text); i++)
    {
        char c = text[i];
        if (c == '\n')
        {
            cursor_x = (float)x;
            baseline_y += font_size * 1.2f;
            continue;
        }

        Glyph ch = ctx.glyph_cache[(unsigned char)c];
        if (ch.textureID == 0)
            continue;

        float xpos = cursor_x + ch.bearingX * scale;
        float ypos = baseline_y - ch.bearingY * scale;

        float w = ch.width * scale;
        float h = ch.height * scale;

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

        cursor_x += ch.advance * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void glps_unload_image(unsigned int texture_id)
{
    glDeleteTextures(1, &texture_id);
}

static int is_stb_supported_image_format(const char *path)
{
    if (!path)
        return 0;

    const char *ext = strrchr(path, '.');
    if (!ext)
        return 0;

    ext++;

    if (strcasecmp(ext, "png") == 0 ||
        strcasecmp(ext, "jpg") == 0 ||
        strcasecmp(ext, "jpeg") == 0)
    {
        return 1;
    }

    return 0;
}
unsigned int glps_load_image(const char *image_path)
{
    // First check if file exists and is accessible
    FILE *file = fopen(image_path, "rb");
    if (!file)
    {
        LOG_ERROR("Image file not found or inaccessible: %s", image_path);
        return 0;
    }
    fclose(file);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *data = NULL;
    int img_width = 0, img_height = 0, nrChannels = 0;
    int success = 0;

    // Check file extension first
    if (is_stb_supported_image_format(image_path))
    {
        // Use stb_image for common formats (PNG, JPG, BMP, etc.)
        stbi_set_flip_vertically_on_load(1);
        data = stbi_load(image_path, &img_width, &img_height, &nrChannels, 0);
        if (data)
        {
            success = 1;
            LOG_INFO("Loaded image with stb: %s (%dx%d, %d channels)",
                     image_path, img_width, img_height, nrChannels);
        }
    }
    else
    {
        // Try NanoSVG for SVG files
        const char *ext = strrchr(image_path, '.');
        if (ext && (strcasecmp(ext, ".svg") == 0))
        {
            NSVGimage *image = nsvgParseFromFile(image_path, "px", 96);
            if (image)
            {
                NSVGrasterizer *rast = nsvgCreateRasterizer();
                img_width = (int)image->width;
                img_height = (int)image->height;
                nrChannels = 4;

                // Allocate memory for RGBA data
                size_t data_size = img_width * img_height * 4;
                data = malloc(data_size);
                if (data)
                {
                    memset(data, 0, data_size); // Initialize to transparent
                    nsvgRasterize(rast, image, 0, 0, 1, data, img_width, img_height, img_width * 4);
                    success = 1;
                    LOG_INFO("Loaded SVG with NanoSVG: %s (%dx%d)",
                             image_path, img_width, img_height);
                }
                else
                {
                    LOG_ERROR("Failed to allocate memory for SVG rasterization: %s", image_path);
                }

                nsvgDeleteRasterizer(rast);
                nsvgDelete(image);
            }
            else
            {
                LOG_ERROR("NanoSVG failed to parse SVG file: %s", image_path);
            }
        }
        else
        {
            LOG_ERROR("Unsupported image format: %s", image_path);
        }
    }

    if (!success || !data)
    {
        LOG_ERROR("Failed to load image data: %s", image_path);
        glDeleteTextures(1, &texture);

        // Clean up allocated data if any
        if (data && !is_stb_supported_image_format(image_path))
        {
            free(data);
        }
        return 0;
    }

    // Validate image dimensions
    if (img_width <= 0 || img_height <= 0)
    {
        LOG_ERROR("Invalid image dimensions: %s (%dx%d)", image_path, img_width, img_height);
        glDeleteTextures(1, &texture);
        if (is_stb_supported_image_format(image_path))
        {
            stbi_image_free(data);
        }
        else
        {
            free(data);
        }
        return 0;
    }

    // Determine OpenGL format
    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 2)
        format = GL_RG;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    else
    {
        LOG_ERROR("Unsupported number of channels: %d for %s", nrChannels, image_path);
        glDeleteTextures(1, &texture);
        if (is_stb_supported_image_format(image_path))
        {
            stbi_image_free(data);
        }
        else
        {
            free(data);
        }
        return 0;
    }

    // Upload to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, img_width, img_height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Clean up
    if (is_stb_supported_image_format(image_path))
    {
        stbi_image_free(data);
    }
    else
    {
        free(data);
    }

    LOG_INFO("Successfully loaded texture: %s (ID: %u)", image_path, texture);
    return texture;
}

unsigned int glps_load_image_from_bin(unsigned char *data, long unsigned binary_len)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(1);
    int width, height, channels;
    unsigned char *img_data = stbi_load_from_memory(data, binary_len, &width, &height, &channels, 0);
    if (!img_data)
    {
        LOG_ERROR("Failed to load image");
        glDeleteTextures(1, &texture);
        return 0;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(img_data);

    return texture;
}

GooeyWindow *glps_create_window(const char *title, int x, int y, int width, int height)
{
    GooeyWindow *window = (GooeyWindow *)malloc(sizeof(GooeyWindow));

    size_t window_id = glps_wm_window_create(ctx.wm, x, y, title, width, height);
    window->creation_id = window_id;

    glps_init_ft();
    glps_generate_glyphs(28);
    if (window->creation_id == 0)
        glps_setup_shared();

    glps_setup_seperate_vao(window->creation_id);
    ctx.active_window_count++;

    return window;
}

void glps_make_window_visible(int window_id, bool visibility)
{
}

void glps_set_window_resizable(bool value, int window_id)
{
    glps_wm_window_is_resizable(ctx.wm, value, window_id);
}

void glps_hide_current_child(void)
{
}

void glps_destroy_windows()
{
}

void glps_clear(GooeyWindow *win)
{
    size_t window_id = win->creation_id;

    glps_wm_set_window_ctx_curr(ctx.wm, window_id);
    glClear(GL_COLOR_BUFFER_BIT);
    vec3 color;
    convert_hex_to_rgb(&color, win->active_theme->base);
    glClearColor(color[0], color[1], color[2], 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void glps_cleanup()
{
    ctx.is_running = false;

    for (int i = 0; i < 128; i++)
    {
        if (ctx.characters[i].textureID != 0)
        {
            glDeleteTextures(1, &ctx.characters[i].textureID);
        }
    }

    if (ctx.text_vaos)
    {
        for (size_t i = 0; i < ctx.active_window_count; i++)
        {
            if (ctx.text_vaos[i] != 0)
            {
                glDeleteVertexArrays(1, &ctx.text_vaos[i]);
            }
        }
        free(ctx.text_vaos);
        ctx.text_vaos = NULL;
    }

    if (ctx.shape_vaos)
    {
        for (size_t i = 0; i < ctx.active_window_count; i++)
        {
            if (ctx.shape_vaos[i] != 0)
            {
                glDeleteVertexArrays(1, &ctx.shape_vaos[i]);
            }
        }
        free(ctx.shape_vaos);
        ctx.shape_vaos = NULL;
    }

    if (ctx.text_programs)
    {
        for (size_t i = 0; i < ctx.active_window_count; i++)
        {
            if (ctx.text_programs[i] != 0)
            {
                glDeleteProgram(ctx.text_programs[i]);
            }
        }
        free(ctx.text_programs);
        ctx.text_programs = NULL;
    }

    if (ctx.shape_program != 0)
    {
        glDeleteProgram(ctx.shape_program);
        ctx.shape_program = 0;
    }
    if (ctx.text_vertex_shader != 0)
    {
        glDeleteShader(ctx.text_vertex_shader);
        ctx.text_vertex_shader = 0;
    }
    if (ctx.text_fragment_shader != 0)
    {
        glDeleteShader(ctx.text_fragment_shader);
        ctx.text_fragment_shader = 0;
    }

    if (ctx.text_vbo != 0)
    {
        glDeleteBuffers(1, &ctx.text_vbo);
        ctx.text_vbo = 0;
    }
    if (ctx.shape_vbo != 0)
    {
        glDeleteBuffers(1, &ctx.shape_vbo);
        ctx.shape_vbo = 0;
    }

    if (ctx.timers)
    {
        for (size_t i = 0; i < ctx.timer_count; i++)
        {
            if (ctx.timers[i])
            {
                glps_timer_destroy(ctx.timers[i]);
            }
        }
        free(ctx.timers);
        ctx.timers = NULL;
    }

    if (ctx.wm)
    {
        glps_wm_destroy(ctx.wm);
        ctx.wm = NULL;
    }

    NFD_Quit();
}

void glps_update_background(GooeyWindow *win)
{
    if (!win || !win->active_theme)
        return;
    vec3 color;
    glps_wm_set_window_ctx_curr(ctx.wm, win->creation_id);
    convert_hex_to_rgb(&color, win->active_theme->base);
    glClearColor(color[0], color[1], color[2], 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void glps_render(GooeyWindow *win)
{
    glps_wm_swap_buffers(ctx.wm, win->creation_id);
}
float glps_get_text_width(const char *text, int length)
{
    float total_width = 0.0f;
    float scale = 18.0f / 28.0f;

    for (int i = 0; i < length; ++i)
    {
        Glyph ch = ctx.glyph_cache[(int)text[i]];
        if (ch.textureID != 0)
        {
            total_width += ch.advance * scale;
        }
    }
    return total_width;
}

float glps_get_text_height(const char *text, int length)
{
    float max_height = 0;
    float scale = 18.0f / 28.0f;

    for (int i = 0; i < length; ++i)
    {
        Glyph ch = ctx.glyph_cache[(int)text[i]];
        if (ch.textureID != 0 && ch.height > max_height)
        {
            max_height = ch.height * scale;
        }
    }
    return max_height;
}

const char *glps_get_key_from_code(void *gooey_event)
{
    if (!gooey_event)
    {
        LOG_ERROR("Invalid event.");
        return NULL;
    }

    GooeyEvent *event = (GooeyEvent *)gooey_event;

    return event->key_press.value;
}

void glps_set_cursor(GOOEY_CURSOR cursor)
{
    switch (cursor)
    {
    case GOOEY_CURSOR_HAND:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_HAND);
        break;
    case GOOEY_CURSOR_TEXT:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_IBEAM);
        break;
    case GOOEY_CURSOR_ARROW:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_ARROW);
        break;
    case GOOEY_CURSOR_CROSSHAIR:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_CROSSHAIR);
        break;
    case GOOEY_CURSOR_RESIZE_H:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_HRESIZE);
        break;
    case GOOEY_CURSOR_RESIZE_V:
        glps_wm_cursor_change(ctx.wm, GLPS_CURSOR_VRESIZE);
        break;
    default:
        break;
    }
}

void glps_stop_cursor_reset(bool state)
{
    ctx.inhibit_reset = state;
}

void glps_destroy_window_from_id(int window_id)
{
    glps_wm_window_destroy(ctx.wm, window_id);
    ctx.active_window_count--;
}

static void drag_n_drop_callback(size_t origin_window_id, char *mime, char *buff, int x, int y, void *data)
{
    GooeyWindow **windows = (GooeyWindow **)data;
    GooeyWindow *window = windows[origin_window_id];
    GooeyEvent *event = (GooeyEvent *)window->current_event;
    event->type = GOOEY_EVENT_DROP;
    event->drop_data.drop_x = x;
    event->drop_data.drop_y = y;
    strncpy(event->drop_data.file_path, buff, sizeof(event->drop_data.file_path) - 1);
    event->drop_data.file_path[sizeof(event->drop_data.file_path) - 1] = '\0';

    strncpy(event->drop_data.mime, buff, sizeof(event->drop_data.mime) - 1);
    event->drop_data.mime[sizeof(event->drop_data.mime) - 1] = '\0';
    LOG_INFO("%ld", origin_window_id);
    glps_wm_window_update(ctx.wm, window->creation_id);
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
}

void glps_run()
{
    while (!glps_wm_should_close(ctx.wm) && ctx.is_running)
    {
        for (size_t i = 0; i < ctx.active_window_count; ++i)
        {
            glps_wm_window_update(ctx.wm, i);
        }

        for (size_t i = 0; i < ctx.timer_count; ++i)
            glps_timer_check_and_call(ctx.timers[i]);

        usleep(500);
    }
}

GooeyTimer *glps_create_timer()
{
    glps_timer *timer = glps_timer_init();

    ctx.timers[ctx.timer_count++] = timer;

    GooeyTimer *gooey_timer = (GooeyTimer *)calloc(1, sizeof(GooeyTimer));
    gooey_timer->timer_ptr = timer;

    return gooey_timer;
}

void glps_stop_timer(GooeyTimer *timer)
{
    glps_timer_stop((glps_timer *)timer->timer_ptr);
}

void glps_destroy_timer(GooeyTimer *gooey_timer)
{
    if (!gooey_timer || !gooey_timer->timer_ptr)
    {
        return;
    }

    glps_timer *internal_timer = (glps_timer *)gooey_timer->timer_ptr;

    for (size_t i = 0; i < ctx.timer_count; ++i)
    {
        if (ctx.timers[i] == internal_timer)
        {
            glps_timer_destroy(internal_timer);

            memmove(&ctx.timers[i], &ctx.timers[i + 1],
                    (ctx.timer_count - i - 1) * sizeof(glps_timer *));
            ctx.timer_count--;
            ctx.timers[ctx.timer_count] = NULL;

            break;
        }
    }

    free(gooey_timer);
}

void glps_set_callback_for_timer(uint64_t time, GooeyTimer *timer, void (*callback)(void *user_data), void *user_data)
{
    glps_timer_start((glps_timer *)timer->timer_ptr, time, callback, user_data);
}

void glps_window_toggle_decorations(GooeyWindow *win, bool enable)
{
    if (!win)
    {
        LOG_ERROR("Window is null.");
        return;
    }

    glps_wm_toggle_window_decorations(ctx.wm, enable, win->creation_id);
}

size_t glps_get_active_window_count()
{
    return ctx.active_window_count;
}

size_t glps_get_total_window_count()
{
    return glps_wm_get_window_count(ctx.wm);
}

void glps_reset_events(GooeyWindow *win)
{
    GooeyEvent *event = (GooeyEvent *)win->current_event;
    event->type = GOOEY_EVENT_RESET;
}

double glps_get_window_framerate(int window_id)
{
    static struct timespec last_time[MAX_WINDOWS] = {0};
    static double last_fps[MAX_WINDOWS] = {0};
    static struct timespec now;

    if (last_time[window_id].tv_sec == 0 && last_time[window_id].tv_nsec == 0)
    {
        clock_gettime(CLOCK_MONOTONIC, &last_time[window_id]);
        last_fps[window_id] = glps_wm_get_fps(ctx.wm, window_id);
        return last_fps[window_id];
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed = (now.tv_sec - last_time[window_id].tv_sec) +
                     (now.tv_nsec - last_time[window_id].tv_nsec) / 1000000000.0;

    if (elapsed >= 1.0)
    {
        last_fps[window_id] = glps_wm_get_fps(ctx.wm, window_id);
        last_time[window_id] = now;
    }

    return last_fps[window_id];
}

GooeyTFT_Sprite *glps_create_widget_sprite(int x, int y, int width, int height)
{
    return NULL;
}

void glps_redraw_sprite(GooeyTFT_Sprite *sprite)
{
}

void glps_clear_area(int x, int y, int width, int height)
{
}

void glps_clear_old_widget(GooeyTFT_Sprite *sprite)
{
}

void glps_create_view()
{
}

GooeyEvent *glps_get_events(GooeyWindow *window)
{
    return window->current_event;
}

void glps_destroy_ultralight()
{
}

void glps_draw_webview(GooeyWindow *win, void *webview, int x, int y, int width, int height, int window_id)
{
}

void glps_request_close()
{
    ctx.is_running = false;
}

void glps_init_fdialog()
{
}

static nfdwindowhandle_t nfd_get_window_type()
{
    nfdwindowhandle_t parentWindow = {
        .type = glps_wm_get_platform(),
        .handle = glps_wm_window_get_native_ptr(ctx.wm, 0)};
    return parentWindow;
}

void glps_open_fdialog(const char *start_path, nfdu8filteritem_t *filters, size_t filter_count, void (*on_file_selected)(const char *file_path))
{
    nfdwindowhandle_t parentWindow = nfd_get_window_type();

    nfdu8char_t *outPath = NULL;
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = filter_count;
    args.defaultPath = start_path;
    args.parentWindow = parentWindow;

    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result == NFD_OKAY)
    {
        if (on_file_selected && outPath)
            on_file_selected((const char *)outPath);

        NFD_FreePathU8(outPath);
    }
    else if (result == NFD_CANCEL)
    {
        puts("User pressed cancel.");
    }
    else
    {
        printf("Error: %s\n", NFD_GetError());
    }
}

void glps_get_platform_name(char *platform, size_t max_length)
{
    const uint8_t platform_type = glps_wm_get_platform();
    char platform_name[1024];
    switch (platform_type)
    {
    case 0:
    {
        strncpy(platform_name, "Windows", sizeof(platform_name));
        break;
    }
    case 3:
    {
        strncpy(platform_name, "Linux X11", sizeof(platform_name));
        break;
    }
    case 1:
    {
        strncpy(platform_name, "Linux Wayland", sizeof(platform_name));
        break;
    }
    default:
    {
        strncpy(platform_name, "Unknown", sizeof(platform_name));
        break;
    }
    }

    strncpy(platform, platform_name, max_length - 1);
    platform[max_length - 1] = '\0';
}

void glps_make_window_transparent(GooeyWindow *win, int blur_radius, float opacity)
{
    if (!win)
    {
        LOG_ERROR("Window is null.");
        return;
    }

    glps_wm_set_window_background_transparent(ctx.wm, win->creation_id);
    glps_wm_set_window_opacity(ctx.wm, win->creation_id, opacity);
    glps_wm_set_window_blur(ctx.wm, win->creation_id, true, blur_radius);
}

GooeyBackend glps_backend = {
    .Init = glps_init,
    .Run = glps_run,
    .Cleanup = glps_cleanup,
    .SetupCallbacks = glps_setup_callbacks,
    .RequestRedraw = glps_request_redraw,
    .SetViewport = glps_set_viewport,
    .GetActiveWindowCount = glps_get_active_window_count,
    .GetTotalWindowCount = glps_get_total_window_count,
    .CreateGooeyWindow = glps_create_window,
    .MakeWindowVisible = glps_make_window_visible,
    .MakeWindowResizable = glps_set_window_resizable,
    .WindowToggleDecorations = glps_window_toggle_decorations,
    .GetCurrentClickedWindow = glps_get_current_clicked_window,
    .DestroyWindows = glps_destroy_windows,
    .DestroyWindowFromId = glps_destroy_window_from_id,
    .HideCurrentChild = glps_hide_current_child,
    .UpdateBackground = glps_update_background,
    .Clear = glps_clear,
    .Render = glps_render,
    .SetForeground = glps_set_foreground,
    .DrawGooeyText = glps_draw_text,
    .LoadGooeyImage = glps_load_image,
    .LoadImageFromBin = glps_load_image_from_bin,
    .DrawImage = glps_draw_image,
    .FillRectangle = glps_fill_rectangle,
    .DrawRectangle = glps_draw_rectangle,
    .FillArc = glps_fill_arc,
    .GetKeyFromCode = glps_get_key_from_code,
    .ResetEvents = glps_reset_events,
    .GetEvents = glps_get_events,
    .GetWinDim = glps_window_dim,
    .GetWinFramerate = glps_get_window_framerate,
    .DrawLine = glps_draw_line,
    .GetTextWidth = glps_get_text_width,
    .GetTextHeight = glps_get_text_height,
    .SetCursor = glps_set_cursor,
    .UnloadImage = glps_unload_image,
    .CreateTimer = glps_create_timer,
    .SetTimerCallback = glps_set_callback_for_timer,
    .StopTimer = glps_stop_timer,
    .DestroyTimer = glps_destroy_timer,
    .CursorChange = glps_set_cursor,
    .StopCursorReset = glps_stop_cursor_reset,
    .ForceCallRedraw = glps_force_redraw,
    .RequestClose = glps_request_close,
    .CreateSpriteForWidget = glps_create_widget_sprite,
    .RedrawSprite = glps_redraw_sprite,
    .ClearArea = glps_clear_area,
    .ClearOldWidget = glps_clear_old_widget,
    .CreateView = glps_create_view,
    .DestroyUltralight = glps_destroy_ultralight,
    .DrawWebview = glps_draw_webview,
    .OpenFileDialog = glps_open_fdialog,
    .GetPlatformName = glps_get_platform_name,
    .MakeWindowTransparent = glps_make_window_transparent,
};

#endif
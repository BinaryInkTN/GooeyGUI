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

#include "./glad/glad.h"
#include "./linmath.h"
#include <GLPS/glps_window_manager.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

typedef struct {
  GLuint shader_program;
  GLuint VBO, VAO, EBO;
  float angle;
  glps_WindowManager *wm;
} CubeData;

static const float cube_vertices[] = {
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  0.0f,  0.5f, -0.5f, -0.5f, 0.0f,
    1.0f,  0.0f,  0.5f,  0.5f, -0.5f, 0.0f,  0.0f, 1.0f,  -0.5f, 0.5f,
    -0.5f, 1.0f,  1.0f,  0.0f, -0.5f, -0.5f, 0.5f, 1.0f,  0.0f,  1.0f,
    0.5f,  -0.5f, 0.5f,  0.0f, 1.0f,  1.0f,  0.5f, 0.5f,  0.5f,  1.0f,
    1.0f,  1.0f,  -0.5f, 0.5f, 0.5f,  0.0f,  0.0f, 0.0f};

static const unsigned int cube_indices[] = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4,
                                            0, 1, 5, 5, 4, 0, 2, 3, 7, 7, 6, 2,
                                            0, 3, 7, 7, 4, 0, 1, 2, 6, 6, 5, 1};

static const char *vertex_shader_source =
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aColor;\n"
    "out vec3 ourColor;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "}\0";

static const char *fragment_shader_source =
    "#version 330 core\n"
    "in vec3 ourColor;\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(ourColor, 1.0);\n"
    "}\0";

GLuint compile_shader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  int success;
  char info_log[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, info_log);
    LOG_ERROR("Shader compilation failed: %s", info_log);
  }
  return shader;
}

GLuint create_shader_program(const char *vertex_source,
                             const char *fragment_source) {
  GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
  GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  int success;
  char info_log[512];
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, info_log);
    LOG_ERROR("Shader program linking failed: %s", info_log);
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return shader_program;
}

void render_cube(glps_WindowManager *wm, size_t window_id,
                 CubeData *cube_data) {
  glps_wm_set_window_ctx_curr(wm, window_id);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  float radius = 5.0f;
  float camX = sin(cube_data->angle) * radius;
  float camZ = cos(cube_data->angle) * radius;

  mat4x4 view, projection, model;
  vec3 eye = {camX, 0.0f, camZ};
  vec3 center = {0.0f, 0.0f, 0.0f};
  vec3 up = {0.0f, 1.0f, 0.0f};
  mat4x4_look_at(view, eye, center, up);

  int width, height;
  glps_wm_window_get_dimensions(wm, window_id, &width, &height);

  mat4x4_perspective(projection, M_PI / 4.0f, (float)width / (float)height,
                     0.1f, 100.0f);

  mat4x4_identity(model);
  mat4x4_rotate(model, model, cube_data->angle, 0.5f, 1.0f, 0.0f);

  glUseProgram(cube_data->shader_program);
  glUniformMatrix4fv(glGetUniformLocation(cube_data->shader_program, "model"),
                     1, GL_FALSE, (float *)model);
  glUniformMatrix4fv(glGetUniformLocation(cube_data->shader_program, "view"), 1,
                     GL_FALSE, (float *)view);
  glUniformMatrix4fv(
      glGetUniformLocation(cube_data->shader_program, "projection"), 1,
      GL_FALSE, (float *)projection);

  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  cube_data->angle += 0.01f;

  glps_wm_swap_buffers(wm, window_id);
}

void mouse_leave_callback(size_t window_id, void *data) {
  LOG_INFO("Mouse left.");
}

void mouse_scroll_callback(size_t window_id, GLPS_SCROLL_AXES axe,
                           GLPS_SCROLL_SOURCE source, double value,
                           int discrete, bool is_stopped, void *data) {
  LOG_INFO("%s scroll via %s with value %.2lf discrete %d stopped %d",
           axe == GLPS_SCROLL_H_AXIS ? "horizontal" : "vertical",
           source == GLPS_SCROLL_SOURCE_FINGER ? "finger" : "idk", value,
           discrete, is_stopped);
}

void drag_n_drop_callback(size_t origin_window_id, char *mime, char *buff,
                          void *data) {
  LOG_INFO("Origin window id: %ld, Mime: %s, Buffer: %s", origin_window_id,
           mime, buff);
}

void mouse_enter_callback(size_t window_id, double mouse_x, double mouse_y,
                          void *data) {
  LOG_INFO("Mouse entered at x: %lf y:%lf", mouse_x, mouse_y);
}

void mouse_click_callback(size_t window_id, bool state, void *data) {
  if (data == NULL) {
    LOG_ERROR("null");
  } else {
    if (state == true)
      glps_wm_attach_to_clipboard((glps_WindowManager *)data, "text/plain",
                                  "dhiee");
  }

  LOG_INFO("Window %ld Mouse %s", window_id, state ? "pressed" : "released");
}

void mouse_move_callback(size_t window_id, double mouse_x, double mouse_y,
                         void *data) {
  LOG_INFO("x: %lf y:%lf", mouse_x, mouse_y);
}

void keyboard_enter_callback() { LOG_INFO("keyboard entered."); }

void keyboard_callback(size_t window_id, bool state, const char *value,
                       void *data) {
  glps_WindowManager *wm = (glps_WindowManager *)data;
  LOG_INFO("window %ld state: %d value:%s", window_id, state, value);
  char buff[1024];
  glps_wm_get_from_clipboard(wm, buff, 1024);
  LOG_INFO("Clipboard content is: %s", buff);
}

void keyboard_leave_callback(size_t window_id, void *data) {
  LOG_INFO("keyboard left.");
}

void window_resize_callback(size_t window_id, int width, int height,
                            void *data) {
  glViewport(0, 0, width, height);
}

void window_frame_update_callback(size_t window_id, void *data) {
  CubeData *cube_data = (CubeData *)data;
  render_cube(cube_data->wm, window_id, cube_data);
  LOG_INFO("%.2lf FPS", (double)glps_wm_get_fps(cube_data->wm, window_id));

  //  glps_wl_update(cube_data->wm, window_id);
}

void window_close_callback(size_t window_id, void *data) {
  glps_WindowManager *wm = (glps_WindowManager *)data;
  glps_wm_window_destroy(wm, window_id);
}

int main(int argc, char *argv[]) {
  // set_logging_enabled(false);
  glps_WindowManager *wm = glps_wm_init();
  // set_minimum_log_level(DEBUG_LEVEL_WARNING);

  size_t window_id = glps_wm_window_create(wm, "3D Cube Example", 800, 600);
  if (!gladLoadGLLoader((GLADloadproc)glps_get_proc_addr)) {
    fprintf(stderr, "Failed to initialize GLAD\n");
    exit(EXIT_FAILURE);
  }

  glps_wm_set_window_ctx_curr(wm, window_id);
  glEnable(GL_DEPTH_TEST);

  CubeData cube_data;
  cube_data.shader_program =
      create_shader_program(vertex_shader_source, fragment_shader_source);
  cube_data.angle = 0.0f;
  cube_data.wm = wm;

  glGenVertexArrays(1, &cube_data.VAO);
  glGenBuffers(1, &cube_data.VBO);
  glGenBuffers(1, &cube_data.EBO);

  glBindVertexArray(cube_data.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, cube_data.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices,
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_data.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glps_wm_set_keyboard_callback(wm, keyboard_callback, (void *)wm);
  glps_wm_set_mouse_move_callback(wm, mouse_move_callback, (void *)wm);
  glps_wm_set_mouse_click_callback(wm, mouse_click_callback, (void *)wm);
  glps_wm_set_mouse_enter_callback(wm, mouse_enter_callback, (void *)wm);
  glps_wm_set_mouse_leave_callback(wm, mouse_leave_callback, (void *)wm);
  glps_wm_set_keyboard_leave_callback(wm, keyboard_leave_callback, (void *)wm);
  glps_wm_set_keyboard_enter_callback(wm, keyboard_enter_callback, (void *)wm);
  glps_wm_set_scroll_callback(wm, mouse_scroll_callback, (void *)wm);
  glps_wm_window_set_resize_callback(wm, window_resize_callback,
                                     (void *)&cube_data);
  glps_wm_window_set_close_callback(wm, window_close_callback, (void *)wm);
  glps_wm_window_set_frame_update_callback(wm, window_frame_update_callback,
                                           (void *)&cube_data);

  render_cube(wm, window_id, &cube_data);
  while (!glps_wm_should_close(wm)) {
    glps_wm_window_update(wm, window_id);
  }

 glps_wm_destroy(wm);
  return 0;
}

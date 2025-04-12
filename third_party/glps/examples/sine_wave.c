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

#include "glad/glad.h"
#include "linmath.h"
#include <GLPS/glps_window_manager.h>
#include <math.h>
#include <stdio.h>

typedef struct {
  GLuint shader_program;
  GLuint VBO, VAO;
  float angle;
  glps_WindowManager *wm;
} SineWaveData;

static const char *vertex_shader_source =
    "#version 330 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "uniform float time;\n"
    "void main()\n"
    "{\n"
    "   float y = sin(aPos.x + time);\n"
    "   gl_Position = vec4(aPos.x, y, 0.0, 1.0);\n"
    "}\0";

static const char *fragment_shader_source =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(0.0, 0.5, 1.0, 1.0);\n"
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

void render_sine_wave(glps_WindowManager *wm, size_t window_id,
                      SineWaveData *sine_wave_data) {
  glps_wm_set_window_ctx_curr(wm, window_id);

  int width, height;
  glps_wm_window_get_dimensions(wm, window_id, &width, &height);
  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glUseProgram(sine_wave_data->shader_program);
  glUniform1f(glGetUniformLocation(sine_wave_data->shader_program, "time"),
              sine_wave_data->angle);

  glBindVertexArray(sine_wave_data->VAO);
  glDrawArrays(GL_LINE_STRIP, 0, 100);

  glps_wm_swap_buffers(wm, window_id);
  //glps_wl_update(wm, window_id);

  sine_wave_data->angle += 0.01f;
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
  SineWaveData *sine_data = (SineWaveData *)data;
  render_sine_wave(sine_data->wm, window_id, sine_data);
  LOG_INFO("%.2lf FPS", (double)glps_wm_get_fps(sine_data->wm, window_id));
  //glps_wl_update(sine_data->wm, window_id);
}

void window_close_callback(size_t window_id, void *data) {
  glps_WindowManager *wm = (glps_WindowManager *)data;
  glps_wm_window_destroy(wm, window_id);
}

int main(int argc, char *argv[]) {
  glps_WindowManager *wm = glps_wm_init();

  size_t window_id = glps_wm_window_create(wm, "Sine Wave Example", 800, 600);
  
  
  /*if (!gladLoadGL()) {
    fprintf(stderr, "Failed to initialize GLAD\n");
    exit(EXIT_FAILURE);
  }
  glps_wm_set_window_ctx_curr(wm, window_id);

  SineWaveData sine_wave_data;
  sine_wave_data.shader_program =
      create_shader_program(vertex_shader_source, fragment_shader_source);
  sine_wave_data.angle = 0.0f;
  sine_wave_data.wm = wm;

  glGenVertexArrays(1, &sine_wave_data.VAO);
  glGenBuffers(1, &sine_wave_data.VBO);

  glBindVertexArray(sine_wave_data.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, sine_wave_data.VBO);

  float vertices[200];
  for (int i = 0; i < 100; ++i) {
    vertices[2 * i] = -1.0f + 2.0f * i / 99.0f;
    vertices[2 * i + 1] = 0.0f;
  }

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glps_wm_set_keyboard_callback(wm, keyboard_callback, (void *)wm);
  glps_wm_set_mouse_move_callback(wm, mouse_move_callback, (void *)wm);
  glps_wm_set_mouse_click_callback(wm, mouse_click_callback, (void *)wm);
  glps_wm_set_mouse_enter_callback(wm, mouse_enter_callback, (void *)wm);
  glps_wm_set_mouse_leave_callback(wm, mouse_leave_callback, (void *)wm);
  glps_wm_set_keyboard_leave_callback(wm, keyboard_leave_callback, (void *)wm);
  glps_wm_set_keyboard_enter_callback(wm, keyboard_enter_callback, (void *)wm);
  glps_wm_set_scroll_callback(wm, mouse_scroll_callback, (void *)wm);
  glps_wm_window_set_resize_callback(wm, window_resize_callback,
                                     (void *)&sine_wave_data);
  glps_wm_window_set_close_callback(wm, window_close_callback, (void *)wm);
  glps_wm_window_set_frame_update_callback(wm, window_frame_update_callback,
                                           (void *)&sine_wave_data);

  render_sine_wave(wm, window_id, &sine_wave_data);

  while (!glps_wm_should_close(wm)) {
    //glps_wl_update(wm, window_id); // smooth continuous rendering.
    
  }
*/
  glps_wm_destroy(wm);
  return 0;
}

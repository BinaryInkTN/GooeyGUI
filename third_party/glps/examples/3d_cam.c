#include "glad/glad.h"
#include "linmath.h"
#include "glps_wayland.h"
#include "glps_window_manager.h"
#include "utils/logger/pico_logger.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

typedef struct {
  GLuint shader_program;
  GLuint VBO, VAO, EBO;
  float angle;
  glps_WindowManager *wm;
  vec3 camera_pos;
  vec3 camera_front;
  vec3 camera_up;
  double last_x, last_y;
  float yaw, pitch;
  bool first_mouse;
  bool keys[1024];
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
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  mat4x4 view, projection, model;
  vec3 center;
  vec3_add(center, cube_data->camera_pos, cube_data->camera_front);
  mat4x4_look_at(view, cube_data->camera_pos, center, cube_data->camera_up);

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

void window_frame_update_callback(size_t window_id, void *data) {
  CubeData *cube_data = (CubeData *)data;
  render_cube(cube_data->wm, window_id, cube_data);
  LOG_INFO("%.2lf FPS", (double)glps_wm_get_fps(cube_data->wm, window_id));

  glps_wl_update(cube_data->wm, window_id);
}

void mouse_move_callback(size_t window_id, double mouse_x, double mouse_y, void *data) {
  CubeData *cube_data = (CubeData *)data;
  if (cube_data->first_mouse) {
    cube_data->last_x = mouse_x;
    cube_data->last_y = mouse_y;
    cube_data->first_mouse = false;
  }

  float x_offset = mouse_x - cube_data->last_x;
  float y_offset = cube_data->last_y - mouse_y;
  cube_data->last_x = mouse_x;
  cube_data->last_y = mouse_y;

  float sensitivity = 0.1f;
  x_offset *= sensitivity;
  y_offset *= sensitivity;

  cube_data->yaw += x_offset;
  cube_data->pitch += y_offset;

  if (cube_data->pitch > 89.0f)
    cube_data->pitch = 89.0f;
  if (cube_data->pitch < -89.0f)
    cube_data->pitch = -89.0f;

  vec3 front;
  front[0] = cosf(cube_data->yaw * (M_PI / 180.0f)) * cosf(cube_data->pitch * (M_PI / 180.0f));
  front[1] = sinf(cube_data->pitch * (M_PI / 180.0f));
  front[2] = sinf(cube_data->yaw * (M_PI / 180.0f)) * cosf(cube_data->pitch * (M_PI / 180.0f));

  vec3_norm(cube_data->camera_front, front);
}

static inline void vec3_mul_add(vec3 r, vec3 a, float s, vec3 b) {
    r[0] = a[0] * s + b[0];
    r[1] = a[1] * s + b[1];
    r[2] = a[2] * s + b[2];
}

static inline void vec3_mul_sub(vec3 r, vec3 a, float s, vec3 b) {
    r[0] = b[0] - a[0] * s;
    r[1] = b[1] - a[1] * s;
    r[2] = b[2] - a[2] * s;
}
static inline void vec3_set(vec3 r, float x, float y, float z) {
    r[0] = x;
    r[1] = y;
    r[2] = z;
}


void window_resize_callback(size_t window_id, int width, int height,
                            void *data) {
  glViewport(0, 0, width, height);
}

void window_close_callback(size_t window_id, void *data)
{
    CubeData *cube_data = (CubeData*) data;
    glps_wm_window_destroy(cube_data->wm, window_id);
}

void keyboard_callback(size_t window_id, bool state, const char *value,
                       void *data) {
  CubeData *cube_data = (CubeData *)data;
  float camera_speed = 0.1f;

  if (state) {
    if (strcmp(value, "w") == 0) {
      vec3_mul_add(cube_data->camera_pos, cube_data->camera_front, camera_speed,
                   cube_data->camera_pos);
    }
    if (strcmp(value, "s") == 0) {
      vec3_mul_sub(cube_data->camera_pos, cube_data->camera_front, camera_speed,
                   cube_data->camera_pos);
    }
    if (strcmp(value, "a") == 0) {
      vec3 side;
      vec3_mul_cross(side, cube_data->camera_front, cube_data->camera_up);
      vec3_norm(side, side);
      vec3_mul_sub(cube_data->camera_pos, side, camera_speed,
                   cube_data->camera_pos);
    }
    if (strcmp(value, "d") == 0) {
      vec3 side;
      vec3_mul_cross(side, cube_data->camera_front, cube_data->camera_up);
      vec3_norm(side, side);
      vec3_mul_add(cube_data->camera_pos, side, camera_speed,
                   cube_data->camera_pos);
    }
  }
}
int main(int argc, char *argv[]) {
  glps_WindowManager *wm = glps_wm_init();

  size_t window_id = glps_wm_window_create(wm, "3D Game with Camera", 800, 600);

  if (!gladLoadGLLoader((GLADloadproc)glps_get_proc_addr())) {
    fprintf(stderr, "Failed to initialize GLAD\n");
    exit(EXIT_FAILURE);
  }

  glps_wm_set_window_ctx_curr(wm, window_id);
  glEnable(GL_DEPTH_TEST);

  CubeData cube_data;
  cube_data.shader_program =
      create_shader_program(vertex_shader_source, fragment_shader_source);
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

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  cube_data.camera_pos[0] = 0.0f;
  cube_data.camera_pos[1] = 0.0f;
  cube_data.camera_pos[2] = 3.0f;
  cube_data.camera_front[0] = 0.0f;
  cube_data.camera_front[1] = 0.0f;
  cube_data.camera_front[2] = -1.0f;
  cube_data.camera_up[0] = 0.0f;
  cube_data.camera_up[1] = 1.0f;
  cube_data.camera_up[2] = 0.0f;

  cube_data.first_mouse = true;
  cube_data.yaw = -90.0f;
  cube_data.pitch = 0.0f;
  cube_data.wm = wm;

  glps_wm_set_mouse_move_callback(wm, mouse_move_callback, &cube_data);
  glps_wm_set_keyboard_callback(wm, keyboard_callback, &cube_data);
  glps_wm_window_set_frame_update_callback(wm, window_frame_update_callback, &cube_data);
  glps_wm_window_set_close_callback(wm, window_close_callback, &cube_data);
  glps_wm_window_set_resize_callback(wm, window_resize_callback, &cube_data);

  render_cube(wm, window_id, &cube_data);

  while (!glps_wm_should_close(wm)) {
      glps_wl_update(wm, window_id);
  }

  return 0;
}

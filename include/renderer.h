#ifndef RENDERER_H_
#define RENDERER_H_

#include "scene.h"
#include <glad/gl.h>
//
#include <GLFW/glfw3.h>
#include <stdbool.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

#define PI 3.1415926535897932

// renders just pure white
#define DEFAULT_FRAGMENT_SHADER                                                \
  "#version 330 core\nvoid main(){gl_FragColor=vec4(1.0,1.0,1.0,1.0);}"
#define DEFAULT_VERTEX_SHADER                                                  \
  "#version 330 core\nlayout(location = 0) in vec3 aPos;\nvoid "               \
  "main(){gl_Position=vec4(aPos,1.0);}"

// NOTE: all renderer structs are prefixed with `R` as they are intended to be
// used only from within the renderer

typedef struct {
  int num_of_files;
  int *watcher_fds;
  const char **file_names;
} RFilesWatcher;

typedef struct {
  GLuint fbo, fboTex;
} RBackBuffer;

// Renderer's OpenGL buffers for all mesh related data that should be on the GPU
typedef struct {
  int triangle_count, bvh_nodes_count, mats_count;
  GLuint triangles_ssbo, bvh_nodes_ssbo, mats_ssbo, primitives_ssbo;
} RMeshBuffers;

// buffers used internally by the renderer
typedef struct {
  GLuint vbo;
  // vertex array object, describes how to interpret vbo?
  GLuint vao;
} RBuffers;

typedef struct {
  unsigned int iFrame;
  float iResolution[2];
  vec3 cPos, cLookat;
  float cFov;
} RUniforms;

typedef struct {
  RUniforms *uniforms;
  RBuffers *rb;
  RBackBuffer *back_buffer;
  RMeshBuffers *rmb;
} RFrameStructs;

GLFWwindow *setup_opengl(int width, int height, bool disable_vsync);
void setup_renderer_buffers(RBuffers *rb);
GLuint compile_shader(const char *shader_source, GLenum shader_type);
GLuint create_shader_program(const char *vertex_shader_filename,
                             const char *fragment_shader_filename);
GLuint create_shader_program_from_source(const char *vertex_shader_src,
                                         const char *fragment_shader_src);
bool did_shader_change(RFilesWatcher *shader_files_watcher);
void reload_shader(GLuint *shader_program, RFilesWatcher *shader_files_watcher);

void setup_back_buffer(RBackBuffer *bb, unsigned int width,
                       unsigned int height);
void display_fps(GLFWwindow *window, unsigned int *frame_counter,
                 double *last_frame_time);
void update_frame(GLuint shader_program, GLFWwindow *window,
                  const RFrameStructs *rfs);
RUniforms runiforms_new(int width, int height);
RMeshBuffers rmb_build(const Scene *scene);
/* void do_gl_binds(GLuint shader_program, const RMeshBuffers *rmb); */

#endif // RENDERER_H_

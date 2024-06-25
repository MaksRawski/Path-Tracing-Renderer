#ifndef RENDERER_H_
#define RENDERER_H_

#include <glad/gl.h>
//
#include <GLFW/glfw3.h>
#include <stdbool.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

#define PI 3.1415926535897932

// renders just pure white
#define DEFAULT_SHADER_PROGRAM                                                 \
  "#version 330 core\nvoid main(){gl_FragColor=vec4(1.0,1.0,1.0,1.0);}"

void debug();

typedef struct {
  GLuint fbo, fboTex;
} BackBuffer;

typedef struct {
  GLuint vbo;
  GLuint
      vao; // vertex array object, describes how to interpret vbo?, one per mesh
} RendererBuffers;

typedef struct {
  float a[3], b[3], c[3];
  float na[3], nb[3], nc[3];
} Triangle;

typedef struct {
  float emissionColor[3];
  float emissionStrength;
  float albedo[3];
  // NOTE: its value should be normalized
  float specularComponent;
} Material;

typedef struct {
  int firstTriangleIndex;
  int numTriangles;
  int materialIndex;
  float boundsMin[3];
  float boundsMax[3];
} MeshInfo;

typedef struct {
  GLuint tbo_triangles, tbo_meshes, tbo_materials;
  GLuint tbo_tex_triangles, tbo_tex_meshes, tbo_tex_materials;
  Triangle *triangles;
  MeshInfo *meshesInfo;
  Material *materials;
  unsigned int num_of_meshes;
} ModelsBuffer;

typedef struct {
  float camPos[3], camLookat[3], camUp[3], camFov;
} Camera;

typedef struct {
  unsigned int iFrame;
  float iResolution[2];
  float camPos[3], camLookat[3], camUp[3], camFov;
} Uniforms;

GLFWwindow *setup_opengl(bool disable_vsync);
void setup_renderer(const char *shader_filename, GLuint *shader_program,
                    int *shader_watcher_fd, RendererBuffers *rb);
char *read_file(const char *shader_file);
GLuint compile_shader(const char *shaderSource, GLenum shaderType);
GLuint create_shader_program(const char *fragment_shader_source);
bool reload_shader(int watcher_fd, GLuint *program, const char *shader_path);
int watch_shader_file(const char *shader_path);
void setup_back_buffer(GLuint shader_program, BackBuffer *bb, unsigned int width, unsigned int height);
void display_fps(GLFWwindow *window, unsigned int *frame_counter,
                 double *last_frame_time);
void update_frame(GLuint shader_program, GLFWwindow *window, Uniforms *uniforms,
                  RendererBuffers *rb, BackBuffer *back_buffer,
                  ModelsBuffer *mb);

void free_gl_buffers(RendererBuffers *rb, BackBuffer *bb, ModelsBuffer *mb);

#endif // RENDERER_H_

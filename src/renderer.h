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
#define DEFAULT_FRAGMENT_SHADER                                                \
  "#version 330 core\nvoid main(){gl_FragColor=vec4(1.0,1.0,1.0,1.0);}"
#define DEFAULT_VERTEX_SHADER                                                  \
  "#version 330 core\nlayout(location = 0) in vec3 aPos;\nvoid "               \
  "main(){gl_Position=vec4(aPos,1.0);}"

void debug();

typedef struct {
  int num_of_files;
  int *watcher_fds;
  char **file_names;
} FilesWatcher;

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
  float specularComponent;
} Material;

typedef struct {
  float firstTriangleIndex;
  float numTriangles;
  float materialIndex;
  float boundsMin[3];
  float boundsMax[3];
} MeshInfo;

typedef struct {
  GLuint tbo_triangles, tbo_meshes, tbo_materials;
  GLuint tbo_tex_triangles, tbo_tex_meshes, tbo_tex_materials;
  Triangle *triangles;
  MeshInfo *meshesInfo;
  Material *materials;
  unsigned int num_of_triangles, num_of_meshes, num_of_materials;
  unsigned int triangles_capacity, meshes_capacity, materials_capacity;
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
void setup_renderer(char *vertex_shader_filename,
                    char *fragment_shader_filename, GLuint *shader_program,
                    FilesWatcher *shader_watcher, RendererBuffers *rb);
GLuint compile_shader(const char *shader_source, GLenum shader_type);
GLuint create_shader_program(const char *vertex_shader_filename,
                             const char *fragment_shader_filename);
GLuint create_shader_program_from_source(const char *vertex_shader_src,
                                         const char *fragment_shader_src);
bool reload_shader(GLuint *shader_program, FilesWatcher *shader_files_watcher);
void setup_back_buffer(GLuint shader_program, BackBuffer *bb,
                       unsigned int width, unsigned int height);
void display_fps(GLFWwindow *window, unsigned int *frame_counter,
                 double *last_frame_time);
void update_frame(GLuint shader_program, GLFWwindow *window, Uniforms *uniforms,
                  RendererBuffers *rb, BackBuffer *back_buffer,
                  ModelsBuffer *mb);

#endif // RENDERER_H_

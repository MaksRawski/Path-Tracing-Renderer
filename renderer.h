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

typedef struct BackBuffer{
	GLuint fbo, fbo_texture;
} BackBuffer;

typedef struct RendererBuffers {
	GLuint vbo;
	GLuint vao; // vertex array object, describes how to interpret vbo?, one per mesh
} RendererBuffers;

typedef struct Uniforms{
	unsigned int iFrame;
	float iResolution[2];
	GLuint backBufferTexture;
	float camPos[3], camLookat[3], camUp[3], camFov;
} Uniforms;

GLFWwindow * setup_opengl(bool disable_vsync);
void setup_renderer(GLuint *shader_program, int *shader_watcher_fd, RendererBuffers *rb);
char *read_shader_source(const char *shader_file);
GLuint compile_shader(const char *shaderSource, GLenum shaderType);
GLuint create_shader_program(const char *fragment_shader_source);
bool reload_shader(int watcher_fd, GLuint *program, const char *shader_path);
int watch_shader_file(const char *shader_path);
void setup_back_buffer(BackBuffer *bb, unsigned int width,
                       unsigned int height);
void display_fps(GLFWwindow *window, unsigned int frame_counter,
                 double *last_frame_time);
void update_frame(GLuint shader_program, GLFWwindow *window, Uniforms *uniforms, RendererBuffers *rb, BackBuffer *back_buffer) ;

void free_gl_buffers(RendererBuffers *rb, BackBuffer *bb);

#endif // RENDERER_H_

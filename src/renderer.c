#include "renderer.h"
#include "inputs.h"
#include "utils.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#define WINDOW_TITLE "Path Tracing Renderer"

int debug_fn_counter = 0;

void debug() {
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
    fprintf(stderr, "%d. OpenGL Error: %x\n", ++debug_fn_counter, error);
  else
    printf("%d. all good\n", ++debug_fn_counter);
}

// Function to handle glfw errors
void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Error (%d): %s\n", error, description);
}

GLFWwindow *setup_opengl(int width, int height, bool disable_vsync) {
  glfwSetErrorCallback(glfw_error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(width, height, WINDOW_TITLE, NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  int version = gladLoadGL(glfwGetProcAddress);
  if (version == 0) {
    fprintf(stderr, "Failed to initialize OpenGL context\n");
    exit(EXIT_FAILURE);
  }
  void *userDataPtr = calloc(1, sizeof(GLFWUserData));
  glfwSetWindowUserPointer(window, userDataPtr);
  glfwSetKeyCallback(window, key_callback);

  if (glfwRawMouseMotionSupported())
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, cursor_callback);
  glfwSetCursorEnterCallback(window, cursor_enter_callback);

  if (disable_vsync)
    glfwSwapInterval(0);

  // Successfully loaded OpenGL
  printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version),
         GLAD_VERSION_MINOR(version));

  return window;
}

void setup_renderer_buffers(RBuffers *rb) {
  // Define vertices for a full-screen quad
  float vertices[] = {
      -1.0f, 1.0f,  // Top left
      1.0f,  1.0f,  // Top right
      1.0f,  -1.0f, // Bottom right
      -1.0f, -1.0f  // Bottom left
  };

  // generate vertex array object
  glGenVertexArrays(1, &rb->vao);
  // generate vertex buffer object
  glGenBuffers(1, &rb->vbo);

  // bind both buffers (order is important!)
  glBindVertexArray(rb->vao);
  glBindBuffer(GL_ARRAY_BUFFER, rb->vbo);

  // upload vertex data to VBO
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), vertices, GL_STATIC_DRAW);

  // index of the attribute = 0 (will be used in the vertex shader)
  // how many values per vertex = 2,
  // type of each element = GL_FLOAT,
  // do we have coordinates as integers?,
  // stride, space between every vertex
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  // unbind the vbo
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // unbind the vao
  glBindVertexArray(0);
}

void generate_ssbo(GLuint *ssbo, const void *data, int size, int index) {
  glGenBuffers(1, ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, *ssbo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_READ);
  /* glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, *ssbo); */
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, *ssbo, 0, size);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

RMeshBuffers rmb_build(const Scene *scene) {
  RMeshBuffers rmb = {0};

  generate_ssbo(&rmb.triangles_ssbo, scene->triangles,
                scene->triangles_count * sizeof(Triangle), 1);
  generate_ssbo(&rmb.bvh_nodes_ssbo, scene->bvh.nodes,
                scene->bvh.nodes_count * sizeof(BVHnode), 2);
  generate_ssbo(&rmb.mats_ssbo, scene->mats,
                scene->mats_count * sizeof(Material), 3);
  // NOTE: this assumes that primitives are just a LUT for triangles
  generate_ssbo(&rmb.primitives_ssbo, scene->primitives,
                scene->triangles_count * sizeof(Primitive), 4);

  rmb.bvh_nodes_count = scene->bvh.nodes_count;
  rmb.triangle_count = scene->triangles_count;
  rmb.mats_count = scene->mats_count;

  return rmb;
}

void update_frame(GLuint shader_program, GLFWwindow *window,
                  const RFrameStructs *rfs) {
  // setup the program and bind the vao associated with the quad
  // and the vbo holding the vertices of the quad
  glUseProgram(shader_program);
  glBindVertexArray(rfs->rb->vao);

  // render the quad to the back buffer
  glBindFramebuffer(GL_FRAMEBUFFER, rfs->back_buffer->fbo);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rfs->back_buffer->fboTex);
  glUniform1i(glGetUniformLocation(shader_program, "BackBufferTexture"), 0);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  // update "simple" uniforms
  glUniform1i(glGetUniformLocation(shader_program, "iFrame"),
              rfs->uniforms->iFrame);
  glUniform2f(glGetUniformLocation(shader_program, "iResolution"),
              rfs->uniforms->iResolution[0], rfs->uniforms->iResolution[1]);

  glUniform3f(glGetUniformLocation(shader_program, "cPos"),
              rfs->uniforms->cPos.x, rfs->uniforms->cPos.y,
              rfs->uniforms->cPos.z);
  glUniform3f(glGetUniformLocation(shader_program, "cLookat"),
              rfs->uniforms->cLookat.x, rfs->uniforms->cLookat.y,
              rfs->uniforms->cLookat.z);
  glUniform1f(glGetUniformLocation(shader_program, "cFov"),
              rfs->uniforms->cFov);

  // render the quad to the screen
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(rfs->rb->vao);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glfwSwapBuffers(window);
  glfwPollEvents();
}

GLuint compile_shader(const char *shader_source, GLenum shader_type) {
  GLuint shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &shader_source, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    fprintf(stderr, "Error: Shader compilation failed\n%s", infoLog);
    return -1;
  }

  return shader;
}

GLuint create_shader_program(const char *vertex_shader_filename,
                             const char *fragment_shader_filename) {

  char *vertex_shader_src = read_file(vertex_shader_filename);
  char *fragment_shader_src = read_file(fragment_shader_filename);

  GLuint shader_program =
      create_shader_program_from_source(vertex_shader_src, fragment_shader_src);

  free(vertex_shader_src);
  free(fragment_shader_src);

  return shader_program;
}

GLuint create_shader_program_from_source(const char *vertex_shader_src,
                                         const char *fragment_shader_src) {

  GLuint vertex_shader = compile_shader(vertex_shader_src, GL_VERTEX_SHADER);
  GLuint fragment_shader =
      compile_shader(fragment_shader_src, GL_FRAGMENT_SHADER);

  if (vertex_shader == (GLuint)-1 || fragment_shader == (GLuint)-1)
    return -1;

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertex_shader);
  glAttachShader(shaderProgram, fragment_shader);
  glLinkProgram(shaderProgram);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  GLint success;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    fprintf(stderr, "Error: Shader program linking failed\n%s\n", infoLog);
    return -1;
  }

  return shaderProgram;
}

// expects two files to be in shader_files_watcher structure
// first must be a vertex shader and second one a fragment shader
bool did_shader_change(RFilesWatcher *shader_files_watcher) {
  char buffer[BUF_LEN];
  int num_of_events;
  bool did_reload = false;

  if (shader_files_watcher->num_of_files != 2) {
    fprintf(stderr,
            "reload_shader expects a list of two files (vertex shader, "
            "fragment shader) but got %d files\n",
            shader_files_watcher->num_of_files);
    exit(EXIT_FAILURE);
  }
  // iterate through both files
  for (int i = 0; i < shader_files_watcher->num_of_files; ++i) {
    num_of_events = read(shader_files_watcher->watcher_fds[i], buffer, BUF_LEN);
    if (num_of_events < 0 && errno != EAGAIN) {
      fprintf(stderr, "Failed to read file %s, %s\n",
              shader_files_watcher->file_names[i], strerror(errno));
      exit(EXIT_FAILURE);
    } else if (num_of_events > 0) {
      // iterate through all events
      for (int j = 0; j < num_of_events; ++j) {
        struct inotify_event *event = (struct inotify_event *)&buffer[j];
        if (event->mask & IN_MODIFY) {
          printf("%s modified, recompiling...\n",
                 shader_files_watcher->file_names[i]);
          did_reload = true;
        }
        j += EVENT_SIZE + event->len;
      }
    }
  }

  return did_reload;
}

void reload_shader(GLuint *shader_program,
                   RFilesWatcher *shader_files_watcher) {
  GLuint new_program = create_shader_program(
      shader_files_watcher->file_names[0], shader_files_watcher->file_names[1]);

  if (new_program == (GLuint)-1) {
    fprintf(stderr,
            "Failed to create a shader program, loading the default...\n");
    new_program = create_shader_program_from_source(DEFAULT_VERTEX_SHADER,
                                                    DEFAULT_FRAGMENT_SHADER);
  }
  glDeleteProgram(*shader_program);
  *shader_program = new_program;
}

void setup_back_buffer(RBackBuffer *bb, unsigned int width,
                       unsigned int height) {
  glGenFramebuffers(1, &bb->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, bb->fbo);

  // create the texture for the framebuffer
  glGenTextures(1, &bb->fboTex);
  glBindTexture(GL_TEXTURE_2D, bb->fboTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  // attach the texture to our framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         bb->fboTex, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void display_fps(GLFWwindow *window, unsigned int *frame_counter,
                 double *last_frame_time) {
  double current_time = glfwGetTime();
  double delta_time = current_time - *last_frame_time;
  char window_title[40];

  if (delta_time >= 2.0) {
    double fps = *frame_counter / delta_time;
    sprintf(window_title, "%s [%.2f FPS]", WINDOW_TITLE, fps);
    glfwSetWindowTitle(window, window_title);
#ifdef LOG_FPS
    printf("FPS: %.2f\n", fps);
#endif
    *frame_counter = 0;
    *last_frame_time = current_time;
  }
}

const float DEFAULT_CAM_FOV = PI / 4.0; // 45 deg
const vec3 DEFAULT_CAM_POS = {0, 0, 0, 0};
const vec3 DEFAULT_CAM_LOOKAT = {0, 0, 1, 0}; // look down z axis

RUniforms runiforms_new(int width, int height) {
  return (RUniforms){.cFov = DEFAULT_CAM_FOV,
                     .cLookat = DEFAULT_CAM_LOOKAT,
                     .cPos = DEFAULT_CAM_POS,
                     .iFrame = 0,
                     .iResolution = {width, height}};
}

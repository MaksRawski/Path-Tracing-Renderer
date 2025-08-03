#ifndef UTILS_H_
#define UTILS_H_
#include "obj_parser.h"
#include "renderer.h"

// ModelsBuffer
ModelsBuffer mb_init();
int mb_add_triangles(ModelsBuffer *mb, Triangle *triangles, int num_of_triangles);
void mb_add_mesh(ModelsBuffer *mb, ObjInfo info, int first_triangle_index);
void set_material_slot(ModelsBuffer *mb, int index, const Material *mat);
void set_model_material(ModelsBuffer *mb, int model_id, int mat_index);

// FILES
char *read_file(const char *filename);
int watch_file(const char *path);
void delete_file_watcher(FilesWatcher *fw);

// OPENGL
void create_gl_texture(GLuint *tbo, GLuint *tbo_tex, unsigned long size,
                      void *data, GLenum tex_format, GLenum tex_index);
void setup_gl_buffers(ModelsBuffer *mb, GLuint shader_program);
void free_gl_buffers(RendererBuffers *rb, BackBuffer *bb, ModelsBuffer *mb);

#endif // UTILS_H_

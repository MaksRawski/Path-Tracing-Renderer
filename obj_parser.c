#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj_parser.h"

const Material default_mat = {.albedo = {0, 0.4, 0.7},
                              .emissionColor = {0, 0, 0},
                              .emissionStrength = 0,
                              .specularComponent = 0.2};

ObjStats get_obj_stats(const char *filename) {
  ObjStats s = {0, 0, 0};
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Failed to open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  for (char line[255]; fgets(line, sizeof(line), fp);) {
    if (line[0] == 'v' && line[1] == ' ')
      ++s.v;
    else if (line[0] == 'v' && line[1] == 'n')
      ++s.vn;
    else if (line[0] == 'f' && line[1] == ' ')
      ++s.f;
  }
  printf("Loaded %s, stats: v = %d, vn = %d, f = %d\n", filename, s.v, s.vn,
         s.f);
  fclose(fp);
  return s;
}

typedef struct {
  float l[3];
} vec3;

vec3 extract_ve3_from_line(char *line) {
  vec3 res;
  sscanf(line, "%f %f %f", &res.l[0], &res.l[1], &res.l[2]);
  return res;
}

void add_vertex(vec3 vertices[], int *num_of_vertices, char line[]) {
  /* printf("Adding vertex for line = %s", line); */
  vec3 v = extract_ve3_from_line(line);
  vertices[*num_of_vertices].l[0] = v.l[0];
  vertices[*num_of_vertices].l[1] = v.l[1];
  vertices[*num_of_vertices].l[2] = v.l[2];
  /* printf("vertices = %f %f %f\n", vertices[*num_of_vertices].l[0], */
  /*        vertices[*num_of_vertices].l[1], vertices[*num_of_vertices].l[2]);
   */
  (*num_of_vertices)++;
}

void add_triangle(Triangle triangles[], int *num_of_triangles, vec3 vertices[],
                  vec3 vns[], char line[]) {
  Triangle t;
  int v[3], vt[3], vn[3];
  int s;
  s = sscanf(line, "%d %d %d", &v[0], &v[1], &v[2]);
  if (s == 3) {
    t.a[0] = vertices[v[0] - 1].l[0];
    t.a[1] = vertices[v[0] - 1].l[1];
    t.a[2] = vertices[v[0] - 1].l[2];
    triangles[(*num_of_triangles)++] = t;
    return;
  }
  s = sscanf(line, "%d/%d/%d %d/%d/%d %d/%d/%d", //
             &v[0], &vt[0], &vn[0],              //
             &v[1], &vt[1], &vn[1],              //
             &v[2], &vt[2], &vn[2]               //
  );
  if (s == 9) {
    // TODO: would be nicer to use memcpy here
    t.a[0] = vertices[v[0] - 1].l[0];
    t.a[1] = vertices[v[0] - 1].l[1];
    t.a[2] = vertices[v[0] - 1].l[2];

    t.b[0] = vertices[v[1] - 1].l[0];
    t.b[1] = vertices[v[1] - 1].l[1];
    t.b[2] = vertices[v[1] - 1].l[2];

    t.c[0] = vertices[v[2] - 1].l[0];
    t.c[1] = vertices[v[2] - 1].l[1];
    t.c[2] = vertices[v[2] - 1].l[2];

    t.na[0] = vns[vn[0] - 1].l[0];
    t.na[1] = vns[vn[0] - 1].l[1];
    t.na[2] = vns[vn[0] - 1].l[2];

    t.nb[0] = vns[vn[1] - 1].l[0];
    t.nb[1] = vns[vn[1] - 1].l[1];
    t.nb[2] = vns[vn[1] - 1].l[2];

    t.nc[0] = vns[vn[2] - 1].l[0];
    t.nc[1] = vns[vn[2] - 1].l[1];
    t.nc[2] = vns[vn[2] - 1].l[2];

    triangles[(*num_of_triangles)++] = t;
    return;
  }

  fprintf(stderr, "Failed to match on line: %s\n", line);
  exit(EXIT_FAILURE);
}

void parse_obj(const char *filename, Triangle *triangles[],
               int *num_of_triangles, ObjStats *stats) {
  *stats = get_obj_stats(filename);
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Failed to open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  *triangles = malloc(stats->f * sizeof(Triangle));
  vec3 *vertices = malloc(stats->v * sizeof(vec3));
  int num_of_vertices = 0;
  vec3 *vns = malloc(stats->vn * sizeof(vec3));
  int num_of_vns = 0;

  for (char line[255]; fgets(line, sizeof(line), fp);) {
    char type[5];
    sscanf(line, "%s", type);

    if (type[0] == '#')
      continue;
    else if (type[0] == 'v' && type[1] == 0) {
      add_vertex(vertices, &num_of_vertices, line + 2);
      /* printf("Added vertex = %f %f %f\n", vertices[num_of_vertices - 1].l[0],
       */
      /*        vertices[num_of_vertices - 1].l[1], */
      /*        vertices[num_of_vertices - 1].l[2]); */
    } else if (type[0] == 'v' && type[1] == 'n') {
      /* printf("NORMAL\n"); */
      add_vertex(vns, &num_of_vns, line + 2);
      /* printf("Added vertex NORMAL = %f %f %f\n", vns[num_of_vns - 1].l[0], */
      /*        vns[num_of_vns - 1].l[1], vns[num_of_vns - 1].l[2]); */
    } else if (line[0] == 'f') {
      add_triangle(*triangles, num_of_triangles, vertices, vns, line + 2);
    }
  }
  free(vertices);
  free(vns);

  fclose(fp);
}

// expects the shader program to be loaded
void create_gl_buffer(GLuint *tbo, GLuint *tbo_tex, unsigned long size,
                      void *data, GLenum tex_format, GLenum tex_index) {
  glGenBuffers(1, tbo);
  glBindBuffer(GL_ARRAY_BUFFER, *tbo);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

  // texture buffer object, will bind the vbo to it
  // so that the shader can sample the texture and
  // get the data from the vbo
  glGenTextures(1, tbo_tex);
  glBindTexture(GL_TEXTURE_BUFFER, *tbo_tex);
  // depending on the chosen format each "texel" (pixel of a texture)
  // will contain a value in this format so e.g. for GL_RGB32F each texel
  // will store 3 floats AKA vec3
  glTexBuffer(GL_TEXTURE_BUFFER, tex_format, *tbo);

  glActiveTexture(tex_index);
  glBindTexture(GL_TEXTURE_BUFFER, *tbo_tex);
}

void load_obj_model(const char *filename, GLuint shader_program,
                    ModelsBuffer *mb) {
  Triangle *triangles = NULL;
  int num_of_triangles = 0;
  ObjStats stats;
  parse_obj(filename, &triangles, &num_of_triangles, &stats);
  /* #ifdef DEBUG_OBJ_LOADING */
  if (triangles != NULL) {
    for (int i = 0; i < num_of_triangles; i++) {
      printf("tri[%d].a %f %f %f\n", i, triangles[i].a[0], triangles[i].a[1],
             triangles[i].a[2]);
      printf("tri[%d].b %f %f %f\n", i, triangles[i].b[0], triangles[i].b[1],
             triangles[i].b[2]);
      printf("tri[%d].c %f %f %f\n", i, triangles[i].c[0], triangles[i].c[1],
             triangles[i].c[2]);
      printf("tri[%d].na %f %f %f\n", i, triangles[i].na[0], triangles[i].na[1],
             triangles[i].na[2]);
      printf("tri[%d].nb %f %f %f\n", i, triangles[i].nb[0], triangles[i].nb[1],
             triangles[i].nb[2]);
      printf("tri[%d].nc %f %f %f\n", i, triangles[i].nc[0], triangles[i].nc[1],
             triangles[i].nc[2]);
    }
  }
  /* #endif */

  int triangles_already_loaded = 0;
  if (mb == NULL || mb->num_of_meshes == 0) {
    if (mb == NULL)
      mb = malloc(sizeof(ModelsBuffer));
    ModelsBuffer mb_local;
    // allocate memory for REALLOC_EVERY_N_MESHES instances of elements for
    // these arrays, because these structs are small it's better to
    // over-allocate memory than allocate each time
    mb_local.meshesInfo = malloc(4 * sizeof(MeshInfo));
    mb_local.materials = NULL;
    mb_local.triangles = triangles;
    mb_local.num_of_meshes = 1;
    *mb = mb_local;
  } else {
    mb->num_of_meshes += 1;

    // realloc if the initial 4 elements have been filled in and
    // every time num_of_meshes is a power of two
    if (mb->num_of_meshes >= 4 &&
        (mb->num_of_meshes & (mb->num_of_meshes - 1)) == 0) {
      void *result = realloc(mb->meshesInfo,
                             2 * mb->num_of_meshes * sizeof(mb->meshesInfo));
      if (result == NULL) {
        printf("Failed to reallocate %lu bytes of memory for meshesInfo!\n",
               2 * mb->num_of_meshes * sizeof(mb->meshesInfo));
        free(mb->meshesInfo);
        exit(EXIT_FAILURE);
      }
      mb->meshesInfo = result;
    }

    triangles_already_loaded =
        mb->meshesInfo[mb->num_of_meshes - 1].firstTriangleIndex +
        mb->meshesInfo[mb->num_of_meshes - 1].numTriangles;

    // because triangles buffer will be of quite unpredictable size and
    // potentially large we will realloc memory per every new model and only
    // as much as needed
    void *result =
        realloc(mb->triangles, (triangles_already_loaded + num_of_triangles) *
                                   sizeof(Triangle));
    if (result == NULL) {
      free(mb->triangles);
      fprintf(stderr,
              "Failed to reallocate %lu bytes of memory for triangles!\n",
              (triangles_already_loaded + num_of_triangles) * sizeof(Triangle));
      exit(EXIT_FAILURE);
    }
    mb->triangles = result;
    memcpy(mb->triangles + triangles_already_loaded, triangles,
           num_of_triangles);

    // delete previous buffers and textures as we will set them later
    // with new data
    glDeleteBuffers(1, &mb->tbo_triangles);
    glDeleteBuffers(1, &mb->tbo_meshes);
    glDeleteTextures(1, &mb->tbo_tex_triangles);
    glDeleteTextures(1, &mb->tbo_tex_meshes);
  }

  // set the mesh info for the new model
  MeshInfo mi = {
      .firstTriangleIndex = triangles_already_loaded,
      .numTriangles = num_of_triangles,
      .materialIndex = 0, // set to default material
      // TODO: use BVH here instead
      .boundsMax = {2, 2, 2},
      .boundsMin = {-2, -2, -2},
  };
  mb->meshesInfo[mb->num_of_meshes - 1] = mi;

  // set the default material if there are no defined
  if (mb->materials == NULL) {
    mb->materials = malloc(sizeof(Material));
    mb->materials[0] = default_mat;
    create_gl_buffer(&mb->tbo_materials, &mb->tbo_tex_materials,
                     sizeof(Material), mb->materials, GL_RGBA32F, GL_TEXTURE3);
    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "materialsBuffer"), 3);
  }

  // create texture buffers for triangles and meshesInfo
  glUseProgram(shader_program);
  create_gl_buffer(&mb->tbo_triangles, &mb->tbo_tex_triangles,
                   (triangles_already_loaded + num_of_triangles) *
                       sizeof(Triangle),
                   mb->triangles, GL_RGB32F, GL_TEXTURE1);
  glUniform1i(glGetUniformLocation(shader_program, "trianglesBuffer"), 1);

  create_gl_buffer(&mb->tbo_meshes, &mb->tbo_tex_meshes,
                   mb->num_of_meshes * sizeof(MeshInfo), mb->meshesInfo,
                   GL_RGB32F, GL_TEXTURE2);

  glUniform1i(glGetUniformLocation(shader_program, "meshesInfoBuffer"), 2);
  glUniform1i(glGetUniformLocation(shader_program, "numOfMeshes"),
              mb->num_of_meshes);

  // unbind the texture buffer
  glBindBuffer(GL_TEXTURE_BUFFER, 0);
  // TODO: is there more to unbind?
}

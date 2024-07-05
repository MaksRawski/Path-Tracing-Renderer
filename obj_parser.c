#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj_parser.h"

#define INFINITY 1.0e30

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

vec3 min(const vec3 a, const vec3 b) {
  vec3 res = {0};
  res.l[0] = a.l[0] <= b.l[0] ? a.l[0] : b.l[0];
  res.l[1] = a.l[1] <= b.l[1] ? a.l[1] : b.l[1];
  res.l[2] = a.l[2] <= b.l[2] ? a.l[2] : b.l[2];
  return res;
}

vec3 max(const vec3 a, const vec3 b) {
  vec3 res = {0};
  res.l[0] = a.l[0] >= b.l[0] ? a.l[0] : b.l[0];
  res.l[1] = a.l[1] >= b.l[1] ? a.l[1] : b.l[1];
  res.l[2] = a.l[2] >= b.l[2] ? a.l[2] : b.l[2];
  return res;
}

void calculate_bounds(vec3 vertices[], int num_of_vertices, vec3 *bounds_min,
                      vec3 *bounds_max) {
  if (bounds_min == NULL)
    bounds_min = malloc(sizeof(vec3));

  bounds_min->l[0] = INFINITY;
  bounds_min->l[1] = INFINITY;
  bounds_min->l[2] = INFINITY;

  if (bounds_max == NULL)
    bounds_max = malloc(sizeof(vec3));

  bounds_max->l[0] = -INFINITY;
  bounds_max->l[1] = -INFINITY;
  bounds_max->l[2] = -INFINITY;

  for (int i = 0; i < num_of_vertices; ++i) {
    *bounds_min = min(*bounds_min, vertices[i]);
    *bounds_max = max(*bounds_max, vertices[i]);
  }
  // HACK: add extra width to every flat triangle
  for (int i = 0; i < 3; ++i) {
    if (bounds_min->l[i] == bounds_max->l[i]) {
      bounds_min->l[i] -= 0.00001;
      bounds_max->l[i] += 0.00001;
    }
  }
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
               int *num_of_triangles, ObjStats *stats, vec3 *bounds_min,
               vec3 *bounds_max) {
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
  calculate_bounds(vertices, num_of_vertices, bounds_min, bounds_max);
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

void offset_triangles(Triangle *triangles, int num_of_triangles, vec3 offset) {
  for (int i = 0; i < num_of_triangles; ++i) {
    triangles[i].a[0] += offset.l[0];
    triangles[i].a[1] += offset.l[1];
    triangles[i].a[2] += offset.l[2];

    triangles[i].b[0] += offset.l[0];
    triangles[i].b[1] += offset.l[1];
    triangles[i].b[2] += offset.l[2];

    triangles[i].c[0] += offset.l[0];
    triangles[i].c[1] += offset.l[1];
    triangles[i].c[2] += offset.l[2];

    triangles[i].na[0] += offset.l[0];
    triangles[i].na[1] += offset.l[1];
    triangles[i].na[2] += offset.l[2];

    triangles[i].nb[0] += offset.l[0];
    triangles[i].nb[1] += offset.l[1];
    triangles[i].nb[2] += offset.l[2];

    triangles[i].nc[0] += offset.l[0];
    triangles[i].nc[1] += offset.l[1];
    triangles[i].nc[2] += offset.l[2];
  }
}

// returns index to meshes info
int load_obj_model(const char *filename, GLuint shader_program,
                   ModelsBuffer *mb, vec3 *offset_from_origin) {
  Triangle *triangles = NULL;
  int num_of_triangles = 0;
  ObjStats stats;

  vec3 bounds_min, bounds_max;
  parse_obj(filename, &triangles, &num_of_triangles, &stats, &bounds_min,
            &bounds_max);

  if (offset_from_origin != NULL)
    offset_triangles(triangles, num_of_triangles, *offset_from_origin);

  /* #ifdef DEBUG_OBJ_LOADING */
  /* if (triangles != NULL) { */
  /*   for (int i = 0; i < num_of_triangles; i++) { */
  /*     printf("tri[%d].a %f %f %f\n", i, triangles[i].a[0], triangles[i].a[1], */
  /*            triangles[i].a[2]); */
  /*     printf("tri[%d].b %f %f %f\n", i, triangles[i].b[0], triangles[i].b[1], */
  /*            triangles[i].b[2]); */
  /*     printf("tri[%d].c %f %f %f\n", i, triangles[i].c[0], triangles[i].c[1], */
  /*            triangles[i].c[2]); */
  /*     printf("tri[%d].na %f %f %f\n", i, triangles[i].na[0], triangles[i].na[1], */
  /*            triangles[i].na[2]); */
  /*     printf("tri[%d].nb %f %f %f\n", i, triangles[i].nb[0], triangles[i].nb[1], */
  /*            triangles[i].nb[2]); */
  /*     printf("tri[%d].nc %f %f %f\n", i, triangles[i].nc[0], triangles[i].nc[1], */
  /*            triangles[i].nc[2]); */
  /*   } */
  /* } */
  printf("bounds_min = %f %f %f, bounds_max = %f %f %f\n", bounds_min.l[0],
         bounds_min.l[1], bounds_min.l[2], bounds_max.l[0], bounds_max.l[1],
         bounds_max.l[2]);
  /* #endif */

  int triangles_already_loaded = 0;
  if (mb == NULL || mb->num_of_meshes == 0) {
    if (mb == NULL)
      mb = malloc(sizeof(ModelsBuffer));
    ModelsBuffer mb_local;
    mb_local.meshesInfo = malloc(4 * sizeof(MeshInfo));
    mb_local.materials = NULL;
    mb_local.num_of_materials = 0;
    mb_local.triangles = triangles;
    mb_local.num_of_meshes = 1;
    *mb = mb_local;
  } else {
    triangles_already_loaded =
        mb->meshesInfo[mb->num_of_meshes - 1].firstTriangleIndex +
        mb->meshesInfo[mb->num_of_meshes - 1].numTriangles;

    mb->num_of_meshes += 1;

    // realloc if the initial 4 elements have been filled in and
    // every time num_of_meshes is a power of two
    if (mb->num_of_meshes >= 4 &&
        (mb->num_of_meshes & (mb->num_of_meshes - 1)) == 0) {
      void *result =
          realloc(mb->meshesInfo, 2 * mb->num_of_meshes * sizeof(MeshInfo));
      if (result == NULL) {
        fprintf(stderr,
                "Failed to reallocate %zu bytes of memory for meshesInfo!\n",
                2 * mb->num_of_meshes * sizeof(MeshInfo));
        free(mb->meshesInfo);
        exit(EXIT_FAILURE);
      }
      mb->meshesInfo = result;
    }

    // because triangles buffer will be of quite unpredictable size and
    // potentially large we will realloc memory per every new model and only
    // as much as needed
    void *result =
        realloc(mb->triangles, (triangles_already_loaded + num_of_triangles) *
                                   sizeof(Triangle));
    if (result == NULL) {
      free(mb->triangles);
      fprintf(stderr,
              "Failed to reallocate %zu bytes of memory for triangles!\n",
              (triangles_already_loaded + num_of_triangles) * sizeof(Triangle));
      exit(EXIT_FAILURE);
    } else
      mb->triangles = result;

    memcpy(mb->triangles + triangles_already_loaded, triangles,
           num_of_triangles * sizeof(Triangle));

    // delete previous buffers and textures as we will set them later
    // with new data
    glDeleteBuffers(1, &mb->tbo_triangles);
    glDeleteTextures(1, &mb->tbo_tex_triangles);
    glDeleteBuffers(1, &mb->tbo_meshes);
    glDeleteTextures(1, &mb->tbo_tex_meshes);
  }

  // set the mesh info for the new model
  MeshInfo mi = {
      .firstTriangleIndex = triangles_already_loaded,
      .numTriangles = num_of_triangles,
      .materialIndex = 0, // default material will be stored at index 0
      .boundsMin = {bounds_min.l[0], bounds_min.l[1], bounds_min.l[2]},
      .boundsMax = {bounds_max.l[0], bounds_max.l[1], bounds_max.l[2]}};

  if (offset_from_origin != NULL) {
    mi.boundsMin[0] += offset_from_origin->l[0];
    mi.boundsMin[1] += offset_from_origin->l[1];
    mi.boundsMin[2] += offset_from_origin->l[2];

    mi.boundsMax[0] += offset_from_origin->l[0];
    mi.boundsMax[1] += offset_from_origin->l[1];
    mi.boundsMax[2] += offset_from_origin->l[2];
  }

  mb->meshesInfo[mb->num_of_meshes - 1] = mi;

  // set the default material if there are no defined
  if (mb->materials == NULL || mb->num_of_materials == 0) {
    mb->num_of_materials = 1;
    mb->materials = malloc(4 * sizeof(Material));
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
  // TODO: should we unbind anything else?

  return mb->num_of_meshes - 1;
}

void set_obj_pos(ModelsBuffer *mb, int model_id, vec3 pos) {
  if ((unsigned int)model_id >= mb->num_of_meshes) {
    fprintf(stderr, "Tried to modify model %d but there are only %d models!\n",
            model_id, mb->num_of_meshes);
    exit(EXIT_FAILURE);
  }

  offset_triangles(mb->triangles +
                       (int)mb->meshesInfo[model_id].firstTriangleIndex,
                   mb->meshesInfo[model_id].numTriangles, pos);

  // TODO: also offset BVH here
  mb->meshesInfo[model_id].boundsMin[0] += pos.l[0];
  mb->meshesInfo[model_id].boundsMin[1] += pos.l[1];
  mb->meshesInfo[model_id].boundsMin[2] += pos.l[2];

  mb->meshesInfo[model_id].boundsMax[0] += pos.l[0];
  mb->meshesInfo[model_id].boundsMax[1] += pos.l[1];
  mb->meshesInfo[model_id].boundsMax[2] += pos.l[2];

  int overall_num_of_triangles =
      mb->meshesInfo[mb->num_of_meshes - 1].firstTriangleIndex +
      mb->meshesInfo[mb->num_of_meshes - 1].numTriangles;

  glDeleteBuffers(1, &mb->tbo_triangles);
  glDeleteTextures(1, &mb->tbo_tex_triangles);

  create_gl_buffer(&mb->tbo_triangles, &mb->tbo_tex_triangles,
                   overall_num_of_triangles * sizeof(Triangle), mb->triangles,
                   GL_RGB32F, GL_TEXTURE1);
}

void set_material_slot(ModelsBuffer *mb, int index, const Material *mat) {
  // if we try to set a material that has a index bigger than 4
  // that are allocated initially (or are allocated overall),
  // we should reallocate but only when the num_of_materials
  // is a power of 2 and then we increase the size by multiplying by 2
  if (index >= 4 && (unsigned int)index >= mb->num_of_materials &&
      (mb->num_of_materials & (mb->num_of_materials - 1)) == 0) {
    void *result = realloc(mb->materials, 2 * index * sizeof(Material));
    if (result == NULL) {
      fprintf(stderr,
              "Failed to realocate %zu bytes of memory for materials!\n",
              2 * index * sizeof(Material));
      exit(EXIT_FAILURE);
    } else
      mb->materials = result;
  }
  if ((unsigned int)index + 1 > mb->num_of_materials)
    mb->num_of_materials = index + 1;
  mb->materials[index] = *mat;
  glDeleteBuffers(1, &mb->tbo_materials);
  glDeleteTextures(1, &mb->tbo_tex_materials);

  create_gl_buffer(&mb->tbo_materials, &mb->tbo_tex_materials,
                   mb->num_of_materials * sizeof(Material), mb->materials,
                   GL_RGBA32F, GL_TEXTURE3);
}

void set_model_material(ModelsBuffer *mb, int model_id, int mat_index) {
  if ((unsigned int)model_id >= mb->num_of_meshes) {
    fprintf(stderr, "No loaded model has ID %d!\n", model_id);
    exit(EXIT_FAILURE);
  }
  if ((unsigned int)mat_index >= mb->num_of_materials) {
    fprintf(stderr, "No material has ID %d!\n", model_id);
    exit(EXIT_FAILURE);
  }

  mb->meshesInfo[model_id].materialIndex = mat_index;

  glDeleteBuffers(1, &mb->tbo_meshes);
  glDeleteTextures(1, &mb->tbo_tex_meshes);

  create_gl_buffer(&mb->tbo_meshes, &mb->tbo_tex_meshes,
                   mb->num_of_meshes * sizeof(MeshInfo), mb->meshesInfo,
                   GL_RGB32F, GL_TEXTURE2);
}

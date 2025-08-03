#include "obj_parser.h"
#include "renderer.h"
#include "utils.h"
#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>

#define INFINITY 1.0e30

const Material default_mat = {.albedo = {0, 0.4, 0.7},
                              .emissionColor = {0, 0, 0},
                              .emissionStrength = 0,
                              .specularComponent = 0.2};

// NOTE: doesn't populate bounds
ObjInfo get_obj_info_basic(const char *filename) {
  ObjInfo s = {0};
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

// will populate bounds_min and bounds_max in info
void calculate_bounds(vec3 vertices[], int num_of_vertices, ObjInfo *info) {
  info->bounds_min.l[0] = INFINITY;
  info->bounds_min.l[1] = INFINITY;
  info->bounds_min.l[2] = INFINITY;

  info->bounds_max.l[0] = -INFINITY;
  info->bounds_max.l[1] = -INFINITY;
  info->bounds_max.l[2] = -INFINITY;

  for (int i = 0; i < num_of_vertices; ++i) {
    info->bounds_min = min(info->bounds_min, vertices[i]);
    info->bounds_max = max(info->bounds_max, vertices[i]);
  }
  // HACK: add extra width to every flat triangle
  for (int i = 0; i < 3; ++i) {
    if (info->bounds_min.l[i] == info->bounds_max.l[i]) {
      info->bounds_min.l[i] -= 0.00001;
      info->bounds_max.l[i] += 0.00001;
    }
  }
}

// parses line and fills all the other parameters with information from it
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

// loads obj from filename into triangles array and return info about the obj
ObjInfo parse_obj(const char *filename, Triangle *triangles[]) {
  ObjInfo info = get_obj_info_basic(filename);
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Failed to open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  *triangles = malloc(info.f * sizeof(Triangle));
  vec3 *vertices = malloc(info.v * sizeof(vec3));
  int num_of_vertices = 0;
  vec3 *vns = malloc(info.vn * sizeof(vec3));
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
      add_triangle(*triangles, &info.f, vertices, vns, line + 2);
    }
  }
  calculate_bounds(vertices, num_of_vertices, &info);
  free(vertices);
  free(vns);

  fclose(fp);
  return info;
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
// expects mb to be initialized with `models_buffer_init`
int load_obj_model(const char *filename, ModelsBuffer *mb,
                   vec3 *offset_from_origin) {
  Triangle *triangles = NULL;
  ObjInfo info = parse_obj(filename, &triangles);

  if (offset_from_origin != NULL) {
    offset_triangles(triangles, info.f, *offset_from_origin);
    info.bounds_min.l[0] += offset_from_origin->l[0];
    info.bounds_min.l[1] += offset_from_origin->l[1];
    info.bounds_min.l[2] += offset_from_origin->l[2];

    info.bounds_max.l[0] += offset_from_origin->l[0];
    info.bounds_max.l[1] += offset_from_origin->l[1];
    info.bounds_max.l[2] += offset_from_origin->l[2];
  }

  /* #ifdef DEBUG_OBJ_LOADING */
  /* if (triangles != NULL) { */
  /*   for (int i = 0; i < num_of_triangles; i++) { */
  /*     printf("tri[%d].a %f %f %f\n", i, triangles[i].a[0], triangles[i].a[1],
   */
  /*            triangles[i].a[2]); */
  /*     printf("tri[%d].b %f %f %f\n", i, triangles[i].b[0], triangles[i].b[1],
   */
  /*            triangles[i].b[2]); */
  /*     printf("tri[%d].c %f %f %f\n", i, triangles[i].c[0], triangles[i].c[1],
   */
  /*            triangles[i].c[2]); */
  /*     printf("tri[%d].na %f %f %f\n", i, triangles[i].na[0],
   * triangles[i].na[1], */
  /*            triangles[i].na[2]); */
  /*     printf("tri[%d].nb %f %f %f\n", i, triangles[i].nb[0],
   * triangles[i].nb[1], */
  /*            triangles[i].nb[2]); */
  /*     printf("tri[%d].nc %f %f %f\n", i, triangles[i].nc[0],
   * triangles[i].nc[1], */
  /*            triangles[i].nc[2]); */
  /*   } */
  /* } */
  printf("bounds_min = %f %f %f, bounds_max = %f %f %f\n", info.bounds_min.l[0],
         info.bounds_min.l[1], info.bounds_min.l[2], info.bounds_max.l[0],
         info.bounds_max.l[1], info.bounds_max.l[2]);
  /* #endif */

  int first_triangle_index = mb_add_triangles(mb, triangles, info.f);
  mb_add_mesh(mb, info, first_triangle_index);

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

  // TODO: if we had BVH we would also offset it here
  mb->meshesInfo[model_id].boundsMin[0] += pos.l[0];
  mb->meshesInfo[model_id].boundsMin[1] += pos.l[1];
  mb->meshesInfo[model_id].boundsMin[2] += pos.l[2];

  mb->meshesInfo[model_id].boundsMax[0] += pos.l[0];
  mb->meshesInfo[model_id].boundsMax[1] += pos.l[1];
  mb->meshesInfo[model_id].boundsMax[2] += pos.l[2];

  /* int overall_num_of_triangles = */
  /*     mb->meshesInfo[mb->num_of_meshes - 1].firstTriangleIndex + */
  /*     mb->meshesInfo[mb->num_of_meshes - 1].numTriangles; */

  /* glDeleteBuffers(1, &mb->tbo_triangles); */
  /* glDeleteTextures(1, &mb->tbo_tex_triangles); */

  /* create_gl_buffer(&mb->tbo_triangles, &mb->tbo_tex_triangles, */
  /*                  overall_num_of_triangles * sizeof(Triangle), mb->triangles, */
  /*                  GL_RGB32F, GL_TEXTURE1); */
}

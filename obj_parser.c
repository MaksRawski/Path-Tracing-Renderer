#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj_parser.h"

ObjStats get_obj_stats(const char *filename) {
  ObjStats s = {0, 0, 0};
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Failed to open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  for (char line[255]; fgets(line, sizeof(line), fp);) {
    if (line[0] == 'v')
      ++s.v;
    else if (line[0] == 'f')
      ++s.f;
    else if (strncmp(line, "vn", 2))
      ++s.vn;
  }
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
  /*        vertices[*num_of_vertices].l[1], vertices[*num_of_vertices].l[2]); */
  (*num_of_vertices)++;
}

void add_triangle(Triangle triangles[], int *num_of_triangles,
                  vec3 vertices[], vec3 vns[], char line[]) {
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

  printf("Failed to match on line: %s\n", line);
  exit(EXIT_FAILURE);
}

void parse_obj(const char *filename, Triangle *triangles[],
               int *num_of_triangles) {
  ObjStats stats = get_obj_stats(filename);
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Failed to open file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  *triangles = malloc(stats.f * sizeof(Triangle));
  vec3 *vertices = malloc(stats.v * sizeof(vec3));
  int num_of_vertices = 0;
  vec3 *vns = malloc(stats.vn * sizeof(vec3));
  int num_of_vns = 0;

  for (char line[255]; fgets(line, sizeof(line), fp);) {
    char type[5];
    sscanf(line, "%s", type);

    if (type[0] == '#')
      continue;
    else if (type[0] == 'v' && type[1] == 0) {
      add_vertex(vertices, &num_of_vertices, line + 2);
      /* printf("Added vertex = %f %f %f\n\n", vertices[num_of_vertices - 1].l[0], */
      /*        vertices[num_of_vertices - 1].l[1], */
      /*        vertices[num_of_vertices - 1].l[2]); */
    } else if (type[0] == 'v' && type[1] == 'n') {
      add_vertex(vns, &num_of_vns, line + 2);
      /* printf("Added vertex normal = %f %f %f\n\n", vns[num_of_vns - 1].l[0], */
      /*        vns[num_of_vns - 1].l[1], */
      /*        vns[num_of_vns - 1].l[2]); */
    }
    else if (line[0] == 'f') {
      add_triangle(*triangles, num_of_triangles, vertices, vns, line + 2);
    }
  }
  free(vertices);
  free(vns);

  fclose(fp);
}

void load_obj_model(const char *filename, GLuint shader_program,
                    ModelBuffer *mb) {
  Triangle *triangles = NULL;
  int num_of_triangles = 0;
  parse_obj(filename, &triangles, &num_of_triangles);
  if (triangles != NULL){
    for (int i = 0; i < num_of_triangles; i++){
      printf("tri[%d].a %f %f %f\n", i, triangles[i].a[0], triangles[i].a[1], triangles[i].a[2]);
      printf("tri[%d].b %f %f %f\n", i, triangles[i].b[0], triangles[i].b[1], triangles[i].b[2]);
      printf("tri[%d].c %f %f %f\n", i, triangles[i].c[0], triangles[i].c[1], triangles[i].c[2]);
      printf("tri[%d].na %f %f %f\n", i, triangles[i].na[0], triangles[i].na[1], triangles[i].na[2]);
      printf("tri[%d].nb %f %f %f\n", i, triangles[i].nb[0], triangles[i].nb[1], triangles[i].nb[2]);
      printf("tri[%d].nc %f %f %f\n", i, triangles[i].nc[0], triangles[i].nc[1], triangles[i].nc[2]);
    }
  }
  mb->numOfTriangles = num_of_triangles;
  mb->triangles = triangles;

  // to store vertices
  glGenBuffers(1, &mb->tbo);
  glBindBuffer(GL_ARRAY_BUFFER, mb->tbo);
  glBufferData(GL_ARRAY_BUFFER, num_of_triangles * sizeof(Triangle), triangles, GL_STATIC_DRAW);

  // texture buffer object, will bind the vbo to it
  // so that the shader can sample the texture and actually
  // get the data from the vbo
  glGenTextures(1, &mb->tboTex);
  glBindTexture(GL_TEXTURE_BUFFER, mb->tboTex);
  // we choose GL_RGB32F to have each pixel of the texture store 3 floats AKA vec3
  // and then use tbo as data for that texture,
  glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, mb->tbo);

  // bind the texture as a uniform in the shader
  glUseProgram(shader_program);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_BUFFER, mb->tboTex);
  // set to texture at index 1 as index 0 we will be storing back buffer
  glUniform1i(glGetUniformLocation(shader_program, "trianglesBuffer"), 1);
  glUniform1i(glGetUniformLocation(shader_program, "numOfTriangles"), num_of_triangles);
  /* printf("mb.numOfTriangles = %d\n", mb->numOfTriangles); */

  // unbind the texture buffer
  glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

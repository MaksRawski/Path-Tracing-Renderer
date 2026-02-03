#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <stdint.h>
#define NO_TEXTURE (unsigned int)-1

// *_texture are set to MAX_INT if no texture is used
typedef struct {
  float base_color_factor[4];
  float emissive_factor[3];
  uint32_t base_color_texture;
  uint32_t metallic_texture;
  float metallic_factor;
  uint32_t roughness_texture;
  float roughness_factor;
  uint32_t emissive_texture;
  // HACK: since padding must be added anyway, this allows determining whether
  // a given material has been set, or just allocated.
  int32_t _set;
  uint64_t _;
} Material;

inline static Material Material_default(void) {
  Material mat = {
      .base_color_factor = {1, 1, 1, 1},
      // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#additional-textures
      .emissive_factor = {0, 0, 0}};
  mat.base_color_texture = NO_TEXTURE;
  mat.metallic_texture = NO_TEXTURE;
  mat.roughness_texture = NO_TEXTURE;
  mat.emissive_texture = NO_TEXTURE;
  mat.metallic_factor = 1.0;
  mat.roughness_factor = 1.0;
  mat._set = 1;
  return mat;
}

#endif // MATERIAL_H_

#ifndef MATERIAL_H_
#define MATERIAL_H_

#define NO_TEXTURE (unsigned int)-1

// *_texture are set to MAX_INT if no texture is used
typedef struct {
  float base_color_factor[4];
  float emissive_factor[3];
  unsigned int base_color_texture;
  unsigned int metallic_texture;
  float metallic_factor;
  unsigned int roughness_texture;
  float roughness_factor;
  unsigned int emissive_texture;
  // HACK: since padding must be added anyway, this allows to determine whether
  // a given material has been set, or just allocated.
  int _set;
  long _;
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

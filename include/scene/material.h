#ifndef MATERIAL_H_
#define MATERIAL_H_


// NOTE: this struct WILL change
typedef struct {
  float emission_color[3];
  float emission_strength;
  float albedo[3];
  float specular_component;
} Material;


#endif // MATERIAL_H_

# Path Tracing Renderer
Simple OpenGL Path Tracing Renderer with a backend in C.

## Current features:
Right now my goal is to make it a glTF 2.0 viewer of sorts
and so far:
- Meshes can be loaded with all their transformations
- Materials:
    - PBR base color is used as albedo
    - Emission factor (color) and strength are supported
- If there is a perspective camera available it will be used

## Naming convention
- snake_case for functions_names, variable_names and struct_members
- PascalCase for StructNames

### Resources used:
- [Sebastian Lague's video on ray tracing](https://www.youtube.com/watch?v=Qz0KTGYJtUk)
- [tinyraytracer](https://github.com/ssloy/tinyraytracer/wiki/Part-1:-understandable-raytracing)
- [casual shadertoy path tracing](https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/)
- [how to build a BVH series](https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/)

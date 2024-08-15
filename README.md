# Path Tracing Renderer
Simple OpenGL Path Tracing Renderer with a backend in C.

## Current features:
- spheres and triangles* rendering
- .obj loading with automatic bounding box around the mesh generation

\* Right now, for some reason, even though the
 triangles look right, they don't leave any shadow.

## Planned features:
- BVH
- port to Vulkan and use Vulkan Ray Tracing acceleration structures

### [Writeup in Polish on the design of the shader](https://gitlab.com/MaksRawski/lak-sprawozdanie-pdf/raw/main/raport.pdf)
_Projekt został wykonany na zaliczenie przedmiotu
Laboratorium Algebry Komputerowej, stąd
taki tytuł oraz wstęp._


### Resources used:
- [Sebastian Lague's video on ray tracing](https://www.youtube.com/watch?v=Qz0KTGYJtUk)
- [tinyraytracer](https://github.com/ssloy/tinyraytracer/wiki/Part-1:-understandable-raytracing)
- [casual shadertoy path tracing](https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/
)
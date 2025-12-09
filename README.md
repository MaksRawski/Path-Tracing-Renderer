# Path Tracing Renderer
Interactive Path Tracing Renderer with an OpenGL backend, written in C.

## Current features:
- GUI to control internal rendering parameters
- Partial support of glTF 2.0:
    - Meshes can be loaded with all their transformations
    - Materials:
        - PBR base color is used as albedo
        - Emission factor (color) and strength are supported
    - If there are perspective cameras available, first one of them will be used
- Simple BVH is applied to the loaded scene

## Naming convention
- snake_case for functions_names, variable_names and struct_members
- PascalCase for StructNames
- StructName_new for "constructors" and StructName_delete for "destructors"
- StructName_function_name for functions that operate on a given struct directly

## Building
This project uses submodules for libraries so if you don't specify `--recurse-submodules`
during `git clone`, you will have to run `git submodule update --init --recursive` in the
root of the cloned project.

### Build dependencies (required only for Make):
```
make -Clib
```

### Build the project:

CMake:
```
cmake -Bbuild && cmake --build build
```

Make:
```
make 
```

### Build & Run tests:

CMake:
```
cmake -Bbuild -DBUILD_TESTS=ON && cmake --build build && ./build/tests
```

Make:
```
make tests
```

## Tests
My DIY "testing framework" is made up of `include/asserts.h`, `src/asserts.c` 
and a simple `TEST_RUN` macro from `tests/tests_macros.h` 

`asserts` are included in the main part of the project as they turned out
to be convenient as runtime checks on debug builds, and so they are completely 
removed when compiling with `-DNDEBUG`.

### Resources used:
- [Sebastian Lague's video on ray tracing](https://www.youtube.com/watch?v=Qz0KTGYJtUk)
- [tinyraytracer](https://github.com/ssloy/tinyraytracer/wiki/Part-1:-understandable-raytracing)
- [casual shadertoy path tracing](https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/)
- [how to build a BVH series](https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/)

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

## Building
This project uses submodules for libraries so if you don't specify `--recurse-submodules`
during `git clone`, you will have to run `git submodule update --init --recursive` in the
root of the cloned project.

After making sure all the libraries are downloaded, compile them with:

```
make -Clib
```

This has to be done only once. Afterwards regular compilation can be performed with just:

```
make
```

or 

```
make MODE=release
```

## Tests
Tests are provided in the `tests/` directory and can be run with:

```
make tests
```

My DIY "testing framework" is made up of 
- `include/asserts.h`, `src/asserts.c`
- and a simple `TEST_RUN` macro from `tests/tests_macros.h` 

`asserts` are included in the main part of the project as they turned out
to be convenient as runtime checks on debug builds, and so they are completely 
removed when compiling with `-DNDEBUG`.

### Resources used:
- [Sebastian Lague's video on ray tracing](https://www.youtube.com/watch?v=Qz0KTGYJtUk)
- [tinyraytracer](https://github.com/ssloy/tinyraytracer/wiki/Part-1:-understandable-raytracing)
- [casual shadertoy path tracing](https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/)
- [how to build a BVH series](https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/)

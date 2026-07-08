# Path Tracing Renderer
Interactive Path Tracing Renderer with an OpenGL backend, written in C.

![Screenshot of the application rendering a model of Stanford Dragon in Cornell Box.](images/dragon.png)


## Current features:
- GUI to control internal rendering parameters
- Partial support of glTF 2.0:
    - Meshes can be loaded with all their transformations
    - Materials:
        - PBR base color is used as albedo
        - Emission factor (color) and strength are supported
    - If there are perspective cameras available, first one of them will be used
- Two BVH types are available: Midpoint split, (simple) SAH


## Building
This project uses submodules for its dependencies, so make sure to download them while cloning.
```
git clone https://github.com/MaksRawski/PathTracingRenderer --recurse-submodules
```

If you have cloned without submodules, run this inside the project directory
```
git submodule update --init --recursive
```

### Make
Build dependencies 
```
make -Clib
```

Build the project
```
make MODE=release
```

Run it
```
./build/release/main
```

Run tests:
```
make tests
```

### CMake
Configure 
```
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
```

Build the project
```
cmake --build build
```

Run it
```
./build/PathTracingRenderer
```

Run tests:
```
cmake -Bbuild -DBUILD_TESTS=ON && cmake --build build && ./build/tests
```


## Tests
My DIY "testing framework" is made up of `include/asserts.h`, `src/asserts.c` 
and a simple `TEST_RUN` macro from `tests/tests_macros.h` 

`asserts` are included in the main part of the project as they turned out
to be convenient as runtime checks on debug builds, and so they are completely 
removed when compiling with `-DNDEBUG`.


## Coding conventions
### Naming
- `snake_case` for function\_names, variable\_names and struct\_members
- `PascalCase` for StructNames

- `StructName_new` for "constructors" and `StructName_delete` for "destructors"
- `StructName_function_name` for functions that operate on a given struct directly
- `function__template` for macros that expand into function definitons

### Other
- "private" struct functions/helper functions are `static` defined within the same translation unit.
- stack allocation is heavily preferred
    - functions that "return a string" take a `char *buf` as argument or return either `SmallString` (`char[1024]`) or `TinyString` (`char[16]`)
    - 16MB Arena is allocated at the beginning of main and is passed around for all temporary allocations needs
    - only big allocations (theoretically gigabytes) are `malloc`'ed
- `stdint.h` types are preferred over `int`, `long` etc. 

# AGENTS.md

## Cursor Cloud specific instructions

This is a C++ CMake project (port of The Simpsons: Hit & Run). There are no web services, package managers, or interpreted-language dependencies. Build instructions are in `README.md`.

### Building

- **Must use GCC** (`-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++`). The VM's default compiler is Clang 18, which fails to link because it can't find `libstdc++` through its default search paths.
- Release build: `cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ && cmake --build build-linux -j$(nproc)`
- Debug build (enables `-Werror`): `cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ && cmake --build build-debug -j$(nproc)`
- Output binary: `build-linux/code/SRR2`

### Lint / Quality check

There is no separate linter. The Debug build enables `-Werror` (warnings as errors) which serves as the quality gate. Build in Debug mode to check for warnings.

### Tests

There is no automated test suite. `SRR2_BUILD_TESTS=ON` (default) builds two sample programs:
- `build-linux/libs/radsound/simplesound` — audio library test
- `build-linux/libs/radmovie/simplemovie` — movie playback test (initializes OpenGL via Mesa llvmpipe in headless environments)

Both run without issues; they stay alive until terminated (use `timeout 5` to run them).

### Running the game

The `SRR2` binary requires PC retail game assets in the working directory. Without those assets the binary will segfault immediately after SDL/audio initialization. ALSA warnings about missing sound card are expected in the VM and do not affect the build.

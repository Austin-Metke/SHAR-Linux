# D'oh!

This is a Linux fork of [ZenoArrows' Nintendo Switch & PS Vita port](https://github.com/ZenoArrows/The-Simpsons-Hit-and-Run) of The Simpsons Hit & Run, based on the leaked source code. The upstream port targets the Nintendo Switch and PS Vita; this fork adds native Linux desktop support.

The full game should be playable, including local multiplayer in the bonus game. The port is however still incomplete, so some glitches can be observed and some visual effects are missing compared to the PC version.

Please report any Linux-specific bugs or feature requests in the issues tab on this repository. For issues related to the base port, please refer to the [upstream project](https://github.com/ZenoArrows/The-Simpsons-Hit-and-Run).

# Installation

This port uses the PC assets, so you will need to have the PC version of the game installed. Do not use the assets from the source code leak as those are not the final version, instead use the assets from the official release. Also make sure you're using the original `.rmv` movie files in the `movies` folder rather than the converted `.bk2` files that older releases of the port required.

For Linux, build from source using the instructions below, or use a pre-compiled binary from the releases page, then place the resulting binary alongside your PC game assets.

## Runtime Dependencies

If you are running a pre-compiled binary, the following shared libraries must be installed on your system:

| Library | Debian/Ubuntu | Arch/Manjaro | Fedora |
|---------|--------------|--------------|--------|
| SDL2 (or SDL3) | `libsdl2-2.0-0` | `sdl2` | `SDL2` |
| OpenAL | `libopenal1` | `openal` | `openal-soft` |
| libpng | `libpng16-16` | `libpng` | `libpng` |
| FFmpeg (≥ 5.0) | `ffmpeg` | `ffmpeg` | `ffmpeg-free` |
| OpenGL | `libgl1` | `mesa` | `mesa-libGL` |

Quick install (Debian/Ubuntu):
```
sudo apt install libsdl2-2.0-0 libopenal1 libpng16-16 ffmpeg libgl1
```

Quick install (Arch/Manjaro):
```
sudo pacman -S sdl2 openal libpng ffmpeg mesa
```

For Nintendo Switch and PS Vita installation, see the [upstream project releases and instructions](https://github.com/ZenoArrows/The-Simpsons-Hit-and-Run).

# Building on Linux

Dependencies (package names for Debian/Ubuntu shown):

```
sudo apt install build-essential cmake pkg-config libsdl2-dev libopenal-dev libpng-dev \
    libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev \
    libgl1-mesa-dev
```

SDL3 is used when available; otherwise SDL2 is required. FFmpeg is used for video playback unless the proprietary Bink SDK is provided.

Build steps:

```
mkdir -p build-linux
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux -j$(nproc)
```

The native Linux binary is placed at `build-linux/code/SRR2`. Keep the executable next to your PC game assets (the working directory is used to find the data files).

# Building for AARCH64 (ARM64)

An AARCH64 build can be produced using Docker to provide a consistent ARM64 environment.

Dependencies are the same as the Linux build. The included `Dockerfile.aarch64` sets up an ARM64 Ubuntu 22.04 container with all required packages.

Build steps:

```
docker build -t shar-aarch64 -f Dockerfile.aarch64 .
docker run --rm -v "$(pwd)":/src shar-aarch64 bash -c \
    "mkdir -p build-aarch64 && cmake -S . -B build-aarch64 -DCMAKE_BUILD_TYPE=Release && cmake --build build-aarch64 -j$(nproc)"
```

The ARM64 binary is placed at `build-aarch64/code/SRR2`.

If you are already on an AARCH64 system you can build natively using the same steps as the standard Linux build, substituting `build-aarch64` for `build-linux`.

# Multi-Language support

The PAL version supports multiple languages and will use the language that matches the system language of your console. If your console is set to a language that is not supported a menu will be shown giving you the option to choose between the supported languages.

No official release has the dialog RCF files for all 4 supported languages, so you will need to make sure you use the game assets from a release that's localized in the language you'd like to play.

If you'd just like to play in English and have no need for multi-language support, then use the NTSC version to play.

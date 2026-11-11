# 🍩 SHAR-ARM for PortMaster / Linux Handhelds

This is a Linux fork of [ZenoArrows' Nintendo Switch & PS Vita port](https://github.com/ZenoArrows/The-Simpsons-Hit-and-Run) and [Austin-Metke ARM port](https://github.com/Austin-Metke/SHAR-Linux) of *The Simpsons Hit & Run*, based on the leaked source code. The upstream port targets the Nintendo Switch and PS Vita; this fork focuses on **native Linux desktop support and optimization for ARM64 (AArch64) handheld devices**.

The full game should be playable, including local multiplayer in the bonus game. The port is however still incomplete, so some glitches can be observed and some visual effects are missing compared to the PC version.

> ⚠️ **Bug Reports:** Please report any Linux/Handheld-specific bugs or feature requests in the issues tab of this repository. For issues related to the base port, please refer to the [upstream project](https://github.com/ZenoArrows/The-Simpsons-Hit-and-Run).

---

## 💾 Installation & Game Assets

This port uses the original PC assets, so you will need to have the PC version of the game installed.

* 🛑 **Do not use assets from the source code leak** as those are not the final retail version. Instead, use the assets from the official PC release.
* 🎬 Make sure you're using the original `.rmv` movie files in the `movies` folder rather than the converted `.bk2` files that older releases of the port required.
* 📂 Place your compiled binary (`shar`) directly inside the folder alongside your PC game assets.

*The PortMaster Ready zip folder with out game data can be found [heare](https://cloud.40gb.club/s/ASmRYNRAZzdE3QS) 
---

## ⚙️ Runtime Dependencies

If you are running a pre-compiled binary, the following shared libraries must be installed on your system:

| Library | Debian/Ubuntu | Arch/Manjaro | Fedora |
| --- | --- | --- | --- |
| **SDL2 (or SDL3)** | `libsdl2-2.0-0` | `sdl2` | `SDL2` |
| **OpenAL** | `libopenal1` | `openal` | `openal-soft` |
| **libpng** | `libpng16-16` | `libpng` | `libpng` |
| **FFmpeg (≥ 5.0)** | `ffmpeg` | `ffmpeg` | `ffmpeg-free` |
| **OpenGL** | `libgl1` | `mesa` | `mesa-libGL` |

### 🚀 Quick Install Commands:

* **Debian/Ubuntu:**
```bash
sudo apt install libsdl2-2.0-0 libopenal1 libpng16-16 ffmpeg libgl1

```


* **Arch/Manjaro:**
```bash
sudo pacman -S sdl2 openal libpng ffmpeg mesa

```



---

## 🛠️ Building on Linux Desktop (x86_64)

### 1. Install Build Dependencies (Debian/Ubuntu):

```bash
sudo apt install build-essential cmake pkg-config libsdl2-dev libopenal-dev libpng-dev \
     libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev \
     libgl1-mesa-dev

```

*Note: SDL3 is used when available; otherwise SDL2 is required. FFmpeg is used for video playback unless the proprietary Bink SDK is provided.*

### 2. Compilation Steps:

```bash
mkdir -p build-linux
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release
cmake --build build-linux -j$(nproc)

```

The native Linux binary is placed at `build-linux/code/shar`.

---

## 🐧 Building for AARCH64 / ARM64 (Powkiddy x55, Rocknix, ArkOS)

An AARCH64 build can be produced using a cross-compilation environment inside a Docker container (`shar-cross`).

### 🛑 Important: Run from the Project Root

Ensure you are in the root directory of the project (`SHAR-Linux`), **not** inside the build folder, before running Docker commands.

---
### Step 0: Get git clone of repository 

```bash
git clone https://github.com/YoshiKill/SHAR-ARM-for-PortMaster-Linux-Handhelds.git
cd SHAR-ARM-for-PortMaster-Linux-Handhelds

```

### Step 1: Project Configuration (CMake)

Run the container to generate the build configuration files for the `aarch64` architecture:

```bash
sudo docker run --rm -v "$(pwd)":/src shar-cross bash -c "cmake -B build-aarch64 -DARCH=aarch64 -DCMAKE_BUILD_TYPE=Release"

```

### Step 2: Compiling the Binary

Start the compilation process using all available CPU cores:

```bash
sudo docker run --rm -v "$(pwd)":/src shar-cross bash -c "cmake --build build-aarch64 -j$(nproc)"

```

The compiled ARM64 binary will be placed at: `build-aarch64/code/shar`.

---

## 🎮 Handheld Controller Configuration Notes

By default, the engine initializes with a standard PC keyboard layout. To get your handheld's built-in gamepad working properly:

1. 🕹️ Launch the game and navigate to the **Controls** settings menu.
2. ⌨️ Manually rebind the game actions to your device's physical buttons.
3. 🛠️ **Analog Stick Issues:** If you encounter system-wide analog stick issues across your OS (affecting both this port and emulators like PSP), check the raw hardware events using the `evtest` utility via SSH. In case of a hardware failure on the left analog stick or scrambled Linux input axes, it is highly recommended to upgrade the hardware to Hall Effect or TMR magnetic analog sticks (the Nintendo Switch Joy-Con form factor is fully drop-in compatible with the Powkiddy x55 housing and ribbon cable connector).

---

## 🌐 Multi-Language Support

The PAL version supports multiple languages and will use the language that matches the system language of your console. If your console is set to a language that is not supported, a menu will be shown giving you the option to choose between the supported languages.

* 🎙️ No official release has the dialog RCF files for all 4 supported languages, so you will need to make sure you use the game assets from a release that's localized in the language you'd like to play.
* 🇺🇸 If you just want to play in English and have no need for multi-language support, use the NTSC version assets.

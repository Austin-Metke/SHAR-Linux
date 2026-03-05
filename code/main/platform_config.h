//=============================================================================
// Platform configuration for SHAR cross-platform port.
//
// This header cleanly separates platform-specific behavior:
//   - Asset path resolution
//   - OS-specific initialization/shutdown
//   - Platform identification
//
// Platforms: Linux (native x86_64), Nintendo Switch, PS Vita
//=============================================================================

#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

//=============================================================================
// Platform identification
//=============================================================================
#if defined(RAD_LINUX)
    #define SHAR_PLATFORM_NAME "Linux"
#elif defined(__SWITCH__)
    #define SHAR_PLATFORM_NAME "NintendoSwitch"
#elif defined(RAD_VITA)
    #define SHAR_PLATFORM_NAME "PSVita"
#else
    #define SHAR_PLATFORM_NAME "Unknown"
#endif

//=============================================================================
// Asset path resolution
//=============================================================================

// On Linux, the game data directory defaults to the current working directory.
// Can be overridden by setting the SHAR_DATA_DIR environment variable, or by
// passing -datadir <path> on the command line.
//
// On Switch, assets are in romfs.
// On Vita, assets are at ux0:data/simpsons.

#if defined(RAD_LINUX)
    #define SHAR_DEFAULT_DATA_DIR "."
#elif defined(__SWITCH__)
    // romfs is mounted at root by romfsInit()
    #define SHAR_DEFAULT_DATA_DIR "."
#elif defined(RAD_VITA)
    #define SHAR_DEFAULT_DATA_DIR "ux0:data/simpsons"
#else
    #define SHAR_DEFAULT_DATA_DIR "."
#endif

//=============================================================================
// OS-specific init/shutdown stubs
//=============================================================================

#if defined(RAD_LINUX)

    static inline void PlatformPreInit(void) {}
    static inline void PlatformPostShutdown(void) {}

#elif defined(__SWITCH__)

    #include <switch.h>
    static inline void PlatformPreInit(void)
    {
    #ifdef RAD_DEBUG
        socketInitializeDefault();
        nxlinkStdio();
    #endif
        romfsInit();
    }
    static inline void PlatformPostShutdown(void) {}

#elif defined(RAD_VITA)

    #include <unistd.h>
    static inline void PlatformPreInit(void)
    {
        chdir(SHAR_DEFAULT_DATA_DIR);
    }
    static inline void PlatformPostShutdown(void) {}

#else

    static inline void PlatformPreInit(void) {}
    static inline void PlatformPostShutdown(void) {}

#endif

#endif // PLATFORM_CONFIG_H

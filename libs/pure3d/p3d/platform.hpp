//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#ifndef _BASE_PLATFORM_HPP
#define _BASE_PLATFORM_HPP

// Note: RAD_WIN32 is the generic "desktop" define (used for Linux, Windows, Switch).
// The win32/platform.hpp uses SDL and works on all desktop platforms.
// The linux/platform.hpp is an old X11 version that's unused.
#if defined(RAD_PS2)
    #include <p3d/platform/ps2/platform.hpp>
#elif defined(RAD_XBOX)
    #include <p3d/platform/xbox/platform.hpp>
#elif defined(RAD_GAMECUBE)
    #include <p3d/platform/GameCube/platform.hpp>
#elif defined(RAD_WIN32)
    #include <p3d/platform/win32/platform.hpp>
#endif


#endif


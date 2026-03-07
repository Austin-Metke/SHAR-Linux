#ifndef GAMEPAD_SDL_H
#define GAMEPAD_SDL_H

#include <input/RealController.h>

// SDL gamepad codes corresponding to the input point indices in sdlcontroller.cpp's g_SDLPoints array
enum eSDLGamepadCode
{
    SDLGP_DPAD_UP = 0,
    SDLGP_DPAD_DOWN,
    SDLGP_DPAD_LEFT,
    SDLGP_DPAD_RIGHT,
    SDLGP_START,
    SDLGP_BACK,
    SDLGP_LEFTSTICK,
    SDLGP_RIGHTSTICK,
    SDLGP_A,
    SDLGP_B,
    SDLGP_X,
    SDLGP_Y,
    SDLGP_LEFTSHOULDER,
    SDLGP_RIGHTSHOULDER,
    SDLGP_LEFTTRIGGER,
    SDLGP_RIGHTTRIGGER,
    SDLGP_LEFTSTICK_X,
    SDLGP_LEFTSTICK_Y,
    SDLGP_RIGHTSTICK_X,
    SDLGP_RIGHTSTICK_Y,
    NUM_SDL_GAMEPAD_CODES
};

class GamepadSDL : public RealController
{
public:
    GamepadSDL();
    virtual ~GamepadSDL();

    virtual bool IsInputAxis( int dxKey ) const;

    virtual bool IsValidInput( int dxKey ) const;

    virtual bool SetMap( int dxKey, eDirectionType dir, int virtualButton );
    virtual int  GetMap( int dxKey, eDirectionType dir, eMapType map ) const;
    virtual void ClearMap( int dxKey, eDirectionType dir, int virtualButton );
    virtual void ClearMappedButtons();

private:
    virtual void MapInputToDICode();

private:
    int m_ButtonMap[ NUM_MAPTYPES ][ NUM_SDL_GAMEPAD_CODES ][ NUM_DIRTYPES ];
};

#endif

#ifndef KEYBOARD_SDL_H
#define KEYBOARD_SDL_H

#include <input/RealController.h>
#include <SDL.h>

const unsigned NUM_SDL_KEYBOARD_KEYS = 512;

class KeyboardSDL : public RealController
{
public:
    KeyboardSDL();
    virtual ~KeyboardSDL();

    virtual bool IsValidInput( int dxKey ) const;
    virtual bool IsBannedInput( int dxKey ) const;

    virtual bool SetMap( int dxKey, eDirectionType dir, int virtualButton );
    virtual int  GetMap( int dxKey, eDirectionType dir, eMapType map ) const;
    virtual void ClearMap( int dxKey, eDirectionType dir, int virtualButton );
    virtual void ClearMappedButtons();

    static const char* GetSDLKeyName( int scancode );

private:
    virtual void MapInputToDICode();

private:
    int m_ButtonMap[ NUM_MAPTYPES ][ NUM_SDL_KEYBOARD_KEYS ];
};

#endif

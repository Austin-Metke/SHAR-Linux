#include <input/KeyboardSDL.h>
#include <string.h>

KeyboardSDL::KeyboardSDL()
: RealController( KEYBOARD )
{
    ClearMappedButtons();
}

KeyboardSDL::~KeyboardSDL()
{
}

bool KeyboardSDL::IsValidInput( int dxKey ) const
{
    return dxKey >= SDL_SCANCODE_A && dxKey < (int)NUM_SDL_KEYBOARD_KEYS;
}

bool KeyboardSDL::IsBannedInput( int dxKey ) const
{
    switch( dxKey )
    {
        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_RALT:
        case SDL_SCANCODE_LGUI:
        case SDL_SCANCODE_RGUI:
            return true;
        default:
            return false;
    }
}

bool KeyboardSDL::SetMap( int dxKey, eDirectionType dir, int virtualButton )
{
    rAssert( dxKey >= 0 && dxKey < (int)NUM_SDL_KEYBOARD_KEYS );
    rAssert( virtualButton >= 0 && virtualButton < Input::MaxPhysicalButtons );
    rAssert( dir == DIR_UP );

    eMapType maptype = VirtualInputs::GetType( virtualButton );

    if( dxKey >= 0 && dxKey < (int)NUM_SDL_KEYBOARD_KEYS )
    {
        m_ButtonMap[ maptype ][ dxKey ] = virtualButton;
        return true;
    }
    else
    {
        return false;
    }
}

void KeyboardSDL::ClearMap( int dxKey, eDirectionType dir, int virtualButton )
{
    rAssert( dxKey >= 0 && dxKey < (int)NUM_SDL_KEYBOARD_KEYS );
    rAssert( virtualButton >= 0 && virtualButton < Input::MaxPhysicalButtons );
    rAssert( dir == DIR_UP );

    eMapType maptype = VirtualInputs::GetType( virtualButton );

    if( dxKey >= 0 && dxKey < (int)NUM_SDL_KEYBOARD_KEYS )
    {
        m_ButtonMap[ maptype ][ dxKey ] = Input::INVALID_CONTROLLERID;
    }
}

int KeyboardSDL::GetMap( int dxKey, eDirectionType dir, eMapType map ) const
{
    rAssert( dxKey >= 0 && dxKey < (int)NUM_SDL_KEYBOARD_KEYS );

    if( dir == DIR_UP && dxKey >= 0 && dxKey < (int)NUM_SDL_KEYBOARD_KEYS )
    {
        return m_ButtonMap[ map ][ dxKey ];
    }
    else
    {
        return Input::INVALID_CONTROLLERID;
    }
}

void KeyboardSDL::ClearMappedButtons()
{
    memset( &m_ButtonMap, Input::INVALID_CONTROLLERID, sizeof( m_ButtonMap ) );
}

const char* KeyboardSDL::GetSDLKeyName( int scancode )
{
    if( scancode >= 0 && scancode < (int)NUM_SDL_KEYBOARD_KEYS )
    {
        return SDL_GetScancodeName( (SDL_Scancode)scancode );
    }
    return "Unknown";
}

void KeyboardSDL::MapInputToDICode()
{
    // On SDL, scancodes are the key codes directly.
    // No reverse mapping needed since we don't use a radcontroller for keyboard.
}

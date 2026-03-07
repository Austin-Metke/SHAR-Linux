#include <input/GamepadSDL.h>
#include <string.h>

GamepadSDL::GamepadSDL()
: RealController( GAMEPAD )
{
    ClearMappedButtons();
}

GamepadSDL::~GamepadSDL()
{
}

bool GamepadSDL::IsInputAxis( int dxKey ) const
{
    switch( dxKey )
    {
        case SDLGP_LEFTSTICK_X:
        case SDLGP_LEFTSTICK_Y:
        case SDLGP_RIGHTSTICK_X:
        case SDLGP_RIGHTSTICK_Y:
            return true;
        default:
            return false;
    }
}

bool GamepadSDL::IsValidInput( int dxKey ) const
{
    return dxKey >= 0 && dxKey < NUM_SDL_GAMEPAD_CODES;
}

bool GamepadSDL::SetMap( int dxKey, eDirectionType dir, int virtualButton )
{
    rAssert( virtualButton >= 0 && virtualButton < Input::MaxPhysicalButtons );

    if( dxKey >= 0 && dxKey < NUM_SDL_GAMEPAD_CODES )
    {
        eMapType maptype = VirtualInputs::GetType( virtualButton );
        m_ButtonMap[ maptype ][ dxKey ][ dir ] = virtualButton;
        return true;
    }
    return false;
}

void GamepadSDL::ClearMap( int dxKey, eDirectionType dir, int virtualButton )
{
    if( dxKey >= 0 && dxKey < NUM_SDL_GAMEPAD_CODES )
    {
        eMapType maptype = VirtualInputs::GetType( virtualButton );
        m_ButtonMap[ maptype ][ dxKey ][ dir ] = Input::INVALID_CONTROLLERID;
    }
}

int GamepadSDL::GetMap( int dxKey, eDirectionType dir, eMapType map ) const
{
    if( dxKey >= 0 && dxKey < NUM_SDL_GAMEPAD_CODES )
    {
        return m_ButtonMap[ map ][ dxKey ][ dir ];
    }
    return Input::INVALID_CONTROLLERID;
}

void GamepadSDL::ClearMappedButtons()
{
    memset( &m_ButtonMap, Input::INVALID_CONTROLLERID, sizeof( m_ButtonMap ) );
}

void GamepadSDL::MapInputToDICode()
{
    if( m_InputToDICode != NULL )
    {
        delete [] m_InputToDICode;
        m_InputToDICode = NULL;
    }

    if( m_radController != NULL )
    {
        m_numInputPoints = m_radController->GetNumberOfInputPoints();
        m_InputToDICode = new int[ m_numInputPoints ];

        // Identity mapping: input point index IS the SDL gamepad code
        for( int i = 0; i < m_numInputPoints && i < NUM_SDL_GAMEPAD_CODES; i++ )
        {
            m_InputToDICode[ i ] = i;
        }
        for( int i = NUM_SDL_GAMEPAD_CODES; i < m_numInputPoints; i++ )
        {
            m_InputToDICode[ i ] = Input::INVALID_CONTROLLERID;
        }
    }
}

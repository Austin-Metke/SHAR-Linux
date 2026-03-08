#include <radcontroller.hpp>
#include <raddebug.hpp>
#include <radmath/radmath.hpp>

#include <string.h>
#include <SDL.h>

#include <input/usercontrollerLinux.h>
#include <input/mappable.h>
#include <input/mapper.h>
#include <input/button.h>
#include <input/KeyboardSDL.h>
#include <input/GamepadSDL.h>
#include <input/inputmanager.h>

#include <cheats/cheatinputsystem.h>
#include <cheats/cheatinputhandler.h>

#include <presentation/tutorialmanager.h>

#include <console/fbstricmp.h>
#include <data/config/gameconfigmanager.h>

#ifndef WORLD_BUILDER
#include <main/commandlineoptions.h>
#endif

#include <gameflow/gameflow.h>
#include <contexts/context.h>

#ifdef WORLD_BUILDER
#ifdef CONTROLLER_DEBUG
#undef CONTROLLER_DEBUG
#endif
enum { CLO_NO_HAPTIC, CLO_RANDOM_BUTTONS };
namespace CommandLineOptions
{
    static bool Get( int i ) { return false; };
};
#endif

class InputCode
{
public:
    InputCode( unsigned int inputcode ) { mInputCode = inputcode; }
    InputCode( eControllerType controller, int dxKey, eDirectionType direction )
    {
        mInputCode = dxKey | (direction << 24) | (controller << 28);
    }

    eControllerType GetController() const
    {
        rAssert( (mInputCode >> 28) < NUM_CONTROLLERTYPES );
        return eControllerType( mInputCode >> 28 );
    }
    int GetDxKeyCode() const
    {
        return mInputCode & 0xFFFFFF;
    }
    eDirectionType GetDirection() const
    {
        rAssert( ((mInputCode >> 24) & 0xF) <= NUM_DIRTYPES );
        return eDirectionType( (mInputCode >> 24) & 0xF );
    }

    int GetInputCode() const { return mInputCode; }

private:
    unsigned int mInputCode;
};

UserController::UserController()
:
m_controllerId( -1 ),
mIsConnected( false ),
mGameState( Input::ACTIVE_ALL ),
mbInputPointsRegistered( false ),
mKeyboardBack( false ),
mbIsRumbleOn( false ),
m_bTutorialDisabled( true )
{
    int i = 0;
    for ( i = 0; i < Input::MaxMappables; i++ )
    {
        mMappable[ i ] = 0;
    }

    mNumButtons = VirtualInputs::GetNumber();
    rAssert(mNumButtons < Input::MaxPhysicalButtons);

    for( i = 0; i < mNumButtons; i++ )
    {
        mButtonNames[ i ] = radMakeKey( VirtualInputs::GetName( i ) );
        mButtonDeadZones[ i ] = 0.10f;
        mButtonSticky[ i ] = false;
    }

    m_pController[GAMEPAD] = new GamepadSDL();
    m_pController[KEYBOARD] = new KeyboardSDL();
    m_pController[MOUSE] = NULL;
    m_pController[STEERINGWHEEL] = NULL;

    memset( mPrevKeyState, 0, sizeof( mPrevKeyState ) );

    GetGameConfigManager()->RegisterConfig( this );
}

void UserController::NotifyConnect( void )
{
    if ( !IsConnected() )
    {
        for( int i = 0; i < NUM_CONTROLLERTYPES; i++ )
        {
            if( m_pController[i] != NULL )
                m_pController[i]->Connect();
        }

        for ( unsigned int i = 0; i < Input::MaxMappables; i++ )
        {
            if( mMappable[ i ] )
                mMappable[ i ]->OnControllerConnect( GetControllerId() );
        }

        mIsConnected = true;
    }
}

void UserController::NotifyDisconnect( void )
{
    if ( IsConnected() )
    {
        for( int i = 0; i < NUM_CONTROLLERTYPES; i++ )
        {
            if( m_pController[i] != NULL )
                m_pController[i]->Disconnect();
        }

        for ( unsigned int i = 0; i < Input::MaxMappables; i++ )
        {
            if( mMappable[ i ] )
                mMappable[ i ]->OnControllerDisconnect( GetControllerId() );
        }

        mIsConnected = false;
    }
}

bool UserController::IsConnected() const
{
    return mIsConnected;
}

void UserController::Create( int id )
{
    m_controllerId = id;
    LoadDefaults();
}

void UserController::SetGameState( unsigned state )
{
    mGameState = state;
    for ( unsigned i = 0; i < Input::MaxMappables; i++ )
    {
        if( mMappable[ i ] )
        {
            mMappable[ i ]->SetGameState( state );
            if( mMappable[ i ]->GetResync() )
            {
                mMappable[ i ]->InitButtons( m_controllerId, mButtonArray );
                mMappable[ i ]->SetResync( false );
            }
        }
    }
}

void UserController::SetButtonValue( unsigned int buttonId, float value, bool sticky )
{
    float fLastValue = mButtonArray[ buttonId ].GetValue();
    float fDeadZone = mButtonDeadZones[ buttonId ];

    if( mButtonSticky[ buttonId ] && !sticky )
    {
        return;
    }

    if ( rmt::Epsilon( value, 0.0f, fDeadZone ) )
    {
        value = 0.0f;
    }
    else
    {
        value = rmt::Clamp( value, MIN_AXIS_THRESH, MAX_AXIS_THRESH );

        float sign = rmt::Sign( value );
        float calibratedButtonData = rmt::Fabs( value ) - fDeadZone;
        calibratedButtonData *= 1.0f / ( 1.0f - fDeadZone );
        calibratedButtonData *= sign;

        bool bChanged = !( rmt::Epsilon( fLastValue, calibratedButtonData, 0.05f ) );

        if ( bChanged )
        {
            value = calibratedButtonData;
        }
        else
        {
            value = fLastValue;
        }
    }

    if( value > 0.0f || fLastValue > 0.0f )
    {
        mButtonArray[ buttonId ].SetValue( value );
    }

    mButtonSticky[ buttonId ] = sticky;
    if( sticky && value == 0.0f )
    {
        mButtonSticky[ buttonId ] = false;
    }
}

void UserController::OnControllerInputPointChange( unsigned int code, float value )
{
    InputCode icode( code );

    eControllerType cont = icode.GetController();
    int dxKey = icode.GetDxKeyCode();

    if( cont == KEYBOARD && dxKey == SDL_SCANCODE_ESCAPE && value > 0 )
    {
        mKeyboardBack = true;
    }

    // Route physical buttons directly to the cheat system, bypassing the
    // Mappable remapping system entirely. This ensures cheats always use
    // the default button layout regardless of user remapping.
    {
        int cheatInput = -1;
        if( cont == GAMEPAD )
        {
            switch( dxKey )
            {
                case SDLGP_A:              cheatInput = CHEAT_INPUT_0; break;
                case SDLGP_B:              cheatInput = CHEAT_INPUT_1; break;
                case SDLGP_X:              cheatInput = CHEAT_INPUT_2; break;
                case SDLGP_Y:              cheatInput = CHEAT_INPUT_3; break;
                case SDLGP_LEFTSHOULDER:   cheatInput = CHEAT_INPUT_LTRIGGER; break;
                case SDLGP_RIGHTSHOULDER:  cheatInput = CHEAT_INPUT_RTRIGGER; break;
            }
        }
        else if( cont == KEYBOARD )
        {
            switch( dxKey )
            {
                case SDL_SCANCODE_UP:      cheatInput = CHEAT_INPUT_0; break;
                case SDL_SCANCODE_DOWN:     cheatInput = CHEAT_INPUT_1; break;
                case SDL_SCANCODE_LEFT:     cheatInput = CHEAT_INPUT_2; break;
                case SDL_SCANCODE_RIGHT:    cheatInput = CHEAT_INPUT_3; break;
                case SDL_SCANCODE_F1:       cheatInput = CHEAT_INPUT_LTRIGGER; break;
            }
        }
        if( cheatInput >= 0 )
        {
            CheatInputSystem* pCheatSystem = CheatInputSystem::GetInstance();
            if( pCheatSystem != NULL && pCheatSystem->IsEnabled() )
            {
                pCheatSystem->HandlePhysicalButton( m_controllerId, cheatInput, value );
            }
        }
    }

    float dvalues[ NUM_DIRTYPES ];
    int ndirections = DeriveDirectionValues( cont, dxKey, value, dvalues );

    if( mMapData.MapNext )
    {
        Remap( cont, dxKey, dvalues, ndirections );
        return;
    }

    for( int map = 0; map < NUM_MAPTYPES; map++ )
    {
        for( int dir = 0; dir < ndirections; dir++ )
        {
            int vButton = m_pController[ cont ]->GetMap( dxKey, (eDirectionType) dir, (eMapType) map );
            if( vButton != Input::INVALID_CONTROLLERID )
            {
                SetButtonValue( vButton, dvalues[ dir ], cont == KEYBOARD );
            }
            if( vButton == InputManager::feBack && cont != KEYBOARD )
            {
                mKeyboardBack = false;
            }
        }
    }
}

int UserController::DeriveDirectionValues( eControllerType type, int dxKey, float value, float* values )
{
    values[ DIR_UP ] = values[ DIR_DOWN ] = values[ DIR_RIGHT ] = values[ DIR_LEFT ] = 0.0f;

    if( m_pController[ type ] != NULL && m_pController[ type ]->IsInputAxis( dxKey ) )
    {
        value = ( MAX_AXIS_THRESH - MIN_AXIS_THRESH ) * value + MIN_AXIS_THRESH;

        values[ DIR_UP ] = ( value >= 0 ) ? value : 0.0f;
        values[ DIR_DOWN ] = ( value >= 0 ) ? 0.0f : -value;

        return 2;
    }
    else
    {
        values[ DIR_UP ] = value;
        return 1;
    }
}

void UserController::PollKeyboard()
{
#if SDL_MAJOR_VERSION < 3
    const Uint8* keyState = SDL_GetKeyboardState( NULL );
#else
    const bool* keyState = SDL_GetKeyboardState( NULL );
#endif
    if( keyState == NULL )
        return;

    for( int i = 0; i < 512; i++ )
    {
        Uint8 current = keyState[i] ? 1 : 0;
        if( current != mPrevKeyState[i] )
        {
            mPrevKeyState[i] = current;
            float value = current ? 1.0f : 0.0f;
            InputCode icode( KEYBOARD, i, NUM_DIRTYPES );
            OnControllerInputPointChange( icode.GetInputCode(), value );
        }
    }
}

void UserController::Initialize( IRadController* pIController )
{
    rAssert(!mbInputPointsRegistered);

    // Disable the old hardcoded keyboard→gamepad merging in the SDL controller
    // since this UserController handles keyboard input directly
    radControllerSetKeyboardMappingDisabled( true );

    // Initialize the gamepad with the SDL controller
    if( pIController != NULL )
    {
        m_pController[GAMEPAD]->Init( pIController );
    }

    RegisterInputPoints();

    // Set up rumbling for the gamepad
    RADCONTROLLER pGamepad = m_pController[GAMEPAD]->getController();
    if( pGamepad != NULL )
    {
        IRadControllerOutputPoint* p_OutputPoint = pGamepad->GetOutputPointByName( "LeftMotor" );
        if ( p_OutputPoint )
        {
            mRumbleEffect.SetMotor( 0, p_OutputPoint );
        }

        p_OutputPoint = pGamepad->GetOutputPointByName( "RightMotor" );
        if ( p_OutputPoint )
        {
            mRumbleEffect.SetMotor( 1, p_OutputPoint );
        }
    }
}

void UserController::ReleaseRadController( void )
{
    if( mbInputPointsRegistered )
    {
        for( int i = 0; i < NUM_CONTROLLERTYPES; i++ )
        {
            if( m_pController[ i ] != NULL )
            {
                m_pController[ i ]->ReleaseInputPoints( this );
            }
        }

        for( int i = 0; i < mNumButtons; i++ )
        {
            mButtonArray[ i ].SetValue( 0.0f );
        }

        mbInputPointsRegistered = false;
    }

    mRumbleEffect.ShutDownEffects();
}

void UserController::SetRumble( bool bRumbleOn, bool pulse )
{
    if ( bRumbleOn && !mbIsRumbleOn && !CommandLineOptions::Get( CLO_NO_HAPTIC ) )
    {
        StartForceEffects();
    }
    else if ( !bRumbleOn && mbIsRumbleOn )
    {
        StopForceEffects();
    }

    if ( pulse )
    {
        PulseRumble();
    }

    if ( !bRumbleOn && mbIsRumbleOn )
    {
        mRumbleEffect.ShutDownEffects();
    }
    mbIsRumbleOn = bRumbleOn;
}

bool UserController::IsRumbleOn( void ) const
{
    return mbIsRumbleOn;
}

void UserController::PulseRumble()
{
    mRumbleEffect.SetEffect( RumbleEffect::PULSE, 500 );
}

void UserController::ApplyEffect( RumbleEffect::Effect effect, unsigned int durationms )
{
    if ( mbIsRumbleOn && !CommandLineOptions::Get( CLO_NO_HAPTIC ) )
    {
        mRumbleEffect.SetEffect( effect, durationms );
    }
}

void UserController::ApplyDynaEffect( RumbleEffect::DynaEffect effect, unsigned int durationms, float gain )
{
    if ( mbIsRumbleOn && !CommandLineOptions::Get( CLO_NO_HAPTIC ) )
    {
        mRumbleEffect.SetDynaEffect( effect, durationms, gain );
    }
}

void UserController::Update( unsigned timeins )
{
    if( !IsConnected() )
        return;

    PollKeyboard();

    for( unsigned int i = 0; i < Input::MaxPhysicalButtons; i++ )
    {
        unsigned int timesincechange = mButtonArray[ i ].TimeSinceChange();
        bool bIsDown = mButtonArray[ i ].IsDown();
        if( (timesincechange == 0) || bIsDown )
        {
            for ( unsigned j = 0; j < Input::MaxMappables; j++ )
            {
                if ( mMappable[ j ] )
                {
                    mMappable[ j ]->DispatchOnButton( m_controllerId, i, &mButtonArray[ i ] );
                }
            }
        }
    }

    mRumbleEffect.Update( timeins );
}

void UserController::StartForceEffects()
{
}

void UserController::StopForceEffects()
{
}

float UserController::GetInputValue( unsigned int index ) const
{
    switch( index )
    {
        case InputManager::LeftStickX:
        {
            float up = mButtonArray[ InputManager::CameraRight ].GetValue();
            float down = mButtonArray[ InputManager::CameraLeft ].GetValue();
            return ( up > down ) ? up : -down;
        }
        case InputManager::LeftStickY:
        {
            float up = mButtonArray[ InputManager::CameraMoveIn ].GetValue();
            float down = mButtonArray[ InputManager::CameraMoveOut ].GetValue();
            return ( up > down ) ? up : -down;
        }
        case InputManager::KeyboardEsc:
        {
            return mKeyboardBack ? 1.0f : 0.0f;
        }
        default:
            return mButtonArray[ index ].GetValue();
    }
}

float UserController::GetInputValueRT( unsigned int index ) const
{
    return 0.0f;
}

Button* UserController::GetInputButton( unsigned int index )
{
    return &mButtonArray[ index ];
}

UserController::~UserController( void )
{
    unsigned int i = 0;
    for ( i = 0; i < Input::MaxMappables; i++ )
    {
        if( mMappable[ i ] != NULL )
        {
            mMappable[ i ]->Release();
            mMappable[ i ] = 0;
        }
    }
    for( int i = 0; i < NUM_CONTROLLERTYPES; i++ )
    {
        delete m_pController[i];
        m_pController[i] = NULL;
    }
}

int UserController::RegisterMappable( Mappable *pMappable )
{
    unsigned int i = 0;
    for ( i = 0; i < Input::MaxMappables; i++ )
    {
        if( mMappable[ i ] == NULL )
        {
            break;
        }
    }

    if ( i < Input::MaxMappables )
    {
        mMappable[ i ] = pMappable;
        mMappable[ i ]->AddRef();
        mMappable[ i ]->LoadControllerMappings( m_controllerId );
        mMappable[ i ]->InitButtons( m_controllerId, mButtonArray );
    }
    else
    {
        rAssertMsg( false, "Not enough mappables allocated.\n" );
        return -1;
    }

    pMappable->SetResync( true );
    pMappable->SetGameState( mGameState );

    return (int)i;
}

void UserController::UnregisterMappable( int handle )
{
    rAssert( handle < static_cast< int >( Input::MaxMappables ) );
    if ( mMappable[ handle ] )
    {
        mMappable[ handle ]->Reset();
        mMappable[ handle ]->Release();
        mMappable[ handle ] = 0;
        LoadControllerMappings();
    }
}

void UserController::UnregisterMappable( Mappable* mappable )
{
    for ( unsigned i = 0; i < Input::MaxMappables; i++ )
    {
        if( mMappable[ i ] == mappable )
        {
            mMappable[ i ]->Reset();
            mMappable[ i ]->Release();
            mMappable[ i ] = 0;
            LoadControllerMappings();
            return;
        }
    }
}

void UserController::LoadControllerMappings( void )
{
    unsigned int i = 0;
    for ( i = 0; i < Input::MaxMappables; i++ )
    {
        if( mMappable[ i ] != NULL )
        {
            mMappable[ i ]->LoadControllerMappings( m_controllerId );
            mMappable[ i ]->InitButtons( m_controllerId, mButtonArray );
        }
    }
}

int UserController::GetIdByName( const char* pszName ) const
{
    radKey key = radMakeKey( pszName );
    unsigned int i;
    for( i = 0; i < Input::MaxPhysicalButtons; i++ )
    {
        if ( mButtonNames[i] == key )
        {
            return i;
        }
    }
    return -1;
}

void UserController::RegisterInputPoints()
{
    if( mbInputPointsRegistered )
    {
        return;
    }

    // Register input points for the gamepad (keyboard is polled directly)
    if( m_pController[GAMEPAD] != NULL )
    {
        RADCONTROLLER pController = m_pController[GAMEPAD]->getController();
        if( pController != NULL )
        {
            unsigned num = pController->GetNumberOfInputPoints();
            for( unsigned ip = 0; ip < num; ip++ )
            {
                RegisterInputPoint( GAMEPAD, ip );
            }
        }
    }

    mbInputPointsRegistered = true;
}

void UserController::RegisterInputPoint( eControllerType type, int inputpoint )
{
    if( m_pController[type] == NULL )
        return;

    RADCONTROLLER pController = m_pController[type]->getController();
    rAssert( pController != NULL );

    IRadControllerInputPoint* pInputPoint = pController->GetInputPointByIndex( inputpoint );
    rAssert( pInputPoint != NULL );

    int dxKey = m_pController[type]->GetDICode( inputpoint );

    if( dxKey != Input::INVALID_CONTROLLERID && !m_pController[type]->IsBannedInput( dxKey ) )
    {
        InputCode icode( type, dxKey, NUM_DIRTYPES );
        pInputPoint->RegisterControllerInputPointCallback( this, icode.GetInputCode() );
        m_pController[type]->AddInputPoints( pInputPoint );
    }
}

const char* UserController::GetConfigName() const
{
    switch( m_controllerId )
    {
        case -1: return "Control-default";
        case 0:  return "Controller";
        case 1:  return "Controller1";
        case 2:  return "Controller2";
        case 3:  return "Controller3";
        default:
            rAssert( false );
            return "Controller3";
    }
}

int UserController::GetNumProperties() const
{
    return Input::MaxVirtualMappings * Input::MaxPhysicalButtons + 2;
}

void UserController::LoadDefaults()
{
    ClearMappings();

    if ( m_controllerId == 0 )
    {
        // Primary mapping: keyboard
        SetMap( SLOT_PRIMARY, InputManager::MoveUp, KEYBOARD, SDL_SCANCODE_W );
        SetMap( SLOT_PRIMARY, InputManager::MoveDown, KEYBOARD, SDL_SCANCODE_S );
        SetMap( SLOT_PRIMARY, InputManager::MoveLeft, KEYBOARD, SDL_SCANCODE_A );
        SetMap( SLOT_PRIMARY, InputManager::MoveRight, KEYBOARD, SDL_SCANCODE_D );
        SetMap( SLOT_PRIMARY, InputManager::Jump, KEYBOARD, SDL_SCANCODE_SPACE );
        SetMap( SLOT_PRIMARY, InputManager::Sprint, KEYBOARD, SDL_SCANCODE_LSHIFT );
        SetMap( SLOT_PRIMARY, InputManager::Attack, KEYBOARD, SDL_SCANCODE_Q );
        SetMap( SLOT_PRIMARY, InputManager::DoAction, KEYBOARD, SDL_SCANCODE_E );

        SetMap( SLOT_PRIMARY, InputManager::Accelerate, KEYBOARD, SDL_SCANCODE_W );
        SetMap( SLOT_PRIMARY, InputManager::Reverse, KEYBOARD, SDL_SCANCODE_S );
        SetMap( SLOT_PRIMARY, InputManager::SteerLeft, KEYBOARD, SDL_SCANCODE_A );
        SetMap( SLOT_PRIMARY, InputManager::SteerRight, KEYBOARD, SDL_SCANCODE_D );
        SetMap( SLOT_PRIMARY, InputManager::Horn, KEYBOARD, SDL_SCANCODE_LSHIFT );
        SetMap( SLOT_PRIMARY, InputManager::ResetCar, KEYBOARD, SDL_SCANCODE_R );
        SetMap( SLOT_PRIMARY, InputManager::GetOutCar, KEYBOARD, SDL_SCANCODE_F );
        SetMap( SLOT_PRIMARY, InputManager::HandBrake, KEYBOARD, SDL_SCANCODE_LCTRL );

        SetMap( SLOT_PRIMARY, InputManager::CameraLeft, KEYBOARD, SDL_SCANCODE_KP_4 );
        SetMap( SLOT_PRIMARY, InputManager::CameraRight, KEYBOARD, SDL_SCANCODE_KP_6 );
        SetMap( SLOT_PRIMARY, InputManager::CameraMoveIn, KEYBOARD, SDL_SCANCODE_KP_8 );
        SetMap( SLOT_PRIMARY, InputManager::CameraMoveOut, KEYBOARD, SDL_SCANCODE_KP_2 );
        SetMap( SLOT_PRIMARY, InputManager::CameraZoom, KEYBOARD, SDL_SCANCODE_KP_5 );
        SetMap( SLOT_PRIMARY, InputManager::CameraLookUp, KEYBOARD, SDL_SCANCODE_KP_0 );
        SetMap( SLOT_PRIMARY, InputManager::CameraToggle, KEYBOARD, SDL_SCANCODE_KP_0 );
        SetMap( SLOT_PRIMARY, InputManager::CameraCarLeft, KEYBOARD, SDL_SCANCODE_KP_4 );
        SetMap( SLOT_PRIMARY, InputManager::CameraCarRight, KEYBOARD, SDL_SCANCODE_KP_6 );
        SetMap( SLOT_PRIMARY, InputManager::CameraCarLookUp, KEYBOARD, SDL_SCANCODE_KP_8 );
        SetMap( SLOT_PRIMARY, InputManager::CameraCarLookBack, KEYBOARD, SDL_SCANCODE_KP_2 );
    }

    // Secondary mapping: gamepad
    SetMap( SLOT_SECONDARY, InputManager::MoveUp, GAMEPAD, SDLGP_LEFTSTICK_Y, DIR_UP );
    SetMap( SLOT_SECONDARY, InputManager::MoveDown, GAMEPAD, SDLGP_LEFTSTICK_Y, DIR_DOWN );
    SetMap( SLOT_SECONDARY, InputManager::MoveLeft, GAMEPAD, SDLGP_LEFTSTICK_X, DIR_DOWN );
    SetMap( SLOT_SECONDARY, InputManager::MoveRight, GAMEPAD, SDLGP_LEFTSTICK_X, DIR_UP );
    SetMap( SLOT_SECONDARY, InputManager::Attack, GAMEPAD, SDLGP_Y );
    SetMap( SLOT_SECONDARY, InputManager::Jump, GAMEPAD, SDLGP_B );
    SetMap( SLOT_SECONDARY, InputManager::Sprint, GAMEPAD, SDLGP_A );
    SetMap( SLOT_SECONDARY, InputManager::DoAction, GAMEPAD, SDLGP_X );

    SetMap( SLOT_SECONDARY, InputManager::Accelerate, GAMEPAD, SDLGP_A );
    SetMap( SLOT_SECONDARY, InputManager::Reverse, GAMEPAD, SDLGP_LEFTTRIGGER );
    SetMap( SLOT_SECONDARY, InputManager::SteerLeft, GAMEPAD, SDLGP_LEFTSTICK_X, DIR_DOWN );
    SetMap( SLOT_SECONDARY, InputManager::SteerRight, GAMEPAD, SDLGP_LEFTSTICK_X, DIR_UP );
    SetMap( SLOT_SECONDARY, InputManager::GetOutCar, GAMEPAD, SDLGP_Y );
    SetMap( SLOT_SECONDARY, InputManager::HandBrake, GAMEPAD, SDLGP_B );
    SetMap( SLOT_SECONDARY, InputManager::Horn, GAMEPAD, SDLGP_LEFTSHOULDER );
    SetMap( SLOT_SECONDARY, InputManager::ResetCar, GAMEPAD, SDLGP_DPAD_UP );

    SetMap( SLOT_SECONDARY, InputManager::CameraLeft, GAMEPAD, SDLGP_RIGHTSTICK_X, DIR_DOWN );
    SetMap( SLOT_SECONDARY, InputManager::CameraRight, GAMEPAD, SDLGP_RIGHTSTICK_X, DIR_UP );
    SetMap( SLOT_SECONDARY, InputManager::CameraMoveIn, GAMEPAD, SDLGP_RIGHTSTICK_Y, DIR_UP );
    SetMap( SLOT_SECONDARY, InputManager::CameraMoveOut, GAMEPAD, SDLGP_RIGHTSTICK_Y, DIR_DOWN );
    SetMap( SLOT_SECONDARY, InputManager::CameraZoom, GAMEPAD, SDLGP_LEFTTRIGGER );
    SetMap( SLOT_SECONDARY, InputManager::CameraLookUp, GAMEPAD, SDLGP_RIGHTTRIGGER );
    SetMap( SLOT_SECONDARY, InputManager::CameraToggle, GAMEPAD, SDLGP_RIGHTSHOULDER );
    SetMap( SLOT_SECONDARY, InputManager::CameraCarLeft, GAMEPAD, SDLGP_RIGHTSTICK_X, DIR_DOWN );
    SetMap( SLOT_SECONDARY, InputManager::CameraCarRight, GAMEPAD, SDLGP_RIGHTSTICK_X, DIR_UP );
    SetMap( SLOT_SECONDARY, InputManager::CameraCarLookUp, GAMEPAD, SDLGP_RIGHTSTICK_Y, DIR_UP );
    SetMap( SLOT_SECONDARY, InputManager::CameraCarLookBack, GAMEPAD, SDLGP_RIGHTSTICK_Y, DIR_DOWN );

    m_bTutorialDisabled = false;

    TutorialManager* pTutorialManager = GetTutorialManager();
    if( pTutorialManager )
        pTutorialManager->EnableTutorialMode( m_bTutorialDisabled );
}

void UserController::LoadConfig( ConfigString& config )
{
    ClearMappings();

    char property[ ConfigString::MaxLength ];
    char value[ ConfigString::MaxLength ];

    int map;
    int virtualKey;
    int cont;
    int dxKey;
    int dir;

    while ( config.ReadProperty( property, value ) )
    {
        if( strcasecmp( property, "buttonmap" ) == 0 )
        {
            int ret = sscanf( value, "%d, %d: ( %d %d %d )", &map, &virtualKey, &cont, &dxKey, &dir );

            if( ret == 5 &&
                map >= 0 && map < Input::MaxVirtualMappings &&
                virtualKey >= 0 && virtualKey < (int) mNumButtons &&
                VirtualInputs::GetType( virtualKey ) != MAP_FRONTEND &&
                cont >= 0 && cont < NUM_CONTROLLERTYPES &&
                m_pController[ cont ] != NULL &&
                m_pController[ cont ]->IsValidInput( dxKey ) &&
                dir >= 0 && dir < NUM_DIRTYPES
              )
            {
                SetMap( map, virtualKey, (eControllerType) cont, dxKey, (eDirectionType) dir );
            }
        }
        else if( strcasecmp( property, "disabletutorials" ) == 0 )
        {
            if( strcmp( value, "yes" ) == 0 )
            {
                m_bTutorialDisabled = true;
            }
            else if( strcmp( value, "no" ) == 0 )
            {
                m_bTutorialDisabled = false;
            }
        }
    }
}

void UserController::SaveConfig( ConfigString& config )
{
    char value[ ConfigString::MaxLength ];
    eControllerType cont;
    int dxKey;
    eDirectionType dir;

    for( int map = 0; map < Input::MaxVirtualMappings; map++ )
    {
        for( int vk = 0; vk < (int) mNumButtons; vk++ )
        {
            if( VirtualInputs::GetType( vk ) != MAP_FRONTEND &&
                GetMap( map, vk, cont, dxKey, dir ) )
            {
                sprintf( value, "%d, %d: ( %d %d %d )", map, vk, cont, dxKey, dir );
                config.WriteProperty( "buttonmap", value );
            }
        }
    }

    config.WriteProperty( "disabletutorials", m_bTutorialDisabled ? "yes" : "no" );
}

void UserController::RemapButton( int map, int VirtualButton, ButtonMappedCallback* callback )
{
    rAssert( map >= 0 && map < Input::MaxVirtualMappings );
    rAssert( VirtualButton >= 0 && VirtualButton < mNumButtons );
    rAssert( VirtualInputs::GetType( VirtualButton ) != MAP_FRONTEND );

    mMapData.MapNext = true;
    mMapData.map = map;
    mMapData.virtualButton = VirtualButton;
    mMapData.callback = callback;
}

void UserController::SetMap( int map, int virtualKey, eControllerType cont, int dxKey, eDirectionType dir )
{
    rAssert( map >= 0 && map < Input::MaxVirtualMappings );
    rAssert( virtualKey >= 0 && virtualKey < mNumButtons );
    if( m_pController[ cont ] == NULL )
        return;
    rAssert( m_pController[ cont ]->IsValidInput( dxKey ) );
    rAssert( VirtualInputs::GetType( virtualKey ) != MAP_FRONTEND );

    InputCode icode( cont, dxKey, dir );

    eControllerType oldcont;
    int olddxKey;
    eDirectionType olddir;
    if( GetMap( map, virtualKey, oldcont, olddxKey, olddir ) )
    {
        if( m_pController[ oldcont ] != NULL )
            m_pController[ oldcont ]->ClearMap( olddxKey, olddir, virtualKey );
    }

    eMapType maptype = VirtualInputs::GetType( virtualKey );
    int old_vb = m_pController[ cont ]->GetMap( dxKey, dir, maptype );
    if( old_vb != Input::INVALID_CONTROLLERID )
    {
        for( int i = 0; i < Input::MaxVirtualMappings; i++ )
        {
            if( mVirtualMap[ i ].GetLogicalIndex( old_vb ) == icode.GetInputCode() )
            {
                mVirtualMap[ i ].SetAssociation( old_vb, Input::INVALID_CONTROLLERID );
            }
        }
    }

    mVirtualMap[ map ].SetAssociation( virtualKey, icode.GetInputCode() );
    m_pController[ cont ]->SetMap( dxKey, dir, virtualKey );
}

bool UserController::GetMap( int map, int virtualKey, eControllerType& type, int& dxKey, eDirectionType& dir ) const
{
    rAssert( map >= 0 && map < Input::MaxVirtualMappings );
    rAssert( virtualKey >= 0 && virtualKey < mNumButtons );

    int code = mVirtualMap[ map ].GetLogicalIndex( virtualKey );

    if( code != Input::INVALID_CONTROLLERID )
    {
        InputCode icode = code;
        type = icode.GetController();
        dxKey = icode.GetDxKeyCode();
        dir = icode.GetDirection();
    }

    return code != Input::INVALID_CONTROLLERID;
}

const char* UserController::GetMap( int map, int virtualKey, int& numDirs, eControllerType& cont, eDirectionType& dir ) const
{
    int dxKey;

    if( !GetMap( map, virtualKey, cont, dxKey, dir ) ||
        m_pController[ cont ] == NULL )
    {
        return NULL;
    }

    if( m_pController[ cont ]->IsInputAxis( dxKey ) )
    {
        numDirs = 2;
    }
    else
    {
        numDirs = 1;
    }

    if( cont == KEYBOARD )
    {
        const char* name = KeyboardSDL::GetSDLKeyName( dxKey );
        if( name != NULL && name[0] != '\0' )
        {
            return name;
        }
        return "Unknown Key";
    }
    else if( m_pController[ cont ]->getController() != NULL )
    {
        return m_pController[ cont ]->GetInputName( dxKey );
    }

    return NULL;
}

void UserController::Remap( eControllerType cont, int dxKey, float* dvalues, int ndirections )
{
    if( cont == KEYBOARD &&
        m_pController[ cont ] != NULL &&
        m_pController[ cont ]->GetMap( dxKey, DIR_UP, MAP_FRONTEND ) == InputManager::feBack )
    {
        mMapData.MapNext = false;
        mMapData.callback->OnButtonMapped( NULL, cont, 1, NUM_DIRTYPES );
    }
    else
    {
        for( int dir = 0; dir < ndirections; dir++ )
        {
            if( dvalues[ dir ] >= MAPPING_DEADZONE )
            {
                SetMap( mMapData.map, mMapData.virtualButton, cont, dxKey, (eDirectionType) dir );

                mMapData.MapNext = false;

                const char* inputName = NULL;
                if( cont == KEYBOARD )
                {
                    inputName = KeyboardSDL::GetSDLKeyName( dxKey );
                }
                else if( m_pController[ cont ] != NULL )
                {
                    inputName = m_pController[ cont ]->GetInputName( dxKey );
                }

                mMapData.callback->OnButtonMapped( inputName, cont, ndirections, (eDirectionType) dir );

                break;
            }
        }
    }
}

void UserController::ClearMappings()
{
    for( int i = 0; i < Input::MaxVirtualMappings; i++ )
    {
        mVirtualMap[ i ].ClearAssociations();
    }

    for( int j = 0; j < NUM_CONTROLLERTYPES; j++ )
    {
        if( m_pController[ j ] != NULL )
            m_pController[ j ]->ClearMappedButtons();
    }

    LoadFEMappings();
}

void UserController::LoadFEMappings()
{
    if( m_pController[KEYBOARD] == NULL )
        return;

    switch ( m_controllerId )
    {
    case 0:
        {
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_ESCAPE, DIR_UP, InputManager::feBack );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_UP, DIR_UP, InputManager::feMoveUp );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_DOWN, DIR_UP, InputManager::feMoveDown );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_LEFT, DIR_UP, InputManager::feMoveLeft );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_RIGHT, DIR_UP, InputManager::feMoveRight );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_RETURN, DIR_UP, InputManager::feSelect );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_KP_8, DIR_UP, InputManager::feMoveUp );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_KP_2, DIR_UP, InputManager::feMoveDown );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_KP_4, DIR_UP, InputManager::feMoveLeft );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_KP_6, DIR_UP, InputManager::feMoveRight );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_KP_ENTER, DIR_UP, InputManager::feSelect );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_F1, DIR_UP, InputManager::feFunction1 );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_F2, DIR_UP, InputManager::feFunction2 );
            break;
        }
    case 3:
        {
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_F4, DIR_UP, InputManager::P1_KBD_Start );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_O, DIR_UP, InputManager::P1_KBD_Gas );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_L, DIR_UP, InputManager::P1_KBD_Brake );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_RETURN, DIR_UP, InputManager::P1_KBD_EBrake );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_RSHIFT, DIR_UP, InputManager::P1_KBD_Nitro );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_K, DIR_UP, InputManager::P1_KBD_Left );
            m_pController[KEYBOARD]->SetMap( SDL_SCANCODE_SEMICOLON, DIR_UP, InputManager::P1_KBD_Right );
            break;
        }
    }

    if( m_pController[GAMEPAD] != NULL )
    {
        m_pController[GAMEPAD]->SetMap( SDLGP_LEFTSTICK_Y, DIR_UP, InputManager::feMoveUp );
        m_pController[GAMEPAD]->SetMap( SDLGP_LEFTSTICK_Y, DIR_DOWN, InputManager::feMoveDown );
        m_pController[GAMEPAD]->SetMap( SDLGP_LEFTSTICK_X, DIR_UP, InputManager::feMoveRight );
        m_pController[GAMEPAD]->SetMap( SDLGP_LEFTSTICK_X, DIR_DOWN, InputManager::feMoveLeft );
        m_pController[GAMEPAD]->SetMap( SDLGP_A, DIR_UP, InputManager::feSelect );
        m_pController[GAMEPAD]->SetMap( SDLGP_B, DIR_UP, InputManager::feBack );
        m_pController[GAMEPAD]->SetMap( SDLGP_START, DIR_UP, InputManager::feStart );
        m_pController[GAMEPAD]->SetMap( SDLGP_DPAD_UP, DIR_UP, InputManager::feMoveUp );
        m_pController[GAMEPAD]->SetMap( SDLGP_DPAD_DOWN, DIR_UP, InputManager::feMoveDown );
        m_pController[GAMEPAD]->SetMap( SDLGP_DPAD_RIGHT, DIR_UP, InputManager::feMoveRight );
        m_pController[GAMEPAD]->SetMap( SDLGP_DPAD_LEFT, DIR_UP, InputManager::feMoveLeft );
    }
}

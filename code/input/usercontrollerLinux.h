#ifndef USERCONTROLLERLINUX_HPP
#define USERCONTROLLERLINUX_HPP

#ifndef RAD_LINUX
#error 'This implementation of the user controller is specific to Linux.'
#endif

#include <radcontroller.hpp>
#include <radkey.hpp>
#include <SDL.h>

#include <data/config/gameconfig.h>
#include <input/controller.h>
#include <input/button.h>
#include <input/mapper.h>
#include <input/rumbleeffect.h>
#include <input/RealController.h>

class Mappable;
class Mapper;

enum eVirtualMapSlot
{
    SLOT_PRIMARY = 0,
    SLOT_SECONDARY
};

#define MAX_AXIS_THRESH 1.0f
#define MIN_AXIS_THRESH -1.0f
#define MAPPING_DEADZONE 0.5f

struct ButtonMappedCallback
{
    virtual void OnButtonMapped( const char* InputName, eControllerType cont, int num_dirs, eDirectionType direction ) = 0;
};

struct ButtonMapData
{
    ButtonMapData() : MapNext(false) {};

    bool MapNext;
    int map;
    int virtualButton;
    ButtonMappedCallback* callback;
};

class UserController :
    public IRadControllerInputPointCallback,
    public GameConfigHandler
{
public:
    UserController( void );
    virtual ~UserController( void );

    virtual void Create( int id );
    virtual void Initialize( IRadController* pIController );
    virtual void ReleaseRadController( void );

    RealController* GetRealController( eControllerType type ) { return m_pController[type]; }
    unsigned int GetControllerId( void ) const { return m_controllerId; }

    void Update( unsigned timeins );

    void StartForceEffects();
    void StopForceEffects();

    void SetGameState(unsigned);

    void SetRumble( bool bRumbleOn, bool pulse = false );
    bool IsRumbleOn( void ) const;
    void PulseRumble();
    void ApplyEffect( RumbleEffect::Effect effect, unsigned int durationms );
    void ApplyDynaEffect( RumbleEffect::DynaEffect effect, unsigned int durationms, float gain );

    bool IsConnected( void ) const;
    void NotifyConnect( void );
    void NotifyDisconnect( void );

    float GetInputValue( unsigned int index ) const;
    Button* GetInputButton( unsigned int index );
    float GetInputValueRT( unsigned int index ) const;

    int RegisterMappable( Mappable* pMappable );
    void UnregisterMappable( int handle );
    void UnregisterMappable( Mappable* pMappable );

    void LoadControllerMappings( void );

    int GetIdByName( const char* pszName ) const;

    Mappable* GetMappable( unsigned int which ) { return mMappable[ which ]; };

    bool IsWheel() const { return false; }

    virtual const char* GetConfigName() const;
    virtual int GetNumProperties() const;
    virtual void LoadDefaults();
    virtual void LoadConfig( ConfigString& config );
    virtual void SaveConfig( ConfigString& config );

    const char* GetMap( int map, int virtualKey, int& numDirs, eControllerType& cont, eDirectionType& dir ) const;
    void RemapButton( int map, int VirtualButton, ButtonMappedCallback* callback );

    void SetTutorialDisabled( bool bDisabled ) { m_bTutorialDisabled = bDisabled; }
    bool IsTutorialDisabled() const { return m_bTutorialDisabled; }

protected:
    void RegisterInputPoints();
    void RegisterInputPoint( eControllerType type, int inputpoint );
    void OnControllerInputPointChange( unsigned int buttonId, float value );
    int DeriveDirectionValues( eControllerType type, int dxKey, float value, float* dir_values );
    void SetButtonValue( unsigned int virtualButton, float value, bool sticky );
    void PollKeyboard();

    void SetMap( int map, int virtualKey, eControllerType cont, int dxKey, eDirectionType dir = DIR_UP );
    bool GetMap( int map, int virtualKey, eControllerType& cont, int& dxKey, eDirectionType& dir ) const;
    void Remap( eControllerType cont, int dxKey, float* dvalues, int directions );
    void ClearMappings();
    void LoadFEMappings();

private:
    int m_controllerId;
    bool mIsConnected;

    unsigned mGameState;
    bool mbInputPointsRegistered;

    RealController* m_pController[ NUM_CONTROLLERTYPES ];

    Mappable* mMappable[ Input::MaxMappables ];

    Mapper mVirtualMap[ Input::MaxVirtualMappings ];
    ButtonMapData mMapData;

    int mNumButtons;
    Button mButtonArray[ Input::MaxPhysicalButtons ];
    radKey mButtonNames[ Input::MaxPhysicalButtons ];
    float  mButtonDeadZones[ Input::MaxPhysicalButtons ];
    bool   mButtonSticky[ Input::MaxPhysicalButtons ];

    bool mKeyboardBack;

    bool mbIsRumbleOn;
    RumbleEffect mRumbleEffect;

    bool m_bTutorialDisabled;

    // Previous keyboard state for change detection in PollKeyboard()
    Uint8 mPrevKeyState[ 512 ];

    // Name buffer for returning key names from GetMap
    mutable char mKeyNameBuffer[ 64 ];
};

#endif

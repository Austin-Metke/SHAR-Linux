//===========================================================================
// Copyright (C) 2000 Radical Entertainment Ltd.  All rights reserved.
//
// Component:   CheatInputHandler
//
// Description: Implementation of the CheatInputHandler class.
//
// Authors:     Tony Chu
//
// Revisions		Date			Author	    Revision
//                  2002/09/19      TChu        Created for SRR2
//
//===========================================================================

//===========================================================================
// Includes
//===========================================================================
#include <cheats/cheatinputhandler.h>
#include <cheats/cheatinputsystem.h>

//===========================================================================
// Local Constants
//===========================================================================

struct CheatInputMapping
{
    const char* inputName;
    int inputID;
};

static const CheatInputMapping CHEAT_INPUT_MAPPINGS[] =
{
#ifdef RAD_GAMECUBE
    { "A",              CHEAT_INPUT_0 },
    { "B",              CHEAT_INPUT_1 },
    { "X",              CHEAT_INPUT_2 },
    { "Y",              CHEAT_INPUT_3 },
    { "TriggerL",       CHEAT_INPUT_LTRIGGER },
    { "TriggerR",       CHEAT_INPUT_RTRIGGER },
#endif

#ifdef RAD_PS2
    { "X",              CHEAT_INPUT_0 },
    { "Circle",         CHEAT_INPUT_1 },
    { "Square",         CHEAT_INPUT_2 },
    { "Triangle",       CHEAT_INPUT_3 },
    { "L1",             CHEAT_INPUT_LTRIGGER },
    { "R1",             CHEAT_INPUT_RTRIGGER },
#endif

#if defined(RAD_XBOX) || defined(RAD_CONSOLE) && defined(RAD_WIN32)
    { "A",              CHEAT_INPUT_0 },
    { "B",              CHEAT_INPUT_1 },
    { "X",              CHEAT_INPUT_2 },
    { "Y",              CHEAT_INPUT_3 },
    { "Black",          CHEAT_INPUT_LTRIGGER },
    { "White",          CHEAT_INPUT_RTRIGGER },
    { "LeftTrigger",    CHEAT_INPUT_LTRIGGER },
    { "RightTrigger",   CHEAT_INPUT_RTRIGGER },
#endif

#if defined(RAD_PC) || defined(RAD_LINUX)
    // All cheat input on PC/Linux is routed through HandlePhysicalButton
    // from UserController::OnControllerInputPointChange, bypassing the
    // Mappable remapping system entirely.
#endif

    { "",               UNKNOWN_CHEAT_INPUT }
};

static const int unsigned NUM_CHEAT_INPUT_MAPPINGS =
    sizeof( CHEAT_INPUT_MAPPINGS ) / sizeof( CHEAT_INPUT_MAPPINGS[ 0 ] );

//===========================================================================
// Public Member Functions
//===========================================================================

//===========================================================================
// CheatInputHandler::CheatInputHandler
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      
//
//===========================================================================
CheatInputHandler::CheatInputHandler()
:   m_LTriggerBitMask( 0 ),
    m_RTriggerBitMask( 0 ),
    m_currentInputIndex( 0 )
{
    this->ResetInputSequence();

    for( unsigned int i = 0; i < NUM_AUXILIARY_CHEAT_INPUTS; i++ )
    {
        m_prevPhysicalButtonState[ i ] = false;
    }
}

//===========================================================================
// CheatInputHandler::~CheatInputHandler
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      
//
//===========================================================================
CheatInputHandler::~CheatInputHandler()
{
}

//===========================================================================
// CheatInputHandler::ResetInputSequence
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      
//
//===========================================================================
void
CheatInputHandler::ResetInputSequence()
{
    m_currentInputIndex = 0;

    // just to be sure
    //
    for( unsigned int i = 0; i < NUM_CHEAT_SEQUENCE_INPUTS; i++ )
    {
        m_inputSequence[ i ] = UNKNOWN_CHEAT_INPUT;
    }
}

//===========================================================================
// CheatInputHandler::GetInputName
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      
//
//===========================================================================
const char*
CheatInputHandler::GetInputName( eCheatInput cheatInput )
{
#if defined(RAD_PC) || defined(RAD_LINUX)
    // On PC/Linux, cheat input bypasses the Mappable mapping table entirely,
    // so CHEAT_INPUT_MAPPINGS has no entries. Return fixed button names.
    static const char* s_inputNames[] = { "Y", "B", "A", "X" };
    if( cheatInput >= 0 && cheatInput < NUM_CHEAT_INPUTS )
    {
        return s_inputNames[ cheatInput ];
    }
    return "";
#else
    rAssert( cheatInput < static_cast<int>( NUM_CHEAT_INPUT_MAPPINGS ) );

    return CHEAT_INPUT_MAPPINGS[ cheatInput ].inputName;
#endif
}

//===========================================================================
// CheatInputHandler::OnButton
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      
//
//===========================================================================
void CheatInputHandler::OnButton( int controllerId,
                                  int buttonId,
                                  const IButton* pButton )
{
}

//===========================================================================
// CheatInputHandler::OnButtonDown
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      
//
//===========================================================================
void CheatInputHandler::OnButtonDown( int controllerId,
                                      int buttonId,
                                      const IButton* pButton )
{
    switch( buttonId )
    {
        case CHEAT_INPUT_LTRIGGER:
        {
            m_LTriggerBitMask |= (1 << controllerId);

#if defined(RAD_LINUX) || defined(RAD_PC)
            // On PC/Linux, activate on either trigger so keyboard F1 alone works
            GetCheatInputSystem()->SetActivated( controllerId, true );
#else
            bool isRTriggerDown = ((m_RTriggerBitMask & (1 << controllerId)) > 0);
            GetCheatInputSystem()->SetActivated( controllerId,
                                                 isRTriggerDown );
#endif

            break;
        }
        case CHEAT_INPUT_RTRIGGER:
        {
            m_RTriggerBitMask |= (1 << controllerId);

#if defined(RAD_LINUX) || defined(RAD_PC)
            // On PC/Linux, activate on either trigger so keyboard F1 alone works
            GetCheatInputSystem()->SetActivated( controllerId, true );
#else
            bool isLTriggerDown = ((m_LTriggerBitMask & (1 << controllerId)) > 0);
            GetCheatInputSystem()->SetActivated( controllerId,
                                                 isLTriggerDown );
#endif

            break;
        }
        default:
        {
            if( GetCheatInputSystem()->IsActivated( controllerId ) )
            {
                rAssert( buttonId < NUM_CHEAT_INPUTS );

                rReleasePrintf( "Received Cheat Input [%d] = [%d]\n",
                                m_currentInputIndex, buttonId );

                // add input to current sequence
                //
                m_inputSequence[ m_currentInputIndex++ ] = static_cast<eCheatInput>( buttonId );
                m_currentInputIndex %= NUM_CHEAT_SEQUENCE_INPUTS;

                // if this is the last input for the current sequence,
                // send current sequence to cheat input system for
                // validation
                //
                if( m_currentInputIndex == 0 )
                {
                    GetCheatInputSystem()->ReceiveInputs( m_inputSequence );
                }
            }

            break;
        }
    }
}

//===========================================================================
// CheatInputHandler::OnButtonUp
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      
//
//===========================================================================
void CheatInputHandler::OnButtonUp( int controllerId,
                                    int buttonId,
                                    const IButton* pButton )
{
    switch( buttonId )
    {
        case CHEAT_INPUT_LTRIGGER:
        {
            m_LTriggerBitMask &= ~(1 << controllerId);

#if defined(RAD_LINUX) || defined(RAD_PC)
            // Stay activated if the other trigger is still held
            {
                bool isRTriggerDown = ((m_RTriggerBitMask & (1 << controllerId)) > 0);
                GetCheatInputSystem()->SetActivated( controllerId, isRTriggerDown );
            }
#else
            GetCheatInputSystem()->SetActivated( controllerId, false );
#endif

            break;
        }
        case CHEAT_INPUT_RTRIGGER:
        {
            m_RTriggerBitMask &= ~(1 << controllerId);

#if defined(RAD_LINUX) || defined(RAD_PC)
            // Stay activated if the other trigger is still held
            {
                bool isLTriggerDown = ((m_LTriggerBitMask & (1 << controllerId)) > 0);
                GetCheatInputSystem()->SetActivated( controllerId, isLTriggerDown );
            }
#else
            GetCheatInputSystem()->SetActivated( controllerId, false );
#endif

            break;
        }
        default:
        {
            break;
        }
    }
}

//===========================================================================
// CheatInputHandler::HandlePhysicalButton
//===========================================================================
// Description: Process a physical controller button directly, bypassing the
//              Mappable remapping system. This ensures cheats always use the
//              default physical button layout regardless of user remapping.
//
// Constraints:	None.
//
// Parameters:	controllerId - the controller index
//              cheatInputId - CHEAT_INPUT_0..3 or CHEAT_INPUT_LTRIGGER/RTRIGGER
//              value        - 0.0 for released, >0 for pressed
//
// Return:      None.
//
//===========================================================================
void CheatInputHandler::HandlePhysicalButton( int controllerId,
                                              int cheatInputId,
                                              float value )
{
    rAssert( cheatInputId >= 0 && cheatInputId < NUM_AUXILIARY_CHEAT_INPUTS );

    bool wasDown = m_prevPhysicalButtonState[ cheatInputId ];
    bool isDown = ( value > 0.0f );
    m_prevPhysicalButtonState[ cheatInputId ] = isDown;

    if( isDown && !wasDown )
    {
        Button btn;
        btn.SetValue( value );
        OnButtonDown( controllerId, cheatInputId, &btn );
    }
    else if( !isDown && wasDown )
    {
        Button btn;
        btn.SetValue( 0.0f );
        OnButtonUp( controllerId, cheatInputId, &btn );
    }
}

//===========================================================================
// CheatInputHandler::LoadControllerMappings
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      
//
//===========================================================================
void CheatInputHandler::LoadControllerMappings( unsigned int controllerId )
{
    for( unsigned int i = 0; i < NUM_CHEAT_INPUT_MAPPINGS; i++ )
    {
        this->Map( CHEAT_INPUT_MAPPINGS[ i ].inputName,
                   CHEAT_INPUT_MAPPINGS[ i ].inputID,
                   0,
                   controllerId );
/*
        rTunePrintf( "Load Mapping: %s, %d, %d\n",
                     CHEAT_INPUT_MAPPINGS[ i ].inputName,
                     CHEAT_INPUT_MAPPINGS[ i ].inputID,
                     controllerId );
*/
    }
}


//===========================================================================
// Private Member Functions
//===========================================================================


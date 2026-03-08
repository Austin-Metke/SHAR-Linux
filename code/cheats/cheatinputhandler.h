//===========================================================================
// Copyright (C) 2000 Radical Entertainment Ltd.  All rights reserved.
//
// Component:   CheatInputHandler
//
// Description: Interface for the CheatInputHandler class.
//
// Authors:     Tony Chu
//
// Revisions		Date			Author	    Revision
//                  2002/09/19      TChu        Created for SRR2
//
//===========================================================================

#ifndef CHEATINPUTHANDLER_H
#define CHEATINPUTHANDLER_H

//===========================================================================
// Nested Includes
//===========================================================================

#include <cheats/cheatinputs.h>
#include <input/mappable.h>

//===========================================================================
// Forward References
//===========================================================================

enum eAuxiliaryCheatInput
{
    CHEAT_INPUT_LTRIGGER = NUM_CHEAT_INPUTS,
    CHEAT_INPUT_RTRIGGER,

    NUM_AUXILIARY_CHEAT_INPUTS
};

//===========================================================================
// Interface Definitions
//===========================================================================

class CheatInputHandler : public Mappable
{
public:
	CheatInputHandler();
    virtual ~CheatInputHandler();

    void ResetTriggerStates()
    {
        m_LTriggerBitMask = 0;
        m_RTriggerBitMask = 0;

        for( unsigned int i = 0; i < NUM_AUXILIARY_CHEAT_INPUTS; i++ )
        {
            m_prevPhysicalButtonState[ i ] = false;
        }
    }

    void ResetInputSequence();

    // Handle a physical controller button directly (bypassing Mappable remapping).
    // cheatInputId should be a CHEAT_INPUT_* or CHEAT_INPUT_LTRIGGER/RTRIGGER value.
    void HandlePhysicalButton( int controllerId, int cheatInputId, float value );

    static const char* GetInputName( eCheatInput cheatInput );

    // Implements Mappable Interface
    //
    virtual void OnButton( int controllerId, int buttonId, const IButton* pButton );
	virtual void OnButtonUp( int controllerId, int buttonId, const IButton* pButton );
	virtual void OnButtonDown( int controllerId, int buttonId, const IButton* pButton );
    virtual void LoadControllerMappings( unsigned int controllerId );

private:
    //---------------------------------------------------------------------
    // Private Functions
    //---------------------------------------------------------------------

    // No copying or assignment. Declare but don't define.
    //
    CheatInputHandler( const CheatInputHandler& );
    CheatInputHandler& operator= ( const CheatInputHandler& );

    //---------------------------------------------------------------------
    // Private Data
    //---------------------------------------------------------------------

    unsigned int m_LTriggerBitMask;
    unsigned int m_RTriggerBitMask;

    eCheatInput m_inputSequence[ NUM_CHEAT_SEQUENCE_INPUTS ];
    unsigned int m_currentInputIndex;

    // Tracks previous pressed state for physical buttons handled via HandlePhysicalButton
    bool m_prevPhysicalButtonState[ NUM_AUXILIARY_CHEAT_INPUTS ];

};

#endif // CHEATINPUTHANDLER_H

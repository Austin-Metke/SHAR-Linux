/******************************************************************************
    File:       GuiScreenControllerLinux.cpp
    Desc:       Linux controller configuration screen with keyboard/gamepad
                remapping. Adapted from the Win32 version (GuiScreenControllerWin32.cpp).
*****************************************************************************/
#include <presentation/gui/frontend/guiscreencontrollerLinux.h>
#include <presentation/gui/guiscreenprompt.h>
#include <presentation/gui/guimanager.h>
#include <input/inputmanager.h>
#include <memory/srrmemory.h>
#include <data/config/gameconfigmanager.h>
#include <presentation/tutorialmanager.h>
#include <events/eventmanager.h>

#include <Group.h>
#include <Layer.h>
#include <Page.h>
#include <Screen.h>

//===========================================================================
// Global Data, Local Data, Local Classes
//===========================================================================

const char* szMainControllerPage = "ControllerPC";
const char* szCCPage             = "CharacterControls";
const char* szVCPage             = "VehicleControls";
const char* szGSPage             = "GameSettings";
const char* szNotAssigned        = "---";

const int MAX_INPUTNAME_LENGTH = 10;

const tColour DEFAULT_SELECTED_ITEM_COLOUR( 255, 255, 0 );
const tColour DEFAULT_INPUTMAPPED_ITEM_COLOUR( 255, 0, 0 );

/******************************************************************************
    Construction/Destruction
*****************************************************************************/
CGuiScreenController::CGuiScreenController( Scrooby::Screen* pScreen, CGuiEntity* pParent )
:   CGuiScreen( pScreen, pParent, GUI_SCREEN_ID_CONTROLLER ),
    m_pMenu( NULL ),
    m_currentPage( MENU_PAGE_MAIN ),
    m_currentControllerID( 0 ),
    m_bMapInput( false ),
    m_currentTextLabel( NULL ),
    m_bDisableBack( false ),
    m_numControllerGroups( 0 )
{
    memset( m_menuGroupStartIndex, 0, sizeof( m_menuGroupStartIndex ) );
    m_pMenu = new(GMA_LEVEL_FE) CGuiMenu( this, NUM_MENU_ITEMS );
    rAssert( m_pMenu != NULL );

    // Add main menu items
    SetGroups( m_pMenuLabels, NUM_MAINMENU_LABELS, szMainControllerPage, "Menu", "Label", ALL_ATTRIBUTES_ON );
    m_numControllerGroups = 1;

    // Add character control map items (primary and secondary columns)
    SetGroups( m_pCCLabels[MAP_PRIMARY], NUM_CHARACTERCONTROL_LABELS, szCCPage, "Map1", "Map1Label" );
    SetGroups( m_pCCLabels[MAP_SECONDARY], NUM_CHARACTERCONTROL_LABELS, szCCPage, "Map2", "Map2Label" );

    // Add vehicle control map items (primary and secondary columns)
    SetGroups( m_pVCLabels[MAP_PRIMARY], NUM_VEHICLECONTROL_LABELS, szVCPage, "Map1", "Map1Label" );
    SetGroups( m_pVCLabels[MAP_SECONDARY], NUM_VEHICLECONTROL_LABELS, szVCPage, "Map2", "Map2Label" );

    // Add game setting items (just vibration on Linux)
    SetGroups( m_pMSLabels, NUM_GAMESETTING_LABELS, szGSPage, "Menu",
               "Label", (SELECTION_ENABLED | VALUES_WRAPPED | TEXT_OUTLINE_ENABLED) );

    // Hide unused game settings items (mouse/wheel stuff from the PC page)
    {
        Scrooby::Page* pPage = m_pScroobyScreen->GetPage( szGSPage );
        if( pPage != NULL )
        {
            Scrooby::Group* group = pPage->GetGroup( "Menu" );
            if( group != NULL )
            {
                // Hide items after the first one (Label01 through Label06 are mouse/wheel settings)
                for( int i = NUM_GAMESETTING_LABELS; i < 7; i++ )
                {
                    char name[32];
                    sprintf( name, "Label%02d", i );
                    Scrooby::Text* pText = group->GetText( name );
                    if( pText != NULL )
                        pText->SetVisible( false );

                    // Also hide associated value/arrow/slider elements
                    sprintf( name, "Label%02d_Value", i );
                    Scrooby::Text* pValue = group->GetText( name );
                    if( pValue != NULL )
                        pValue->SetVisible( false );

                    sprintf( name, "Label%02d_ArrowL", i );
                    Scrooby::Sprite* pL = group->GetSprite( name );
                    if( pL != NULL )
                        pL->SetVisible( false );

                    sprintf( name, "Label%02d_ArrowR", i );
                    Scrooby::Sprite* pR = group->GetSprite( name );
                    if( pR != NULL )
                        pR->SetVisible( false );

                    sprintf( name, "Label%02d_Slider", i );
                    Scrooby::Sprite* pS = group->GetSprite( name );
                    if( pS != NULL )
                        pS->SetVisible( false );
                }
            }
        }
    }

    // Turn off the console controller page and show the PC one
    SetPageVisiblility( "Controller", false );
    SetPageVisiblility( szMainControllerPage, true );
    SetPageVisiblility( szCCPage, false );
    SetPageVisiblility( szVCPage, false );
    SetPageVisiblility( szGSPage, false );
}

CGuiScreenController::~CGuiScreenController()
{
    if( m_pMenu != NULL )
    {
        delete m_pMenu;
        m_pMenu = NULL;
    }
}

//===========================================================================
// CGuiScreenController::HandleMessage
//===========================================================================
void CGuiScreenController::HandleMessage
(
    eGuiMessage message,
    unsigned int param1,
    unsigned int param2
)
{
    static bool bRelayMessage = true;

    if( message == GUI_MSG_MENU_PROMPT_RESPONSE )
    {
        switch( param1 )
        {
        case PROMPT_CONFIRM_RESTOREALLDEFAULTS:
            {
                switch( param2 )
                {
                case (CGuiMenuPrompt::RESPONSE_YES):
                    {
                        GetInputManager()->GetController( m_currentControllerID )->LoadDefaults();
                        this->InitIntro();
                        this->ReloadScreen();
                        break;
                    }
                case (CGuiMenuPrompt::RESPONSE_NO):
                    {
                        this->ReloadScreen();
                        break;
                    }
                default:
                    {
                        rAssertMsg( 0, "WARNING: *** Invalid prompt response!\n" );
                        break;
                    }
                }
            }
            break;
        default: break;
        }
    }

    if( m_state == GUI_WINDOW_STATE_RUNNING )
    {
        switch( message )
        {
        case GUI_MSG_MENU_SELECTION_VALUE_CHANGED:
            {
                rAssert( m_pMenu );

                if( m_currentPage == MENU_PAGE_GAMESETTINGS )
                {
                    int menuItem = param1 - m_menuGroupStartIndex[MENU_PAGE_GAMESETTINGS];
                    if( menuItem == 0 ) // GS_VIBRATION
                    {
                        bool vibOn = ( m_pMenu->GetSelectionValue( param1 ) == 1 );
                        GetInputManager()->SetRumbleEnabled( vibOn );
                    }
                }
                break;
            }
        case GUI_MSG_CONTROLLER_UP:
            {
                switch( m_currentPage )
                {
                case MENU_PAGE_MAIN:
                    if( m_pMenu->GetSelection() == m_menuGroupStartIndex[m_currentPage] )
                    {
                        m_pMenu->Reset( m_menuGroupStartIndex[m_currentPage] + (NUM_MAINMENU_LABELS - 1) );
                        GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                        return;
                    }
                    break;
                case MENU_PAGE_CHARACTERCONTROLS:
                    {
                        int menuItem = m_pMenu->GetSelection();
                        if( menuItem == m_menuGroupStartIndex[m_currentPage] )
                        {
                            m_pMenu->Reset( m_menuGroupStartIndex[m_currentPage] + (NUM_CHARACTERCONTROL_LABELS - 1) );
                            GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                            return;
                        }
                        else if( menuItem == m_menuGroupStartIndex[m_currentPage] + NUM_CHARACTERCONTROL_LABELS )
                        {
                            m_pMenu->Reset( m_menuGroupStartIndex[m_currentPage] + (NUM_CHARACTERCONTROL_LABELS * 2) - 1 );
                            GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                            return;
                        }
                    }
                    break;
                case MENU_PAGE_VEHICLECONTROLS:
                    {
                        int menuItem = m_pMenu->GetSelection();
                        if( menuItem == m_menuGroupStartIndex[m_currentPage] )
                        {
                            m_pMenu->Reset( m_menuGroupStartIndex[m_currentPage] + (NUM_VEHICLECONTROL_LABELS - 1) );
                            GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                            return;
                        }
                        else if( menuItem == m_menuGroupStartIndex[m_currentPage] + NUM_VEHICLECONTROL_LABELS )
                        {
                            m_pMenu->Reset( m_menuGroupStartIndex[m_currentPage] + (NUM_VEHICLECONTROL_LABELS * 2) - 1 );
                            GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                            return;
                        }
                    }
                    break;
                default:
                    break;
                }
                break;
            }
        case GUI_MSG_CONTROLLER_DOWN:
            {
                switch( m_currentPage )
                {
                case MENU_PAGE_MAIN:
                    if( m_pMenu->GetSelection() == m_menuGroupStartIndex[m_currentPage] + (NUM_MAINMENU_LABELS - 1) )
                    {
                        m_pMenu->Reset( m_menuGroupStartIndex[m_currentPage] );
                        GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                        return;
                    }
                    break;
                case MENU_PAGE_CHARACTERCONTROLS:
                    {
                        int menuItem = m_pMenu->GetSelection();
                        if( menuItem == m_menuGroupStartIndex[m_currentPage] + (NUM_CHARACTERCONTROL_LABELS - 1) )
                        {
                            m_pMenu->Reset( m_menuGroupStartIndex[m_currentPage] );
                            GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                            return;
                        }
                        else if( menuItem == m_menuGroupStartIndex[m_currentPage] + (NUM_CHARACTERCONTROL_LABELS * 2) - 1 )
                        {
                            m_pMenu->Reset( m_menuGroupStartIndex[m_currentPage] + NUM_CHARACTERCONTROL_LABELS );
                            GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                            return;
                        }
                    }
                    break;
                case MENU_PAGE_VEHICLECONTROLS:
                    {
                        int menuItem = m_pMenu->GetSelection();
                        if( menuItem == m_menuGroupStartIndex[m_currentPage] + (NUM_VEHICLECONTROL_LABELS - 1) )
                        {
                            m_pMenu->Reset( m_menuGroupStartIndex[m_currentPage] );
                            GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                            return;
                        }
                        else if( menuItem == m_menuGroupStartIndex[m_currentPage] + (NUM_VEHICLECONTROL_LABELS * 2) - 1 )
                        {
                            m_pMenu->Reset( m_menuGroupStartIndex[m_currentPage] + NUM_VEHICLECONTROL_LABELS );
                            GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                            return;
                        }
                    }
                    break;
                default:
                    break;
                }
                break;
            }
        case GUI_MSG_CONTROLLER_LEFT:
        case GUI_MSG_CONTROLLER_RIGHT:
            {
                switch( m_currentPage )
                {
                case MENU_PAGE_CHARACTERCONTROLS:
                    {
                        int menuItem = m_pMenu->GetSelection();
                        if( menuItem < m_menuGroupStartIndex[m_currentPage] + NUM_CHARACTERCONTROL_LABELS )
                        {
                            m_pMenu->Reset( menuItem + NUM_CHARACTERCONTROL_LABELS );
                        }
                        else
                        {
                            m_pMenu->Reset( menuItem - NUM_CHARACTERCONTROL_LABELS );
                        }
                        GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                        return;
                    }
                case MENU_PAGE_VEHICLECONTROLS:
                    {
                        int menuItem = m_pMenu->GetSelection();
                        if( menuItem < m_menuGroupStartIndex[m_currentPage] + NUM_VEHICLECONTROL_LABELS )
                        {
                            m_pMenu->Reset( menuItem + NUM_VEHICLECONTROL_LABELS );
                        }
                        else
                        {
                            m_pMenu->Reset( menuItem - NUM_VEHICLECONTROL_LABELS );
                        }
                        GetEventManager()->TriggerEvent( EVENT_FE_MENU_UPORDOWN );
                        return;
                    }
                default:
                    break;
                }
                break;
            }
        case GUI_MSG_MENU_SELECTION_MADE:
            {
                switch( m_currentPage )
                {
                case MENU_PAGE_MAIN:
                    {
                        switch( param1 )
                        {
                        case MENU_ITEM_CHARACTERCONTROLS:
                            {
                                SetPageVisiblility( szMainControllerPage, false );
                                SetPageVisiblility( szCCPage, true );
                                m_currentPage = MENU_PAGE_CHARACTERCONTROLS;
                                break;
                            }
                        case MENU_ITEM_VEHICLECONTROLS:
                            {
                                SetPageVisiblility( szMainControllerPage, false );
                                SetPageVisiblility( szVCPage, true );
                                m_currentPage = MENU_PAGE_VEHICLECONTROLS;
                                break;
                            }
                        case MENU_ITEM_GAMESETTINGS:
                            {
                                SetPageVisiblility( szMainControllerPage, false );
                                SetPageVisiblility( szGSPage, true );
                                m_currentPage = MENU_PAGE_GAMESETTINGS;
                                break;
                            }
                        case MENU_ITEM_RESTOREALLDEFAULTS:
                            {
                                rAssert( m_guiManager );
                                m_guiManager->DisplayPrompt( PROMPT_CONFIRM_RESTOREALLDEFAULTS, this );
                                break;
                            }
                        default:
                            break;
                        }
                    } break;
                case MENU_PAGE_CHARACTERCONTROLS:
                    {
                        if( param1 >= (unsigned int)m_menuGroupStartIndex[m_currentPage] )
                        {
                            int menuItem = param1;
                            if( menuItem < m_menuGroupStartIndex[m_currentPage] + NUM_CHARACTERCONTROL_LABELS )
                            {
                                menuItem = menuItem - m_menuGroupStartIndex[m_currentPage];
                                RemapButton( MAP_PRIMARY, menuItem );
                            }
                            else
                            {
                                menuItem = menuItem - m_menuGroupStartIndex[m_currentPage] - NUM_CHARACTERCONTROL_LABELS;
                                RemapButton( MAP_SECONDARY, menuItem );
                            }
                        }
                    } break;
                case MENU_PAGE_VEHICLECONTROLS:
                    {
                        if( param1 >= (unsigned int)m_menuGroupStartIndex[m_currentPage] )
                        {
                            int menuItem = param1;
                            if( menuItem < m_menuGroupStartIndex[m_currentPage] + NUM_VEHICLECONTROL_LABELS )
                            {
                                menuItem = menuItem - m_menuGroupStartIndex[m_currentPage];
                                RemapButton( MAP_PRIMARY, menuItem );
                            }
                            else
                            {
                                menuItem = menuItem - m_menuGroupStartIndex[m_currentPage] - NUM_VEHICLECONTROL_LABELS;
                                RemapButton( MAP_SECONDARY, menuItem );
                            }
                        }
                    } break;
                default: break;
                }
                break;
            }
        case GUI_MSG_CONTROLLER_BACK:
            {
                if( !m_bDisableBack )
                {
                    switch( m_currentPage )
                    {
                    case MENU_PAGE_CHARACTERCONTROLS:
                        SetPageVisiblility( szCCPage, false );
                        SetPageVisiblility( szMainControllerPage, true );
                        m_currentPage = MENU_PAGE_MAIN;
                        bRelayMessage = false;
                        break;
                    case MENU_PAGE_VEHICLECONTROLS:
                        SetPageVisiblility( szVCPage, false );
                        SetPageVisiblility( szMainControllerPage, true );
                        m_currentPage = MENU_PAGE_MAIN;
                        bRelayMessage = false;
                        break;
                    case MENU_PAGE_GAMESETTINGS:
                        SetPageVisiblility( szGSPage, false );
                        SetPageVisiblility( szMainControllerPage, true );
                        m_currentPage = MENU_PAGE_MAIN;
                        bRelayMessage = false;
                        break;
                    default:
                        this->StartTransitionAnimation( 560, 590 );
                        bRelayMessage = true;
                    }
                }
                else
                {
                    m_bDisableBack = false;
                    bRelayMessage = false;
                }

                break;
            }

        default:
            break;
        }

        // relay message to menu
        if( m_pMenu != NULL && !m_bMapInput )
        {
            m_pMenu->HandleMessage( message, param1, param2 );
        }
    }

    // Propagate the message up the hierarchy.
    if( bRelayMessage )
        CGuiScreen::HandleMessage( message, param1, param2 );
    else
        bRelayMessage = true;
}

void CGuiScreenController::RemapButton( eControllerColumn column, int menuItem )
{
    switch( m_currentPage )
    {
    case MENU_PAGE_CHARACTERCONTROLS:
        m_currentTextLabel = m_pCCLabels[column][menuItem];
        break;
    case MENU_PAGE_VEHICLECONTROLS:
        m_currentTextLabel = m_pVCLabels[column][menuItem];
        break;
    default:
        break;
    }
    if( m_currentTextLabel )
        m_currentTextLabel->SetColour( DEFAULT_INPUTMAPPED_ITEM_COLOUR );

    GetInputManager()->GetController( m_currentControllerID )->RemapButton(
        column,
        GetVirtualKey( m_currentPage, menuItem ),
        this );
    m_bMapInput = true;
}

//===========================================================================
// CGuiScreenController::InitIntro
//===========================================================================
void CGuiScreenController::InitIntro()
{
    InitPageLabels( MENU_PAGE_CHARACTERCONTROLS );
    InitPageLabels( MENU_PAGE_VEHICLECONTROLS );

    // Initialize game settings (vibration toggle)
    rAssert( m_pMenu );
    int gsStart = m_menuGroupStartIndex[MENU_PAGE_GAMESETTINGS];
    m_pMenu->SetSelectionValue( gsStart, GetInputManager()->IsRumbleEnabled() ? 1 : 0 );
}

//===========================================================================
// CGuiScreenController::InitRunning
//===========================================================================
void CGuiScreenController::InitRunning()
{
}

//===========================================================================
// CGuiScreenController::InitOutro
//===========================================================================
void CGuiScreenController::InitOutro()
{
    // Save the new controller mappings to the config file.
    GetGameConfigManager()->SaveConfigFile();
}

//===========================================================================
// CGuiScreenController::OnButtonMapped
//===========================================================================
void CGuiScreenController::OnButtonMapped( const char* szNewInput, eControllerType cont, int num_dirs, eDirectionType direction )
{
    char szText[255];
    memset( szText, 0, sizeof( szText ) );

    if( !szNewInput )
    {
        // User cancelled the mapper event (pressed back/escape).
        m_bDisableBack = true;
    }
    else
    {
        if( m_currentTextLabel )
            m_currentTextLabel->SetColour( DEFAULT_SELECTED_ITEM_COLOUR );

        strcpy( szText, szNewInput );
        GetAppropriateInputName( szText, cont, direction, num_dirs );

        UpdatePageLabels( m_currentPage, szText );

        // Disable tutorials if controls are changed.
        GetInputManager()->GetController( 0 )->SetTutorialDisabled( true );
        GetTutorialManager()->EnableTutorialMode( false );
    }

    m_bMapInput = false;
    m_currentTextLabel = NULL;
}

//---------------------------------------------------------------------
// Private Functions
//---------------------------------------------------------------------
void CGuiScreenController::UpdatePageLabels( eMenuPages page, const char* szNewInput )
{
    switch( page )
    {
    case MENU_PAGE_CHARACTERCONTROLS:
        if( szNewInput )
        {
            for( int mapCount = 0; mapCount < NUM_COLUMNS; mapCount++ )
            {
                for( int itemCount = 0; itemCount < NUM_CHARACTERCONTROL_LABELS; itemCount++ )
                {
                    UnicodeString uniStrTemp = m_pCCLabels[mapCount][itemCount]->GetString();
                    char tempStr[255];
                    memset( tempStr, 0, sizeof( tempStr ) );
                    uniStrTemp.MakeAscii( tempStr, uniStrTemp.Length() + 1 );
                    if( strcmp( szNewInput, tempStr ) == 0 )
                    {
                        m_pCCLabels[mapCount][itemCount]->SetString( 0, szNotAssigned );
                        break;
                    }
                }
            }
        }
        break;
    case MENU_PAGE_VEHICLECONTROLS:
        if( szNewInput )
        {
            for( int mapCount = 0; mapCount < NUM_COLUMNS; mapCount++ )
            {
                for( int itemCount = 0; itemCount < NUM_VEHICLECONTROL_LABELS; itemCount++ )
                {
                    UnicodeString uniStrTemp = m_pVCLabels[mapCount][itemCount]->GetString();
                    char tempStr[255];
                    memset( tempStr, 0, sizeof( tempStr ) );
                    uniStrTemp.MakeAscii( tempStr, uniStrTemp.Length() + 1 );
                    if( strcmp( szNewInput, tempStr ) == 0 )
                    {
                        m_pVCLabels[mapCount][itemCount]->SetString( 0, szNotAssigned );
                        break;
                    }
                }
            }
        }
        break;
    default:
        break;
    }
    if( m_currentTextLabel )
        m_currentTextLabel->SetString( 0, szNewInput );
}

void CGuiScreenController::InitPageLabels( eMenuPages page )
{
    UserController* pController = GetInputManager()->GetController( m_currentControllerID );

    switch( page )
    {
    case MENU_PAGE_CHARACTERCONTROLS:
        {
            for( int mapCount = 0; mapCount < NUM_COLUMNS; mapCount++ )
            {
                eControllerType controllerType;
                int ndirections;
                eDirectionType dir;

                for( int i = 0; i < NUM_CONTROLLERTYPES; i++ )
                {
                    for( int ccItem = 0; ccItem < NUM_CHARACTERCONTROL_LABELS; ccItem++ )
                    {
                        const char* szName = pController->GetMap(
                            mapCount,
                            GetVirtualKey( MENU_PAGE_CHARACTERCONTROLS, ccItem ),
                            ndirections, controllerType, dir );
                        if( szName )
                        {
                            char szText[255];
                            memset( szText, 0, sizeof( szText ) );
                            strcpy( szText, szName );
                            GetAppropriateInputName( szText, controllerType, dir, ndirections );
                            m_pCCLabels[mapCount][ccItem]->SetString( 0, szText );
                        }
                        else
                        {
                            m_pCCLabels[mapCount][ccItem]->SetString( 0, szNotAssigned );
                        }
                    }
                }
            }
        } break;
    case MENU_PAGE_VEHICLECONTROLS:
        {
            for( int mapCount = 0; mapCount < NUM_COLUMNS; mapCount++ )
            {
                eControllerType controllerType;
                int ndirections;
                eDirectionType dir;

                for( int i = 0; i < NUM_CONTROLLERTYPES; i++ )
                {
                    for( int ccItem = 0; ccItem < NUM_VEHICLECONTROL_LABELS; ccItem++ )
                    {
                        const char* szName = pController->GetMap(
                            mapCount,
                            GetVirtualKey( MENU_PAGE_VEHICLECONTROLS, ccItem ),
                            ndirections, controllerType, dir );
                        if( szName )
                        {
                            char szText[255];
                            memset( szText, 0, sizeof( szText ) );
                            strcpy( szText, szName );
                            GetAppropriateInputName( szText, controllerType, dir, ndirections );
                            m_pVCLabels[mapCount][ccItem]->SetString( 0, szText );
                        }
                        else
                        {
                            m_pVCLabels[mapCount][ccItem]->SetString( 0, szNotAssigned );
                        }
                    }
                }
            }
        } break;
    default:
        break;
    }
}

void CGuiScreenController::SetGroups( Scrooby::Text** pLabels,
                                      int numMenuItems,
                                      const char* strPage,
                                      const char* strGroup,
                                      const char* szLabel,
                                      int attributes )
{
    Scrooby::Page* pPage = m_pScroobyScreen->GetPage( strPage );
    rAssert( pPage );

    // Track the start index of each controller page's menu items.
    if( m_numControllerGroups < NUM_MENU_PAGES )
    {
        if( strcmp( strGroup, "Map1" ) == 0 || strcmp( strPage, szGSPage ) == 0 )
        {
            m_menuGroupStartIndex[m_numControllerGroups++] = m_pMenu->GetNumItems();
        }
    }

    Scrooby::Group* pMenuGroup = pPage->GetGroup( strGroup );
    for( int itemCount = 0; itemCount < numMenuItems; itemCount++ )
    {
        char objectName[32];
        sprintf( objectName, "%s%02d", szLabel, itemCount );
        Scrooby::Text* pMenuItemText = pMenuGroup->GetText( objectName );
        if( pMenuItemText != NULL )
        {
            pMenuItemText->SetTextMode( Scrooby::TEXT_WRAP );

            char itemName[32];
            sprintf( itemName, "%s%02d_Value", szLabel, itemCount );
            Scrooby::Text* pTextValue = pMenuGroup->GetText( itemName );

            sprintf( itemName, "%s%02d_ArrowL", szLabel, itemCount );
            Scrooby::Sprite* pLArrow = pMenuGroup->GetSprite( itemName );

            sprintf( itemName, "%s%02d_ArrowR", szLabel, itemCount );
            Scrooby::Sprite* pRArrow = pMenuGroup->GetSprite( itemName );

            sprintf( itemName, "%s%02d_Slider", szLabel, itemCount );
            Scrooby::Sprite* pSlider = pMenuGroup->GetSprite( itemName );

            m_pMenu->AddMenuItem( pMenuItemText,
                                  pTextValue,
                                  NULL,
                                  pSlider,
                                  pLArrow,
                                  pRArrow,
                                  attributes );
            pLabels[itemCount] = pMenuItemText;
        }
        else
        {
            break;
        }
    }
}

void CGuiScreenController::SetPageVisiblility( const char* strPage, bool bVisible )
{
    Scrooby::Page* pPage = m_pScroobyScreen->GetPage( strPage );
    if( pPage == NULL )
        return;

    pPage->GetLayerByIndex( 0 )->SetVisible( bVisible );

    if( strcmp( strPage, szMainControllerPage ) == 0 )
    {
        for( int i = 0; i < NUM_MAINMENU_LABELS; i++ )
            m_pMenuLabels[i]->SetVisible( bVisible );
        if( bVisible )
            m_pMenu->Reset( MENU_ITEM_CHARACTERCONTROLS );
    }
    else if( strcmp( strPage, szCCPage ) == 0 )
    {
        for( int columnCount = 0; columnCount < NUM_COLUMNS; columnCount++ )
            for( int labelCount = 0; labelCount < NUM_CHARACTERCONTROL_LABELS; labelCount++ )
                m_pCCLabels[columnCount][labelCount]->SetVisible( bVisible );
        if( bVisible )
            m_pMenu->Reset( m_menuGroupStartIndex[MENU_PAGE_CHARACTERCONTROLS] );
    }
    else if( strcmp( strPage, szVCPage ) == 0 )
    {
        for( int columnCount = 0; columnCount < NUM_COLUMNS; columnCount++ )
            for( int labelCount = 0; labelCount < NUM_VEHICLECONTROL_LABELS; labelCount++ )
                m_pVCLabels[columnCount][labelCount]->SetVisible( bVisible );
        if( bVisible )
            m_pMenu->Reset( m_menuGroupStartIndex[MENU_PAGE_VEHICLECONTROLS] );
    }
    else if( strcmp( strPage, szGSPage ) == 0 )
    {
        for( int i = 0; i < NUM_GAMESETTING_LABELS; i++ )
            m_pMSLabels[i]->SetVisible( bVisible );
        if( bVisible )
            m_pMenu->Reset( m_menuGroupStartIndex[MENU_PAGE_GAMESETTINGS] );
    }
}

int CGuiScreenController::GetVirtualKey( eMenuPages page, int menuItem )
{
    switch( page )
    {
    case MENU_PAGE_CHARACTERCONTROLS:
        menuItem += CC_MOVEUP;
        break;
    case MENU_PAGE_VEHICLECONTROLS:
        menuItem += VC_ACCELERATE;
        break;
    default:
        break;
    }

    switch( menuItem )
    {
    case CC_MOVEUP:        return InputManager::MoveUp;
    case CC_MOVEDOWN:      return InputManager::MoveDown;
    case CC_MOVELEFT:      return InputManager::MoveLeft;
    case CC_MOVERIGHT:     return InputManager::MoveRight;
    case CC_ATTACK:        return InputManager::Attack;
    case CC_JUMP:          return InputManager::Jump;
    case CC_SPRINT:        return InputManager::Sprint;
    case CC_ACTION:        return InputManager::DoAction;
    case CC_CAMERALEFT:    return InputManager::CameraLeft;
    case CC_CAMERARIGHT:   return InputManager::CameraRight;
    case CC_CAMERAMOVEIN:  return InputManager::CameraMoveIn;
    case CC_CAMERAMOVEOUT: return InputManager::CameraMoveOut;
    case CC_LOOKUP:        return InputManager::CameraLookUp;
    case CC_ZOOM:          return InputManager::CameraZoom;
    case VC_ACCELERATE:    return InputManager::Accelerate;
    case VC_REVERSE:       return InputManager::Reverse;
    case VC_STEERLEFT:     return InputManager::SteerLeft;
    case VC_STEERRIGHT:    return InputManager::SteerRight;
    case VC_EBRAKE:        return InputManager::HandBrake;
    case VC_ACTION:        return InputManager::GetOutCar;
    case VC_HORN:          return InputManager::Horn;
    case VC_RESET:         return InputManager::ResetCar;
    case VC_LOOKLEFT:      return InputManager::CameraCarLeft;
    case VC_LOOKRIGHT:     return InputManager::CameraCarRight;
    case VC_LOOKUP:        return InputManager::CameraCarLookUp;
    case VC_LOOKBACK:      return InputManager::CameraCarLookBack;
    case VC_CHANGECAMERA:  return InputManager::CameraToggle;
    default:               return 0;
    }
}

void CGuiScreenController::GetAppropriateInputName( char* szInputName,
                                                    eControllerType controllerType,
                                                    eDirectionType direction,
                                                    int numDirections )
{
    char szText[255];
    memset( szText, 0, sizeof( szText ) );

    if( szInputName )
    {
        // Abbreviate long key names
        if( controllerType == KEYBOARD )
        {
            if( strcmp( szInputName, "Right Shift" ) == 0 )
                strcpy( szInputName, "R. Shift" );
            else if( strcmp( szInputName, "Left Shift" ) == 0 )
                strcpy( szInputName, "L. Shift" );
            else if( strcmp( szInputName, "Right Ctrl" ) == 0 )
                strcpy( szInputName, "R. Ctrl" );
            else if( strcmp( szInputName, "Left Ctrl" ) == 0 )
                strcpy( szInputName, "L. Ctrl" );
        }

        // Truncate to max allowed length
        if( strlen( szInputName ) > MAX_INPUTNAME_LENGTH )
        {
            szInputName[MAX_INPUTNAME_LENGTH - 1] = '\0';
        }

        // For gamepad axes, append direction indicator
        if( controllerType == GAMEPAD )
        {
            if( numDirections == 2 )
            {
                switch( direction )
                {
                case DIR_UP:
                    strcat( szInputName, " +" );
                    break;
                case DIR_DOWN:
                    strcat( szInputName, " -" );
                    break;
                default:
                    break;
                }
            }
            sprintf( szText, "J %s", szInputName );
            strcpy( szInputName, szText );
        }
    }
}

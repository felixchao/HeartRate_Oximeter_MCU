/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_oled.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "app_oled.h"
#include "Microchip_Logo.h"
#include "CString.h"
#include "GraphicLib.h"

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_OLED_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/
APP_OLED_DATA app_oledData;

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define LCM_ROW               8    // LCM Text Row count
#define LCM_COL               18   // LCM Text Column count

int8_t LogoX, LogoY;
bool UI_Language = UI_CHINESE; // 0: English, 1:Chinese (BT1 to toogle)

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
APP_OLED_LANG_ID APP_OLED_Get_Language( void )
{
    return UI_Language;
}

void APP_OLED_FreeFall_Alert( bool display )
{
    char OutStr[20];

    if( display )
    {
        GPL_LayerClean( LAYER_ALERT );
        GPL_LayerSet( LAYER_ALERT );
        if( APP_OLED_Get_Language() == UI_CHINESE )
        {
            GPL_DrawBitmap(8, 24, 112, 16, BG_SOLID, CString7);
        }
        else
        {
            sprintf( OutStr, "Free Fall detected" );
            GPL_DrawString((LCM_WIDTH-(strlen(OutStr)*FONT_WIDTH))/2, (LCM_HEIGHT-FONT_WIDTH)/2, OutStr, BG_SOLID, TEXT_HIGHLIGHT);
        }
        GPL_LayerShow( LAYER_ALERT, GPL_SHOW );
        GPL_LayerShow( LAYER_STRING, GPL_HIDE );
        GPL_LayerShow( LAYER_GRAPHIC, GPL_HIDE );
    }
    else
    {
        GPL_LayerClean( LAYER_ALERT );
        GPL_LayerShow( LAYER_ALERT, GPL_HIDE );
        GPL_LayerShow( LAYER_STRING, GPL_SHOW );
        GPL_LayerShow( LAYER_GRAPHIC, GPL_SHOW );
    }
}

void APP_OLED_Oximeter_Finger( uint32_t aun_red )
{
    uint16_t Radius, cx, cy;
    char OutStr[20];

    GPL_LayerClean( LAYER_GRAPHIC );
    GPL_LayerSet( LAYER_GRAPHIC );
    GPL_SetPenSize( 1 );
    cx = LCM_WIDTH/2;
    cy = (LCM_HEIGHT+FONT_HEIGHT_REAL)/2;
    if( aun_red!=0 )
    {
        Radius = (aun_red*(LCM_HEIGHT/2))/90000;
        if( Radius > LCM_HEIGHT-cy-2 )
            Radius = LCM_HEIGHT-cy-2;
        GPL_DrawCircle( cx, cy, Radius);
        sprintf( OutStr, "%ld", aun_red );
        GPL_DrawString(cx-((FONT_WIDTH*strlen(OutStr))/2), cy-(FONT_HEIGHT_REAL/2), OutStr, BG_SOLID, TEXT_NORMAL);
    }
}

void APP_OLED_Oximeter_Status( OXIMETER_FINGER_STATES status )
{
    char OutStr[20];

    GPL_LayerClean( LAYER_STRING );
    GPL_LayerSet( LAYER_STRING );
    switch( status )
    {
    case OXIMETER_FINGER_DETECT:
        if( UI_Language == UI_CHINESE )
            GPL_DrawBitmap((LCM_WIDTH-78)/2, 0, 78, 16, BG_SOLID, CString1);
        else
            sprintf( OutStr, "Put your Finger on" );
        break;
    case OXIMETER_FINGER_WAIT_ATTACH:
        if( UI_Language == UI_CHINESE )
            GPL_DrawBitmap((LCM_WIDTH-78)/2, 0, 78, 16, BG_SOLID, CString2);
        else
            sprintf( OutStr, "Move Finger close" );
        break;
    case OXIMETER_FINGER_CHECKING:
        if( UI_Language == UI_CHINESE )
            GPL_DrawBitmap((LCM_WIDTH-78)/2, 0, 78, 16, BG_SOLID, CString3);
        else
            sprintf( OutStr, "Calculating..." );
        break;
    }

    if( UI_Language == UI_ENGLISH )
        GPL_DrawString((LCM_WIDTH-(strlen(OutStr)*FONT_WIDTH))/2, 0, OutStr, BG_SOLID, TEXT_NORMAL);
}

void APP_OLED_Oximeter_Checking( uint16_t Current, uint16_t Maximum )
{
    uint16_t Radius, cx, cy;
    char OutStr[20];

    GPL_LayerClean( LAYER_GRAPHIC );
    GPL_LayerSet( LAYER_GRAPHIC );
    GPL_SetPenSize( 1 );
    cx = LCM_WIDTH/2;
    cy = (LCM_HEIGHT+FONT_HEIGHT_REAL)/2;
    Radius = LCM_HEIGHT-cy-2;
    GPL_DrawCircle( cx, cy, Radius);
    sprintf( OutStr, "%2d%%", Current*100/Maximum );
    GPL_DrawString(cx-((FONT_WIDTH*strlen(OutStr))/2), cy-(FONT_HEIGHT_REAL/2), OutStr, BG_SOLID, TEXT_NORMAL);
}

void APP_OLED_Oximeter_Result( int16_t nHR, int8_t cHRVaild, int16_t nSPO2, int8_t cSPO2Valid )
{
    char OutStr[20];

    GPL_LayerClean( LAYER_STRING );
    GPL_LayerSet( LAYER_STRING );

    if( UI_Language == UI_CHINESE )
    {
        GPL_DrawBitmap( 0,  0, 52, 16, BG_SOLID, CString4);
        sprintf( OutStr, "%3d", nHR );
        if( cHRVaild )   GPL_DrawString(56, 0, OutStr, BG_SOLID, TEXT_NORMAL);
        else             GPL_DrawString(56, 0, OutStr, BG_SOLID, TEXT_HIGHLIGHT);
        GPL_DrawBitmap(86,  0, 42, 16, BG_SOLID, CString41);
        GPL_DrawBitmap( 0, 13, 52, 16, BG_SOLID, CString5);
        sprintf( OutStr, "%3d %%", nSPO2 );
        if( cSPO2Valid ) GPL_DrawString(56, 13, OutStr, BG_SOLID, TEXT_NORMAL);
        else             GPL_DrawString(56, 13, OutStr, BG_SOLID, TEXT_HIGHLIGHT);
        GPL_DrawBitmap( 0, 26, 52, 16, BG_SOLID, CString6);
        sprintf( OutStr, "%3.1f 'C", MCP9700_Temp );
        GPL_DrawString(63, 26, OutStr, BG_SOLID, TEXT_NORMAL);
    }
    else
    {
        sprintf( OutStr, "HR   : %3d bpm", nHR );
        if( cHRVaild )   GPL_DrawString(0, 0, OutStr, BG_SOLID, TEXT_NORMAL);
        else             GPL_DrawString(0, 0, OutStr, BG_SOLID, TEXT_HIGHLIGHT);
        sprintf( OutStr, "SpO2 : %3d %%", nSPO2 );
        if( cSPO2Valid ) GPL_DrawString(0, 13, OutStr, BG_SOLID, TEXT_NORMAL);
        else             GPL_DrawString(0, 13, OutStr, BG_SOLID, TEXT_HIGHLIGHT);
        sprintf( OutStr, "Temp :  %3.1f 'C", MCP9700_Temp );
        GPL_DrawString(0, 26, OutStr, BG_SOLID, TEXT_NORMAL);
    }
}

#define HR_WAVE_TIMESPAN    100
#define HR_WAVE_SIZE        (LCM_WIDTH)
#define HR_WAVE_HEIGHT      (LCM_HEIGHT-44)
#define HR_WAVE_START       (0)
#define HR_WAVE_ZERO        (LCM_HEIGHT-5)
void APP_OLED_Oximeter_Wave( uint32_t FIFO_min, uint32_t FIFO_max, uint32_t FIFO_data )
{
    static uint8_t RingIdx = 0;
    static uint32_t HR_Wave[HR_WAVE_SIZE];
    uint8_t CurIdx, PreIdx;

    GPL_LayerClean( LAYER_GRAPHIC );
    GPL_LayerSet( LAYER_GRAPHIC );
    GPL_SetPenSize( 1 );

    HR_Wave[RingIdx]  = (FIFO_data-FIFO_min)*HR_WAVE_HEIGHT/(FIFO_max-FIFO_min+1);
    if( HR_Wave[RingIdx]> HR_WAVE_HEIGHT ) HR_Wave[RingIdx] =  HR_WAVE_HEIGHT;

    for( int Col=0 ; Col<HR_WAVE_SIZE-1 ; Col++ )
    {
        CurIdx = (RingIdx-Col+HR_WAVE_SIZE)%HR_WAVE_SIZE;
        PreIdx = (RingIdx-1-Col+HR_WAVE_SIZE)%HR_WAVE_SIZE;
        GPL_DrawLine(Col+HR_WAVE_START, HR_WAVE_ZERO-HR_Wave[CurIdx], Col+HR_WAVE_START+1, HR_WAVE_ZERO-HR_Wave[PreIdx]);
    }

    // Move to next buffer
    RingIdx = (RingIdx+1)%HR_WAVE_SIZE;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_OLED_Initialize ( void )

  Remarks:
    See prototype in app_oled.h.
 */

void APP_OLED_Initialize ( void )
{
    app_oledData.state = APP_OLED_STATE_INIT;
}

/******************************************************************************
  Function:
    void APP_OLED_Tasks ( void )

  Remarks:
    See prototype in app_oled.h.
 */
void APP_OLED_Tasks ( void )
{
    switch ( app_oledData.state )
    {
        case APP_OLED_STATE_INIT:
        {
            // LCM initial @ LCM.c
            if( GPL_ScreenInit()==false )
            {
                app_oledData.state = APP_OLED_STATE_ERROR;
                break;
            }

            // Initial Splash Bitmap location and Layer
            LogoX = 128;
            LogoY = 0;

            // Show Graphic only
            GPL_LayerShow( LAYER_GRAPHIC, GPL_SHOW );
            GPL_LayerShow( LAYER_STRING, GPL_HIDE );

            app_oledData.state = APP_OLED_STATE_SPLASH;
            break;
        }

        case APP_OLED_STATE_SPLASH:
        {
            // OLED LCM Logo Animation start
            if( (LogoX-=4)>=0 )
            {
                // Fill LCM with LOGO
                GPL_LayerSet( LAYER_GRAPHIC );
                GPL_ScreenClean();
                GPL_DrawBitmap(LogoX, LogoY, LCM_WIDTH, LCM_HEIGHT, BG_SOLID, Microchip_Logo);
                GPL_ScreenUpdate();
            }
            else
            {
                TC4_DelayMS( SPLASH_WAIT_DELAY, DELAY_TIMER_SPLASH_WAIT );
                app_oledData.state = APP_OLED_STATE_WAIT_SPLASH_COMPLETE;
            }
            break;
        }

        case APP_OLED_STATE_WAIT_SPLASH_COMPLETE:
        {
            if( TC4_DelayIsComplete( DELAY_TIMER_SPLASH_WAIT ) )
            {
                // Screen Clean
                GPL_LayerSet( LAYER_GRAPHIC );
                GPL_ScreenClean();
                GPL_ScreenUpdate();

                TC4_DelayMS( GRPAHIC_UPDATE_DELAY, DELAY_TIMER_GRPAHIC_UPDATE );

                // Show all layer
                GPL_LayerShow( LAYER_GRAPHIC, GPL_SHOW );
                GPL_LayerShow( LAYER_STRING, GPL_SHOW );

                app_oledData.state = APP_OLED_STATE_UPDATE;
            }
            break;
        }

        case APP_OLED_STATE_UPDATE:
        {
            if( !BT1_Get() )
            {
                while(!BT1_Get());
                UI_Language = !UI_Language;
            }
            if( TC4_DelayIsComplete( DELAY_TIMER_GRPAHIC_UPDATE ) )
            {
                GPL_ScreenUpdate();

                TC4_DelayMS( GRPAHIC_UPDATE_DELAY, DELAY_TIMER_GRPAHIC_UPDATE );
            }

            break;
        }

        case APP_OLED_STATE_ERROR:
        {
            app_oledData.state = APP_OLED_STATE_IDLE;
            break;
        }

        case APP_OLED_STATE_IDLE:
        default:
        {
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */

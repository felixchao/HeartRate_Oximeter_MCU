/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_oled.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_OLED_Initialize" and "APP_OLED_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_OLED_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_OLED_H
#define _APP_OLED_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "definitions.h"
#include "app_oximeter.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
typedef enum
{
    UI_ENGLISH,
    UI_CHINESE
} APP_OLED_LANG_ID;

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APP_OLED_STATE_INIT=0,
    APP_OLED_STATE_SPLASH,
    APP_OLED_STATE_WAIT_SPLASH_COMPLETE,
    APP_OLED_STATE_UPDATE,
    APP_OLED_STATE_ERROR,
    APP_OLED_STATE_IDLE,

} APP_OLED_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    APP_OLED_STATES state;

} APP_OLED_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_OLED_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_OLED_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_OLED_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_OLED_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_OLED_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_OLED_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_OLED_Tasks( void );
void APP_OLED_FreeFall_Alert( bool display );
void APP_OLED_Oximeter_Finger( uint32_t aun_red );
void APP_OLED_Oximeter_Status( OXIMETER_FINGER_STATES status );
void APP_OLED_Oximeter_Checking( uint16_t Current, uint16_t Maximum );
void APP_OLED_Oximeter_Result( int16_t nHR, int8_t cHRVaild, int16_t nSPO2, int8_t cSPO2Valid );
void APP_OLED_Oximeter_Wave( uint32_t FIFO_min, uint32_t FIFO_max, uint32_t FIFO_data );

#endif /* _APP_OLED_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */


/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_freefall.c

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
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "app_freefall.h"
#include "app_oled.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define I2C3_CALLBACK_ENABLE       1
#define DEBUG_ENABLE               0

#define FREEFALL_GRAVITY_THRESHOLD 1000
#define KXTJ3_ADDRESS   0x0E
uint8_t KXTJ3_WriteData[2];
uint8_t KXTJ3_ReadData[1];
#ifdef I2C3_CALLBACK_ENABLE
volatile uint8_t I2C3_IsDataReady = 0;
#endif
uint8_t AxisOutReg[6] = {0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
uint8_t AxisOutByte[6];

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_FREEFALL_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/
APP_FREEFALL_DATA app_freefallData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
#if I2C3_CALLBACK_ENABLE
void I2C3_TransferCallback(uintptr_t contextHandle)
{
    if (KXTJ3_ReadData[0] & (1 << 4))
    {
        I2C3_IsDataReady = 1;
    }
}
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_FREEFALL_Initialize ( void )

  Remarks:
    See prototype in app_freefall.h.
 */

void APP_FREEFALL_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_freefallData.state = APP_FREEFALL_STATE_INIT;
    app_freefallData.DataReady = false;
}


/******************************************************************************
  Function:
    void APP_FREEFALL_Tasks ( void )

  Remarks:
    See prototype in app_freefall.h.
 */

void APP_FREEFALL_Tasks ( void )
{
    int i=0;

    /* Check the application's current state. */
    switch ( app_freefallData.state )
    {
    /* Application's initial state. */
    case APP_FREEFALL_STATE_INIT:
        KXTJ3_WriteData[0] = 0x1D;
        KXTJ3_WriteData[1] = 0x80;
        SERCOM3_I2C_Write(KXTJ3_ADDRESS, KXTJ3_WriteData, 2);
        while (SERCOM3_I2C_IsBusy());
        TC4_DelayMS(GSENSOR_READ_DELAY, DELAY_TIMER_GSENSOR_READ);
        while(!TC4_DelayIsComplete(DELAY_TIMER_GSENSOR_READ));
#if DEBUG_ENABLE
        myprintf("G-Sensor Software Reset Completed\r\n");
        KXTJ3_WriteData[0] = 0x0F;
        SERCOM3_I2C_WriteRead(KXTJ3_ADDRESS, KXTJ3_WriteData, 1, KXTJ3_ReadData, 1);
        while (SERCOM3_I2C_IsBusy());
        myprintf("G-Sensor WHO AM I : 0x%X\r\n", KXTJ3_ReadData[0]);
#endif
        KXTJ3_WriteData[0] = 0x1B;
        KXTJ3_WriteData[1] = 0xF0;
        SERCOM3_I2C_Write(KXTJ3_ADDRESS, KXTJ3_WriteData, 2);
        while (SERCOM3_I2C_IsBusy());
#if DEBUG_ENABLE
        myprintf("G-Sensor Initial Completed\r\n");
#endif
#if I2C3_CALLBACK_ENABLE
        SERCOM3_I2C_CallbackRegister(I2C3_TransferCallback, 0);
#endif
        app_freefallData.state = APP_FREEFALL_STATE_DATA_WAIT;
        break;

    case APP_FREEFALL_STATE_DATA_WAIT:
        TC4_DelayMS(GSENSOR_READ_DELAY, DELAY_TIMER_GSENSOR_READ);
        app_freefallData.state = APP_FREEFALL_STATE_DATA_REQUEST;
        break;

    case APP_FREEFALL_STATE_DATA_REQUEST:
        if (!TC4_DelayIsComplete(DELAY_TIMER_GSENSOR_READ) || SERCOM3_I2C_IsBusy())
            break;
        KXTJ3_WriteData[0] = 0x16;
        KXTJ3_ReadData[0] = 0;
        SERCOM3_I2C_WriteRead(KXTJ3_ADDRESS, KXTJ3_WriteData, 1, KXTJ3_ReadData, 1);
#if I2C3_CALLBACK_ENABLE == 0
        while (SERCOM3_I2C_IsBusy());
#endif
        app_freefallData.state = APP_FREEFALL_STATE_DATA_READ;
        break;

    case APP_FREEFALL_STATE_DATA_READ:
#if I2C3_CALLBACK_ENABLE
        if( I2C3_IsDataReady )
        {
            I2C3_IsDataReady = 0;
#else
        if (KXTJ3_ReadData[0] & (1 << 4))
        {
#endif
            LED2_Toggle();
            for (i = 0; i < 6; i++)
            {
                SERCOM3_I2C_WriteRead(KXTJ3_ADDRESS, &AxisOutReg[i], 1, &AxisOutByte[i], 1);
                while (SERCOM3_I2C_IsBusy());
            }
#if DEBUG_ENABLE
            myprintf("\033[4;1HG-Sensor = (%5d, %5d, %5d)\r\n", (short) (AxisOutByte[1] << 8 | AxisOutByte[0]), (short) (AxisOutByte[3] << 8 | AxisOutByte[2]), (short) (AxisOutByte[5] << 8 | AxisOutByte[4]));
#endif
            app_freefallData.state = APP_FREEFALL_STATE_FREEFALL_CHECK;
        }
        break;

    case APP_FREEFALL_STATE_FREEFALL_CHECK:
        {
            // Challenge 1
            // Please complete the Freefall algorithm in below state.
            // The Freefall Gravity Threshold could use the define FREEFALL_GRAVITY_THRESHOLD
            bool FreeFall_Detected = false;

            // Implement Freefall detect algorithm between this ---------------v

            // ----------------------------------------------------------------^

            if ( FreeFall_Detected )
            {
                app_freefallData.state = APP_FREEFALL_STATE_FREEFALL_ALERT_ON;
            }
            else
            {
                app_freefallData.state = APP_FREEFALL_STATE_DATA_WAIT;
            }
        }
        break;

    case APP_FREEFALL_STATE_FREEFALL_ALERT_ON:
        LED2_Set();
        TC4_DelayMS(FREEFALL_LED_DELAY, DELAY_TIMER_FREEFALL_LED);
        APP_OLED_FreeFall_Alert(true);

        app_freefallData.state = APP_FREEFALL_STATE_FREEFALL_ALERT_OFF;
        break;

    case APP_FREEFALL_STATE_FREEFALL_ALERT_OFF:
        {
            // Challenge 2
            // Try add a Buzzer Alarm while Freefall event detected
            // Implement Buzzer Alarm between this ----------------------------v

            // ----------------------------------------------------------------^

            if (TC4_DelayIsComplete(DELAY_TIMER_FREEFALL_LED))
            {
                LED2_Clear();
                APP_OLED_FreeFall_Alert(false);

                app_freefallData.state = APP_FREEFALL_STATE_DATA_WAIT;
            }
        }
        break;

    /* The default state should never be executed. */
    case APP_FREEFALL_STATE_ERROR:
        break;
    }
}


/*******************************************************************************
 End of File
 */

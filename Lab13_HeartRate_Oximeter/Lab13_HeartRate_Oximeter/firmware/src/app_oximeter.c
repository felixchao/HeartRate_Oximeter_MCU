/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_oximeter.c

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
#include "main.h"
#include "app_oximeter.h"
#include "app_oled.h"
#include "max30102.h"
#include "algorithm.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_OXIMETER_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/
#define DV_ENABLE    0
#define DEBUG_ENABLE 0
#define HARDWARE_INT 0 // Please configure EXINT_4 (PA04) in Harmony and remember wire interrupt PIN to Module

APP_OXIMETER_DATA app_oximeterData;

#define DUMMY_SAMPLES           150     // Skip first 150 dummy samples for finger stable
#define TAKE_SAMPLES            500     // Take 500 samples to calculate
#define RECALCUATE_RATE         100     // re-Calculate after N new samples coming
#define RESULT_UPDATE_RATE      100     // Calculate result update after N new samples coming
#define HR_WAVE_UPDATE_RATE     4       // Heart Rate wave update after N new samples coming
#define SAMPLING_RATE           (6000)  // 100 Hz * 60secs = 6000 samples/minute
#define FINGER_DETECT_THRESHOLD (3000) // IR average generally less than 5000, if Finger attacged will larger than 50000
#define FINGER_ATTACH_THRESHOLD (90000) // IR average generally less than 5000, if Finger attacged will larger than 50000

uint32_t HeartRatePeriod = 0;
uint8_t sample_FIFO[6];     // MAX30120 FIFO samples
uint32_t word_RED, word_IR; // 18 bits word result
uint32_t min_RED, max_RED;  // min and max of RED LED
uint32_t sample_IR[TAKE_SAMPLES]; // IR LED sensor samples
uint32_t sample_RED[TAKE_SAMPLES];// Red LED sensor samples
int16_t result_SpO2;      // Result of SpO2
int16_t result_HeartRate; // Result of Heart Rate
int8_t  vaild_SpO2;       // Valid of SpO2
int8_t  valid_HeartRate;  // Valid of Heart Rate
int TakeSampleCount = 0;
int RecalSampleCount = 0;
int ResultUpdateCount = 0;
int WaveUpdateCount = 0;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
#if HARDWARE_INT
void OximeterDataReadyHandler(uintptr_t context)
{
    app_oximeterData.DataReady = true;
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
    void APP_OXIMETER_Initialize ( void )

  Remarks:
    See prototype in app_oximeter.h.
 */

void APP_OXIMETER_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_oximeterData.state = APP_OXIMETER_STATE_INIT;
    app_oximeterData.DataReady = false;
}


/******************************************************************************
  Function:
    void APP_OXIMETER_Tasks ( void )

  Remarks:
    See prototype in app_oximeter.h.
 */

void APP_OXIMETER_Tasks ( void )
{
    int i=0;

    /* Check the application's current state. */
    switch ( app_oximeterData.state )
    {
    /* Application's initial state. */
    case APP_OXIMETER_STATE_INIT:

#if HARDWARE_INT
        // Enable MAX30102 interrupt (EXINT4), must before max30102_init() to catch first interrupt
        EIC_CallbackRegister(EIC_PIN_4, OximeterDataReadyHandler, 0);
        EIC_InterruptEnable(EIC_PIN_4);
#endif
        // Oximeter sensor initialize
        max30102_Init();
        // Clean Interrupt Flags
        max30102_CheckInterrupt1();

        memset( sample_IR,  0, TAKE_SAMPLES*4 );
        memset( sample_RED, 0, TAKE_SAMPLES*4 );

        TakeSampleCount = 0;
        RecalSampleCount = 0;
        ResultUpdateCount = 0;
        WaveUpdateCount = 0;

        app_oximeterData.state = APP_OXIMETER_STATE_DATA_READY_CHECK;
        break;

    case APP_OXIMETER_STATE_DATA_READY_CHECK:
#if HARDWARE_INT
        if( app_oximeterData.DataReady==true )
        {
            app_oximeterData.DataReady=false;
#else
        if( max30102_CheckInterrupt1() & (INTFLAG_PPG_RDY|INTFLAG_A_FULL) )
        {
#endif
            max30102_ReadFIFO(sample_FIFO);

            app_oximeterData.state = APP_OXIMETER_STATE_FINGER_ATTACH_CHECK;
        }
        break;

    case APP_OXIMETER_STATE_FINGER_ATTACH_CHECK:
        // Combine values to get the actual number
        word_IR  = (long)((long)((long)sample_FIFO[0]&0x03)<<16) | (long)sample_FIFO[1]<<8 | (long)sample_FIFO[2];
        word_RED = (long)((long)((long)sample_FIFO[3]&0x03)<<16) | (long)sample_FIFO[4]<<8 | (long)sample_FIFO[5];

// Data Visualizer output, IR uint32, LED uint32
#if DV_ENABLE
        myprintf( "%c%c%c%c%c%c%c%c%c%c", 0x03,
            sample_FIFO[2], sample_FIFO[1], sample_FIFO[0]&0x03, 0x00,
            sample_FIFO[5], sample_FIFO[4], sample_FIFO[3]&0x03, 0x00,
            0xFC );
#endif

        if( word_RED < FINGER_DETECT_THRESHOLD )
        {
            // Less than background threshold
            TakeSampleCount=0;
            // Disable SpO2 LED
            max30102_SpO2_Enable( false );
            // Initial UI message
            APP_OLED_Oximeter_Status( OXIMETER_FINGER_DETECT );
            APP_OLED_Oximeter_Finger( 0 );
#if DEBUG_ENABLE
            myprintf("\033[4;1HPut your finger on sensor.");
            myprintf("\033[5;1H                          ");
#endif
        }
        else
        {
            if( word_RED> FINGER_ATTACH_THRESHOLD )
            {
                // Great than finger full attach threshold
                if( TakeSampleCount>=DUMMY_SAMPLES+TAKE_SAMPLES )
                {
                    // Collect full samples, start claculate
                    app_oximeterData.state = APP_OXIMETER_STATE_CALCULATE;
                    break;
                }

                if( (TakeSampleCount%20) == 0 )
                {
                    // Update UI message
                    APP_OLED_Oximeter_Status( OXIMETER_FINGER_CHECKING );
                    APP_OLED_Oximeter_Checking( TakeSampleCount, DUMMY_SAMPLES+TAKE_SAMPLES );
#if DEBUG_ENABLE
                    myprintf("\033[4;1HCalculating....%3d%%.....", TakeSampleCount*100/(DUMMY_SAMPLES+TAKE_SAMPLES));
#endif
                }

                TakeSampleCount++;
                if( TakeSampleCount < DUMMY_SAMPLES )
                {
                    // Drop dummy samples
                    if( TakeSampleCount == 50 )
                    {
                        // Turn SpO2 sensor after 50 samples coming in
                        max30102_SpO2_Enable( true );
                    }
                }
                else
                {
                    // Store stable FIFO data after DUMMY_SAMPLES
                    sample_IR[TakeSampleCount-DUMMY_SAMPLES]  = word_IR;
                    sample_RED[TakeSampleCount-DUMMY_SAMPLES] = word_RED;
                }
            }
            else
            {
                // Less than finger full attach threshold
                TakeSampleCount=0;
                // Clean data buffer
                memset( sample_IR,  0, TAKE_SAMPLES*4 );
                memset( sample_RED, 0, TAKE_SAMPLES*4 );
                // Update UI message
                APP_OLED_Oximeter_Status( OXIMETER_FINGER_WAIT_ATTACH );
                APP_OLED_Oximeter_Finger( word_RED );
#if DEBUG_ENABLE
                myprintf("\033[4;1HMove your finger close !  ");
#endif
            }
        }

        // Collect next FIFO data
        app_oximeterData.state = APP_OXIMETER_STATE_DATA_READY_CHECK;
        break;

    case APP_OXIMETER_STATE_CALCULATE:
        // Move out oldest FIFO data and push in latest one
        // Find Min and Max data in clooected FIFO buffers for UI upper/lowwer boundary
        min_RED=0x3FFFF;
        max_RED=0;
        for( i=1 ; i<TAKE_SAMPLES ; i++)
        {
            sample_IR[i-1]  = sample_IR[i];
            sample_RED[i-1] = sample_RED[i];
            if(min_RED>sample_RED[i]) min_RED=sample_RED[i];
            if(max_RED<sample_RED[i]) max_RED=sample_RED[i];
        }
        sample_IR[i-1]  = word_IR;
        sample_RED[i-1] = word_RED;
        if(min_RED>word_RED) min_RED=word_RED;
        if(max_RED<word_RED) max_RED=word_RED;

        // Calculate SpO2 and HR, in interval of RECALCUATE_RATE
        RecalSampleCount = (RecalSampleCount+1)%RECALCUATE_RATE;
        if( RecalSampleCount==0 )
        {
            maxim_heart_rate_and_oxygen_saturation(SAMPLING_RATE, sample_IR, TAKE_SAMPLES, sample_RED, &result_SpO2, &vaild_SpO2, &result_HeartRate, &valid_HeartRate);
        }

        // Go output result state
        app_oximeterData.state = APP_OXIMETER_STATE_DISPLAY_RESULT;
        break;

    case APP_OXIMETER_STATE_DISPLAY_RESULT:
        // Heart Rate LED1 control
        if( HeartRatePeriod==0 )
        {
            // New delay period start
            if( valid_HeartRate && result_HeartRate )
            {
                // Update HeartRatePeriod if result validate
                HeartRatePeriod = 60000/result_HeartRate;
                // Initial a delay about HeartRatePeriod
                TC4_DelayMS( HeartRatePeriod, DELAY_TIMER_HEARTBEAT_LED );
            }
        }
        else
        {
            // TurnOn LED1 after HeartRatePeriod
            if( TC4_DelayIsComplete( DELAY_TIMER_HEARTBEAT_LED ) )
            {
                LED1_Set();
                TC4_DelayMS( HEARTBEAT_LED_DUTY_DELAY, DELAY_TIMER_HEARTBEAT_LED_DUTY );
                HeartRatePeriod = 0;
            }
            // TurnOff LED1 after 100ms
            if( TC4_DelayIsComplete( DELAY_TIMER_HEARTBEAT_LED_DUTY ) )
            {
                LED1_Clear();
            }
        }

        // Output Result UI in interval of RESULT_UPDATE_RATE
        ResultUpdateCount = (ResultUpdateCount+1)%RESULT_UPDATE_RATE;
        if( ResultUpdateCount==0 )
        {
#if !DV_ENABLE && DEBUG_ENABLE
            myprintf("\033[4;1HHeart Rate (%d) : %3d bpm", valid_HeartRate,   result_HeartRate);
            myprintf("\033[5;1HSpO2       (%d) : %3d %%",  vaild_SpO2, result_SpO2);
#endif
            APP_OLED_Oximeter_Result( result_HeartRate, valid_HeartRate, result_SpO2, vaild_SpO2 );
        }

        // Output Wave UI in interval of HR_WAVE_UPDATE_RATE
        WaveUpdateCount = (WaveUpdateCount+1)%HR_WAVE_UPDATE_RATE;
        if( WaveUpdateCount==0 )
        {
            APP_OLED_Oximeter_Wave(min_RED, max_RED, word_RED);
        }

        app_oximeterData.state = APP_OXIMETER_STATE_DATA_READY_CHECK;
        break;

    /* The default state should never be executed. */
    case APP_OXIMETER_STATE_ERROR:
        break;
    }
}


/*******************************************************************************
 End of File
 */

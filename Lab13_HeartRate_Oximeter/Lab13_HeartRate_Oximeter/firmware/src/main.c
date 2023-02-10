/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "main.h"
#include "app_oled.h"
#include "app_oximeter.h"
#include "app_freefall.h"
#include "GraphicLib.h"

#define SYS_CONSOLE_PRINT_BUFFER_SIZE   200
static char consolePrintBuffer[SYS_CONSOLE_PRINT_BUFFER_SIZE];
volatile uint8_t TC3_HasExpired = 0;
uint16_t ADC_Result[2];
volatile uint8_t ADC_IsCompleted = 0;
volatile int8_t ADC_ChannelIdx = 0;
uint8_t Duty = 50;
int8_t DutyDistance = 2;

volatile uint32_t IntervalTimerCount[MAX_DELAY_TIMER];
volatile uint32_t IntervalTimerTarget[MAX_DELAY_TIMER];
float MCP9700_Temp;

void TC4_DelayMS(uint32_t ms, uint8_t idx)
{
    IntervalTimerCount[idx] = 1; // Start Delay Counter
    IntervalTimerTarget[idx] = ms;
}

bool TC4_DelayIsComplete(uint8_t idx)
{
    if (IntervalTimerCount[idx] != 0 &&
        IntervalTimerCount[idx] > IntervalTimerTarget[idx])
    {
        // Disable current Interval Timer
        IntervalTimerCount[idx] = 0;
        return true;
    }

    return false;
}

void ADC_Complete(ADC_STATUS status, uintptr_t context)
{
    if (status & ADC_INTFLAG_RESRDY_Msk)
    {
        ADC_Result[ADC_ChannelIdx] = ADC_ConversionResultGet();
        ADC_ChannelIdx = (ADC_ChannelIdx + 1) % 2;
        if (ADC_ChannelIdx == 0)
        {
            ADC_IsCompleted = 1;
        }
    }
}

void TC3_TimerExpired(TC_TIMER_STATUS status, uintptr_t context)
{
    if (status & TC_INTFLAG_OVF_Msk)
    {
        TC3_HasExpired = 1;
    }
}

void TC4_TimerExpired(TC_TIMER_STATUS status, uintptr_t context)
{
    int idx;

    for (idx = 0; idx < MAX_DELAY_TIMER; idx++)
    {
        if (IntervalTimerCount[idx] != 0)
        {
            IntervalTimerCount[idx]++;
        }
    }
}

void myprintf(const char *format, ...)
{
    size_t len = 0;
    va_list args = {0};

    va_start(args, format);
    len = vsnprintf(consolePrintBuffer, SYS_CONSOLE_PRINT_BUFFER_SIZE, format, args);
    va_end(args);

    if ((len > 0) && (len < SYS_CONSOLE_PRINT_BUFFER_SIZE))
    {
        consolePrintBuffer[len] = '\0';
        SERCOM5_USART_Write(consolePrintBuffer, len);
        while (SERCOM5_USART_WriteIsBusy());
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
int main(void)
{
    /* Initialize all modules */
    SYS_Initialize(NULL);
    APP_OLED_Initialize();
    APP_OXIMETER_Initialize();
    APP_FREEFALL_Initialize();

    TC3_TimerCallbackRegister(TC3_TimerExpired, (uintptr_t) NULL);
    TC3_TimerStart();
    TC4_TimerCallbackRegister(TC4_TimerExpired, (uintptr_t) NULL);
    TC4_TimerStart();
    TC4_DelayMS(BREATH_LED_DELAY, DELAY_TIMER_BREATH_LED);

    ADC_CallbackRegister(ADC_Complete, (uintptr_t) NULL);
    ADC_Enable();

    myprintf("\033[2J"); // Clear screen
    myprintf("\033[?25l"); // Hide cursor
    myprintf("\033[1;1H"); // Cursor to (1,1)

    TCC2_PWMStart();

    while (true)
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks();

        APP_OLED_Tasks();
        APP_OXIMETER_Tasks();
        APP_FREEFALL_Tasks();

        if (TC3_HasExpired)
        {
            TC3_HasExpired = 0;
            ADC_ConversionStart();
        }

        if (TC4_DelayIsComplete(DELAY_TIMER_BREATH_LED))
        {
            TC4_DelayMS(BREATH_LED_DELAY, DELAY_TIMER_BREATH_LED);

            Duty += DutyDistance;
            if (Duty >= 100 || Duty <= 0)
                DutyDistance = -DutyDistance;

            if (Duty >= 100)
                TCC2_PWM16bitDutySet(1, TCC2_PWM16bitPeriodGet() + 1);
            else
                TCC2_PWM16bitDutySet(1, ((uint32_t) Duty * TCC2_PWM16bitPeriodGet()) / 100);
        }

        if (ADC_IsCompleted)
        {
            ADC_IsCompleted = 0;

            MCP9700_Temp = (float) ((ADC_Result[1]*3300 / 4095) - 500) / 10.0f;
        }
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE);
}


/*******************************************************************************
 End of File
 */


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
#include "definitions.h"                // SYS function prototypes
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "Microchip_Logo.h"
#include "Alphabet_Fonts.h"


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
uint32_t i = 0;
uint8_t USARTRTxBuffer[100];
uint8_t USART5_ReceiveData[1];
volatile uint8_t USART5_IsReceived = 0;
volatile uint8_t USART5_IsTransmitted = 1;
#define SYS_CONSOLE_PRINT_BUFFER_SIZE    200
static char consolePrintBuffer[SYS_CONSOLE_PRINT_BUFFER_SIZE];
volatile uint8_t TC3_HasExpired = 0;
volatile uint8_t TC4_HasExpired = 0;
uint16_t ADC_Result[2];
volatile uint8_t ADC_IsCompleted = 0;
volatile int8_t ADC_ChannelIdx = 0;
uint8_t Duty = 50;
int8_t DutyDistance = 2;
#define LCM_WIDTH 128
#define LCM_HEIGHT 64
#define FONT_WIDTH  7
#define FONT_HEIGHT 8
uint8_t LogoX, LogoY;
unsigned char LCM_Blank[1024];
char Alphabet[95] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz~!@#$%^&()[]{}_\\|;:,.`'\"<>+-*/=? ";
// TODO 12.01
#define KXTJ3_ADDRESS   0x0E
uint8_t KXTJ3_WriteData[2];
uint8_t KXTJ3_ReadData[1];
volatile uint8_t I2C3_IsDataReady = 0;
uint8_t AxisOutReg[6] = {0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
uint8_t AxisOutByte[6];

const uint8_t LCM_InitCMD[] = {
    0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x02, 0xA1,
    0xC8, 0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF
};

void LCM_SetCursor(uint8_t x, uint8_t y)
{
    uint8_t OUT[3] = {0x0F & x, 0x10 + (x >> 4), 0xB0 + y};
    SSD1306_RS_PIN_Clear();
    SERCOM4_SPI_Write((void*) OUT, 3);
}

void LCM_DrawBitmap(uint8_t sx, uint8_t sy, uint8_t width, uint8_t height, const unsigned char *byte)
{
    for (int y = 0; y < (height / 8); y++)
    {
        LCM_SetCursor(sx, sy + y);
        SSD1306_RS_PIN_Set();
        SERCOM4_SPI_Write((void*) byte, (width - sx));
        byte += width;
    }
}

void LCM_DrawFont(uint8_t sx, uint8_t row, char ch)
{
    int chIdx = 0;

    while (ch != Alphabet[chIdx])
    {
        chIdx++;
    }
    LCM_SetCursor(sx, row);
    SSD1306_RS_PIN_Set();
    SERCOM4_SPI_Write((void*) (Alphabet_Fonts + (chIdx * FONT_WIDTH)), FONT_WIDTH);
}

void LCM_DrawString(uint8_t sx, uint8_t row, char *str)
{
    for (int x = 0; x < strlen(str); x++)
    {
        LCM_DrawFont(sx + (x * FONT_WIDTH), row, str[x]);
    }
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

void USART5_Transmit(uintptr_t context)
{
    USART5_IsTransmitted = 1;
}

void USART5_Receive(uintptr_t context)
{
    USART5_IsReceived = 1;
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
    if (status & TC_INTFLAG_OVF_Msk)
    {
        TC4_HasExpired = 1;
    }
}

// TODO 12.02
void I2C3_TransferCallback(uintptr_t contextHandle)
{
    if (KXTJ3_ReadData[0] & (1 << 4))
    {
        I2C3_IsDataReady = 1;
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

int main(void)
{
    /* Initialize all modules */
    SYS_Initialize(NULL);

    TC3_TimerCallbackRegister(TC3_TimerExpired, (uintptr_t) NULL);
    TC3_TimerStart();

    TC4_TimerCallbackRegister(TC4_TimerExpired, (uintptr_t) NULL);
    TC4_TimerStart();

    SERCOM5_USART_ReadCallbackRegister(USART5_Receive, (uintptr_t) NULL);
    SERCOM5_USART_WriteCallbackRegister(USART5_Transmit, (uintptr_t) NULL);
    SERCOM5_USART_Read(USART5_ReceiveData, 1);

    ADC_CallbackRegister(ADC_Complete, (uintptr_t) NULL);
    ADC_Enable();

    TCC2_PWMStart();

    SYSTICK_TimerStart();
    SSD1306_CS_PIN_Clear();
    SSD1306_RS_PIN_Clear();
    SERCOM4_SPI_Write((void*) LCM_InitCMD, sizeof (LCM_InitCMD));
    memset(LCM_Blank, 0, 1024);
    LCM_DrawBitmap(0, 0, LCM_WIDTH, LCM_HEIGHT, LCM_Blank);
    LogoX = 127;
    LogoY = 0;
    for (i = 0; i < 128; i += 2)
    {
        LCM_DrawBitmap(LogoX - i, LogoY, LCM_WIDTH, LCM_HEIGHT, Microchip_Logo);
        SYSTICK_DelayMs((i * i) / 400);
    }
    SYSTICK_DelayMs(1000);
    memset(LCM_Blank, 0, 1024);
    LCM_DrawBitmap(0, 0, LCM_WIDTH, LCM_HEIGHT, LCM_Blank);
    sprintf((char *) USARTRTxBuffer, "SysClock %ld.%-ldMHz", SYSTICK_TimerFrequencyGet() / 1000000, (SYSTICK_TimerFrequencyGet() % 1000000) / 1000);
    LCM_DrawString(0, 0, (char *) USARTRTxBuffer);
    sprintf((char *) USARTRTxBuffer, "TC3 period %ldms", TC3_Timer16bitPeriodGet()*1000 / TC3_TimerFrequencyGet());
    LCM_DrawString(0, 1, (char *) USARTRTxBuffer);
    sprintf((char *) USARTRTxBuffer, "TC4 period %ldms", TC4_Timer16bitPeriodGet()*1000 / TC4_TimerFrequencyGet());
    LCM_DrawString(0, 2, (char *) USARTRTxBuffer);
    sprintf((char *) USARTRTxBuffer, "Received Data :");
    LCM_DrawString(0, 7, (char *) USARTRTxBuffer);

    // TODO 12.03
    KXTJ3_WriteData[0] = 0x1D;
    KXTJ3_WriteData[1] = 0x80;
    SERCOM3_I2C_Write(KXTJ3_ADDRESS, KXTJ3_WriteData, 2);
    myprintf("G-Sensor Software Reset Completed\r\n");
    KXTJ3_WriteData[0] = 0x0F;
    SERCOM3_I2C_WriteRead(KXTJ3_ADDRESS, KXTJ3_WriteData, 1, KXTJ3_ReadData, 1);
    while (SERCOM3_I2C_IsBusy());
    myprintf("G-Sensor WHO AM I : 0x%X\r\n", KXTJ3_ReadData[0]);
    KXTJ3_WriteData[0] = 0x1B;
    KXTJ3_WriteData[1] = 0xF0;
    SERCOM3_I2C_Write(KXTJ3_ADDRESS, KXTJ3_WriteData, 2);
    myprintf("G-Sensor Initial Completed\r\n");
    SERCOM3_I2C_CallbackRegister(I2C3_TransferCallback, 0);

    myprintf("\033[2J"); // Clear screen
    myprintf("\033[?25l"); // Hide cursor
    while (true)
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks();

        if (TC3_HasExpired)
        {
            TC3_HasExpired = 0;
            LED1_Toggle();

            sprintf((char *) USARTRTxBuffer, "Hello World!");
            LCM_DrawString(0, 3, (char *) USARTRTxBuffer);

            myprintf("\033[1;1HHello World!!");

            ADC_ConversionStart();

            // TODO 12.04
            KXTJ3_WriteData[0] = 0x16;
            SERCOM3_I2C_WriteRead(KXTJ3_ADDRESS, KXTJ3_WriteData, 1, KXTJ3_ReadData, 1);
        }

        if (TC4_HasExpired)
        {
            TC4_HasExpired = 0;

            Duty += DutyDistance;
            if (Duty >= 100 || Duty <= 0)
                DutyDistance = -DutyDistance;

            if (Duty >= 100)
                TCC2_PWM16bitDutySet(1, TCC2_PWM16bitPeriodGet() + 1);
            else
                TCC2_PWM16bitDutySet(1, ((uint32_t) Duty * TCC2_PWM16bitPeriodGet()) / 100);

            sprintf((char *) USARTRTxBuffer, "PWM Duty : %3d%%", Duty);
            LCM_DrawString(0, 4, (char *) USARTRTxBuffer);

            LED2_Toggle();
           
        }

        if (USART5_IsReceived)
        {
            USART5_IsReceived = 0;

            sprintf((char *) USARTRTxBuffer, "Received Data : %1c", USART5_ReceiveData[0]);
            LCM_DrawString(0, 7, (char *) USARTRTxBuffer);

            myprintf("\033[5;1HReceived Data : %1c", USART5_ReceiveData[0]);
            SERCOM5_USART_Read(USART5_ReceiveData, 1);
        }

        if (ADC_IsCompleted)
        {
            ADC_IsCompleted = 0;

            if (ADC_Result[0] >= 4095)
                TCC2_PWM16bitDutySet(1, TCC2_PWM16bitPeriodGet() + 1);
            else
                TCC2_PWM16bitDutySet(1, (uint32_t) ADC_Result[0] * TCC2_PWM16bitPeriodGet() / 4095);

            sprintf((char *) USARTRTxBuffer, "VR1  : %4d", ADC_Result[0]);
            LCM_DrawString(0, 5, (char *) USARTRTxBuffer);
            sprintf((char *) USARTRTxBuffer, "Temp : %4d", ADC_Result[1]);
            LCM_DrawString(0, 6, (char *) USARTRTxBuffer);

            myprintf("\033[2;1HVR1 Value : %4d", ADC_Result[0]);
            myprintf("\033[3;1HTemperature Value : %4d", ADC_Result[1]);
        }

        // TODO 12.05
        if (I2C3_IsDataReady)
        {
            for (i = 0; i < 6; i++)
            {
                SERCOM3_I2C_WriteRead(KXTJ3_ADDRESS, &AxisOutReg[i], 1, &AxisOutByte[i], 1);
                while (SERCOM3_I2C_IsBusy());
            }
            myprintf("\033[4;1HG-Sensor = (%5d, %5d, %5d)\r\n", (short) (AxisOutByte[1] << 8 | AxisOutByte[0]), (short) (AxisOutByte[3] << 8 | AxisOutByte[2]), (short) (AxisOutByte[5] << 8 | AxisOutByte[4]));
            I2C3_IsDataReady = 0;
        }
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE);
}


/*******************************************************************************
 End of File
 */


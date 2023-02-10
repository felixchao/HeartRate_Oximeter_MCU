/* ************************************************************************** */

#include <string.h>

/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include "LCM.h"
#include "peripheral/port/plib_port.h"
#include "peripheral/sercom/spi_master/plib_sercom4_spi_master.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
// TIME system service
// SPI DRIVER
// SERCOM SPI     : MOSI (PB10)
// SERCOM SPI     : SCK  (PB11)
// SERCOM SPI     : SS   (PA13)          , Hardware control
// SSD1306_CS_PIN : SS   (PA13)          , GPIO Output Latch high
// SSD1306_RESET  : Reset(PA27)          , GPIO Output Latch high
// SSD1306_RS_PIN : A0/Data/Command(PA12), GPIO Output Latch high
#define OLED_RESET_ENABLE  0  // Enable OLED Reset PIN on PA27
#define OLED_CS_PIN_GPIO   1  // Enable SPI GPIO Slave Select on PA13

uint8_t LCM_FrameBuf[LCM_FRAME_SIZE]; // (128x64)/8, 1bit/pixel
uint8_t *LCM_pFrameBuf = NULL; // The Frame Buffer Pointer
const uint8_t LCM_InitCMD[]={
    0xAE,          // DISPLAY OFF
    0xD5,          // SET OSC FREQUENY
    0x80,          // divide ratio = 1 (bit 3-0), OSC (bit 7-4)
    0xA8,          // SET MUX RATIO
    0x3F,          // 64MUX
    0xD3,          // SET DISPLAY OFFSET
    0x00,          // offset = 0
    0x40,          // set display start line, start line = 0
    0x8D,          // ENABLE CHARGE PUMP REGULATOR
    0x14,          //
    0x20,          // SET MEMORY ADDRESSING MODE
    0x02,          // horizontal addressing mode
    0xA1,          // set segment re-map, column address 127 is mapped to SEG0
    0xC8,          // set COM/Output scan direction, remapped mode (COM[N-1] to COM0)
    0xDA,          // SET COM PINS HARDWARE CONFIGURATION
    0x12,          // alternative COM pin configuration
    0x81,          // SET CONTRAST CONTROL
    0xCF,          //
    0xD9,          // SET PRE CHARGE PERIOD
    0xF1,          //
    0xDB,          // SET V_COMH DESELECT LEVEL
    0x40,          //
    0xA4,          // DISABLE ENTIRE DISPLAY ON
    0xA6,          // TEXT_NORMAL MODE (A7 for inverse display)
    0xAF           // DISPLAY ON
};

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
// Example of Pixel to Bit calculate
//     X0  X1  X2  X3  X4  X5  X6  X7  X8  X9  X10 X11 X12 X13 X14 X15
//     B00 B01 B02 B03 B04 B05 B06 B07 B08 B09 B10 B11 B12 B13 B14 B15
// Y00 b00                                                         b00
// Y01 b01                                                         b01
// Y02 b02                                                         b02
// Y03 b03                                                         b03
// Y04 b04                                                         b04
// Y05 b05                                                         b05
// Y06 b06              #                                          b06
// Y07 b07                                                         b07
//
//     B16 B17 B18 B19 B20 B21 B22 B23 B24 B25 B26 B27 B28 B29 B30 B31
// Y08 b00                                                         b00
// Y09 b01                                                         b01
// Y10 b02                                                         b02
// Y11 b03                                                         b03
// Y12 b04                                                         b04
// Y13 b05                                                         b05
// Y14 b06                                                  *      b06
// Y15 b07                                                         b07
//
// WxH = 16x16 = 256 pixels(bit) > 256/8 = 32 Bytes
//
// * = (X,Y)  = (13,14)
// PixelByte  = x+((y/8)*W) = 13+((14/8)*16) = B29
// PixelOfset = y%8         = 14%8           = b06
//
// # = (X,Y)  = (4,6)
// PixelByte  = x+((y/8)*W) =  4+(( 6/8)*16) = B04
// PixelOfset = y%8         =  6%8           = b06
//
static void _Pixel2Bit( uint16_t x, uint16_t y, uint8_t pixel )
{
    uint16_t Byte;
    uint8_t  Bit;

    if( x>=LCM_WIDTH || y>=LCM_HEIGHT ) return;

    Byte = x+((int)(y/8)*LCM_WIDTH);
    Bit  = y%8;
    LCM_pFrameBuf[Byte] = (LCM_pFrameBuf[Byte]&(~(1<<Bit)))|(pixel<<Bit);
}

void LCM_SetLayerBuf( uint8_t *pLayer )
{
    // Assign LayerBuf for Drawing
    LCM_pFrameBuf = pLayer;
}

uint8_t* LCM_GetFrameBuf( void )
{
    // Return LCM Frame Buffer Address for Merge
    LCM_pFrameBuf = LCM_FrameBuf;

    return LCM_pFrameBuf;
}

uint8_t LCM_Init( void )
{
#if OLED_RESET_ENABLE
    // Reset Start
    SSD1306_RESET_Clear();

    // Reset Release
    SSD1306_RESET_Set();
#endif

#if OLED_CS_PIN_GPIO
    // Chip Enable
    SSD1306_CS_PIN_Clear();
#endif

    // Command Transfer
    SSD1306_RS_PIN_Clear();

    // Send LCM Init Command
    SERCOM4_SPI_Write(( void* )LCM_InitCMD, sizeof (LCM_InitCMD));

#if OLED_CS_PIN_GPIO
    // Chip Disable
    SSD1306_CS_PIN_Set();
#endif

    // Assign Default FrameBuf
    LCM_pFrameBuf = LCM_FrameBuf;

    return true;
}

void LCM_Clean( void )
{
    // Erase Frame Buffer
    memset(( void* )LCM_pFrameBuf, 0, 1024);
}

void LCM_Update( void )
{
    uint8_t *pFrame = LCM_pFrameBuf;
    uint8_t PixelAddr[3] = {0x00, 0x10, 0xB0};

#if OLED_CS_PIN_GPIO
    // Chip Enable
    SSD1306_CS_PIN_Clear();
#endif

    // Update full Frame
    while( PixelAddr[2] < (0xB0+(LCM_HEIGHT/8)) )
    {
        // Command Transfer (Pixel Address)
        SSD1306_RS_PIN_Clear();
        SERCOM4_SPI_Write(( void* )PixelAddr, 3);

        // Data Transfer (Pixel Data)
        SSD1306_RS_PIN_Set();
        SERCOM4_SPI_Write(( void* )pFrame, LCM_WIDTH);

        // Index to next 8 row for Pixels
        pFrame += LCM_WIDTH;
        PixelAddr[2]++;
    }

#if OLED_CS_PIN_GPIO
    // Chip Disable
    SSD1306_CS_PIN_Set();
#endif
}

void LCM_Pixel( uint16_t x, uint16_t y, uint8_t pixel )
{
    if( x>=LCM_WIDTH || y>=LCM_HEIGHT ) return;

    // Update Pixel to Frame Buffer
    _Pixel2Bit( x, y, pixel );
}

void LCM_Region( uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t pixel )
{
    uint16_t Col, Row;

    if( x+w>LCM_WIDTH || y+h>LCM_HEIGHT ) return;

    // Update a Region to Frame Buffer
    for( Row = 0 ; Row < h ; Row++ )
    {
        for( Col = 0 ; Col < w ; Col++ )
        {
            _Pixel2Bit( x+Col, y+Row, pixel );
        }
    }
}

void LCM_Bitmap( uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t background, const unsigned char *bitmap )
{
    uint16_t Col, Row;
    uint16_t Byte;
    uint8_t Bit;
    uint8_t Pixel;

    // Update a Bitmap to Frame Buffer
    for( Row = 0 ; Row < h ; Row++ )
    {
        for( Col = 0 ; Col < w ; Col++ )
        {
            Byte = Col+((int)(Row/8)*w);
            Bit  = Row%8;
            Pixel  = (bitmap[Byte]>>Bit)&0x1;
            if( Pixel || (!Pixel && background) )
            {
                _Pixel2Bit( x+Col, y+Row, Pixel );
            }
        }
    }
}

/* *****************************************************************************
 End of File
 */

/* ************************************************************************** */
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
#include <math.h>
#include <string.h>
#include "GraphicLib.h"
#include "Font7x16.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
#define  GPL_LAYERS LAYER_MAX
struct {
    uint8_t Buf[LCM_FRAME_SIZE]; // (128x64)/8, 1bit/pixel, Layer Buffer
    uint8_t Show;
} GPL_Layer[GPL_LAYERS];

uint8_t  GPL_Invalidate = false;
uint16_t GPL_PenSize  = 1; // Good for Odd number 1,3,5,7,9
char     GPL_Alphabet[95] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz~!@#$%^&()[]{}_\\|;:,.`'\"<>+-*/=? ";

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
void GPL_SetPenSize( uint16_t pixel )
{
    GPL_PenSize = pixel;
}

uint16_t GPL_GetPenSize( uint16_t pixel )
{
    return GPL_PenSize;
}

uint8_t GPL_ScreenInit( void )
{
    for( int Layer=0 ; Layer < GPL_LAYERS ; Layer++ )
    {
        GPL_Layer[Layer].Show = GPL_SHOW;
    }
    return LCM_Init();
}

void GPL_LayerSet( uint8_t index )
{
    if( index < GPL_LAYERS )
    {
        // Assign Layer Frame Buffer
        LCM_SetLayerBuf( GPL_Layer[index].Buf );
    }
}

void GPL_LayerShow( uint8_t index, uint8_t show )
{
    if( index < GPL_LAYERS )
    {
        // Assign Layer Frame Buffer
        GPL_Layer[index].Show = show;
    }
}

void GPL_LayerClean( uint8_t index )
{
    if( index < GPL_LAYERS )
    {
        // Assign Layer Frame Buffer
        memset(GPL_Layer[index].Buf, 0, sizeof(GPL_Layer[index].Buf));
    }
}

void GPL_ScreenClean( void )
{
    LCM_Clean();

    // Request to Update Screen
    GPL_Invalidate = true;
}

void GPL_ScreenUpdate( void )
{
    uint8_t *pFrame = NULL;
    uint16_t  Layer, Byte;

    // If Update Screen is requested
    if( GPL_Invalidate )
    {
        pFrame = LCM_GetFrameBuf();
        memset( pFrame, 0, LCM_FRAME_SIZE );
        for( Layer=0 ; Layer < GPL_LAYERS ; Layer++ )
        {
            if( GPL_Layer[Layer].Show == GPL_SHOW )
            {
                for( Byte=0 ; Byte<LCM_FRAME_SIZE ; Byte++ )
                {
                    pFrame[Byte]|=GPL_Layer[Layer].Buf[Byte];
                }
            }
        }
        LCM_Update();
        GPL_Invalidate = false;
    }
}

void GPL_DrawPoint( uint16_t x, uint16_t y, uint8_t pixel )
{
    if( x < (GPL_PenSize>>1) )  x=(GPL_PenSize>>1);
    if( y < (GPL_PenSize>>1) )  y=(GPL_PenSize>>1);
    LCM_Region( x-(GPL_PenSize>>1), y-(GPL_PenSize>>1), GPL_PenSize, GPL_PenSize, pixel );

    // Request to Update Screen
    GPL_Invalidate = true;
}

void GPL_DrawLine( uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2 )
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;

    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if( delta_x > 0 )       {   incx =  1;  }
    else if( delta_x == 0 ) {   incx =  0;  }
    else                    {   incx = -1;  delta_x = -delta_x; }

    if( delta_y > 0 )       {   incy =  1;  }
    else if( delta_y == 0 ) {   incy =  0;  }
    else                    {   incy = -1;  delta_y = -delta_y; }

    if( delta_x > delta_y ) {   distance = delta_x; }
    else                    {   distance = delta_y; }

    for( t = 0; t <= distance + 1; t++ )
    {
        GPL_DrawPoint(uRow, uCol, PIXEL_SET);

        xerr += delta_x;
        yerr += delta_y;

        if( xerr > distance )
        {
            xerr -= distance;
            uRow += incx;
        }

        if( yerr > distance )
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

void GPL_DrawRect( uint16_t x, uint16_t y, uint16_t w, uint16_t h )
{
    GPL_DrawLine(    x,     y, x+w-1,     y);
    GPL_DrawLine(    x, y+h-1, x+w-1, y+h-1);
    GPL_DrawLine(    x,     y,     x, y+h-1);
    GPL_DrawLine(x+w-1,     y, x+w-1, y+h-1);
}

void GPL_FillRect( uint16_t x, uint16_t y, uint16_t w, uint16_t h )
{
    LCM_Region( x, y, w, h, PIXEL_SET );

    // Request to Update Screen
    GPL_Invalidate = true;
}

void GPL_DrawCross( uint16_t x0, uint16_t y0, uint16_t r )
{
    GPL_DrawLine( x0-r, y0, x0+r, y0 );
    GPL_DrawLine( x0, y0-r, x0, y0+r );
}

#if 0
// General Circle Algorithm
void GPL_DrawCircle( uint16_t x0, uint16_t y0, uint8_t r )
{
    int x, y, k;

    for( x=(x0-r) ; x<=(x0+r) ; x++ )
    {
        for( y=(y0-r) ; y<=(y0+r) ; y++ )
        {
            k = ((x-x0)*(x-x0))+((y-y0)*(y-y0))-(r*r);

            if( k>-(r+1) && k<(r+1) )
            {
                GPL_DrawPoint(x, y, PIXEL_SET);
            }
        }
    }
}

#else

// Bresenhxm's Circle Algorithm
void GPL_DrawCircle( uint16_t x0, uint16_t y0, uint16_t r )
{
    int x, y, d;

    x = 0;
    y = r;
    d = 3-(r<<1);

    while( x <= y )
    {
        GPL_DrawPoint( x0+x, y0+y, PIXEL_SET );
        GPL_DrawPoint( x0+y, y0+x, PIXEL_SET );
        GPL_DrawPoint( x0-y, y0+x, PIXEL_SET );
        GPL_DrawPoint( x0-x, y0+y, PIXEL_SET );
        GPL_DrawPoint( x0-x, y0-y, PIXEL_SET );
        GPL_DrawPoint( x0-y, y0-x, PIXEL_SET );
        GPL_DrawPoint( x0+y, y0-x, PIXEL_SET );
        GPL_DrawPoint( x0+x, y0-y, PIXEL_SET );

        if( d > 0 )
        {
            y = y - 1;
            d = d + ((x-y)<<2) + 10;
        }
        else
        {
            d = d + (x<<2) + 6;
        }

        GPL_DrawPoint( x0+x, y0+y, PIXEL_SET );
        GPL_DrawPoint( x0+y, y0+x, PIXEL_SET );
        GPL_DrawPoint( x0-y, y0+x, PIXEL_SET );
        GPL_DrawPoint( x0-x, y0+y, PIXEL_SET );
        GPL_DrawPoint( x0-x, y0-y, PIXEL_SET );
        GPL_DrawPoint( x0-y, y0-x, PIXEL_SET );
        GPL_DrawPoint( x0+y, y0-x, PIXEL_SET );
        GPL_DrawPoint( x0+x, y0-y, PIXEL_SET );

        x = x + 1;
    }
}

#endif

void GPL_DrawBitmap( uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t background, const unsigned char *bitmap )
{
    LCM_Bitmap( x, y, w, h, background, bitmap );

    // Request to Update Screen
    GPL_Invalidate = true;
}

void GPL_DrawFont( uint16_t x, uint16_t y, char chr, uint8_t background, uint8_t highlight )
{
    uint16_t chIdx = 0;
    uint8_t FontChar[FONT_WIDTH*FONT_HEIGHT/8];

    // Search chr in GPL_Alphabet[]
    while( chr != GPL_Alphabet[chIdx] && chIdx<sizeof(GPL_Alphabet) )
    { chIdx++; }
    if( chIdx>=sizeof(GPL_Alphabet) )
    { return; }

    // Extract chr bitmap from Font Bitmap, Font Bitmap height should multiple of 8 (8, 16, 24 ...)
    // Example of Two Font Char and Font Width = 7, Height = 16
    //
    // Font Bitmap Array = Byte00, Byte01, Byte02, ... Byte15,| Byte16, Byte17, ... Byte 31
    //
    //                           [ Font Char 1 ]   [ Font Char 2 ]
    //                         |    Font W=7     |     Font W=7
    // Font H = 16 | Bit[0: 7] | Byte00 - Byte07 |  Byte08 - Byte15
    //             | Bit[8:15] | Byte16 - Byte23 |  Byte24 - Byte31

    // First Row of Font
    memcpy(FontChar,            Font7x16+(chIdx*FONT_WIDTH),                        FONT_WIDTH);
    // Second Row of Font
    memcpy(FontChar+FONT_WIDTH, Font7x16+((chIdx+sizeof(GPL_Alphabet))*FONT_WIDTH), FONT_WIDTH);

    // Highlight(Invert) font
    if( highlight )
    {
        for(int i=0 ; i<FONT_WIDTH*(FONT_HEIGHT+(FONT_HEIGHT%8))/8 ; i++ )
        {
            FontChar[i] = ~FontChar[i];
        }
    }

    GPL_DrawBitmap(x, y, FONT_WIDTH, FONT_HEIGHT, background, FontChar);
}

void GPL_DrawRowString( uint16_t x, uint16_t row, char *str, uint8_t background, uint8_t highlight )
{
    for( int chrIdx=0 ; chrIdx<strlen(str); chrIdx++ )
    {
        GPL_DrawFont( x+(chrIdx*FONT_WIDTH), row*FONT_HEIGHT, str[chrIdx], background, highlight );
    }
}

void GPL_DrawString( uint16_t x, uint16_t y, char *str, uint8_t background, uint8_t highlight )
{
    for( int chrIdx=0 ; chrIdx<strlen(str); chrIdx++ )
    {
        GPL_DrawFont( x+(chrIdx*FONT_WIDTH), y, str[chrIdx], background, highlight );
    }
}

/* *****************************************************************************
 End of File
 */

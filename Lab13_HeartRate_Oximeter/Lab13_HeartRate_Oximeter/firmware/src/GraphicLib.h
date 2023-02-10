/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _GRAPHICLIB_H    /* Guard against multiple inclusion */
#define _GRAPHICLIB_H

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include "definitions.h"
#include "LCM.h"

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
// app_oled.c external functions

// LCM size
#define LCM_WIDTH       128
#define LCM_HEIGHT      64
#define LCM_CENTER_X    (LCM_WIDTH>>1)
#define LCM_CENTER_Y    (LCM_HEIGHT>>1)

// Font size, Font Bitmap height should multiple of 8 (8, 16, 24 ...)
#define FONT_WIDTH       7
#define FONT_HEIGHT      16
#define FONT_HEIGHT_REAL 12

// Math
#define PI              3.14159
#define DEG2RAD( deg )    ( deg * PI / 180 )
#define RAD2DEG( rad )    ( rad * 180 / PI )

enum {
    PIXEL_CLEAN = 0,
    PIXEL_SET
};

enum {
    BG_TRANSPARENT = 0,
    BG_SOLID
};

enum {
    TEXT_NORMAL = 0,
    TEXT_HIGHLIGHT
};

enum {
    LAYER_ALERT,
    LAYER_STRING,
    LAYER_GRAPHIC,
    LAYER_MAX
};

enum {
    GPL_HIDE,
    GPL_SHOW,
};

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************
uint8_t GPL_ScreenInit( void );
void GPL_LayerSet( uint8_t index );
void GPL_LayerShow( uint8_t index, uint8_t show );
void GPL_LayerClean( uint8_t index );
void GPL_ScreenClean( void );
void GPL_ScreenUpdate( void );
void GPL_SetPenSize( uint16_t pixel );
uint16_t GPL_GetPenSize( uint16_t pixel );
void GPL_DrawPoint( uint16_t x, uint16_t y, uint8_t pixel );
void GPL_DrawLine( uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2 );
void GPL_DrawRect( uint16_t x, uint16_t y, uint16_t w, uint16_t h );
void GPL_FillRect( uint16_t x, uint16_t y, uint16_t w, uint16_t h );
void GPL_DrawCross( uint16_t x0, uint16_t y0, uint16_t r );
void GPL_DrawCircle( uint16_t x0, uint16_t y0, uint16_t r );
void GPL_DrawBitmap( uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t background, const unsigned char *bitmap );
void GPL_DrawFont( uint16_t x, uint16_t y, char chr, uint8_t background, uint8_t highlight );
void GPL_DrawRowString( uint16_t x, uint16_t row, char *str, uint8_t background, uint8_t highlight );
void GPL_DrawString( uint16_t x, uint16_t y, char *str, uint8_t background, uint8_t highlight );

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _GRAPHICLIB_H */

/* *****************************************************************************
 End of File
 */

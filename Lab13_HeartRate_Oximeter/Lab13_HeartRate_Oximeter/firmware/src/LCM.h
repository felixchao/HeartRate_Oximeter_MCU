/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Microchip

  @File Name
    lcm.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _LCM_H    /* Guard against multiple inclusion */
#define _LCM_H

#include "definitions.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Constants                                                         */
    /* ************************************************************************** */
    /* ************************************************************************** */

#define LCM_WIDTH   128
#define LCM_HEIGHT  64
#define LCM_FRAME_SIZE  ((LCM_WIDTH*LCM_HEIGHT)/8) // (128x64)/8) = 1024, 1bit/pixel

    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************


    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************
void LCM_SetLayerBuf( uint8_t *pLayer );
uint8_t * LCM_GetFrameBuf( void );
uint8_t LCM_Init( void );
void LCM_Clean( void );
void LCM_Update( void );
void LCM_Pixel( uint16_t x, uint16_t y, uint8_t pixel );
void LCM_Region( uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t pixel );
void LCM_Bitmap( uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t background, const unsigned char *bitmap );

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _LCM_H */

/* *****************************************************************************
 End of File
 */

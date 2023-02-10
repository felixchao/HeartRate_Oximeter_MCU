/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Microchip

  @File Name
    main.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _MAIN_H    /* Guard against multiple inclusion */
#define _MAIN_H

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
enum {

    DELAY_TIMER_BREATH_LED,
    DELAY_TIMER_HEARTBEAT_LED,
    DELAY_TIMER_HEARTBEAT_LED_DUTY,
    DELAY_TIMER_SPLASH_WAIT,
    DELAY_TIMER_GRPAHIC_UPDATE,
    DELAY_TIMER_GSENSOR_READ,
    DELAY_TIMER_FREEFALL_LED,
    MAX_DELAY_TIMER
};

#define BREATH_LED_DELAY            50   // The Breath LED PWM dimming interval delay (LED3)
#define HEARTBEAT_LED_DUTY_DELAY    100  // The Heartbeat LED On to Off interval duty delay (LED1)
#define SPLASH_WAIT_DELAY           1000 // The delay time after splash screen
#define GRPAHIC_UPDATE_DELAY        100  // The OLED update interval delay
#define GSENSOR_READ_DELAY          100  // The G-Sensor Data Read interval delay
#define FREEFALL_LED_DELAY          2000 // The Free-Fall Aleart On delay (LED2)

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
extern float MCP9700_Temp;
void myprintf(const char *format, ...);
void TC4_DelayMS( uint32_t ms, uint8_t idx );
bool TC4_DelayIsComplete( uint8_t idx );

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _MAIN_H */

/* *****************************************************************************
 End of File
 */

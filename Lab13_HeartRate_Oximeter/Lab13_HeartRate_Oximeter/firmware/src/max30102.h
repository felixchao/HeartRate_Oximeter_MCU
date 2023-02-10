#ifndef __MAX30102_H
#define __MAX30102_H

#include "definitions.h"

#define MAX30102_SLAVE_ADDR 0x57

// Register addresses and configuration
#define REG_INTR_STATUS_1 0x00 // Interrupt Status 1
    #define INTFLAG_A_FULL  0x80 // FIFO Almost Full Interrupt Flag
    #define INTFLAG_PPG_RDY 0x40 // New FIFO Data Ready Interrupt Flag
    #define INTFLAG_ALC_OVF 0x20 // Ambient Light Cancellation Overflow Iinterrupt Flag
    #define INTFLAG_PWR_RDY 0x01 // Power Ready Flag Iinterrupt Flag
#define REG_INTR_STATUS_2 0x01 // Interrupt Status 2
    #define INTFLAG_DIE_TEMP_RDY 0x02 // Internal Temperature Ready Interrupt Flag
#define REG_INTR_ENABLE_1 0x02 // Interrupt Enable 1
    #define A_FULL_EN  0x80 // FIFO Almost Full Interrupt Enable
    #define PPG_RDY_EN 0x40 // New FIFO Data Ready Interrupt Enable
    #define ALC_OVF_EN 0x20 // Ambient Light Cancellation Overflow Iinterrupt Enable
#define REG_INTR_ENABLE_2 0x03 // Interrupt Enable 2
    #define DIE_TEMP_RDY_EN 0x02 // Internal Temperature Ready Interrupt Enable
#define REG_FIFO_WR_PTR   0x04 // FIFO Write Pointer
#define REG_OVF_COUNTER   0x05 // FIFO Overflow Counter
#define REG_FIFO_RD_PTR   0x06 // FIFO Read Pointer
#define REG_FIFO_DATA     0x07 // FIFO Data Register
#define REG_FIFO_CONFIG   0x08 // FIFO Configuration
    // Sample Averaging
    #define SMP_AVE_NO 0x00
    #define SMP_AVE_02 0x01
    #define SMP_AVE_04 0x02
    #define SMP_AVE_08 0x03
    #define SMP_AVE_16 0x04
    #define SMP_AVE_32 0x05
    // FIFO Rolls on Full
    #define FIFO_ROLLOVER_ENABLE  0x10
    #define FIFO_ROLLOVER_DISABLE 0x00
    // FIFO Almost Full Value
    #define FIFO_A_FULL_UNREAD_32 0x00
    #define FIFO_A_FULL_UNREAD_31 0x01
    #define FIFO_A_FULL_UNREAD_30 0x02
    #define FIFO_A_FULL_UNREAD_29 0x03
    #define FIFO_A_FULL_UNREAD_28 0x04
    #define FIFO_A_FULL_UNREAD_27 0x05
    #define FIFO_A_FULL_UNREAD_26 0x06
    #define FIFO_A_FULL_UNREAD_25 0x07
    #define FIFO_A_FULL_UNREAD_24 0x08
    #define FIFO_A_FULL_UNREAD_23 0x09
    #define FIFO_A_FULL_UNREAD_22 0x0A
    #define FIFO_A_FULL_UNREAD_21 0x0B
    #define FIFO_A_FULL_UNREAD_20 0x0C
    #define FIFO_A_FULL_UNREAD_19 0x0D
    #define FIFO_A_FULL_UNREAD_18 0x0E
    #define FIFO_A_FULL_UNREAD_17 0x0F
#define REG_MODE_CONFIG 0x09 // Mode Configuration
    // Shutdown Control
    #define SHDN_ENABLE   0x80
    // Reset Control
    #define RESET_ENABLE  0x40
    // Mode Control, 0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
    #define MODE_HEARRATE 0x02
    #define MODE_SPO2     0x03
    #define MODE_MULTILED 0x07
#define REG_SPO2_CONFIG 0x0A // SpO2 Configuration
    // SpO2 ADC Range Control
    #define SPO2_ADC_RGE_2048nA  0x00
    #define SPO2_ADC_RGE_4096nA  0x20
    #define SPO2_ADC_RGE_8192nA  0x40
    #define SPO2_ADC_RGE_16384nA 0x60
    // SpO2 Sample Rate Control
    #define SPO2_SR_50HZ   0x00
    #define SPO2_SR_100HZ  0x04
    #define SPO2_SR_200HZ  0x08
    #define SPO2_SR_400HZ  0x0C
    #define SPO2_SR_800HZ  0x10
    #define SPO2_SR_1000HZ 0x14
    #define SPO2_SR_1600HZ 0x18
    #define SPO2_SR_3200HZ 0x1C
    // LED Pulse Width Control and ADC Resolution
    #define LED_PW_69uS  0x00
    #define LED_PW_118uS 0x01
    #define LED_PW_215uS 0x02
    #define LED_PW_411uS 0x03
#define REG_LED1_PA 0x0C // LED1 Pulse Amplitude, 0x00~0xFF = 0.00mA ~ 51.0mA, 0.2mA/bit
#define REG_LED2_PA 0x0D // LED2 Pulse Amplitude, 0x00~0xFF = 0.00mA ~ 51.0mA, 0.2mA/bit
#define REG_MULTI_LED_CTRL1 0x11 // Multi-LED Mode Control Registers 1
#define REG_MULTI_LED_CTRL2 0x12 // Multi-LED Mode Control Registers 2
#define REG_TEMP_INTR 0x1F   // Temperature Integer  (Temperature Data)
#define REG_TEMP_FRAC 0x20   // Temperature Fraction (Temperature Data)
#define REG_TEMP_CONFIG 0x21 // Temperature Config   (Temperature Data)

void max30102_Init(void);
void max30102_Write(uint8_t Register_Address, uint8_t WriteByte);
void max30102_Read(uint8_t Register_Address, uint8_t *ReadByte);
uint8_t max30102_CheckInterrupt1( void );
uint8_t max30102_CheckInterrupt2( void );
void max30102_ReadFIFO(uint8_t* ReadBytes);
void max30102_SpO2_Enable(bool OnOff);
#endif

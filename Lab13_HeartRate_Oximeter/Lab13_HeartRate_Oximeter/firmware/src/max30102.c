#include "max30102.h"

// Write one byte to target register address
void max30102_Write(uint8_t Register_Address, uint8_t WriteByte)
{
    uint8_t WriteBuf[2];

    // Write byte (Blocking)
    WriteBuf[0] = Register_Address;
    WriteBuf[1] = WriteByte;
    SERCOM3_I2C_Write( MAX30102_SLAVE_ADDR, WriteBuf, 2 );
    while( SERCOM3_I2C_IsBusy() );
}

// Write to target register address and read one byte
void max30102_Read(uint8_t Register_Address, uint8_t *ReadByte)
{
    uint8_t WriteBuf[1];
    uint8_t ReadBuf[1];

    // Write and Read byte (Blocking)
    WriteBuf[0] = Register_Address;
    SERCOM3_I2C_WriteRead( MAX30102_SLAVE_ADDR, WriteBuf, 1, ReadBuf, 1 );
    while( SERCOM3_I2C_IsBusy() );

    if( ReadByte!=NULL )
        *ReadByte = ReadBuf[0];
}

// Get and Clear interrupt 1 flags
uint8_t max30102_CheckInterrupt1( void )
{
    uint8_t INTFlag1;

    // Clear Interrupt Flags (Blocking)
    max30102_Read(REG_INTR_STATUS_1, &INTFlag1);
    while( SERCOM3_I2C_IsBusy() );

    return INTFlag1;
}

// Get and Clear interrupt 2 flags
uint8_t max30102_CheckInterrupt2( void )
{
    uint8_t INTFlag2;

    // Clear Interrupt Flags (Blocking)
    max30102_Read(REG_INTR_STATUS_2, &INTFlag2);
    while( SERCOM3_I2C_IsBusy() );

    return INTFlag2;
}

// Clear interrupt flags, Write to target register address and read 6 FIFO bytes
// Both Red amd IR FIFO are 18 bits
// Byte[0:2] : 0 0  IR[17:16] +  IR[15:8] +  IR[7:0]
// Byte[3:5] : 0 0 Red[17:16] + Red[15:8] + Red[7:0]
void max30102_ReadFIFO(uint8_t* ReadBytes)
{
    uint8_t WriteBuf[1];

    // Clear Interrupt Flags (Blocking)
    max30102_Read(REG_INTR_STATUS_1, NULL);

    // Read FIFO bytes (Blocking)
    WriteBuf[0] = REG_FIFO_DATA;
    SERCOM3_I2C_WriteRead( MAX30102_SLAVE_ADDR, WriteBuf, 1, ReadBytes, 6 );
    while( SERCOM3_I2C_IsBusy() );
}

// Initialization
void max30102_Init(void)
{
    // Reset chip
    max30102_Write(REG_MODE_CONFIG, RESET_ENABLE);
    max30102_Write(REG_MODE_CONFIG, RESET_ENABLE);
    // INTR setting, FIFO Almost Full and New FIFO Data Ready Interrupt Enable
    max30102_Write(REG_INTR_ENABLE_1, A_FULL_EN|PPG_RDY_EN);
    max30102_Write(REG_INTR_ENABLE_2, 0x00);
    //FIFO_WR_PTR[4:0], FIFO Write Pointer
    max30102_Write(REG_FIFO_WR_PTR, 0x00);
    //OVF_COUNTER[4:0], FIFO Overflow Counter
    max30102_Write(REG_OVF_COUNTER, 0x00);
    //FIFO_RD_PTR[4:0], FIFO Read Pointer
    max30102_Write(REG_FIFO_RD_PTR, 0x00);
    // sample avg = 1, fifo rollover=false, fifo almost full = 17
    max30102_Write(REG_FIFO_CONFIG, SMP_AVE_NO|FIFO_ROLLOVER_DISABLE|FIFO_A_FULL_UNREAD_17);
    // 0x02 for Heart Rate only for start
    max30102_Write(REG_MODE_CONFIG, MODE_HEARRATE);
    // 0x27 SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)
    max30102_Write(REG_SPO2_CONFIG, SPO2_ADC_RGE_4096nA|SPO2_SR_100HZ|LED_PW_411uS);
    // Choose value for ~ 7.2mA for LED1, 0x00~0xFF = 0.00mA ~ 51.0mA, 0.2mA/bit
    max30102_Write(REG_LED1_PA, 0x24);
    // Choose value for ~ 7.2mA for LED2, 0x00~0xFF = 0.00mA ~ 51.0mA, 0.2mA/bit
    max30102_Write(REG_LED2_PA, 0x24);
}

// SpO2 enable/disable control
void max30102_SpO2_Enable( bool OnOff )
{
    static bool lastOnOff = false;

    // Status no change
    if( OnOff == lastOnOff ) return;

    lastOnOff = OnOff;

    if( OnOff )
        max30102_Write(REG_MODE_CONFIG, MODE_SPO2);
    else
        max30102_Write(REG_MODE_CONFIG, MODE_HEARRATE);
}

/** @file ina219.c */

#include "i2c.h"
#include "ina219.h"

int INA219calibrate(unsigned char i2c_addr)
{
    unsigned char reg[2];
    reg[0] = (INA219_CAL_VALUE >> 8) & 0xFF;
    reg[1] = (INA219_CAL_VALUE) & 0xFF;
    if(i2c_write(i2c_addr, INA219_CAL_REG, (void*)&reg, 2) == 2)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int INA219readVoltage(unsigned char i2c_addr, unsigned short *millivolts)
{
    unsigned char reg[2];
    if(i2c_read(i2c_addr, INA219_VOLT_REG, reg, 2) != 2)
    {
        *millivolts = 0;
        return 1;
    }
    *millivolts = ((reg[0] << 5) + (reg[1] >> 3)) * 4;
    return 0;
}

int INA219readCurrent(unsigned char i2c_addr, short *milliamps)
{
    unsigned char reg[2];
    if(i2c_read(i2c_addr, INA219_CURR_REG, reg, 2) != 2)
    {
        *milliamps = 0;
        return 1;
    }
    *milliamps = (short)((reg[0] << 8) + reg[1]);
    return 0;
}

int INA219readPower(unsigned char i2c_addr, int *milliwatts)
{
    /* power register precision is only 4 mW per bit */
    /*unsigned char reg[2];
    if(i2c_read(i2c_addr, INA219_POWER_REG, reg, 2) != 2)
    {
        *milliwatts = 0;
        return 1;
    }
    *milliwatts = ((reg[0] << 8) + reg[1])*4;*/
    
    unsigned short millivolts;
    short milliamps;
    
    INA219readVoltage(i2c_addr, &millivolts);
    INA219readCurrent(i2c_addr, &milliamps);
    
    *milliwatts = ((int)millivolts * (int)milliamps) / 1000;
    
    return 0;
}

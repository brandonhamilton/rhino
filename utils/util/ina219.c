/** @file ina219.c */

#include "i2c.h"
#include "ina219.h"

int INA219calibrate(unsigned char i2c_addr)
{
    unsigned short reg = INA219_CAL_VALUE;
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

int INA219readCurrent(unsigned char i2c_addr, short *microamps)
{
    unsigned char reg[2];
    if(i2c_read(i2c_addr, INA219_CURR_REG, reg, 2) != 2)
    {
        *microamps = 0;
        return 1;
    }
    *microamps = (short)((reg[0] << 8) + reg[1]) * 200;
    return 0;
}

int INA219readPower(unsigned char i2c_addr, int *microwatts)
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
    short microamps;
    
    INA219readVoltage(i2c_addr, &millivolts);
    INA219readCurrent(i2c_addr, &microamps);
    
    *microwatts = ((int)millivolts * (int)microamps) / 1000;
    
    return 0;
}

/** @file max1668.c */

#include "i2c.h"
#include "max1668.h"

int MAX1668readLocalTemp(unsigned char i2c_addr, char *degreesC)
{
    if(i2c_read(i2c_addr, MAX1668_RIT_REG, degreesC, 1) != 1)
    {
        *degreesC = 0;
        return 1;
    }
    return 0;
}

int MAX1668readDX1Temp(unsigned char i2c_addr, char *degreesC)
{
    if(i2c_read(i2c_addr, MAX1668_RET1_REG, degreesC, 1) != 1)
    {
        *degreesC = 0;
        return 1;
    }
    return 0;
}

int MAX1668readDX2Temp(unsigned char i2c_addr, char *degreesC)
{
    if(i2c_read(i2c_addr, MAX1668_RET2_REG, degreesC, 1) != 1)
    {
        *degreesC = 0;
        return 1;
    }
    return 0;
}

int MAX1668readDX3Temp(unsigned char i2c_addr, char *degreesC)
{
    if(i2c_read(i2c_addr, MAX1668_RET3_REG, degreesC, 1) != 1)
    {
        *degreesC = 0;
        return 1;
    }
    return 0;
}

int MAX1668readDX4Temp(unsigned char i2c_addr, char *degreesC)
{
    if(i2c_read(i2c_addr, MAX1668_RET4_REG, degreesC, 1) != 1)
    {
        *degreesC = 0;
        return 1;
    }
    return 0;
}

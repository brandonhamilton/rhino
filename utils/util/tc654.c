#include "i2c.h"
#include "tc654.h"

int TC654readRPM1(unsigned char i2c_addr, unsigned short *rpm)
{
    unsigned char trpm;
    if(i2c_read(i2c_addr, TC654_RPM1, &trpm, 1) != 1)
    {
        *rpm = 0;
        return 1;
    }
    unsigned char config;
    unsigned char mult = 50;
    if(TC654readConfig(i2c_addr, &config))
    {
        if(config & 0x40)
        {
            mult = 25;
        }
    }
    *rpm = trpm * mult;
    return 0;
}

int TC654readRPM2(unsigned char i2c_addr, unsigned short *rpm)
{
    unsigned char trpm;
    if(i2c_read(i2c_addr, TC654_RPM2, &trpm, 1) != 1)
    {
        *rpm = 0;
        return 1;
    }
    unsigned char config;
    unsigned char mult = 50;
    if(TC654readConfig(i2c_addr, &config))
    {
        if(config & 0x40)
        {
            mult = 25;
        }
    }
    *rpm = trpm * mult;
    return 0;
}

int TC654readFault1(unsigned char i2c_addr, unsigned short *threshold)
{
    unsigned char tt;
    if(i2c_read(i2c_addr, TC654_FAN_FAULT1, &tt, 1) != 1)
    {
        *threshold = 0;
        return 1;
    }
    *threshold = tt * 50;
    return 0;
}

int TC654readFault2(unsigned char i2c_addr, unsigned short *threshold)
{
    unsigned char tt;
    if(i2c_read(i2c_addr, TC654_FAN_FAULT2, &tt, 1) != 1)
    {
        *threshold = 0;
        return 1;
    }
    *threshold = tt * 50;
    return 0;
}

int TC654writeFault1(unsigned char i2c_addr, const unsigned short threshold)
{
    unsigned char tt;
    tt = threshold / 50;
    if(i2c_write(i2c_addr, TC654_FAN_FAULT1, &tt, 1) != 1)
    {
        return 1;
    }
    return 0;
}

int TC654writeFault2(unsigned char i2c_addr, const unsigned short threshold)
{
    unsigned char tt;
    tt = threshold / 50;
    if(i2c_write(i2c_addr, TC654_FAN_FAULT2, &tt, 1) != 1)
    {
        return 1;
    }
    return 0;
}

int TC654readConfig(unsigned char i2c_addr, unsigned char *config)
{
    if(i2c_read(i2c_addr, TC654_CONFIG, config, 1) != 1)
    {
        *config = 0;
        return 1;
    }
    return 0;
}

int TC654writeConfig(unsigned char i2c_addr, const unsigned char config)
{
    if(i2c_write(i2c_addr, TC654_CONFIG, &config, 1) != 1)
    {
        return 1;
    }
    return 0;
}

int TC654readStatus(unsigned char i2c_addr, unsigned char *status)
{
    if(i2c_read(i2c_addr, TC654_STATUS, status, 1) != 1)
    {
        *status = 0;
        return 1;
    }
    return 0;
}

int TC654readDutyCycle(unsigned char i2c_addr, unsigned char *ds)
{
    if(i2c_read(i2c_addr, TC654_DUTY_CYCLE, ds, 1) != 1)
    {
        *ds = 0;
        return 1;
    }
    return 0;
}

int TC654writeDutyCycle(unsigned char i2c_addr, const unsigned char ds)
{
    if(i2c_write(i2c_addr, TC654_DUTY_CYCLE, &ds, 1) != 1)
    {
        return 1;
    }
    return 0;
}

int TC654readManufacturerID(unsigned char i2c_addr, unsigned char *mfr)
{
    if(i2c_read(i2c_addr, TC654_MFR_ID, mfr, 1) != 1)
    {
        *mfr = 0;
        return 1;
    }
    return 0;
}

int TC654readVersionID(unsigned char i2c_addr, unsigned char *version)
{
    if(i2c_read(i2c_addr, TC654_VER_ID, version, 1) != 1)
    {
        *version = 0;
        return 1;
    }
    return 0;
}

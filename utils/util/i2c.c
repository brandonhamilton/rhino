#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "i2c.h"

#define I2C_SLAVE   0x0703
#define I2C_MAX_BUFLEN 4

static int i2c_dev = 0;
static int slave_address = 0;

int i2c_open(const char *file)
{
    i2c_dev = open(file, O_RDWR);

    if(i2c_dev == -1)
    {
        fprintf(stderr, "Failed to open %s\n", file);
        return 1;
    }
    
    return 0;
}

int i2c_close()
{
    int rv = 0;
    if(i2c_dev)
    {
        rv = close(i2c_dev);
        i2c_dev = 0;
    }
    
    return rv;
}

int i2c_read(unsigned char adr, unsigned char reg, void *buf, int buf_len)
{
    if(adr != slave_address)
    {
        if(ioctl(i2c_dev, I2C_SLAVE, adr) < 0)
        {
            fprintf(stderr, "Failed to set slave address\n");
            return -1;
        }
        else
        {
            slave_address = adr;
        }
    }
    
    if(write(i2c_dev, &reg, 1) != 1)
    {
        return -1;
    }
    
    return read(i2c_dev, buf, buf_len);
}

int i2c_write(unsigned char adr, unsigned char reg, const void *buf, int buf_len)
{
    static unsigned char regbuf[I2C_MAX_BUFLEN + 1];
    int i;
    
    if(buf_len > I2C_MAX_BUFLEN)
    {
        return -1;
    }
    
    if(adr != slave_address)
    {
        if(ioctl(i2c_dev, I2C_SLAVE, adr) < 0)
        {
            fprintf(stderr, "Failed to set slave address\n");
            return -1;
        }
        else
        {
            slave_address = adr;
        }
    }
    
    regbuf[0] = reg;
    for(i = 0; i < buf_len; i++)
    {
        regbuf[i+1] = ((unsigned char*)buf)[i];
    }
    
    if(write(i2c_dev, regbuf, buf_len + 1) == (buf_len + 1))
    {
        return buf_len;
    }
    else
    {
        return -1;
    }
}

/*
 * This program samples the registers of the INA219 chip over I2C to 
 * calculate the power utilization of the ARM3517 processor on 
 * the rhino platform.
 *
 * Author: Brandon Hamilton
 */


#include <stdio.h>
#include <stdint.h>
#include "i2c-dev.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MON_5V_PROC_ADDR	0x49

#define INA219_VOLT_REG		0x02
#define INA219_CURR_REG		0x04
#define INA219_CALIB_REG	0x05

int main(void)
{
	unsigned char reg_val[3];
	int fd;
	unsigned int res;
	int voltage;
	int current;
	float power;
	fd = open("/dev/i2c-1", O_RDWR );

	if( fd == -1 )
	{
		fprintf( stderr, "Failed to open /dev/i2c-1: %m\n" );
		return 1;
	}

	if( ioctl( fd, I2C_SLAVE, MON_5V_PROC_ADDR ) < 0 )
	{
		fprintf( stderr, "Failed to set slave address: %m\n" );
		return 1;
	}

	reg_val[0] = INA219_CALIB_REG;
	reg_val[1] = 0x20;
	reg_val[2] = 0x00;

	if (write(fd, reg_val, 3) != 3)
	{
		fprintf( stderr, "Failed to calibrate the current sensors: %m\n" );
		return 1;
	}

	while(1)
	{
		/* Voltage */
		reg_val[0] = INA219_VOLT_REG;
		if (write(fd, reg_val, 1) != 1)
		{
			fprintf( stderr, "Failed to write the voltage sensors address: %m\n" );
			return 1;
		}

		if (read(fd, reg_val, 2) != 2)
		{
			fprintf( stderr, "Failed to read the voltage sensors: %m\n" );
			return 1;
		}
		else
		{
			voltage = ((reg_val[0] << 5) + (reg_val[1] >> 3)) * 4;
		}

		/* Current */
		reg_val[0] = INA219_CURR_REG;
		if (write(fd, reg_val, 1) != 1)
		{
			fprintf( stderr, "Failed to write the current sensors address: %m\n" );
			return 1;
		}

		if (read(fd, reg_val, 2) != 2)
		{
			fprintf( stderr, "Failed to read the current sensors: %m\n" );
			return 1;
		}
		else
		{
			current =  ((reg_val[0] << 8) + (reg_val[1]) * 4;
		}

		power = ((float)voltage*(float)current)/1000000;
		printf("Power: %.4f W   (%d mA * %d mV)\n", power, current, voltage);

		sleep(1);
	}
}


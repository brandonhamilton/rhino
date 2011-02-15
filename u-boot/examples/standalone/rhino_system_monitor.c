/*
 * Simon Scott
 * University of Cape Town
 * February 2011
 *
 * Monitors all voltages, currents, temperatures and fan speeds, and prints them to the console.
 * After 30 seconds, the FPGA is powered up. After 3 minutes, the FMC cards are powered up.
 * After 4 minutes, the fans are switched on.
 */

#include <common.h>
#include <exports.h>

/* Define GPIOs */

#define FPGA_VCCINT_EN      	99
#define FMC0_SUPPLY_EN		102

/* I2C addresses*/

#define TEMP_MON_ADDR		0x18
#define FAN_CONTRLR_ADDR	0x1B
#define MON_12V_ADDR		0x40
#define MON_1V2_FPGA_ADDR	0x41
#define MON_1V5_FPGA_ADDR	0x42
#define MON_2V5_FPGA_ADDR	0x43
#define MON_3V3_FPGA_ADDR	0x44
#define MON_2V5_FMC_ADDR	0x45
#define MON_3V3_FMC_ADDR	0x46
#define MON_12V_FMC_ADDR	0x47
#define MON_5V_PROC_ADDR	0x49

/* I2C register definitions*/

#define INA219_VOLT_REG		0x02
#define INA219_CURR_REG		0x04
#define INA219_CALIB_REG	0x05

#define MAX1668_DX1_TEMP_REG	0x01
#define MAX1668_DX2_TEMP_REG	0x02
#define MAX1668_DX3_TEMP_REG	0x03

#define TC654_RPM1_REG		0x00
#define TC654_RPM2_REG		0x01
#define TC654_CONFIG_REG	0x04


/* Declare array of I2C addresses and names for the power monitors */
uchar pwr_mon_addresses[] = {MON_12V_ADDR, MON_1V2_FPGA_ADDR, MON_1V5_FPGA_ADDR, 
					MON_2V5_FPGA_ADDR, MON_3V3_FPGA_ADDR, MON_2V5_FMC_ADDR,
					MON_3V3_FMC_ADDR, MON_12V_FMC_ADDR, MON_5V_PROC_ADDR};


/*
 * The "main" function.
 */
int rhino_system_monitor (int argc, char *argv[])
{
	/* Declare local variables */
	int seconds, i;
	uchar reg_val[2];

	app_startup(argv);
	printf ("\n== Rhino System Monitor ==\n\n");

	/* Set the I2C bus to I2C_1, as all the power devices are on this bus */
	if(i2c_set_bus_num(0) != 0)
		printf("Error setting I2C bus number\n");

	/* Switch off fans */
	reg_val[0] = 0x15;

	if(i2c_write(FAN_CONTRLR_ADDR, TC654_CONFIG_REG, 1, reg_val, 1) != 0)
		printf("Error switching off fans.\n");

	/* Calibrate current sensors */
	reg_val[0] = 0x20;
	reg_val[1] = 0x00;

	for(i = 0; i < 9; i++)
	{
		if(i2c_write(pwr_mon_addresses[i], INA219_CALIB_REG, 1, reg_val, 2) != 0)
			printf("Error calibrating current sensor %d.\n", (i+1));
	}

	/* Print out column headings */
	printf("%s %s %s %s\n", "TIME, 12V VOLT, 1V2 FPGA VOLT, 1V5 FPGA VOLT, 2V5 FPGA VOLT, 3V3 FPGA VOLT,",
			"2V5 FMC VOLT, 3V3 FMC VOLT, 12V FMC VOLT, 5V PROC VOLT, 12V CURR, 1V2 FPGA CURR,",
			"1V5 FPGA CURR, 2V5 FPGA CURR, 3V3 FPGA CURR, 2V5 FMC CURR, 3V3 FMC CURR, 12V FMC CURR,",
			"5V PROC CURR, PROC TEMP, FPGA TEMP, AMBIENT TEMP, FAN1 RPM, FAN2 RPM");

	/* Main loop: runs at 0.5Hz, for 6 minutes */
	for(seconds = 0; seconds < 6*60; seconds += 2)
	{
		printf("%d,", seconds);

		/* Print out all voltage readings */
		for(i = 0; i < 9; i++)
		{
			if(i2c_read(pwr_mon_addresses[i], INA219_VOLT_REG, 1, reg_val, 2) != 0)
				printf("Error reading I2C voltage register for monitor %d.\n", (i+1));

			printf("%d,", ((reg_val[0] << 5) + (reg_val[1] >> 3)) * 4);
		}

		/* Print out all current readings */
		for(i = 0; i < 9; i++)
		{
			if(i2c_read(pwr_mon_addresses[i], INA219_CURR_REG, 1, reg_val, 2) != 0)
				printf("Error reading I2C current register for monitor %d.\n", (i+1));

			printf("%d,", (short)((reg_val[0] << 8) + reg_val[1]));
		}

		/* Print out all temperature readings */

		if(i2c_read(TEMP_MON_ADDR, MAX1668_DX1_TEMP_REG, 1, reg_val, 1) != 0)
			printf("Error reading proc temperature.\n");

		printf("%d,", reg_val[0]);

		if(i2c_read(TEMP_MON_ADDR, MAX1668_DX2_TEMP_REG, 1, reg_val, 1) != 0)
			printf("Error reading FPGA temperature.\n");

		printf("%d,", reg_val[0]);

		if(i2c_read(TEMP_MON_ADDR, MAX1668_DX3_TEMP_REG, 1, reg_val, 1) != 0)
			printf("Error reading AMB temperature.\n");

		printf("%d,", reg_val[0]);

		/* Print out all fan speeds */

		if(i2c_read(FAN_CONTRLR_ADDR, TC654_RPM1_REG, 1, reg_val, 1) != 0)
			printf("Error reading fan1 speed.\n");

		printf("%d,", reg_val[0] * 50);

		if(i2c_read(FAN_CONTRLR_ADDR, TC654_RPM1_REG, 1, reg_val, 1) != 0)
			printf("Error reading fan2 speed.\n");

		printf("%d\n", reg_val[0] * 50);


		/* After 30 seconds, switch on FPGA */
		if(seconds == 30)
		{
			printf("Switching on FPGA\n");

			for (i=0; i<3; i++)
			{
    			/*Request each GPIO line and make it output*/
    			omap_request_gpio(FPGA_VCCINT_EN + i);
    			omap_set_gpio_direction(FPGA_VCCINT_EN + i, 0);

    			/*Set power supply enable signal to appropriate level*/
        		omap_set_gpio_dataout(FPGA_VCCINT_EN + i, 1);
			}
		}

		/* After 1 minute, send message to program FPGA */
		if(seconds == 60)
		{
			printf("FPGA programming now beginning\n");
		}

		/* After 2.5 minutes, FPGA should be programmed */
		if(seconds == 150)
		{
			printf("FPGA now programmed\n");
		}

		/* After 3 minutes, switch on FMC cards */
		if(seconds == 3*60)
		{
			printf("Switching on FMC cards\n");
    
			for (i=0; i<4; i++)
			{
    			/*Request each GPIO line and make it output*/
    			omap_request_gpio(FMC0_SUPPLY_EN + i);
    			omap_set_gpio_direction(FMC0_SUPPLY_EN + i, 0);

    			/*Set power supply enable signal to appropriate level*/
        		omap_set_gpio_dataout(FMC0_SUPPLY_EN + i, 1);
			}
		}

		/* After 4 minutes, switch on fans */
		if(seconds == 4*60)
		{
			printf("Switching on fans\n");
			reg_val[0] = 0x14;

			if(i2c_write(FAN_CONTRLR_ADDR, TC654_CONFIG_REG, 1, reg_val, 1) != 0)
				printf("Error switching on fans.\n");
		}

		/* Wait for the rest of the 2 second cycle */
		udelay(600000);
	}

	/* Done */
	printf("Finished.\n");
	
	return (0);
}

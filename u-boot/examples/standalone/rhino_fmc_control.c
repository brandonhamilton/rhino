/*
 * Simon Scott
 * University of Cape Town
 * January 2011
 *
 * FMC control program. Switches on the two FMC cards, sets the GA[1:0] and PGOOD signals,
 * and then reads the FMC_nPRSNT signals to determine which FMC cards are present.
 * The software then writes to the SiLabs clock on the Xilinx FMC Debug Board
 * (which may be plugged into either FMC connector), and changes the frequency from
 * 156MHz to 312MHz.
 */

#include <common.h>
#include <exports.h>

/* GPIO pin definitions */

#define FMC0_SUPPLY_EN		102
#define FMC0_AUX_EN			103
#define FMC1_SUPPLY_EN		104
#define FMC1_AUX_EN			105

#define FMC0_GA0			140
#define FMC0_GA1			141
#define FMC0_nPRSNT_M2C		142

#define FMC1_GA0			155
#define FMC1_GA1			152
#define FMC1_nPRSNT_M2C		153

#define FMC_PG_C2M			143

#define FMC_I2C_BUS_NUM		3


/* Macros to simplify GPIO access */

#define READ_GPIO(GPIO_NUM, result) omap_request_gpio(GPIO_NUM); \
									omap_set_gpio_direction(GPIO_NUM, 1); \
									result = omap_get_gpio_datain(GPIO_NUM)

#define WRITE_GPIO(GPIO_NUM, value) omap_request_gpio(GPIO_NUM); \
									omap_set_gpio_direction(GPIO_NUM, 0); \
									omap_set_gpio_dataout(GPIO_NUM, value)

/* I2C register definitions */

#define SI_CLK_ADDR		0x5D



/*
 * The "main" function.
 */
int rhino_fmc_control (int argc, char *argv[])
{
	/* Declare local variables */
	int i;
	char fmc0_nprsnt_m2c_in, fmc1_nprsnt_m2c_in;
	uchar si_clk_reg7;

	app_startup(argv);
	printf ("\n== Rhino FMC Control Program ==\n\n");

	/* 	Enable all the FMC power supplies 
		Note: in practice, we would enable only the AUX supply, read the EEPROM chip,
		check if the board supports VADJ=2.5V, and then turn on the main FMC supplies.
		However, since we always use the FMC debug board, we can be lazy. */
	
	printf("Switching on FMC supplies.\n");

    for (i=0; i<4; i++)
    {
        WRITE_GPIO(FMC0_SUPPLY_EN + i, 1);
    }

	/* 	Set the "power good" signal high.
		Note: in practice, we should check the voltages first using the power monitors.
		Also set the GA0, GA1 signals appropriately. */

	printf("Setting FMC_PG_C2M = 1, FMC0_GA[1:0] = 00, FMC1_GA[1:0] = 01.\n");

    WRITE_GPIO(FMC_PG_C2M, 1);
   	WRITE_GPIO(FMC0_GA0, 0);
   	WRITE_GPIO(FMC0_GA1, 0);
   	WRITE_GPIO(FMC1_GA0, 1);
   	WRITE_GPIO(FMC1_GA1, 0);

	printf("Done setting signals.\n");
	
	/* Display the FMC "present" signals */

    READ_GPIO(FMC0_nPRSNT_M2C, fmc0_nprsnt_m2c_in);
    READ_GPIO(FMC1_nPRSNT_M2C, fmc1_nprsnt_m2c_in);

	printf("FMC0_nPRST_M2C> %d\n", fmc0_nprsnt_m2c_in);
	printf("FMC1_nPRST_M2C> %d\n", fmc1_nprsnt_m2c_in);
	
	/* Wait 100msecs before accessing clock chip via I2C */
	udelay(100000);

	/* Program the SiLabs clock on the FMC Debug Card to 312.5MHz, using I2C
	 * Register 7 is the clock divider. By setting it to zero, we double original
	 * frequency of 156.25MHz. */
	if(i2c_set_bus_num(2) != 0)
		printf("Error setting I2C bus number\n");

	if(i2c_read(SI_CLK_ADDR	, 7, 1, &si_clk_reg7, 1) != 0)
		printf("Error reading I2C register 7.\n");

	printf("Register 7 of SiLab chip was %d. Setting to 0x00\n", si_clk_reg7);

	si_clk_reg7 = 0;

	if(i2c_write(SI_CLK_ADDR, 7, 1, &si_clk_reg7, 1) != 0)
		printf("Error writing I2c register 7.\n");

	/* Done */
	printf("Finished.\n");

    return (0);
}

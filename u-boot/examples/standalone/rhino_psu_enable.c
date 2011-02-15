/*
 * Simon Scott
 * University of Cape Town
 * January 2011
 */

#include <common.h>
#include <exports.h>

#define FPGA_VCCINT_EN      99
#define FPGA_VCCO_AUX_EN    100
#define FPGA_VCCMGT_EN      101
#define FMC0_SUPPLY_EN      102
#define FMC0_AUX_EN         103
#define FMC1_SUPPLY_EN      104
#define FMC1_AUX_EN         105

int rhino_psu_enable (int argc, char *argv[])
{
    int i;

	app_startup(argv);
	printf ("Rhino PSU Enable/Disable\n");

    if(argc < 4)
    {
        printf("Incorrect arguments.\n\
                Usage: rhino_psu_enable (FPGA_VCCINT) (FPGA_VCCO_AUX) (FPGA_VCCMGT)\n\
                where each parameter is '0' or '1'");
        return -1;
    }

    for (i=0; i<3; i++)
    {
        /*Request each GPIO line and make it output*/
        omap_request_gpio(99 + i);
        omap_set_gpio_direction(99 + i, 0);

        /*Set power supply enable signal to appropriate level*/
        if(argv[i+1][0] == '1')
            omap_set_gpio_dataout(99 + i, 1);
        else if(argv[i+1][0] == '0')
            omap_set_gpio_dataout(99 + i, 0);
    }

    return (0);
}

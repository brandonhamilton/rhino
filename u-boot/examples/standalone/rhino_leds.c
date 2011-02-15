/*
 * Simon Scott
 * University of Cape Town
 * January 2011
 */

#include <common.h>
#include <exports.h>

int rhino_leds (int argc, char *argv[])
{
    int i;

	app_startup(argv);
	printf ("Rhino LED Control Program\n");

    if(argc < 3)
    {
        printf("Incorrect arguments.\nUsage: rhino_leds x y   ,\
                 where x,y = '0' or '1'");
        return -1;
    }

    for (i=0; i<2; i++)
    {
        /*Request LED GPIO line and make it output*/
        /*U_LED_0 is on GPIO_130. U_LED_1 is on GPIO_131*/
        omap_request_gpio(130 + i);
        omap_set_gpio_direction(130 + i, 0);

        if(argv[i+1][0] == '1')
            omap_set_gpio_dataout(130 + i, 1);
        else if(argv[i+1][0] == '0')
            omap_set_gpio_dataout(130 + i, 0);
    }

    return (0);
}

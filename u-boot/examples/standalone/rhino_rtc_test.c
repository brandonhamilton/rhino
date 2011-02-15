/*
 * Simon Scott
 * University of Cape Town
 * January 2011
 */

#include <common.h>
#include <exports.h>
#include <spi.h>

int rhino_leds (int argc, char *argv[])
{
    struct spi_slave* rtc;
	uchar dout[8];
	uchar din[8];
	int i;

	app_startup(argv);
	printf ("Rhino Real-time Clock Test Program\n");

    if(argc < 1)
    {
        return -1;
    }

	/*Bus 0 (McSPI1), CS0, 40 kHz, mode*/
	rtc = spi_setup_slave(0, 0,	40000, SPI_MODE_1);

	/*Claim the bus*/
	if(spi_claim_bus(rtc) != 0)
	{
		printf("Error claiming SPI bus. Exiting...\n");
		return -1;
	}

	/*Write seconds register*/
	dout[0] = 0x81;
	dout[1] = 0x01;

	if( spi_xfer(rtc, 16, &dout[0], NULL, SPI_XFER_BEGIN | SPI_XFER_END) != 0 )
	{
		printf("Error with SPI transfer 1.\n");
	}

	/*Wait 4 seconds*/
	for(i = 0; i < 2500000; i++)
	{
		omap_request_gpio(130);
	}

	/*Read it back*/
	dout[0] = 0x01;
	dout[1] = 0x00;

	/*Enable CS*/
	spi_xfer(rtc, 0, NULL, NULL, SPI_XFER_BEGIN);

	if( spi_xfer(rtc, 16, &dout[0], NULL, 0) != 0 )
	{
		printf("Error with SPI transfer 3.\n");
	}
	if( spi_xfer(rtc, 16, NULL, &din[0], 0) != 0 )
	{
		printf("Error with SPI transfer 4.\n");
	}

	/*Disable CS*/
	spi_xfer(rtc, 0, NULL, NULL, SPI_XFER_END);

	/*Print the day*/
	printf("Seconds: %d.\n", din[1]);

	/*Release bus and free up resources*/
	spi_release_bus(rtc);
	spi_free_slave(rtc);

    return (0);
}

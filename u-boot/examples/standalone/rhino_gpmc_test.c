/*
 * Simon Scott
 * University of Cape Town
 * February 2011
 *
 * Description: Tests the GPMC interface to the FPGA by reading and writing data.
 */

#include <common.h>
#include <exports.h>
#include <asm/io.h>

/* Base address of the 6 FPGA CS regions. Each region is 128MB */
#define FPGA_CS1_BASE		0x08000000
#define FPGA_CS2_BASE		0x10000000
#define FPGA_CS3_BASE		0x18000000
#define FPGA_CS4_BASE		0x20000000
#define FPGA_CS5_BASE		0x28000000
#define FPGA_CS6_BASE		0x38000000


int rhino_gpmc_test (int argc, char *argv[])
{
    unsigned long int address, i;
	unsigned short data_recvd;
	int num_errors = 0;

	app_startup(argv);
	printf ("\n== Rhino GPMC Test ==\n\n");

	printf("Reading data from FPGA...\n");


	/* Read 768MB from the FPGA, in sequential order, and ensure that (data) = (addr + 1) */
    for (address = FPGA_CS1_BASE; address < FPGA_CS6_BASE + 128*1024*1024; address += 2)
    {
		/* Skip over the NAND address space */
		if(address == 0x30000000)
			address = FPGA_CS6_BASE;

		if((address & 0x07FFFFFF) == 0x00000000)
			printf("Read test 1: testing next CS region....\n");

		data_recvd = readw(address);

		/* Check that the received data is correct */
		if(data_recvd != ((address + 1) & 0xFFFF))
			num_errors++;
	}

	/* Read 128MB from the FPGA, using 2 addresses with inverting bits,
	 * and ensure that (data) = (addr + 1) */

	printf("Read test 2: starting...\n");

	address = 0x10AAAAAA;

    for (i = 0; i < 64*1024*1024; i++)
    {
		data_recvd = readw(address);
		
		/* Check that the received data is correct */
		if(data_recvd != ((address + 1) & 0xFFFF))
			num_errors++;

		/*Now invert the bits of the lower 3 address bytes, using XOR operation*/
		address = address ^ 0x00FFFFFE;
	}

	printf("Finished reading from FPGA. Number of errors: %d\n", num_errors);

	printf("Writing data to FPGA to flash LEDs\n");

	/* Write a "walking" pattern of 1s to the FPGA, to flash the LEDs */
	writew(0x01, FPGA_CS2_BASE);
	udelay(500000);
	writew(0x02, FPGA_CS2_BASE);
	udelay(500000);
	writew(0x04, FPGA_CS2_BASE);
	udelay(500000);
	writew(0x08, FPGA_CS2_BASE);

	printf("GPMC test finished.\n");

    return (0);
}

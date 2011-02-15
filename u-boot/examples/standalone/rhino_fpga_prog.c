/*
 * Bruce Raw
 * University of Cape Town
 * January 2011
 */

#include <common.h>
#include <exports.h>
#include <asm/io.h>
#include <spi.h>
#define PROG_B 126
#define INIT_B 127
#define INIT_B_DIR 129
#define DONE 128

/*  RHINO U-Boot FPGA Programmer
 *  Usage instructions:
 *  Tftp(or loadb) the bin file you wish to program to memory address 0x80400000  	eg. tftp 0x80400000 rhino_blinky_core.bin
 *  Tftp(or loadb) rhino_fpga_prog.bin to 0x80300000					eg. tftp 0x80300000 rhino_fpga_prog.bin
 *  run program with: go 0x80300000 2151677952 4220212    (first number is 0x80400000 in decimal, second is byte size of a .bin file)
 */



int rhino_fpga_prog (int argc, char* argv[])
{
  
    	unsigned int  start, len,mem,i;

   	app_startup(argv);
   	printf ("Rhino FPGA Programmer\n");

	//Argument Check
    	if(argc < 3)
    	{
        	printf("Incorrect arguments.\nUsage: rhino_fpga_prog x y ,where x Is the input bin file starting address and y is the length\n");
        	return -1;
    	}
	
    	start = 0; //Starting mem Adress
    	len = 0;   //length of Bin file
    	int digit;
   	i = 0;

//simple str to int, coded like this cos functions suck.
        while (argv[1][i] != '\0')
        {
		start = start * 10;
                digit = argv[1][i] - '0';
                start = start + digit;
                i++;
		
        }
	i=0;
//again with the str to int	
        while (argv[2][i] != '\0')

        { 
 		len = len * 10;
                digit = argv[2][i] - '0';
                len = len + digit;
                i++;
        }

	printf("Setting INIT_B Direction\n");
	//Set INIT_B_DIR for INIT b as input
	omap_request_gpio(INIT_B_DIR);
	omap_set_gpio_direction(INIT_B_DIR, 0);  //set as output
	omap_set_gpio_dataout(INIT_B_DIR, 0);


	//Decalre Spi Variable.
	struct spi_slave *spi;
	printf("Starting Configure\n");

	


	
	
	//Pulse Prog_B---------------
	omap_request_gpio(PROG_B);
	omap_set_gpio_direction(PROG_B, 0);  //set as output
	omap_set_gpio_dataout(PROG_B, 1);
	omap_set_gpio_direction(PROG_B, 0);
        
	omap_set_gpio_dataout(PROG_B, 0);
	
        for (i=0; i<4000000;i++) //delay or wait until init_b goes low
	{
		omap_request_gpio(INIT_B);
		omap_set_gpio_direction(INIT_B, 1);//set as input
		if (!(omap_get_gpio_datain(INIT_B)))
		{
			//if init be is low, we can stop the delay.
			i = 4000000;
		}

        }
	omap_request_gpio(PROG_B);
	omap_set_gpio_dataout(PROG_B, 1);
	//PROG_B Pulsed..
	

	//Setup SPI
	printf("Starting SPI Setup\n");
	spi = spi_setup_slave(1,		//bus
			      0,		//CS
			      48000000,		//Max Hz
			      0			//Mode
			      );
	printf("SPI Setup Compelte\n");
	printf("Claiming SPI Bus\n");
	spi_claim_bus(spi);
	printf("Bus Claimed!\n");
        spi_xfer(spi, 0, NULL, NULL,SPI_XFER_BEGIN);//send spi transfer begin flags
        
	//WAIT for INIT_B to go high to signal FPGA ready for data to be clocked.
	omap_request_gpio(INIT_B);
	omap_set_gpio_direction(INIT_B, 1);//set as input
	printf("Waiting for INIT_B\n");
	while(!(omap_get_gpio_datain(INIT_B)))
	{}
	printf("INIT_B Ready, starting to program\n");
	

	





	printf("Sending %u  bytes from address %u  \n",len,start);

    	char c[2];
	int count = 0;
    	for (i=0; i<len; i = i+2)
    	{
		mem =start + i;
		c[0] = readb(mem);	//read byte 1
		c[1] = readb(mem+1);  	//read byte 2
		//Write to SPI	
		spi_xfer(spi, 16, c, NULL, 0);  //Transfer both bytes to the FPGA

		//To show progress		
		if (count>100000)
		{
			count =0;
			printf("Byte: %d  Out of: %d      \n",i,len);
		}
		else
		{			
			count++;
		}
		
    	}
	if(!(omap_get_gpio_datain(INIT_B)))  //if INIT_B is low we have CRC Error
		{
			printf("INIT_B went low, Error Programming FPGA\n");
			return(0);
		}
	omap_request_gpio(DONE);
	omap_set_gpio_direction(DONE, 1);//set as input
	if(omap_get_gpio_datain(DONE))  //Check Done pin, if high, programming is successful.
	{
		printf("\nProgramming Complete\n");
	}
	else
	printf("\nProgramming Failed \n");

	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);  //send spi transfer end flags
	spi_release_bus(spi);
  
	
	
    return (0);
}



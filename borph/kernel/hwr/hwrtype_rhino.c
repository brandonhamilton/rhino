/*********************************************************************
 * File  : kernel/hwr/hwrtype_rhino.c
 * Author: Brandon Hamilton
 * Date  : 18/02/2011
 * Description:
 *   Define hwrtype for RHINO board
 *		This file deals with the platform specific functions for
 *      communication with the FPGA.
 *********************************************************************/
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>    /* kmalloc/kfree */
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/ioport.h>  /* request_mem_region */
#include <linux/spi/spi.h>

#include <mach/gpio.h>

#include <asm/io.h>

#include <linux/bof.h>
#include <linux/borph.h>
#define HDEBUG
#define HDBG_NAME "hwrtype_rhino"
#define HDBG_LVL 9
#include <linux/hdebug.h>

/*
 * A static buffer for data transfer.  It should be expanded to a 
 * kmem_cache when higher performance is needed.  (Right now, there
 * can only be one ioreg performing I/O at a time.
 */
static buf_t* rhino_page;
static struct hwr_iobuf* iobuf;

struct rhino_fpga_device {
	struct spi_device *spi;
};

struct rhino_fpga_device *rhino_fpga;

/* Mutex Semaphore to ensure proper access to page buffer */
static DECLARE_MUTEX(rhino_mutex);

/* FPGA configuration pins definitions */
#define PROG_B     126
#define INIT_B     127
#define INIT_B_DIR 129
#define DONE       128

#define CFG_INITB_WAIT 4000000 //100000
#define CFG_DONE_WAIT  100000
                        

#define FPGA_CS1_BASE		0x08000000
#define FPGA_CS2_BASE		0x10000000
#define FPGA_CS3_BASE		0x18000000
#define FPGA_CS4_BASE		0x20000000
#define FPGA_CS5_BASE		0x28000000
#define FPGA_CS6_BASE		0x38000000

/*****************************************************************
 * functions definitions
 *****************************************************************/
static ssize_t rhino_send_iobuf (struct hwr_iobuf* iobuf)
{	
	int i;
	volatile int j;
	printk("Writing data to FPGA to flash LEDs\n");

	/* Write a "walking" pattern of 1s to the FPGA, to flash the LEDs */
	writew(0x01, FPGA_CS2_BASE);
	for (i=0; i < CFG_INITB_WAIT + 1; i++) {
			j = i;	
	}
	writew(0x02, FPGA_CS2_BASE);
	writew(0x04, FPGA_CS2_BASE);
	writew(0x08, FPGA_CS2_BASE);

	printk("GPMC test finished.\n");
	return iobuf->size;
}

static ssize_t rhino_recv_iobuf (struct hwr_iobuf* iobuf)
{
	
	return iobuf->size;
}

static struct hwr_iobuf* rhino_get_iobuf(void)
{
	if (down_interruptible(&rhino_mutex)) {
	    /* signal received, semaphore not acquired ... */
		return NULL;
	}
	iobuf->size = PAGE_SIZE - 12;
	return iobuf;
}

static ssize_t rhino_put_iobuf (struct hwr_iobuf* iobuf)
{
	up(&rhino_mutex);
	return 0;
}

static int rhino_configure(struct hwr_addr* addr, struct file* file, uint32_t offset, uint32_t len)
{
	/*
	 * Xilinx XAPP 502: Using a Microprocessor to Configure Xilinx FPGAs
	 *   via Slave Serial Mode.
	 *   _____________                _____________ 
	 *  |   Assert    |              |  Send bit   |
	 *  | NOT_PROGRAM |        ----->|  of Data    |<----
	 *  |_____________|        |     |_____________|    |
	 *         | > 300ns       |            |           | More
	 *   ______v______         |      ______v______     | Data
	 *  |   Deassert  |        |     |  Increment  |    |
	 *  | NOT_PROGRAM |        |     |   Address   |-----
	 *  |_____________|        |     |   Counter   | 
	 *         |<------------  |     |_____________|
	 *   ______v________ No |  |            |
	 *  |   Check for   |   |  |      ______v______
	 *  | NOT_INIT high |----  |     |  Check for  |  
	 *  |_______________|      |     |  DONE high  |
	 *         |__________Yes__|     |_____________|
	 */
	int i;
	int count;
	int retval = -EIO;
	u16* src;
	PDEBUG(9, "Configuring RHINO HWR %u from (offset %u, len %u) of %s\n", addr->addr, offset, len, file->f_dentry->d_name.name);

	if (addr->addr != 0) {
		PDEBUG(9, "Invalid FPGA #%d for RHINO\n", addr->addr); /* Rhino only has 1 HWR */
		goto out; 
	}

	if (down_interruptible(&rhino_mutex)) {
	    /* signal received, semaphore not acquired ... */
		goto out;
	}

	if (!rhino_fpga) {
		PDEBUG(9, "FPGA configuration error: Invalid SPI device\n");
		goto out;
	}
	/*************************************************
	 * Setup GPIO				                     *
	 *************************************************/
	if (gpio_request(INIT_B_DIR, "init_b_dir_gpio") != 0) {
		PDEBUG(9, "Could not request GPIO %d\n", INIT_B_DIR);
		retval = -ENODEV;
		goto out_free_mutex;
	}

	if (gpio_request(PROG_B, "prog_b_gpio") != 0) {
		PDEBUG(9, "Could not request GPIO %d\n", PROG_B);
		retval = -ENODEV;
		goto out_freegpio1;
	}

	if (gpio_request(INIT_B, "init_b_gpio") != 0) {
		PDEBUG(9, "Could not request GPIO %d\n", PROG_B);
		retval = -ENODEV;
		goto out_freegpio2;
	}

	if (gpio_request(DONE, "done_gpio") != 0) {
		PDEBUG(9, "Could not request GPIO %d\n", PROG_B);
		retval = -ENODEV;
		goto out_freegpio3;
	}

	gpio_direction_output(INIT_B_DIR, 0);

	/* Reset the FPGA */
	gpio_direction_output(PROG_B, 1);
	

	/*************************************************
	 * Clear Configuration Memory                    *
	 *************************************************/
	PDEBUG(9, "[1] Clear Configuration Memory\n");

	/* Start to clear configuration memory */
	gpio_set_value(PROG_B, 0);

	/* Wait for FPGA initialization */
	gpio_direction_input(INIT_B);
	for (i=0; i < CFG_INITB_WAIT + 1; i++) {
		if (!gpio_get_value(INIT_B)) {
			break;
		}
		if (i == CFG_INITB_WAIT) {
			PDEBUG(9, "Warning: INIT_B did not go low during PROG_B pulse\n");
		}
	}
					
	/* Clear configuration memory */
	gpio_set_value(PROG_B, 1);

	/*************************************************
	 * Bitstream Loading                             *
	 *************************************************/

	/* Wait for FPGA to be ready for configuration data */
	gpio_direction_input(INIT_B);
	for (i=0; i < CFG_INITB_WAIT + 1; i++) {
		if (gpio_get_value(INIT_B)) {
			break;
		}
		if (i == CFG_INITB_WAIT) {
			PDEBUG(9, "FPGA configuration error: Could not initialize FPGA for transfer\n");
			goto out_freegpio4;
		}
	}

	count = 0;
	while (len > 0) {
		count = min(PAGE_SIZE, len);
		retval = kernel_read(file, offset, rhino_page, count);
		if (retval < 0) {
			goto out_freegpio4;
		}
		if (retval != count) {
			PDEBUG(9, "kernel_read returns less than requested...\n");
			count = retval;
		}

		len -= count;
		offset += count;

		if (count % 2 != 0) {
			PDEBUG(9, "Warning: Odd number of bytes in FPGA config data...\n");
		}
 		
		src = (u16 *)(rhino_page);
		while(count > 0) {
			retval = spi_write(rhino_fpga->spi, src, sizeof(u16));
			if (retval) {
				PDEBUG(9, "FPGA configuration error: Configuration data write over SPI failed (%d)\n", retval);
				goto out_freegpio4;
			}
			src++;
			count -= sizeof(u16);
		}
	}
		
	/* CRC Check */
	if(!(gpio_get_value(INIT_B)))
	{
		PDEBUG(9, "FPGA configuration error: CRC check failed\n");
		goto out_freegpio4;
	}

	/*************************************************
	 * Startup sequence                              *
	 *************************************************/
	gpio_direction_input(DONE);
	for (i=0; i <= CFG_DONE_WAIT; i++) {
		if(gpio_get_value(DONE)) {
			break;
		}
		else {
			PDEBUG(9, "FPGA configuration error: Error in startup sequence, DONE pin not asserted\n");
			goto out_freegpio4;
    	}
    }

	PDEBUG(9, "RHINO HWR %u configuration completed successfully\n", addr->addr);
	retval = 0;

out_freegpio4:
	gpio_free(DONE);
out_freegpio3:
	gpio_free(INIT_B);
out_freegpio2:
	gpio_free(PROG_B);
out_freegpio1:
	gpio_free(INIT_B_DIR);
out_free_mutex:
	up(&rhino_mutex);
out:
	return retval;	
}

static int rhino_unconfigure(struct hwr_addr* addr)
{
	PDEBUG(9, "Unconfiguring RHINO HWR %u\n", addr->addr);
	return 0;
}

struct phyhwr* rhino_reserve_hwr(struct hwr_addr* a)
{
	struct phyhwr* ret;
	if (a && a->class != HAC_RHINO) {
                return NULL;
        }
	
	/* safety check */
	if (a->addr >= 1)
		return NULL;

	ret = phyhwrs[a->class][a->addr];
	if (!atomic_inc_and_test(&ret->count)) {
		atomic_dec(&ret->count);
		/* was being used */
		return NULL;
	}
	/* count is now a usage count */
	atomic_inc(&ret->count);
	return ret;
}

void rhino_release_hwr(struct hwr_addr* a)
{
	struct phyhwr* hwr;
	if (a && a->class != HAC_RHINO)
		return;

	/* safety check */
	if (a->addr >= 1)
		return;
	
	hwr = phyhwrs[a->class][a->addr];
	if (atomic_dec_and_test(&hwr->count)) {
		hwr->task = NULL;
		atomic_set(&hwr->count, -1);
	}
}

static struct hwr_operations rhino_hwr_operations = {
	.configure = rhino_configure,
	.unconfigure = rhino_unconfigure,
	.reserve_hwr = rhino_reserve_hwr,
	.release_hwr = rhino_release_hwr,
	.get_iobuf   = rhino_get_iobuf,
	.put_iobuf   = rhino_put_iobuf,
	.send_iobuf  = rhino_send_iobuf,
	.recv_iobuf  = rhino_recv_iobuf
};

static struct hwrtype hwrtype_rhino = {
	name: "rhino",
	type: HAC_RHINO,
	count: ATOMIC_INIT(0),
    num_devs: 1,
	hwr_ops: &rhino_hwr_operations,
};

static int __devinit fpga_probe(struct spi_device *spi)
{
	int retval = 0;
	printk("RHINO Spartan-6 FPGA interface driver");

	/*
	spi->mode = SPI_MODE_0;
	spi->max_speed_hz = 48000000;
	spi->bits_per_word = 8;
	retval = spi_setup(spi);
	if (retval < 0)
		return retval;*/

	rhino_fpga = kzalloc(sizeof(struct rhino_fpga_device), GFP_KERNEL);
	if (rhino_fpga == NULL) {
		dev_err(&spi->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	rhino_fpga->spi = spi;
	spi_set_drvdata(spi, rhino_fpga);

	return retval;
}

static int __devexit fpga_remove(struct spi_device *spi)
{
	return 0;
}


static struct spi_driver rhino_spartan6_driver = {
	.driver = {
		.name	= "rhino-spartan6",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.probe	= fpga_probe,
	.remove = __devexit_p(fpga_remove),
};


static int __init hwrtype_rhino_init(void)
{
	int retval = 0;
	rhino_fpga = 0;

	if ((retval = register_hwrtype(&hwrtype_rhino)) < 0) {
		printk("Error registering RHINO HWR\n");
		goto out;
	} else {
		printk("Registered RHINO HWR\n");
	}

	/* initialize hwr */
    atomic_set(&(phyhwrs[HAC_RHINO][0])->count, -1);
	      	
	/* initialize iobuf memory */
	rhino_page = (buf_t*)__get_free_page(GFP_KERNEL);
	iobuf = (struct hwr_iobuf*) kmalloc(sizeof(struct hwr_iobuf), GFP_KERNEL);
	if (iobuf) {
		iobuf->data = rhino_page + 12;
		iobuf->size = PAGE_SIZE - 12;
	} else {
		printk("failed getting memory for RHINO iobuf\n");
		retval = -ENOMEM;
		goto out;
	}

	spi_register_driver(&rhino_spartan6_driver);


out:
	return retval;
}

static void __exit hwrtype_rhino_exit(void)
{
	/* Free iobuf */
	if (iobuf) {
		kfree(iobuf);
	}

	if (rhino_page) {
		free_page((unsigned long) rhino_page);
	}

	spi_unregister_driver(&rhino_spartan6_driver);

	if (unregister_hwrtype(&hwrtype_rhino)) {
		printk("Error unregistering RHINO HWR\n");
	} else {
		printk("Unregistered RHINO HWR\n");
	}
}

module_init(hwrtype_rhino_init);
module_exit(hwrtype_rhino_exit);

MODULE_AUTHOR("Brandon Hamilton");
MODULE_DESCRIPTION("RHINO hardware region support");
MODULE_LICENSE("GPL");

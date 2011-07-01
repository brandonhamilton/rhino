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

/* Buffers used for asynchronous configuration data transfers */
struct config_transfer_part {
	buf_t* page;
	struct spi_transfer	transfer;
	struct spi_message	message;
	int number;
	char last;
	
};
static atomic_t config_transfer_parts;
struct kmem_cache *config_transport_part_cache;

/* Bus mapping for GPMC */
typedef struct fpga_map {
	resource_size_t cs0_base;
	void __iomem *cs0_virtual;
	resource_size_t cs1_base;
	void __iomem *cs1_virtual;
	resource_size_t cs2_base;
	void __iomem *cs2_virtual;
	resource_size_t cs3_base;
	void __iomem *cs3_virtual;
	resource_size_t cs4_base;
	void __iomem *cs4_virtual;
	resource_size_t cs5_base;
	void __iomem *cs5_virtual;
} rhino_fpga_map_t;

static rhino_fpga_map_t *rhino_fpga_bus;

struct rhino_fpga_device *rhino_fpga;

DECLARE_COMPLETION(configuration_data_sent);

/* Mutex Semaphore to ensure proper access to page buffer */
static DECLARE_MUTEX(rhino_mutex);

/* FPGA configuration pins definitions */
#define PROG_B     126
#define INIT_B     127
#define INIT_B_DIR 129
#define DONE       128

#define CFG_INITB_WAIT 4000000 //100000
#define CFG_DONE_WAIT  100000
                        
#define FPGA_VCCINT		99
#define FPGA_VCCO_AUX	100
#define FPGA_VCCMGT		101

#define FPGA_CS0_BASE		0x08000000
#define FPGA_CS1_BASE		0x10000000
#define FPGA_CS2_BASE		0x18000000
#define FPGA_CS3_BASE		0x20000000
#define FPGA_CS4_BASE		0x28000000
#define FPGA_CS5_BASE		0x38000000

static void rhino_set_fpga_psu(int enable)
{
	/* Enable power to FPGA */
    gpio_direction_output(FPGA_VCCINT, enable);
    gpio_direction_output(FPGA_VCCO_AUX, enable);
    gpio_direction_output(FPGA_VCCMGT, enable);
}
/*****************************************************************
 * functions definitions
 *****************************************************************/

static unsigned int get_mapped_location(unsigned int reg_loc) {
	unsigned int location = reg_loc;
	if (reg_loc >= FPGA_CS0_BASE && reg_loc < FPGA_CS0_BASE + SZ_128M) {
		location = rhino_fpga_bus->cs0_virtual + (reg_loc - FPGA_CS0_BASE);
	} else if (reg_loc >= FPGA_CS1_BASE && reg_loc < FPGA_CS1_BASE + SZ_128M) {
		location = rhino_fpga_bus->cs1_virtual + (reg_loc - FPGA_CS1_BASE);
	} else if (reg_loc >= FPGA_CS2_BASE && reg_loc < FPGA_CS2_BASE + SZ_128M) {
		location = rhino_fpga_bus->cs2_virtual + (reg_loc - FPGA_CS2_BASE);
	} else if (reg_loc >= FPGA_CS3_BASE && reg_loc < FPGA_CS3_BASE + SZ_128M) {
		location = rhino_fpga_bus->cs3_virtual + (reg_loc - FPGA_CS3_BASE);
	} else if (reg_loc >= FPGA_CS4_BASE && reg_loc < FPGA_CS4_BASE + SZ_128M) {
		location = rhino_fpga_bus->cs4_virtual + (reg_loc - FPGA_CS4_BASE);
	} else if (reg_loc >= FPGA_CS5_BASE && reg_loc < FPGA_CS5_BASE + SZ_128M) {
		location = rhino_fpga_bus->cs5_virtual + (reg_loc - FPGA_CS5_BASE);
	}
	return location;
}

static ssize_t rhino_send_iobuf (struct hwr_iobuf* iobuf)
{	
	int count;
	unsigned short *src;
	unsigned int dst;
	int i;

	count = iobuf->size / sizeof(unsigned short);
	src = (unsigned short *)iobuf->data;	
	dst = get_mapped_location(iobuf->location) + iobuf->offset;	

	PDEBUG(9, "Writing data to RHINO FPGA Register (%d bytes) at location 0x%x\n", iobuf->size, dst);
	/* Write 16-bit words to the GPMC bus */
	for (i = 0; i < count; i++) {
		__raw_writew(*src, dst);
		dst += sizeof(unsigned short);
		src++; 
	}

	return (count * sizeof(unsigned short));
}

static ssize_t rhino_recv_iobuf (struct hwr_iobuf* iobuf)
{
	int count;
	unsigned short *dst;
	unsigned int src;
	int i;

	count = iobuf->size / sizeof(unsigned short);
	dst = (unsigned short *)iobuf->data;	
	src = get_mapped_location(iobuf->location) + iobuf->offset;

	PDEBUG(9, "Reading data from RHINO FPGA register (%d bytes) at location 0x%x\n", iobuf->size,  src);
	/* Read 16-bit words from the GPMC bus */
	for (i = 0; i < count; i++){
		*dst = __raw_readw(src);
		src += sizeof(unsigned short);
		dst++; 
	}

	return (count * sizeof(unsigned short));
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

static void spi_transfer_completed(void *buf) 
{
	struct config_transfer_part *ctp = buf;
	int status = ctp->message.status;
	free_page(ctp->page);
	kmem_cache_free(config_transport_part_cache, ctp);
	if (status == 0) {
		/* Transfer succeeded */
		atomic_dec(&config_transfer_parts);	
	}
	else {
		/* Transfer failed */
		PDEBUG(9, "SPI transfer failed with status: %d...\n", status);
		atomic_set(&config_transfer_parts, -1);
	}

	/* Indicate to configuration function that SPI transfers are done */
	if (ctp->last || status) {
		complete(&configuration_data_sent);
	}
}

static void setup_config_transport_part(struct config_transfer_part *ctp, int len)
{
	/* Setup SPI transfer request */
	memset(&ctp->transfer, 0, sizeof (struct spi_transfer));
	ctp->transfer.tx_buf = ctp->page;
	ctp->transfer.len = len;
	/* Setup SPI message */
	spi_message_init(&ctp->message);
	spi_message_add_tail(&ctp->transfer, &ctp->message);
	/* Use asynchronous callback method */
	ctp->message.complete = spi_transfer_completed;
	ctp->message.context  = ctp;
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
	int number;
	PDEBUG(9, "Configuring RHINO HWR %u from (offset %u, len %u) of %s\n", addr->addr, offset, len, file->f_dentry->d_name.name);

	if (addr->addr != 0) {
		PDEBUG(9, "Invalid FPGA #%d for RHINO\n", addr->addr); /* Rhino only has 1 HWR */
		goto out; 
	}

	if (!rhino_fpga) {
		PDEBUG(9, "FPGA configuration error: Invalid SPI device\n");
		goto out;
	}

	if (down_interruptible(&rhino_mutex)) {
	    /* signal received, semaphore not acquired ... */
		goto out;
	}

	/*************************************************
	 * Initialize the FPGA		                     *
	 *************************************************/
	/* Turn on FPGA */
	rhino_set_fpga_psu(1);

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

	PDEBUG(9, "[2] Loading Bitstream\n");
	/* Wait for FPGA to be ready for configuration data */
	gpio_direction_input(INIT_B);
	for (i=0; i < CFG_INITB_WAIT + 1; i++) {
		if (gpio_get_value(INIT_B)) {
			break;
		}
		if (i == CFG_INITB_WAIT) {
			PDEBUG(9, "FPGA configuration error: Could not initialize FPGA for transfer\n");
			goto out_free_mutex;
		}
	}

	/* Transfer configuration data to FPGA using SPI (DMA) */
	atomic_set(&config_transfer_parts, 0);
	count = 0;
	number = 0;
	while (len > 0) {
		/* Setup buffer to hold segment of configuration data */
		struct config_transfer_part* ctp = kmem_cache_alloc(config_transport_part_cache, GFP_KERNEL);
		ctp->page = (buf_t*)__get_free_page(GFP_KERNEL | GFP_DMA);
		ctp->number = ++number;
		/* Read configuration data into buffer */
		count = min(PAGE_SIZE, len);
		retval = kernel_read(file, offset, ctp->page, count);
		if (retval < 0) {
			free_page(ctp->page);
			kmem_cache_free(config_transport_part_cache, ctp);
			goto out_free_mutex;
		}
		if (retval != count) {
			PDEBUG(9, "kernel_read returns less than requested...\n");
			count = retval;
		}

		len -= count;
		offset += count;

		if (atomic_inc_and_test( &config_transfer_parts )) {
			PDEBUG(9, "FPGA configuration error: Configuration data write over SPI failed (%d)\n", retval);
			goto out_free_mutex;
		}

		setup_config_transport_part(ctp, count);
		ctp->last = count < PAGE_SIZE;

		/* Send configuration data over SPI using DMA */
		spi_async(rhino_fpga->spi, &ctp->message);
	}

	/* Wait for signal that all configuration data has been sent */
	wait_for_completion(&configuration_data_sent);

	PDEBUG(9, "[3] CRC Check\n");
	/* CRC Check */
	if(!(gpio_get_value(INIT_B)))
	{
		PDEBUG(9, "FPGA configuration error: CRC check failed\n");
		goto out_free_mutex;
	}

	/*************************************************
	 * Startup sequence                              *
	 *************************************************/
	PDEBUG(9, "[4] Checking configuration results\n");
	gpio_direction_input(DONE);
	for (i=0; i <= CFG_DONE_WAIT; i++) {
		if(gpio_get_value(DONE)) {
			break;
		}
		else {
			PDEBUG(9, "FPGA configuration error: Error in startup sequence, DONE pin not asserted\n");
			goto out_free_mutex;
    	}
    }

	PDEBUG(9, "RHINO HWR %u configuration completed successfully\n", addr->addr);
	retval = 0;

out_free_mutex:
	up(&rhino_mutex);
out:
	return retval;	
}

static int rhino_unconfigure(struct hwr_addr* addr)
{
	rhino_set_fpga_psu(0);
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

	spi->mode = SPI_MODE_0;
	spi->max_speed_hz = 48000000;
	retval = spi_setup(spi);
	if (retval < 0)
		return retval;

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

static void hwrtype_rhino_cleanup(void) {

	/* Release GPMC bus mappings */
	if (rhino_fpga_bus) {
#define release_rhino_gmpc(p, q) \
	if(rhino_fpga_bus->p##_virtual){ \
		iounmap(rhino_fpga_bus->p##_virtual); \
		rhino_fpga_bus->p##_virtual = NULL; \
	} \
	if(rhino_fpga_bus->p##_base){ \
		release_mem_region(rhino_fpga_bus->p##_base, SZ_128M); \
		rhino_fpga_bus->p##_base = 0; \
	}

	release_rhino_gmpc(cs0, CS0)
	release_rhino_gmpc(cs1, CS1)
	release_rhino_gmpc(cs2, CS2)
	//release_rhino_gmpc(cs3, CS3)
	//release_rhino_gmpc(cs4, CS4)
	//release_rhino_gmpc(cs5, CS5)
#undef release_rhino_gmpc

		kfree(rhino_fpga_bus);
		rhino_fpga_bus = NULL;
	}

	/* Free iobuf */
	if (iobuf) {
		kfree(iobuf);
		iobuf = NULL;
	}

	if (rhino_page) {
		free_page((unsigned long) rhino_page);
		rhino_page = NULL;
	}

	if (config_transport_part_cache) {
		kmem_cache_destroy(config_transport_part_cache);
		config_transport_part_cache = NULL;
	}

	spi_unregister_driver(&rhino_spartan6_driver);

	if (unregister_hwrtype(&hwrtype_rhino)) {
		printk("Error unregistering RHINO HWR\n");
	} else {
		printk("Unregistered RHINO HWR\n");
	}
}

static int __init hwrtype_rhino_init(void) {
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
		printk("Failed getting memory for RHINO iobuf\n");
		retval = -ENOMEM;
		goto out;
	}

	/* Map GPMC memory regions */
	rhino_fpga_bus = kmalloc(sizeof(rhino_fpga_map_t), GFP_KERNEL);
	if (rhino_fpga_bus == NULL) {
		printk("Failed getting memory for RHINO FPGA bus mapping\n");
		free_page((unsigned long) rhino_page);
		rhino_page = 0;
		kfree(iobuf);
		iobuf = 0;
		retval = -ENOMEM;
		goto out;	
	}

	rhino_fpga_bus->cs0_base = 0;
	rhino_fpga_bus->cs0_virtual = NULL;
	rhino_fpga_bus->cs1_base = 0;
	rhino_fpga_bus->cs1_virtual = NULL;
	rhino_fpga_bus->cs2_base = 0;
	rhino_fpga_bus->cs2_virtual = NULL;
	rhino_fpga_bus->cs3_base = 0;
	rhino_fpga_bus->cs3_virtual = NULL;
	rhino_fpga_bus->cs4_base = 0;
	rhino_fpga_bus->cs4_virtual = NULL;
	rhino_fpga_bus->cs5_base = 0;
	rhino_fpga_bus->cs5_virtual = NULL;

#define map_rhino_gmpc(p, q)  \
	if(!request_mem_region(FPGA_##q##_BASE, SZ_128M, "rhino-fpga-" #p)){ \
		printk("Unable to request memory region 0x%08x for GPMC\n", FPGA_##q##_BASE); \
		hwrtype_rhino_cleanup(); \
		return -ENOMEM; \
	} \
	rhino_fpga_bus->p##_base = FPGA_##q##_BASE; \
	rhino_fpga_bus->p##_virtual = ioremap(FPGA_##q##_BASE, SZ_128M); \
	if(!rhino_fpga_bus->p##_virtual){ \
		printk("Unable to map memory region 0x%08x for GPMC\n", FPGA_##q##_BASE); \
		hwrtype_rhino_cleanup(); \
		return -ENOMEM; \
	} \
	PDEBUG(9, "RHINO FPGA: " #p " GPMC at 0x%08x:0x%08x mapped to 0x%08x\n", FPGA_##q##_BASE, FPGA_##q##_BASE + SZ_128M, rhino_fpga_bus->p##_virtual); 

	map_rhino_gmpc(cs0, CS0)
	map_rhino_gmpc(cs1, CS1)
	map_rhino_gmpc(cs2, CS2)
	//map_rhino_gmpc(cs3, CS3)
	//map_rhino_gmpc(cs4, CS4)
	//map_rhino_gmpc(cs5, CS5)

#undef map_rhino_gmpc

	config_transport_part_cache = KMEM_CACHE(config_transfer_part, SLAB_HWCACHE_ALIGN);

	spi_register_driver(&rhino_spartan6_driver);

	/* request FPGA gpio */
	if (gpio_request(FPGA_VCCINT, "FPGA_VCCINT") != 0) {
		PDEBUG(9, "Could not request GPIO %d\n", FPGA_VCCINT);
	}
	if (gpio_request(FPGA_VCCO_AUX, "FPGA_VCCO_AUX") != 0) {
		PDEBUG(9, "Could not request GPIO %d\n", FPGA_VCCO_AUX);
	}
    if (gpio_request(FPGA_VCCMGT, "FPGA_VCCMGT") != 0) {
		PDEBUG(9, "Could not request GPIO %d\n", FPGA_VCCMGT);
	}

    if (gpio_request(INIT_B_DIR, "init_b_dir_gpio") != 0) {
		PDEBUG(9, "Could not request GPIO %d\n", INIT_B_DIR);
	}

	if (gpio_request(PROG_B, "prog_b_gpio") != 0) {
		PDEBUG(9, "Could not request GPIO %d\n", PROG_B);
	}

	if (gpio_request(INIT_B, "init_b_gpio") != 0) {
		PDEBUG(9, "Could not request GPIO %d\n", INIT_B);
	}

	if (gpio_request(DONE, "done_gpio") != 0) {
		PDEBUG(9, "Could not request GPIO %d\n", DONE);
	}

	gpio_direction_output(INIT_B_DIR, 0);

out:
	return retval;
}

static void __exit hwrtype_rhino_exit(void)
{
	hwrtype_rhino_cleanup();
}

module_init(hwrtype_rhino_init);
module_exit(hwrtype_rhino_exit);

MODULE_AUTHOR("Brandon Hamilton");
MODULE_DESCRIPTION("RHINO hardware region support");
MODULE_LICENSE("GPL");

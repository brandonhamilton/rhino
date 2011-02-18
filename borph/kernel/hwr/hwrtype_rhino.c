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
#include <asm/uaccess.h>   /* copy_from_user */
#include <linux/ioport.h>  /* request_mem_region */

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

/* Mutex Semaphore to ensure proper access to page buffer */
static DECLARE_MUTEX(rhino_mutex);

/*****************************************************************
 * functions definitions
 *****************************************************************/
static ssize_t rhino_send_iobuf (struct hwr_iobuf* iobuf)
{	
	return iobuf->size;
}

static ssize_t rhino_recv_iobuf (struct hwr_iobuf* iobuf)
{
	
	return iobuf->size;
}

static struct hwr_iobuf* rhino_get_iobuf(void)
{
    PDEBUG(9, "Locking IOBUF\n");

	/* If there is different buffer for differ hwr, should
	 * deferentiate them here using *reg* */

	if (down_interruptible(&rhino_mutex)) {
	        /* signal received, semaphore not acquired ... */
		return NULL;
	}

	iobuf->size = PAGE_SIZE - 12;
	return iobuf; //HHH
}

static ssize_t rhino_put_iobuf (struct hwr_iobuf* iobuf)
{
	PDEBUG(9, "Unlocking IOBUF\n");
	up(&rhino_mutex);
	return 0; //HHH
}

static int rhino_configure(struct hwr_addr* addr, struct file* file, uint32_t offset, uint32_t len)
{
	int retval = -EIO;

	PDEBUG(9, "Configuring RHINO HWR %u from (offset %u, len %u) of %s\n", addr->addr, offset, len, file->f_dentry->d_name.name);

	if (addr->addr != 0) {
		PDEBUG(9, "Invalid FPGA #%d for RHINO\n", addr->addr); /* Rhino only has 1 HWR */
		goto out; 
	}

	if (down_interruptible(&rhino_mutex)) {
	        /* signal received, semaphore not acquired ... */
		goto out;
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

static int __init hwrtype_rhino_init(void)
{
	int retval = 0;
	int i;

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

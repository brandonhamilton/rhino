/*********************************************************************
 * $Id: bkexecd.c,v 1.1 2006/10/31 07:28:57 skhay Exp $
 * File  : bkexecd.c
 * Author: Hayden Kwok-Hay So, Brandon Hamilton
 * Date  : 12/09/2005
 * Modified: 18/02/2011
 * Description:
 *   This is the main code for bkexecd. It is responsible for
 * configuring and de-configure FPGA as part of a BORPH process
 *
 *  bkexecd is responsible for all communication to "user
 * fpga" via the platform specific code in hwr/ directory.
 * A FPGA is configured during exec of
 * a bof file.  Therefore. bkexecd simply sleep upon start up and wait
 * forever until a bof file is exec-ed and binfmt_bof wakes it up.
 *********************************************************************/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/borph.h>
#include <linux/bof.h>
#include <linux/init.h>
#include <linux/highmem.h>
#include <linux/binfmts.h>
#include <linux/gfp.h>

#define HAC_TYPE HAC_RHINO

#define HDEBUG
#define HDBG_NAME "bkexecd"
#define HDBG_LVL 6
#include <linux/hdebug.h>

struct bkexecd_info bked_info;
static pid_t bked_pid;

static size_t get_args(char* buf, struct linux_binprm* bprm, size_t size) {
	char* bptr = buf;
	char* kaddr;
	size_t ret = 0;
	unsigned long offset, len, p;
	struct page* page;
	int argc;

	if (!buf || !bprm || !size) return -EINVAL;

	argc = bprm->argc;
	p = bprm->p;

	while (argc && (ret < size)) {
		offset = p % PAGE_SIZE;
		len = PAGE_SIZE - offset;

#ifdef CONFIG_MMU
		get_user_pages(current, bprm->mm, p, 1, 0, 1, &page, NULL);
#else
		page = bprm->page[p / PAGE_SIZE];
#endif
		kaddr = kmap(page) + offset;
		
		if (ret + len > size) {
			len = size - ret;
		}

		while (len-- && argc) {
			char c;
			*bptr++ = c = *kaddr++;
			if (!c) {
				argc -= 1;
			}
			p += 1;
			ret += 1;
		}
		kunmap(page);
	}

	return ret;
}

static inline struct phyhwr* bpr_nextfree(struct hwr_addr* a)
{
	int i;
	struct phyhwr* retval;

	if (!a || a->class != HAC_TYPE) return NULL;
	/* Make sure it never checks control FPGA */
	
	for (i = 0; i < 4; i++) {
		a->addr = i;
		retval = reserve_hwr(a);
		if (retval) {
			return retval;
		}
	}
	return NULL;
}

static inline int borph_load_hw(struct execq_item *execq_item)
{
	struct borph_info* bi;
	struct borph_hw_region* region;
	struct bofhdr bofhdr;
	struct hwr_operations* hwrops;
	uint32_t cur_foff;
	int retval;
	struct hwrhdr hwrhdr;
	char task_id[10];

	PDEBUG(9, "configure from file: %s\n", execq_item->bprm->filename);

	// SHOULD USE SLAB!!!
	bi = kmalloc(sizeof(struct borph_info), GFP_KERNEL);
	memset(bi, 0, sizeof(struct borph_info));
	INIT_LIST_HEAD(&bi->hw_region);
	INIT_LIST_HEAD(&bi->ioreg);
	
	// Set binary as default mode
	bi->ioreg_mode = 1;

	execq_item->task->borph_info = bi;

	retval = -EIO;
        bofhdr = *((struct bofhdr *) execq_item->bprm->buf);

	PDEBUG(9, "version=%d, numchip=%d, elf_off=0x%x, hw_off=0x%x, ekver=0x%x\n",
	       bofhdr.b_version, bofhdr.b_numchip, bofhdr.b_elfoff, 
	       bofhdr.b_hwoff, bofhdr.b_ekver);
	cur_foff = bofhdr.b_hwoff;
	while (bofhdr.b_numchip--) {
		struct phyhwr* hwr;
		char* strtab = NULL;
		uint32_t strtab_len = 0;
		uint32_t strtab_foff;

		if (cur_foff == 0) {
			PDEBUG(9, "invalid bof file\n");
			goto out_delregion;
		}
		retval = kernel_read(execq_item->bprm->file, cur_foff, 
				     (char*) &hwrhdr, sizeof(struct hwrhdr));
		if (retval < 0) {
			goto out_delregion;
		}
		cur_foff += sizeof(struct hwrhdr);

		PDEBUG(9, "hwrhdr: flag=0x%x, pl_off=0x%x, pl_len=0x%x," 
		       " next_hwr=0x%x, strtab_off=0x%x, nr_symbol=%d\n", 
		       hwrhdr.flag, hwrhdr.pl_off, hwrhdr.pl_len, 
		       hwrhdr.next_hwr, hwrhdr.strtab_off, hwrhdr.nr_symbol);

		retval = -EBUSY;
		if (hwrhdr.flag & HFG_PLACED) {
			hwr = reserve_hwr(&hwrhdr.addr);

			if (!hwr ){
				goto out_delregion;
			}
		} else {
			// need P&R
			hwr = bpr_nextfree(&hwrhdr.addr);
			if (!hwr) {
				goto out_delregion;
			}
		}
		hwr->task = execq_item->task;
		
		PDEBUG(5, "Configuring FPGA %d\n", hwrhdr.addr.addr);
		/* strtab */
		retval = -ENOEXEC;
		strtab_foff = 
			cur_foff + hwrhdr.nr_symbol * sizeof(struct bofioreg);
		strtab_len = hwrhdr.pl_off - hwrhdr.strtab_off;
		if (!strtab_len && hwrhdr.nr_symbol) {
			PDEBUG(0, "0 length string table\n");
			goto out_delregion;
		}
		strtab = kmalloc(strtab_len, GFP_KERNEL);
		if (!strtab) {
		    PDEBUG(9, "kmalloc strtab failed\n");
		    goto out_delregion;
		}
		retval = kernel_read(execq_item->bprm->file, strtab_foff,
				     (char*) strtab, strtab_len);
		if (retval < 0) goto out_delregion;

		/* region */
		// should use slab
		region = kmalloc(sizeof(struct borph_hw_region), GFP_KERNEL);
		if (!region) {
			PDEBUG(9, "kmalloc region failed\n");
		} else {
			PDEBUG(9, "got region\n");
		}
		region->addr = hwrhdr.addr;
		region->strtab = strtab;
		list_add(&(region->list), &(bi->hw_region));

		/* ioreg */
		PDEBUG(9, "Loading %d IOREG\n", hwrhdr.nr_symbol);
		while (hwrhdr.nr_symbol--) {
			struct borph_ioreg *reg;
			struct bofioreg bof_ioreg;

			// should use slab
			reg = kmalloc(sizeof(struct borph_ioreg), GFP_KERNEL);
			if (!reg) {
				PDEBUG(9, "kmalloc borph_ioreg failed\n");
			}
			retval = kernel_read(execq_item->bprm->file, cur_foff,
					     (char*) &bof_ioreg, 
					     sizeof(struct bofioreg));
			if (retval < 0) {
				goto out_delregion;
			}
			cur_foff += sizeof(struct bofioreg);
		    
			reg->name = strtab + bof_ioreg.name;
			reg->mode = bof_ioreg.mode;
			reg->loc = bof_ioreg.loc;
			reg->len = bof_ioreg.len;
			reg->hwraddr = &region->addr;
			list_add(&(reg->list), &(bi->ioreg));
		}

		hwrops = get_hwrops(&hwrhdr.addr);
		if (hwrops && hwrops->configure) {
			retval = hwrops->configure(&(hwrhdr.addr), 
							   execq_item->bprm->file, 
							   hwrhdr.pl_off,
							   hwrhdr.pl_len);

		} else {
			printk(KERN_INFO "no hwrops from hwr class %d\n",
			       hwrhdr.addr.class);
			retval = -ENOEXEC;
			goto out_delregion;
		}
		put_hwrops(&hwrhdr.addr);
		if (retval < 0) {
			goto out_delregion;
		}
		/* done with all the configuration for this chip
		 * tell the rest of the kernel about it */
		hwr_activate(&(hwrhdr.addr));
		/* prepare for next hardware region */
		cur_foff = hwrhdr.next_hwr;
	}
	//done
	PDEBUG(5, "FPGA configuration completed\n");
	return 0;
 out_delregion:
	borph_exit_fpga(execq_item->task);
	return retval;
}

/* program FPGA according to execq_item until no more item on
   execq_list */
static inline void run_execq(void)
{
	unsigned long flags;
	int retval;

	spin_lock_irqsave(&bked_info.execq_lock, flags);
	while (!list_empty(&bked_info.execq_list)) {
		struct execq_item* execq_item;
		struct bofhdr* bhdr;
		execq_item = list_entry(bked_info.execq_list.next, 
					struct execq_item, list);
		list_del_init(bked_info.execq_list.next);
		spin_unlock_irqrestore(&bked_info.execq_lock, flags);

		/***** actual fpga configuration *****/
		retval = borph_load_hw(execq_item);
		bhdr = (struct bofhdr*) (execq_item->bprm->buf);
		bhdr->load_err = retval;

		wake_up_process(execq_item->task);
		/***** end fpga configuration *****/
		kfree(execq_item);  // HHH return to slab

		// get lock in preparation for list_empty check
		spin_lock_irqsave(&bked_info.execq_lock, flags);
		wake_up_interruptible(&bked_info.exec_done);
	}
	spin_unlock_irqrestore(&bked_info.execq_lock, flags);
}

static int bkexecd(void *dummy)
{
	DECLARE_WAITQUEUE(wait, current);

	/* initialize bked_info */
	INIT_LIST_HEAD(&bked_info.execq_list);
	spin_lock_init(&bked_info.execq_lock);
	init_waitqueue_head(&bked_info.more_exec);
	init_waitqueue_head(&bked_info.exec_done);

	/* detach myself from calling process (e.g. insmod) */
	daemonize("bkexecd");
	
	/* my info */
	sprintf(current->comm, "bkexecd");

	/* Block all signals except SIGKILL and SIGSTOP */
	spin_lock_irq(&current->sighand->siglock);
	siginitsetinv(&current->blocked, sigmask(SIGKILL) | sigmask(SIGSTOP) );
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);


	// loop forever until we have any signal (SIGKILL | SIGSTOP)
	for(;;) {
		set_current_state(TASK_INTERRUPTIBLE);
		add_wait_queue(&bked_info.more_exec, &wait);
		if (list_empty(&bked_info.execq_list)) {
			schedule();
		}
		__set_current_state(TASK_RUNNING);
		remove_wait_queue(&bked_info.more_exec, &wait);

		if (signal_pending(current)) {
			// we got SIGKILL | SIGSTOP
			break;
		}

		if (!list_empty(&bked_info.execq_list)) {
			run_execq();
		}
	}
	PDEBUG(9, "Shutting down BORPH execution thread\n");
	return 0;
}

static __init int bkexecd_init(void)
{
	bked_pid = kernel_thread(bkexecd, NULL, CLONE_FS | CLONE_FILES | CLONE_SIGHAND);
	return 0;
}

static __exit void bkexecd_exit(void) {
//	kill_proc(bked_pid, SIGKILL, 0);
}

EXPORT_SYMBOL(bked_info);

module_init(bkexecd_init);
module_exit(bkexecd_exit);
MODULE_LICENSE("GPL");

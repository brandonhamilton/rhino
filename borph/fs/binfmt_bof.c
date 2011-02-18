/*********************************************************************
 * fs/binfmt_bof.c
 * Author: Hayden Kwok-Hay So, Brandon Hamilton
 *
 * Description:
 *   Thie file describes a new binary format to be loaded by a Linux
 * kernel, the BORPH Object File (BOF) format.  A file in BOF format
 * encapsulates both an ELF image and a configuration information for
 * one or more FPGA.
 *   The actual FPGA configuration is handled by bkexecd
 *
 * Ported to kernel 2.6: 2008/11/03
 *********************************************************************/
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/binfmts.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/elf.h>
#include <linux/fs.h>

#include <linux/bof.h>
#include <linux/borph.h>
#define HDEBUG
#define HDBG_NAME "binfmt_bof"
#define HDBG_LVL  9
#include <linux/hdebug.h>

extern struct bkexecd_info bked_info;

static inline int bof_has_fpga(struct bofhdr* bhdr) {
	return (bhdr->b_numchip > 0);
}

static int load_bof_binary(struct linux_binprm *bprm, struct  pt_regs *regs)
{
	int retval;
	struct bofhdr bhdr;
	struct bofhdr * bhdrp;
	char current_b_machine;
	char current_elfmachine;

	bhdr = *((struct bofhdr *) bprm->buf);
	if (bhdr.ident[0] != 0x19 || bhdr.ident[1] != 'B' ||
	    bhdr.ident[2] != 'O' || bhdr.ident[3] != 'F') {
		return -ENOEXEC;
	}
	PDEBUG(9, "b_machine=0x%x, b_elfmachine=0x%x, b_version=0x%x\n",
	       bhdr.b_machine, bhdr.b_elfmachine, bhdr.b_version);

	// check machine
#ifdef CONFIG_RHINO
	current_b_machine  = BM_RHINO;
	current_elfmachine = EM_ARM;
#else
	current_b_machine = BM_BEE2;
        current_elfmachine = EM_PPC;
#endif
	if ( bhdr.b_machine != current_b_machine || bhdr.b_elfmachine != current_elfmachine ) {
		PDEBUG(9, "Wrong b_machine (expecting %x) or b_elfmachine (expecting %x)!\n", current_b_machine, current_elfmachine);
		return -ENOEXEC;
	}

	// only handle version 6
	if (bhdr.b_version != 6) {
		return -ENOEXEC;
	}

	// bof file is valid
	if (bof_has_fpga(&bhdr)) {    
		struct execq_item* execq_item;
		unsigned long flags;
		PDEBUG(9, "queue this file to bkexecd...\n");
		// HHH should change to slab

		execq_item = kmalloc(sizeof(struct execq_item), GFP_KERNEL);
		execq_item->bprm = bprm;
		execq_item->task = current;
    
		spin_lock_irqsave(&bked_info.execq_lock, flags);
		list_add_tail(&(execq_item->list), &bked_info.execq_list);
		spin_unlock_irqrestore(&bked_info.execq_lock, flags);
		
		//wake_up_sync(&bked_info.more_exec);
		wake_up(&bked_info.more_exec);

		/* Sleep till our fpga partner is done.
		 * It is less efficient but save me trouble for synching 
		 * the 2 sides */
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
	}

	/* hw should have been loaded by now...  I spent all my time
	 * to setup bkexecd to do the hw loading but now I realize
	 * there's no way to notify binfmt if it fails... bummer... so
	 * right now I'll just use a byte in bofhdr (in brpm->buf) to
	 * store the return value...
	 * To fix it?  Note that for "simplicity" sake, hw loading is
	 * synchronous to sw, thus I could have eliminated bkexecd all
	 * together as a thread.  Instead, just all the functions
	 * directly here.
	 */
	bhdrp = (struct bofhdr*) (bprm->buf);
	if (bhdrp->load_err) {
		PDEBUG(5, "hw load error\n");
		return bhdrp->load_err;
	}

	// make it looks like an ELF and start over
	retval = kernel_read(bprm->file, bhdr.b_elfoff, bprm->buf, BINPRM_BUF_SIZE);
	if (retval < 0) {
		PDEBUG(5, "kernel_read failed\n");
		return -ENOEXEC;
	}

	PDEBUG(9, "read elf header at 0x%x [%02x %02x %02x %02x]\n", 
	       bhdr.b_elfoff,
	       bprm->buf[0], bprm->buf[1], bprm->buf[2], bprm->buf[3]);
	return search_binary_handler(bprm,regs);
}

static struct linux_binfmt bof_format = {
	.module		= THIS_MODULE,
	.load_binary	= load_bof_binary,
};

static int __init init_bof_binfmt (void) {
	PDEBUG(0, "binfmt_bof v4 loaded\n");
	return register_binfmt(&bof_format);
}

static void __exit exit_bof_binfmt(void) {
	PDEBUG(0, "binfmt_bof v4 unloaded\n");
	unregister_binfmt(&bof_format);
}


core_initcall(init_bof_binfmt);
module_exit(exit_bof_binfmt);
MODULE_AUTHOR("Hayden Kwok-Hay So");
MODULE_LICENSE("GPL");

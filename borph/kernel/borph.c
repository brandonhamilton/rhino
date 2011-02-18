/*********************************************************************
 * $Id: borph.c,v 1.10 2006/10/31 07:28:57 skhay Exp $
 * File  : borph.c
 * Author: Hayden Kwok-Hay So
 * Date  : 12/14/2005
 * Description:
 *   Main entry point for BORPH + Linux
 *********************************************************************/
#include <linux/borph.h>
#include <linux/slab.h>
#include <linux/init.h>    // needed for __init
#include <linux/module.h>
#define HDEBUG
#define HDBG_NAME "borph"
#define HDBG_LVL 9l
#include <linux/hdebug.h>

/*
 * borph_exit_fpga()
 * De-configure fpga associated with @tsk
 */
void borph_exit_fpga(struct task_struct *tsk)
{
	struct borph_hw_region *region, *tmp;
	struct borph_ioreg *reg, *tmpreg;
	struct task_list *tl, *tmptl;
	struct borph_info *bi;
	struct hwr_operations *hwrops;

	if (!(bi = tsk->borph_info)) {
	    goto exit;
	}

	/* HHH Race between /proc/<pid>/hw and here... */
	PDEBUG(0,"BORPH: exiting FPGA: closing ioreg access\n");

	if (!list_empty(&bi->ioreg)) {
		list_for_each_entry_safe(reg, tmpreg, &bi->ioreg, list) {
			list_del(&reg->list);
			kfree(reg);
		}
	}
	PDEBUG(0,"BORPH: exiting FPGA: checking hardware region\n");

	if (!list_empty(&bi->hw_region)) {
		list_for_each_entry_safe(region, tmp, &bi->hw_region, list) {
			PDEBUG(0,"kill fpga process at region 0x%X\n", region->addr.addr);
			kfree(region->strtab);
			PDEBUG(0,"BORPH: exiting FPGA: deactivating region\n");
			hwr_deactivate(&region->addr);
			hwrops = get_hwrops(&region->addr);
			PDEBUG(0,"BORPH: exiting FPGA: calling device unconfigure method\n");
			if (hwrops && hwrops->unconfigure) {
				hwrops->unconfigure(&region->addr);
			} else {
				printk("unknown hwr type %d\n", region->addr.class);
			}
			put_hwrops(&region->addr);

			PDEBUG(0,"BORPH: exiting FPGA: releasing region memory\n");
			release_hwr(&region->addr);
			list_del(&region->list);
			kfree(region);
		}
	}
	PDEBUG(0,"BORPH: exiting FPGA: Free kernel memory\n");
	kfree(tsk->borph_info);
	tsk->borph_info = NULL;
 exit:
	return;
}

/**************************************
 * Slab
 **************************************/
struct kmem_cache* task_list_cachep;

/**************************************
 * Initialization specific to BORPH 
 **************************************/
void __init borph_init(void)
{
	hwr_init();

	task_list_cachep = kmem_cache_create("task_list", 
					     sizeof(struct task_list),
					     0, 0, NULL);
	if (!task_list_cachep) {
	    return;
	}

	printk("BORPH (2.6.x) for RHINO initialized\n");
	return;
}

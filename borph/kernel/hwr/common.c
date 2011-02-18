/*********************************************************************
 * File  : kernel/hwr/common.c
 * Author: Brandon Hamilton, Hayden Kwok-Hay So
 * Date  : 8/29/2008
 * Description:
 *   Common code needed to handle different hwrtypes
 *********************************************************************/
#include <linux/borph.h>
#define HDEBUG
#define HDBG_NAME "borph"
#define HDBG_LVL 9 //mkd_info->dbg_lvl
#include <linux/hdebug.h>


struct hwrtype* hwrtypes[MAX_HWRTYPES];

/*
 * BKH: a 2 Dimensional array, where the first
 *      dimension is the hwrtype, which will determine the
 *      length of the second dimension of phyhwr (allocated
 *      on registration of the hwrtype).
 */
struct phyhwr** phyhwrs[MAX_HWRTYPES];

struct hwr_operations* get_hwrops(struct hwr_addr* a)
{
	struct hwrtype* ht = NULL;
	if (a->class >= MAX_HWRTYPES) {
		return NULL;
	}
	ht = hwrtypes[a->class];
	if (!ht) {
		return NULL;
	}
	atomic_inc(&ht->count);
	return ht->hwr_ops;
}

void put_hwrops(struct hwr_addr* a)
{
	struct hwrtype* ht = NULL;
	if (a->class >= MAX_HWRTYPES) {
		return;
	}
	ht = hwrtypes[a->class];
	if (!ht) {
		return;
	}
	atomic_dec(&ht->count);
	return;
}

/*
 * reserve_hwr checks if hwr addressed by hwr_addr is free
 * if so, return a pointer to that hwr
 * returns NULL otherwise.
 * 
 * It is like an atomic version of test and get_hwr
 */
struct phyhwr* reserve_hwr(struct hwr_addr* a)
{
        struct phyhwr *ret = NULL;
	struct hwr_operations *ops = get_hwrops(a);
	if (!ops) {
	        return NULL;
	}
	ret = ops->reserve_hwr(a);
	put_hwrops(a);
        return ret;
}

void release_hwr(struct hwr_addr* a)
{
	struct hwr_operations *ops = get_hwrops(a);
	if (!ops) {
		return;
	}
	ops->release_hwr(a);
	put_hwrops(a);
	return;
}

struct hwr_iobuf* get_iobuf(struct hwr_addr* a)
{
        struct hwr_iobuf *ret = NULL;
	struct hwr_operations *ops = get_hwrops(a);
	if (!ops) {
	        return NULL;
	}
	ret = ops->get_iobuf();
	put_hwrops(a);
        return ret;
}

ssize_t put_iobuf(struct hwr_addr* a, struct hwr_iobuf* iobuf)
{
        ssize_t ret = 0;
	struct hwr_operations *ops = get_hwrops(a);
	if (!ops) {
	        return 0;
	}
	ret = ops->put_iobuf(iobuf);
	put_hwrops(a);
        return ret;
}

ssize_t send_iobuf(struct hwr_addr* a, struct hwr_iobuf* iobuf)
{
        ssize_t ret = 0;
	struct hwr_operations *ops = get_hwrops(a);
	if (!ops) {
	        return 0;
	}
	ret = ops->send_iobuf(iobuf);
	put_hwrops(a);
        return ret;
}

ssize_t recv_iobuf(struct hwr_addr* a, struct hwr_iobuf* iobuf)
{
        ssize_t ret = 0;
	struct hwr_operations *ops = get_hwrops(a);
	if (!ops) {
	        return 0;
	}
	ret = ops->recv_iobuf(iobuf);
	put_hwrops(a);
        return ret;
}

struct phyhwr* get_hwr(struct hwr_addr* a)
{
        /* Use global array to get phyhwr for this 
         * class of hwr 
         */
	struct phyhwr* ret;
	if (!a || a->class >= MAX_HWRTYPES) return NULL;
	if (!hwrtypes[a->class] || a->addr >= hwrtypes[a->class]->num_devs) return NULL;
	ret = phyhwrs[a->class][a->addr];
	atomic_inc(&ret->count);
	return ret;
}

void put_hwr(struct hwr_addr* a)
{
	struct phyhwr* hwr;
	if (a && a->class >= MAX_HWRTYPES)
		return;
        if (!hwrtypes[a->class] || a->addr >= hwrtypes[a->class]->num_devs) 
                return;

	hwr = phyhwrs[a->class][a->addr];
	if (atomic_dec_and_test(&hwr->count)) {
		hwr->task = NULL;
		atomic_set(&hwr->count, -1);
	}
}

int hwr_inuse(struct hwr_addr* a)
{
	int ret;
	struct phyhwr* hwr = get_hwr(a);
	ret = (atomic_read(&hwr->active) > 0);
	put_hwr(a);
	return ret;
}

void hwr_activate(struct hwr_addr* a)
{
	struct phyhwr* hwr = get_hwr(a);
	if (!hwr)
		return;
	atomic_set(&hwr->active, 1);
	put_hwr(a);
}

void hwr_deactivate(struct hwr_addr* a)
{
	struct phyhwr* hwr = get_hwr(a);
	if (!hwr)
		return;
	atomic_set(&hwr->active, 0);
	put_hwr(a);
}

int register_hwrtype(struct hwrtype* ht)
{
	struct hwrtype* ct;
        uint16_t i;

	if (ht->type > MAX_HWRTYPES) {
		return -1;
	}
	ct = hwrtypes[ht->type];
	if (!ct) {
	        /* Initialize phyhwr array for this hwrtype */
                if (!phyhwrs[ht->type]) {
		          phyhwrs[ht->type] = (struct phyhwr**) kmalloc(sizeof(struct phywr*), GFP_KERNEL);
			  if (!phyhwrs[ht->type]) goto nomem;
			  /* Create a phyhwr struct for each device */
                          for (i = 0; i < ht->num_devs; i++) {
			          phyhwrs[ht->type][i] = (struct phyhwr*) kmalloc(sizeof(struct phyhwr), GFP_KERNEL);
			          if (!phyhwrs[ht->type][i]) goto nomem_free;		   
			  }
		}
	        hwrtypes[ht->type] = ht;
		atomic_inc(&ht->count);
		return 0;
	}
	printk("register_hwrtype: type %d already registered\n", ht->type);
        goto leave;
 nomem_free:
	while (i-- > 0) {
	         if (phyhwrs[ht->type][i])
	                 kfree(phyhwrs[ht->type][i]);
	}
        kfree(phyhwrs[ht->type]);
        phyhwrs[ht->type] = NULL;
 nomem:
	printk("register_hwrtype: no memory to allocate phyhwr for type %d\n", ht->type);
 leave:
	return -1;
}
// EXPORT_SYMBOL(register_hwrtype);

int unregister_hwrtype(struct hwrtype* ht)
{
	struct hwrtype* ct;
        uint16_t i;

	ct = hwrtypes[ht->type];
	if (ct && ct->type == ht->type) {
		if (atomic_dec_and_test(&ht->count)) {
			hwrtypes[ht->type] = NULL;

                        /* Free phyhwr memory */
                        if (phyhwrs[ht->type]) {
			         for (i = 0; i < ht->num_devs; i++) {
          				   if (phyhwrs[ht->type][i])
					            kfree(phyhwrs[ht->type][i]); 
				 }
			         kfree(phyhwrs[ht->type]);
                                 phyhwrs[ht->type] = NULL;
		        }
			return 0;
		}
	}
	return -1;
}
// EXPORT_SYMBOL(unregister_hwrtype);

void hwr_init(void)
{
	/* initialize hwrtypes */
	memset(&hwrtypes, 0, MAX_HWRTYPES*sizeof(struct hwrtype*));

	/* initialize phyhwrs */
    memset(&phyhwrs, 0, MAX_HWRTYPES*sizeof(struct phyhwr**));
}

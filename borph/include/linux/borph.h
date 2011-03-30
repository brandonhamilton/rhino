/**************************************************
 * File  : borph.h
 * Author: Hayden So, Brandon Hamilton
 * Date  : 06/11/2009
 * Description:
 *    Top level header file for all BORPH related files
 **************************************************/
#ifndef _BORPH_H_
#define _BORPH_H_

#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/bof.h>
#include <linux/file.h>
#include <linux/fdtable.h>

typedef unsigned char buf_t;

/* a file to be exec-ed to FPGA */
struct execq_item {
    struct list_head list;
    struct linux_binprm *bprm;
    struct task_struct *task;
};

/* all info for communicating to/from bkexecd
 * It has much resemblance to workqueue in 2.6 kernel */
struct bkexecd_info {
    wait_queue_head_t more_exec;  /* bkexecd to be awaken */
    wait_queue_head_t exec_done;  /* exec-er to be notified done exec */
    spinlock_t execq_lock;
    struct list_head execq_list;  /* list of execq_item */
};

/* function exported to the rest of kernel */
extern void borph_exit_fpga(struct task_struct* tsk);

/* each reconfigurable region in BORPH is represented by a 
 * struct borph_hw_region */
struct borph_hw_region {
	struct hwr_addr addr;
	char* strtab;
	struct list_head list;
};

/*
 * each reconfigurable region can have zero or more ioreg
 */
struct borph_ioreg {
	char* name;
	uint32_t mode;
	uint32_t loc;
	uint32_t len;
	struct hwr_addr* hwraddr;
	struct list_head list;
	struct borph_info *bi;
};

/* A dummy struct so that we can create a task_list list without
 * messing with the actual structure of task_list */
struct task_list {
	struct task_struct* tsk;
	rwlock_t data_lock;
	void* data;  // argument for passing to fringe
	struct list_head tsk_list;
};

extern struct kmem_cache* task_list_cachep;

/* borph_info contains all hardware specific information in a
 * task_struct.  */
struct borph_info {
	struct list_head hw_region;
	struct list_head ioreg;
	int ioreg_mode; 			 // 0 for ascii, 1 for raw
	unsigned int status;		 // configuration status
};

/*******************************************
 * functions to control the hwr device
 *******************************************/

struct hwr_iobuf {
	unsigned location;
	unsigned offset;
	size_t size;
	buf_t* data;
};

typedef struct hwr_iobuf* (*hwr_getiobuf_t) (void);

struct hwr_operations {
	int (*configure) (struct hwr_addr*, struct file*, uint32_t, uint32_t);
	int (*unconfigure) (struct hwr_addr*);
	struct phyhwr* (*reserve_hwr)(struct hwr_addr* a);
	void (*release_hwr) (struct hwr_addr* a);
	hwr_getiobuf_t get_iobuf;
	ssize_t (*put_iobuf) (struct hwr_iobuf* iobuf);
	ssize_t (*send_iobuf) (struct hwr_iobuf* iobuf);
	ssize_t (*recv_iobuf) (struct hwr_iobuf* iobuf);
};

struct hwrtype {
	char* name;
	uint16_t type;  		// identical to hwr_addr.class
	atomic_t count; 		// use count
	uint16_t num_devs;  	// number of physical devices
	struct hwr_operations *hwr_ops;
};

/* each physical hwr is represented by a hwr in kernel */
struct phyhwr {
	struct hwr_addr hwraddr;
	atomic_t count;            	// use count
	atomic_t active;   			// set to 1 if it is actively running
	struct task_struct *task;   // task that uses this hwr
};

#define MAX_HWRTYPES 16

extern struct hwrtype* hwrtypes[MAX_HWRTYPES];
extern struct phyhwr** phyhwrs[MAX_HWRTYPES];

extern int register_hwrtype(struct hwrtype*);
extern int unregister_hwrtype(struct hwrtype*);
extern struct hwr_operations* get_hwrops(struct hwr_addr* a);
extern void put_hwrops(struct hwr_addr* a);
extern struct phyhwr* reserve_hwr(struct hwr_addr*);
extern void release_hwr(struct hwr_addr* a);
extern struct phyhwr* get_hwr(struct hwr_addr* a);
extern void put_hwr(struct hwr_addr* a);
extern void __put_hwr(struct phyhwr* hwr);
extern struct hwr_iobuf* get_iobuf(struct hwr_addr* a);
extern ssize_t put_iobuf(struct hwr_addr* a, struct hwr_iobuf* iobuf);
extern ssize_t send_iobuf(struct hwr_addr* a, struct hwr_iobuf* iobuf);
extern ssize_t recv_iobuf(struct hwr_addr* a, struct hwr_iobuf* iobuf);
extern int hwr_inuse(struct hwr_addr* a);
extern void hwr_activate(struct hwr_addr* a);
extern void hwr_deactivate(struct hwr_addr* a);
extern void hwr_init(void);

/************************************************
 * misc
 ************************************************/
extern void borph_init(void);
extern void borph_exit_fpga(struct task_struct* tsk);
#endif /* _BORPH_H_ */

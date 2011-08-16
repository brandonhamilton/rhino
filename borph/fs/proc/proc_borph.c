/*
 * linux/fs/proc/proc_borph.c
 *
 * procfs virtual filesystem for communication with borph HW processes
 *
 * Authors: Brandon Hamilton, Hayden So
 *
 * Description:
 *   proc ioreg functions
 *
 */
#include <asm/uaccess.h>

#include <linux/errno.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/mount.h>
#include <linux/nsproxy.h>
#include <net/net_namespace.h>
#include <linux/seq_file.h>
#include <linux/mm.h>

#define HDEBUG
#define HDBG_NAME "proc_borph"
#ifdef CONFIG_MKD
# define HDBG_LVL mkd_info->dbg_lvl
#else
# define HDBG_LVL 9
#endif
#include <linux/hdebug.h>
#include <linux/borph.h>
#include "internal.h"
/* The following helpler functions were originally defined "static"
 * within base.c.  I need to use them here so they are not 
 * defined "static" anymore
 */

extern struct dentry *proc_pident_lookup(struct inode *dir, 
					 struct dentry *dentry,
					 const struct pid_entry *ents,
					 unsigned int nents);
extern int proc_pident_readdir(struct file *filp,
			       void *dirent, filldir_t filldir,
			       const struct pid_entry *ents, 
			       unsigned int nents);
extern ssize_t proc_info_read(struct file * file, char __user * buf,
			      size_t count, loff_t *ppos);
extern struct file_operations proc_info_file_operations;
extern struct inode *proc_pid_make_inode(struct super_block * sb, struct task_struct *task);
extern int pid_revalidate(struct dentry *dentry, struct nameidata *nd);

// End extern

/**** begin /proc/<pid>/hw/ioreg/ content ****/
static inline ssize_t to_ascii(buf_t* buf, ssize_t length)
{
	u32 val;
	char* cbuf = (char*) buf;
	int j, m;
	//PDEBUG(9, "to_ascii: buf[0] = %d, length=%d\n", buf[0], length);
	/* I only convert 1 word at most */
	if (length != 4) return -EINVAL;

	/* by default, I like hex. In fact, it will only work for hex
	 * for now, because having a dec values give me variable
	 * length to return, and thus I cannot return to user if I
	 * see a *fpos being too big without reading the actual 
	 * hardware, which wasted time */
	val = *((__u32*) buf);
	j = 8;  // always 8 characters long
	do {
		m = (val & 0xF);
		cbuf[--j] = ((m >= 10)?('A'-10):'0') + m;
	} while (val >>= 4);
	for (j = j-1; j >= 0; j--) {
		cbuf[j] = '0';
	}
	cbuf[8] = '\n';
	return 9;
}

static ssize_t ioreg_reg_read(struct file *filp, char __user *buf,
			       size_t fcount, loff_t *fpos)
{
	int retval = 0;
	int i;

	struct dentry *dentry = filp->f_path.dentry;
	struct inode *inode = dentry->d_inode;
	struct borph_ioreg* reg = (struct borph_ioreg*) PROC_I(inode)->data;
	struct hwr_operations *hwrops;
	struct task_struct *task;
	struct hwr_iobuf *iobuf;
	struct borph_info *bi;
	ssize_t regblen;

	if (!reg)
		goto out_no_task;

//	PDEBUG(5, "reading from ioreg name=%s, mode=0x%x loc=0x%x, len=%d, fcount=%d, *fpos=%llu\n", reg->name, reg->mode, reg->loc, reg->len, fcount, *fpos);

	if (reg->mode & IORM_PIPE && fpos != &filp->f_pos) {
		retval = -ESPIPE;
		goto out_no_task;
	}

	task = get_proc_task(inode);
	if (!task)
		goto out_no_task;
	bi = task->borph_info;
	if (!bi)
		goto out_no_hwrops;

	if (bi->ioreg_mode == 0) { /* ascii hex */
		regblen = reg->len << 1;
	} else { /* raw */
		regblen = reg->len;
	}

	if (*fpos + fcount > regblen) {
		if (*fpos >= regblen) 
			goto out_no_hwrops;
		fcount = (regblen - *fpos);
	}

	/* for now, let me cap myself at 900 bytes */
	if (fcount > (PAGE_SIZE - offsetof(struct hwr_iobuf, data))) {
		fcount = PAGE_SIZE - offsetof(struct hwr_iobuf, data);
	}

	hwrops = get_hwrops(reg->hwraddr);
	if (!hwrops)
		goto out_no_hwrops;
	if (!hwrops->get_iobuf || !hwrops->recv_iobuf || !hwrops->put_iobuf)
		goto out;

	/* Construct iobuf to get data */
	iobuf = hwrops->get_iobuf();
	if (!iobuf)
		goto out;

	iobuf->location = reg->loc;
	iobuf->offset = *fpos;
	// Only read the required number of bytes
        //iobuf->size = reg->len;
        iobuf->size = bi->ioreg_mode?fcount:fcount>>1;
	retval = hwrops->recv_iobuf(iobuf);
	if (retval < 0) {
		goto out_put_iobuf;
	}

//	PDEBUG(9, "Data received from FPGA (%d bytes)\n", retval);

	// Print out data for debugging
        // for (i = 0; i < retval; i++)
 	//	PDEBUG(9, "0x%x ", iobuf->data[i]);

//	PDEBUG(9, "\n");

	if (bi->ioreg_mode == 0) { /* ascii mode required */
//		PDEBUG(9, "Converting to ASCII\n");
		retval = to_ascii(iobuf->data, retval);
		if (retval < 0) {
			goto out_put_iobuf;
		}
	}

//	PDEBUG(9, "Copying to user buffer\n");

	/* Write the data from the iobuf into the file pointer */
	copy_to_user(buf, iobuf->data, retval);

	if (reg->mode & IORM_PIPE) {
		*fpos = 0;
	} else {
		*fpos += retval;
	}

//	PDEBUG(9, "ioreg reading completed\n");

 out_put_iobuf:
	hwrops->put_iobuf(iobuf);
 out:
	put_hwrops(reg->hwraddr);
 out_no_hwrops:
	put_task_struct(task);
 out_no_task:
	return retval;
}

/* convert_input: Convert the first *cnt* bytes from *buf* into its
 *                numerical value
 * Note: All invalid character from buf will be happily ignored 
 * instead of generating error.  It is useful so that if a user input
 * "123\n", it will return the value 123 and consumes the \n at the end
 * gracefully.
 */
static int convert_input(char* buf, int cnt) {
	int retval = -EINVAL;
	char c;
	int num = 0;
	if (buf[0] == 'x') {  // hex input
//		PDEBUG(9, "converting hexadecimal\n");
		if (cnt < 2) goto out;
		buf += 1;
		while (cnt > 0) {
			c = *buf;
			if (c != '\n') {
				num *= 16;
				if (c >= 'a' && c <= 'f') {
					num += c - 'a';
				} else if (c >= 'A' && c <= 'F') {
					num += c - 'A';
				} else if (c >= '0' && c <= '9') {
					num += c - '0';
				}
			}
			buf += 1;
			cnt -= 1;
		}
	} else if (buf[0] == 'b') { // binary number input
//		PDEBUG(9, "converting binary\n");
		if (cnt < 2) goto out;
		buf += 1;
		while (cnt > 0) {
			c = *buf;
			if (c != '\n') {
				num *= 2;
				if (c >= '0' && c <= '1') {
					num += c - '0';
				} else {
					goto out;
				}
			}
			buf += 1;
			cnt -= 1;
		}
	} else { // assume decimal number
//		PDEBUG(9, "converting decimal\n");
		while (cnt > 0) {
			c = *buf;
//			PDEBUG(9, "c = 0x%02X\n", c);
			if (c >= '0' && c <='9')  {
				num *= 10;
				num += c - '0';
			}
			buf += 1;
			cnt -= 1;
		}
	}
	retval = num;
 out:
	return retval;
}

#define NUMTMPBUF 12
static struct hwr_iobuf* ioreg_reg_cp_udata_ascii(struct borph_ioreg* reg,
						  const char __user *ubuf,
						  size_t fcount, loff_t *fpos, 
						  hwr_getiobuf_t get_iobuf)
{
	int datacnt = fcount;
	struct hwr_iobuf *retval = ERR_PTR(-EINVAL);
	struct hwr_iobuf *iobuf;

	/* at most 12 characters for 2^32 (plus base+newline)  */
	char tmpbuf[NUMTMPBUF]; 
	int num;

	/* ascii mode is only for simple register with no seeking, so *fpos
	 * should really be 0 all the time (at least when I am lazy)*/
	if (*fpos != 0) {
		retval = ERR_PTR(-ESPIPE);
		goto out;
	}

	if (datacnt > NUMTMPBUF) {
		retval = ERR_PTR(-EINVAL);
		goto out;
	}

	if (copy_from_user(tmpbuf, ubuf, datacnt)) {
		retval = ERR_PTR(-EFAULT);
		goto out;
	}
//	PDEBUG(9, "converting num: tmpbuf=[%02X %02X %02X %02X %02X ... ]\n", tmpbuf[0],tmpbuf[1],tmpbuf[2],tmpbuf[3],tmpbuf[4]);
	num = convert_input(tmpbuf, datacnt);
//	PDEBUG(5, "num returned as %d\n", num);
	if (num < 0) {
		retval = ERR_PTR(-EINVAL);
		goto out;
	}

	/* Get I/O buffer from the specific hwr */
	iobuf = get_iobuf();

	// In ASCII mode, I don't want anything strange
	if (!iobuf || iobuf->size < datacnt)
		goto out;

	if (!memcpy(iobuf->data, &num, sizeof(num))) {
		retval = ERR_PTR(-EFAULT);
		goto out;
	}
	iobuf->size = sizeof(num);
	retval = iobuf;
 out:
	return retval;
}

/* ioreg_reg_cp_udata_raw:
 *  Implements the actual writing process of an ioreg file when 
 * ioreg_mode is set to 1
 *  Relies on hwrops->get_iobuf as kernel buffer in which user
 * data is copied.  Usually hwrops->get_iobuf will give a pointer
 * into the middle of the entire buffer s.t. it can put header in
 * the front and still be able to make it preperly aligned for better
 * (e.g. DMA) transfer.
 * return:
 *   non-NULL : a hwr_iobuf filled with data+cnt to send
 *   ERR_PTR : error
 */
static struct hwr_iobuf* ioreg_reg_cp_udata_raw(struct borph_ioreg* reg, 
						const char __user *ubuf,
						size_t fcount, loff_t *fpos, 
						hwr_getiobuf_t get_iobuf)
{
	int datacnt = fcount;
	struct hwr_iobuf* retval = ERR_PTR(-EINVAL);
	struct hwr_iobuf* iobuf;

	int len_in_byte;
 
	len_in_byte = reg->len;

	/* non aligned access */
/*	if (*fpos % 4 || fcount % 4) {
		PDEBUG(9, "raw_write: unaligned write\n");
		return -EINVAL;
	}
*/
	/* datacnt = min(len-*fpos, fcount) */
	if (datacnt > (len_in_byte - *fpos)) {
		datacnt = len_in_byte - *fpos;
	}

	/* cap fcount to 1 page */
	/* I used to have 900 as a temp hack, now trying to reuse PAGE_SIZE
	 *  H,S. 1/30/08 
	 * This is needed so that we won't overflow kbuf (H.S. 7/25/08)
	 */
	if (datacnt > PAGE_SIZE)
		datacnt = PAGE_SIZE;

	iobuf = get_iobuf();
	if (!iobuf || !iobuf->size)
		goto out;

	if (iobuf->size < datacnt)
		datacnt = iobuf->size;

//	PDEBUG(9, "copy %d bytes from user\n", datacnt);
	if (copy_from_user(iobuf->data, ubuf, datacnt)) {
		retval = ERR_PTR(-EFAULT);
		goto out;
	}
	{
		unsigned char* p = iobuf->data;
//		PDEBUG(5, "ioreg_cp_valid_user_data: copied bytes[0-7]: %02X %02X %02X %02X %02X %02X %02X %02X\n",  p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
	}
	iobuf->size = datacnt;

	retval = iobuf;

 out:
	return retval;
}

static ssize_t ioreg_reg_write(struct file *filp, const char __user *buf,
			       size_t fcount, loff_t *fpos)
{
	struct dentry *dentry = filp->f_path.dentry;
	struct inode *inode = dentry->d_inode;
	struct borph_ioreg* reg = (struct borph_ioreg*) PROC_I(inode)->data;
	struct task_struct *task;
	struct hwr_operations *hwrops;
	struct borph_info *bi;
	struct hwr_iobuf *iobuf;
	int retval = -EINVAL;

	if (!reg)
		goto out_no_task;

//	PDEBUG(5, "writing to ioreg name=%s, mode=0x%x loc=0x%x, len=%d, fcount=%d, *fpos=%llu\n", reg->name, reg->mode, reg->loc, reg->len, fcount, *fpos);

	if (reg->mode & IORM_PIPE && fpos != &filp->f_pos) {
		retval = -ESPIPE;
		goto out_no_task;
	}
	/* no appending */
	if (*fpos >= reg->len) {
		retval = fcount;
		goto out_no_task;
	}

	task = get_proc_task(inode);
	if (!task)
		goto out_no_task;
	bi = task->borph_info;
	if (!bi)
		goto out_no_hwrops;

	hwrops = get_hwrops(reg->hwraddr);
	if (!hwrops)
		goto out_no_hwrops;
	if (!hwrops->get_iobuf || !hwrops->send_iobuf || !hwrops->put_iobuf)
		goto out;

	if (bi->ioreg_mode == 1) {
		iobuf = ioreg_reg_cp_udata_raw(reg, buf, fcount, fpos, 
						hwrops->get_iobuf);
	} else {
		iobuf = ioreg_reg_cp_udata_ascii(reg, buf, fcount, fpos,
						 hwrops->get_iobuf);
	}
	if (IS_ERR(iobuf))
		goto out;

	/* Set the iobuf parameters */
	iobuf->offset = *fpos;
	iobuf->location = reg->loc;

	retval = hwrops->send_iobuf(iobuf);
	if (retval <= 0)
		goto out_with_iobuf;

	if (reg->mode & IORM_PIPE) {
		*fpos = 0;
	} else {
		*fpos += retval;
	}

//	PDEBUG(9, "ioreg writing completed\n");

 out_with_iobuf:
	hwrops->put_iobuf(iobuf);
 out:
	put_hwrops(reg->hwraddr);
 out_no_hwrops:
	put_task_struct(task);
 out_no_task:
	return retval;
}

static loff_t ioreg_reg_lseek(struct file *filp, loff_t off, int whence)
{
	struct dentry *dentry = filp->f_path.dentry;
	struct inode *inode = dentry->d_inode;
	struct borph_ioreg* reg;
	loff_t newpos;

	reg = (struct borph_ioreg*) PROC_I(inode)->data;
	if (!reg)
		return -EINVAL;
	if (reg->mode & IORM_PIPE)
		return -ESPIPE; /* unseekable */

	switch(whence) {
	case 0: /* SEEK_SET */
		newpos = off;
		break;
	case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;
	case 2: /* SEEK_END */
		newpos = reg->len + off;
		break;
	default:
		return -EINVAL;
	}
	if (newpos < 0) return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}

static int ioreg_reg_mmap(struct file *filp, struct vm_area_struct *vma) {

	struct dentry *dentry = filp->f_path.dentry;
	struct inode *inode = dentry->d_inode;
	struct borph_ioreg* reg;
	struct hwr_operations *hwrops;
	unsigned long off;
	unsigned long physical;
	unsigned long vsize;
	unsigned long psize;

	reg = (struct borph_ioreg*) PROC_I(inode)->data;
	if (!reg)
		return -EINVAL;

	off = vma->vm_pgoff << PAGE_SHIFT;
	physical = reg->loc + off;
	vsize = vma->vm_end - vma->vm_start;
	psize = reg->len - off;

	if (vsize > psize)
	    return -EINVAL;
	
	if (remap_pfn_range(vma, vma->vm_start, physical, vsize, vma->vm_page_prot))
        return -EAGAIN;
      
    return 0;
}

static const struct file_operations ioreg_reg_fops = {
	.read		= ioreg_reg_read,
	.write		= ioreg_reg_write,
	.llseek		= ioreg_reg_lseek,
	.mmap		= ioreg_reg_mmap,
};

/**** end /proc/<pid>/hw/ioreg/ content ****/

/**** begin /proc/<pid>/hw/ioreg directory ****/
static struct dentry *proc_ioreg_instantiate(struct inode *dir,
	struct dentry *dentry, struct task_struct *task, const void *ptr)
{
	const struct borph_ioreg *reg = ptr;
	struct inode *inode;
	struct proc_inode *ei;
	struct dentry *error = ERR_PTR(-EINVAL);

	inode = proc_pid_make_inode(dir->i_sb, task);
	if (!inode)
		goto out;

	ei = PROC_I(inode);
	ei->data = (void*) reg;

	if (reg->mode & IORM_PIPE) {
	    inode->i_mode = S_IFIFO;
	} else {
	    inode->i_mode = S_IFREG;
	}
	inode->i_mode |= (reg->mode&IORM_READ)?S_IRUGO:0;
	inode->i_mode |= (reg->mode&IORM_WRITE)?S_IWUGO:0;
	inode->i_size = reg->len;
	inode->i_fop = &ioreg_reg_fops;

	dentry->d_op = &pid_dentry_operations;
	d_add(dentry, inode);

	/* Close the race of the process dying before we return the dentry */
	if (pid_revalidate(dentry, NULL))
		error = NULL;
out:
	return error;
}

static struct dentry *proc_ioreg_lookup(struct inode *dir,
					struct dentry *dentry,
					struct nameidata *nd)
{
	struct task_struct *task = get_proc_task(dir);
	struct borph_info* bi;
	struct borph_ioreg* reg;
	struct dentry *retval;
	int regnamelen;
	int i;

	retval = ERR_PTR(-EINVAL);
	if (!task)
		goto out_no_task;

	bi = task->borph_info;
	if (!bi)
		goto out;

	retval = ERR_PTR(-ENOENT);
	i = 2;
	list_for_each_entry(reg, &bi->ioreg, list) {
		regnamelen = strlen(reg->name);
		if (dentry->d_name.len == regnamelen &&
		    !memcmp(dentry->d_name.name,reg->name,regnamelen)) {
			retval = NULL;
			break;
		}
		i += 1;
	}
	if (retval) goto out;

	retval = proc_ioreg_instantiate(dir, dentry, task, reg);

 out:
	put_task_struct(task);
 out_no_task:
	return retval;
}

static int proc_ioreg_fill_cache(struct file *filp, void *dirent,
	filldir_t filldir, struct task_struct *task, const struct borph_ioreg *reg)
{
	return proc_fill_cache(filp, dirent, filldir, reg->name, strlen(reg->name),
				proc_ioreg_instantiate, task, reg);
}

static int proc_ioreg_readdir(struct file * filp, void * dirent, filldir_t filldir)
{
	struct dentry *dentry = filp->f_path.dentry;
	struct inode *inode = dentry->d_inode;
	struct task_struct *task = get_proc_task(inode);
	unsigned int pos, pid, i;
	ino_t ino;
	int retval;
	struct borph_info *bi;
	struct borph_ioreg *reg;


	retval = -ENOENT;
	if (!task)
		goto out_no_task;

	pid = task->pid;
	if (!pid)
		goto out;

	retval = 0;
	pos = filp->f_pos;
	switch (pos) {
	case 0:
		ino = inode->i_ino;
		if (filldir(dirent, ".", 1, 0, ino, DT_DIR) < 0)
			goto out;
		filp->f_pos++;
		/* fall through */
	case 1:
		ino = parent_ino(dentry);
		if (filldir(dirent, "..", 2, 1, ino, DT_DIR) < 0)
			goto out;
		filp->f_pos++;
		/* fall through */
	default:
		/* nothing in hw directory if not a borph process */
		bi = task->borph_info;
		if (!bi) {
			retval = 1;
			goto out;
		}
		i = 2;
		list_for_each_entry(reg, &bi->ioreg, list) {
			if (i >= pos) {
				if (proc_ioreg_fill_cache(filp, dirent, 
							  filldir, task, reg) < 0)
					goto out;
				filp->f_pos++;
			}
			i += 1;
		}
	}
	retval = 1;
 out:
	put_task_struct(task);

 out_no_task:
	return retval;
}

static struct inode_operations proc_ioreg_inode_operations = {
	.lookup		= proc_ioreg_lookup,
};

static struct file_operations proc_ioreg_operations = {
	.read		= generic_read_dir,
	.readdir	= proc_ioreg_readdir,
};
/**** end /proc/<pid>/hw/ioreg directory ****/

static ssize_t proc_status_read(struct task_struct *task, char *buffer)
{
	struct borph_info *bi = task->borph_info;
	if (!bi)
		return -EINVAL;
  	snprintf(buffer,5,"%04x",bi->status);
	buffer[4] = '\n';
	return 5;
}

static ssize_t status_read(struct file * file, char __user * buf, size_t count, loff_t *ppos)
{
	return proc_info_read(file, buf, count, ppos);
}

static ssize_t status_write(struct file *filp, const char *buf, size_t fcount, loff_t *fpos)
{
	struct dentry *dentry = filp->f_path.dentry;
	struct inode *inode = dentry->d_inode;
	struct task_struct *task = get_proc_task(inode);
	struct borph_info *bi = task->borph_info;
	int retval;

	if (!bi) 
		return -EINVAL;
	return fcount;
}

static const struct file_operations proc_status_file_operations = {
	.read		= status_read,
	.write		= status_write,
};

/**** begin /proc/<pid>/hw/ content ****/
// /proc/<pid>/hw/ioreg_mode
static ssize_t proc_ioregmode_read(struct task_struct *task, char *buffer)
{
	struct borph_info *bi = task->borph_info;
	if (!bi) return -EINVAL;
	buffer[0] = '0' + bi->ioreg_mode;
	buffer[1] = '\n';
	return 2;
}

static ssize_t ioregmode_read(struct file * file, char __user * buf,
			      size_t count, loff_t *ppos)
{
	return proc_info_read(file, buf, count, ppos);
}

static ssize_t ioregmode_write(struct file *filp, const char *buf,
			       size_t fcount, loff_t *fpos)
{
	struct dentry *dentry = filp->f_path.dentry;
	struct inode *inode = dentry->d_inode;
	struct task_struct *task = get_proc_task(inode);
	struct borph_info *bi = task->borph_info;
	int retval;

	if (!bi) return -EINVAL;
	if (fcount) {
		char c;
		if ((retval = get_user(c, buf)) < 0) return retval;
		if (c < '0' || c > '9') return -EINVAL;
		bi->ioreg_mode = (c - '0');
	}
	return fcount;
}

static const struct file_operations proc_ioregmode_file_operations = {
	.read		= ioregmode_read,
	.write		= ioregmode_write,
};


// /proc/<pid>/hw/hwregion
static ssize_t proc_hwregion_read(struct task_struct *task, char *buffer)
{
	int len, i;
	struct borph_hw_region *region, *tmp;
	struct borph_info *bi;

	if (!(bi = task->borph_info)) {
		return -EINVAL;
	}
	i = 0;
	len = 0;
	list_for_each_entry_safe(region, tmp, &bi->hw_region, list) {
		len = sprintf(buffer, "HW Region %d\n Address: 0x%X\n", i, region->addr.addr);
	}
	return len;
}

/**** end /proc/<pid>/hw/ content ****/

/**** begin /proc/<pid>/hw directory ****/
const struct inode_operations proc_bphhw_inode_operations;
const struct file_operations proc_bphhw_operations;

static const struct pid_entry bphhw_stuff[] = {
	DIR("ioreg",    S_IRUGO|S_IXUGO, proc_ioreg_inode_operations, proc_ioreg_operations),
	NOD("status", S_IFREG|S_IRUSR|S_IWUSR, NULL, &proc_status_file_operations,
		{ .proc_read = &proc_status_read } ),
	NOD("ioreg_mode", S_IFREG|S_IRUSR|S_IWUSR, 
	    NULL, &proc_ioregmode_file_operations,
		{ .proc_read = &proc_ioregmode_read } ),
	INF("hwregion", S_IRUSR, &proc_hwregion_read),
};

static struct dentry *proc_tgid_bphhw_lookup(struct inode *dir, 
					     struct dentry *dentry,
					     struct nameidata *nd)
{
	return proc_pident_lookup(dir, dentry,
				  bphhw_stuff, ARRAY_SIZE(bphhw_stuff));
}

static int proc_tgid_bphhw_readdir(struct file * filp, 
				   void * dirent, filldir_t filldir)
{
	struct dentry *dentry = filp->f_path.dentry;
	struct inode *inode = dentry->d_inode;
	struct task_struct *task = get_proc_task(inode);
	/* nothing in "hw" directory if not a hw process */
	if (!task->borph_info) {
		return 1;
	}
	return proc_pident_readdir(filp,dirent,filldir,
				   bphhw_stuff,ARRAY_SIZE(bphhw_stuff));
}

/* iop -- named according to DIR() macro */
const struct inode_operations proc_bphhw_inode_operations = {
	.lookup = proc_tgid_bphhw_lookup,
};

/* fop -- named according to DIR() macro */
const struct file_operations proc_bphhw_operations = {
	.read = generic_read_dir,
	.readdir = proc_tgid_bphhw_readdir,
};

/**** end /proc/<pid>/hw directory ****/


/**************************************************
 * File  : bof.h
 * Author: Hayden So, Brandon Hamilton
 * Date  : 06/11/2009
 * Description:
 *    header file for the BOF file format
 **************************************************/
#ifndef _BOF_H_
#define _BOF_H_

/******************** Main BOF header ******************
 * It MUST be at most 52 bytes long.  i.e. it MUST be the exact
 * same size as a elf header Elf32_EHdr
 *******************************************************/
struct bofhdr {
	unsigned char ident[16];
	uint32_t b_version;
	uint16_t b_machine;
	uint16_t b_elfmachine;
	uint32_t b_numchip;
	uint32_t b_elfoff;  // offset of elf header from beginning of file
	uint32_t b_hwoff;   // offset of the 1st hwregion from beginning of file
	int32_t load_err;
	uint32_t b_ekver;   // embedded kernel version
	uint32_t pad[2];
};

/* position of various misc identifying info within ident */
#define BI_ENDIAN 5
#define BOFDATA2LSB 1
#define BOFDATA2MSB 2

/* legal values for b_machine */
#define BM_BEE2		2
#define BM_ROACH	3
#define BM_NETFPGA	4
#define BM_RHINO	5

/***********************
 * header per hw region
 ***********************/
struct hwr_addr {
	uint16_t class;
	uint16_t addr;
};

#define HAC_BEE2FPGA 0x1
#define HAC_PRMOD    0x2
#define HAC_ROACHV5  0x3
#define HAC_NETFPGA  0x4
#define HAC_RHINO	 0x5

/* hwrhdr flags */
#define HFG_PLACED   0x1
#define HFG_NOCONFIG   0x2
struct hwrhdr {
	uint32_t flag;
	struct hwr_addr addr;    /* address of placed */
	uint32_t pl_off;         /* payload offset */
	uint32_t pl_len;         /* payload len */
	uint32_t nr_symbol;
	uint32_t strtab_off;     /* string table offset */
	uint32_t next_hwr;       /* next hw_region on file */
	// HHH and more info about register mapping, etc
};

#define IORM_READ 1
#define IORM_WRITE 2
#define IORM_READWRITE 3
#define IORM_PIPE 4
struct bofioreg {
	uint16_t name;     //offset into strtab where this name can be found
	uint16_t mode;
	uint32_t loc;
	uint32_t len;
};

#endif /* _BOF_H_ */

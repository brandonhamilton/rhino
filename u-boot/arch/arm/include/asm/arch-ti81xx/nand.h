/*
 * (C) Copyright 2010-2011 Texas Instruments, <www.ti.com>
 * Mansoor Ahamed <mansoor.ahamed@ti.com>
 *
 * Derived from work done by Rohit Choraria <rohitkc@ti.com> for omap3
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __ASM_ARCH_OMAP_GPMC_H
#define __ASM_ARCH_OMAP_GPMC_H

#define GPMC_BUF_EMPTY		0
#define GPMC_BUF_FULL		1

#define ECCCLEAR		(0x1 << 8)
#define ECCRESULTREG1		(0x1 << 0)
#define ECCSIZE512BYTE		0xFF
#define ECCSIZE1		(ECCSIZE512BYTE << 22)
#define ECCSIZE0		(ECCSIZE512BYTE << 12)
#define ECCSIZE0SEL		(0x000 << 0)

/* Generic ECC Layouts */
/* Large Page x8 NAND device Layout */
#ifdef GPMC_NAND_ECC_LP_x8_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 12,\
	.eccpos = {1, 2, 3, 4, 5, 6, 7, 8,\
		9, 10, 11, 12},\
	.oobfree = {\
		{.offset = 13,\
		 .length = 51 } } \
}
#endif

/* Large Page x16 NAND device Layout */
#ifdef GPMC_NAND_ECC_LP_x16_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 12,\
	.eccpos = {2, 3, 4, 5, 6, 7, 8, 9,\
		10, 11, 12, 13},\
	.oobfree = {\
		{.offset = 14,\
		 .length = 50 } } \
}
#endif

/* NAND device layout in synch with the kernel */
#ifdef GPMC_NAND_ECC_LP_x16_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT_KERNEL {\
	.eccbytes = 12,\
	.eccpos = {\
		   40, 41, 42, 43, 44, 45, 46, 47,\
		   48, 49, 50, 51},\
	.oobfree = {\
		{.offset = 2,\
		 .length = 38} } \
}
#endif

/* Small Page x8 NAND device Layout */
#ifdef GPMC_NAND_ECC_SP_x8_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 3,\
	.eccpos = {1, 2, 3},\
	.oobfree = {\
		{.offset = 4,\
		 .length = 12 } } \
}
#endif

/* Small Page x16 NAND device Layout */
#ifdef GPMC_NAND_ECC_SP_x16_LAYOUT
#define GPMC_NAND_HW_ECC_LAYOUT {\
	.eccbytes = 3,\
	.eccpos = {2, 3, 4},\
	.oobfree = {\
		{.offset = 58,\
		 .length = 6 } } \
}
#endif

#define GPMC_NAND_HW_BCH4_ECC_LAYOUT {\
	.eccbytes = 32,\
	.eccpos = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,\
				16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,\
				28, 29, 30, 31, 32, 33},\
	.oobfree = {\
		{.offset = 34,\
		 .length = 30 } } \
}

#define GPMC_NAND_HW_BCH8_ECC_LAYOUT {\
	.eccbytes = 56,\
	.eccpos = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,\
				16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,\
				28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,\
				40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,\
				52, 53, 54, 55, 56, 57},\
	.oobfree = {\
		{.offset = 58,\
		 .length = 6 } } \
}

#define GPMC_NAND_HW_BCH16_ECC_LAYOUT {\
	.eccbytes = 104,\
	.eccpos = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,\
				16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,\
				28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,\
				40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,\
				52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,\
				64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75,\
				76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87,\
				88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,\
				100, 101, 102, 103, 104, 105},\
	.oobfree = {\
		{.offset = 106,\
		 .length = 8 } } \
}

/*
 * ELM Module Registers
 */

/* ELM registers bit fields */
#define ELM_SYSCONFIG_SOFTRESET_MASK			(0x2)
#define ELM_SYSCONFIG_SOFTRESET			(0x2)
#define ELM_SYSSTATUS_RESETDONE_MASK			(0x1)
#define ELM_SYSSTATUS_RESETDONE			(0x1)
#define ELM_LOCATION_CONFIG_ECC_BCH_LEVEL_MASK		(0x3)
#define ELM_LOCATION_CONFIG_ECC_SIZE_MASK		(0x7FF0000)
#define ELM_LOCATION_CONFIG_ECC_SIZE_POS		(16)
#define ELM_SYNDROME_FRAGMENT_6_SYNDROME_VALID		(0x00010000)
#define ELM_LOCATION_STATUS_ECC_CORRECTABLE_MASK	(0x100)
#define ELM_LOCATION_STATUS_ECC_NB_ERRORS_MASK		(0x1F)

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__

enum bch_level {
	BCH_4_BIT = 0,
	BCH_8_BIT,
	BCH_16_BIT
};


/* BCH syndrome registers */
struct syndrome {
	u32 syndrome_fragment_x[7]; 	/* 0x400, 0x404.... 0x418 */
	u8 res1[36]; 			/* 0x41c */
};

/* BCH error status & location register */
struct location {
	u32 location_status;		/* 0x800 */
	u8 res1[124];			/* 0x804 */
	u32 error_location_x[16];	/* 0x880.... */
	u8 res2[64];			/* 0x8c0 */
};

/* BCH ELM register map - do not try to allocate memmory for this structure.
 * We have used plenty of reserved variables to fill the slots in the ELM
 * register memory map.
 * Directly initialize the struct pointer to ELM base address.
 */
struct elm {
	u32 rev; 				/* 0x000 */
	u8 res1[12];				/* 0x004 */
	u32 sysconfig;  			/* 0x010 */
	u32 sysstatus;				/* 0x014 */
	u32 irqstatus;				/* 0x018 */
	u32 irqenable;				/* 0x01c */
	u32 location_config;			/* 0x020 */
	u8 res2[92]; 				/* 0x024 */
	u32 page_ctrl;				/* 0x080 */
	u8 res3[892]; 				/* 0x084 */
	struct  syndrome syndrome_fragments[8]; /* 0x400 */
	u8 res4[512]; 				/* 0x600 */
	struct location  error_location[8]; 	/* 0x800 */
};

int elm_check_error(u8 *syndrome, u32 nibbles, u32 *error_count,
		u32 *error_locations);
int elm_config(enum bch_level level);
void elm_reset(void);
void elm_init(void);
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */


#endif /* __ASM_ARCH_OMAP_GPMC_H */


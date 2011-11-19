/*
 * (C) Copyright 2006-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author
 *		Mansoor Ahamed <mansoor.ahamed@ti.com>
 *
 * Initial Code from:
 * 		Richard Woodruff <r-woodruff2@ti.com>
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

#ifndef _MEM_H_
#define _MEM_H_

#define CS0		0x0
#define CS1		0x1 /* mirror CS1 regs appear offset 0x30 from CS0 */

#ifndef __ASSEMBLY__
enum {
	STACKED = 0,
	IP_DDR = 1,
	COMBO_DDR = 2,
	IP_SDR = 3,
};
#endif /* __ASSEMBLY__ */

#define EARLY_INIT	1

/*
 * GPMC settings -
 * Definitions is as per the following format
 * #define <PART>_GPMC_CONFIG<x> <value>
 * Where:
 * PART is the part name e.g. STNOR - Intel Strata Flash
 * x is GPMC config registers from 1 to 6 (there will be 6 macros)
 * Value is corresponding value
 *
 * For every valid PRCM configuration there should be only one definition of
 * the same. if values are independent of the board, this definition will be
 * present in this file if values are dependent on the board, then this should
 * go into corresponding mem-boardName.h file
 *
 * Currently valid part Names are (PART):
 * STNOR - Intel Strata Flash
 * SMNAND - Samsung NAND
 * MPDB - H4 MPDB board
 * SBNOR - Sibley NOR
 * MNAND - Micron Large page x16 NAND
 * ONNAND - Samsung One NAND
 *
 * include/configs/file.h contains the defn - for all CS we are interested
 * #define OMAP34XX_GPMC_CSx PART
 * #define OMAP34XX_GPMC_CSx_SIZE Size
 * #define OMAP34XX_GPMC_CSx_MAP Map
 * Where:
 * x - CS number
 * PART - Part Name as defined above
 * SIZE - how big is the mapping to be
 *   GPMC_SIZE_128M - 0x8
 *   GPMC_SIZE_64M  - 0xC
 *   GPMC_SIZE_32M  - 0xE
 *   GPMC_SIZE_16M  - 0xF
 * MAP  - Map this CS to which address(GPMC address space)- Absolute address
 *   >>24 before being used.
 */
#define GPMC_SIZE_256M		0x0
#define GPMC_SIZE_128M		0x8
#define GPMC_SIZE_64M		0xC
#define GPMC_SIZE_32M		0xE
#define GPMC_SIZE_16M		0xF

#define SMNAND_GPMC_CONFIG1	0x00000800
#define SMNAND_GPMC_CONFIG2	0x00141400
#define SMNAND_GPMC_CONFIG3	0x00141400
#define SMNAND_GPMC_CONFIG4	0x0F010F01
#define SMNAND_GPMC_CONFIG5	0x010C1414
#define SMNAND_GPMC_CONFIG6	0x1F0F0A80
#define SMNAND_GPMC_CONFIG7	0x00000C44

#define M_NAND_GPMC_CONFIG1	0x00001810
#define M_NAND_GPMC_CONFIG2 	0x001e1e00
#define M_NAND_GPMC_CONFIG3 	0x001e1e00
#define M_NAND_GPMC_CONFIG4 	0x16051807
#define M_NAND_GPMC_CONFIG5 	0x00151e1e
#define M_NAND_GPMC_CONFIG6	0x16000f80
#define M_NAND_GPMC_CONFIG7	0x00000008

#define STNOR_GPMC_CONFIG1	0x3
#define STNOR_GPMC_CONFIG2	0x00151501
#define STNOR_GPMC_CONFIG3	0x00060602
#define STNOR_GPMC_CONFIG4	0x11091109
#define STNOR_GPMC_CONFIG5	0x01141F1F
#define STNOR_GPMC_CONFIG6	0x000004c4

#define SIBNOR_GPMC_CONFIG1	0x1200
#define SIBNOR_GPMC_CONFIG2	0x001f1f00
#define SIBNOR_GPMC_CONFIG3	0x00080802
#define SIBNOR_GPMC_CONFIG4	0x1C091C09
#define SIBNOR_GPMC_CONFIG5	0x01131F1F
#define SIBNOR_GPMC_CONFIG6	0x1F0F03C2

#define SDPV2_MPDB_GPMC_CONFIG1	0x00611200
#define SDPV2_MPDB_GPMC_CONFIG2	0x001F1F01
#define SDPV2_MPDB_GPMC_CONFIG3	0x00080803
#define SDPV2_MPDB_GPMC_CONFIG4	0x1D091D09
#define SDPV2_MPDB_GPMC_CONFIG5	0x041D1F1F
#define SDPV2_MPDB_GPMC_CONFIG6	0x1D0904C4

#define MPDB_GPMC_CONFIG1	0x00011000
#define MPDB_GPMC_CONFIG2	0x001f1f01
#define MPDB_GPMC_CONFIG3	0x00080803
#define MPDB_GPMC_CONFIG4	0x1c0b1c0a
#define MPDB_GPMC_CONFIG5	0x041f1F1F
#define MPDB_GPMC_CONFIG6	0x1F0F04C4

#define P2_GPMC_CONFIG1	0x0
#define P2_GPMC_CONFIG2	0x0
#define P2_GPMC_CONFIG3	0x0
#define P2_GPMC_CONFIG4	0x0
#define P2_GPMC_CONFIG5	0x0
#define P2_GPMC_CONFIG6	0x0

#define ONENAND_GPMC_CONFIG1	0x00001200
#define ONENAND_GPMC_CONFIG2	0x000F0F01
#define ONENAND_GPMC_CONFIG3	0x00030301
#define ONENAND_GPMC_CONFIG4	0x0F040F04
#define ONENAND_GPMC_CONFIG5	0x010F1010
#define ONENAND_GPMC_CONFIG6	0x1F060000

#define NET_GPMC_CONFIG1	0x00001000
#define NET_GPMC_CONFIG2	0x001e1e01
#define NET_GPMC_CONFIG3	0x00080300
#define NET_GPMC_CONFIG4	0x1c091c09
#define NET_GPMC_CONFIG5	0x04181f1f
#define NET_GPMC_CONFIG6	0x00000FCF
#define NET_GPMC_CONFIG7	0x00000f6c

/* max number of GPMC Chip Selects */
#define GPMC_MAX_CS		8
/* max number of GPMC regs */
#define GPMC_MAX_REG		7

#define PISMO1_NOR		1
#define PISMO1_NAND		2
#define PISMO2_CS0		3
#define PISMO2_CS1		4
#define PISMO1_ONENAND		5
#define DBG_MPDB		6
#define PISMO2_NAND_CS0 	7
#define PISMO2_NAND_CS1 	8

/* make it readable for the gpmc_init */
#define PISMO1_NOR_BASE	FLASH_BASE
#define PISMO1_NAND_BASE	NAND_BASE
#define PISMO1_NAND_SIZE	GPMC_SIZE_256M

#endif /* endif _MEM_H_ */


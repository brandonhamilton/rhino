/*
 * (C) Copyright 2006
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
  */
#ifndef _OMAP34XX_CLOCKS_H_
#define _OMAP34XX_CLOCKS_H_

#define LDELAY      12000000

#define S12M		12000000
#define S13M		13000000
#define S19_2M		19200000
#define S24M		24000000
#define S26M		26000000
#define S38_4M		38400000

#define FCK_IVA2_ON     0x00000001
#define FCK_CORE1_ON    0x03fffe29
#define ICK_CORE1_ON    0x3ffffffb
#define ICK_CORE2_ON    0x0000001f
#define FCK_WKUP_ON     0x000000e9
#define ICK_WKUP_ON     0x0000003f
#define FCK_DSS_ON      0x00000005
#define ICK_DSS_ON      0x00000001
#define FCK_CAM_ON      0x00000001
#define ICK_CAM_ON      0x00000001
#define FCK_PER_ON      0x0003ffff
#define ICK_PER_ON      0x0003ffff

#include <asm/arch/clocks343x.h>

#endif

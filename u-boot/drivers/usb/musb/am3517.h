/*
 * TI's AM3517 platform specific USB wrapper functions.
 *
 * Copyright (c) 2009 Texas Instruments
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Author: Ajay Kumar Gupta ajay.gupta@ti.com, Texas Instruments
 */

#ifndef __AM3517_USB_H__
#define __AM3517_USB_H__

#include "musb_core.h"

/* Base address of musb wrapper */
#define MUSB_BASE 0x5C040000

/* Base address of musb core */
#define MENTOR_BASE (MUSB_BASE+0x400)

/* Base address of system control register used to program phy */
#define OMAP3517_CONF0	0x48002580
#define OMAP3517_IP_SW_RESET	0x48002598
/*
 * AM3517 platform USB wrapper register overlay. Note: Only the required
 * registers are included in this structure. It can be expanded as required.
 */
struct am3517_usb_regs {
	u32	revision;
	u32	control;
	u32	status;
	u32	emulation;
	u32	reserved0[1];
	u32	autoreq;
	u32	srpfixtime;
	u32	ep_intsrc;
	u32	ep_intsrcset;
	u32	ep_intsrcclr;
	u32	ep_intmsk;
	u32	ep_intmskset;
	u32	ep_intmskclr;
	u32	ep_intsrcmsked;
	u32	reserved1[1];
	u32	core_intsrc;
	u32	core_intsrcset;
	u32	core_intsrcclr;
	u32	core_intmsk;
	u32	core_intmskset;
	u32	core_intmskclr;
	u32	core_intsrcmsked;
	u32	reserved2[1];
	u32	eoi;
	u32	mop_sop_en;
	u32	reserved3[2];
	u32	txmode;
	u32	rxmode;
	u32	epcount_mode;
};

/* Control register bits */
#define USB_SOFT_RESET_MASK	1

/* Timeout for MUSB module */
#define MUSB_TIMEOUT 0x3FFFFFF

/* USB 2.0 PHY Control */
#define CONF0_PHY_GPIOMODE     (1 << 23)
#define CONF0_OTGMODE          (3 << 14)
#define CONF0_SESENDEN         (1 << 13)       /* Vsess_end comparator */
#define CONF0_VBDTCTEN         (1 << 12)       /* Vbus comparator */
#define CONF0_REFFREQ_24MHZ    (2 << 8)
#define CONF0_REFFREQ_26MHZ    (7 << 8)
#define CONF0_REFFREQ_13MHZ    (6 << 8)
#define CONF0_REFFREQ          (0xf << 8)
#define CONF0_PHYCLKGD         (1 << 7)
#define CONF0_VBUSSENSE        (1 << 6)
#define CONF0_PHY_PLLON        (1 << 5)        /* override PLL suspend */
#define CONF0_RESET            (1 << 4)
#define CONF0_PHYPWRDN         (1 << 3)
#define CONF0_OTGPWRDN         (1 << 2)
#define CONF0_DATPOL           (1 << 1)

#endif	/* __AM3517_USB_H__ */

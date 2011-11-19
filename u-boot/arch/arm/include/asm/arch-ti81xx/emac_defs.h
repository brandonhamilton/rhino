/*
 * Copyright (C) 2010 Texas Instruments
 *
 * Based on:
 *
 * ----------------------------------------------------------------------------
 *
 * dm644x_emac.h
 *
 * TI DaVinci (DM644X) EMAC peripheral driver header for DV-EVM
 *
 * Copyright (C) 2005 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------

 * Modifications:
 * ver. 1.0: Sep 2005, TI PSP Team - Created EMAC version for uBoot.
 * ver. 2.0: May 2010, TI PSP Team - Cleanup, move platform independent
 *				portion to driver header file.
 *
 */

#ifndef _EMAC_DEFS_H_
#define _EMAC_DEFS_H_

#include <asm/arch/hardware.h>

#ifdef CONFIG_TI81XX
#define EMAC_BASE_ADDR			(0x4A100000)
#define EMAC_WRAPPER_BASE_ADDR		(0x4A100900)
#define EMAC_WRAPPER_RAM_ADDR		(0x4A102000)
#define EMAC_MDIO_BASE_ADDR		(0x4A100800)
#define DAVINCI_EMAC_VERSION2
#define DAVINCI_EMAC_GIG_ENABLE

#define EMAC_MDIO_BUS_FREQ		(250000000UL)
#define EMAC_MDIO_CLOCK_FREQ		(2000000UL)

/*TODO: verify Phy address */
#define EMAC_MDIO_PHY_NUM		(1)
#define EMAC_MDIO_PHY_MASK		(1 << EMAC_MDIO_PHY_NUM)
#endif

#endif  /* _DM644X_EMAC_H_ */


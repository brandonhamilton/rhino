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

#include <common.h>
#include "am3517.h"

/* MUSB platform configuration */
struct musb_config musb_cfg = {
	(struct	musb_regs *)MENTOR_BASE,
	MUSB_TIMEOUT,
	0,
	0
};

/* MUSB module register overlay */
struct am3517_usb_regs *regs;

/*
 * Enable the USB phy
 */
static u8 phy_on(void)
{
	u32 cfgchip0;
	u32 timeout;

	/*
	 * Start the on-chip PHY and its PLL.
	 */
	cfgchip0 = readl(OMAP3517_CONF0);

	cfgchip0 &= ~(0x0000FFFF);
	writel(cfgchip0, OMAP3517_CONF0);
	cfgchip0 = readl(OMAP3517_CONF0);

	cfgchip0 &= ~(CONF0_RESET | CONF0_PHYPWRDN | CONF0_OTGPWRDN |
		      CONF0_OTGMODE | CONF0_REFFREQ | CONF0_PHY_GPIOMODE);
	cfgchip0 |= CONF0_SESENDEN | CONF0_VBDTCTEN | CONF0_PHY_PLLON |
		    CONF0_REFFREQ_13MHZ | CONF0_DATPOL;
	writel(cfgchip0, OMAP3517_CONF0);

	/* Wait until the USB phy is turned on */
	timeout = musb_cfg.timeout;
	while (timeout--)
		if (readl(OMAP3517_CONF0) & CONF0_PHYCLKGD)
			return 1;

	/* USB phy was not turned on */
	return 0;
}

/*
 * Disable the USB phy
 */
static void phy_off(void)
{
	u32 cfgchip0;

	/*
	 * Power down the on-chip PHY.
	 */
	cfgchip0 = readl(OMAP3517_CONF0);

	cfgchip0 &= ~CONF0_PHY_PLLON;
	cfgchip0 |= CONF0_PHYPWRDN | CONF0_OTGPWRDN;
	writel(cfgchip0, OMAP3517_CONF0);
}

/*
 * This function performs platform specific initialization for usb0.
 */
int musb_platform_init(void)
{
	u32 revision;
	u32 sw_reset;

	/* global usb reset */
	sw_reset = readl(OMAP3517_IP_SW_RESET);
	sw_reset |= (1 << 0);
	writel(sw_reset, OMAP3517_IP_SW_RESET);
	sw_reset &= ~(1 << 0);
	writel(sw_reset, OMAP3517_IP_SW_RESET);

	/* reset the controller */
	regs = (struct am3517_usb_regs *)MUSB_BASE;
	writel(USB_SOFT_RESET_MASK, &regs->control);
	udelay(5000);

	if (!phy_on())
		return -1;

	/* Returns zero if e.g. not clocked */
	revision = readl(&regs->revision);
	if (!revision)
		return -1;

	return 0;
}

/*
 * This function performs platform specific deinitialization for usb0.
 */
void musb_platform_deinit(void)
{
	/* Turn of the phy */
	phy_off();
}

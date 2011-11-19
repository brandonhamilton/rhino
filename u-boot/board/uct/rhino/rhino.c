/*
 * rhino.c - board file for the Rhino board (with an AM3517 processor)
 *
 * Author: Simon Scott, University of Cape Town
 * 
 * Based on board/logicpd/am3517evm/am3517evm.c by Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Copyright (C) 2011 University of Cape Town
 * Original Copyright (C) 2009 Texas Instruments Incorporated
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/emac_defs.h>
#include <asm/arch/gpio.h>
#include <i2c.h>
#include <asm/mach-types.h>
#include <asm/arch/omap_gpmc.h>
#include "rhino.h"


#define AM3517_IP_SW_RESET	0x48002598
#define CPGMACSS_SW_RST		(1 << 1)

#define FPGA_CS1_BASE		0x08000000
#define FPGA_CS2_BASE		0x10000000
#define FPGA_CS3_BASE		0x18000000
#define FPGA_CS4_BASE		0x20000000
#define FPGA_CS5_BASE		0x28000000
#define FPGA_CS6_BASE		0x38000000


/*
 * GPMC CS settings for the FPGA memory accesses
 */
static u32 gpmc_fpga_config[GPMC_MAX_REG] = {
	0x2C001201,
	0x00050500,
	0x00030301,
	0x05030503,
	0x00050608,
	0x04030000,
	0
};


/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	/* Now that we are in SRAM or SDRAM, finish GPMC initialisation for NAND flash */
	gpmc_init(); 
	
	/* Configure GPMC for FPGA memory accesses */
	enable_gpmc_cs_config(gpmc_fpga_config, &gpmc_cfg->cs[1], FPGA_CS1_BASE, GPMC_SIZE_128M);
	enable_gpmc_cs_config(gpmc_fpga_config, &gpmc_cfg->cs[2], FPGA_CS2_BASE, GPMC_SIZE_128M);
	enable_gpmc_cs_config(gpmc_fpga_config, &gpmc_cfg->cs[3], FPGA_CS3_BASE, GPMC_SIZE_128M);
	enable_gpmc_cs_config(gpmc_fpga_config, &gpmc_cfg->cs[4], FPGA_CS4_BASE, GPMC_SIZE_128M);
	enable_gpmc_cs_config(gpmc_fpga_config, &gpmc_cfg->cs[5], FPGA_CS5_BASE, GPMC_SIZE_128M);
	enable_gpmc_cs_config(gpmc_fpga_config, &gpmc_cfg->cs[6], FPGA_CS6_BASE, GPMC_SIZE_128M);

	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_RHINO;

	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/*
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 */
int misc_init_r(void)
{
	volatile unsigned int ctr;
	u32 reset;
	int i;

#ifdef CONFIG_DRIVER_OMAP34XX_I2C

/* If multiple I2C busses, we need to configure all of them */
#if defined(CONFIG_I2C_MULTI_BUS)
	for(i = CONFIG_SYS_MAX_I2C_BUS - 1; i >= 0; i--)
	{
		i2c_set_bus_num(i);
		i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	}
#else
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif

#endif

	dieid_num_r();


#if defined(CONFIG_DRIVER_TI_EMAC)

	/*Set nPWR_KILL high, to ensure power supply stays on*/
	omap_request_gpio(106);
	omap_set_gpio_direction(106, 0);
	omap_set_gpio_dataout(106, 1);

	/*Set PROGRAM_B high, to take FPGA out of reset*/
	omap_request_gpio(126);
	omap_set_gpio_direction(126, 0);
	omap_set_gpio_dataout(126, 1);

	/*On all boards after revision 1.0, FPGA_SUSPEND must be kept low */
#if CONFIG_RHINO_REV > 10
	omap_request_gpio(61);
	omap_set_gpio_direction(61, 0);
	omap_set_gpio_dataout(61, 0);
#endif

	/*Ensure Ethernet PHY is powered up*/
#if CONFIG_RHINO_REV == 10
	omap_request_gpio(61);
	omap_set_gpio_direction(61, 0);
	omap_set_gpio_dataout(61, 1);
#else
	omap_request_gpio(30);
	omap_set_gpio_direction(30, 0);
	omap_set_gpio_dataout(30, 1);
#endif

	/*Now reset the PHY */
	omap_request_gpio(65);
	omap_set_gpio_direction(65, 0);
	omap_set_gpio_dataout(65, 0);
	ctr  = 0;
	do{
		udelay(1000);
		ctr++;
		}while (ctr <300);

	omap_set_gpio_dataout(65, 1);
	ctr =0;

	/*Allow the PHY to stabilize and settle down */
	do{
		udelay(1000);
		ctr++;
		}while (ctr <300);


    	/*Enable the boot buffer. Note: this does not work on v1.0 boards*/
#if CONFIG_RHINO_REV > 10
	omap_request_gpio(11);
	omap_set_gpio_direction(11, 0);
	omap_set_gpio_dataout(11, 0);
#endif

	/*Ensure that the module is out of reset*/
	reset = readl(AM3517_IP_SW_RESET);
	reset &= (~CPGMACSS_SW_RST);
	writel(reset,AM3517_IP_SW_RESET);

#endif


	return 0;
}


/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_DRIVER_TI_EMAC)
	printf("davinci_emac_initialize\n");
	davinci_emac_initialize();
#endif
	return 0;
 }


/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_RHINO();
}

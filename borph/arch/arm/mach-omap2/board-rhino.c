/*
 * linux/arch/arm/mach-omap2/board-rhino.c
 * Board support for the RHINO platform
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 * Copyright (C) 2009 Texas Instruments Incorporated
 * Author: Ranjith Lohithakshan <ranjithl@ti.com>
 *
 * Based on mach-omap2/board-am3517evm.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as  published by the
 * Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/i2c/pca953x.h>
#include <linux/can/platform/ti_hecc.h>
#include <linux/regulator/machine.h>
#include <linux/davinci_emac.h>
#include <linux/spi/spi.h>

#include <mach/hardware.h>
#include <mach/am35xx.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/board.h>
#include <plat/common.h>
#include <plat/usb.h>
#include <plat/omap_hwmod.h>
#include <plat/mcspi.h>
#include <video/omapdss.h>
#include <video/omap-panel-generic-dpi.h>

#include "mux.h"
#include "control.h"

#define AM35XX_EVM_MDIO_FREQUENCY	(1000000)

static struct mdio_platform_data rhino_mdio_pdata = {
	.bus_freq	= AM35XX_EVM_MDIO_FREQUENCY,
};

static struct resource am3517_mdio_resources[] = {
	{
		.start  = AM35XX_IPSS_EMAC_BASE + AM35XX_EMAC_MDIO_OFFSET,
		.end    = AM35XX_IPSS_EMAC_BASE + AM35XX_EMAC_MDIO_OFFSET +
			  SZ_4K - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct platform_device am3517_mdio_device = {
	.name		= "davinci_mdio",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(am3517_mdio_resources),
	.resource	= am3517_mdio_resources,
	.dev.platform_data = &rhino_mdio_pdata,
};

static struct emac_platform_data rhino_emac_pdata = {
	.rmii_en        = 1,
};

static struct resource am3517_emac_resources[] = {
	{
		.start  = AM35XX_IPSS_EMAC_BASE,
		.end    = AM35XX_IPSS_EMAC_BASE + 0x2FFFF,
		.flags  = IORESOURCE_MEM,
	},
	{
		.start  = INT_35XX_EMAC_C0_RXTHRESH_IRQ,
		.end    = INT_35XX_EMAC_C0_RXTHRESH_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = INT_35XX_EMAC_C0_RX_PULSE_IRQ,
		.end    = INT_35XX_EMAC_C0_RX_PULSE_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = INT_35XX_EMAC_C0_TX_PULSE_IRQ,
		.end    = INT_35XX_EMAC_C0_TX_PULSE_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = INT_35XX_EMAC_C0_MISC_PULSE_IRQ,
		.end    = INT_35XX_EMAC_C0_MISC_PULSE_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device am3517_emac_device = {
	.name           = "davinci_emac",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(am3517_emac_resources),
	.resource       = am3517_emac_resources,
};

static void am3517_enable_ethernet_int(void)
{
	u32 regval;

	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = (regval | AM35XX_CPGMAC_C0_RX_PULSE_CLR |
		AM35XX_CPGMAC_C0_TX_PULSE_CLR | AM35XX_CPGMAC_C0_MISC_PULSE_CLR |
		AM35XX_CPGMAC_C0_RX_THRESH_CLR );
	omap_ctrl_writel(regval,AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
}

static void am3517_disable_ethernet_int(void)
{
	u32 regval;

	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = (regval | AM35XX_CPGMAC_C0_RX_PULSE_CLR |
		AM35XX_CPGMAC_C0_TX_PULSE_CLR);
	omap_ctrl_writel(regval,AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
}

static void rhino_ethernet_init(struct emac_platform_data *pdata)
{
	unsigned int regval;

	pdata->ctrl_reg_offset          = AM35XX_EMAC_CNTRL_OFFSET;
	pdata->ctrl_mod_reg_offset      = AM35XX_EMAC_CNTRL_MOD_OFFSET;
	pdata->ctrl_ram_offset          = AM35XX_EMAC_CNTRL_RAM_OFFSET;
	pdata->ctrl_ram_size            = AM35XX_EMAC_CNTRL_RAM_SIZE;
	pdata->version                  = EMAC_VERSION_2;
	pdata->hw_ram_addr              = AM35XX_EMAC_HW_RAM_ADDR;
	pdata->interrupt_enable 	    = am3517_enable_ethernet_int;
	pdata->interrupt_disable 	    = am3517_disable_ethernet_int;
	am3517_emac_device.dev.platform_data     = pdata;
	platform_device_register(&am3517_emac_device);
	platform_device_register(&am3517_mdio_device);
	clk_add_alias(NULL, dev_name(&am3517_mdio_device.dev), NULL, &am3517_emac_device.dev);

	regval = omap_ctrl_readl(AM35XX_CONTROL_IP_SW_RESET);
	regval = regval & (~(AM35XX_CPGMACSS_SW_RST));
	omap_ctrl_writel(regval,AM35XX_CONTROL_IP_SW_RESET);
	regval = omap_ctrl_readl(AM35XX_CONTROL_IP_SW_RESET);

	return ;
}

static int rhino_enable_tv(struct omap_dss_device *dssdev)
{
	return 0;
}

static void rhino_disable_tv(struct omap_dss_device *dssdev)
{
}

static struct omap_dss_device rhino_tv_device = {
	.type 				= OMAP_DISPLAY_TYPE_VENC,
	.name 				= "tv",
	.driver_name		= "venc",
	.phy.venc.type		= OMAP_DSS_VENC_TYPE_SVIDEO,
	.platform_enable	= rhino_enable_tv,
	.platform_disable	= rhino_disable_tv,
};

static int rhino_enable_dvi(struct omap_dss_device *dssdev)
{
	return 0;
}

static void rhino_disable_dvi(struct omap_dss_device *dssdev)
{

}

static struct panel_generic_dpi_data dvi_panel = {
	.name				= "generic",
	.platform_enable	= rhino_enable_dvi,
	.platform_disable	= rhino_disable_dvi,
};

static struct omap_dss_device rhino_dvi_device = {
	.type			= OMAP_DISPLAY_TYPE_DPI,
	.name			= "dvi",
	.driver_name	= "generic_dpi_panel",
	.data			= &dvi_panel,
	.phy.dpi.data_lines	= 24,
};

static struct omap_dss_device *rhino_dss_devices[] = {
	&rhino_tv_device,
	&rhino_dvi_device,
};

static struct omap_dss_board_info rhino_dss_data = {
	.num_devices	= ARRAY_SIZE(rhino_dss_devices),
	.devices		= rhino_dss_devices,
	.default_device	= &rhino_dvi_device,
};

/* TPS65023 specific initialization */
/* VDCDC1 -> VDD_CORE */
static struct regulator_consumer_supply rhino_vdcdc1_supplies[] = {
	{
		.supply = "vdd_core",
	},
};

/* VDCDC2 -> VDDSHV */
static struct regulator_consumer_supply rhino_vdcdc2_supplies[] = {
	{
		.supply = "vddshv",
	},
};

/* VDCDC2 |-> VDDS
	   |-> VDDS_SRAM_CORE_BG
	   |-> VDDS_SRAM_MPU */
static struct regulator_consumer_supply rhino_vdcdc3_supplies[] = {
	{
		.supply = "vdds",
	},
	{
		.supply = "vdds_sram_core_bg",
	},
	{
		.supply = "vdds_sram_mpu",
	},
};

/* LDO1 |-> VDDA1P8V_USBPHY
	 |-> VDDA_DAC */
static struct regulator_consumer_supply rhino_ldo1_supplies[] = {
	{
		.supply = "vdda1p8v_usbphy",
	},
	{
		.supply = "vdda_dac",
	},
};

/* LDO2 -> VDDA3P3V_USBPHY */
static struct regulator_consumer_supply rhino_ldo2_supplies[] = {
	{
		.supply = "vdda3p3v_usbphy",
	},
};

static struct regulator_init_data rhino_regulator_data[] = {
	/* DCDC1 */
	{
		.constraints = {
			.min_uV = 1200000,
			.max_uV = 1200000,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS,
			.always_on = true,
			.apply_uV = false,
		},
		.num_consumer_supplies = ARRAY_SIZE(rhino_vdcdc1_supplies),
		.consumer_supplies = rhino_vdcdc1_supplies,
	},
	/* DCDC2 */
	{
		.constraints = {
			.min_uV = 3300000,
			.max_uV = 3300000,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS,
			.always_on = true,
			.apply_uV = false,
		},
		.num_consumer_supplies = ARRAY_SIZE(rhino_vdcdc2_supplies),
		.consumer_supplies = rhino_vdcdc2_supplies,
	},
	/* DCDC3 */
	{
		.constraints = {
			.min_uV = 1800000,
			.max_uV = 1800000,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS,
			.always_on = true,
			.apply_uV = false,
		},
		.num_consumer_supplies = ARRAY_SIZE(rhino_vdcdc3_supplies),
		.consumer_supplies = rhino_vdcdc3_supplies,
	},
	/* LDO1 */
	{
		.constraints = {
			.min_uV = 1800000,
			.max_uV = 1800000,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS,
			.always_on = false,
			.apply_uV = false,
		},
		.num_consumer_supplies = ARRAY_SIZE(rhino_ldo1_supplies),
		.consumer_supplies = rhino_ldo1_supplies,
	},
	/* LDO2 */
	{
		.constraints = {
			.min_uV = 3300000,
			.max_uV = 3300000,
			.valid_modes_mask = REGULATOR_MODE_NORMAL,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS,
			.always_on = false,
			.apply_uV = false,
		},
		.num_consumer_supplies = ARRAY_SIZE(rhino_ldo2_supplies),
		.consumer_supplies = rhino_ldo2_supplies,
	},
};

static struct i2c_board_info __initdata rhino_i2c1_boardinfo[] = {
	{
		I2C_BOARD_INFO("tc654", 0x15),
		.type		= "tc654",
	},
	{
		I2C_BOARD_INFO("ina219", 0x20),
		.type		= "ina219",
	},
	{
		I2C_BOARD_INFO("tps65023", 0x48),
		.flags = I2C_CLIENT_WAKE,
		.platform_data = &rhino_regulator_data[0],
	},
};

static struct i2c_board_info __initdata rhino_dvi_i2c_eeprom[] = {
	{
		I2C_BOARD_INFO("eeprom", 0x50),
	},
};

static int __init rhino_i2c_init(void)
{
	/* I2C 1 - Power Management */
	omap_register_i2c_bus(1, 400, rhino_i2c1_boardinfo, ARRAY_SIZE(rhino_i2c1_boardinfo));
	/* I2C 2 - DDC Bus on HDMI connector */
	omap_register_i2c_bus(2, 100, rhino_dvi_i2c_eeprom, ARRAY_SIZE(rhino_dvi_i2c_eeprom));
	/* I2C 3 - FMC connectors */
	omap_register_i2c_bus(3, 400, NULL, 0);
	return 0;
}

static struct omap2_mcspi_device_config rhino_mcspi_config = {
	.turbo_mode	= 0,
	.single_channel	= 1,	/* 0: slave, 1: master */
};

#define	GPIO_RTCDS1390_IRQ	154

static struct spi_board_info rhino_spi_board_info[] __initdata = {
	[0] = {
		.modalias	= "rtc-ds1390",
		.max_speed_hz	= 48000000,
		.chip_select	= 0,
		.bus_num 		= 1,
		.mode 			= SPI_MODE_0,
	},
	[1] = {
		.modalias		= "tlv320aic23_spi",
		.max_speed_hz	= 4000000,
		.chip_select	= 1,
		.bus_num 		= 1,
		.mode 			= SPI_MODE_0,
	},
};

static struct spi_board_info rhino_spi2_board_info[] __initdata = {
	[0] = {
		.modalias		  = "rhino-spartan6",
		.max_speed_hz	  = 48000000,
		.chip_select	  = 0,
		.bus_num 	 	  = 2,
		.mode 			  = SPI_MODE_0,
		.controller_data  = &rhino_mcspi_config,
	},
};

static int __init rhino_spi_init(void)
{
	spi_register_board_info(rhino_spi_board_info, ARRAY_SIZE(rhino_spi_board_info));
	spi_register_board_info(rhino_spi2_board_info, ARRAY_SIZE(rhino_spi2_board_info));
	return 0;
}

static char *rhino_unused_hwmods[] = {
	"iva",
	"sr1_hwmod",
	"sr2_hwmod",
	"mailbox",
	"usb_otg_hs",
	NULL,
};

/*
 * Board initialization
 */
static void __init rhino_init_early(void)
{
	omap2_disable_unused_hwmods(rhino_unused_hwmods);
	omap2_init_common_infrastructure();
	omap2_init_common_devices(NULL, NULL);
}

static struct omap_musb_board_data musb_board_data = {
	.interface_type         = MUSB_INTERFACE_ULPI,
	.mode                   = MUSB_OTG,
	.power                  = 500,
	.set_phy_power			= am35x_musb_phy_power,
	.clear_irq				= am35x_musb_clear_irq,
	.set_mode				= am35x_set_mode,
	.reset					= am35x_musb_reset,
};

static __init void rhino_musb_init(void)
{
	u32 devconf2;

	/*
	 * Set up USB clock/mode in the DEVCONF2 register.
	 */
	devconf2 = omap_ctrl_readl(AM35XX_CONTROL_DEVCONF2);

	/* USB2.0 PHY reference clock is 13 MHz */
	devconf2 &= ~(CONF2_REFFREQ | CONF2_OTGMODE | CONF2_PHY_GPIOMODE);
	devconf2 |=  CONF2_REFFREQ_13MHZ | CONF2_SESENDEN | CONF2_VBDTCTEN
			| CONF2_DATPOL;

	omap_ctrl_writel(devconf2, AM35XX_CONTROL_DEVCONF2);

	usb_musb_init(&musb_board_data);
}

static const struct usbhs_omap_board_data usbhs_bdata __initconst = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,

	.phy_reset  = true,
	.reset_gpio_port[0]  = 57,
	.reset_gpio_port[1]  = -EINVAL,
	.reset_gpio_port[2]  = -EINVAL
};

static struct resource am3517_hecc_resources[] = {
	{
		.start	= AM35XX_IPSS_HECC_BASE,
		.end	= AM35XX_IPSS_HECC_BASE + 0x3FFF,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_35XX_HECC0_IRQ,
		.end	= INT_35XX_HECC0_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device am3517_hecc_device = {
	.name		= "ti_hecc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(am3517_hecc_resources),
	.resource	= am3517_hecc_resources,
};

static struct ti_hecc_platform_data rhino_hecc_pdata = {
	.scc_hecc_offset	= AM35XX_HECC_SCC_HECC_OFFSET,
	.scc_ram_offset		= AM35XX_HECC_SCC_RAM_OFFSET,
	.hecc_ram_offset	= AM35XX_HECC_RAM_OFFSET,
	.mbx_offset		= AM35XX_HECC_MBOX_OFFSET,
	.int_line		= AM35XX_HECC_INT_LINE,
	.version		= AM35XX_HECC_VERSION,
};

static void rhino_hecc_init(struct ti_hecc_platform_data *pdata)
{
	am3517_hecc_device.dev.platform_data = pdata;
	platform_device_register(&am3517_hecc_device);
}

static struct omap_board_config_kernel rhino_config[] __initdata = {
};

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	/* USB OTG DRVVBUS offset = 0x212 */
	OMAP3_MUX(SAD2D_MCAD23, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#endif

static void __init rhino_init(void)
{
	omap_board_config = rhino_config;
	omap_board_config_size = ARRAY_SIZE(rhino_config);
	omap3_mux_init(board_mux, OMAP_PACKAGE_CBB);

	rhino_i2c_init();		
	omap_display_init(&rhino_dss_data);
	omap_serial_init();

	/* Configure GPIO for EHCI port */
	omap_mux_init_gpio(57, OMAP_PIN_OUTPUT);
	usbhs_init(&usbhs_bdata);
	rhino_hecc_init(&rhino_hecc_pdata);

	/* RTC */
	omap_mux_init_gpio(GPIO_RTCDS1390_IRQ, OMAP_PIN_INPUT_PULLDOWN);
	if (gpio_request(GPIO_RTCDS1390_IRQ, "rtcs35390a-irq") < 0)
		printk(KERN_WARNING "failed to request GPIO#%d\n", GPIO_RTCDS1390_IRQ);
	if (gpio_direction_input(GPIO_RTCDS1390_IRQ))
		printk(KERN_WARNING "GPIO#%d cannot be configured as input\n", GPIO_RTCDS1390_IRQ);
	rhino_spi_board_info[0].irq = gpio_to_irq(GPIO_RTCDS1390_IRQ);
	
	rhino_spi_init();

	/*Ethernet*/
	rhino_ethernet_init(&rhino_emac_pdata);
	
	/* MUSB */
	rhino_musb_init();
}

MACHINE_START(RHINO, "RHINO v1")
	.boot_params	= 0x80000100,
	.reserve		= omap_reserve,
	.map_io			= omap3_map_io,
	.init_early		= rhino_init_early,
	.init_irq		= omap3_init_irq,
	.init_machine	= rhino_init,
	.timer			= &omap3_timer,
MACHINE_END

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
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <linux/can/platform/ti_hecc.h>
#include <linux/regulator/machine.h>
#include <linux/davinci_emac.h>
#include <linux/spi/spi.h>
#include <linux/mmc/host.h>

#include <mach/hardware.h>
#include <mach/am35xx.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/board.h>
#include <plat/common.h>
#include <plat/usb.h>
#include <plat/display.h>
#include <plat/gpmc.h>
#include <plat/nand.h>
#include <plat/mcspi.h>


#include "mux.h"
#include "control.h"
#include "hsmmc.h"
#include "board-flash.h"

#define GPMC_CS0_BASE  0x60
#define GPMC_CS_SIZE   0x30

/************************************************
 * NAND Flash                                   *
 ************************************************/
#define NAND_BLOCK_SIZE        SZ_128K

static struct mtd_partition rhino_nand_partitions[] = {
/* All the partition sizes are listed in terms of NAND block size */
{
       .name           = "xloader-nand",
       .offset         = 0,
       .size           = 4*(SZ_128K),
       .mask_flags     = MTD_WRITEABLE
},
{
       .name           = "uboot-nand",
       .offset         = MTDPART_OFS_APPEND,
       .size           = 15*(SZ_128K),
       .mask_flags     = MTD_WRITEABLE
},
{
       .name           = "params-nand",
       .offset         = MTDPART_OFS_APPEND,
       .size           = SZ_128K
},
{
       .name           = "linux-nand",
       .offset         = MTDPART_OFS_APPEND,
       .size           = 40*(SZ_128K)
},
{
       .name           = "jffs2-nand",
       .size           = MTDPART_SIZ_FULL,
       .offset         = MTDPART_OFS_APPEND,
},
};

/************************************************
 * Ethernet                                     *
 ************************************************/

#define AM35XX_EVM_MDIO_FREQUENCY    	(1000000)

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
	.name		       = "davinci_mdio",
	.id		           = -1,
	.num_resources	   = ARRAY_SIZE(am3517_mdio_resources),
	.resource	       = am3517_mdio_resources,
	.dev.platform_data = &rhino_mdio_pdata,
};

static struct emac_platform_data rhino_emac_pdata = {
	.rmii_en	= 1,
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
		AM35XX_CPGMAC_C0_TX_PULSE_CLR |
		AM35XX_CPGMAC_C0_MISC_PULSE_CLR |
		AM35XX_CPGMAC_C0_RX_THRESH_CLR);
	omap_ctrl_writel(regval, AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
}

static void am3517_disable_ethernet_int(void)
{
	u32 regval;

	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = (regval | AM35XX_CPGMAC_C0_RX_PULSE_CLR |
		AM35XX_CPGMAC_C0_TX_PULSE_CLR);
	omap_ctrl_writel(regval, AM35XX_CONTROL_LVL_INTR_CLEAR);
	regval = omap_ctrl_readl(AM35XX_CONTROL_LVL_INTR_CLEAR);
}

void rhino_ethernet_init(struct emac_platform_data *pdata)
{
	u32 regval, mac_lo, mac_hi;

	mac_lo = omap_ctrl_readl(AM35XX_CONTROL_FUSE_EMAC_LSB);
	mac_hi = omap_ctrl_readl(AM35XX_CONTROL_FUSE_EMAC_MSB);

	pdata->mac_addr[0] = (u_int8_t)((mac_hi & 0xFF0000) >> 16);
	pdata->mac_addr[1] = (u_int8_t)((mac_hi & 0xFF00) >> 8);
	pdata->mac_addr[2] = (u_int8_t)((mac_hi & 0xFF) >> 0);
	pdata->mac_addr[3] = (u_int8_t)((mac_lo & 0xFF0000) >> 16);
	pdata->mac_addr[4] = (u_int8_t)((mac_lo & 0xFF00) >> 8);
	pdata->mac_addr[5] = (u_int8_t)((mac_lo & 0xFF) >> 0);

	pdata->ctrl_reg_offset		= AM35XX_EMAC_CNTRL_OFFSET;
	pdata->ctrl_mod_reg_offset	= AM35XX_EMAC_CNTRL_MOD_OFFSET;
	pdata->ctrl_ram_offset		= AM35XX_EMAC_CNTRL_RAM_OFFSET;
	pdata->ctrl_ram_size		= AM35XX_EMAC_CNTRL_RAM_SIZE;
	pdata->version			    = EMAC_VERSION_2;
	pdata->hw_ram_addr		    = AM35XX_EMAC_HW_RAM_ADDR;
	pdata->interrupt_enable		= am3517_enable_ethernet_int;
	pdata->interrupt_disable	= am3517_disable_ethernet_int;
	am3517_emac_device.dev.platform_data	= pdata;
	platform_device_register(&am3517_mdio_device);
	platform_device_register(&am3517_emac_device);
	clk_add_alias(NULL, dev_name(&am3517_mdio_device.dev),
		      NULL, &am3517_emac_device.dev);

	regval = omap_ctrl_readl(AM35XX_CONTROL_IP_SW_RESET);
	regval = regval & (~(AM35XX_CPGMACSS_SW_RST));
	omap_ctrl_writel(regval, AM35XX_CONTROL_IP_SW_RESET);
	regval = omap_ctrl_readl(AM35XX_CONTROL_IP_SW_RESET);

	return ;
}

/************************************************
 * Display                                      *
 ************************************************/

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

static struct omap_dss_device rhino_dvi_device = {
	.type				= OMAP_DISPLAY_TYPE_DPI,
	.name				= "dvi",
	.driver_name		= "generic_panel",
	.phy.dpi.data_lines	= 24,
	.platform_enable	= rhino_enable_dvi,
	.platform_disable	= rhino_disable_dvi,
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

static struct platform_device rhino_dss_device = {
		.name		= "omapdss",
		.id		= -1,
		.dev		= {
			.platform_data	= &rhino_dss_data,
		},
};


/************************************************
 * I2C devices                                  *
 ************************************************/

static struct i2c_board_info __initdata rhino_i2c1_boardinfo[] = {
	{
		I2C_BOARD_INFO("tc654", 0x15),
		.type		= "tc654",
	},
	{
		I2C_BOARD_INFO("ina219", 0x20),
		.type		= "ina219",
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

/************************************************
 * SPI devices                                  *
 ************************************************/

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

/************************************************
 * CAN controller                               *
 ************************************************/

#define CAN_STB         214
static void hecc_phy_control(int on)
{
        int r;

        r = gpio_request(CAN_STB, "can_stb");
        if (r) {
                printk(KERN_ERR "failed to get can_stb \n");
                return;
        }

        gpio_direction_output(CAN_STB, (on==1)?0:1);
}

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
	.transceiver_switch     = hecc_phy_control,
};

static void rhino_hecc_init(struct ti_hecc_platform_data *pdata)
{
	am3517_hecc_device.dev.platform_data = pdata;
	platform_device_register(&am3517_hecc_device);
}

/************************************************
 * Board Initialization                         *
 ************************************************/
static struct omap_board_config_kernel rhino_config[] __initdata = {
};

static struct platform_device *rhino_devices[] __initdata = {
	&rhino_dss_device,
};

static void __init rhino_init_irq(void)
{
	omap_board_config = rhino_config;
	omap_board_config_size = ARRAY_SIZE(rhino_config);

	omap2_init_common_infrastructure();
	omap2_init_common_devices(NULL, NULL);
	omap_init_irq();
	gpmc_init();
}

static struct omap2_hsmmc_info mmc[] = {
	{
		.mmc		= 2,
		.caps		= MMC_CAP_8_BIT_DATA,
		.gpio_cd	= 137,
		.gpio_wp	= 136,
	},
	{}      /* Terminator */
};

static struct omap_musb_board_data musb_board_data = {
	.interface_type         = MUSB_INTERFACE_ULPI,
	.mode                   = MUSB_OTG,
	.power                  = 500,
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

static const struct ehci_hcd_omap_platform_data ehci_pdata __initconst = {
	.port_mode[0] = EHCI_HCD_OMAP_MODE_PHY,
	.port_mode[1] = EHCI_HCD_OMAP_MODE_PHY,
	.port_mode[2] = EHCI_HCD_OMAP_MODE_UNKNOWN,

	.phy_reset  = true,
	.reset_gpio_port[0]  = 57,
	.reset_gpio_port[1]  = -EINVAL,
	.reset_gpio_port[2]  = -EINVAL
};

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#else
#define board_mux	NULL
#endif

static void __init rhino_init(void)
{
	/* Initialize MUX overrides */
	omap3_mux_init(board_mux, OMAP_PACKAGE_CBB);

	/* Initialize I2C lines */
	rhino_i2c_init();
	
	/* Add all platform devices */
	platform_add_devices(rhino_devices, ARRAY_SIZE(rhino_devices));

	/* Initialize the serial device */
	omap_serial_init();
	
	/* Configure GPIO for EHCI port */
	omap_mux_init_gpio(57, OMAP_PIN_OUTPUT);
	usb_ehci_init(&ehci_pdata);
	rhino_hecc_init(&rhino_hecc_pdata);

	/* Initialize the NAND flash */
	board_nand_init(rhino_nand_partitions, ARRAY_SIZE(rhino_nand_partitions), 0, NAND_BUSWIDTH_16);

	/* Configure the RTC */
	omap_mux_init_gpio(GPIO_RTCDS1390_IRQ, OMAP_PIN_INPUT_PULLDOWN);
	if (gpio_request(GPIO_RTCDS1390_IRQ, "rtcs35390a-irq") < 0)
		printk(KERN_WARNING "failed to request GPIO#%d\n", GPIO_RTCDS1390_IRQ);
	if (gpio_direction_input(GPIO_RTCDS1390_IRQ))
		printk(KERN_WARNING "GPIO#%d cannot be configured as input\n", GPIO_RTCDS1390_IRQ);
	rhino_spi_board_info[0].irq = gpio_to_irq(GPIO_RTCDS1390_IRQ);
	
	/* Initialize SPI devices */
	rhino_spi_init();
		
	/* Initialize Ethernet */
	rhino_ethernet_init(&rhino_emac_pdata);

	rhino_musb_init();

	/* MMC init function */
	omap2_hsmmc_init(mmc);
}

MACHINE_START(RHINO, "RHINO v1")
	.boot_params	= 0x80000100,
	.map_io			= omap3_map_io,
	.reserve	    = omap_reserve,
	.init_irq		= rhino_init_irq,
	.init_machine	= rhino_init,
	.timer			= &omap_timer,
MACHINE_END

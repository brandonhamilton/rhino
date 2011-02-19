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

#include <mach/hardware.h>
#include <mach/am35xx.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/board.h>
#include <plat/control.h>
#include <plat/common.h>
#include <plat/usb.h>
#include <plat/display.h>
#include <plat/gpmc.h>
#include <plat/nand.h>

#include "mmc-am3517evm.h"
#include "mux.h"

#define GPMC_CS0_BASE  0x60
#define GPMC_CS_SIZE   0x30

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
       .size           = 14*(SZ_128K),
       .mask_flags     = MTD_WRITEABLE
},
{
       .name           = "params-nand",
       .offset         = MTDPART_OFS_APPEND,
       .size           = 2*(SZ_128K)
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

static struct omap_nand_platform_data rhino_nand_data = {
       .parts          = rhino_nand_partitions,
       .nr_parts       = ARRAY_SIZE(rhino_nand_partitions),
       .nand_setup     = NULL,
       .dma_channel    = -1,           /* disable DMA in OMAP NAND driver */
       .dev_ready      = NULL,
};

static struct resource rhino_nand_resource = {
       .flags          = IORESOURCE_MEM,
};

static struct platform_device rhino_nand_device = {
       .name           = "omap2-nand",
       .id             = 0,
       .dev            = {
                       .platform_data  = &rhino_nand_data,
       },
       .num_resources  = 1,
       .resource       = &rhino_nand_resource,
};

void __init rhino_flash_init(void)
{
       u8 cs = 0;
       u8 nandcs = GPMC_CS_NUM + 1;
       u32 gpmc_base_add = OMAP34XX_GPMC_VIRT;

       while (cs < GPMC_CS_NUM) {
               u32 ret = 0;
               ret = gpmc_cs_read_reg(cs, GPMC_CS_CONFIG1);

               if ((ret & 0xC00) == 0x800) {
                       /* Found it!! */
                       if (nandcs > GPMC_CS_NUM)
                               nandcs = cs;
               }
               cs++;
       }
       if (nandcs > GPMC_CS_NUM) {
               printk(KERN_INFO "NAND: Unable to find configuration "
                       " in GPMC\n ");
               return;
       }

       if (nandcs < GPMC_CS_NUM) {
               rhino_nand_data.cs   = nandcs;
               rhino_nand_data.gpmc_cs_baseaddr = (void *)(gpmc_base_add +
                                       GPMC_CS0_BASE + nandcs*GPMC_CS_SIZE);
               rhino_nand_data.gpmc_baseaddr   = (void *) (gpmc_base_add);

               if (platform_device_register(&rhino_nand_device) < 0)
                       printk(KERN_ERR "Unable to register NAND device\n");

       }
}


#define AM35XX_EVM_PHY_MASK		(0xF)
#define AM35XX_EVM_MDIO_FREQUENCY    	(1000000)

static struct emac_platform_data rhino_emac_pdata = {
	.phy_mask       = AM35XX_EVM_PHY_MASK,
	.mdio_max_freq  = AM35XX_EVM_MDIO_FREQUENCY,
	.rmii_en        = 1,
};

static int __init eth_addr_setup(char *str)
{
	int i;

	if(str == NULL)
		return 0;
	for(i = 0; i <  ETH_ALEN; i++)
		rhino_emac_pdata.mac_addr[i] = simple_strtol(&str[i*3],
							(char **)NULL, 16);
	return 1;
}

/* Get MAC address from kernel boot parameter eth=AA:BB:CC:DD:EE:FF */
__setup("eth=", eth_addr_setup);

static struct resource am3517_emac_resources[] = {
	{
		.start  = AM35XX_IPSS_EMAC_BASE,
		.end    = AM35XX_IPSS_EMAC_BASE + 0x3FFFF,
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

void rhino_ethernet_init(struct emac_platform_data *pdata)
{
	unsigned int regval;

	pdata->ctrl_reg_offset          = AM35XX_EMAC_CNTRL_OFFSET;
	pdata->ctrl_mod_reg_offset      = AM35XX_EMAC_CNTRL_MOD_OFFSET;
	pdata->ctrl_ram_offset          = AM35XX_EMAC_CNTRL_RAM_OFFSET;
	pdata->mdio_reg_offset          = AM35XX_EMAC_MDIO_OFFSET;
	pdata->ctrl_ram_size            = AM35XX_EMAC_CNTRL_RAM_SIZE;
	pdata->version                  = EMAC_VERSION_2;
	pdata->hw_ram_addr              = AM35XX_EMAC_HW_RAM_ADDR;
	pdata->interrupt_enable 	= am3517_enable_ethernet_int;
	pdata->interrupt_disable 	= am3517_disable_ethernet_int;
	am3517_emac_device.dev.platform_data     = pdata;
	platform_device_register(&am3517_emac_device);

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
	{
		I2C_BOARD_INFO("tlv320aic23", 0xA1),
		.type		= "tlv320aic23",
	},
};

static int __init rhino_i2c_init(void)
{
	/* I2C 1 - Power Management */
	omap_register_i2c_bus(1, 400, rhino_i2c1_boardinfo, ARRAY_SIZE(rhino_i2c1_boardinfo));
	/* I2C 2 - DDC Bus on HDMI connector */
	omap_register_i2c_bus(2, 400, NULL, 0);
	/* I2C 3 - FMC connectors */
	omap_register_i2c_bus(3, 400, NULL, 0);
	return 0;
}


static struct spi_board_info rhino_spi_board_info[] __initdata = {
	[0] = {
		.modalias	= "rtc-ds1390",
		.max_speed_hz	= 4000000,
		.chip_select	= 0,
		.bus_num 		= 1,
		.mode = SPI_MODE_1,
	},
	[1] = {
		.modalias	= "tlv320aic23",
		.max_speed_hz	= 4000000,
		.chip_select	= 1,
		.bus_num 		= 1,
		.mode 			= SPI_MODE_1,
	},
};

/*
 * Board initialization
 */
static struct omap_board_config_kernel rhino_config[] __initdata = {
};

static struct platform_device *rhino_devices[] __initdata = {
	&rhino_dss_device,
};

static void __init rhino_init_irq(void)
{
	omap_board_config = rhino_config;
	omap_board_config_size = ARRAY_SIZE(rhino_config);

	omap2_init_common_hw(NULL, NULL, NULL, NULL, NULL);
	omap_init_irq();
	omap_gpio_init();
}

static struct am3517_hsmmc_info mmc[] = {
	{
		.mmc            = 1,
		.wires          = 4,
		.gpio_cd        = 127,
		.gpio_wp        = 126,
	},
	{
		.mmc            = 2,
		.wires          = 4,
		.gpio_cd        = 128,
		.gpio_wp        = 129,
	},
	{}      /* Terminator */
};

static __init void rhino_musb_init(void)
{
	usb_musb_init();
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
	/* USB OTG DRVVBUS offset = 0x212 */
	OMAP3_MUX(CHASSIS_DMAREQ3, OMAP_MUX_MODE0 | OMAP_PIN_INPUT_PULLDOWN),
	OMAP3_MUX(MCBSP_CLKS, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP),
	OMAP3_MUX(GPMC_NCS4, OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLDOWN),
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#else
#define board_mux	NULL
#endif


#define CAN_STB		214
static void am3517_hecc_plat_init(void)
{
	int r;

    r = gpio_request(CAN_STB, "can_stb");
    if (r) {
        printk(KERN_ERR "failed to get can_stb \n");
		return;
    }

   gpio_direction_output(CAN_STB, 0);
}

static struct resource am3517_hecc_resources[] = {
        {
                .start  = AM35XX_IPSS_HECC_BASE,
                .end    = AM35XX_IPSS_HECC_BASE + 0x3FFF,
                .flags  = IORESOURCE_MEM,
        },
        {
                .start  = INT_35XX_HECC0_IRQ,
                .end    = INT_35XX_HECC0_IRQ,
                .flags  = IORESOURCE_IRQ,
        },
};

static struct platform_device am3517_hecc_device = {
        .name           = "ti_hecc",
        .id             = 1,
        .num_resources  = ARRAY_SIZE(am3517_hecc_resources),
        .resource       = am3517_hecc_resources,
};

static struct ti_hecc_platform_data rhino_hecc_pdata = {
        .scc_hecc_offset        = AM35XX_HECC_SCC_HECC_OFFSET,
        .scc_ram_offset         = AM35XX_HECC_SCC_RAM_OFFSET,
        .hecc_ram_offset        = AM35XX_HECC_RAM_OFFSET,
        .mbx_offset            = AM35XX_HECC_MBOX_OFFSET,
        .int_line               = AM35XX_HECC_INT_LINE,
        .version                = AM35XX_HECC_VERSION,
		.platform_init		= am3517_hecc_plat_init,
};

static void rhino_hecc_init(struct ti_hecc_platform_data *pdata)
{
        am3517_hecc_device.dev.platform_data = pdata;
        platform_device_register(&am3517_hecc_device);
}

static void __init rhino_init(void)
{
	rhino_i2c_init();

	spi_register_board_info(rhino_spi_board_info, ARRAY_SIZE(rhino_spi_board_info));
	omap3_mux_init(board_mux, OMAP_PACKAGE_CBB);
	platform_add_devices(rhino_devices,
				ARRAY_SIZE(rhino_devices));

	omap_serial_init();
	rhino_flash_init();
	rhino_musb_init();
	/* Configure GPIO for EHCI port */
	omap_mux_init_gpio(57, OMAP_PIN_OUTPUT);
	usb_ehci_init(&ehci_pdata);

	rhino_hecc_init(&rhino_hecc_pdata);
	
	/*Ethernet*/
	rhino_ethernet_init(&rhino_emac_pdata);
	
	/* MMC init function */
	am3517_mmc_init(mmc);
}

static void __init rhino_map_io(void)
{
	omap2_set_globals_343x();
	omap2_map_common_io();
}

MACHINE_START(RHINO, "RHINO v1")
	.phys_io	= 0x48000000,
	.io_pg_offst	= ((0xd8000000) >> 18) & 0xfffc,
	.boot_params	= 0x80000100,
	.map_io			= rhino_map_io,
	.init_irq		= rhino_init_irq,
	.init_machine	= rhino_init,
	.timer			= &omap_timer,
MACHINE_END

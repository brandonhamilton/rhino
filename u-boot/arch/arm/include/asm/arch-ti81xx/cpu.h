/*
 * (C) Copyright 2006
 * Texas Instruments, <www.ti.com>
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
 *
 */

#ifndef _TI81XX_CPU_H
#define _TI81XX_CPU_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>
#endif /* !(__KERNEL_STRICT_NAMES || __ASSEMBLY__) */

#include <asm/arch/hardware.h>

#define BIT(x)				(1 << x)
#define CL_BIT(x)			(0 << x)

/* Timer registers */
#define TIMER_TCLR			0x38		/* Timer control register */
#define TIMER_TCRR			0x3C		/* Timer counter register */
#define TIMER_TLDR			0x40		/* Timer load value register*/

/* Timer register bits */
#define TCLR_ST				BIT(0)		/* Start=1 Stop=0 */
#define TCLR_AR				BIT(1)		/* Auto reload */
#define TCLR_PRE			BIT(5)		/* Pre-scaler enable */
#define TCLR_PTV_SHIFT			(2)		/* Pre-scaler shift value */
#define TCLR_PRE_DISABLE		CL_BIT(5)	/* Pre-scalar disable */

/* Control */
#define CONTROL_STATUS			(CTRL_BASE + 0x40)

/* device type */
#define DEVICE_MASK			(BIT(8) | BIT(9) | BIT(10))
#define TST_DEVICE			0x0
#define EMU_DEVICE			0x1
#define HS_DEVICE			0x2
#define GP_DEVICE			0x3

/* cpu-id for TI81XX family */
#define TI8168				0xb81e

#define DEVICE_ID			(CTRL_BASE + 0x0600)
/* This gives the status of the boot mode pins on the evm */
#define SYSBOOT_MASK			(BIT(0) | BIT(1) | BIT(2) |BIT(3) |BIT(4))

/* Reset control */
#define PRM_DEVICE_RSTCTRL		(PRCM_BASE + 0x00A0)

/* TI816X specific bits for PRM_DEVICE module */
#define GLOBAL_RST_COLD			BIT(1)

/* PLL related registers */
#ifdef CONFIG_TI816X
#define MAINPLL_CTRL			(CTRL_BASE + 0x0400)
#define MAINPLL_PWD			(CTRL_BASE + 0x0404)
#define MAINPLL_FREQ1			(CTRL_BASE + 0x0408)
#define MAINPLL_DIV1			(CTRL_BASE + 0x040C)
#define MAINPLL_FREQ2			(CTRL_BASE + 0x0410)
#define MAINPLL_DIV2			(CTRL_BASE + 0x0414)
#define MAINPLL_FREQ3			(CTRL_BASE + 0x0418)
#define MAINPLL_DIV3			(CTRL_BASE + 0x041C)
#define MAINPLL_FREQ4			(CTRL_BASE + 0x0420)
#define MAINPLL_DIV4			(CTRL_BASE + 0x0424)
#define MAINPLL_FREQ5			(CTRL_BASE + 0x0428)
#define MAINPLL_DIV5			(CTRL_BASE + 0x042C)
#define MAINPLL_DIV6			(CTRL_BASE + 0x0434)
#define MAINPLL_DIV7			(CTRL_BASE + 0x043C)

#define DDRPLL_CTRL			(CTRL_BASE + 0x0440)
#define DDRPLL_PWD			(CTRL_BASE + 0x0444)
#define DDRPLL_DIV1			(CTRL_BASE + 0x044C)
#define DDRPLL_FREQ2			(CTRL_BASE + 0x0450)
#define DDRPLL_DIV2			(CTRL_BASE + 0x0454)
#define DDRPLL_FREQ3			(CTRL_BASE + 0x0458)
#define DDRPLL_DIV3			(CTRL_BASE + 0x045C)
#define DDRPLL_FREQ4			(CTRL_BASE + 0x0460)
#define DDRPLL_DIV4			(CTRL_BASE + 0x0464)
#define DDRPLL_FREQ5			(CTRL_BASE + 0x0468)
#define DDRPLL_DIV5			(CTRL_BASE + 0x046C)

#define DDR_RCD				(CTRL_BASE + 0x070C)

#define VIDEOPLL_CTRL			(CTRL_BASE + 0x0470)
#define VIDEOPLL_PWD			(CTRL_BASE + 0x0474)
#define VIDEOPLL_FREQ1			(CTRL_BASE + 0x0478)
#define VIDEOPLL_DIV1			(CTRL_BASE + 0x047C)
#define VIDEOPLL_FREQ2			(CTRL_BASE + 0x0480)
#define VIDEOPLL_DIV2			(CTRL_BASE + 0x0484)
#define VIDEOPLL_FREQ3			(CTRL_BASE + 0x0488)
#define VIDEOPLL_DIV3			(CTRL_BASE + 0x048C)

#define AUDIOPLL_CTRL			(CTRL_BASE + 0x04A0)
#define AUDIOPLL_PWD			(CTRL_BASE + 0x04A4)
#define AUDIOPLL_FREQ2			(CTRL_BASE + 0x04B0)
#define AUDIOPLL_DIV2			(CTRL_BASE + 0x04B4)
#define AUDIOPLL_FREQ3			(CTRL_BASE + 0x04B8)
#define AUDIOPLL_DIV3			(CTRL_BASE + 0x04BC)
#define AUDIOPLL_FREQ4			(CTRL_BASE + 0x04C0)
#define AUDIOPLL_DIV4			(CTRL_BASE + 0x04C4)
#define AUDIOPLL_FREQ5			(CTRL_BASE + 0x04C8)
#define AUDIOPLL_DIV5			(CTRL_BASE + 0x04CC)

#endif

#ifdef CONFIG_TI814X

#define PCIE_PLLCFG0			(CTRL_BASE + 0x6D8)
#define PCIE_PLLCFG1			(CTRL_BASE + 0x6DC)
#define PCIE_PLLCFG2			(CTRL_BASE + 0x6E0)
#define PCIE_PLLCFG3			(CTRL_BASE + 0x6E4)
#define PCIE_PLLCFG4			(CTRL_BASE + 0x6E8)
#define PCIE_PLLSTATUS			(CTRL_BASE + 0x6EC)
#define PCIE_RXSTATUS			(CTRL_BASE + 0x6F0)
#define PCIE_TXSTATUS			(CTRL_BASE + 0x6F4)
#define SERDES_REFCLK_CTRL		(CTRL_BASE + 0xE24)

#define SATA_PLLCFG0			(CTRL_BASE + 0x720)
#define SATA_PLLCFG1			(CTRL_BASE + 0x724)
#define SATA_PLLCFG2			(CTRL_BASE + 0x728)
#define SATA_PLLCFG3			(CTRL_BASE + 0x72C)
#define SATA_PLLCFG4			(CTRL_BASE + 0x730)
#define SATA_PLLSTATUS			(CTRL_BASE + 0x734)
#define SATA_RXSTATUS			(CTRL_BASE + 0x738)
#define SATA_TXSTATUS			(CTRL_BASE + 0x73C)

/* pin muxing registers */
#define PIN_CTRL_BASE			(CTRL_BASE + 0x800)
#define N_PINS				(271) /* PIN1=800, PIN 271=800+270*4=C38) */

/* Clocks are derived from ADPLLJ */
#define ADPLLJ_CLKCTRL			0x4
#define ADPLLJ_TENABLE			0x8
#define ADPLLJ_TENABLEDIV		0xC
#define ADPLLJ_M2NDIV			0x10
#define ADPLLJ_MN2DIV			0x14
#define ADPLLJ_STATUS			0x24

/* ADPLLJ register values */
#define ADPLLJ_CLKCTRL_HS2		0x00000801 /* HS2 mode, TINT2 = 1 */
#define ADPLLJ_CLKCTRL_HS1		0x00001001 /* HS1 mode, TINT2 = 1 */
#define ADPLLJ_CLKCTRL_CLKDCO		0x200A0000 /* Enable CLKDCOEN, CLKLDOEN, CLKDCOPWDNZ */

#define MODENA_PLL_BASE			(PLL_SUBSYS_BASE + 0x048)
#define DSP_PLL_BASE			(PLL_SUBSYS_BASE + 0x080)
#define SGX_PLL_BASE			(PLL_SUBSYS_BASE + 0x0B0)
#define IVA_PLL_BASE			(PLL_SUBSYS_BASE + 0x0E0)
#define L3_PLL_BASE			(PLL_SUBSYS_BASE + 0x110)
#define ISS_PLL_BASE			(PLL_SUBSYS_BASE + 0x140)
#define DSS_PLL_BASE			(PLL_SUBSYS_BASE + 0x170)
#define VIDEO0_PLL_BASE			(PLL_SUBSYS_BASE + 0x1A0)
#define VIDEO1_PLL_BASE			(PLL_SUBSYS_BASE + 0x1D0)
#define HDMI_PLL_BASE			(PLL_SUBSYS_BASE + 0x200)
#define AUDIO_PLL_BASE			(PLL_SUBSYS_BASE + 0x230)
#define USB_PLL_BASE			(PLL_SUBSYS_BASE + 0x260)
#define DDR_PLL_BASE			(PLL_SUBSYS_BASE + 0x290)

#define OSC_SRC				(PLL_SUBSYS_BASE + 0x2C0)
#define ARM_CLKSRC			(PLL_SUBSYS_BASE + 0x2C4)
#define VIDEO_PLL_CLKSRC		(PLL_SUBSYS_BASE + 0x2C8)
#define MLB_ATL_CLKSRC			(PLL_SUBSYS_BASE + 0x2CC)
#define McASP235_AUX_CLKSRC		(PLL_SUBSYS_BASE + 0x2D0)
#define McASP_AHCLK_CLKSRC		(PLL_SUBSYS_BASE + 0x2D4)
#define McBSP_UART_CLKSRC		(PLL_SUBSYS_BASE + 0x2D8)
#define HDMI_I2S_CLKSRC			(PLL_SUBSYS_BASE + 0x2DC)
#define DMTIMER_CLKSRC			(PLL_SUBSYS_BASE + 0x2E0)
#define CLKOUT_MUX			(PLL_SUBSYS_BASE + 0x2E4)
#define RMII_REFCLK_SRC			(PLL_SUBSYS_BASE + 0x2E8)
#define SECSS_CLKSRC			(PLL_SUBSYS_BASE + 0x2EC)
#define SYSCLK18_SRC			(PLL_SUBSYS_BASE + 0x2F0)
#define WDT0_CLKSRC			(PLL_SUBSYS_BASE + 0x2F4)

#endif

/* PRCM */
#define CM_DPLL_OFFSET			(PRCM_BASE + 0x0300)

#ifdef CONFIG_TI816X
#define CM_TIMER1_CLKSEL		(CM_DPLL_OFFSET + 0x90)

/* Timers */
#define CM_ALWON_TIMER_0_CLKCTRL	(PRCM_BASE + 0x156C)
#define CM_ALWON_TIMER_1_CLKCTRL	(PRCM_BASE + 0x1570)
#define CM_ALWON_TIMER_2_CLKCTRL	(PRCM_BASE + 0x1574)
#define CM_ALWON_TIMER_3_CLKCTRL	(PRCM_BASE + 0x1578)
#define CM_ALWON_TIMER_4_CLKCTRL	(PRCM_BASE + 0x157C)
#define CM_ALWON_TIMER_5_CLKCTRL	(PRCM_BASE + 0x1580)
#define CM_ALWON_TIMER_6_CLKCTRL	(PRCM_BASE + 0x1584)
#define CM_ALWON_TIMER_7_CLKCTRL	(PRCM_BASE + 0x1588)
#endif

#define CM_ALWON_WDTIMER_CLKCTRL	(PRCM_BASE + 0x158C)
#define CM_ALWON_SPI_CLKCTRL		(PRCM_BASE + 0x1590)
#define CM_ALWON_CONTROL_CLKCTRL	(PRCM_BASE + 0x15C4)

#define CM_ALWON_L3_SLOW_CLKSTCTRL	(PRCM_BASE + 0x1400)

#ifdef CONFIG_TI816X
#define CM_ALWON_CUST_EFUSE_CLKCTRL	(PRCM_BASE + 0x1628)
#endif

#define CM_ALWON_GPIO_0_CLKCTRL		(PRCM_BASE + 0x155c)
#define CM_ALWON_GPIO_0_OPTFCLKEN_DBCLK (PRCM_BASE + 0x155c)

/* Ethernet */
#define CM_ETHERNET_CLKSTCTRL		(PRCM_BASE + 0x1404)
#define CM_ALWON_ETHERNET_0_CLKCTRL	(PRCM_BASE + 0x15D4)
#define CM_ALWON_ETHERNET_1_CLKCTRL	(PRCM_BASE + 0x15D8)

/* UARTs */
#define CM_ALWON_UART_0_CLKCTRL		(PRCM_BASE + 0x1550)
#define CM_ALWON_UART_1_CLKCTRL		(PRCM_BASE + 0x1554)
#define CM_ALWON_UART_2_CLKCTRL		(PRCM_BASE + 0x1558)

/* I2C */
/* Note: In ti814x I2C0 and I2C2 have common clk control */
#define CM_ALWON_I2C_0_CLKCTRL		(PRCM_BASE + 0x1564)

/* HSMMC */
#ifdef CONFIG_TI816X
#define CM_ALWON_HSMMC_CLKCTRL		(PRCM_BASE + 0x15B0)
#endif

#ifdef CONFIG_TI814X
#define CM_ALWON_HSMMC_CLKCTRL		(PRCM_BASE + 0x1620)
#endif

/* UART2 registers */
#ifdef CONFIG_TI816X
#define DEFAULT_UART_BASE		UART2_BASE
#endif

#ifdef CONFIG_TI814X
#define DEFAULT_UART_BASE		UART0_BASE
#endif

/* UART registers */
/*TODO:Move to a new file */
#define UART_SYSCFG			(DEFAULT_UART_BASE + 0x54)
#define UART_SYSSTS			(DEFAULT_UART_BASE + 0x58)
#define UART_LCR			(DEFAULT_UART_BASE + 0x0C)
#define UART_EFR			(DEFAULT_UART_BASE + 0x08)
#define UART_MCR			(DEFAULT_UART_BASE + 0x10)
#define UART_SCR			(DEFAULT_UART_BASE + 0x40)
#define UART_TCR			(DEFAULT_UART_BASE + 0x18)
#define UART_FCR			(DEFAULT_UART_BASE + 0x08)
#define UART_DLL			(DEFAULT_UART_BASE + 0x00)
#define UART_DLH			(DEFAULT_UART_BASE + 0x04)
#define UART_MDR			(DEFAULT_UART_BASE + 0x20)

/*DMM & EMIF4 MMR Declaration*/
/*TODO: Move to a new file */
#define DMM_LISA_MAP__0			(DMM_BASE + 0x40)
#define DMM_LISA_MAP__1			(DMM_BASE + 0x44)
#define DMM_LISA_MAP__2			(DMM_BASE + 0x48)
#define DMM_LISA_MAP__3			(DMM_BASE + 0x4C)
#define DMM_PAT_BASE_ADDR		(DMM_BASE + 0x460)

#define EMIF4_0_SDRAM_CONFIG		(EMIF4_0_CFG_BASE + 0x08)
#define EMIF4_0_SDRAM_CONFIG2		(EMIF4_0_CFG_BASE + 0x0C)
#define EMIF4_0_SDRAM_REF_CTRL		(EMIF4_0_CFG_BASE + 0x10)
#define EMIF4_0_SDRAM_REF_CTRL_SHADOW	(EMIF4_0_CFG_BASE + 0x14)
#define EMIF4_0_SDRAM_TIM_1		(EMIF4_0_CFG_BASE + 0x18)
#define EMIF4_0_SDRAM_TIM_1_SHADOW	(EMIF4_0_CFG_BASE + 0x1C)
#define EMIF4_0_SDRAM_TIM_2		(EMIF4_0_CFG_BASE + 0x20)
#define EMIF4_0_SDRAM_TIM_2_SHADOW	(EMIF4_0_CFG_BASE + 0x24)
#define EMIF4_0_SDRAM_TIM_3		(EMIF4_0_CFG_BASE + 0x28)
#define EMIF4_0_SDRAM_TIM_3_SHADOW	(EMIF4_0_CFG_BASE + 0x2C)
#define EMIF4_0_DDR_PHY_CTRL_1		(EMIF4_0_CFG_BASE + 0xE4)
#define EMIF4_0_DDR_PHY_CTRL_1_SHADOW	(EMIF4_0_CFG_BASE + 0xE8)
#define EMIF4_0_IODFT_TLGC		(EMIF4_0_CFG_BASE + 0x60)

#define EMIF4_1_SDRAM_CONFIG		(EMIF4_1_CFG_BASE + 0x08)
#define EMIF4_1_SDRAM_CONFIG2		(EMIF4_1_CFG_BASE + 0x0C)
#define EMIF4_1_SDRAM_REF_CTRL		(EMIF4_1_CFG_BASE + 0x10)
#define EMIF4_1_SDRAM_REF_CTRL_SHADOW	(EMIF4_1_CFG_BASE + 0x14)
#define EMIF4_1_SDRAM_TIM_1		(EMIF4_1_CFG_BASE + 0x18)
#define EMIF4_1_SDRAM_TIM_1_SHADOW	(EMIF4_1_CFG_BASE + 0x1C)
#define EMIF4_1_SDRAM_TIM_2		(EMIF4_1_CFG_BASE + 0x20)
#define EMIF4_1_SDRAM_TIM_2_SHADOW	(EMIF4_1_CFG_BASE + 0x24)
#define EMIF4_1_SDRAM_TIM_3		(EMIF4_1_CFG_BASE + 0x28)
#define EMIF4_1_SDRAM_TIM_3_SHADOW	(EMIF4_1_CFG_BASE + 0x2C)
#define EMIF4_1_DDR_PHY_CTRL_1		(EMIF4_1_CFG_BASE + 0xE4)
#define EMIF4_1_DDR_PHY_CTRL_1_SHADOW	(EMIF4_1_CFG_BASE + 0xE8)
#define EMIF4_1_IODFT_TLGC		(EMIF4_1_CFG_BASE + 0x60)

#define VTP0_CTRL_REG			0x48140E0C
#define VTP1_CTRL_REG			0x48140E10

/*EMIF4 PRCM Defintion*/
#define CM_DEFAULT_L3_FAST_CLKSTCTRL	(PRCM_BASE + 0x0508)
#define CM_DEFAULT_EMIF_0_CLKCTRL	(PRCM_BASE + 0x0520)
#define CM_DEFAULT_EMIF_1_CLKCTRL	(PRCM_BASE + 0x0524)
#define CM_DEFAULT_DMM_CLKCTRL		(PRCM_BASE + 0x0528)
#define CM_DEFAULT_FW_CLKCTRL		(PRCM_BASE + 0x052C)

/* Smartreflex Registers */
#define TI816X_SMRT_SCALE_ADDR		(CTRL_BASE + 0x06A0)
#define TI816X_SMRT_OPP_SVT_ADDR	(CTRL_BASE + 0x06A8)
#define TI816X_SMRT_OPP_HVT_ADDR	(CTRL_BASE + 0x06AC)


/* ALWON PRCM */
#define CM_ALWON_OCMC_0_CLKSTCTRL	(PRCM_BASE + 0x1414)
#define CM_ALWON_OCMC_0_CLKCTRL		(PRCM_BASE + 0x15B4)

#ifdef CONFIG_TI816X
#define CM_ALWON_OCMC_1_CLKSTCTRL	(PRCM_BASE + 0x1418)
#define CM_ALWON_OCMC_1_CLKCTRL		(PRCM_BASE + 0x15B8)
#endif

#define CM_ALWON_GPMC_CLKCTRL		(PRCM_BASE + 0x15D0)

/* OCMC */
#ifdef CONFIG_TI816X
#define SRAM0_SIZE			(0x40000)
#define SRAM_GPMC_STACK_SIZE		(0x40)
#endif

/* GPMC related */
#define GPMC_CONFIG_CS0			(0x60)
#define GPMC_CONFIG_CS0_BASE		(GPMC_BASE + GPMC_CONFIG_CS0)
#define GPMC_CONFIG1			(0x00)
#define GPMC_CONFIG2			(0x04)
#define GPMC_CONFIG3			(0x08)
#define GPMC_CONFIG4			(0x0C)
#define GPMC_CONFIG5			(0x10)
#define GPMC_CONFIG6			(0x14)
#define GPMC_CONFIG7			(0x18)

/* PAD configuration register offsets and values for gpmc address
 * lines a12 - a26
 */
#ifdef CONFIG_TI816X

#define TIM7_OUT			(CTRL_BASE + 0xb34)	/* a12 */
#define UART1_CTSN			(CTRL_BASE + 0xadc)	/* a13 */
#define UART1_RTSN			(CTRL_BASE + 0xad8)	/* a14 */
#define UART2_RTSN			(CTRL_BASE + 0xae8)	/* a15 */
#define SC1_RST				(CTRL_BASE + 0xb10)	/* a15 */
#define UART2_CTSN			(CTRL_BASE + 0xaec)	/* a16 */
#define UART0_RIN			(CTRL_BASE + 0xacc)	/* a17 */
#define UART0_DCDN			(CTRL_BASE + 0xac8)	/* a18 */
#define UART0_DSRN			(CTRL_BASE + 0xac4)	/* a19 */
#define UART0_DTRN			(CTRL_BASE + 0xac0)	/* a20 */
#define SPI_SCS3			(CTRL_BASE + 0xaa4)	/* a21 */
#define SPI_SC2				(CTRL_BASE + 0xaa0)	/* a22 */
#define GPO_IO6				(CTRL_BASE + 0xca0)	/* a23 */
#define TIM6_OUT			(CTRL_BASE + 0xb30)	/* a24 */
#define SC0_DATA			(CTRL_BASE + 0xafc)	/* a25 */
#define GPMC_A27			(CTRL_BASE + 0xba0)	/* a27 */

/* MMC Pad register offsets */
#define MMC_POW				(CTRL_BASE + 0xa70)
#define MMC_CLK				(CTRL_BASE + 0xa74)
#define MMC_CMD				(CTRL_BASE + 0xa78)
#define MMC_DAT0			(CTRL_BASE + 0xa7c)
#define MMC_DAT1_SDIRQ			(CTRL_BASE + 0xa80)
#define MMC_DAT2_SDRW			(CTRL_BASE + 0xa84)
#define MMC_DAT3			(CTRL_BASE + 0xa88)

#define GPMC_A12			TIM7_OUT
#define GPMC_A13			UART1_CTSN
#define GPMC_A14			UART1_RTSN
#define GPMC_A15			UART2_RTSN
#define GPMC_A16			UART2_CTSN
#define GPMC_A17			UART0_RIN
#define GPMC_A18			UART0_DCDN
#define GPMC_A19			UART0_DSRN
#define GPMC_A20			UART0_DTRN
#define GPMC_A21			SPI_SCS3
#define GPMC_A22			SPI_SC2
#define GPMC_A23			GPO_IO6
#define GPMC_A24			TIM6_OUT
#define GPMC_A25			SC0_DATA

#endif

#ifndef __KERNEL_STRICT_NAMES
#ifndef __ASSEMBLY__
struct gpmc_cs {
	u32 config1;		/* 0x00 */
	u32 config2;		/* 0x04 */
	u32 config3;		/* 0x08 */
	u32 config4;		/* 0x0C */
	u32 config5;		/* 0x10 */
	u32 config6;		/* 0x14 */
	u32 config7;		/* 0x18 */
	u32 nand_cmd;		/* 0x1C */
	u32 nand_adr;		/* 0x20 */
	u32 nand_dat;		/* 0x24 */
	u8 res[8];		/* blow up to 0x30 byte */
};

struct bch_res_0_3 {
	u32 bch_result_x[4];
};



struct gpmc {
	u8 res1[0x10];
	u32 sysconfig;		/* 0x10 */
	u8 res2[0x4];
	u32 irqstatus;		/* 0x18 */
	u32 irqenable;		/* 0x1C */
	u8 res3[0x20];
	u32 timeout_control;	/* 0x40 */
	u8 res4[0xC];
	u32 config;		/* 0x50 */
	u32 status;		/* 0x54 */
	u8 res5[0x8];		/* 0x58 */
	struct gpmc_cs cs[8];	/* 0x60, 0x90, .. */
	u8 res6[0x14];		/* 0x1E0 */
	u32 ecc_config;		/* 0x1F4 */
	u32 ecc_control;	/* 0x1F8 */
	u32 ecc_size_config;	/* 0x1FC */
	u32 ecc1_result;	/* 0x200 */
	u32 ecc2_result;	/* 0x204 */
	u32 ecc3_result;	/* 0x208 */
	u32 ecc4_result;	/* 0x20C */
	u32 ecc5_result;	/* 0x210 */
	u32 ecc6_result;	/* 0x214 */
	u32 ecc7_result;	/* 0x218 */
	u32 ecc8_result;	/* 0x21C */
	u32 ecc9_result;	/* 0x220 */
	u8 res7[12];		/* 0x224 */
	u32 testmomde_ctrl;	/* 0x230 */
	u8 res8[12];		/* 0x234 */
	struct bch_res_0_3 bch_result_0_3[2];	/* 0x240 */
};

/* Used for board specific gpmc initialization */
extern struct gpmc *gpmc_cfg;

#endif /* __ASSEMBLY__ */
#endif /* __KERNEL_STRICT_NAMES */

/* Ethernet MAC ID from EFuse */
#define MAC_ID0_LO	(CTRL_BASE + 0x630)
#define MAC_ID0_HI	(CTRL_BASE + 0x634)
#define MAC_ID1_LO	(CTRL_BASE + 0x638)
#define MAC_ID1_HI	(CTRL_BASE + 0x63c)

/* WDT related */
/* TODO: Move to a new file */
#define WDT_WDSC	(WDT_BASE + 0x010)
#define WDT_WDST	(WDT_BASE + 0x014)
#define WDT_WISR	(WDT_BASE + 0x018)
#define WDT_WIER	(WDT_BASE + 0x01C)
#define WDT_WWER	(WDT_BASE + 0x020)
#define WDT_WCLR	(WDT_BASE + 0x024)
#define WDT_WCRR	(WDT_BASE + 0x028)
#define WDT_WLDR	(WDT_BASE + 0x02C)
#define WDT_WTGR	(WDT_BASE + 0x030)
#define WDT_WWPS	(WDT_BASE + 0x034)
#define WDT_WDLY	(WDT_BASE + 0x044)
#define WDT_WSPR	(WDT_BASE + 0x048)
#define WDT_WIRQEOI	(WDT_BASE + 0x050)
#define WDT_WIRQSTATRAW	(WDT_BASE + 0x054)
#define WDT_WIRQSTAT	(WDT_BASE + 0x058)
#define WDT_WIRQENSET	(WDT_BASE + 0x05C)
#define WDT_WIRQENCLR	(WDT_BASE + 0x060)

#define WDT_UNFREEZE	(CTRL_BASE + 0x100)

#endif /* _TI816X_CPU_H */


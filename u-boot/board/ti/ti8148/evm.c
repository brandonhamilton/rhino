/*
 * Copyright (C) 2009, Texas Instruments, Incorporated
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/cache.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/arch/nand.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <net.h>
#include <miiphy.h>
#include <netdev.h>

#define __raw_readl(a)		(*(volatile unsigned int *)(a))
#define __raw_writel(v, a)	(*(volatile unsigned int *)(a) = (v))
#define __raw_readw(a)		(*(volatile unsigned short *)(a))
#define __raw_writew(v, a)	(*(volatile unsigned short *)(a) = (v))

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SETUP_PLL
static void pll_config(u32, u32, u32, u32, u32);
#if 0
static void pcie_pll_config(void);
#endif
static void audio_pll_config(void);
static void sata_pll_config(void);
static void modena_pll_config(void);
static void l3_pll_config(void);
static void ddr_pll_config(void);
static void dsp_pll_config(void);
static void dss_pll_config(void);
static void iss_pll_config(void);
static void iva_pll_config(void);
static void usb_pll_config(void);
#endif

static void unlock_pll_control_mmr(void);
static void cpsw_pad_config(u32 instance);
/*
 * spinning delay to use before udelay works
 */
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b" : "=r" (loops) : "0"(loops));
}

/*
 * Basic board specific setup
 */
int board_init(void)
{
	u32 regVal;

	/* Do the required pin-muxing before modules are setup */
	set_muxconf_regs();
	cpsw_pad_config(0);

	/* Get Timer and UART out of reset */

	/* UART softreset */
	regVal = __raw_readl(UART_SYSCFG);
	regVal |= 0x2;
	__raw_writel(regVal, UART_SYSCFG);
	while( (__raw_readl(UART_SYSSTS) & 0x1) != 0x1);

	/* Disable smart idle */
	regVal = __raw_readl(UART_SYSCFG);
	regVal |= (1<<3);
	__raw_writel(regVal, UART_SYSCFG);

	/* mach type passed to kernel */
	gd->bd->bi_arch_number = MACH_TYPE_TI8148EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_DRAM_1 + 0x100;
	gpmc_init();
	return 0;
}

/*
 * sets uboots idea of sdram size
 */
int dram_init(void)
{
	/* Fill up board info */
	gd->bd->bi_dram[0].start = PHYS_DRAM_1;
	gd->bd->bi_dram[0].size = PHYS_DRAM_1_SIZE;

	gd->bd->bi_dram[1].start = PHYS_DRAM_2;
	gd->bd->bi_dram[1].size = PHYS_DRAM_2_SIZE;

	return 0;
}


int misc_init_r (void)
{
	#ifdef CONFIG_TI814X_MIN_CONFIG
	printf("The 2nd stage U-Boot will now be auto-loaded\n");
	printf("Please do not interrupt the countdown till TI8148_EVM prompt if 2nd stage is already flashed\n");
	#endif

	#ifdef CONFIG_TI814X_ASCIIART
	int i = 0, j = 0;
	char ti814x[28][54] = {
"                          .:;rrr;;.                   ",
"                    ,5#@@@@#####@@@@@@#2,             ",
"                 ,A@@@hi;;;r5;;;;r;rrSG@@@A,          ",
"               r@@#i;:;s222hG;rrsrrrrrr;ri#@@r        ",
"             :@@hr:r;SG3ssrr2r;rrsrsrsrsrr;rh@@:      ",
"            B@H;;rr;3Hs;rrr;sr;;rrsrsrsrsrsr;;H@B     ",
"           @@s:rrs;5#;;rrrr;r#@H:;;rrsrsrsrsrr:s@@    ",
"          @@;;srs&X#9;r;r;;,2@@@rrr:;;rrsrsrsrr;;@@   ",
"         @@;;rrsrrs@MB#@@@@@###@@@@@@#rsrsrsrsrr;;@@  ",
"        G@r;rrsrsr;#X;SX25Ss#@@#M@#9H9rrsrsrsrsrs;r@G ",
"        @9:srsrsrs;2@;:;;:.X@@@@@H::;rrsrsrsrsrsrr:3@ ",
"       X@;rrsrsrsrr;XAi;;:&@@#@Bs:rrsrsrsrsrsrsrsrr;@X",
"       @#;rsrsrsrsrr;r2ir@@@###::rrsrsrsrsrsrsrsrsr:@@",
"       @A:rrsrsrsrr;:2@29@@M@@@;:;rrrrsrsrsrsrsrsrs;H@",
"       @&;rsrsrsrr;A@@@@@@###@@@s::;:;;rrsrsrsrsrsr;G@",
"       @#:rrsrsrsr;G@5Hr25@@@#@@@#9XG9s:rrrrsrsrsrs:#@",
"       M@;rsrsrsrs;r@&#;::S@@@@@@@M@@@@Grr:;rsrsrsr;@#",
"       :@s;rsrsrsrr:M#Msrr;;&#@@@@@@@@@@H@@5;rsrsr;s@,",
"        @@:rrsrsrsr;S@rrrsr;:;r3MH@@#@M5,S@@irrsrr:@@ ",
"         @A:rrsrsrsrrrrrsrsrrr;::;@##@r:;rH@h;srr:H@  ",
"         ;@9:rrsrsrsrrrsrsrsrsr;,S@Hi@i:;s;MX;rr:h@;  ",
"          r@B:rrrrsrsrsrsrsrr;;sA@#i,i@h;r;S5;r:H@r   ",
"           ,@@r;rrrsrsrsrsrr;2BM3r:;r:G@:rrr;;r@@,    ",
"             B@Mr;rrrrsrsrsr@@S;;;rrr:5M;rr;rM@H      ",
"              .@@@i;;rrrrsrs2i;rrrrr;r@M:;i@@@.       ",
"                .A@@#5r;;;r;;;rrr;r:r#AsM@@H.         ",
"                   ;&@@@@MhXS5i5SX9B@@@@G;            ",
"                       :ihM#@@@@@##hs,                "};

	for (i = 0; i<28; i++)
	{
		for(j = 0; j<54; j++)
			printf("%c",ti814x[i][j]);
			printf("\n");
	}
	printf("\n");
	#endif
	return 0;
}

#ifdef CONFIG_TI814X_CONFIG_DDR
static void config_ti814x_ddr(void)
{
	__raw_writel(0x2, CM_DEFAULT_FW_CLKCTRL);			/*Enable the Power Domain Transition of L3 Fast Domain Peripheral*/
	__raw_writel(0x2, CM_DEFAULT_L3_FAST_CLKSTCTRL);			/*Enable the Power Domain Transition of L3 Fast Domain Peripheral*/
	__raw_writel(0x2, CM_DEFAULT_EMIF_0_CLKCTRL);				/*Enable EMIF0 Clock*/
	__raw_writel(0x2, CM_DEFAULT_EMIF_1_CLKCTRL);				/*Enable EMIF1 Clock*/
	__raw_writel(0x2, CM_DEFAULT_DMM_CLKCTRL);

	while((__raw_readl(CM_DEFAULT_L3_FAST_CLKSTCTRL) & 0x300) != 0x300);	/*Poll for L3_FAST_GCLK  & DDR_GCLK  are active*/
	while((__raw_readl(CM_DEFAULT_EMIF_0_CLKCTRL)) != 0x2);	/*Poll for Module is functional*/
	while((__raw_readl(CM_DEFAULT_EMIF_1_CLKCTRL)) != 0x2);	/*Poll for Module is functional*/
	while((__raw_readl(CM_DEFAULT_DMM_CLKCTRL)) != 0x2);		/*Poll for Module is functional*/

	__raw_writel(__raw_readl(VTP0_CTRL_REG) | 0x00000040 , VTP0_CTRL_REG);
	__raw_writel(__raw_readl(VTP1_CTRL_REG) | 0x00000040 , VTP1_CTRL_REG);

	// Write 0 to CLRZ bit
	__raw_writel(__raw_readl(VTP0_CTRL_REG) & 0xfffffffe , VTP0_CTRL_REG);
	__raw_writel(__raw_readl(VTP1_CTRL_REG) & 0xfffffffe , VTP1_CTRL_REG);

	// Write 1 to CLRZ bit
	__raw_writel(__raw_readl(VTP0_CTRL_REG) | 0x00000001 , VTP0_CTRL_REG);
	__raw_writel(__raw_readl(VTP1_CTRL_REG) | 0x00000001 , VTP1_CTRL_REG);

	// Read VTP control registers & check READY bits
	while( (__raw_readl(VTP0_CTRL_REG) & 0x00000020) != 0x20);
	while( (__raw_readl(VTP1_CTRL_REG) & 0x00000020) != 0x20);

	/*Program the DMM to Access EMIF0 and EMIF1*/
	__raw_writel(0x80600100, DMM_LISA_MAP__0);
	__raw_writel(0x80600100, DMM_LISA_MAP__1);
	__raw_writel(0xC0600200, DMM_LISA_MAP__2);
	__raw_writel(0xC0600200, DMM_LISA_MAP__3);

	while(__raw_readl(DMM_LISA_MAP__0)!=0x80600100);
	while(__raw_readl(DMM_LISA_MAP__1)!=0x80600100);
	while(__raw_readl(DMM_LISA_MAP__2)!=0xC0600200);
	while(__raw_readl(DMM_LISA_MAP__3)!=0xC0600200);

	__raw_writel(0x80000000, DMM_PAT_BASE_ADDR);

	/*Program EMIF0 CFG Registers*/
	__raw_writel(0x7, EMIF4_0_DDR_PHY_CTRL_1);//RL =5
	__raw_writel(0x7, EMIF4_0_DDR_PHY_CTRL_1_SHADOW);//RL =5
	__raw_writel(0x0AAAF552, EMIF4_0_SDRAM_TIM_1);
	__raw_writel(0x0AAAF552, EMIF4_0_SDRAM_TIM_1_SHADOW);
	__raw_writel(0x043631D2, EMIF4_0_SDRAM_TIM_2);
	__raw_writel(0x043631D2, EMIF4_0_SDRAM_TIM_2_SHADOW);
	__raw_writel(0x00000327, EMIF4_0_SDRAM_TIM_3);
	__raw_writel(0x00000327, EMIF4_0_SDRAM_TIM_3_SHADOW);
	__raw_writel(0x10000C30, EMIF4_0_SDRAM_REF_CTRL);
	__raw_writel(0x10000C30, EMIF4_0_SDRAM_REF_CTRL_SHADOW);
	__raw_writel(0x40801AB2, EMIF4_0_SDRAM_CONFIG);// CL = 6

	/*Program EMIF1 CFG Registers*/
	__raw_writel(0x7, EMIF4_1_DDR_PHY_CTRL_1);
	__raw_writel(0x7, EMIF4_1_DDR_PHY_CTRL_1_SHADOW);
	__raw_writel(0x0AAAF552, EMIF4_1_SDRAM_TIM_1);
	__raw_writel(0x0AAAF552, EMIF4_1_SDRAM_TIM_1_SHADOW);
	__raw_writel(0x043631D2, EMIF4_1_SDRAM_TIM_2);
	__raw_writel(0x043631D2, EMIF4_1_SDRAM_TIM_2_SHADOW);
	__raw_writel(0x00000327, EMIF4_1_SDRAM_TIM_3);
	__raw_writel(0x00000327, EMIF4_1_SDRAM_TIM_3_SHADOW);
	__raw_writel(0x10000C30, EMIF4_1_SDRAM_REF_CTRL);
	__raw_writel(0x10000C30, EMIF4_1_SDRAM_REF_CTRL_SHADOW);
	__raw_writel(0x40801AB2, EMIF4_1_SDRAM_CONFIG);// CL = 6


}
#endif

#ifdef CONFIG_SETUP_PLL
static void audio_pll_config()
{
	pll_config(AUDIO_PLL_BASE,
			AUDIO_N, AUDIO_M,
			AUDIO_M2, AUDIO_CLKCTRL);
}

#if 0
static void pcie_pll_config()
{
	/* Powerdown both reclkp/n single ended receiver */
	__raw_writel(0x00000002, SERDES_REFCLK_CTRL);

	__raw_writel(0x00000000, PCIE_PLLCFG0);

	/* PCIe(2.5GHz) mode, 100MHz refclk, MDIVINT = 25, disable (50,100,125M) clks */
	__raw_writel(0x00640000, PCIE_PLLCFG1);

	/* SSC Mantissa and exponent = 0 */
	__raw_writel(0x00000000, PCIE_PLLCFG2);

	/* TBD */
	__raw_writel(0x004008E0, PCIE_PLLCFG3);

	/* TBD */
	__raw_writel(0x0000609C, PCIE_PLLCFG4);

	/* pcie_serdes_cfg_misc */
	/* TODO: verify the address over here (CTRL_BASE + 0x6FC = 0x481406FC ???)*/
	//__raw_writel(0x00000E7B, 0x48141318);
	delay(3);

	/* Enable PLL LDO */
	__raw_writel(0x00000004, PCIE_PLLCFG0);
	delay(3);

	/* Enable DIG LDO, PLL LD0 */
	__raw_writel(0x00000014, PCIE_PLLCFG0);
	delay(3);

	/* Enable DIG LDO, ENBGSC_REF, PLL LDO */
	__raw_writel(0x00000016, PCIE_PLLCFG0);
	delay(3);
	__raw_writel(0x30000016, PCIE_PLLCFG0);
	delay(3);
	__raw_writel(0x70000016, PCIE_PLLCFG0);
	delay(3);

	/* Enable DIG LDO, SELSC, ENBGSC_REF, PLL LDO */
	__raw_writel(0x70000017, PCIE_PLLCFG0);
	delay(3);

	/* wait for ADPLL lock */
	while(__raw_readl(PCIE_PLLSTATUS) != 0x1);

}
#endif

static void sata_pll_config()
{
	__raw_writel(0xC12C003C, SATA_PLLCFG1);
	__raw_writel(0x004008E0, SATA_PLLCFG3);
	delay(0xFFFF);

	__raw_writel(0x80000004, SATA_PLLCFG0);
	delay(0xFFFF);

	/* Enable PLL LDO */
	__raw_writel(0x80000014, SATA_PLLCFG0);
	delay(0xFFFF);

	/* Enable DIG LDO, ENBGSC_REF, PLL LDO */
	__raw_writel(0x80000016, SATA_PLLCFG0);
	delay(0xFFFF);

	__raw_writel(0xC0000017, SATA_PLLCFG0);
	delay(0xFFFF);

	/* wait for ADPLL lock */
	while(((__raw_readl(SATA_PLLSTATUS) & 0x01) == 0x0));

}

static void usb_pll_config()
{
	pll_config(USB_PLL_BASE,
			USB_N, USB_M,
			USB_M2, USB_CLKCTRL);
}

static void modena_pll_config()
{
	pll_config(MODENA_PLL_BASE,
			MODENA_N, MODENA_M,
			0, MODENA_CLKCTRL);
}

static void l3_pll_config()
{
	pll_config(L3_PLL_BASE,
			L3_N, L3_M,
			L3_M2, L3_CLKCTRL);
}

static void ddr_pll_config()
{
	pll_config(DDR_PLL_BASE,
			DDR_N, DDR_M,
			DDR_M2, DDR_CLKCTRL);
}

static void dsp_pll_config()
{
	pll_config(DSP_PLL_BASE,
			DSP_N, DSP_M,
			DSP_M2, DSP_CLKCTRL);
}

static void dss_pll_config()
{
	pll_config(DSS_PLL_BASE,
			DSS_N, DSS_M,
			DSS_M2, DSS_CLKCTRL);
}

static void iss_pll_config()
{
	pll_config(ISS_PLL_BASE,
			ISS_N, ISS_M,
			ISS_M2, ISS_CLKCTRL);
}

static void iva_pll_config()
{
	pll_config(IVA_PLL_BASE,
			IVA_N, IVA_M,
			IVA_M2, IVA_CLKCTRL);
}

/*
 * configure individual ADPLLJ
 */
static void pll_config(u32 base, u32 n, u32 m, u32 m2, u32 clkctrl_val)
{
	u32 m2nval, mn2val, read_clkctrl = 0;
	if (m2 == 0)
		m2nval = n;
	else
		m2nval = (m2 << 16) | n;

	mn2val = m;

	/*
	 * ref_clk = 20/(n + 1);
	 * clkout_dco = ref_clk * m;
	 * clk_out = clkout_dco/m2;
	*/

	__raw_writel(m2nval, (base + ADPLLJ_M2NDIV));
	__raw_writel(mn2val, (base + ADPLLJ_MN2DIV));

	/* Load M2, N2 dividers of ADPLL */
	__raw_writel(0x1, (base + ADPLLJ_TENABLEDIV));
	__raw_writel(0x0, (base + ADPLLJ_TENABLEDIV));

	/* Loda M, N dividers of ADPLL */
	__raw_writel(0x1, (base + ADPLLJ_TENABLE));
	__raw_writel(0x0, (base + ADPLLJ_TENABLE));

	read_clkctrl = __raw_readl(base + ADPLLJ_CLKCTRL);

	if (m2 == 0)
		__raw_writel((read_clkctrl & 0xff7fffff) | clkctrl_val,
			base + ADPLLJ_CLKCTRL);
	else
		__raw_writel((read_clkctrl & 0xff7fe3ff) | clkctrl_val,
			base + ADPLLJ_CLKCTRL);


	/* Wait for phase and freq lock */
	while((__raw_readl(base + ADPLLJ_STATUS) & 0x00000600) != 0x00000600);

}
#endif

/*
 * Enable the clks & power for perifs (TIMER1, UART0,...)
 */
void per_clocks_enable(void)
{
	u32 temp;

	__raw_writel(0x2, CM_ALWON_L3_SLOW_CLKSTCTRL);

	/* TODO: No module level enable as in ti8148 ??? */
#if 0
	/* TIMER 1 */
	__raw_writel(0x2, CM_ALWON_TIMER_1_CLKCTRL);
#endif
	/* Selects OSC0 (20MHz) for DMTIMER1 */
	temp = __raw_readl(DMTIMER_CLKSRC);
	temp &= ~(0x7 << 3);
	temp |= (0x4 << 3);
	__raw_writel(temp, DMTIMER_CLKSRC);

#if 0
	while(((__raw_readl(CM_ALWON_L3_SLOW_CLKSTCTRL) & (0x80000<<1)) >> (19+1)) != 1);
	while(((__raw_readl(CM_ALWON_TIMER_1_CLKCTRL) & 0x30000)>>16) !=0);
#endif
	__raw_writel(0x2,(DM_TIMER1_BASE + 0x54));
	while(__raw_readl(DM_TIMER1_BASE + 0x10) & 1);

	__raw_writel(0x1,(DM_TIMER1_BASE + 0x38));

	/* UARTs */
	__raw_writel(0x2, CM_ALWON_UART_0_CLKCTRL);
	while(__raw_readl(CM_ALWON_UART_0_CLKCTRL) != 0x2);

	__raw_writel(0x2, CM_ALWON_UART_1_CLKCTRL);
	while(__raw_readl(CM_ALWON_UART_1_CLKCTRL) != 0x2);

	__raw_writel(0x2, CM_ALWON_UART_2_CLKCTRL);
	while(__raw_readl(CM_ALWON_UART_2_CLKCTRL) != 0x2);

	while((__raw_readl(CM_ALWON_L3_SLOW_CLKSTCTRL) & 0x2100) != 0x2100);

	/* SPI */
	__raw_writel(0x2, CM_ALWON_SPI_CLKCTRL);
	while(__raw_readl(CM_ALWON_SPI_CLKCTRL) != 0x2);

	/* I2C0 and I2C2 */
	__raw_writel(0x2, CM_ALWON_I2C_0_CLKCTRL);
	while(__raw_readl(CM_ALWON_I2C_0_CLKCTRL) != 0x2);

	/* Ethernet */
	__raw_writel(0x2, CM_ETHERNET_CLKSTCTRL);
	__raw_writel(0x2, CM_ALWON_ETHERNET_0_CLKCTRL);
	while((__raw_readl(CM_ALWON_ETHERNET_0_CLKCTRL) & 0x30000) != 0);

	/* HSMMC */
	__raw_writel(0x2, CM_ALWON_HSMMC_CLKCTRL);
	while(__raw_readl(CM_ALWON_HSMMC_CLKCTRL) != 0x2);


}

/*
 * inits clocks for PRCM as defined in clocks.h
 */
void prcm_init(u32 in_ddr)
{
	/* Enable the control module */
	__raw_writel(0x2, CM_ALWON_CONTROL_CLKCTRL);

#ifdef CONFIG_SETUP_PLL
	/* Setup the various plls */
	audio_pll_config();
	sata_pll_config();
//	pcie_pll_config();
	modena_pll_config();
	l3_pll_config();
	ddr_pll_config();
	dsp_pll_config();
	dss_pll_config();
	iva_pll_config();
	iss_pll_config();

	usb_pll_config();

	/*  With clk freqs setup to desired values, enable the required peripherals */
	per_clocks_enable();
#endif
}

static void cpsw_pad_config(u32 instance)
{
#define PADCTRL_BASE 0x48140000
#define PAD232_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0B9C))
#define PAD233_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BA0))
#define PAD234_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BA4))
#define PAD235_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BA8))
#define PAD236_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BAC))
#define PAD237_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BB0))
#define PAD238_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BB4))
#define PAD239_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BB8))
#define PAD240_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BBC))
#define PAD241_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BC0))
#define PAD242_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BC4))
#define PAD243_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BC8))
#define PAD244_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BCC))
#define PAD245_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BD0))
#define PAD246_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BD4))
#define PAD247_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BD8))
#define PAD248_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BDC))
#define PAD249_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BE0))
#define PAD250_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BE4))
#define PAD251_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BE8))
#define PAD252_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BEC))
#define PAD253_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BF0))
#define PAD254_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BF4))
#define PAD255_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BF8))
#define PAD256_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0BFC))
#define PAD257_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0C00))
#define PAD258_CNTRL  (*(volatile unsigned int *)(PADCTRL_BASE + 0x0C04))

	if (instance == 0) {
		volatile u32 val = 0;
		val = PAD232_CNTRL;
		PAD232_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD233_CNTRL;
		PAD233_CNTRL = (volatile unsigned int) (BIT(19) | BIT(17) | BIT(0));
		val = PAD234_CNTRL;
		PAD234_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(17) | BIT(0));
		val = PAD235_CNTRL;
		PAD235_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(0));
		val = PAD236_CNTRL;
		PAD236_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(0));
		val = PAD237_CNTRL;
		PAD237_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(0));
		val = PAD238_CNTRL;
		PAD238_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(0));
		val = PAD239_CNTRL;
		PAD239_CNTRL = (volatile unsigned int) (BIT(19) | BIT(18) | BIT(0));
		val = PAD240_CNTRL;
		PAD240_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD241_CNTRL;
		PAD241_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD242_CNTRL;
		PAD242_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD243_CNTRL;
		PAD243_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD244_CNTRL;
		PAD244_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD245_CNTRL;
		PAD245_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD246_CNTRL;
		PAD246_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD247_CNTRL;
		PAD247_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD248_CNTRL;
		PAD248_CNTRL = (volatile unsigned int) (BIT(18) | BIT(0));
		val = PAD249_CNTRL;
		PAD249_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD250_CNTRL;
		PAD250_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD251_CNTRL;
		PAD251_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD252_CNTRL;
		PAD252_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD253_CNTRL;
		PAD253_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD254_CNTRL;
		PAD254_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD255_CNTRL;
		PAD255_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD256_CNTRL;
		PAD256_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD257_CNTRL;
		PAD257_CNTRL = (volatile unsigned int) (BIT(0));
		val = PAD258_CNTRL;
		PAD258_CNTRL = (volatile unsigned int) (BIT(0));
	}
}


/*
 * baord specific muxing of pins
 */
void set_muxconf_regs(void)
{
	u32 i, add, val;
	u32 pad_conf[] = {
#include "mux.h"
	};

	for (i = 0; i<N_PINS; i++)
	{
		add = PIN_CTRL_BASE + (i*4);
		val = __raw_readl(add);
		val |= pad_conf[i];
		__raw_writel(val, add);
	}
	/* MMC/SD pull-down enable */
	__raw_writel(0x000C0040, 0x48140928);
}

void unlock_pll_control_mmr()
{
	/* ??? */
	__raw_writel(0x1EDA4C3D, 0x481C5040);
	__raw_writel(0x2FF1AC2B, 0x48140060);
	__raw_writel(0xF757FDC0, 0x48140064);
	__raw_writel(0xE2BC3A6D, 0x48140068);
	__raw_writel(0x1EBF131D, 0x4814006c);
	__raw_writel(0x6F361E05, 0x48140070);

}

/*
 * early system init of muxing and clocks.
 */
void s_init(u32 in_ddr)
{
	/* TODO: Revisit enabling of I/D-cache in 1st stage */
#if 0
	icache_enable();
	dcache_enable();
#endif
	l2_cache_enable();		/* Can be removed as A8 comes up with L2 enabled */
	unlock_pll_control_mmr();
	prcm_init(in_ddr);		/* Setup the PLLs and the clocks for the peripherals */
#ifdef CONFIG_TI814X_CONFIG_DDR
	if (!in_ddr)
		config_ti814x_ddr();	/* Do DDR settings */
#endif
}

/*
 * Reset the board
 */
void reset_cpu (ulong addr)
{
	addr = __raw_readl(PRM_DEVICE_RSTCTRL);
	addr &= ~BIT(1);
	addr |= BIT(1);
	__raw_writel(addr, PRM_DEVICE_RSTCTRL);
}

#ifdef CONFIG_DRIVER_TI_CPSW

#define PHY_CONF_REG           22
#define PHY_CONF_TXCLKEN       (1 << 5)

/* TODO : Check for the board specific PHY */
static void phy_init(char *name, int addr)
{
	unsigned short val;
	unsigned int   cntr = 0;

	/* Enable PHY to clock out TX_CLK */
	miiphy_read(name, addr, PHY_CONF_REG, &val);
	val |= PHY_CONF_TXCLKEN;
	miiphy_write(name, addr, PHY_CONF_REG, val);
	miiphy_read(name, addr, PHY_CONF_REG, &val);

	/* Enable Autonegotiation */
	if (miiphy_read(name, addr, PHY_BMCR, &val) != 0) {
		printf("failed to read bmcr\n");
		return;
	}
	val |= PHY_BMCR_DPLX | PHY_BMCR_AUTON | PHY_BMCR_100_MBPS;
	if (miiphy_write(name, addr, PHY_BMCR, val) != 0) {
		printf("failed to write bmcr\n");
		return;
	}
	miiphy_read(name, addr, PHY_BMCR, &val);

	/* Setup GIG advertisement */
	miiphy_read(name, addr, PHY_1000BTCR, &val);
	val |= PHY_1000BTCR_1000FD;
	val &= ~PHY_1000BTCR_1000HD;
	miiphy_write(name, addr, PHY_1000BTCR, val);
	miiphy_read(name, addr, PHY_1000BTCR, &val);

	/* Setup general advertisement */
	if (miiphy_read(name, addr, PHY_ANAR, &val) != 0) {
		printf("failed to read anar\n");
		return;
	}
	val |= (PHY_ANLPAR_10 | PHY_ANLPAR_10FD | PHY_ANLPAR_TX | PHY_ANLPAR_TXFD);
	if (miiphy_write(name, addr, PHY_ANAR, val) != 0) {
		printf("failed to write anar\n");
		return;
	}
	miiphy_read(name, addr, PHY_ANAR, &val);

	/* Restart auto negotiation*/
	miiphy_read(name, addr, PHY_BMCR, &val);
	val |= PHY_BMCR_RST_NEG;
	miiphy_write(name, addr, PHY_BMCR, val);

       /*check AutoNegotiate complete - it can take upto 3 secs*/
       do{
               udelay(40000);
               cntr++;

               if (!miiphy_read(name, addr, PHY_BMSR, &val)){
                       if(val & PHY_BMSR_AUTN_COMP)
                               break;
               }

       }while(cntr < 250);

       if (!miiphy_read(name, addr, PHY_BMSR, &val)) {
               if (!(val & PHY_BMSR_AUTN_COMP))
                       printf("Auto negotitation failed\n");
       }

       return;
}

static void cpsw_control(int enabled)
{
       /* nothing for now */
       /* TODO : VTP was here before */
       return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
       {
               .slave_reg_ofs  = 0x50,
               .sliver_reg_ofs = 0x700,
               .phy_id         = 1,
       },
       {
               .slave_reg_ofs  = 0x90,
               .sliver_reg_ofs = 0x740,
               .phy_id         = 0,
       },
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base              = TI814X_CPSW_MDIO_BASE,
	.cpsw_base              = TI814X_CPSW_BASE,
	.mdio_div               = 0xff,
	.channels               = 8,
	.cpdma_reg_ofs          = 0x100,
	.slaves                 = 1,
	.slave_data             = cpsw_slaves,
	.ale_reg_ofs            = 0x600,
	.ale_entries            = 1024,
	.host_port_reg_ofs      = 0x28,
	.hw_stats_reg_ofs       = 0x400,
	.mac_control            = (1 << 5) /* MIIEN      */,
	.control                = cpsw_control,
	.phy_init               = phy_init,
	.host_port_num		= 0,
};

extern void cpsw_eth_set_mac_addr (const u_int8_t *addr);

int board_eth_init(bd_t *bis)
{
	u_int8_t mac_addr[6];
	u_int32_t mac_hi,mac_lo;

	if (!eth_getenv_enetaddr("ethaddr", mac_addr)) {
		printf("<ethaddr> not set. Reading from E-fuse\n");
		/* try reading mac address from efuse */
		mac_lo = __raw_readl(MAC_ID0_LO);
		mac_hi = __raw_readl(MAC_ID0_HI);
		mac_addr[0] = mac_hi & 0xFF;
		mac_addr[1] = (mac_hi & 0xFF00) >> 8;
		mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
		mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
		mac_addr[4] = mac_lo & 0xFF;
		mac_addr[5] = (mac_lo & 0xFF00) >> 8;
		/* set the ethaddr variable with MACID detected */
		setenv("ethaddr",(char*)mac_addr);
	}

	if(is_valid_ether_addr(mac_addr)) {
		printf("Detected MACID:%x:%x:%x:%x:%x:%x\n",mac_addr[0],
			mac_addr[1], mac_addr[2], mac_addr[3],
			mac_addr[4], mac_addr[5]);
		cpsw_eth_set_mac_addr(mac_addr);
	} else {
		printf("Caution:using static MACID!! Set <ethaddr> variable\n");
	}

	return cpsw_register(&cpsw_data);
}
#endif

#ifdef CONFIG_NAND_TI81XX
/******************************************************************************
 * Command to switch between NAND HW and SW ecc
 *****************************************************************************/
extern void ti81xx_nand_switch_ecc(nand_ecc_modes_t hardware, int32_t mode);
static int do_switch_ecc(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int type = 0;
	if (argc < 2)
		goto usage;

	if (strncmp(argv[1], "hw", 2) == 0) {
		if (argc == 3)
			type = simple_strtoul(argv[2], NULL, 10);
		ti81xx_nand_switch_ecc(NAND_ECC_HW, type);
	}
	else if (strncmp(argv[1], "sw", 2) == 0)
		ti81xx_nand_switch_ecc(NAND_ECC_SOFT, 0);
	else
		goto usage;

	return 0;

usage:
	printf ("Usage: nandecc %s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	nandecc, 3, 1,	do_switch_ecc,
	"Switch NAND ECC calculation algorithm b/w hardware and software",
	"[sw|hw <hw_type>] \n"
	"   [sw|hw]- Switch b/w hardware(hw) & software(sw) ecc algorithm\n"
	"   hw_type- 0 for Hamming code\n"
	"            1 for bch4\n"
	"            2 for bch8\n"
	"            3 for bch16\n"
);

#endif /* CONFIG_NAND_TI81XX */


/*
 * rhino.h - Header file for UCT's Rhino board (with an AM3517 processor)
 *
 * Author: Simon Scott, University of Cape Town
 *
 * Based on ti/evm/evm.h
 *
 * Copyright (C) 2009 University of Cape Town
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

#ifndef _RHINO_H_
#define _RHINO_H_

const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"RHINO Board",
	"NAND",
};

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */
#define MUX_RHINO() \
	\
	/* SDRAM Controller */\
	\
	MUX_VAL(CP(SDRC_D0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D2),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D4),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D5),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D6),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D7),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D8),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D9),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D10),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D11),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D12),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D13),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D14),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D15),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D16),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D17),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D18),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D19),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D20),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D21),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D22),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D23),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D24),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D25),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D26),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D27),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D28),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D29),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D30),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_D31),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_CLK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS2),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SDRC_DQS0N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS1N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS2N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_DQS3N),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(SDRC_CKE0),		(M0)) \
	MUX_VAL(CP(STRBEN_DLY0),	(IEN  | PTD | EN  | M0)) /*sdrc_strben_dly0*/\
	MUX_VAL(CP(STRBEN_DLY1),	(IEN  | PTD | EN  | M0)) /*sdrc_strben_dly1*/\
	\
	/* GPMC */\
	\
	MUX_VAL(CP(GPMC_A1),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A2),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A3),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A4),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A5),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A6),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A7),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A8),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A9),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_A10),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D0),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D1),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D2),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D3),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D4),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D5),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D6),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D7),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D8),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D9),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D10),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D11),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D12),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D13),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D14),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_D15),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS0),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS1),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS2),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS3),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS4),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS5),		(IDIS | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS6),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_NCS7),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_CLK),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(GPMC_NADV_ALE),	(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NOE),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NWE),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_NBE0_CLE),	(IDIS | PTU | DIS | M0)) \
	MUX_VAL(CP(GPMC_NBE1),		(IDIS | PTU | DIS | M4)) /*GPIO_61*/\
	MUX_VAL(CP(GPMC_NWP),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(GPMC_WAIT0),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_WAIT1),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_WAIT2),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(GPMC_WAIT3),		(IDIS | PTU | DIS | M4)) /*GPIO_65*/\
	\
	/* DSS */\
	\
	MUX_VAL(CP(DSS_PCLK),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_HSYNC),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_VSYNC),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_ACBIAS),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA0),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA1),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA2),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA3),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA4),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA5),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA6),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA7),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA8),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA9),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA10),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA11),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA12),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA13),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA14),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA15),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA16),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA17),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA18),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA19),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA20),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA21),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA22),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(DSS_DATA23),		(IDIS | PTD | DIS | M0)) \
	\
	/* MMC1 : Used as GPIOs and SPI for programming FPGA */\
	\
	MUX_VAL(CP(MMC1_CLK),		(IEN  | PTU | DIS | M4)) /*GPIO_120*/\
	MUX_VAL(CP(MMC1_CMD),		(IEN  | PTU | DIS | M4)) /*GPIO_121*/\
	MUX_VAL(CP(MMC1_DAT0),		(IDIS | PTU | DIS | M1)) /*MCSPI2_CLK*/\
	MUX_VAL(CP(MMC1_DAT1),		(IDIS | PTU | DIS | M1)) /*MCSPI2_SIMO*/\
	MUX_VAL(CP(MMC1_DAT2),		(IEN  | PTU | DIS | M1)) /*MCSPI2_SOMI*/\
	MUX_VAL(CP(MMC1_DAT3),		(IDIS | PTU | DIS | M1)) /*MCSPI2_CS0*/\
	MUX_VAL(CP(MMC1_DAT4),		(IDIS | PTU | DIS | M2)) /*GPIO_126*/\
	MUX_VAL(CP(MMC1_DAT5),		(IEN  | PTU | DIS | M2)) /*GPIO_127*/\
	MUX_VAL(CP(MMC1_DAT6),		(IEN  | PTU | DIS | M2)) /*GPIO_128*/\
	MUX_VAL(CP(MMC1_DAT7),		(IDIS | PTU | DIS | M2)) /*GPIO_129*/\
	\
	/* MMC2 */ \
	\
	MUX_VAL(CP(MMC2_CLK),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(MMC2_CMD),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MMC2_DAT0),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MMC2_DAT1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MMC2_DAT2),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MMC2_DAT3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MMC2_DAT4),		(IEN  | PTD | DIS | M4)) /*GPIO_136*/\
	MUX_VAL(CP(MMC2_DAT5),		(IEN  | PTD | DIS | M4)) /*GPIO_137*/\
	MUX_VAL(CP(MMC2_DAT6),		(IEN  | PTD | DIS | M4)) /*GPIO_138*/\
	MUX_VAL(CP(MMC2_DAT7),		(IEN  | PTD | DIS | M4)) /*GPIO_139*/\
	\
	/* McBSP1 : Used as GPIOs*/\
	\
	MUX_VAL(CP(MCBSP_CLKS),		(IEN  | PTU | DIS | M4)) /*GPIO_160*/\
	MUX_VAL(CP(MCBSP1_CLKR),	(IDIS | PTU | DIS | M4)) /*GPIO_156*/\
	MUX_VAL(CP(MCBSP1_FSR),		(IDIS | PTU | DIS | M4)) /*GPIO_157*/\
	MUX_VAL(CP(MCBSP1_DX),		(IDIS | PTD | DIS | M4)) /*GPIO_158*/\
	MUX_VAL(CP(MCBSP1_DR),		(IEN  | PTD | DIS | M4)) /*GPIO_159*/\
	MUX_VAL(CP(MCBSP1_FSX),		(IEN  | PTD | DIS | M4)) /*GPIO_161*/\
	MUX_VAL(CP(MCBSP1_CLKX),	(IEN  | PTD | DIS | M4)) /*GPIO_162*/\
	\
    /* McBSP2 : Audio Codec*/\
    \
	MUX_VAL(CP(MCBSP2_FSX),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MCBSP2_CLKX),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MCBSP2_DR),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MCBSP2_DX),		(IDIS | PTD | DIS | M0)) \
    \
    /* McBSP3 : GPIO for FMC_0_CTRL */\
    \
	MUX_VAL(CP(MCBSP3_DX),		(IEN  | PTD | DIS | M4)) /*GPIO_140*/\
	MUX_VAL(CP(MCBSP3_DR),		(IEN  | PTD | DIS | M4)) /*GPIO_141*/\
	MUX_VAL(CP(MCBSP3_CLKX),	(IEN  | PTD | DIS | M4)) /*GPIO_142*/\
	MUX_VAL(CP(MCBSP3_FSX),		(IDIS | PTD | DIS | M4)) /*GPIO_143*/\
    \
    /* McBSP4 : GPIO for FMC_0_CTRL4 */\
    \
	MUX_VAL(CP(MCBSP4_CLKX),	(IEN  | PTD | DIS | M4)) /*GPIO_152*/\
	MUX_VAL(CP(MCBSP4_DR),		(IEN  | PTD | DIS | M4)) /*GPIO_153*/\
	MUX_VAL(CP(MCBSP4_DX),		(IEN  | PTD | DIS | M4)) /*GPIO_154*/\
	MUX_VAL(CP(MCBSP4_FSX),		(IEN  | PTD | DIS | M4)) /*GPIO_155*/\
    \
	/* UART1, UART2, UART3 */\
    \
	MUX_VAL(CP(UART1_TX),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART1_RTS),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART1_CTS),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(UART1_RX),		(IEN  | PTD | DIS | M0)) \
	\
	MUX_VAL(CP(UART2_CTS),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(UART2_RTS),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART2_TX),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART2_RX),		(IEN  | PTD | DIS | M0)) \
	\
	MUX_VAL(CP(UART3_CTS_RCTX),	(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(UART3_RTS_SD),	(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(UART3_RX_IRRX),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(UART3_TX_IRTX),	(IDIS | PTD | DIS | M0)) \
	\
	/* I2C (1 to 3) */\
	\
	MUX_VAL(CP(I2C1_SCL),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(I2C1_SDA),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(I2C2_SCL),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(I2C2_SDA),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(I2C3_SCL),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(I2C3_SDA),		(IEN  | PTU | DIS | M0)) \
	\
	/* McSPI1 */\
	\
	MUX_VAL(CP(MCSPI1_CLK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MCSPI1_SIMO),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MCSPI1_SOMI),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(MCSPI1_CS0),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(MCSPI1_CS1),		(IEN  | PTD | EN  | M0)) \
	MUX_VAL(CP(MCSPI1_CS2),		(IEN  | PTU | DIS | M0)) \
	MUX_VAL(CP(MCSPI1_CS3),		(IEN  | PTD | DIS | M3)) /*HSUSB2_DATA2*/\
	\
	/* McSPI2 : Configured for HSUSB2 */\
	\
	MUX_VAL(CP(MCSPI2_CLK),		(IEN  | PTD | DIS | M3)) /*HSUSB2_DATA7*/\
	MUX_VAL(CP(MCSPI2_SIMO),	(IEN  | PTD | DIS | M3)) /*HSUSB2_DATA4*/\
	MUX_VAL(CP(MCSPI2_SOMI),	(IEN  | PTD | DIS | M3)) /*HSUSB2_DATA5*/\
	MUX_VAL(CP(MCSPI2_CS0),		(IEN  | PTD | DIS | M3)) /*HSUSB2_DATA6*/\
	MUX_VAL(CP(MCSPI2_CS1),		(IEN  | PTD | DIS | M3)) /*HSUSB2_DATA3*/\
	\
	/* CCDC : Configured as GPIOs for Power Supply Enables and Modem Control */\
	\
	MUX_VAL(CP(CCDC_PCLK),		(IEN  | PTU | DIS | M4)) /*GPIO_94*/\
	MUX_VAL(CP(CCDC_FIELD),		(IDIS | PTD | DIS | M4)) /*GPIO_95*/\
	MUX_VAL(CP(CCDC_HD),		(IEN  | PTU | DIS | M4)) /*GPIO_96*/\
	MUX_VAL(CP(CCDC_VD),		(IEN  | PTU | DIS | M4)) /*GPIO_97*/\
	MUX_VAL(CP(CCDC_WEN),		(IEN  | PTD | DIS | M4)) /*GPIO_98*/\
	MUX_VAL(CP(CCDC_DATA0),		(IDIS | PTD | DIS | M4)) /*GPIO_99*/\
	MUX_VAL(CP(CCDC_DATA1),		(IDIS | PTD | DIS | M4)) /*GPIO_100*/\
	MUX_VAL(CP(CCDC_DATA2),		(IDIS | PTD | DIS | M4)) /*GPIO_101*/\
	MUX_VAL(CP(CCDC_DATA3),		(IDIS | PTD | DIS | M4)) /*GPIO_102*/\
	MUX_VAL(CP(CCDC_DATA4),		(IDIS | PTD | DIS | M4)) /*GPIO_103*/\
	MUX_VAL(CP(CCDC_DATA5),		(IDIS | PTD | DIS | M4)) /*GPIO_104*/\
	MUX_VAL(CP(CCDC_DATA6),		(IDIS | PTD | DIS | M4)) /*GPIO_105*/\
	MUX_VAL(CP(CCDC_DATA7),		(IDIS | PTD | DIS | M4)) /*GPIO_106*/\
	\
	/* RMII */\
	\
	MUX_VAL(CP(RMII_MDIO_DATA),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_MDIO_CLK),	(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_RXD0)	,	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_RXD1),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_CRS_DV),	(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_RXER),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_TXD0),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_TXD1),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_TXEN),		(IDIS | PTD | DIS | M0)) \
	MUX_VAL(CP(RMII_50MHZ_CLK),	(IEN  | PTD | EN  | M0)) \
	\
	/* HECC : Used as GPIOs for User LEDs*/\
	\
	MUX_VAL(CP(HECC1_TXD),		(IDIS | PTD | DIS | M4)) \
	MUX_VAL(CP(HECC1_RXD),		(IDIS | PTD | DIS | M4)) \
	\
	/* USB0 (OTG) */\
	\
	MUX_VAL(CP(USB0_DRVBUS),	(IDIS | PTD | DIS | M0)) \
	\
	/* HDQ (configured as SYS_ALTCLK for 48MHz USB CLK) */\
	\
	MUX_VAL(CP(HDQ_SIO),		(IEN  | PTU | DIS | M1)) \
	\
	/* Boot pins, control and clocks */\
	\
	MUX_VAL(CP(SYS_32K),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_CLKREQ),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_NIRQ),		(IEN  | PTU | EN  | M0)) \
	MUX_VAL(CP(SYS_NRESWARM),   (IEN  | PTU | DIS | M4)) /*GPIO_30*/\
	\
	MUX_VAL(CP(SYS_BOOT0),		(IEN  | PTD | DIS | M4)) /*GPIO_2*/\
	MUX_VAL(CP(SYS_BOOT1),		(IEN  | PTD | DIS | M4)) /*GPIO_3 */\
	MUX_VAL(CP(SYS_BOOT2),		(IEN  | PTD | DIS | M4)) /*GPIO_4*/\
	MUX_VAL(CP(SYS_BOOT3),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT4),		(IEN  | PTD | DIS | M4)) /*GPIO_6*/\
	MUX_VAL(CP(SYS_BOOT5),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(SYS_BOOT6),		(IEN  | PTD | DIS | M4)) /*GPIO_8*/\
	MUX_VAL(CP(SYS_BOOT7),		(IEN  | PTD | DIS | M4)) /*GPIO_115 (NOT CERTAIN)*/\
	MUX_VAL(CP(SYS_BOOT8),		(IEN  | PTD | DIS | M0)) \
	\
	MUX_VAL(CP(SYS_CLKOUT1),	(IEN  | PTU | DIS | M4)) /*GPIO_10*/\
	MUX_VAL(CP(SYS_CLKOUT2),	(IEN  | PTU | DIS | M4)) /*GPIO_186*/\
	\
	/* JTAG */\
	\
	MUX_VAL(CP(JTAG_nTRST),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TCK),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TMS),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_TDI),		(IEN  | PTD | DIS | M0)) \
	MUX_VAL(CP(JTAG_EMU0),		(IDIS | PTD | DIS | M4)) /*GPIO_11*/\
	MUX_VAL(CP(JTAG_EMU1),		(IEN  | PTD | DIS | M4)) /*GPIO_31*/\
	\
	/* ETK : Configured as HSUSB1 and HSUSB2 pins */\
	\
	MUX_VAL(CP(ETK_CLK_ES2),	(IDIS | PTU | EN  | M3)) /*HSUSB1_STP*/\
	MUX_VAL(CP(ETK_CTL_ES2),	(IDIS | PTD | DIS | M3)) /*HSUSB1_CLK*/\
	MUX_VAL(CP(ETK_D0_ES2 ),	(IEN  | PTD | DIS | M3)) /*HSUSB1_DATA0*/\
	MUX_VAL(CP(ETK_D1_ES2 ),	(IEN  | PTD | DIS | M3)) /*HSUSB1_DATA1*/\
	MUX_VAL(CP(ETK_D2_ES2 ),	(IEN  | PTD | DIS | M3)) /*HSUSB1_DATA2*/\
	MUX_VAL(CP(ETK_D3_ES2 ),	(IEN  | PTD | DIS | M3)) /*HSUSB1_DATA7*/\
	MUX_VAL(CP(ETK_D4_ES2 ),	(IEN  | PTD | DIS | M3)) /*HSUSB1_DATA4*/\
	MUX_VAL(CP(ETK_D5_ES2 ),	(IEN  | PTD | DIS | M3)) /*HSUSB1_DATA5*/\
	MUX_VAL(CP(ETK_D6_ES2 ),	(IEN  | PTD | DIS | M3)) /*HSUSB1_DATA6*/\
	MUX_VAL(CP(ETK_D7_ES2 ),	(IEN  | PTD | DIS | M3)) /*HSUSB1_DATA3*/\
	MUX_VAL(CP(ETK_D8_ES2 ),	(IEN  | PTD | DIS | M3)) /*HSUSB1_DIR*/\
	MUX_VAL(CP(ETK_D9_ES2 ),	(IEN  | PTD | DIS | M3)) /*HSUSB1_NXT*/\
	\
	MUX_VAL(CP(ETK_D10_ES2),	(IDIS | PTD | DIS | M3)) /*HSUSB2_CLK*/\
	MUX_VAL(CP(ETK_D11_ES2),	(IDIS | PTU | EN  | M3)) /*HSUSB2_STP*/\
	MUX_VAL(CP(ETK_D12_ES2),	(IEN  | PTD | DIS | M3)) /*HSUSB2_DIR*/\
	MUX_VAL(CP(ETK_D13_ES2),	(IEN  | PTD | DIS | M3)) /*HSUSB2_NXT*/\
	MUX_VAL(CP(ETK_D14_ES2),	(IEN  | PTD | DIS | M3)) /*HSUSB2_DATA0*/\
	MUX_VAL(CP(ETK_D15_ES2),	(IEN  | PTD | DIS | M3)) /*HSUSB2_DATA1*/

#endif

/*
 * Copyright (C) 2010 Texas Instruments
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
 * ----------------------------------------------------------------------------
 *
 */

#ifndef _DDR_DEFS_H
#define _DDR_DEFS_H

#include <asm/arch/hardware.h>

#ifdef CONFIG_TI816X_EVM_DDR3

#define CONFIG_TI816X_DDR3_400 /* Values supported 400,531,675,796 */
#define CONFIG_TI816X_DDR3_SW_LEVELING	/* Enable software leveling as part of DDR3 init*/


/*
 * DDR3 force values.  These are board dependent
 */

/* EVM 400 MHz clock Settings
 * EVM has only a single RANK (chip select) */
#define N_RANK			1

/*
 * Invert clock adds an additional half cycle delay on the command
 * interface.  The additional half cycle, is usually meant to enable
 * leveling in the situation that DQS is later than CK on the board.  It
 * also helps provide some additional margin for leveling.
 *
 * For the EVM this is helping us with additional room for the write
 * leveling.  Since the dqs delays are very small.
 */
#define INVERT_CLOCK		1

/*
 * CMD_SLAVE_RATIO determines where is the command placed with respect
 * to the clock edge.  This is a ratio, implying 0x100 is one cycle.
 * Ideally the command is centered so - this should be half cycle
 * delay (0x80).  But if invert clock is in use, an additional half
 * cycle must be added
 */
#define CMD_SLAVE_FROM_INV_CLOCK(i) (((i)==0) ? 0x80 : 0x100)
#define CMD_SLAVE_RATIO		CMD_SLAVE_FROM_INV_CLOCK(INVERT_CLOCK)

#ifdef TI816X_DDR3_PG_1_0
/*
 * EMIF PHY allows for controlling write DQS delay w.r.t. clock.  The
 * value may be forced or use values determined from the leveling
 * process.  Since we are doing the leveling - these are actually
 * don't care and are not used.  The force is in delay units
 */
#define WR_DQS_FORCE_3	0x00000010
#define WR_DQS_FORCE_2	0x00000010
#define WR_DQS_FORCE_1	0x00000010
#define WR_DQS_FORCE_0	0x00000010

/*
 * EMIF PHY allows for controlling how much the read DQS must be
 * delayed to sample the data correctly.  Ideally this is about half a
 * cycle.  The force here is delay units.  The value here is in use -
 * as the current leveling process may not obtain this reliably.  The
 * value has been determined via various tests on the EVM and changing
 * this setting is no recomended.
 */
#define RD_DQS_FORCE_0		0x00000028
#define RD_DQS_FORCE_1		0x00000028
#define RD_DQS_FORCE_2		0x00000028
#define RD_DQS_FORCE_3		0x00000028

/*
 * read gate represents the round trip delay from command to read data
 * on the board + some package delay.  This is the period for which
 * the bust may be tristated between a write and a read command and
 * hence must not be sampled (gated off).  This is actually determined
 * via the read leveling process and hence this value is a don't care
 * for the EVM
 */
#define RD_GATE_FORCE_3	0x44
#define RD_GATE_FORCE_2	0x44
#define RD_GATE_FORCE_1	0x44
#define RD_GATE_FORCE_0	0x44

#endif

/*
 * This represents the initial value for the leveling process.  The
 * value is a ratio - so 0x100 represents one cycle.  The real delay
 * is determined through the leveling process.
 *
 * During the leveling process, 0x20 is subtracted from the value, so
 * we have added that to the value we want to set.  We also set the
 * values such that byte3 completes leveling after byte2 and byte1
 * after byte0.
 */
#define WR_DQS_RATIO_0		0x20
#define WR_DQS_RATIO_1		0x20
#define WR_DQS_RATIO_2		0x20
#define WR_DQS_RATIO_3		0x20

#ifdef TI816X_DDR3_PG_1_0
/*
 * read dqs ratio is only used in DDR2
 */
#define RD_DQS_RATIO_0		0x40
#define RD_DQS_RATIO_1		0x40
#define RD_DQS_RATIO_2		0x40
#define RD_DQS_RATIO_3		0x40
#endif

/*
 * This represents the initial value for the leveling process.  The
 * value is a ratio - so 0x100 represents one cycle.  The real delay
 * is determined through the leveling process.
 *
 * During the leveling process, 0x20 is subtracted from the value, so
 * we have added that to the value we want to set.  We also set the
 * values such that byte3 completes leveling after byte2 and byte1
 * after byte0.
 */
#define RD_GATE_RATIO_0	0x20
#define RD_GATE_RATIO_1	0x20
#define RD_GATE_RATIO_2	0x20
#define RD_GATE_RATIO_3	0x20

#ifdef TI816X_DDR3_PG_1_0
/*
 * currently there is an issue with the automatic training process for
 * DDR3 by setting the initial leveling ratios appropriately we are
 * able to work arround write leveling and read gate leveling.  How
 * ever the automatic process may not work well for the read eye
 * training (determining rd dqs delay).  To work arround this - we
 * leverage the pre-knowledge of a working RD DQS delay and make the
 * leveling process complete by forcing good and bad values
 * This is enabled via HACK_EYE_TRAINING
 */
#define HACK_EYE_TRAINING	0

/*
 * only the rd dqs delay needs to be forced.  Others are determined via the leveling process
 */
#define USE_WR_DQS_FORCE	0
#define USE_RD_DQS_FORCE	HACK_EYE_TRAINING
#define USE_RD_GATE_FORCE	0

#endif
/*
 * data rate in MHz.  The DDR clock will be 1/2 of this value
 */
#define DDR_DATA_RATE		800

#define USE_EMIF0		1
#define USE_EMIF1		1

/*
 * EMIF Paramters.  Refer the EMIF register documentation and the
 * memory datasheet for details
 */
/* For 400 MHz */
#if defined(CONFIG_TI816X_DDR3_400)
#define EMIF_TIM1    0x0CCCE524
#define EMIF_TIM2    0x30308023
#define EMIF_TIM3    0x009F82CF
#define EMIF_SDREF   0x10000C30
#define EMIF_SDCFG   0x62A41032
#define EMIF_PHYCFG  0x0000030B

#if defined(CONFIG_TI816X_DDR3_SW_LEVELING)
/* These values are obtained from the CCS app */
#define RD_DQS_GATE	0x12A
#define RD_DQS		0x3B
#define WR_DQS		0xA6
#endif

#endif	/* CONFIG_TI816X_DDR3_400 */

/* For 531 MHz */
#if defined(CONFIG_TI816X_DDR3_531)
#define EMIF_TIM1    0x0EF136AC
#define EMIF_TIM2    0x30408063
#define EMIF_TIM3    0x009F83AF
#define EMIF_SDREF   0x1000102E
#define EMIF_SDCFG   0x62A51832
#define EMIF_PHYCFG  0x0000030C

#if defined(CONFIG_TI816X_DDR3_SW_LEVELING)
/* These values are obtained from the CCS app */
#define RD_DQS_GATE	0x13D
#define RD_DQS		0x39
#define WR_DQS		0xB4
#endif

#endif /* CONFIG_TI816X_DDR_531 */

/* For 675 MHz */
#if defined(CONFIG_TI816X_DDR3_675)
#define EMIF_TIM1    0x13358875
#define EMIF_TIM2    0x5051806C
#define EMIF_TIM3    0x009F84AF
#define EMIF_SDREF   0x10001491
#define EMIF_SDCFG   0x62A63032
#define EMIF_PHYCFG  0x0000030F

#if defined(CONFIG_TI816X_DDR3_SW_LEVELING)
/* These values are obtained from the CCS app */
#define RD_DQS_GATE	0x196
#define RD_DQS		0x39
#define WR_DQS		0x91

#endif

#endif /* CONFIG_TI816X_DDR3_675 */

/* For 796 MHz */
#if defined(CONFIG_TI816X_DDR3_796)
#define EMIF_TIM1   0x1779C9FE
#define EMIF_TIM2   0x50608074
#define EMIF_TIM3   0x009F857F
#define EMIF_SDREF  0x10001841
#define EMIF_SDCFG  0x62A73832
#define EMIF_PHYCFG 0x00000310

#if defined(CONFIG_TI816X_DDR3_SW_LEVELING)
/* These values are obtained from the CCS app */
#define RD_DQS_GATE	0x1B3
#define RD_DQS		0x35
#define WR_DQS		0x93

#endif

#endif /* CONFIG_TI816X_DDR_796 */


#if defined(CONFIG_TI816X_DDR3_SW_LEVELING)
#define WR_DQS_RATIO_BYTE_LANE3	((WR_DQS << 10) | WR_DQS)
#define WR_DQS_RATIO_BYTE_LANE2	((WR_DQS << 10) | WR_DQS)
#define WR_DQS_RATIO_BYTE_LANE1	((WR_DQS << 10) | WR_DQS)
#define WR_DQS_RATIO_BYTE_LANE0	((WR_DQS << 10) | WR_DQS)

#define WR_DATA_RATIO_BYTE_LANE3	(((WR_DQS + 0x40) << 10) | (WR_DQS + 0x40))
#define WR_DATA_RATIO_BYTE_LANE2	(((WR_DQS + 0x40) << 10) | (WR_DQS + 0x40))
#define WR_DATA_RATIO_BYTE_LANE1	(((WR_DQS + 0x40) << 10) | (WR_DQS + 0x40))
#define WR_DATA_RATIO_BYTE_LANE0	(((WR_DQS + 0x40) << 10) | (WR_DQS + 0x40))

#define RD_DQS_RATIO			((RD_DQS << 10) | RD_DQS)

#define DQS_GATE_BYTE_LANE0		((RD_DQS_GATE << 10) | RD_DQS_GATE)
#define DQS_GATE_BYTE_LANE1		((RD_DQS_GATE << 10) | RD_DQS_GATE)
#define DQS_GATE_BYTE_LANE2		((RD_DQS_GATE << 10) | RD_DQS_GATE)
#define DQS_GATE_BYTE_LANE3		((RD_DQS_GATE << 10) | RD_DQS_GATE)

#endif	/* CONFIG_TI816X_DDR3_SW_LEVELING */

#endif	/* CONFIG_TI816X_EVM_DDR3 */

#ifdef CONFIG_TI816X_EVM_DDR2

#define INVERT_CLK_OUT		0x0
#define CMD_SLAVE_RATIO		0x80
/*
 * DDR2 ratio values.  These are board dependent
 * obtained from sweep experiments
 */

/* EVM 400 MHz clock Settings */

#define WR_DQS_RATIO_BYTE_LANE3	((0x4a << 10) | 0x4a)
#define WR_DQS_RATIO_BYTE_LANE2	((0x4a << 10) | 0x4a)
#define WR_DQS_RATIO_BYTE_LANE1	((0x4a << 10) | 0x4a)
#define WR_DQS_RATIO_BYTE_LANE0	((0x4a << 10) | 0x4a)

#define WR_DATA_RATIO_BYTE_LANE3	(((0x4a + 0x40) << 10) | (0x4a + 0x40))
#define WR_DATA_RATIO_BYTE_LANE2	(((0x4a + 0x40) << 10) | (0x4a + 0x40))
#define WR_DATA_RATIO_BYTE_LANE1	(((0x4a + 0x40) << 10) | (0x4a + 0x40))
#define WR_DATA_RATIO_BYTE_LANE0	(((0x4a + 0x40) << 10) | (0x4a + 0x40))

#define RD_DQS_RATIO			((0x40 << 10) | 0x40)

#define DQS_GATE_BYTE_LANE0		((0x13a << 10) | 0x13a)
#define DQS_GATE_BYTE_LANE1		((0x13a << 10) | 0x13a)
#define DQS_GATE_BYTE_LANE2		((0x13a << 10) | 0x13a)
#define DQS_GATE_BYTE_LANE3		((0x13a << 10) | 0x13a)

/*
 * EMIF Paramters
 */
#define EMIF_TIM1    0xAAB15E2
#define EMIF_TIM2    0x423631D2
#define EMIF_TIM3    0x80032F
#define EMIF_SDREF   0x10000C30
#define EMIF_SDCFG   0x43801A3A  /* 32 bit ddr2, CL=6, CWL=5, 13 rows, 8 banks, 10 bit column, 2 CS */
#define EMIF_PHYCFG  0x0000030B  /* local odt = 3, read latency = 11 (max = 12, min=6) */

#endif	/* CONFIG_TI816X_EVM_DDR2 */

#ifdef CONFIG_TI814X_EVM_DDR2
/* ti814x specific settings to be added */
#endif

#endif  /* _DDR_DEFS_H */


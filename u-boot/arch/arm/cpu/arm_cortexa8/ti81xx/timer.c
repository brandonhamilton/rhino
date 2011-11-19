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
#include <asm/io.h>

#include <asm/arch/hardware.h>
#include <asm/arch/cpu.h>

/* Up counter */
#define TIMER_LOAD_VAL		0
#define TIMER_MAX_VAL		0xFFFFFFFF
#define TIMER_CLOCK	(CONFIG_SYS_CLK_FREQ)

/* macro to read the 32 bit timer */
#define TIMER_REG(offset)	(CONFIG_SYS_TIMERBASE + offset)

static ulong timestamp;
static ulong lastinc;

/*
 * Start the timer. The timer is an upcounter.
 * In s_init the clk source has been set to CLKIN i.e. 27MHz
 * We want the timer to be reloaded on overflow.
 * With CLKIN = 27 MHz, timer clk period = 1/27MHz = 370.37 uSec
 * Timer overflows after:
 * 	(0xFFFFFFFF - TLDR + 1) * clk period * clock divider (2^(PTV+1))
 * With PTV = 0 we get
 *	timer period = (0xFFFFFFFF - 0 + 1) * 370.37 * 10e-6 * 1 (with PRE-SCALAR disabled)
 * 	timer period = (0xFFFFFFFF - 0 + 1) * 370.37 * 10e-6 * 2 (with PRE-SCALAR enabled)
 */

int timer_init(void)
{
	int ptv = 0;
	/* Clk source selection is s_init code, we select CLKIN ie 27MHz */
	/* Keeping ptv value as 0 right now AND disabling the PRE_SCALAR */

	writel(TIMER_LOAD_VAL, TIMER_REG(TIMER_TLDR));
	writel((ptv << TCLR_PTV_SHIFT) | TCLR_PRE_DISABLE
			| TCLR_AR | TCLR_ST, TIMER_REG(TIMER_TCLR));

	reset_timer_masked();	/* init the timestamp and lastinc value */

	return 0;
}

/*
 * timer without interrupts
 */
void reset_timer(void)
{
	reset_timer_masked();
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
	timestamp = t;
}

/* delay x useconds AND perserve advance timstamp value  */
void __udelay(unsigned long usec)
{
	long tmo = usec * (TIMER_CLOCK / 1000) / 1000;
	unsigned long now, last = readl(TIMER_REG(TIMER_TCRR));

	while (tmo > 0) {
		now = readl(TIMER_REG(TIMER_TCRR));
		if (last > now) /* count up timer overflow */
			tmo -= TIMER_MAX_VAL - last + now;
		else
			tmo -= now - last;
		last = now;
	}
}

void reset_timer_masked(void)
{
	/* reset time */
	lastinc = readl(TIMER_REG(TIMER_TCRR)) / (TIMER_CLOCK / CONFIG_SYS_HZ);
	timestamp = 0;	       /* start "advancing" time stamp from 0 */
}

ulong get_timer_masked(void)
{
	/* current tick value */
	ulong now = readl(TIMER_REG(TIMER_TCRR)) / (TIMER_CLOCK / CONFIG_SYS_HZ);

	if (now >= lastinc)	/* normal mode (non roll) */
		/* move stamp fordward with absoulte diff ticks */
		timestamp += (now - lastinc);
	else	/* we have rollover of incrementer */
		timestamp += ((TIMER_MAX_VAL / (TIMER_CLOCK / CONFIG_SYS_HZ))
				- lastinc) + now;
	lastinc = now;
	/* ulong now = READ_TIMER;*/		/* current tick value */

	return timestamp;
}

/* waits specified delay value and resets timestamp */
void udelay_masked(unsigned long usec)
{
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

#ifdef CONFIG_CPU_CORTEXA8
/* The functions below are taken from cpu/arm_cortexa8/omap3/board.c
 * TODO: Will move to an appropriate place later.
 */
/******************************************************************************
 * Dummy function to handle errors for EABI incompatibility
 *****************************************************************************/
void raise(void)
{
}

/******************************************************************************
 * Dummy function to handle errors for EABI incompatibility
 *****************************************************************************/
void abort(void)
{
}
#endif


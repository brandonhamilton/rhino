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

#ifndef __CONFIG_TI8148_EVM_H
#define __CONFIG_TI8148_EVM_H

/* In the 1st stage we have just 110K, so cut down wherever possible */
#ifdef CONFIG_TI814X_MIN_CONFIG

# define CONFIG_CMD_MEMORY	/* for mtest */
# undef CONFIG_GZIP
# undef CONFIG_ZLIB
//# undef CONFIG_BOOTM_LINUX
//# undef CONFIG_BOOTM_NETBSD
//# undef CONFIG_BOOTM_RTEMS
//# undef CONFIG_SREC
//# undef CONFIG_XYZMODEM
# undef CONFIG_SYS_HUSH_PARSER
# define CONFIG_CMD_LOADB	/* loadb			*/
# define CONFIG_CMD_LOADY	/* loady */
# define CONFIG_SETUP_PLL
# define CONFIG_TI814X_CONFIG_DDR
# define CONFIG_ENV_SIZE			0x400
# define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (8 * 1024))
# define CONFIG_SYS_PROMPT		"TI-MIN#"
# define CONFIG_BOOTDELAY		3	/* set to negative value for no autoboot */
# if defined(CONFIG_SPI_BOOT)		/* Autoload the 2nd stage from SPI */
#  define CONFIG_SPI			1
#  define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0" \
	"bootcmd=sf probe 0; sf read 0x81000000 0x20000 0x40000; go 0x81000000\0" \

# elif defined(CONFIG_NAND_BOOT)		/* Autoload the 2nd stage from NAND */
#  define CONFIG_NAND			1
#  define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0" \
	"bootcmd=nand read 0x81000000 0x20000 0x40000; go 0x81000000\0" \

# elif defined(CONFIG_SD_BOOT)		/* Autoload the 2nd stage from SD */
#  define CONFIG_MMC			1
#  define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0" \
	"bootcmd=mmc init; fatload mmc 1 0x80800000 u-boot.bin; go 0x80800000\0" \

# endif

#else

# include <config_cmd_default.h>
# define CONFIG_SKIP_LOWLEVEL_INIT	/* 1st stage would have done the basic init */
# define CONFIG_ENV_SIZE			0x2000
# define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (32 * 1024))
# define CONFIG_ENV_OVERWRITE
# define CONFIG_SYS_LONGHELP
# define CONFIG_SYS_PROMPT		"TI8148_EVM#"
# define CONFIG_SYS_HUSH_PARSER		/* Use HUSH parser to allow command parsing */
# define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
# define CONFIG_CMDLINE_TAG        	1	/* enable passing of ATAGs  */
# define CONFIG_SETUP_MEMORY_TAGS  	1
# define CONFIG_INITRD_TAG	  	1	/* Required for ramdisk support */
# define CONFIG_BOOTDELAY		3	/* set to negative value for no autoboot */
/* By default, 2nd stage will have MMC, NAND, SPI and I2C support */
# define CONFIG_MMC			1
# define CONFIG_NAND			1
# define CONFIG_SPI			1
# define CONFIG_I2C			1
# define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=yes\0" \
	"bootfile=uImage\0" \
	"ramdisk_file=ramdisk.gz\0" \
	"loadaddr=0x81000000\0" \
	"script_addr=0x80900000\0" \
	"loadbootscript=fatload mmc 1 ${script_addr} boot.scr\0" \
	"bootscript= echo Running bootscript from MMC/SD to set the ENV...; " \
		"source ${script_addr}\0" \

# define CONFIG_BOOTCOMMAND \
	"if mmc init; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"echo In case ENV on MMC/SD is required; "\
			"echo Please put a valid script named boot.scr on the card; " \
			"echo Refer to the User Guide on how to generate the image; " \
		"fi; " \
	"else " \
		"echo Please set bootargs and bootcmd before booting the kernel; " \
		"echo If that has already been done please ignore this message; "\
	"fi"


#endif

#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for
						   initial data */

#define CONFIG_MISC_INIT_R		1
#ifndef CONFIG_TI814X_MIN_CONFIG
# define CONFIG_TI814X_ASCIIART		1	/* The centaur */
#endif
#define CONFIG_SYS_AUTOLOAD		"yes"
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_ECHO

/*
 * Miscellaneous configurable options
 */

/* max number of command args */
#define CONFIG_SYS_MAXARGS		32
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		512
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
/* memtest works on 8 MB in DRAM after skipping 32MB from start addr of ram disk*/
#define CONFIG_SYS_MEMTEST_START	(PHYS_DRAM_1 + (64 *1024 *1024))
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START \
					+ (8 * 1024 * 1024))

#undef  CONFIG_SYS_CLKS_IN_HZ				/* everything, incl board info, in Hz */
#define CONFIG_SYS_LOAD_ADDR		0x81000000  	/* Default load address */
#define CONFIG_SYS_HZ			1000        	/* 1ms clock */

/* Hardware related */

/**
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS		2		/* we have 2 banks of DRAM */
#define PHYS_DRAM_1			0x80000000	/* DRAM Bank #1 */
#define PHYS_DRAM_1_SIZE		0x40000000	/* 1 GB */
#define PHYS_DRAM_2			0xC0000000	/* DRAM Bank #2 */
#define PHYS_DRAM_2_SIZE		0x40000000	/* 1 GB */


/**
 * Platform/Board specific defs
 */
#define CONFIG_SYS_CLK_FREQ		27000000
#define CONFIG_SYS_TIMERBASE		0x4802E000

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		(48000000)
#define CONFIG_SYS_NS16550_COM1		0x48020000	/* Base EVM has UART0 */

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 110, 300, 600, 1200, 2400, \
4800, 9600, 14400, 19200, 28800, 38400, 56000, 57600, 115200 }

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1			1
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_CONSOLE_INFO_QUIET

#if defined(CONFIG_NO_ETH)
# undef CONFIG_CMD_NET
#else
# define CONFIG_CMD_DHCP
# define CONFIG_CMD_PING
#endif

#if defined(CONFIG_CMD_NET)
# define CONFIG_DRIVER_TI_CPSW
# define CONFIG_MII
# define CONFIG_BOOTP_DEFAULT
# define CONFIG_BOOTP_DNS
# define CONFIG_BOOTP_DNS2
# define CONFIG_BOOTP_SEND_HOSTNAME
# define CONFIG_BOOTP_GATEWAY
# define CONFIG_BOOTP_SUBNETMASK
# define CONFIG_NET_RETRY_COUNT 	10
# define CONFIG_NET_MULTI
# define CONFIG_PHY_GIGE
#endif

#if defined(CONFIG_SYS_NO_FLASH)
# define CONFIG_ENV_IS_NOWHERE
#endif

/* NAND support */
#ifdef CONFIG_NAND
#define CONFIG_CMD_NAND
#define CONFIG_NAND_TI81XX
#define GPMC_NAND_ECC_LP_x16_LAYOUT 	1
#define NAND_BASE			(0x08000000)
#define CONFIG_SYS_NAND_ADDR		NAND_BASE	/* physical address */
							/* to access nand */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access nand at */
							/* CS0 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of NAND */
#endif							/* devices */

/* ENV in NAND */
#if defined(CONFIG_NAND_ENV)
# undef CONFIG_ENV_IS_NOWHERE
# define CONFIG_ENV_IS_IN_NAND		1
# ifdef CONFIG_ENV_IS_IN_NAND
#  define CONFIG_SYS_MAX_FLASH_SECT	520		/* max number of sectors in a chip */
#  define CONFIG_SYS_MAX_FLASH_BANKS	2		/* max number of flash banks */
#  define CONFIG_SYS_MONITOR_LEN	(256 << 10)	/* Reserve 2 sectors */
#  define CONFIG_SYS_FLASH_BASE		boot_flash_base
#  define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#  define MNAND_ENV_OFFSET		0x260000	/* environment starts here */
#  define CONFIG_SYS_ENV_SECT_SIZE	boot_flash_sec
#  define CONFIG_ENV_OFFSET		boot_flash_off
#  define CONFIG_ENV_ADDR		MNAND_ENV_OFFSET
# endif

# ifndef __ASSEMBLY__
extern unsigned int boot_flash_base;
extern volatile unsigned int boot_flash_env_addr;
extern unsigned int boot_flash_off;
extern unsigned int boot_flash_sec;
extern unsigned int boot_flash_type;
# endif
#endif /* NAND support */

/* SPI support */
#ifdef CONFIG_SPI
#define CONFIG_OMAP3_SPI
#define CONFIG_MTD_DEVICE
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_SPEED	(75000000)
#endif

/* ENV in SPI */
#if defined(CONFIG_SPI_ENV)
# undef CONFIG_ENV_IS_NOWHERE
# define CONFIG_ENV_IS_IN_SPI_FLASH	1
# ifdef CONFIG_ENV_IS_IN_SPI_FLASH
#  define CONFIG_SYS_FLASH_BASE		(0)
#  define SPI_FLASH_ERASE_SIZE		(4 * 1024) /* sector size of SPI flash */
#  define CONFIG_SYS_ENV_SECT_SIZE	(2 * SPI_FLASH_ERASE_SIZE) /* env size */
#  define CONFIG_ENV_SECT_SIZE		(CONFIG_SYS_ENV_SECT_SIZE)
#  define CONFIG_ENV_OFFSET		(96 * SPI_FLASH_ERASE_SIZE)
#  define CONFIG_ENV_ADDR		(CONFIG_ENV_OFFSET)
#  define CONFIG_SYS_MAX_FLASH_SECT	(1024) /* no of sectors in SPI flash */
#  define CONFIG_SYS_MAX_FLASH_BANKS	(1)
# endif
#endif /* SPI support */

/* NOR support */
#if defined(CONFIG_NOR_BOOT)
# undef CONFIG_CMD_NAND			/* Remove NAND support */
# undef CONFIG_NAND_TI81XX
# undef CONFIG_ENV_IS_NOWHERE
# ifdef CONFIG_SYS_MALLOC_LEN
#  undef CONFIG_SYS_MALLOC_LEN
# endif
# define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1
# define CONFIG_SYS_MALLOC_LEN		(0x100000)
# define CONFIG_SYS_FLASH_CFI
# define CONFIG_FLASH_CFI_DRIVER
# define CONFIG_FLASH_CFI_MTD
# define CONFIG_SYS_MAX_FLASH_SECT	512
# define CONFIG_SYS_MAX_FLASH_BANKS	1
# define CONFIG_SYS_FLASH_BASE		(0x08000000)
# define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
# define CONFIG_ENV_IS_IN_FLASH	1
# define NOR_SECT_SIZE			(128 * 1024)
# define CONFIG_SYS_ENV_SECT_SIZE	(NOR_SECT_SIZE)
# define CONFIG_ENV_SECT_SIZE		(NOR_SECT_SIZE)
# define CONFIG_ENV_OFFSET		(2 * NOR_SECT_SIZE)
# define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# define CONFIG_MTD_DEVICE
#endif	/* NOR support */


/* No I2C support in 1st stage */
#ifdef CONFIG_I2C

# define CONFIG_CMD_I2C
# define CONFIG_HARD_I2C			1
# define CONFIG_SYS_I2C_SPEED		100000
# define CONFIG_SYS_I2C_SLAVE		1
# define CONFIG_SYS_I2C_BUS		0
# define CONFIG_SYS_I2C_BUS_SELECT	1
# define CONFIG_DRIVER_TI81XX_I2C	1

/* EEPROM definitions */
# define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		3
# define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
# define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	6
# define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	20

#endif

/* HSMMC support */
#ifdef CONFIG_MMC
# define CONFIG_OMAP3_MMC	1
# define CONFIG_CMD_MMC		1
# define CONFIG_DOS_PARTITION	1
# define CONFIG_CMD_FAT		1
#endif

/* Unsupported features */
#undef CONFIG_USE_IRQ

#endif	  /* ! __CONFIG_TI8148_EVM_H */


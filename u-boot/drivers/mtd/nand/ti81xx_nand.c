/*
 * (C) Copyright 2004-2008 Texas Instruments, <www.ti.com>
 * Mansoor Ahamed <mansoor.ahamed@ti.com>
 *
 * Derived from work done by Rohit Choraria <rohitkc@ti.com> for omap
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
 */

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mem.h>
#include <asm/arch/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <nand.h>

struct nand_bch_priv {
	uint8_t mode;
	uint8_t type;
	uint8_t nibbles;
};

/* bch types */
#define ECC_BCH4	0
#define ECC_BCH8	1
#define ECC_BCH16	2

/* BCH nibbles for diff bch levels */
#define NAND_ECC_HW_BCH ((uint8_t)(NAND_ECC_HW_OOB_FIRST) + 1)
#define ECC_BCH4_NIBBLES	13
#define ECC_BCH8_NIBBLES	26
#define ECC_BCH16_NIBBLES	52

static uint8_t cs;
static struct nand_ecclayout hw_nand_oob = GPMC_NAND_HW_ECC_LAYOUT_KERNEL;
static struct nand_ecclayout hw_bch4_nand_oob = GPMC_NAND_HW_BCH4_ECC_LAYOUT;
static struct nand_ecclayout hw_bch8_nand_oob = GPMC_NAND_HW_BCH8_ECC_LAYOUT;
static struct nand_ecclayout hw_bch16_nand_oob = GPMC_NAND_HW_BCH16_ECC_LAYOUT;


static struct nand_bch_priv bch_priv = {
	.mode = NAND_ECC_HW_BCH,
	.type = ECC_BCH8,
	.nibbles = ECC_BCH8_NIBBLES
};

/*
 * ti81xx_read_bch8_result - Read BCH result for BCH8 level
 *
 * @mtd:	MTD device structure
 * @big_endian:	When set read register 3 first
 * @ecc_code:	Read syndrome from BCH result registers
 */
static void ti81xx_read_bch8_result(struct mtd_info *mtd, uint8_t big_endian,
				uint8_t *ecc_code)
{
	uint32_t *ptr;
	int8_t i = 0, j;

	if (big_endian) {
		ptr = &gpmc_cfg->bch_result_0_3[0].bch_result_x[3];
		ecc_code[i++] = readl(ptr) & 0xFF;
		ptr--;
		for (j = 0; j < 3; j++) {
			ecc_code[i++] = (readl(ptr) >> 24) & 0xFF;
			ecc_code[i++] = (readl(ptr) >> 16) & 0xFF;
			ecc_code[i++] = (readl(ptr) >>  8) & 0xFF;
			ecc_code[i++] = readl(ptr) & 0xFF;
			ptr--;
		}
	}
	else {
		ptr = &gpmc_cfg->bch_result_0_3[0].bch_result_x[0];
		for (j = 0; j < 3; j++) {
			ecc_code[i++] = readl(ptr) & 0xFF;
			ecc_code[i++] = (readl(ptr) >>  8) & 0xFF;
			ecc_code[i++] = (readl(ptr) >> 16) & 0xFF;
			ecc_code[i++] = (readl(ptr) >> 24) & 0xFF;
			ptr++;
		}
		ecc_code[i++] = readl(ptr) & 0xFF;
	}
}

/*
 * ti81xx_ecc_disable - Disable H/W ECC calculation
 *
 * @mtd:	MTD device structure
 *
 */
static void ti81xx_ecc_disable(struct mtd_info *mtd) {

	writel((readl(&gpmc_cfg->ecc_config) & ~0x1),
		&gpmc_cfg->ecc_config);
}

#if 0
/*
 * ti81xx_ecc_enable_bch - Enable BCH H/W ECC calculation
 *
 * @mtd:	MTD device structure
 *
 */
static void ti81xx_ecc_enable_bch(struct mtd_info *mtd) {
	uint32_t val;

	val = readl(&gpmc_cfg->ecc_control);
	val |= 0x00000100u;
	writel(val, &gpmc_cfg->ecc_control); /* clear ECC outputs */

	val = readl(&gpmc_cfg->ecc_control);
	val &= ~0xF;
	val |= 0x1;
	writel(val, &gpmc_cfg->ecc_control);  /* reset ecc pointer to result 1 */

	val = readl(&gpmc_cfg->ecc_config);
	val |= 0x1;
	writel(val, &gpmc_cfg->ecc_config); /* enable ecc */
}
#endif

/*
 * ti81xx_nand_hwcontrol - Set the address pointers correctly for the
 *			following address/data/command operation
 */
static void ti81xx_nand_hwcontrol(struct mtd_info *mtd, int32_t cmd,
				uint32_t ctrl)
{
	register struct nand_chip *this = mtd->priv;

	/*
	 * Point the IO_ADDR to DATA and ADDRESS registers instead
	 * of chip address
	 */
	switch (ctrl) {
	case NAND_CTRL_CHANGE | NAND_CTRL_CLE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_cmd;
		break;
	case NAND_CTRL_CHANGE | NAND_CTRL_ALE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_adr;
		break;
	case NAND_CTRL_CHANGE | NAND_NCE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_dat;
		break;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

/*
 * ti81xx_hwecc_init_bch - Initialize the BCH Hardware ECC for NAND flash in
 * 				GPMC controller
 * @mtd:       MTD device structure
 * @mode:	Read/Write mode
 */
static void ti81xx_hwecc_init_bch(struct nand_chip *chip, int32_t mode)
{
	uint32_t val, dev_width = (chip->options & NAND_BUSWIDTH_16) >> 1;
	uint32_t unused_length = 0;
	struct nand_bch_priv *bch = chip->priv;

	switch(bch->nibbles) {
		case ECC_BCH4_NIBBLES:
			unused_length = 3;
			break;
		case ECC_BCH8_NIBBLES:
			unused_length = 2;
			break;
		case ECC_BCH16_NIBBLES:
			unused_length = 0;
			break;
	}

	/* Clear the ecc result registers, select ecc reg as 1 */
	writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);

	switch (mode) {
		case NAND_ECC_WRITE:
			/* eccsize1 config */
			val = ((unused_length + bch->nibbles) << 22);
			break;

		case NAND_ECC_READ:
		default:
			/* by default eccsize0 selected for ecc1resultsize */
			/* eccsize0 config */
			val  = (bch->nibbles << 12);
			/* eccsize1 config */
			val |= (unused_length << 22);
			break;
	}
	/* ecc size configuration */
	writel(val, &gpmc_cfg->ecc_size_config);
	/* by default 512bytes sector page is selected */
	/* set bch mode */
	val  = (1 << 16);
	/* bch4 / bch8 / bch16 */
	val |= (bch->type << 12);
	/* set wrap mode to 1 */
	val |= (1 << 8);
	val |= (dev_width << 7);
	val |= (cs << 1);
	/* enable ecc */
	/* val |= (1); */ /* should not enable ECC just init i.e. config */
	writel(val, &gpmc_cfg->ecc_config);
}


/*
 * ti81xx_hwecc_init - Initialize the Hardware ECC for NAND flash in
 *                   GPMC controller
 * @mtd:        MTD device structure
 *
 */
static void ti81xx_hwecc_init(struct nand_chip *chip)
{
	/*
	 * Init ECC Control Register
	 * Clear all ECC | Enable Reg1
	 */
	writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);
	writel(ECCSIZE1 | ECCSIZE0 | ECCSIZE0SEL, &gpmc_cfg->ecc_size_config);
}

/*
 * gen_true_ecc - This function will generate true ECC value, which
 * can be used when correcting data read from NAND flash memory core
 *
 * @ecc_buf:	buffer to store ecc code
 *
 * @return:	re-formatted ECC value
 */
static uint32_t gen_true_ecc(uint8_t *ecc_buf)
{
	return ecc_buf[0] | (ecc_buf[1] << 16) | ((ecc_buf[2] & 0xF0) << 20) |
		((ecc_buf[2] & 0x0F) << 8);
}

/*
 * ti81xx_rotate_ecc_bch - Rotate the syndrome bytes
 *
 * @mtd:	MTD device structure
 * @calc_ecc:	ECC read from ECC registers
 * @syndrome:	Rotated syndrome will be retuned in this array
 *
 */
static void ti81xx_rotate_ecc_bch(struct mtd_info *mtd, uint8_t *calc_ecc,
		uint8_t *syndrome)
{
	struct nand_chip *chip = mtd->priv;
	struct nand_bch_priv *bch = chip->priv;
	uint8_t n_bytes = 0;
	int8_t i,j;

	switch(bch->type) {
		case ECC_BCH4:
			n_bytes = 8;
			break;

		case ECC_BCH16:
			n_bytes = 28;
			break;

		case ECC_BCH8:
		default:
			n_bytes = 13;
			break;
	}

	for (i = 0, j = (n_bytes-1); i < n_bytes; i++, j--)
		syndrome[i] =  calc_ecc[j];
}


/*
 * ti81xx_fix_errors_bch - Correct bch error in the data
 *
 * @mtd:	MTD device structure
 * @data:	Data read from flash
 * @error_count:Number of errors in data
 * @error_loc:	Locations of errors in the data
 *
 */
static void ti81xx_fix_errors_bch(struct mtd_info *mtd, uint8_t *data,
		uint32_t error_count, uint32_t *error_loc)
{
	struct nand_chip *chip = mtd->priv;
	struct nand_bch_priv *bch = chip->priv;
	uint8_t count = 0;
	uint32_t error_byte_pos;
	uint32_t error_bit_mask;
	uint32_t last_bit = (bch->nibbles * 4) - 1;

	/* Flip all bits as specified by the error location array. */
	/* FOR( each found error location flip the bit ) */
	for (count = 0; count < error_count; count++) {
		if (error_loc[count] > last_bit) {
			/* Remove the ECC spare bits from correction. */
			error_loc[count] -= (last_bit + 1);
			/* Offset bit in data region */
			error_byte_pos = (512 * 8) - (error_loc[count] / 8) - 1;
			/* Error Bit mask */
			error_bit_mask = 0x1 << (error_loc[count] % 8);
			/* Toggle the error bit to make the correction. */
			data[error_byte_pos] ^= error_bit_mask;
		}
	}
}

/*
 * ti81xx_correct_data_bch - Compares the ecc read from nand spare area
 * with ECC registers values and corrects one bit error if it has occured
 *
 * @mtd:	MTD device structure
 * @dat:	page data
 * @read_ecc:	ecc read from nand flash (ignored)
 * @calc_ecc:	ecc read from ECC registers
 *
 * @return 0 if data is OK or corrected, else returns -1
 */
static int ti81xx_correct_data_bch(struct mtd_info *mtd, uint8_t *dat,
				uint8_t *read_ecc, uint8_t *calc_ecc)
{
	struct nand_chip *chip = mtd->priv;
	struct nand_bch_priv *bch = chip->priv;
	uint8_t syndrome[28];
	uint32_t error_count = 0;
	uint32_t error_loc[8];

	elm_reset();
	elm_config((enum bch_level)(bch->type));

	/* while reading ECC result we read it in big endian.
	 * Hence while loading to ELM we have rotate to get the right endian.
	 */
	ti81xx_rotate_ecc_bch(mtd, calc_ecc, syndrome);

#ifdef NAND_DEBUG
	{
		uint8_t i = 0;
		printf("----\necc\n---\n");
		for (i = 0; i < 13; i++)
			printf("  0x%2x", syndrome[i]);
		printf("\n");
	}
#endif


	/* use elm module to check for errors */
	if (elm_check_error(syndrome, bch->nibbles, &error_count, error_loc) != 0) {
		printf("ECC: uncorrectable.\n");
		return -1;
	}

	/* correct bch error */
	if (error_count > 0) {
		ti81xx_fix_errors_bch(mtd, dat, error_count, error_loc);
	}

	return 0;
}


/*
 * ti81xx_correct_data - Compares the ecc read from nand spare area with ECC
 * registers values and corrects one bit error if it has occured
 * Further details can be had from TI81xx TRM and the following selected links:
 * http://en.wikipedia.org/wiki/Hamming_code
 * http://www.cs.utexas.edu/users/plaxton/c/337/05f/slides/ErrorCorrection-4.pdf
 *
 * @mtd:		 MTD device structure
 * @dat:		 page data
 * @read_ecc:		 ecc read from nand flash
 * @calc_ecc:		 ecc read from ECC registers
 *
 * @return 0 if data is OK or corrected, else returns -1
 */
static int ti81xx_correct_data(struct mtd_info *mtd, uint8_t *dat,
				uint8_t *read_ecc, uint8_t *calc_ecc)
{
	uint32_t orig_ecc, new_ecc, res, hm;
	uint16_t parity_bits, byte;
	uint8_t bit;

	/* Regenerate the orginal ECC */
	orig_ecc = gen_true_ecc(read_ecc);
	new_ecc = gen_true_ecc(calc_ecc);
	/* Get the XOR of real ecc */
	res = orig_ecc ^ new_ecc;
	if (res) {
		/* Get the hamming width */
		hm = hweight32(res);
		/* Single bit errors can be corrected! */
		if (hm == 12) {
			/* Correctable data! */
			parity_bits = res >> 16;
			bit = (parity_bits & 0x7);
			byte = (parity_bits >> 3) & 0x1FF;
			/* Flip the bit to correct */
			dat[byte] ^= (0x1 << bit);
		} else if (hm == 1) {
			printf("Error: Ecc is wrong\n");
			/* ECC itself is corrupted */
			return 2;
		} else {
			/*
			 * hm distance != parity pairs OR one, could mean 2 bit
			 * error OR potentially be on a blank page..
			 * orig_ecc: contains spare area data from nand flash.
			 * new_ecc: generated ecc while reading data area.
			 * Note: if the ecc = 0, all data bits from which it was
			 * generated are 0xFF.
			 * The 3 byte(24 bits) ecc is generated per 512byte
			 * chunk of a page. If orig_ecc(from spare area)
			 * is 0xFF && new_ecc(computed now from data area)=0x0,
			 * this means that data area is 0xFF and spare area is
			 * 0xFF. A sure sign of a erased page!
			 */
			if ((orig_ecc == 0x0FFF0FFF) && (new_ecc == 0x00000000))
				return 0;
			printf("Error: Bad compare! failed\n");
			/* detected 2 bit error */
			return -1;
		}
	}
	return 0;
}

/*
 *  ti81xx_calculate_ecc_bch - Read BCH ECC result
 *
 *  @mtd:	MTD structure
 *  @dat:	unused
 *  @ecc_code:	ecc_code buffer
 */
static int ti81xx_calculate_ecc_bch(struct mtd_info *mtd, const uint8_t *dat,
				uint8_t *ecc_code)
{
	struct nand_chip *chip = mtd->priv;
	struct nand_bch_priv *bch = chip->priv;
	uint8_t big_endian = 1;
	int8_t ret = 0;

	if (bch->type == ECC_BCH8)
		ti81xx_read_bch8_result(mtd, big_endian, ecc_code);
	else /* BCH4 and BCH16 currently not supported */
		ret = -1;

#ifdef NAND_DEBUG
	{
		int8_t i = 0;
		printf("----\nECC\n---\n");
		for (i = 0; i < 13; i++)
			printf("  0x%2x", ecc_code[i]);
		printf("\n");
	}
#endif

	/*
	 * Stop reading anymore ECC vals and clear old results
	 * enable will be called if more reads are required
	 */
	ti81xx_ecc_disable(mtd);

	return ret;
}


/*
 *  ti81xx_calculate_ecc - Generate non-inverted ECC bytes.
 *
 *  Using noninverted ECC can be considered ugly since writing a blank
 *  page ie. padding will clear the ECC bytes. This is no problem as
 *  long nobody is trying to write data on the seemingly unused page.
 *  Reading an erased page will produce an ECC mismatch between
 *  generated and read ECC bytes that has to be dealt with separately.
 *  E.g. if page is 0xFF (fresh erased), and if HW ECC engine within GPMC
 *  is used, the result of read will be 0x0 while the ECC offsets of the
 *  spare area will be 0xFF which will result in an ECC mismatch.
 *  @mtd:	MTD structure
 *  @dat:	unused
 *  @ecc_code:	ecc_code buffer
 */
static int ti81xx_calculate_ecc(struct mtd_info *mtd, const uint8_t *dat,
				uint8_t *ecc_code)
{
	u_int32_t val;

	/* Start Reading from HW ECC1_Result = 0x200 */
	val = readl(&gpmc_cfg->ecc1_result);

	ecc_code[0] = val & 0xFF;
	ecc_code[1] = (val >> 16) & 0xFF;
	ecc_code[2] = ((val >> 8) & 0x0F) | ((val >> 20) & 0xF0);

	/*
	 * Stop reading anymore ECC vals and clear old results
	 * enable will be called if more reads are required
	 */
	writel(0x000, &gpmc_cfg->ecc_config);

	return 0;
}

/**
 * ti81xx_write_page_bch - [REPLACABLE] hardware ecc based page write function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	data buffer
 */
static void ti81xx_write_page_bch(struct mtd_info *mtd,
	struct nand_chip *chip, const uint8_t *buf)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	const uint8_t *p = buf;
	uint32_t *eccpos = chip->ecc.layout->eccpos;

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
		chip->write_buf(mtd, p, eccsize);
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);
	}

	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];

	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);
}


/**
 * ti81xx_read_page_bch - hardware ecc based page read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	buffer to store read data
 * @page:	page number to read
 *
 */
static int ti81xx_read_page_bch(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int page)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint8_t *ecc_code = chip->buffers->ecccode;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint8_t *oob = chip->oob_poi;
	uint32_t data_pos;
	uint32_t oob_pos;

	data_pos = 0;
	/* oob area start */
	oob_pos = (eccsize * eccsteps) + chip->ecc.layout->eccpos[0];
	//oob_pos = (eccsize * eccsteps) + 2;

	chip->ecc.hwctl(mtd, NAND_ECC_READ);
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize,
				oob += eccbytes) {
		/* read data */
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, data_pos, page);
		chip->read_buf(mtd, p, eccsize);
#ifdef NAND_DEBUG
		{
			uint32_t j;
			printf("----\nDATA\n---\n");
			for (j = 0; j < 13; j++)
				printf("  0x%2x", p[j]);
			printf("\n");
		}
#endif

		/* read respective ecc from oob area */
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, oob_pos, page);
		chip->read_buf(mtd, oob, eccbytes);
#ifdef NAND_DEBUG
		{
			uint32_t j;
			printf("----\nOOB\n---\n");
			for (j = 0; j < 13; j++)
				printf("  0x%2x", oob[j]);
			printf("\n");
		}
#endif
		/* read syndrome */
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

		data_pos += eccsize;
		oob_pos += eccbytes;
	}

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccpos[i]];

	eccsteps = chip->ecc.steps;
	p = buf;

	for (i = 0 ; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;

		stat = chip->ecc.correct(mtd, p, &ecc_code[i], &ecc_calc[i]);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}
	return 0;
}


/*
 * ti81xx_enable_ecc_bch- This function enables the bch h/w ecc functionality
 * @mtd:        MTD device structure
 * @mode:       Read/Write mode
 *
 */
static void ti81xx_enable_ecc_bch(struct mtd_info *mtd, int32_t mode)
{
	struct nand_chip *chip = mtd->priv;

	ti81xx_hwecc_init_bch(chip, mode);
	/* enable ecc */
	writel((readl(&gpmc_cfg->ecc_config) | 0x1), &gpmc_cfg->ecc_config);
}


/*
 * ti81xx_enable_ecc - This function enables the hardware ecc functionality
 * @mtd:        MTD device structure
 * @mode:       Read/Write mode
 */
static void ti81xx_enable_ecc(struct mtd_info *mtd, int32_t mode)
{
	struct nand_chip *chip = mtd->priv;
	uint32_t val, dev_width = (chip->options & NAND_BUSWIDTH_16) >> 1;

	switch (mode) {
	case NAND_ECC_READ:
	case NAND_ECC_WRITE:
		/* Clear the ecc result registers, select ecc reg as 1 */
		writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);

		/*
		 * Size 0 = 0xFF, Size1 is 0xFF - both are 512 bytes
		 * tell all regs to generate size0 sized regs
		 * we just have a single ECC engine for all CS
		 */
		writel(ECCSIZE1 | ECCSIZE0 | ECCSIZE0SEL,
			&gpmc_cfg->ecc_size_config);
		val = (dev_width << 7) | (cs << 1) | (0x1);
		writel(val, &gpmc_cfg->ecc_config);
		break;
	default:
		printf("Error: Unrecognized Mode[%d]!\n", mode);
		break;
	}
}
/*
 * __ti81xx_nand_switch_ecc - switch the ECC operation ib/w h/w ecc
 * (i.e. hamming / bch) and s/w ecc.
 * The default is to come up on s/w ecc
 *
 * @nand:	NAND chip datastructure
 * @hardware:  NAND_ECC_HW -switch to h/w ecc
 * 				NAND_ECC_SOFT -switch to s/w ecc
 *
 * @mode: 		0 - hamming code
 * 				1 - bch4
 *				2 - bch8
 *				3 - bch16
 *
 *
 */
void __ti81xx_nand_switch_ecc(struct nand_chip *nand,
		nand_ecc_modes_t hardware, int32_t mode)
{
	struct nand_bch_priv *bch;

	bch = nand->priv;
	nand->options |= NAND_OWN_BUFFERS;

	/* Reset ecc interface */
	nand->ecc.read_page = NULL;
	nand->ecc.write_page = NULL;
	nand->ecc.read_oob = NULL;
	nand->ecc.write_oob = NULL;
	nand->ecc.hwctl = NULL;
	nand->ecc.correct = NULL;
	nand->ecc.calculate = NULL;

	nand->ecc.write_page = NULL;
	nand->ecc.read_page = NULL;
	nand->ecc.mode = hardware;
	/* Setup the ecc configurations again */
	if (hardware == NAND_ECC_HW) {
		if (mode) {
			bch->mode = NAND_ECC_HW_BCH;
			/* -1 for converting mode to bch type */
			bch->type = mode - 1;
			printf("HW ECC BCH");
			switch (bch->type) {
				case ECC_BCH4:
					nand->ecc.layout = &hw_bch4_nand_oob;
					bch->nibbles = ECC_BCH4_NIBBLES;
					nand->ecc.bytes = 8;
					printf("4 not supported\n");
					goto no_support;
					break;
				case ECC_BCH16:
					nand->ecc.bytes = 26;
					nand->ecc.layout = &hw_bch16_nand_oob;
					bch->nibbles = ECC_BCH16_NIBBLES;
					printf("16 not supported\n");
					goto no_support;
					break;
				case ECC_BCH8:
				default:
					nand->ecc.bytes = 14;
					nand->ecc.layout = &hw_bch8_nand_oob;
					bch->nibbles = ECC_BCH8_NIBBLES;
					printf("8 Selected\n");
					break;
			}
			bch->mode = NAND_ECC_HW;
			nand->ecc.mode = NAND_ECC_HW_SYNDROME;
			nand->ecc.steps = 4;
			nand->ecc.size = 512;
			nand->ecc.total = (nand->ecc.steps * nand->ecc.bytes);
			nand->ecc.write_page = ti81xx_write_page_bch;
			nand->ecc.read_page = ti81xx_read_page_bch;
			nand->ecc.hwctl = ti81xx_enable_ecc_bch;
			nand->ecc.correct = ti81xx_correct_data_bch;
			nand->ecc.calculate = ti81xx_calculate_ecc_bch;
			ti81xx_hwecc_init_bch(nand, NAND_ECC_READ);
		} else {
			bch->mode = NAND_ECC_HW;
			nand->ecc.layout = &hw_nand_oob;
			nand->ecc.size = 512;
			nand->ecc.bytes = 3;
			nand->ecc.hwctl = ti81xx_enable_ecc;
			nand->ecc.correct = ti81xx_correct_data;
			nand->ecc.calculate = ti81xx_calculate_ecc;
			ti81xx_hwecc_init(nand);
			printf("HW ECC Hamming Code selected\n");
		}
	} else if(hardware == NAND_ECC_SOFT) {
		/* Use mtd default settings */
		nand->ecc.layout = NULL;
		printf("SW ECC selected\n");
	} else {
		printf("ECC Disabled\n");
	}

no_support:
	return;
}


/*
 * ti81xx_nand_switch_ecc - switch the ECC operation ib/w h/w ecc
 * (i.e. hamming / bch) and s/w ecc.
 * The default is to come up on s/w ecc
 *
 * @hardware -  NAND_ECC_HW -switch to h/w ecc
 * 				NAND_ECC_SOFT -switch to s/w ecc
 *
 * @mode - 		0 - hamming code
 * 				1 - bch4
 *				2 - bch8
 *				3 - bch16
 *
 *
 */
void ti81xx_nand_switch_ecc(nand_ecc_modes_t hardware, int32_t mode)
{
	struct nand_chip *nand;
	struct mtd_info *mtd;

	if (nand_curr_device < 0 ||
	    nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE) {
		printf("Error: Can't switch ecc, no devices available\n");
		return;
	}

	mtd = &nand_info[nand_curr_device];
	nand = mtd->priv;

	__ti81xx_nand_switch_ecc(nand, hardware, mode);

	/* Update NAND handling after ECC mode switch */
	nand_scan_tail(mtd);

	nand->options &= ~NAND_OWN_BUFFERS;
	return;
}

/*
 * Board-specific NAND initialization. The following members of the
 * argument are board-specific:
 * - IO_ADDR_R: address to read the 8 I/O lines of the flash device
 * - IO_ADDR_W: address to write the 8 I/O lines of the flash device
 * - cmd_ctrl: hardwarespecific function for accesing control-lines
 * - waitfunc: hardwarespecific function for accesing device ready/busy line
 * - ecc.hwctl: function to enable (reset) hardware ecc generator
 * - ecc.mode: mode of ecc, see defines
 * - chip_delay: chip dependent delay for transfering data from array to
 *   read regs (tR)
 * - options: various chip options. They can partly be set to inform
 *   nand_scan about special functionality. See the defines for further
 *   explanation
 */
int board_nand_init(struct nand_chip *nand)
{
	/* int32_t gpmc_config = 0; */
	cs = 0;

	/*
	 * xloader/Uboot's gpmc configuration would have configured GPMC for
	 * nand type of memory. The following logic scans and latches on to the
	 * first CS with NAND type memory.
	 * TBD: need to make this logic generic to handle multiple CS NAND
	 * devices.
	 */
	while (cs < GPMC_MAX_CS) {
		/* Check if NAND type is set */
		if ((readl(&gpmc_cfg->cs[cs].config1) & 0xC00) == 0x800) {
			/* Found it!! */
#ifdef NAND_DEBUG
			printf("Searching for NAND device @ GPMC CS:%1d\n", cs);
#endif
			break;
		}
		cs++;
	}
	if (cs >= GPMC_MAX_CS) {
		printf("NAND: Unable to find NAND settings in "
			"GPMC Configuration - quitting\n");
		return -ENODEV;
	}

	nand->IO_ADDR_R = (void __iomem *)&gpmc_cfg->cs[cs].nand_dat;
	nand->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_cmd;

	nand->cmd_ctrl = ti81xx_nand_hwcontrol;
	nand->options = NAND_NO_PADDING | NAND_CACHEPRG | NAND_NO_AUTOINCR;
	/* If we are 16 bit dev, our gpmc config tells us that */
	if ((readl(&gpmc_cfg->cs[cs].config1) & 0x3000) == 0x1000) {
		nand->options |= NAND_BUSWIDTH_16;
	}

	nand->chip_delay = 100;

	/* fallback ecc info, this will be overridden by 
	 * ti81xx_nand_switch_ecc() below to 1-bit h/w ecc
	 */
	nand->priv = &bch_priv;
	nand->ecc.mode = NAND_ECC_SOFT;

	/* required in case of BCH */
	elm_init();

	nand_curr_device = 0;
	ti81xx_nand_switch_ecc(NAND_ECC_HW, 0);

	return 0;
}


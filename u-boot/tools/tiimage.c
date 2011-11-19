/*
 * tiimage.c
 *
 * mkimage extension for Texas Instruments processors
 *
 * Copyright (C) 2010, Texas Instruments, Incorporated
 *
 * Author:
 * Mansoor Ahamed  <mansoor.ahamed@ti.com>
 *
 * Referenced tools/imximage.c
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

/* Required to obtain the getline prototype from stdio.h */
#define _GNU_SOURCE

/* TODO: Instead of device macros we should use -n option to get device info
 * and APIs associated with that TI device.
 */

#ifndef __ASSEMBLY__
#define __ASSEMBLY__
#endif
#define __ASM_STUB_PROCESSOR_H__
#include <config.h>
#undef __ASSEMBLY__

#include "mkimage.h"
#include <image.h>
#include "tiimage.h"

static struct ti_header tiimage_header;

/* SPI image for TI81XX needs endian conversion */
#if defined(CONFIG_TI81XX_SPI_BOOT)
static uint32_t tiimage_swap32(uint32_t data)
{
	uint32_t result = 0;
	result  = (data & 0xFF000000) >> 24;
	result |= (data & 0x00FF0000) >> 8;
	result |= (data & 0x0000FF00) << 8;
	result |= (data & 0x000000FF) << 24;
	return result;
}

static int ti81xximage_spi(void *hdr, int hdr_size,
	char *in_file, char *out_file)
{
	FILE *in_fp = NULL;
	FILE *out_fp = NULL;
	uint32_t *rd;
	uint8_t rds[4];
	uint32_t wr = 0;

	in_fp = fopen(in_file, "r");
	out_fp = fopen(out_file, "w");

	if(in_fp == NULL || out_fp == NULL) {
		printf("file open error \n");
		return 0;
	}
	rd = (uint32_t*)(hdr);

	while (hdr_size > 0) {
		wr = tiimage_swap32(*rd);
		fwrite(&wr, sizeof(uint32_t), 1, out_fp);
		hdr_size -= sizeof(uint32_t);
		rd++;
	}

	rd = (uint32_t*)(rds);
	while (!feof(in_fp)) {
		fread(rds, sizeof(uint32_t), 1, in_fp);
		if (feof(in_fp))
			break;
		wr = tiimage_swap32(*rd);
		fwrite(&wr, sizeof(uint32_t), 1, out_fp);
	}

	fclose(in_fp);
	fclose(out_fp);
	return 0;
}
#endif

static int tiimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_TIIMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int tiimage_verify_header(uint8_t *ptr, int image_size,
			struct mkimage_params *params)
{
	return 0;
}

static void ti81xximage_print_header(const void *ptr)
{
#if !defined(CONFIG_TI814X_PERIPHERAL_BOOT) && !defined(CONFIG_TI_DUMMY_HEADER)
	struct ti_header *ti_hdr = (struct ti_header *) ptr;
	printf("Image Type:   Texas Instruments ti81xx Boot Image\n");
	printf("Image Size:   ");
	genimg_print_size(ti_hdr->image_size);
	printf("Load Address: %08x\n", ti_hdr->load_addr);
	printf("Entry Point:  %08x\n", ti_hdr->load_addr);
#endif
}

#if defined(CONFIG_TI816X)
static void ti816ximage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct mkimage_params *params)
{
	struct ti_header *hdr = (struct ti_header *)ptr;
	FILE *data_fp = NULL;
	uint32_t data_size = 0;
#if defined(CONFIG_TI81XX_SPI_BOOT)
	char *spi_out_file;
#endif

	/* Set default offset */
	hdr->load_addr = params->ep;

	if ((data_fp = fopen(params->datafile, "r")) == NULL) {
		printf("Data FILE [%s] open error.\n", params->datafile);
		return;
	}

	/* calculate image size */
	fseek(data_fp, 0, SEEK_END);
	data_size = ftell(data_fp);
	hdr->image_size = data_size;
	fclose(data_fp);

	/* generate spi image after doing endian conversion */
#if defined(CONFIG_TI81XX_SPI_BOOT)
	spi_out_file = malloc(strlen(params->imagefile) + 6);
	strcpy(spi_out_file, params->imagefile);
	strcat(spi_out_file, ".spi");
	ti81xximage_spi(hdr, sizeof(struct ti_header),
		params->datafile, spi_out_file);
	free(spi_out_file);
#endif
}
#elif defined(CONFIG_TI814X) && defined(CONFIG_TI814X_MIN_CONFIG)
static void ti814ximage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct mkimage_params *params)
{
	struct ti_header *hdr = (struct ti_header *)ptr;
#if !defined(CONFIG_TI814X_PERIPHERAL_BOOT)
	FILE *data_fp = NULL;
	uint32_t data_size = 0;

	/* Set default offset */
	hdr->load_addr = params->ep;

	if ((data_fp = fopen(params->datafile, "r")) == NULL) {
		printf("Data FILE [%s] open error.\n", params->datafile);
		return;
	}

	/* calculate image size */
	fseek(data_fp, 0, SEEK_END);
	data_size = ftell(data_fp);
	/* image size should include initial jump, jump location and
	 * stack. (sizeof(unsigned int) * 2) is for excluding, image size
	 * and load address fields in the header.
	 */
	hdr->image_size = data_size + sizeof(struct ti_header) -
				(sizeof(unsigned int) * 2);
	fclose(data_fp);
#endif
	/* add short jump detals to hdr */
	hdr->ldrpc	= 0xe51ff004; /* opcode for 'ldr pc, [pc, #-4]' */
	/* loc will be placed immediate after ldrpc in memroy and
	 * it will have the location to jump
	 */
	hdr->loc	= params->ep + CONFIG_TI814X_STACK;

	/* spi image has to be endian swapped */
#if defined(CONFIG_TI81XX_SPI_BOOT)
	if (strstr(params->imagefile, "min.spi") != NULL) {
		/* generate spi image */
		ti81xximage_spi(hdr, sizeof(struct ti_header),
			params->datafile, "u-boot.min.spi");
	}
#endif
}
#else
static void ti_dummy_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct mkimage_params *params)
{
	/* Dummy function for set header */
}
#endif

int tiimage_check_params(struct mkimage_params *params)
{
	if (!params || !params->dflag || !params->eflag)
		return -1;

	if (!strlen(params->datafile)) {
		fprintf(stderr, "Error: %s - Input Image file not specified, "
			"it is needed for ti image generation\n",
			params->cmdname);
		return -1;
	}
	return 0;
}

/*
 * tiimage parameters
 */
static struct image_type_params tiimage_params = {
	.name		= "Texas Instruments Boot Image support",
	.header_size	= sizeof(struct ti_header),
	.hdr			= (void *)&tiimage_header,
	.check_image_type = tiimage_check_image_types,
	.verify_header	= tiimage_verify_header,
	.print_header	= ti81xximage_print_header,
#if defined(CONFIG_TI816X)
	.set_header		= ti816ximage_set_header,
#elif defined(CONFIG_TI814X) && defined(CONFIG_TI814X_MIN_CONFIG)
	.set_header		= ti814ximage_set_header,
#else
	.set_header		= ti_dummy_set_header,
#endif
	.check_params	= tiimage_check_params,
};

void init_ti_image_type(void)
{
	mkimage_register(&tiimage_params);
}


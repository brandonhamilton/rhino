/*
 * rhino.c  -- ALSA SoC support for RHINO
 *
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 * Based on sound/soc/omap/am3517evm.c by Anuj Aggarwal
 *
 * Copyright (C) 2009 Texas Instruments Incorporated
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <plat/mcbsp.h>

#include "omap-mcbsp.h"
#include "omap-pcm.h"

#include "../codecs/tlv320aic23.h"

#define CODEC_CLOCK 	12000000

static struct clk *tlv320aic23_mclk;

static int rhino_startup(struct snd_pcm_substream *substream)
{
	return clk_enable(tlv320aic23_mclk);
}

static void rhino_shutdown(struct snd_pcm_substream *substream)
{
	clk_disable(tlv320aic23_mclk);
}

static int rhino_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	printk("Setting up RHINO audio");

	/* Set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_DSP_B |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	/* Set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_DSP_B |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	/* Set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, 0,
			CODEC_CLOCK, SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_CLKR_SRC_CLKX, 0,
				SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set CPU system clock OMAP_MCBSP_CLKR_SRC_CLKX\n");
		return ret;
	}

	snd_soc_dai_set_sysclk(cpu_dai, OMAP_MCBSP_FSR_SRC_FSX, 0,
				SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set CPU system clock OMAP_MCBSP_FSR_SRC_FSX\n");
		return ret;
	}

	return 0;
}

static struct snd_soc_ops rhino_ops = {
	.startup   = rhino_startup,
	.hw_params = rhino_hw_params,
	.shutdown  = rhino_shutdown,
};

/* rhino machine dapm widgets */
static const struct snd_soc_dapm_widget tlv320aic23_dapm_widgets[] = {	
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_HP("Line Out", NULL),
	SND_SOC_DAPM_LINE("Line In", NULL),
};

static const struct snd_soc_dapm_route audio_map[] = {
	/* Line Out connected to LLOUT, RLOUT */
	//
	{"Headphone Jack", NULL, "LHPOUT"},
	{"Headphone Jack", NULL, "RHPOUT"},

	{"Line Out", NULL, "LOUT"},
	{"Line Out", NULL, "ROUT"},

	{"LLINEIN", NULL, "Line In"},
	{"RLINEIN", NULL, "Line In"},
};

static int rhino_aic23_init(struct snd_soc_codec *codec)
{
	/* Add rhino specific widgets */
	snd_soc_dapm_new_controls(codec, tlv320aic23_dapm_widgets,  ARRAY_SIZE(tlv320aic23_dapm_widgets));

	/* Set up davinci-evm specific audio path audio_map */
	snd_soc_dapm_add_routes(codec, audio_map, ARRAY_SIZE(audio_map));

	/* always connected */
	snd_soc_dapm_enable_pin(codec, "Headphone Jack");
	snd_soc_dapm_enable_pin(codec, "Line Out");
	snd_soc_dapm_enable_pin(codec, "Line In");


	snd_soc_dapm_sync(codec);

	return 0;
}

/* Digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link rhino_dai = {
	.name = "TLV320AIC23",
	.stream_name = "AIC23",
	.cpu_dai = &omap_mcbsp_dai[0],
	.codec_dai = &tlv320aic23_dai,
	.init = rhino_aic23_init,
	.ops = &rhino_ops,
};

/* Audio machine driver */
static struct snd_soc_card snd_soc_rhino = {
	.name = "rhino",
	.platform = &omap_soc_platform,
	.dai_link = &rhino_dai,
	.num_links = 1,
};

/* Audio subsystem */
static struct snd_soc_device rhino_snd_devdata = {
	.card = &snd_soc_rhino,
	.codec_dev = &soc_codec_dev_tlv320aic23,
};

static struct platform_device *rhino_snd_device;

static int __init rhino_soc_init(void)
{
	int ret;

	if (!machine_is_rhino()) {
		pr_err("Not RHINO!\n");
		return -ENODEV;
	}
	pr_info("RHINO Audio SoC init\n");

	rhino_snd_device = platform_device_alloc("soc-audio", -1);
	if (!rhino_snd_device) {
		printk(KERN_ERR "Platform device allocation failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(rhino_snd_device, &rhino_snd_devdata);
	rhino_snd_devdata.dev = &rhino_snd_device->dev;
	*(unsigned int *)rhino_dai.cpu_dai->private_data = 0; /* McBSP1 */

	ret = platform_device_add(rhino_snd_device);
	if (ret)
		goto err1;

	return 0;

err1:
	printk(KERN_ERR "Unable to add platform device\n");
	clk_put(tlv320aic23_mclk);
	platform_device_del(rhino_snd_device);
	platform_device_put(rhino_snd_device);

	return ret;
}

static void __exit rhino_soc_exit(void)
{
	platform_device_unregister(rhino_snd_device);
}

module_init(rhino_soc_init);
module_exit(rhino_soc_exit);

MODULE_AUTHOR("Brandon Hamilton <brandon.hamilton@gmail.com>");
MODULE_DESCRIPTION("ALSA SoC RHINO");
MODULE_LICENSE("GPL v2");

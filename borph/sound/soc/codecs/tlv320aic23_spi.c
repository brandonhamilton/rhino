/*
 * ALSA SoC TLV320AIC23 codec driver (SPI version)
 *
 * Author:      Brandon Hamilton <brandon.hamilton@gmail.com>
 *
 * Based on sound/soc/codecs/tlv320aic23.c by Arun KS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */


#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/spi/spi.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>
#include <sound/initval.h>

#include "tlv320aic23.h"

#define AIC23_VERSION "0.1"

MODULE_DESCRIPTION("ASoC TLV320AIC23 codec driver (SPI)");
MODULE_AUTHOR("Brandon Hamilton <brandon.hamilton@gmail.com>");
MODULE_LICENSE("GPL");

/* AIC26 driver private data */
struct aic23 {
	struct spi_device *spi;
	struct snd_soc_codec codec;
	int mclk;
	int requested_adc;
	int requested_dac;
};

/*
 * AIC23 register cache
 */
static const u16 tlv320aic23_spi_reg[] = {
	0x0097, 0x0097, 0x00F9, 0x00F9,	/* 0 */
	0x001A, 0x0004, 0x0007, 0x0001,	/* 4 */
	0x0020, 0x0000, 0x0000, 0x0000,	/* 8 */
	0x0000, 0x0000, 0x0000, 0x0000,	/* 12 */
};

/*
 * read tlv320aic23 register cache
 */
static inline unsigned int tlv320aic23_spi_read_reg_cache(struct snd_soc_codec
						      *codec, unsigned int reg)
{
	u16 *cache = codec->reg_cache;
	if (reg >= ARRAY_SIZE(tlv320aic23_spi_reg))
		return -1;
	return cache[reg];
}

/*
 * write tlv320aic23 register cache
 */
static inline void tlv320aic23_spi_write_reg_cache(struct snd_soc_codec *codec,
					       u8 reg, u16 value)
{
	u16 *cache = codec->reg_cache;
	if (reg >= ARRAY_SIZE(tlv320aic23_spi_reg))
		return;
	cache[reg] = value;
}

/*
 * write to the tlv320aic23 register space
 */
static int tlv320aic23_spi_write(struct snd_soc_codec *codec, unsigned int reg,
			     unsigned int value)
{
	int rc;
	struct aic23 *aic23 = codec->private_data;
	u8 data[2];

	if (!aic23) {
		printk(KERN_WARNING "%s Invalid device 0x%x\n", __func__, (int) aic23);
		return -1;
	}
	/* TLV320AIC23 has 7 bit address and 9 bits of data
	 * so we need to switch one data bit into reg and rest
	 * of data into val
	 */

	if (reg > 9 && reg != 15) {
		printk(KERN_WARNING "%s Invalid register R%u\n", __func__, reg);
		return -1;
	}

	data[0] = (reg << 1) | (value >> 8 & 0x01);
	data[1] = value & 0xff;
	rc = spi_write(aic23->spi, data, 2);
	if (rc) {
		dev_err(&aic23->spi->dev, "AIC23 reg read error\n");
		return -EIO;
	}

	tlv320aic23_spi_write_reg_cache(codec, reg, value);

	return 0;
}

static const char *rec_src_text[] = { "Line", "Mic" };
static const char *deemph_text[] = {"None", "32Khz", "44.1Khz", "48Khz"};

static const struct soc_enum rec_src_enum =
	SOC_ENUM_SINGLE(TLV320AIC23_ANLG, 2, 2, rec_src_text);

static const struct snd_kcontrol_new tlv320aic23_spi_rec_src_mux_controls =
SOC_DAPM_ENUM("Input Select", rec_src_enum);

static const struct soc_enum tlv320aic23_spi_rec_src =
	SOC_ENUM_SINGLE(TLV320AIC23_ANLG, 2, 2, rec_src_text);
static const struct soc_enum tlv320aic23_spi_deemph =
	SOC_ENUM_SINGLE(TLV320AIC23_DIGT, 1, 4, deemph_text);

static const DECLARE_TLV_DB_SCALE(out_gain_tlv, -12100, 100, 0);
static const DECLARE_TLV_DB_SCALE(input_gain_tlv, -1725, 75, 0);
static const DECLARE_TLV_DB_SCALE(sidetone_vol_tlv, -1800, 300, 0);

static int snd_soc_tlv320aic23_spi_put_volsw(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	u16 val, reg;

	val = (ucontrol->value.integer.value[0] & 0x07);

	/* linear conversion to userspace
	* 000	=	-6db
	* 001	=	-9db
	* 010	=	-12db
	* 011	=	-18db (Min)
	* 100	=	0db (Max)
	*/
	val = (val >= 4) ? 4  : (3 - val);

	reg = tlv320aic23_spi_read_reg_cache(codec, TLV320AIC23_ANLG) & (~0x1C0);
	tlv320aic23_spi_write(codec, TLV320AIC23_ANLG, reg | (val << 6));

	return 0;
}

static int snd_soc_tlv320aic23_spi_get_volsw(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	u16 val;

	val = tlv320aic23_spi_read_reg_cache(codec, TLV320AIC23_ANLG) & (0x1C0);
	val = val >> 6;
	val = (val >= 4) ? 4  : (3 -  val);
	ucontrol->value.integer.value[0] = val;
	return 0;

}

#define SOC_TLV320AIC23_SINGLE_TLV(xname, reg, shift, max, invert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_volsw, .get = snd_soc_tlv320aic23_spi_get_volsw,\
	.put = snd_soc_tlv320aic23_spi_put_volsw, \
	.private_value =  SOC_SINGLE_VALUE(reg, shift, max, invert) }

static const struct snd_kcontrol_new tlv320aic23_spi_snd_controls[] = {
	SOC_DOUBLE_R_TLV("Digital Playback Volume", TLV320AIC23_LCHNVOL,
			 TLV320AIC23_RCHNVOL, 0, 127, 0, out_gain_tlv),
	SOC_SINGLE("Digital Playback Switch", TLV320AIC23_DIGT, 3, 1, 1),
	SOC_DOUBLE_R("Line Input Switch", TLV320AIC23_LINVOL,
		     TLV320AIC23_RINVOL, 7, 1, 0),
	SOC_DOUBLE_R_TLV("Line Input Volume", TLV320AIC23_LINVOL,
			 TLV320AIC23_RINVOL, 0, 31, 0, input_gain_tlv),
	SOC_SINGLE("Mic Input Switch", TLV320AIC23_ANLG, 1, 1, 1),
	SOC_SINGLE("Mic Booster Switch", TLV320AIC23_ANLG, 0, 1, 0),
	SOC_TLV320AIC23_SINGLE_TLV("Sidetone Volume", TLV320AIC23_ANLG,
				  6, 4, 0, sidetone_vol_tlv),
	SOC_ENUM("Playback De-emphasis", tlv320aic23_spi_deemph),
};

/* PGA Mixer controls for Line and Mic switch */
static const struct snd_kcontrol_new tlv320aic23_spi_output_mixer_controls[] = {
	SOC_DAPM_SINGLE("Line Bypass Switch", TLV320AIC23_ANLG, 3, 1, 0),
	SOC_DAPM_SINGLE("Mic Sidetone Switch", TLV320AIC23_ANLG, 5, 1, 0),
	SOC_DAPM_SINGLE("Playback Switch", TLV320AIC23_ANLG, 4, 1, 0),
};

static const struct snd_soc_dapm_widget tlv320aic23_spi_dapm_widgets[] = {
	SND_SOC_DAPM_DAC("DAC", "Playback", TLV320AIC23_PWR, 3, 1),
	SND_SOC_DAPM_ADC("ADC", "Capture", TLV320AIC23_PWR, 2, 1),
	SND_SOC_DAPM_MUX("Capture Source", SND_SOC_NOPM, 0, 0, &tlv320aic23_spi_rec_src_mux_controls),
	SND_SOC_DAPM_MIXER("Output Mixer", TLV320AIC23_PWR, 4, 1,
			   &tlv320aic23_spi_output_mixer_controls[0],
			   ARRAY_SIZE(tlv320aic23_spi_output_mixer_controls)),
	SND_SOC_DAPM_PGA("Line Input", TLV320AIC23_PWR, 0, 1, NULL, 0),
	SND_SOC_DAPM_PGA("Mic Input", TLV320AIC23_PWR, 1, 1, NULL, 0),

	SND_SOC_DAPM_OUTPUT("LHPOUT"),
	SND_SOC_DAPM_OUTPUT("RHPOUT"),
	SND_SOC_DAPM_OUTPUT("LOUT"),
	SND_SOC_DAPM_OUTPUT("ROUT"),

	SND_SOC_DAPM_INPUT("LLINEIN"),
	SND_SOC_DAPM_INPUT("RLINEIN"),

	SND_SOC_DAPM_INPUT("MICIN"),
};

static const struct snd_soc_dapm_route intercon[] = {
	/* Output Mixer */
	{"Output Mixer", "Line Bypass Switch", "Line Input"},
	{"Output Mixer", "Playback Switch", "DAC"},
	{"Output Mixer", "Mic Sidetone Switch", "Mic Input"},

	/* Outputs */
	{"RHPOUT", NULL, "Output Mixer"},
	{"LHPOUT", NULL, "Output Mixer"},
	{"LOUT", NULL, "Output Mixer"},
	{"ROUT", NULL, "Output Mixer"},

	/* Inputs */
	{"Line Input", "NULL", "LLINEIN"},
	{"Line Input", "NULL", "RLINEIN"},

	{"Mic Input", "NULL", "MICIN"},

	/* input mux */
	{"Capture Source", "Line", "Line Input"},
	{"Capture Source", "Mic", "Mic Input"},
	{"ADC", NULL, "Capture Source"},

};


/*
 * Common Crystals used
 * 11.2896 Mhz /128 = *88.2k  /192 = 58.8k
 * 12.0000 Mhz /125 = *96k    /136 = 88.235K
 * 12.2880 Mhz /128 = *96k    /192 = 64k
 * 16.9344 Mhz /128 = 132.3k /192 = *88.2k
 * 18.4320 Mhz /128 = 144k   /192 = *96k
 */

/*
 * Normal BOSR 0-256/2 = 128, 1-384/2 = 192
 * USB BOSR 0-250/2 = 125, 1-272/2 = 136
 */
static const int bosr_usb_divisor_table[] = {
	128, 125, 192, 136
};
#define LOWER_GROUP ((1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<6) | (1<<7))
#define UPPER_GROUP ((1<<8) | (1<<9) | (1<<10) | (1<<11)        | (1<<15))
static const unsigned short sr_valid_mask[] = {
	LOWER_GROUP|UPPER_GROUP,	/* Normal, bosr - 0*/
	LOWER_GROUP,			/* Usb, bosr - 0*/
	LOWER_GROUP|UPPER_GROUP,	/* Normal, bosr - 1*/
	UPPER_GROUP,			/* Usb, bosr - 1*/
};
/*
 * Every divisor is a factor of 11*12
 */
#define SR_MULT (11*12)
#define A(x) (SR_MULT/x)
static const unsigned char sr_adc_mult_table[] = {
	A(2), A(2), A(12), A(12),  0, 0, A(3), A(1),
	A(2), A(2), A(11), A(11),  0, 0, 0, A(1)
};
static const unsigned char sr_dac_mult_table[] = {
	A(2), A(12), A(2), A(12),  0, 0, A(3), A(1),
	A(2), A(11), A(2), A(11),  0, 0, 0, A(1)
};

static unsigned get_score(int adc, int adc_l, int adc_h, int need_adc,
		int dac, int dac_l, int dac_h, int need_dac)
{
	if ((adc >= adc_l) && (adc <= adc_h) &&
			(dac >= dac_l) && (dac <= dac_h)) {
		int diff_adc = need_adc - adc;
		int diff_dac = need_dac - dac;
		return abs(diff_adc) + abs(diff_dac);
	}
	return UINT_MAX;
}

static int find_rate(int mclk, u32 need_adc, u32 need_dac)
{
	int i, j;
	int best_i = -1;
	int best_j = -1;
	int best_div = 0;
	unsigned best_score = UINT_MAX;
	int adc_l, adc_h, dac_l, dac_h;

	need_adc *= SR_MULT;
	need_dac *= SR_MULT;
	/*
	 * rates given are +/- 1/32
	 */
	adc_l = need_adc - (need_adc >> 5);
	adc_h = need_adc + (need_adc >> 5);
	dac_l = need_dac - (need_dac >> 5);
	dac_h = need_dac + (need_dac >> 5);
	for (i = 0; i < ARRAY_SIZE(bosr_usb_divisor_table); i++) {
		int base = mclk / bosr_usb_divisor_table[i];
		int mask = sr_valid_mask[i];
		for (j = 0; j < ARRAY_SIZE(sr_adc_mult_table);
				j++, mask >>= 1) {
			int adc;
			int dac;
			int score;
			if ((mask & 1) == 0)
				continue;
			adc = base * sr_adc_mult_table[j];
			dac = base * sr_dac_mult_table[j];
			score = get_score(adc, adc_l, adc_h, need_adc,
					dac, dac_l, dac_h, need_dac);
			if (best_score > score) {
				best_score = score;
				best_i = i;
				best_j = j;
				best_div = 0;
			}
			score = get_score((adc >> 1), adc_l, adc_h, need_adc,
					(dac >> 1), dac_l, dac_h, need_dac);
			/* prefer to have a /2 */
			if ((score != UINT_MAX) && (best_score >= score)) {
				best_score = score;
				best_i = i;
				best_j = j;
				best_div = 1;
			}
		}
	}
	return (best_j << 2) | best_i | (best_div << TLV320AIC23_CLKIN_SHIFT);
}

#ifdef DEBUG
static void get_current_sample_rates(struct snd_soc_codec *codec, int mclk,
		u32 *sample_rate_adc, u32 *sample_rate_dac)
{
	int src = tlv320aic23_spi_read_reg_cache(codec, TLV320AIC23_SRATE);
	int sr = (src >> 2) & 0x0f;
	int val = (mclk / bosr_usb_divisor_table[src & 3]);
	int adc = (val * sr_adc_mult_table[sr]) / SR_MULT;
	int dac = (val * sr_dac_mult_table[sr]) / SR_MULT;
	if (src & TLV320AIC23_CLKIN_HALF) {
		adc >>= 1;
		dac >>= 1;
	}
	*sample_rate_adc = adc;
	*sample_rate_dac = dac;
}
#endif

static int set_sample_rate_control(struct snd_soc_codec *codec, int mclk,
		u32 sample_rate_adc, u32 sample_rate_dac)
{
	/* Search for the right sample rate */
	int data = find_rate(mclk, sample_rate_adc, sample_rate_dac);
	if (data < 0) {
		printk(KERN_ERR "%s:Invalid rate %u,%u requested\n",
				__func__, sample_rate_adc, sample_rate_dac);
		return -EINVAL;
	}
	tlv320aic23_spi_write(codec, TLV320AIC23_SRATE, data);
#ifdef DEBUG
	{
		u32 adc, dac;
		get_current_sample_rates(codec, mclk, &adc, &dac);
		printk(KERN_DEBUG "actual samplerate = %u,%u reg=%x\n",
			adc, dac, data);
	}
#endif
	return 0;
}

static int tlv320aic23_spi_add_widgets(struct snd_soc_codec *codec)
{
	snd_soc_dapm_new_controls(codec, tlv320aic23_spi_dapm_widgets, ARRAY_SIZE(tlv320aic23_spi_dapm_widgets));

	/* set up audio path interconnects */
	snd_soc_dapm_add_routes(codec, intercon, ARRAY_SIZE(intercon));

	return 0;
}

static int tlv320aic23_spi_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params,
				 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	u16 iface_reg;
	int ret;
	struct aic23 *aic23 = container_of(codec, struct aic23, codec);
	u32 sample_rate_adc = aic23->requested_adc;
	u32 sample_rate_dac = aic23->requested_dac;
	u32 sample_rate = params_rate(params);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		aic23->requested_dac = sample_rate_dac = sample_rate;
		if (!sample_rate_adc)
			sample_rate_adc = sample_rate;
	} else {
		aic23->requested_adc = sample_rate_adc = sample_rate;
		if (!sample_rate_dac)
			sample_rate_dac = sample_rate;
	}
	ret = set_sample_rate_control(codec, aic23->mclk, sample_rate_adc,
			sample_rate_dac);
	if (ret < 0)
		return ret;

	iface_reg = tlv320aic23_spi_read_reg_cache(codec, TLV320AIC23_DIGT_FMT) & ~(0x03 << 2);
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		iface_reg |= (0x01 << 2);
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		iface_reg |= (0x02 << 2);
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		iface_reg |= (0x03 << 2);
		break;
	}
	tlv320aic23_spi_write(codec, TLV320AIC23_DIGT_FMT, iface_reg);

	return 0;
}

static int tlv320aic23_spi_pcm_prepare(struct snd_pcm_substream *substream,
				   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;

	/* set active */
	tlv320aic23_spi_write(codec, TLV320AIC23_ACTIVE, 0x0001);

	return 0;
}

static void tlv320aic23_spi_shutdown(struct snd_pcm_substream *substream,
				 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct aic23 *aic23 = container_of(codec, struct aic23, codec);

	/* deactivate */
	if (!codec->active) {
		udelay(50);
		tlv320aic23_spi_write(codec, TLV320AIC23_ACTIVE, 0x0);
	}
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		aic23->requested_dac = 0;
	else
		aic23->requested_adc = 0;
}

static int tlv320aic23_spi_mute(struct snd_soc_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;
	u16 reg;

	reg = tlv320aic23_spi_read_reg_cache(codec, TLV320AIC23_DIGT);
	if (mute)
		reg |= TLV320AIC23_DACM_MUTE;

	else
		reg &= ~TLV320AIC23_DACM_MUTE;

	tlv320aic23_spi_write(codec, TLV320AIC23_DIGT, reg);

	return 0;
}

static int tlv320aic23_spi_set_dai_fmt(struct snd_soc_dai *codec_dai,
				   unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u16 iface_reg;

	iface_reg = tlv320aic23_spi_read_reg_cache(codec, TLV320AIC23_DIGT_FMT) & (~0x03);

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		iface_reg |= TLV320AIC23_MS_MASTER;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		return -EINVAL;

	}

	/* interface format */
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		iface_reg |= TLV320AIC23_FOR_I2S;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		iface_reg |= TLV320AIC23_LRP_ON;
	case SND_SOC_DAIFMT_DSP_B:
		iface_reg |= TLV320AIC23_FOR_DSP;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iface_reg |= TLV320AIC23_FOR_LJUST;
		break;
	default:
		return -EINVAL;

	}

	tlv320aic23_spi_write(codec, TLV320AIC23_DIGT_FMT, iface_reg);

	return 0;
}

static int tlv320aic23_spi_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				      int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct aic23 *aic23 = container_of(codec, struct aic23, codec);
	aic23->mclk = freq;
	return 0;
}

static int tlv320aic23_spi_set_bias_level(struct snd_soc_codec *codec,
				      enum snd_soc_bias_level level)
{
	u16 reg = tlv320aic23_spi_read_reg_cache(codec, TLV320AIC23_PWR) & 0xff7f;

	switch (level) {
	case SND_SOC_BIAS_ON:
		/* vref/mid, osc on, dac unmute */
		tlv320aic23_spi_write(codec, TLV320AIC23_PWR, reg);
		break;
	case SND_SOC_BIAS_PREPARE:
		break;
	case SND_SOC_BIAS_STANDBY:
		/* Activate the digital interface */
		tlv320aic23_spi_write(codec, TLV320AIC23_ACTIVE, 0x1);
		break;
	case SND_SOC_BIAS_OFF:
		/* Deactivate the digital interface */
		tlv320aic23_spi_write(codec, TLV320AIC23_ACTIVE, 0x0);
		break;
	}
	codec->bias_level = level;
	return 0;
}

#define AIC23_RATES	SNDRV_PCM_RATE_8000_96000
#define AIC23_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE | \
			 SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_dai_ops tlv320aic23_spi_dai_ops = {
	.prepare	= tlv320aic23_spi_pcm_prepare,
	.hw_params	= tlv320aic23_spi_hw_params,
	.shutdown	= tlv320aic23_spi_shutdown,
	.digital_mute	= tlv320aic23_spi_mute,
	.set_fmt	= tlv320aic23_spi_set_dai_fmt,
	.set_sysclk	= tlv320aic23_spi_set_dai_sysclk,
};

struct snd_soc_dai tlv320aic23_spi_dai = {
	.name = "tlv320aic23_spi",
	.playback = {
		     .stream_name = "Playback",
		     .channels_min = 2,
		     .channels_max = 2,
		     .rates = AIC23_RATES,
		     .formats = AIC23_FORMATS,},
	.capture = {
		    .stream_name = "Capture",
		    .channels_min = 2,
		    .channels_max = 2,
		    .rates = AIC23_RATES,
		    .formats = AIC23_FORMATS,},
	.ops = &tlv320aic23_spi_dai_ops,
};
EXPORT_SYMBOL_GPL(tlv320aic23_spi_dai);

static int tlv320aic23_spi_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	tlv320aic23_spi_set_bias_level(codec, SND_SOC_BIAS_OFF);

	return 0;
}

static int tlv320aic23_spi_resume(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;
	u16 reg;

	/* Sync reg_cache with the hardware */
	for (reg = 0; reg <= TLV320AIC23_ACTIVE; reg++) {
		u16 val = tlv320aic23_spi_read_reg_cache(codec, reg);
		tlv320aic23_spi_write(codec, reg, val);
	}

	tlv320aic23_spi_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	tlv320aic23_spi_set_bias_level(codec, codec->suspend_bias_level);

	return 0;
}

/* ---------------------------------------------------------------------
 * SoC CODEC portion of driver: probe and release routines
 */
static int tlv320aic23_spi_codec_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec;
	struct aic23 *aic23;
	int ret = 0, err;

	printk(KERN_INFO "AIC23 Audio Codec %s\n", AIC23_VERSION);

	aic23 = socdev->codec_data;
	if (aic23 == NULL) {
		dev_err(&pdev->dev, "aic23: missing codec pointer\n");
		return -ENODEV;
	}
	codec = &aic23->codec;
	socdev->card->codec = codec;

	/* register pcms */
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		printk(KERN_ERR "tlv320aic23: failed to create pcms\n");
		return -ENODEV;
	}

	err = snd_soc_add_controls(codec, tlv320aic23_spi_snd_controls, ARRAY_SIZE(tlv320aic23_spi_snd_controls));
	WARN_ON(err < 0);

	return ret;
}

static int tlv320aic23_spi_codec_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;
	struct aic23 *aic23 = container_of(codec, struct aic23, codec);

	if (codec->control_data)
		tlv320aic23_spi_set_bias_level(codec, SND_SOC_BIAS_OFF);

	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
	kfree(codec->reg_cache);
	kfree(aic23);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_tlv320aic23_spi = {
	.probe = tlv320aic23_spi_codec_probe,
	.remove = tlv320aic23_spi_codec_remove,
	.suspend = tlv320aic23_spi_suspend,
	.resume = tlv320aic23_spi_resume,
};
EXPORT_SYMBOL_GPL(soc_codec_dev_tlv320aic23_spi);

struct aic23* tlv320aic23_spi_aic23 = 0;

/* ---------------------------------------------------------------------
 * SPI portion of driver: probe and release routines
 */
static int tlv320aic23_spi_device_probe(struct spi_device *spi)
{
	struct aic23 *aic23;
	int ret, reg;

	dev_dbg(&spi->dev, "probing tlv320aic23 spi device\n");

	/* Allocate driver data */
	aic23 = kzalloc(sizeof *aic23, GFP_KERNEL);
	if (!aic23)
		return -ENOMEM;

	/* Initialize the driver data */
	aic23->spi = spi;
	dev_set_drvdata(&spi->dev, aic23);

	/* Setup what we can in the codec structure so that the register
	 * access functions will work as expected.  More will be filled
	 * out when it is probed by the SoC CODEC part of this driver */
	aic23->codec.private_data = aic23;
	aic23->codec.name = "tlv320aic23_spi";
	aic23->codec.owner = THIS_MODULE;
	aic23->codec.read = tlv320aic23_spi_read_reg_cache;
	aic23->codec.write = tlv320aic23_spi_write;
	aic23->codec.set_bias_level = tlv320aic23_spi_set_bias_level;
	aic23->codec.dai = &tlv320aic23_spi_dai;
	aic23->codec.num_dai = 1;
	mutex_init(&aic23->codec.mutex);
	INIT_LIST_HEAD(&aic23->codec.dapm_widgets);
	INIT_LIST_HEAD(&aic23->codec.dapm_paths);
	aic23->codec.reg_cache_size = ARRAY_SIZE(tlv320aic23_spi_reg);
	aic23->codec.reg_cache = kmemdup(tlv320aic23_spi_reg, sizeof(tlv320aic23_spi_reg), GFP_KERNEL);
	if (aic23->codec.reg_cache == NULL)
		return -ENOMEM;

	tlv320aic23_spi_dai.dev = &spi->dev;
	ret = snd_soc_register_dai(&tlv320aic23_spi_dai);
	if (ret != 0) {
		dev_err(&spi->dev, "Failed to register DAI: %d\n", ret);
		kfree(aic23);
		return ret;
	}

	tlv320aic23_spi_aic23 = aic23;

	/* Reset codec */
	tlv320aic23_spi_write(&aic23->codec, TLV320AIC23_RESET, 0);

	/* power on device */
	tlv320aic23_spi_set_bias_level(&aic23->codec, SND_SOC_BIAS_STANDBY);

	tlv320aic23_spi_write(&aic23->codec, TLV320AIC23_DIGT, TLV320AIC23_DEEMP_44K);

	/* Unmute input */
	reg = tlv320aic23_spi_read_reg_cache(&aic23->codec, TLV320AIC23_LINVOL);
	tlv320aic23_spi_write(&aic23->codec, TLV320AIC23_LINVOL,
			  (reg & (~TLV320AIC23_LIM_MUTED)) |
			  (TLV320AIC23_LRS_ENABLED));

	reg = tlv320aic23_spi_read_reg_cache(&aic23->codec, TLV320AIC23_RINVOL);
	tlv320aic23_spi_write(&aic23->codec, TLV320AIC23_RINVOL,
			  (reg & (~TLV320AIC23_LIM_MUTED)) |
			  TLV320AIC23_LRS_ENABLED);

	reg = tlv320aic23_spi_read_reg_cache(&aic23->codec, TLV320AIC23_ANLG);
	tlv320aic23_spi_write(&aic23->codec, TLV320AIC23_ANLG,
			 (reg) & (~TLV320AIC23_BYPASS_ON) &
			 (~TLV320AIC23_MICM_MUTED));

	/* Default output volume */
	tlv320aic23_spi_write(&aic23->codec, TLV320AIC23_LCHNVOL,
			  TLV320AIC23_DEFAULT_OUT_VOL &
			  TLV320AIC23_OUT_VOL_MASK);
	tlv320aic23_spi_write(&aic23->codec, TLV320AIC23_RCHNVOL,
			  TLV320AIC23_DEFAULT_OUT_VOL &
			  TLV320AIC23_OUT_VOL_MASK);

	tlv320aic23_spi_write(&aic23->codec, TLV320AIC23_ACTIVE, 0x1);

	tlv320aic23_spi_add_widgets(&aic23->codec);

	dev_dbg(&spi->dev, "SPI device initialized\n");
	return ret;

}

static int tlv320aic23_spi_device_remove(struct spi_device *spi)
{
	struct aic23 *aic23 = dev_get_drvdata(&spi->dev);
	snd_soc_unregister_dai(&tlv320aic23_spi_dai);
	kfree(aic23);
	return 0;
}

static struct spi_driver tlv320aic23_spi = {
	.driver = {
		.name = "tlv320aic23_spi",
		.owner = THIS_MODULE,
	},
	.probe = tlv320aic23_spi_device_probe,
	.remove = tlv320aic23_spi_device_remove,
};

static int __init tlv320aic23_spi_init(void)
{
	return spi_register_driver(&tlv320aic23_spi);
}
module_init(tlv320aic23_spi_init);

static void __exit tlv320aic23_spi_exit(void)
{
	spi_unregister_driver(&tlv320aic23_spi);
}
module_exit(tlv320aic23_spi_exit);

#include "ud_image.h"
/* TMP PRINT FUNCTIONS */

char	*tmp_print_color_type_str(ud_png_color_type color_type)
{
	if (color_type == UD_PNG_GREYSCALE) return "Greyscale";
	if (color_type == UD_PNG_RGB) return "RGB";
	if (color_type == UD_PNG_PALETTE) return "Palette (a PLTE Chunk must appear)";
	if (color_type == UD_PNG_GREYSCALE_A) return "Greyscale with alpha (transparency)";
	if (color_type == UD_PNG_RGB_A) return "RGB with alpha (transparency)";
	return (NULL);
}

/* END OF TMP PRINT FUNCTIONS */

static int			ud_img_png_parse_int_val(unsigned char *img_str)
{
	int		val = 0;
	size_t	len = sizeof(int);
	for (ud_ut_count i = 0; i < len; ++i, ++img_str)
		val = ((val << 8) | (int)*img_str);
	return val;
}

static short		ud_img_png_parse_short_val(unsigned char *img_str)
{
	int		val = 0;
	size_t	len = sizeof(short);
	for (ud_ut_count i = 0; i < len; ++i, ++img_str)
		val = ((val << 8) | (int)*img_str);
	return val;
}

static ud_png_color_type	ud_img_png_parse_color_type(unsigned char color_type_byte)
{
	if (color_type_byte == 0) return UD_PNG_GREYSCALE;
	if (color_type_byte == 2) return UD_PNG_RGB;
	if (color_type_byte == 3) return UD_PNG_PALETTE;
	if (color_type_byte == 4) return UD_PNG_GREYSCALE_A;
	if (color_type_byte == 6) return UD_PNG_RGB_A;
	printf("Invalid color type value %hhu, legal values are (0, 2, 3, 4 or 6), file must be corrupted\n", color_type_byte);
	exit(-1);
}

static unsigned char	*ud_img_png_parse_header(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	(void)chunk_len;
	png->img_width = ud_img_png_parse_int_val(img_str);
	img_str += 4;
	png->img_height = ud_img_png_parse_int_val(img_str);
	img_str += 4;
	png->bit_depth = *img_str++;
	png->color_type = ud_img_png_parse_color_type(*img_str++);
	png->compression_method = *img_str++; // must be 0
	png->filter_method = *img_str++; // must be 0
	png->interlace_method = *img_str++;
	printf("IHDR (Header) Chunk:\n\tImage size : %dx%d pixels\n\tBit depth : %hhu\n\tColor type : %s\n\tCompression method : %hhu\n\tFilter method : %hhu\n\tInterlace method : %s\n", png->img_width, png->img_height, png->bit_depth, tmp_print_color_type_str(png->color_type), png->compression_method, png->filter_method, png->interlace_method ? "Adam7 Interlace" : "No Interlace");
	return (img_str + 4);
}

static unsigned char	*ud_img_png_parse_palette(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	printf("PLTE (Palette) Chunk:\n");
	if ((chunk_len % 3))
	{
		printf("Incomplete palette entries, file must be corrupted\n");
		exit(-1);
	}
	size_t	palette_entries = chunk_len / 3;
	if (palette_entries > (1 << png->bit_depth))
	{
		printf("The number of palette (%zu) entries exceed the range that can be represented in the image bit depth (2^%hhu = %d)\n", palette_entries, png->bit_depth, 1 << png->bit_depth);
		exit(-1);
	}
	png->palette_entries = palette_entries;
	ud_ut_prot_malloc(png->palette = ud_ut_malloc(palette_entries * sizeof(ud_img_pix_rgb)));
	ud_img_pix_rgba	*palette = png->palette;
	for (ud_ut_count i = 0; i < palette_entries; ++i, ++palette)
	{
		palette->red = *img_str++;
		palette->green = *img_str++;
		palette->blue = *img_str++;
		palette->alpha = 255; // fully opaque by default
		printf("\t[%zu]\t=> RED %hhu GREEN %hhu BLUE %hhu\n", i, palette->red, palette->green, palette->blue);
	}
	return (img_str + 4);
}

static unsigned char	*ud_img_png_parse_data(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	if (img_str && png)
		;
	printf("IDATA\n");
	return (img_str + chunk_len + 4);
}

static unsigned char	*ud_img_png_parse_transparency(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	printf("tRNS (Transparency) Chunk:\n");
	if (png->color_type == UD_PNG_PALETTE)
	{
		if (chunk_len > png->palette_entries)
		{
			printf("Transparency entries number must not exceed palette entries number\n");
			exit(-1);
		}
		ud_img_pix_rgba	*palette = png->palette;
		for (ud_ut_count i = 0; i < chunk_len; ++i, ++img_str, ++palette)
		{
			palette->alpha = *img_str;
			printf("\t[%zu]\t=> RED %hhu GREEN %hhu BLUE %hhu ALPHA %hhu\n", i, palette->red, palette->green, palette->blue, palette->alpha);
		}
	}
	if (png->color_type == UD_PNG_GREYSCALE)
	{
		ud_ut_prot_malloc(png->transp_tab = ud_ut_malloc(sizeof(ud_img_png_spix)));
		png->transp_tab->red = ud_img_png_parse_short_val(img_str);
		png->transp_tab->green = png->transp_tab->red;
		png->transp_tab->blue = png->transp_tab->red;
		img_str += 2;
		printf("\tPixels GREYSCALE %hu are fully transparent\n", png->transp_tab->red);
	}
	if (png->color_type == UD_PNG_RGB)
	{
		ud_ut_prot_malloc(png->transp_tab = ud_ut_malloc(sizeof(ud_img_png_spix)));
		png->transp_tab->red = ud_img_png_parse_short_val(img_str);
		png->transp_tab->green = ud_img_png_parse_short_val(img_str + 2);
		png->transp_tab->blue = ud_img_png_parse_short_val(img_str + 4);
		printf("\tPixels RED %hu GREEN %hu BLUE %hu are fully transparent\n", png->transp_tab->red, png->transp_tab->green, png->transp_tab->blue);
		img_str += 6;
	}
	return (img_str + 4);
}

static unsigned char	*ud_img_png_parse_chromaticities(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	(void)chunk_len;
	printf("cHRM (Chromaticities) Chunk:\n");
	ud_ut_prot_malloc(png->chroma = ud_ut_malloc(sizeof(ud_img_png_chroma)));
	png->chroma->white_p_x = (float)ud_img_png_parse_int_val(img_str) / 100000.0;
	png->chroma->white_p_y = (float)ud_img_png_parse_int_val(img_str + 4) / 100000.0;
	png->chroma->red_x = (float)ud_img_png_parse_int_val(img_str + 8) / 100000.0;
	png->chroma->red_y = (float)ud_img_png_parse_int_val(img_str + 12) / 100000.0;
	png->chroma->green_x = (float)ud_img_png_parse_int_val(img_str + 16) / 100000.0;
	png->chroma->green_y = (float)ud_img_png_parse_int_val(img_str + 20) / 100000.0;
	png->chroma->blue_x = (float)ud_img_png_parse_int_val(img_str + 24) / 100000.0;
	png->chroma->blue_y = (float)ud_img_png_parse_int_val(img_str + 28) / 100000.0;
	printf("\t\tx\t\ty\nWhite point\t%f\t%f\nRed\t\t%f\t%f\nGreen\t\t%f\t%f\nBlue\t\t%f\t%f\n", png->chroma->white_p_x, png->chroma->white_p_x, png->chroma->red_x, png->chroma->red_y, png->chroma->green_x, png->chroma->green_y, png->chroma->blue_x, png->chroma->blue_y);
	return (img_str + 36);
}

static unsigned char	*ud_img_png_parse_gamma(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	(void)chunk_len;
	printf("gAMA (Gamma) Chunk:\n");
	png->gamma = (float)ud_img_png_parse_int_val(img_str) / 100000.0;
	printf("\tvalue : %f\n", png->gamma);
	return (img_str + 8);
}

static unsigned char	*ud_img_png_parse_significant_bit(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	(void)png;
	printf("sBIT (Significant bits) Chunk: (Not used by decoder a priori)\n");
	return (img_str + chunk_len + 4);
}

static unsigned char	*ud_img_png_parse_text(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	(void)png;
	unsigned char *img_str_cpy = img_str;
	printf("tEXt (Textual info) Chunk:\n");
	printf("\tKeyword: %s\n", img_str);
	while (*img_str)
		++img_str;
	++img_str;
	printf("\ttext: %.*s\n", (int)(chunk_len - (img_str - img_str_cpy)), img_str);
	return (img_str_cpy + chunk_len + 4);
}

static unsigned char	*ud_img_png_parse_text_compressed(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	(void)png;
	printf("zTXt (Compressed text info) Chunk: not usefull\n");
	return (img_str + chunk_len + 4);
}

static unsigned char	*ud_img_png_parse_background(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	(void)chunk_len;
	printf("bKGD (Background) Chunk:\n");
	if (png->color_type == UD_PNG_PALETTE)
	{
		if (*img_str < png->palette_entries)
		{
			png->background.red = png->palette[*img_str].red;
			png->background.green = png->palette[*img_str].green;
			png->background.blue = png->palette[*img_str++].blue;
		}
	}
	else if (png->color_type == UD_PNG_GREYSCALE || png->color_type == UD_PNG_GREYSCALE_A)
	{
		png->background.red = ud_img_png_parse_short_val(img_str);
		png->background.green = png->background.red;
		png->background.blue = png->background.red;
		img_str += 2;
	}
	else if (png->color_type == UD_PNG_RGB || png->color_type == UD_PNG_RGB_A)
	{
		png->background.red = ud_img_png_parse_short_val(img_str);
		img_str += 2;
		png->background.green = ud_img_png_parse_short_val(img_str);
		img_str += 2;
		png->background.blue = ud_img_png_parse_short_val(img_str);
		img_str += 2;
	}
	printf("\t RED %hu GREEN %hu BLUE %hu\n", png->background.red, png->background.green, png->background.blue);
	return (img_str + 4);
}

static unsigned char	*ud_img_png_parse_histogram(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	printf("hIST (Histogram) Chunk:\n");
	if (!png->palette)
	{
		printf("Histogram chunk must be preceded by PLTE chunk\n");
		exit(-1);
	}
	if (chunk_len * 2 != png->palette_entries)
	{	
		printf("Histogram size is not corresponding to palette entries number, file must be corrupted\n");
		exit(-1);
	}
	for (ud_ut_count i = 0; i < png->palette_entries; ++i)
	{
		unsigned short	freq = ud_img_png_parse_short_val(img_str);
		img_str += 2;
		printf("\t[%zu]\t => freq : %hu\n", i, freq);
	}
	return (img_str + 4);
}

static unsigned char	*ud_img_png_parse_physical(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	(void)chunk_len;
	printf("pHYs (Physical dimension) Chunk:\n");
	png->pix_by_unit_x = ud_img_png_parse_int_val(img_str);
	png->pix_by_unit_y = ud_img_png_parse_int_val(img_str + 4);
	png->unit = *(img_str + 8);
	printf("\tPixels per unit, X axis: %u\n\tPixels per unit, Y axis: %u\n\tunit specifier : %s\n", png->pix_by_unit_x, png->pix_by_unit_y, png->unit == 1 ? "meter" : "unknown");
	return (img_str + 13);
}

static unsigned char	*ud_img_png_parse_modif_time(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	(void)png;
	(void)chunk_len;
	printf("tIME (Time of last modification) Chunk:\n");
	unsigned short	year = ud_img_png_parse_short_val(img_str);
	img_str += 2;
	unsigned char	month = *img_str++;
	unsigned char	day = *img_str++;
	unsigned char	hour = *img_str++;
	unsigned char	minute = *img_str++;
	unsigned char	second = *img_str++;
	printf("\t%02hhu/%02hhu/%hu %02hhuh%02hhum%02hhus\n", day, month, year, hour, minute, second);
	return (img_str + 4);
}

static unsigned char	*ud_img_png_skip_chunk(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	(void)png;
	return (img_str + chunk_len + 4);
}

static unsigned char	*(*ud_img_png_get_parse_function(unsigned char *img_str))(unsigned int, unsigned char *, ud_png *)
{
	char	chunk_name[5];
	for (ud_ut_count i = 0; i < 4; ++i, ++img_str)
		chunk_name[i] = *img_str;
	chunk_name[4] = '\0';
	// CRITICAL
	if (!ud_str_cmp(chunk_name, "IHDR")) return &ud_img_png_parse_header;
	if (!ud_str_cmp(chunk_name, "PLTE")) return &ud_img_png_parse_palette;
	if (!ud_str_cmp(chunk_name, "IDAT")) return &ud_img_png_parse_data;
	if (!ud_str_cmp(chunk_name, "IEND")) return NULL;

	// ANCILLARY
	if (!ud_str_cmp(chunk_name, "tRNS")) return &ud_img_png_parse_transparency;
	if (!ud_str_cmp(chunk_name, "cHRM")) return &ud_img_png_parse_chromaticities;
	if (!ud_str_cmp(chunk_name, "gAMA")) return &ud_img_png_parse_gamma;
	if (!ud_str_cmp(chunk_name, "sBIT")) return &ud_img_png_parse_significant_bit;
	if (!ud_str_cmp(chunk_name, "tEXt")) return &ud_img_png_parse_text;
	if (!ud_str_cmp(chunk_name, "zTXt")) return &ud_img_png_parse_text_compressed;
	if (!ud_str_cmp(chunk_name, "bKGD")) return &ud_img_png_parse_background;
	if (!ud_str_cmp(chunk_name, "hIST")) return &ud_img_png_parse_histogram;
	if (!ud_str_cmp(chunk_name, "pHYs")) return &ud_img_png_parse_physical;
	if (!ud_str_cmp(chunk_name, "tIME")) return &ud_img_png_parse_modif_time;
	//if (!ud_str_cmp(chunk_name, "iCCP")) return &ud_img_png_parse_;
	//if (!ud_str_cmp(chunk_name, "sRGB")) return &ud_img_png_parse_;
	//if (!ud_str_cmp(chunk_name, "iTXt")) return ;
	//if (!ud_str_cmp(chunk_name, "sPLT")) return UD_CHUNK_sPLT;
	printf("useless chunk ? : %s\n", chunk_name);
	return &ud_img_png_skip_chunk;
}

static ud_png	ud_img_png_init(void)
{
	ud_png	png;

	png.background.red = 255;
	png.background.green = 255;
	png.background.blue = 255;
	png.palette = NULL;
	png.chroma = NULL;
	png.gamma = 1;
	png.transp_tab = NULL;
	return png;
}

ud_img	*ud_img_png_decryption(unsigned char *img_str)
{
	//char	chunk_name[5] = {'\0', '\0', '\0', '\0', '\0'};
	unsigned char	*(*chunk_f)(unsigned int, unsigned char *, ud_png *);
	unsigned int	chunk_len;
	ud_png			png = ud_img_png_init();
	img_str += 8; // skip png signature
	for (int check_data_chunk = 0; check_data_chunk != 1;)
	{
		chunk_len = ud_img_png_parse_int_val(img_str);
		img_str += 4;
		chunk_f = ud_img_png_get_parse_function(img_str);
		img_str += 4;
		if (chunk_f == ud_img_png_parse_data) check_data_chunk = 1;
		else img_str = (*chunk_f)(chunk_len, img_str, &png);
	}
	while (chunk_f == ud_img_png_parse_data)
	{
		img_str = (*chunk_f)(chunk_len, img_str, &png);
		chunk_len = ud_img_png_parse_int_val(img_str);
		img_str += 4;
		chunk_f = ud_img_png_get_parse_function(img_str);
		img_str += 4;
	}
	while (chunk_f) // IEND chunk return NULL pointer on function
	{
		img_str = (*chunk_f)(chunk_len, img_str, &png);
		chunk_len = ud_img_png_parse_int_val(img_str);
		img_str += 4;
		chunk_f = ud_img_png_get_parse_function(img_str);
		img_str += 4;
	}
	return (NULL);
}

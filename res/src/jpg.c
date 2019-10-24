# include "ud_image.h"

ud_density_unit	ud_img_jpg_get_density_unit(unsigned char density)
{
	if (density == 1)
		return (UD_DU_JPG_PBINCH);
	if (density == 2)
		return (UD_DU_JPG_PBCM);
	return (UD_DU_JPG_UNKNOWN);
}

char			**ud_img_create_new_quant_mat(ud_arr **p_quantization_mat, unsigned char nbr_bytes)
{
	char	***t_quantization_mat_val;
	char	**t_quantization_mat_line_val;
	char	*t_quantization_mat_char_val;
	unsigned char	size = nbr_bytes == 64 ? 8 : 16;

	if (!*p_quantization_mat)
		*p_quantization_mat = ud_arr_set(char **, NULL, NULL, NULL, NULL);
	t_quantization_mat_val = (char ***)(*p_quantization_mat)->val;
	while (*t_quantization_mat_val)
		++t_quantization_mat_val;
	ud_ut_prot_malloc(*t_quantization_mat_val = ud_ut_malloc(sizeof(char **) * (size + 1)));
	t_quantization_mat_line_val = *t_quantization_mat_val;
	for (ud_ut_count i = 0; i < size; ++i, ++t_quantization_mat_line_val)
	{
		ud_ut_prot_malloc(*t_quantization_mat_line_val = ud_ut_malloc(sizeof(char *) * (size + 1)));
		t_quantization_mat_char_val = *t_quantization_mat_line_val;
		for (ud_ut_count j = 0; j < size + 1; ++j, ++t_quantization_mat_char_val) *t_quantization_mat_char_val = '\0';
	}
	*t_quantization_mat_line_val = NULL;
	return (*t_quantization_mat_val);
}

unsigned char	*ud_img_jpg_parse_quantization_mat(unsigned char *img, ud_jpg *jpg)
{
	unsigned short	seg_len = ((((unsigned short)*img) << 8) | (unsigned short)*(img + 1));
	printf("Quantization table(s) definition :seg len %d\n", seg_len);
	char	**new_mat;

	img += 2;
	seg_len -= 2;
	while (seg_len--)
	{
		unsigned char	nbr_bytes = ((*img >> 4) & 1) ? 128 : 64;
		//unsigned char	size_b = ((*img >> 4) & 1) ? 16 : 8;
		printf("nbr bytes : %hhu\n", nbr_bytes);
		new_mat = ud_img_create_new_quant_mat(&(jpg->quantization_mat), nbr_bytes);
		++img;
		for (ud_ut_count i = 0; i < nbr_bytes; --seg_len, ++img)
		{
			printf("%02.2hhx ", *img);
			if (!(++i % 8))
				printf("\n");
		}
		/*img -= 64;
		int y_w = -1;
		int	x_w = 1;
		int	x = 0;
		int	y = 0;
		for (ud_ut_count i = 0; i < nbr_bytes; ++i, ++img)
		{
			if (y == -1)
			{
				y = 0;
				x_w = -1;
				y_w = 1;
			}
			else if (x == -1)
			{
				x = 0;
				x_w = 1;
				y_w = -1;
			}
			if (x == 8)
			{
				x = 7;
				x_w = -1;
				y_w = 1;
			}
			else if (y == 8)
			{
				y = 7;
				x_w = 1;
				y_w = -1;
			}
			printf("new_mat[%2.2x][%2.2x] = %2.2hhx\n", y, x, *img);
			new_mat[x][y] = *img;
			y += y_w;
			x += x_w;
		}
		for (ud_ut_count i = 0; i < nbr_bytes; ++i)
		{
			printf("%02.2hhx ", new_mat[nbr_bytes / size_b][nbr_bytes % size_b]);
			if ((++i % size_b))
				printf("\n");
		}
		if (img && jpg)
			;
*/
	}
	return (img);
}

unsigned char	*ud_img_jpg_app_ctr(unsigned char *img, ud_jpg *jpg, unsigned char app_ref)
{
	unsigned short	seg_len = ((((unsigned short)*img) << 8) | (unsigned short)*(img + 1));
	printf("app %hhd marker %s: seg len %d\n", app_ref, (char *)(img + 2), seg_len);
	
	if (!app_ref)
	{
		jpg->jfif_seg_len = ((((unsigned short)(*img)) << 8) | (unsigned short)*(img + 1));
		jpg->density_unit = (ud_img_jpg_get_density_unit(*(img + 2)));
		jpg->x_pixel_by_unit = ((((unsigned short)(*(img + 10))) << 8) | (unsigned short)*(img + 11));
		jpg->y_pixel_by_unit = ((((unsigned short)(*(img + 12))) << 8) | (unsigned short)*(img + 13));
		jpg->thumbnail_width = *(img + 14);
		jpg->thumbnail_height = *(img + 15);
		printf("x pixel by unit:%d\ny pixel by unit%d\n", jpg->x_pixel_by_unit, jpg->y_pixel_by_unit);
		if (!jpg->thumbnail || !jpg->thumbnail_height)
			jpg->thumbnail = NULL;
		else
			;//parseminiature
		//return (img + 16);
	}
	//else if (app_ref == 1)
	//	return (img);
	return (img + seg_len);
}

unsigned char	*ud_img_jpg_read_segment(unsigned char *img, ud_jpg *jpg)
{
	if (*img == UD_IMG_JPG_SOI)
		return (img + 1);
	else if (*img == UD_IMG_JPG_SOF_BD)
		;
	else if (*img == UD_IMG_JPG_SOF_PD)
		;
	else if (*img == UD_IMG_JPG_DHT)
		;
	else if (*img == UD_IMG_JPG_DQT)
		return (ud_img_jpg_parse_quantization_mat(img + 1, jpg));
	else if (*img == UD_IMG_JPG_DRI)
		;
	else if (*img == UD_IMG_JPG_SOS)
		;
	else if (*img >= UD_IMG_JPG_RST_MIN && *img <= UD_IMG_JPG_RST_MAX)
		;
	else if (*img >= UD_IMG_JPG_APP_MIN && *img <= UD_IMG_JPG_APP_MAX)
		return (ud_img_jpg_app_ctr(img + 1, jpg, (*img & 0x0f)));
	else if (*img == UD_IMG_JPG_COM)
		;
	else if (*img == UD_IMG_JPG_EOI)
		;
	printf("%hhx\n", *img);
	return (img + 1);


}

int		**ud_img_decryption_jpg_to_rgb(unsigned char *img)
{
	ud_jpg	jpg;
	int		**rgb = NULL;

	jpg.quantization_mat = NULL;
	while (ud_img_jpg_check_marker_start(*img))
		img = ud_img_jpg_read_segment(++img, &jpg);
	return (rgb);
}

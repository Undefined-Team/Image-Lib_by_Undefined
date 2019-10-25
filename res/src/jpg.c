# include "ud_image.h"

ud_density_unit	ud_img_jpg_get_density_unit(unsigned char density)
{
	if (density == 1)
		return (UD_DU_JPG_PBINCH);
	if (density == 2)
		return (UD_DU_JPG_PBCM);
	return (UD_DU_JPG_UNKNOWN);
}

unsigned char	ud_img_get_quant_bit_id(unsigned char id_b)
{
	unsigned char	id = 0;
	if (!id_b)
		return (0); // not sure ..
	while (!((id_b >> id)& 1))
		++id;
	return (id);
}

unsigned char	*ud_img_jpg_scan_file(unsigned char *img, ud_jpg *jpg)
{
	unsigned short	seg_len = ((((unsigned short)*img) << 8) | (unsigned short)*(img + 1));
	printf("Start of Scan marker : seg len %d\n", seg_len);
	unsigned char	comp_nbr = *(img + 2);

	img += 3;
	for (ud_ut_count i = 0; i < comp_nbr; ++i)
	{
		unsigned char	comp_id = *img++;
		unsigned char	dc_hf_id = (*img >> 4);
		unsigned char	ac_hf_id = (*img++ & 0x0f);
		printf("comp id %hhu dc_huff %hhu ac_huff %hhu\n", comp_id, dc_hf_id, ac_hf_id);
	}
	unsigned char	spec_start = *img++;
	unsigned char	spec_end = *img++;
	unsigned char	succ_approx = *img++;

	printf("spectral start %hhu spectral end %hhu successive approximation %hhu\n", spec_start, spec_end, succ_approx);
	//for (ud_ut_count i = 0; i < seg_len; ++i)
	//	printf("%02.2hhx\n", *img++);
	if (img && jpg)
		;
	return (img);
	
}

size_t			ud_img_get_huff_table_size(unsigned char *img, size_t *val_nbr)
{
	size_t	size = 0;
	size_t	nb_iter = 16;

	while (!*img)
	{
		--img;
		--nb_iter;
	}
	size_t	mult = 0;
	for (ud_ut_count i = 0; i < nb_iter; ++i, --img)
	{
		mult = (mult + 1) / 2 + *img;
		size += mult;
		*val_nbr += *img;
	}
	return (size + 1);
}

/*void			ud_img_fill_huff_table(ud_huff *table, size_t *table_index, unsigned char *img, unsigned char *stage)
{
	
	if (!*stage_index)
	{
		stage++;
		table->right_1 = table + 1;
		tabel_index++;
		ud_img_fill_huff_table(table + 1, table_index)
}*/

unsigned char	*ud_img_jpg_parse_huffman_table(unsigned char *img, ud_jpg *jpg)
{
	unsigned short	seg_len = ((((unsigned short)*img) << 8) | (unsigned short)*(img + 1));
	printf("Define Huffman Table marker : seg len %d\n", seg_len);
	printf("%02.2hhx %02.2hhx %02.2hhx\n", *img, *(img + 1), *(img + 2));
	ud_huff_class	table_class = (*(img + 2) >> 4) ? UD_HC_AC : UD_HC_DC;
	unsigned char	table_id = (*(img + 2) & 0x0f); // maybe its bitwise comparison : 0000 = 0 0001 = 1 0010 = 2 0100 = 3... to verify
	printf("Huff table[%hhd] class : %s\n", table_id, table_class == UD_HC_AC ? "AC" : "DC");
	img += 18;
	size_t			val_nbr = 0;
	size_t			huff_table_size = ud_img_get_huff_table_size(img++, &val_nbr);
	ud_huff			*table;
	//size_t			actual_index = 0;
	//size_t			size_index = *(img - 16);
	//unsigned char	*stage = (unsigned char *)ud_str_ndup((char *)(img - 16), 16);
	if (table_class == UD_HC_AC)
		table = jpg->ac_huff_tables[table_id];
	else
		table = jpg->dc_huff_tables[table_id];
	// if table != NULL free ??
	ud_ut_prot_malloc(table = ud_ut_malloc(sizeof(ud_huff) * huff_table_size));
	//ud_img_fill_huff_table(table, &actual_index, img, stage);
	//printf("size : %zu val _nbr %zu\n", huff_table_size, val_nbr);
	
	seg_len -= 19;
	for (ud_ut_count i = 0; i < seg_len; ++i, ++img)
	{
		printf("%02.2hhx ", *img);
			
	}
	printf("\n");
	if (jpg)
		;
	return (img);
}

unsigned char	*ud_img_jpg_dct_ctr(unsigned char *img, ud_jpg *jpg, unsigned char dct_type)
{
	unsigned short	seg_len = ((((unsigned short)*img) << 8) | (unsigned short)*(img + 1));
	printf("Start Of Frame marker : seg len %d\ndtc_type : %s\n", seg_len, dct_type == UD_IMG_JPG_SOF_BD ? "Baseline" : (dct_type == UD_IMG_JPG_SOF_PD ? "Progressive" : "UNKNOWN"));
	jpg->data_precision = *(img + 2);
	jpg->img_height = ((((unsigned short)*(img + 3)) << 8) | (unsigned short)*(img + 4));
	jpg->img_width = ((((unsigned short)*(img + 5)) << 8) | (unsigned short)*(img + 6));
	unsigned char	comp_nbr = *(img + 7);
	ud_jpg_comp		*comp_lst;

	jpg->comp_nbr = comp_nbr;
	img += 8;
	ud_ut_prot_malloc(jpg->components = ud_ut_malloc(sizeof(ud_jpg_comp) * comp_nbr));
	comp_lst = jpg->components;
	printf("data precision : %hhu bits\n", jpg->data_precision);
	printf("img size : %hu*%hu pixels\n", jpg->img_width, jpg->img_height);
	printf("component nbr = %hhu:\n", jpg->comp_nbr);
	for (ud_ut_count i = 0; i < comp_nbr; ++i, ++comp_lst)
	{
		comp_lst->comp_id = *img++;
		comp_lst->hor_sampling = ud_img_get_quant_bit_id((*img & 0xf0) >> 4) + 1;
		comp_lst->ver_sampling = ud_img_get_quant_bit_id(*img++ & 0x0f) + 1;
		comp_lst->quant_mat_id = *img++;
		printf("\tcomp[%hhu] :\n\t\t horizontal sampling : %hhu\n\t\t vertical sampling : %hhu\n\t\t quant mat associed : %hhu\n", comp_lst->comp_id, comp_lst->hor_sampling, comp_lst->ver_sampling, comp_lst->quant_mat_id);
	}
	return (img);
	
}

unsigned char	*ud_img_parse_zig_zag(char **new_mat, unsigned char *img, size_t size_y, size_t size_x)
{
	size_t		top_left_iter;
	size_t		bottom_right_iter;
	size_t		middle_iter;
	
	int			y_w = -1;
	int			x_w = 1;
	int			x = 0;
	int			y = 0;

	if (size_x > size_y)
	{
		top_left_iter = (2 * size_y * size_y / 2 + size_y) / 2;
		bottom_right_iter = (2 * size_y * size_y / 2 - size_y) / 2;
		middle_iter = size_y * size_x - top_left_iter - bottom_right_iter;
	}
	else
	{
		top_left_iter = (2 * size_x * size_x / 2 + size_x) / 2;
		bottom_right_iter = (2 * size_x * size_x / 2 - size_x) / 2;
		middle_iter = size_y * size_x - top_left_iter - bottom_right_iter;
	}
	for (ud_ut_count i = 0; i < top_left_iter; ++i, ++img)
	{
		if (y == -1)
		{
			++y;
			x_w = -1;
			y_w = 1;
		}
		else if (x == -1)
		{
			++x;
			x_w = 1;
			y_w = -1;
		}
		//printf("new_mat[%d][%d] = %2.2hhx\n", y, x, *img);
		new_mat[y][x] = *img;
		y += y_w;
		x += x_w;
	}
	if (size_x > size_y)
	{
		for (ud_ut_count i = 0; i < middle_iter; ++i, ++img)
		{
			if (y == -1)
			{
				++y;
				x_w = -1;
				y_w = 1;
			}
			else if (y == (int)size_y)
			{
				--y;
				x += 2;
				x_w = 1;
				y_w = -1;
			}
		}
	}
	else
	{
		for (ud_ut_count i = 0; i < middle_iter; ++i, ++img)
		{
			if (x == (int)size_x)
			{
				--x;
				y += 2;
				x_w = -1;
				y_w = 1;
			}
			else if (x == -1)
			{
				++x;
				x_w = 1;
				y_w = -1;
			}
		}
	}
	for (ud_ut_count i = 0; i < bottom_right_iter; ++i, ++img)
	{
		if (x == (int)size_x)
		{
			--x;
			y += 2;
			x_w = -1;
			y_w = 1;
		}
		else if (y == (int)size_y)
		{
			--y;
			x += 2;
			x_w = 1;
			y_w = -1;
		}
		//printf("new_mat[%d][%d] = %2.2hhx\n", y, x, *img);
		new_mat[y][x] = *img;
		y += y_w;
		x += x_w;
	}
	return (img);
}

void			ud_img_free_quant_mat(char **mat)
{
	char	**mat_tmp = mat;

	while (*mat_tmp)
		ud_ut_free(*mat_tmp++);
	ud_ut_free(mat);
}

char			**ud_img_create_new_quant_mat(ud_arr **p_quantization_mat, unsigned char nbr_bytes)
{
	char	***t_quantization_mat_val;
	char	**t_quantization_mat_line_val;
	char	*t_quantization_mat_char_val;
	unsigned char	size = nbr_bytes == 64 ? 8 : 16;

	if (!*p_quantization_mat)
		*p_quantization_mat = ud_arr_set(char **, NULL, NULL, NULL, NULL); // max quant_table = 4 
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
	while (seg_len)
	{
		unsigned char	nbr_bytes = ((*img >> 4) & 1) ? 128 : 64;
		unsigned char	size_b = ((*img >> 4) & 1) ? 16 : 8;
		unsigned char	mat_id = (*img & 0xf) % 4; // maybe its bitwise comparison : 0000 = 0 0001 = 1 0010 = 2 0100 = 3... to verify
		printf("table[%hhu] : nbr bytes : %hhu\n", mat_id, nbr_bytes);
		new_mat = ud_img_create_new_quant_mat(&(jpg->quantization_mat), nbr_bytes);
		char	***quant_mat_list = (char ***)jpg->quantization_mat->val;
		++img;
		img = ud_img_parse_zig_zag(new_mat, img, (size_t)size_b, (size_t)size_b);
		seg_len -= (nbr_bytes + 1);

		for (ud_ut_count i = 0; i < nbr_bytes;)
		{
			printf("%02.2hhx ", new_mat[i / size_b][i % size_b]);
			if (!(++i % size_b))
				printf("\n");
		}
		if (quant_mat_list[mat_id])
			ud_img_free_quant_mat(quant_mat_list[mat_id]);
		quant_mat_list[mat_id] = new_mat;
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
		return (ud_img_jpg_dct_ctr(img + 1, jpg, UD_IMG_JPG_SOF_BD));
	else if (*img == UD_IMG_JPG_SOF_PD)
		return (ud_img_jpg_dct_ctr(img + 1, jpg, UD_IMG_JPG_SOF_PD));
	else if (*img == UD_IMG_JPG_DHT)
		return (ud_img_jpg_parse_huffman_table(img + 1, jpg));
	else if (*img == UD_IMG_JPG_DQT)
		return (ud_img_jpg_parse_quantization_mat(img + 1, jpg));
	else if (*img == UD_IMG_JPG_DRI)
		printf("UD IMG JPG DRI : %02.2hhx\n", *img);
	else if (*img == UD_IMG_JPG_SOS)
		return (ud_img_jpg_scan_file(img + 1, jpg));
	else if (*img >= UD_IMG_JPG_RST_MIN && *img <= UD_IMG_JPG_RST_MAX)
		printf("UD IMG JPG RST : %02.2hhx\n", *img);
	else if (*img >= UD_IMG_JPG_APP_MIN && *img <= UD_IMG_JPG_APP_MAX)
		return (ud_img_jpg_app_ctr(img + 1, jpg, (*img & 0x0f)));
	else if (*img == UD_IMG_JPG_COM)
		printf("UD IMG JPG COM : %02.2hhx\n", *img);
	else if (*img == UD_IMG_JPG_EOI)
		printf("UD IMG JPG EOI : %02.2hhx\n", *img);
//	printf("%hhx\n", *img);
	return (img + 1);


}

int		**ud_img_decryption_jpg_to_rgb(unsigned char *img)
{
	ud_jpg	jpg;
	int		**rgb = NULL;

	jpg.quantization_mat = NULL;
	jpg.thumbnail = NULL;
	unsigned char	*svae = img;

	while (ud_img_jpg_check_marker_start(*img))
	{
		unsigned char	*img_ptr = img;
		img = ud_img_jpg_read_segment(++img, &jpg);
		img = ud_img_jpg_read_segment(++img, &jpg);
		if (*(img_ptr + 1) == 0xda)
			printf("size of frame %ld\n", img - img_ptr);
	}
	printf("%d\n", (int)(img - svae));
	unsigned char	*img_ptr = img;
	while (*img != 0xff || *(img + 1) != 0xd9)
	{
	//	++img;
		printf("%02.2hhx ", *img);
		if (*img == 0xff && *(img + 1))
		{
			printf("\nsize of frame %ld\n", img - img_ptr);
			img_ptr = img;
			img = ud_img_jpg_read_segment(++img, &jpg);
			if (*(img_ptr + 1) == 0xda)
			{
				printf("size of frame %ld\n", img - img_ptr);
				img_ptr = img;
			}
		//	unsigned char	*img_ptr = img;
		}
		else
			++img;
			//printf("%hhx\n", *(img + 1));
	}
	printf("\n");
	printf("%d\n", (int)(img - svae));
	return (rgb);
}

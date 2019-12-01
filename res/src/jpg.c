# include "ud_image.h"
#include <math.h>

static ud_density_unit	ud_img_jpg_get_density_unit(unsigned char density)
{
	if (density == 1) return (UD_DU_JPG_PBINCH);
	if (density == 2) return (UD_DU_JPG_PBCM);
	return UD_DU_JPG_UNKNOWN;
}

static unsigned char	ud_img_jpg_get_quant_bit_id(unsigned char id_b)
{
	unsigned char	id = 0;

	if (!id_b) return (0);
	while (!((id_b >> id) & 1)) ++id;
	return id;
}

static int				ud_img_jpg_zz_to_row(int zz_pos)
{
	const static int	row[64] = {0,0,1,2,1,0,0,1,2,3,4,3,2,1,0,0,1,2,3,4,5,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,7,6,5,4,3,2,1,2,3,4,5,6,7,7,6,5,4,3,4,5,6,7,7,6,5,6,7,7};
	return row[zz_pos];
}

static int				ud_img_jpg_zz_to_col(int zz_pos)
{
	const static int	col[64] = {0,1,0,0,1,2,3,2,1,0,0,1,2,3,4,5,4,3,2,1,0,0,1,2,3,4,5,6,7,6,5,4,3,2,1,0,1,2,3,4,5,6,7,7,6,5,4,3,2,3,4,5,6,7,7,6,5,4,5,6,7,7,6,7};
	return col[zz_pos];
}

static int				ud_img_jpg_tab_to_zz(int row, int col)
{
	const static int	tab[8][8] = {{0,1,5,6,14,15,27,28},{2,4,7,13,16,26,29,42},{3,8,12,17,25,30,41,43},{9,11,18,24,31,40,44,53},{10,19,23,32,39,45,52,54},{20,22,33,38,46,51,55,60},{21,34,37,47,50,56,59,61},{35,36,48,49,57,58,62,63}};
	return tab[row][col];
}

static ud_jpg_comp		*ud_img_jpg_assign_huff_tables(ud_jpg_comp *components, unsigned char comp_id, unsigned char dc_hf_id, unsigned char ac_hf_id, ud_jpg *jpg)
{
	while (components->comp_id != comp_id) ++components;
	components->ac_table = jpg->ac_huff_tables[ac_hf_id];
	components->dc_table = jpg->dc_huff_tables[dc_hf_id];
	return components;
}

static void			ud_img_jpg_reset_dc_prev(ud_jpg_comp *components, unsigned char comp_nbr)
{
	for (ud_ut_count i = 0; i < comp_nbr; ++i, ++components)
		components->dc_prev = 0;
}

static unsigned char	*ud_img_jpg_read_huff_table(ud_huff *table, char *bit_pos, unsigned char *img, ud_jpg *jpg, unsigned char *huff_val)
{
	while (table->right_1 || table->left_0)
	{
		if (*bit_pos == 7 && *img == 0xff && *(img + 1) >= UD_IMG_JPG_RST_MIN && *(img + 1) <= UD_IMG_JPG_RST_MAX)
		{
			ud_img_jpg_reset_dc_prev(jpg->components, jpg->comp_nbr);
			img += 2;
		}
		if ((*img >> *bit_pos) & 1) table = table->right_1;
		else table = table->left_0;
		if (!*bit_pos)
		{
			*bit_pos = 7;
			++img;
			if (*(img - 1) == 0xff)
			{
				if (!*img) img++;
				else
				{
					printf("An unexpected error occured\n"); // should parse dct and huffman table in progressive DCT but not supported atm.
					exit(-1);
				}
			}
		}
		else --*bit_pos;
	}
	*huff_val = table->val;
	return img;
}

static unsigned char	*ud_img_jpg_parse_ac_mcu(ud_huff *table, unsigned char *img, char *bit_pos, int channel_val[8][8], ud_jpg *jpg)
{
	short	neg_mask[16] = {0xffff,0xfffe,0xfffc,0xfff8,0xfff0,0xffe0,0xffc0,0xff80,0xff00,0xfe00,0xfc00,0xf800,0xf000,0xe000,0xc000,0x8000};
	size_t	zz_index = 1;
	unsigned char	huff_val;

	while (1)
	{
		img = ud_img_jpg_read_huff_table(table, bit_pos, img, jpg, &huff_val);
		unsigned char zero_run = ((huff_val >> 4) & 0xf);
		unsigned char category = huff_val & 0xf;
		if (huff_val == 0x00)
		{
			for (; zz_index != 64; ++zz_index) channel_val[ud_img_jpg_zz_to_row(zz_index)][ud_img_jpg_zz_to_col(zz_index)] = 0;
			return img;
		}
		while (zero_run--)
		{
			channel_val[ud_img_jpg_zz_to_row(zz_index)][ud_img_jpg_zz_to_col(zz_index)] = 0;
			++zz_index;
		}
		short val = ((*img >> *bit_pos) & 1) ? 0 : neg_mask[category];
		for (int i = category - 1; i >= 0; --i)
		{
			if (*bit_pos == 7 && *img == 0xff && *(img + 1) >= 0xd0 && *(img + 1) <= 0xd7)
			{
				img += 2;
				ud_img_jpg_reset_dc_prev(jpg->components, jpg->comp_nbr);
			}
			if (((*img >> *bit_pos) & 1)) val |= (1 << i);
			if (!*bit_pos)
			{
				++img;
				*bit_pos = 7;
				if (*(img - 1) == 0xff)
				{
					if (!*img)
						img++;
					else
					{
						printf("An unexpected error occured\n"); // should parse dct and huffman table in progressive DCT but not supported atm.
						exit(-1);
					}
				}
			}
			else --*bit_pos;
		}
		if (val < 0) ++val;
		channel_val[ud_img_jpg_zz_to_row(zz_index)][ud_img_jpg_zz_to_col(zz_index)] = val;
		if (++zz_index == 64) return img;
	}	
	return img;
}

static unsigned char	*ud_img_jpg_parse_dc_mcu(ud_huff *table, unsigned char *img, char *bit_pos, int channel_val[8][8], int *dc_prev, ud_jpg *jpg)//, int *check_eob)
{
	short	neg_mask[16] = {0xffff,0xfffe,0xfffc,0xfff8,0xfff0,0xffe0,0xffc0,0xff80,0xff00,0xfe00,0xfc00,0xf800,0xf000,0xe000,0xc000,0x8000};
	while (table->right_1 || table->left_0)
	{
		if (*bit_pos == 7 && *img == 0xff && *(img + 1) >= 0xd0 && *(img + 1) <= 0xd7)
		{
			img += 2;
			ud_img_jpg_reset_dc_prev(jpg->components, jpg->comp_nbr);
		}
		if ((*img >> *bit_pos) & 1) table = table->right_1;
		else table = table->left_0;
		if (!*bit_pos)
		{
			*bit_pos = 7;
			++img;
			if (*(img - 1) == 0xff)
			{
				if (!*img) img++;
				else
				{
					printf("An unexpected error occured\n"); // should parse dct and huffman table in progressive DCT but not supported atm.
					exit(-1);
				}
			}
		}
		else --*bit_pos;
	}
	unsigned char category = table->val;
	short diff_val = ((*img >> *bit_pos) & 1) ? 0 : neg_mask[category];
	for (int i = category - 1; i >= 0; --i)
	{
		if (*bit_pos == 7 && *img == 0xff && *(img + 1) >= 0xd0 && *(img + 1) <= 0xd7)
		{
			img += 2;
			ud_img_jpg_reset_dc_prev(jpg->components, jpg->comp_nbr);
		}
		if (((*img >> *bit_pos) & 1)) diff_val |= (1 << i);
		if (!*bit_pos)
		{
			++img;
			*bit_pos = 7;
			if (*(img - 1) == 0xff)
			{
				if (!*img) img++;
				else
				{
					printf("An unexpected error occured\n"); // should parse dct and huffman table in progressive DCT but not supported atm.
					exit(-1);
				}
			}
		}
		else --*bit_pos;
	}
	if (diff_val < 0) ++diff_val;
	channel_val[0][0] = *dc_prev + diff_val;
	*dc_prev = channel_val[0][0];
	return (img);
}

static void			ud_img_jpg_dequantize_mat(int channel_val[8][8], unsigned short *quant_mat)
{
	for (ud_ut_count i = 0; i < 8; ++i)
		for (ud_ut_count j = 0; j < 8; ++j)
			channel_val[i][j] *= (int)(quant_mat[ud_img_jpg_tab_to_zz(i, j)]);
}

static double			get_cos_val(int param)
{
 const static double cos_tab[106] = {1.000000,0.980785,0.923880,0.831470,0.707107,0.555570,0.382683,0.195090,0.000000,-0.195090,-0.382683,-0.555570,-0.707107,-0.831470,-0.923880,-0.980785,-1.000000,-0.980785,-0.923880,-0.831470,-0.707107,-0.555570,-0.382683,-0.195090,-0.000000,0.195090,0.382683,0.555570,0.707107,0.831470,0.923880,0.980785,1.000000,0.980785,0.923880,0.831470,0.707107,0.555570,0.382683,0.195090,0.000000,-0.195090,-0.382683,-0.555570,-0.707107,-0.831470,-0.923880,-0.980785,-1.000000,-0.980785,-0.923880,-0.831470,-0.707107,-0.555570,-0.382683,-0.195090,-0.000000,0.195090,0.382683,0.555570,0.707107,0.831470,0.923880,0.980785,1.000000,0.980785,0.923880,0.831470,0.707107,0.555570,0.382683,0.195090,0.000000,-0.195090,-0.382683,-0.555570,-0.707107,-0.831470,-0.923880,-0.980785,-1.000000,-0.980785,-0.923880,-0.831470,-0.707107,-0.555570,-0.382683,-0.195090,-0.000000,0.195090,0.382683,0.555570,0.707107,0.831470,0.923880,0.980785,1.000000,0.980785,0.923880,0.831470,0.707107,0.555570,0.382683,0.195090,0.000000,-0.195090};
	return (cos_tab[param]);
}

static void			ud_img_jpg_compute_idct(int channel_val[8][8], int idct_val[8][8])
{
	for (ud_ut_count y = 0; y < 8; ++y)
		for (ud_ut_count x = 0; x < 8; ++x)
		{
			double sum = 0.0;
			for (ud_ut_count u = 0; u < 8; ++u)
			{
				for (ud_ut_count v = 0; v < 8; ++v)
				{
					double Cu = u == 0 ? UD_M_1SQRT2 : 1.0;
					double Cv = v == 0 ? UD_M_1SQRT2 : 1.0;
					double Svu = channel_val[v][u];
					int		cos_x_param = (2 * x + 1) * u;
					int		cos_y_param = (2 * y + 1) * v;
					sum += Cu * Cv * Svu * get_cos_val(cos_x_param) * get_cos_val(cos_y_param);
				}
			}
			sum = sum / 4.0 + 128;
			idct_val[y][x] = ud_prot_overflow(sum);
		}
}

static void			ud_img_jpg_reverse_downsampling(int *channel, int idct_val[8][8], ud_jpg_comp *comp, ud_jpg *jpg, size_t comp_off, size_t chan_off)
{
	size_t		step_row, step_col, start_row, start_col, end_row, end_col;
	size_t		chan_row, chan_col, row_off;
	unsigned short	mcu_width = jpg->mcu_width;
	unsigned short	mcu_height = jpg->mcu_height;

	step_row = mcu_height / 8 / comp->ver_sampling;
	step_col = mcu_width / 8 / comp->hor_sampling;
	start_row = (chan_off * 8 * step_col / mcu_width) * 8 * step_row;
	start_col = (chan_off * 8 * step_col) % mcu_width;
	end_row = start_row + 8 * step_row;
	end_col = start_col + 8 * step_col;
	chan_row = start_row;
	for (ud_ut_count i_row = 0; i_row < 8; ++i_row, chan_row += step_row)
	{
		chan_col = start_col;
		for (ud_ut_count j_col = 0; j_col < 8; ++j_col, chan_col += step_col)
			for (ud_ut_count i = 0; i < step_row; ++i)
			{
				row_off = (chan_row + i) * mcu_width;
				for (ud_ut_count j = 0; j < step_col; ++j)
					channel[comp_off + row_off + chan_col + j] = idct_val[i_row][j_col];
			}
	}
}

static unsigned char	*ud_img_jpg_parse_mcu(ud_jpg_comp *comp, ud_jpg *jpg, unsigned char *img, char *bit_pos, int *channel, size_t comp_off, size_t chan_off)
{
	static int	channel_val[8][8];
	static int	idct_val[8][8];

	img = ud_img_jpg_parse_dc_mcu(comp->dc_table, img, bit_pos, channel_val, &(comp->dc_prev), jpg);
	img = ud_img_jpg_parse_ac_mcu(comp->ac_table, img, bit_pos, channel_val, jpg);
	ud_img_jpg_dequantize_mat(channel_val, ((unsigned short **)jpg->quantization_mat->val)[comp->quant_mat_id]);
	ud_img_jpg_compute_idct(channel_val, idct_val);
	ud_img_jpg_reverse_downsampling(channel, idct_val, comp, jpg, comp_off * jpg->mcu_width * jpg->mcu_height, chan_off);
	return (img);
}

static unsigned char	*ud_img_jpg_scan_file(unsigned char *img, ud_jpg *jpg)
{
	unsigned char	comp_nbr = *(img + 2);
	ud_mcu			*mcu = NULL;
	ud_jpg_comp		*components = jpg->components;
	ud_jpg_comp		*comp_list[comp_nbr];
	size_t			mcu_val_size = jpg->comp_nbr * jpg->mcu_width * jpg->mcu_height;
	img += 3;
	for (ud_ut_count i = 0; i < comp_nbr; ++i)
	{
		unsigned char	comp_id = *img++;
		unsigned char	dc_hf_id = (*img >> 4);
		unsigned char	ac_hf_id = (*img++ & 0x0f);
		comp_list[i] = ud_img_jpg_assign_huff_tables(components, comp_id, dc_hf_id, ac_hf_id, jpg);
	}
	// spectral selections and successive approximation are not necessary for baseline DCT but for progressive DCT (not supported atm) so we skip it
	img += 3;
	char	bit_pos = 7;
	size_t			mcu_nbr = jpg->img_width / jpg->mcu_width;
	if (jpg->img_width % jpg->mcu_width) ++mcu_nbr;
	if (jpg->img_height % jpg->mcu_height) mcu_nbr *= (jpg->img_height / jpg->mcu_height + 1);
	else mcu_nbr *= (jpg->img_height / jpg->mcu_height);
	jpg->mcu_nbr = mcu_nbr;
	ud_ut_prot_malloc(jpg->mcu_lst = ud_ut_malloc(sizeof(ud_mcu) * mcu_nbr));
	mcu = jpg->mcu_lst;
	for (ud_ut_count z = 0; z < mcu_nbr; ++z)
	{
		ud_ut_prot_malloc(mcu->val = ud_ut_malloc(sizeof(int) * mcu_val_size));
		int		*mcu_val = mcu->val;
		mcu->nb = (int)z;
		for (ud_ut_count i = 0; i < comp_nbr; ++i)
		{
			size_t	nb_iter = comp_list[i]->hor_sampling * comp_list[i]->ver_sampling;
			for (ud_ut_count j = 0; j < nb_iter; ++j)
				img = ud_img_jpg_parse_mcu(comp_list[i], jpg, img, &bit_pos, mcu_val, i, j);
		}
		++mcu;
		if (jpg->restart_interval && !((z + 1) % jpg->mcu_by_interval) && bit_pos != 7)
		{
			bit_pos = 7;
			++img;
		}
	}
	return (bit_pos == 7 ? img : img + 1);
}

static size_t			ud_img_jpg_get_huff_table_size(unsigned char *img, size_t *val_nbr)
{
	size_t	size = 0;
	size_t	nb_iter = 16;
	size_t	mult = 0;

	while (!*img)
	{
		--img;
		--nb_iter;
	}
	for (ud_ut_count i = 0; i < nb_iter; ++i, --img)
	{
		mult = (mult + 1) / 2 + *img;
		size += mult;
		*val_nbr += *img;
	}
	return (size + 1);
}

static unsigned char	ud_img_jpg_get_byte_len(unsigned char byte)
{
	char i = 7;

	while (i != -1)
		if (((byte >> i++) & 1)) return (i + 1);
	return (0);
}

static void			ud_img_jpg_create_end_huff(ud_huff *table, unsigned char *img)
{
	table->val = *img;
	table->val_len = ud_img_jpg_get_byte_len(*img);
	table->right_1 = NULL;
	table->left_0 = NULL;
}

static unsigned char	*ud_img_jpg_fill_huff_table(ud_huff *table, size_t *table_index, unsigned char *img, unsigned char *stage, size_t *nb_val)
{
	ud_huff	*actual = table + *table_index;

	if (!*stage)
	{
		actual->left_0 = actual + 1;
		++*table_index;
		img = ud_img_jpg_fill_huff_table(table, table_index, img, stage + 1, nb_val);
		++*table_index;
		actual->right_1 = table + *table_index;
		img = ud_img_jpg_fill_huff_table(table, table_index, img, stage + 1, nb_val);
	}
	else if (*stage)
	{
		actual->left_0 = actual + 1;
		++(*table_index);
		ud_img_jpg_create_end_huff(actual->left_0, img);
		++img;
		--*nb_val;
		--*stage;
		++*table_index;
		actual->right_1 = table + *table_index;
		if (!*nb_val) actual->right_1 = NULL;
		else if (!*stage) img = ud_img_jpg_fill_huff_table(table, table_index, img, stage + 1, nb_val);
		else
		{
			ud_img_jpg_create_end_huff(actual->right_1, img);
			++img;
			--*nb_val;
			--*stage;
		}
	}
	return img;
}

static unsigned char	*ud_img_jpg_parse_huffman_table(unsigned char *img, ud_jpg *jpg)
{
	unsigned short	seg_len = ((((unsigned short)*img) << 8) | (unsigned short)*(img + 1));
	unsigned short	index = 2;
	img += 2;
	while (index < seg_len)
	{
		ud_huff_class	table_class = (*img >> 4) ? UD_HC_AC : UD_HC_DC;
		unsigned char	table_id = (*img & 0x0f);
		img += 16;
		size_t			val_nbr = 0;
		size_t			huff_table_size = ud_img_jpg_get_huff_table_size(img++, &val_nbr);
		size_t			val_nbr_cpy = val_nbr;
		ud_huff			*table;
		size_t			actual_index = 0;
		unsigned char	*stage = (unsigned char *)ud_str_ndup((char *)(img - 16), 16);
	   	if (table_class == UD_HC_AC)
		{
			ud_ut_prot_malloc(jpg->ac_huff_tables[table_id] = ud_ut_malloc(sizeof(ud_huff) * huff_table_size));
			table = jpg->ac_huff_tables[table_id];
		}
		else
		{
			ud_ut_prot_malloc(jpg->dc_huff_tables[table_id] = ud_ut_malloc(sizeof(ud_huff) * huff_table_size));
			table = jpg->dc_huff_tables[table_id];
		}
		ud_img_jpg_fill_huff_table(table, &actual_index, img, stage, &val_nbr);
		index += 17 + val_nbr_cpy;
		img += val_nbr_cpy;
	}
	return (img);
}

static unsigned char	*ud_img_jpg_dct_ctr(unsigned char *img, ud_jpg *jpg)
{
	jpg->data_precision = *(img + 2);
	jpg->img_height = ((((unsigned short)*(img + 3)) << 8) | (unsigned short)*(img + 4));
	jpg->img_width = ((((unsigned short)*(img + 5)) << 8) | (unsigned short)*(img + 6));
	unsigned char	comp_nbr = *(img + 7);
	ud_jpg_comp		*comp_lst;

	jpg->comp_nbr = comp_nbr;
	img += 8;
	ud_ut_prot_malloc(jpg->components = ud_ut_malloc(sizeof(ud_jpg_comp) * comp_nbr));
	comp_lst = jpg->components;
	jpg->mcu_height = 8;
	jpg->mcu_width = 8;
	for (ud_ut_count i = 0; i < comp_nbr; ++i, ++comp_lst)
	{
		comp_lst->comp_id = *img++;
		comp_lst->hor_sampling = ud_img_jpg_get_quant_bit_id((*img & 0xf0) >> 4) + 1;
		comp_lst->ver_sampling = ud_img_jpg_get_quant_bit_id(*img++ & 0x0f) + 1;
		comp_lst->quant_mat_id = *img++;
		comp_lst->mcu_lst = NULL;
		comp_lst->mcu_first = NULL;
		comp_lst->dc_prev = 0;
		comp_lst->nbr_by_mcu = comp_lst->hor_sampling * comp_lst->ver_sampling;
		if (comp_lst->hor_sampling * 8 > jpg->mcu_width) jpg->mcu_width = comp_lst->hor_sampling * 8;
		if (comp_lst->ver_sampling * 8 > jpg->mcu_height) jpg->mcu_height = comp_lst->ver_sampling * 8;
	}
	return (img);
}

static unsigned short			*ud_img_jpg_create_new_quant_mat(ud_arr **p_quantization_mat)
{
	unsigned short		**t_quantization_mat_val;

	if (!*p_quantization_mat)
		*p_quantization_mat = ud_arr_set(unsigned short *, NULL, NULL, NULL, NULL); 
	t_quantization_mat_val = (unsigned short **)(*p_quantization_mat)->val;
	while (*t_quantization_mat_val) ++t_quantization_mat_val;
	ud_ut_prot_malloc(*t_quantization_mat_val = ud_ut_malloc(sizeof(unsigned short) * 64));
	return (*t_quantization_mat_val);
}

static unsigned char	*ud_img_jpg_parse_quantization_mat(unsigned char *img, ud_jpg *jpg)
{
	unsigned short	seg_len = ((((unsigned short)*img) << 8) | (unsigned short)*(img + 1));
	unsigned short	*new_mat;
	img += 2;
	seg_len -= 2;
	while (seg_len)
	{
		unsigned char	nbr_bytes = ((*img >> 4) & 1) ? 128 : 64;
		unsigned char	size_b = ((*img >> 4) & 1) ? 16 : 8;
		new_mat = ud_img_jpg_create_new_quant_mat(&(jpg->quantization_mat));
		++img;
		if (size_b == 8)
			for (ud_ut_count i = 0; i < 64; ++i, ++img) new_mat[i] = *img;
		else
			for (ud_ut_count i = 0; i < 64; ++i, img += 2)
				new_mat[i] = ((((unsigned short)*img) << 8) | (unsigned short)*(img + 1));
		seg_len -= (nbr_bytes + 1);
	}
	return (img);
}

static unsigned char	*ud_img_jpg_app_ctr(unsigned char *img, ud_jpg *jpg, unsigned char app_ref)
{
	unsigned short	seg_len = ((((unsigned short)*img) << 8) | (unsigned short)*(img + 1));
	
	if (!app_ref)
	{
		jpg->jfif_seg_len = ((((unsigned short)(*img)) << 8) | (unsigned short)*(img + 1));
		jpg->density_unit = (ud_img_jpg_get_density_unit(*(img + 2)));
		jpg->x_pixel_by_unit = ((((unsigned short)(*(img + 10))) << 8) | (unsigned short)*(img + 11));
		jpg->y_pixel_by_unit = ((((unsigned short)(*(img + 12))) << 8) | (unsigned short)*(img + 13));
		// a thumbnail is encoded here, but we skip it
	}
	return (img + seg_len);
}

static unsigned char *ud_img_jpg_restart_interval_ctr(unsigned char *img, ud_jpg *jpg)
{
	unsigned short	seg_len = ((((unsigned short)*img) << 8) | (unsigned short)*(img + 1));
	jpg->mcu_by_interval = ((((unsigned short)*(img + 2)) << 8) | (unsigned short)*(img + 3));
	jpg->restart_interval = 1;
	if (!jpg->mcu_by_interval) jpg->restart_interval = 0;
	return (img + seg_len);
}

static unsigned char	*ud_img_jpg_skip_com(unsigned char *img)
{
	unsigned short	seg_len = ((((unsigned short)*img) << 8) | (unsigned short)*(img + 1));
	return (img + seg_len);
}

static unsigned char	*ud_img_jpg_read_segment(unsigned char *img, ud_jpg *jpg)
{
	if (*img == UD_IMG_JPG_SOI)
		return (img + 1);
	else if (*img == UD_IMG_JPG_SOF0)
		return (ud_img_jpg_dct_ctr(img + 1, jpg));
	else if (*img == UD_IMG_JPG_SOF1)
		return (ud_img_jpg_dct_ctr(img + 1, jpg));
	else if (*img == UD_IMG_JPG_DHT)
		return (ud_img_jpg_parse_huffman_table(img + 1, jpg));
	else if (*img == UD_IMG_JPG_DQT)
		return (ud_img_jpg_parse_quantization_mat(img + 1, jpg));
	else if (*img == UD_IMG_JPG_DRI)
		return (ud_img_jpg_restart_interval_ctr(img + 1, jpg));
	else if (*img == UD_IMG_JPG_SOS)
		return (ud_img_jpg_scan_file(img + 1, jpg));
	else if (*img >= UD_IMG_JPG_RST_MIN && *img <= UD_IMG_JPG_RST_MAX)
		return (img + 1);
	else if (*img >= UD_IMG_JPG_APP_MIN && *img <= UD_IMG_JPG_APP_MAX)
		return (ud_img_jpg_app_ctr(img + 1, jpg, (*img & 0x0f)));
	else if (*img == UD_IMG_JPG_COM)
		return (ud_img_jpg_skip_com(img + 1));
	else if (*img == UD_IMG_JPG_EOI)
		return (img + 1);
	else if (*img >= UD_IMG_JPG_SOF0 && *img <= UD_IMG_JPG_SOF15)
	{
		printf("Start of frame marker combination not supported\n");
		exit(-1);
	}
	else
	{
		printf("An unexpected error occured, file is certainly damaged %hhx\n", *img);
		exit(-1);
	}
	return (img + 1);
}

static ud_jpg		ud_img_jpg_init_jpg_struct(void)
{
	ud_jpg jpg;

	jpg.quantization_mat = NULL;
	for (ud_ut_count i = 0; i < UD_IMG_JPG_MAX_HUFF_TABLE; ++i)
	{
		jpg.ac_huff_tables[i] = NULL;
		jpg.dc_huff_tables[i] = NULL;
	}
	jpg.mcu_lst = NULL;
	jpg.restart_interval = 0;
	jpg.img_height = 0;
	jpg.img_width = 0;
	jpg.mcu_height = 0;
	jpg.mcu_width = 0;
	return (jpg);
}

static unsigned char	clip_pixel(int val)
{
	if (val > 255) return (255);
	if (val < 0) return (0);
	return ((unsigned char)val);
}

static void		ud_img_jpg_free_jpg(ud_jpg *jpg)
{
	ud_mcu	*mcu = jpg->mcu_lst;
	for (ud_ut_count i = 0; i < UD_IMG_JPG_MAX_QUANT_MAT; ++i)
		if (((short **)jpg->quantization_mat->val)[i])
			free(((short **)jpg->quantization_mat->val)[i]);
	for (ud_ut_count i = 0; i < UD_IMG_JPG_MAX_HUFF_TABLE; ++i)
	{
		if (jpg->ac_huff_tables[i]) free(jpg->ac_huff_tables[i]);
		if (jpg->dc_huff_tables[i]) free(jpg->dc_huff_tables[i]);
	}
	while (mcu->nb + 1 != jpg->mcu_nbr)
	{
		free(mcu->val);
		++mcu;
	}
	free(jpg->mcu_lst);
	free(jpg->components);
}

static ud_img		*ud_img_jpg_build_image(ud_jpg *jpg)
{
	ud_img				*img;
	ud_mcu				*mcu = jpg->mcu_lst;
	ud_mcu				*save = mcu;
	ud_img_pix_ycbcr	*pixels;

	ud_ut_prot_malloc(img = ud_ut_malloc(sizeof(ud_img)));
	img->color_space = UD_CS_YCBCR;
	img->pixels = ud_arr_init(ud_img_pix_ycbcr, jpg->img_height * jpg->img_width);
	img->width = jpg->img_width;
	img->height = jpg->img_height;
	pixels = (ud_img_pix_ycbcr*)img->pixels->val;
	ud_ut_count	mcu_row = 0;
	ud_ut_count	mcu_row_off = 0;
	ud_ut_count	mcu_cb_off = jpg->mcu_width * jpg->mcu_height;
	ud_ut_count	mcu_cr_off = 2 * mcu_cb_off;
	img->comp_nbr = jpg->comp_nbr;
	while (1)
	{
		for (ud_ut_count i = 0; i < img->height; ++i)
		{
			for (ud_ut_count j = 0; j < img->width;)
			{
				for (ud_ut_count mcu_col = 0; mcu_col < jpg->mcu_width && j < img->width; ++mcu_col, ++j, ++pixels)
				{
					pixels->luminance = clip_pixel(mcu->val[mcu_row_off + mcu_col]);
					if (jpg->comp_nbr > 1) pixels->chroma_blue = clip_pixel(mcu->val[mcu_cb_off + mcu_row_off + mcu_col]);
					else pixels->chroma_blue = 0;
					if (jpg->comp_nbr > 2) pixels->chroma_red = clip_pixel(mcu->val[mcu_cr_off + mcu_row_off + mcu_col]);
					else pixels->chroma_red = 0;
				}
				if (j == img->width)
				{
					++mcu_row;
					if (mcu_row == jpg->mcu_height || i + 1 == jpg->img_height)
					{
						if (mcu->nb == jpg->mcu_nbr - 1)
						{
							ud_img_jpg_free_jpg(jpg);
							return (img);
						}
						++mcu;
						save = mcu;
						mcu_row = 0;
					}
					else mcu = save;
					mcu_row_off = mcu_row * jpg->mcu_width;
				}
				else ++mcu;
			}
		}
	}
	ud_img_jpg_free_jpg(jpg);
	return (img);
}

static int			ud_img_jpg_check_valid_sof_marker(unsigned char *img_str)
{
	int		sof_marker_found = 0;
	while (!sof_marker_found)
	{
		++img_str;
		if (*(img_str - 1) == 0xff && *img_str >= 0xc0 && *img_str <= 0xcf && *img_str != 0xc4 && *img_str != 0xc8 && *img_str != 0xcc)
			sof_marker_found = 1;
	}
	if (*img_str == UD_IMG_JPG_SOF0 || *img_str == UD_IMG_JPG_SOF1)
		return (1);
	if (*img_str == UD_IMG_JPG_SOF2)
		printf("SOF2 marker found: (Progressive DCT, Huffman coding) not supported\n");
	if (*img_str == UD_IMG_JPG_SOF3)
		printf("SOF3 marker found: (Lossless sequential, Huffman coding) not supported\n");
	if (*img_str == UD_IMG_JPG_SOF5)
		printf("SOF5 marker found: (Differencial sequential DCT, Huffman coding) not supported\n");
	if (*img_str == UD_IMG_JPG_SOF6)
		printf("SOF6 marker found: (Differencial progressive DCT, Huffman coding) not supported\n");
	if (*img_str == UD_IMG_JPG_SOF7)
		printf("SOF7 marker found: (Differencial lossless sequential, Huffman coding) not supported\n");
	if (*img_str == UD_IMG_JPG_SOF9)
		printf("SOF9 marker found: (Extended sequential DCT, arithmetic coding) not supported\n");
	if (*img_str == UD_IMG_JPG_SOF10)
		printf("SOF10 marker found: (Progressive DCT, arithmetic coding) not supported\n");
	if (*img_str == UD_IMG_JPG_SOF11)
		printf("SOF11 marker found: (Lossless sequential, arithmetic coding) not supported\n");
	if (*img_str == UD_IMG_JPG_SOF13)
		printf("SOF13 marker found: (Differencial sequential DCT, arithmetic coding) not supported\n");
	if (*img_str == UD_IMG_JPG_SOF14)
		printf("SOF14 marker found: (Differencial progressive DCT, arithmetic coding) not supported\n");
	if (*img_str == UD_IMG_JPG_SOF15)
		printf("SOF15 marker found: (Differencial lossless sequential, arithmetic coding) not supported\n");
	return (0);
}

ud_img		*ud_img_jpg_decryption(unsigned char *img_str)
{
	ud_jpg	jpg;
	ud_img	*img;

	jpg = ud_img_jpg_init_jpg_struct();
	if (!ud_img_jpg_check_valid_sof_marker(img_str))
		return (NULL);
	while (ud_img_jpg_check_marker_start(*img_str))
		img_str = ud_img_jpg_read_segment(img_str + 1, &jpg);
	img = ud_img_jpg_build_image(&jpg);
	mlx_print_jpg(img);
	return (img);
}

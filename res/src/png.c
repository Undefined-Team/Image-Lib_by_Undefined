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

char	*tmp_print_comp_method(int comp_method)
{
	if (comp_method == UD_PNG_RAW) return "stored/raw/literal section";
	if (comp_method == UD_PNG_STATICHUFF) return "pre-agreed Huffman tree";
	if (comp_method == UD_PNG_SUPPHUFF) return "Huffman table supplied";
	if (comp_method == UD_PNG_RESERVED) return "reserved, don't use";
	return (NULL);
}

void	tmp_print_huff_tree(ud_png_huff *tree)
{
	if (!tree->right_1 && !tree->left_0)
	{
		unsigned short	huff = tree->tmp_huff_code;
		unsigned char	l = tree->code_len;
		while (--l)
			printf("%c", ((huff >> l) & 1) ? '1' : '0');
		printf("%c", (huff & 1) ? '1' : '0');
		printf("\t = %d\t(len %d)\n", tree->val, tree->code_len);
		return ;
	}
	if (tree->left_0)
	{
		//printf("0");
		tmp_print_huff_tree(tree->left_0);
	}
	if (tree->right_1)
	{
	//	printf("1");
		tmp_print_huff_tree(tree->right_1);
	}
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

void					ud_img_png_init_tree(void *huff_tree, size_t tree_len)
{
	char			*huff_tree_init = (char *)huff_tree;

	for (ud_ut_count i = 0; i < tree_len; ++i) huff_tree_init[i] = '\0';
}

int						ud_img_png_read_bits(unsigned char **img_str_addr, unsigned char *bit_pos, size_t nb_bits)
{
	int	val = 0;
	unsigned char	*img_str = *img_str_addr;
	unsigned char	val_bit = 0;
	while (nb_bits > 8 - *bit_pos)
	{
		val |= ((*img_str >> *bit_pos) << val_bit);
		val_bit += (8 - *bit_pos);
		nb_bits -= (8 - *bit_pos);
		*bit_pos = 0;
		img_str++;
	}
	val |= (((*img_str >> *bit_pos) & (0xff >> (8 - nb_bits))) << val_bit);
	*bit_pos += nb_bits;
	if (*bit_pos == 8)
	{
		img_str++;
		*bit_pos = 0;
	}
	*img_str_addr = img_str;
	return val;
}

/*
void					ud_img_png_fill_static_huffman_tree(int depth, int first_value, int last_value, int rep)
{
	if (first_value > last_value)
		return ;
	ud_img_png_fill_static_huffman_tree(depth, first_value + 1, last_value, rep + 1);
	while (depth--)
		printf("%c", ((rep >> depth) & 1) ? '1': '0');
	printf(" = %d\n", first_value);
}

ud_png_huff				*ud_img_png_build_static_huffman_tree(void)
{
	ud_png_huff		*huff_tree;
	//size_t			*index = 0;

	ud_ut_prot_malloc(huff_tree = ud_ut_malloc(sizeof(ud_png_huff) * UD_IMG_PNG_STATICHUFF_SIZE));
	ud_img_png_init_tree(huff_tree, sizeof(ud_png_huff) * UD_IMG_PNG_STATICHUFF_SIZE);
	ud_img_png_fill_static_huffman_tree(7, 256, 279, 0);
	return NULL;
}
*/
unsigned char				ud_img_png_get_max_code_length(unsigned char *cl_list, size_t len)
{
	unsigned char	max_code_length = 0;

	for (ud_ut_count i = 0; i < len; ++i)
	{
		if (*cl_list > max_code_length)
			max_code_length = *cl_list;
		++cl_list;
	}
	return max_code_length;
}

unsigned char				*ud_img_png_count_code_length(unsigned char *cl_list, unsigned char max_code_length, size_t len)
{
	unsigned char	*cl_count;

	ud_ut_prot_malloc(cl_count = (unsigned char *)ud_ut_malloc(sizeof(unsigned char) * (max_code_length + 1)));
	//cl_count[max_code_length + 1] = 0;
	for (ud_ut_count i = 0; i <= max_code_length; ++i) cl_count[i] = 0;
	for (ud_ut_count i = 0; i < len; ++i)
	{
		printf("cl_list[%zu] : %d\n", i, cl_list[i]);
		++(cl_count[cl_list[i]]);
	}
	for (ud_ut_count i = 0; i <= max_code_length; ++i) printf("%d ",cl_count[i]);
	printf("\n");
	return cl_count;
}

static size_t			ud_img_png_get_huff_table_size(unsigned char *cl_count, size_t nb_iter)
{
	size_t	size = 0;
	size_t	mult = 0;

	for (ud_ut_count i = 0; i < nb_iter; ++i, --cl_count)
	{
		mult = (mult + 1) / 2 + *cl_count;
		size += mult;
	}
	return (size + 1);
}

void	ud_img_png_init_huff_node(ud_png_huff *node)
{
	//ud_ut_prot_malloc(node = ud_ut_malloc(sizeof(ud_png_huff)));
	node->left_0 = NULL;
	node->right_1 = NULL;
	node->val = 0;
	node->tmp_huff_code = 0;
	node->code_len = 0;
}

/*void						ud_img_png_add_huff_val(ud_png_huff *huff_tree, unsigned char depth, unsigned char code_len, unsigned short huff_code, size_t val, size_t *add_index, ud_png_huff *actual)
{
	if (!depth)
	{
		actual->val = val;
		actual->code_len = code_len;
		actual->tmp_huff_code = huff_code;
		return ;
	}
	if (((huff_code >> (depth - 1)) & 1))
	{
		if (!(actual->right_1))
		{
		//	actual->right_1 = malloc(sizeof(ud_png_huff));
		//	ud_img_png_init_huff_node(actual->right_1);
		//	printf("add_index %zu\n", *add_index);
			ud_img_png_init_huff_node(&(huff_tree[*add_index]));
			actual->right_1 = &(huff_tree[(*add_index)++]);
			//ud_img_png_create_huff_node();
		}
			//printf("add_index %zu\n", *add_index);
		return ud_img_png_add_huff_val(huff_tree, depth - 1, code_len, huff_code, val, add_index, actual->right_1);
	}
	if (!(actual->left_0))
	{
//		actual->left_0 = malloc(sizeof(ud_png_huff));
//		ud_img_png_init_huff_node(actual->left_0);
//		printf("add_index %zu\n", *add_index);
		ud_img_png_init_huff_node(&(huff_tree[*add_index]));
		actual->left_0 = &(huff_tree[(*add_index)++]);
	}
//	printf(" afteradd_index %zu\n", *add_index);
	return ud_img_png_add_huff_val(huff_tree, depth - 1, code_len, huff_code, val, add_index, actual->left_0);
}
*/
void						ud_img_png_add_huff_val(ud_png_huff *huff_tree, unsigned char depth, unsigned char code_len, unsigned short huff_code, unsigned short val, size_t *add_index, ud_png_huff *actual)
{
	while (depth--)
	{
		if (((huff_code >> depth) & 1))
		{
			if (!actual->right_1)
			{
				actual->right_1 = &(huff_tree[*add_index]);
				ud_img_png_init_huff_node(&(huff_tree[(*add_index)++]));
			}
			actual = actual->right_1;
		}
		else
		{
			if (!actual->left_0)
			{
				actual->left_0 = &(huff_tree[*add_index]);
				ud_img_png_init_huff_node(&(huff_tree[(*add_index)++]));
			}
			actual = actual->left_0;
		}
	}
	actual->val = val;
	actual->code_len = code_len;
	actual->tmp_huff_code = huff_code;
}

/*void	fct
{
	if ()
}*/

// NEED TO REPLACE HUFF TREE NODE ALLOCAION BY A BIG TAB ALLOCATION LIKE IN JPG

ud_png_huff					*ud_img_png_create_huffman_tree(unsigned char *cl_list, unsigned char *cl_count, unsigned short *next_code, size_t val_nbr, unsigned char max_code_length)
{
	//(void)next_code;
	size_t		add_index = 1;
	size_t		tree_size = ud_img_png_get_huff_table_size(cl_count + max_code_length, max_code_length);
	printf("tree_size %zu\n", tree_size);
	ud_png_huff	*huff_tree; //= ud_img_png_create_huff_node();

	ud_ut_prot_malloc(huff_tree = ud_ut_malloc(sizeof(ud_png_huff) * tree_size));
	ud_img_png_init_huff_node(huff_tree);
	for (ud_ut_count i = 0; i < val_nbr; ++i)
	{
		if (cl_list[i])
			ud_img_png_add_huff_val(huff_tree, cl_list[i], cl_list[i], next_code[cl_list[i]]++, i, &add_index, huff_tree);
	}
	//free(cl_count);
	return huff_tree;
}

ud_png_huff					*ud_img_png_cl_to_huffman_tree(unsigned char *cl_list, size_t len)
{
	unsigned char	max_code_length = ud_img_png_get_max_code_length(cl_list, len);
	unsigned char	*cl_count = ud_img_png_count_code_length(cl_list, max_code_length, len);
	unsigned short	*next_code;
	int				code = 0;

	ud_ut_prot_malloc(next_code = ud_ut_malloc(sizeof(unsigned char) * (max_code_length + 1)));
	//for (ud_ut_count i = 0; i < 8; ++i) printf("%hhu ", cl_count[i]);
	cl_count[0] = 0;
	next_code[0] = 0;
	printf("\n0 ");
	for (ud_ut_count i = 1; i <= max_code_length; ++i)
	{
		code = ((code + cl_count[i - 1]) << 1);
		next_code[i] = code;
		printf("%hu ", next_code[i]);
	}
	printf("\n");
	/*for (ud_ut_count i = 0; i < val_nbr; ++i)
	{
		if (cl_list[i])
			printf("value %zu: length %d repres %d\n", i, cl_list[i], next_code[cl_list[i]]++);
	}*/
	return ud_img_png_create_huffman_tree(cl_list, cl_count,  next_code, len, max_code_length);
}

unsigned short				ud_img_png_read_huffman_tree(ud_png_huff *cl_huff, unsigned char **img_str_addr, unsigned char *bit_pos)
{
	while (cl_huff->right_1 || cl_huff->left_0)
	{
		if (ud_img_png_read_bits(img_str_addr, bit_pos, 1) == 1)
			cl_huff = cl_huff->right_1;
		else
			cl_huff = cl_huff->left_0;
	}
	return cl_huff->val;
}

unsigned char				*ud_img_png_read_symbols(ud_png_huff *cl_huff, unsigned char **img_str_addr, unsigned char *bit_pos, size_t elem_nbr)
{
	unsigned char	*litlen_dist_cl;
	size_t			index = 0;
	unsigned char	symbol;
	unsigned char	repeat;
	unsigned char	prev_cl;

	ud_ut_prot_malloc(litlen_dist_cl = ud_ut_malloc((elem_nbr + 1) * sizeof(unsigned char)));
	while (index < elem_nbr)
	{
		symbol = ud_img_png_read_huffman_tree(cl_huff, img_str_addr, bit_pos);
		if (symbol < 16) litlen_dist_cl[index++] = symbol;
		else
		{
			prev_cl = litlen_dist_cl[index - 1];
			if (symbol == 16) repeat = ud_img_png_read_bits(img_str_addr, bit_pos, 2) + 3;
			else if (symbol == 17) repeat = ud_img_png_read_bits(img_str_addr, bit_pos, 3) + 3;
			else if (symbol == 18) repeat = ud_img_png_read_bits(img_str_addr, bit_pos, 7) + 11;
			while (repeat--) litlen_dist_cl[index++] = prev_cl;
		}
	}
	return litlen_dist_cl;
}

void				ud_img_png_parse_huffman_tree(unsigned char **img_str_addr, unsigned char *bit_pos, ud_png_huff **litlen_tree, ud_png_huff **dist_tree)
{
	const unsigned char	cl_order[19] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
	unsigned short	hlit = ud_img_png_read_bits(img_str_addr, bit_pos, 5) + 257;//(*img_str++ >> *bit_pos) + 257;	//literal/length codes
	unsigned char	hdist = ud_img_png_read_bits(img_str_addr, bit_pos, 5) + 1;//(*img_str & 0x1f) + 1;	//distance codes
	unsigned char	hclen =  ud_img_png_read_bits(img_str_addr, bit_pos, 4) + 4; //((*img_str >> 5) | ((*(img_str + 1) & 1) << 3));	//code length codes
	unsigned char	*code_lengths;
	unsigned char	*litlen_dist_cl;
	ud_png_huff		*cl_huff;

	printf("hlit : %hu hdit : %hhu hclen %hhu\n", hlit, hdist, hclen);
	ud_ut_prot_malloc_void(code_lengths = ud_ut_malloc(sizeof(unsigned char) * 19)); // technically only hclen values maybe change it later
	for (ud_ut_count i = 0; i < 19; ++i) code_lengths[i] = 0;
	for (ud_ut_count i = 0; i < hclen; ++i)
		code_lengths[cl_order[i]] = ud_img_png_read_bits(img_str_addr, bit_pos, 3);
	cl_huff = ud_img_png_cl_to_huffman_tree(code_lengths, 19);
	tmp_print_huff_tree(cl_huff);
	litlen_dist_cl = ud_img_png_read_symbols(cl_huff, img_str_addr, bit_pos, hlit + hdist);
	*litlen_tree = ud_img_png_cl_to_huffman_tree(litlen_dist_cl, hlit);
	tmp_print_huff_tree(*litlen_tree);
	*dist_tree = ud_img_png_cl_to_huffman_tree(litlen_dist_cl + hlit, hdist);
	tmp_print_huff_tree(*dist_tree);
	//return (NULL);
}

static unsigned char	*ud_img_png_parse_data(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	unsigned char	bit_pos = 0;
	int				last_block = 0;
	(void)png;
	printf("IDATA (Datastream) Chunk: cmf byte %hhx flag byte %hhx \n", *img_str, *(img_str + 1));
	int		cm = (*img_str & 0x0f); // compression method
	int		cinfo = ((*img_str++ & 0xf0) >> 4); // compression info
	int		fcheck = (*img_str & 0x1f); // check_bits
	int		fdict = ((*img_str >> 5) & 1); // dictionnary
	int		flevel = ((*img_str++ >> 6) & 2); // compression level
	printf("New block :\n\tcm: %x\n\tcinfo: %x\n\tfcheck: %x\n\tfdcit: %x\n\tflevel: %x\n", cm, cinfo, fcheck, fdict, flevel);
	do // read a block
	{
		last_block = ((*img_str >> bit_pos++) & 1);	
		int		comp_method = ((*img_str >> bit_pos) & 2);
		
		bit_pos += 2;
		printf("\tLast block ? %d Comp method: %s\n",last_block,tmp_print_comp_method(comp_method));
		if (comp_method == UD_PNG_SUPPHUFF) ud_img_png_parse_huffman_tree(&img_str, &bit_pos, &png->litlen_tree, &png->dist_tree);
		//if (comp_method == UD_PNG_STATICHUFF) ud_img_png_build_static_huffman_tree();
		//ud_img_png_parse_huffman_table(img_str);
		//img_str++;
		//chunk_len -= 3;
		//while (chunk_len--)
			//printf("%02.2hhx ", *img_str++);
		exit(0);
	printf("\n");
	return (img_str + 4);

	}
	while (!last_block);
	/*bit_pos--;
	int		comp_method = 0;
	if (((*img_str >> bit_pos--) & 1))
		comp_method |= 2;
	if (((*img_str >> bit_pos--) & 1))
		comp_method |= 1;
	printf("\tComp method: %c%c\n", (comp_method & 2) ? '1': '0', (comp_method & 1) ? '1' : '0');
	*/
	while (chunk_len--)
		printf("%02.2hhx ", *img_str++);
	printf("\n");
	return (img_str + 4);
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
	png.litlen_tree = NULL;
	png.dist_tree = NULL;
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

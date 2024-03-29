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
		printf("\t = %hu\t(len %hhu)\n", tree->val, tree->code_len);
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
	ud_ut_prot_malloc(png->palette = ud_ut_malloc(palette_entries * sizeof(ud_img_pix_rgba)));
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
/*
void					ud_img_png_init_tree(void *huff_tree, size_t tree_len)
{
	char			*huff_tree_init = (char *)huff_tree;

	for (ud_ut_count i = 0; i < tree_len; ++i) huff_tree_init[i] = '\0';
}
*/

/*int						ud_img_png_read_bit(unsigned char **img_str_addr, unsigned char *bit_pos)
{
	int	bit_val = ((**img_str_addr >> *bit_pos) & 1);
	++(*bit_pos);
	if (*bit_pos == 8)
	{
		(*img_str_addr)++;
		*bit_pos = 0;
	}
	return bit_val;
}

int						ud_img_png_read_bits(unsigned char **img_str_addr, unsigned char *bit_pos, size_t nb_bits)
{
	int	val = 0;
	
	for (ud_ut_count i = 0; i < nb_bits; ++i)
		val |= (ud_img_png_read_bit(img_str_addr, bit_pos) << i);
	return val;
}
*/
int                                             ud_img_png_read_bits(unsigned char **img_str_addr, unsigned char *bit_pos, size_t nb_bits)
{
	    int     val = 0;
		unsigned char   *img_str = *img_str_addr;
		unsigned char   val_bit = 0;
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
		//printf("cl_list[%zu] : %d\n", i, cl_list[i]);
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
	node->left_0 = NULL;
	node->right_1 = NULL;
}

void						ud_img_png_add_huff_val(ud_png_huff *huff_tree, unsigned char depth, unsigned char code_len, unsigned short huff_code, size_t val, size_t *add_index, ud_png_huff *actual)
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
			ud_img_png_init_huff_node(&(huff_tree[*add_index]));
			actual->right_1 = &(huff_tree[(*add_index)++]);
		}
		return ud_img_png_add_huff_val(huff_tree, depth - 1, code_len, huff_code, val, add_index, actual->right_1);
	}
	if (!(actual->left_0))
	{
		ud_img_png_init_huff_node(&(huff_tree[*add_index]));
		actual->left_0 = &(huff_tree[(*add_index)++]);
	}
	return ud_img_png_add_huff_val(huff_tree, depth - 1, code_len, huff_code, val, add_index, actual->left_0);
}

/*void						ud_img_png_add_huff_val(ud_png_huff *huff_tree, unsigned char depth, unsigned char code_len, unsigned short huff_code, size_t val, size_t *add_index, ud_png_huff *actual)
{
	if (!depth)
	{
		actual->val = val;
	}
}*/


ud_png_huff					*ud_img_png_create_huffman_tree(unsigned char *cl_list, unsigned char *cl_count, unsigned short *next_code, size_t val_nbr, unsigned char max_code_length)
{
	(void)next_code;
	size_t		add_index = 1;
	size_t		tree_size = ud_img_png_get_huff_table_size(cl_count + max_code_length, max_code_length);
	//printf("tree_size %zu\n", tree_size);
	ud_png_huff	*huff_tree; //= ud_img_png_create_huff_node();

	ud_ut_prot_malloc(huff_tree = ud_ut_malloc(sizeof(ud_png_huff) * (tree_size)));
	ud_img_png_init_huff_node(huff_tree);
	for (ud_ut_count i = 0; i < val_nbr; ++i)
	{
		if (cl_list[i])
		{
			//printf("rep %d val %zu len %d\n", next_code[cl_list[i]], i, cl_list[i]);
			ud_img_png_add_huff_val(huff_tree, cl_list[i], cl_list[i], next_code[cl_list[i]]++, i, &add_index, huff_tree);
		}
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

	ud_ut_prot_malloc(next_code = ud_ut_malloc(sizeof(unsigned short) * (max_code_length + 1)));
	//for (ud_ut_count i = 0; i < 8; ++i) printf("%hhu ", cl_count[i]);
	cl_count[0] = 0;
	next_code[0] = 0;
	printf("\nnext code 0 ");
	for (ud_ut_count i = 1; i <= max_code_length; ++i)
	{
		//if (cl_count[i - 1])
		code = ((code + cl_count[i - 1]) << 1);
		if (cl_count[i])
		next_code[i] = code;
		else
			next_code[i] = 0;
		printf("%hu ", next_code[i]);
	}
	printf("\n");
	/*for (ud_ut_count i = 0; i < len; ++i)
	{
		if (cl_list[i])
			printf("value %zu: length %d repres %d\n", i, cl_list[i], next_code[cl_list[i]]++);
	}*/
	return ud_img_png_create_huffman_tree(cl_list, cl_count,  next_code, len, max_code_length);
}

unsigned short				ud_img_png_read_huffman_tree(ud_png_huff *huff_tree, unsigned char **img_str_addr, unsigned char *bit_pos)
{
	while (huff_tree->right_1 || huff_tree->left_0)
	{
		if (ud_img_png_read_bits(img_str_addr, bit_pos, 1) == 1)
		{
	//		printf("1");
			huff_tree = huff_tree->right_1;
		}
		else
		{
	//		printf("0");
			huff_tree = huff_tree->left_0;
		}
		if (!huff_tree)
		{
			printf("An unexpected error occured\n");
			exit(EXIT_FAILURE);
		}
	}
	//printf("\n");
	return huff_tree->val;
}

unsigned char				*ud_img_png_read_symbols(ud_png_huff *cl_huff, unsigned char **img_str_addr, unsigned char *bit_pos, size_t elem_nbr)
{
	unsigned char	*litlen_dist_cl;
	size_t			index = 0;
	unsigned char	symbol;
	unsigned char	repeat;
	unsigned char	prev_cl;

	ud_ut_prot_malloc(litlen_dist_cl = ud_ut_malloc(elem_nbr * sizeof(unsigned char)));
	while (index < elem_nbr)
	{
		symbol = ud_img_png_read_huffman_tree(cl_huff, img_str_addr, bit_pos);
		if (symbol < 16) litlen_dist_cl[index++] = symbol;
		else
		{
			if (symbol == 16)
			{
				prev_cl = litlen_dist_cl[index - 1];
				repeat = ud_img_png_read_bits(img_str_addr, bit_pos, 2) + 3;
				while (repeat--) litlen_dist_cl[index++] = prev_cl;
			}
			else
			{
				if (symbol == 17) repeat = ud_img_png_read_bits(img_str_addr, bit_pos, 3) + 3;
				else if (symbol == 18) repeat = ud_img_png_read_bits(img_str_addr, bit_pos, 7) + 11;
				while (repeat--) litlen_dist_cl[index++] = 0;
			}
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

void					ud_img_png_inflate_huffman_coding(unsigned char **img_str_addr, unsigned char *bit_pos, ud_png *png)
{
	unsigned short				val = 0;
	size_t						len;
	size_t						dist_val;
	size_t						dist;
	int							eob = 0;
	const static unsigned char	litlen_extra_bits[29] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
	const static unsigned short	litlen_base[29] = {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258};
	const static unsigned char	dist_extra_bits[30] = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
	const static unsigned short	dist_base[30] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};

	while (!eob)
	{
		val = ud_img_png_read_huffman_tree(png->litlen_tree, img_str_addr, bit_pos);
		//printf("val : %hu\n", val);
		if (val < 256) png->pix_stream[png->stream_index++] = val;
		else if (val > 256 && val < 286)
		{
			val -= 257;
			len = ud_img_png_read_bits(img_str_addr, bit_pos, litlen_extra_bits[val]) + litlen_base[val];
			dist_val = ud_img_png_read_huffman_tree(png->dist_tree, img_str_addr, bit_pos);
			if (dist_val > 29)
			{
				printf("Unexpected distance value: %zu \"Distance codes 30-31 will never actually occur in the compressed data\"(RFC 1951)", dist_val);
				exit(EXIT_FAILURE);
			}
			dist = ud_img_png_read_bits(img_str_addr, bit_pos, dist_extra_bits[dist_val]) + dist_base[dist_val];
			for (ud_ut_count i = 0; i < len; ++i, ++png->stream_index)
				png->pix_stream[png->stream_index] = png->pix_stream[png->stream_index - dist];
		}
		else if (val == 256)
			eob = 1;
		else
		{
			printf("Unexpected literal/length value: %hu \"Literal/length values 286-287 will never actually occur in the compressed data\" (RFC 1951)\n", val);
			exit(EXIT_FAILURE);
		}
	}
}

void	ud_img_png_build_static_huffman_tree(ud_png *png)
{
	int		rep_val = 0;
	size_t	add_index = 1;

	ud_ut_prot_malloc_void(png->litlen_tree = ud_ut_malloc(sizeof(ud_png_huff) * UD_IMG_PNG_STATICHUFF_SIZE));
	ud_ut_prot_malloc_void(png->dist_tree = ud_ut_malloc(sizeof(ud_png_huff) * 69)); // add macro
	ud_img_png_init_huff_node(png->litlen_tree);
	ud_img_png_init_huff_node(png->dist_tree);
	for (ud_ut_count i = 256; i < 280; ++i)
		ud_img_png_add_huff_val(png->litlen_tree, 7, 7, rep_val++, i, &add_index, png->litlen_tree);
	rep_val <<= 1;
	for (ud_ut_count i = 0; i < 144; ++i)
		ud_img_png_add_huff_val(png->litlen_tree, 8, 8, rep_val++, i, &add_index, png->litlen_tree);
	for (ud_ut_count i = 280; i < 288; ++i)
		ud_img_png_add_huff_val(png->litlen_tree, 8, 8, rep_val++, i, &add_index, png->litlen_tree);
	rep_val <<= 1;
	for (ud_ut_count i = 144; i < 256; ++i)
		ud_img_png_add_huff_val(png->litlen_tree, 9, 9, rep_val++, i, &add_index, png->litlen_tree);
	add_index = 1;
	rep_val = 0;
	for (ud_ut_count i = 0; i < 30; ++i)
		ud_img_png_add_huff_val(png->dist_tree, 5, 5, rep_val++, i, &add_index, png->dist_tree);
}

static unsigned char	*ud_img_png_parse_data(unsigned int chunk_len, unsigned char *img_str, ud_png *png)
{
	unsigned char	bit_pos = 0;
	int				last_block = 0;
	unsigned char	*img_str_cpy = img_str;
	(void)png;
	printf("IDATA (Datastream) Chunk: cmf byte %hhx flag byte %hhx \n", *img_str, *(img_str + 1));
	int		cm = (*img_str & 0x0f); // compression method
	int		cinfo = ((*img_str++ & 0xf0) >> 4); // compression info
	int		fcheck = (*img_str & 0x1f); // check_bits
	int		fdict = ((*img_str >> 5) & 1); // dictionnary
	int		flevel = ((*img_str++ >> 6) & 2); // compression level
	printf("New block :\n\tcm: %x\n\tcinfo: %x\n\tfcheck: %x\n\tfdcit: %x\n\tflevel: %x\n", cm, cinfo, fcheck, fdict, flevel);
	if (cm != 8)
	{
		printf("Invalid compression method (cm)\n");
		exit(EXIT_FAILURE);
		//return (img_str_cpy + chunk_len + 4);
		
	//	return 
	}
	do // read a block
	{
		last_block = ud_img_png_read_bits(&img_str, &bit_pos, 1);
		int		comp_method = ud_img_png_read_bits(&img_str, &bit_pos, 2);
		printf("\tLast block ? %d Comp method: %s\n",last_block,tmp_print_comp_method(comp_method));
		if (comp_method == UD_PNG_RAW)
		{
			/*if (bit_pos != 0)
			{
				++img_str;
			}*/
			bit_pos = 0;
			unsigned short	len = ((*img_str) | (*(img_str + 1) << 8));
			unsigned short	nlen = ((*(img_str + 2) << 8) | *(img_str + 3));
			(void)nlen;
			img_str += 4;
			printf("%hhu %hhu len %hu\n", *(img_str - 4), *(img_str - 3), len);
			for (ud_ut_count i = 0; i < len; ++i, ++(png->stream_index), ++img_str)
			{
				printf("%zu = %hhu\n", png->stream_index, *img_str);
				png->pix_stream[png->stream_index] = *img_str;
			}
			//read raw 
			//printf("Raw data\n");
			//exit(0);
		}
		else
		{
			if (comp_method == UD_PNG_SUPPHUFF) ud_img_png_parse_huffman_tree(&img_str, &bit_pos, &png->litlen_tree, &png->dist_tree);
			else if (comp_method == UD_PNG_STATICHUFF) ud_img_png_build_static_huffman_tree(png);
			else
			{
				printf("Reserved compression method: 11 \"11 - reserved (error)\" (RFC 1951)");
		//		return (img_str_cpy + chunk_len + 4);
				exit(EXIT_FAILURE);
			}
			//tmp_print_huff_tree(png->litlen_tree);
			//tmp_print_huff_tree(png->dist_tree);
				;//return (img_str_cpy + chunk_len + 4);
				;//build static tree
			ud_img_png_inflate_huffman_coding(&img_str, &bit_pos, png);
			//ud_img_png_parse_huffman_table(img_str);
		//img_str++;
		//chunk_len -= 3;
		//while (chunk_len--)
			//printf("%02.2hhx ", *img_str++);
		}

		//exit(1);
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
	//while (chunk_len--)
	//	printf("%02.2hhx ", *img_str++);
	//printf("\n");
	return (img_str_cpy + chunk_len + 4);
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
	png->bkgd_flag = 1;
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
	else if (chunk_len * 2 != png->palette_entries)
	{	
		printf("Histogram size is not corresponding to palette entries number, file must be corrupted\n");
	//	exit(-1);
	}
	else
	{
		for (ud_ut_count i = 0; i < png->palette_entries; ++i)
		{
			unsigned short	freq = ud_img_png_parse_short_val(img_str);
			img_str += 2;
			printf("\t[%zu]\t => freq : %hu\n", i, freq);
		}
	}
	return (img_str + chunk_len + 4);
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

	png.background.red = 222 * 256 + 1;
	png.background.green = 222 * 256 + 1;
	png.background.blue = 222 * 256 + 1;
	png.palette = NULL;
	png.bkgd_flag = 0;
	png.chroma = NULL;
	png.gamma = 1;
	png.transp_tab = NULL;
	png.litlen_tree = NULL;
	png.dist_tree = NULL;
	png.img_width = 32;
	png.img_height = 32;
	png.stream_index = 0;
	return png;
}

size_t	ud_img_png_get_chann_nbr(ud_png_color_type color_type)
{
	if (color_type == UD_PNG_GREYSCALE_A) return 2;
	if (color_type == UD_PNG_RGB_A) return 4;
	if (color_type == UD_PNG_RGB) return 3;
	return 1; // GREYSCALE // PALETTE
}


int		ud_img_png_get_bit_depth_mask(unsigned char bit_depth)
{
	if (bit_depth == 1) return 0x01;
	if (bit_depth == 2) return 0x03;
	if (bit_depth == 4) return 0x0f;
	if (bit_depth == 8) return 0xff;
	return 0;//error
}

void		ud_img_png_set_color_type(ud_img *img, int comp_nbr, ud_img_color_space color_space, ud_arr *arr_init)
{
	img->comp_nbr = comp_nbr;
	img->color_space = color_space;
	img->pixels = arr_init;
}

void		ud_img_png_define_color_space(ud_img *img, ud_png *png)
{
	if (png->color_type == UD_PNG_GREYSCALE)
	{
		if (png->transp_tab)
			ud_img_png_set_color_type(img, 2, UD_CS_GREYSCALEA, ud_arr_init(ud_img_pix_greyscalea, png->img_width * png->img_height));
		else ud_img_png_set_color_type(img, 1, UD_CS_GREYSCALE, ud_arr_init(ud_img_pix_greyscale, png->img_width * png->img_height));
	}
	else if (png->color_type == UD_PNG_RGB)
	{
		if (png->transp_tab) ud_img_png_set_color_type(img, 4, UD_CS_RGBA, ud_arr_init(ud_img_pix_rgba, png->img_width * png->img_height));
		else ud_img_png_set_color_type(img, 3, UD_CS_RGB, ud_arr_init(ud_img_pix_rgb, png->img_width * png->img_height));
	}
	else if (png->color_type == UD_PNG_RGB_A)
		ud_img_png_set_color_type(img, 4, UD_CS_RGBA, ud_arr_init(ud_img_pix_rgba, png->img_width * png->img_height));
	else if (png->color_type == UD_PNG_GREYSCALE_A)
		ud_img_png_set_color_type(img, 2, UD_CS_GREYSCALEA, ud_arr_init(ud_img_pix_greyscalea, png->img_width * png->img_height));
	else if (png->color_type == UD_PNG_PALETTE)
		ud_img_png_set_color_type(img, 4, UD_CS_RGBA, ud_arr_init(ud_img_pix_rgba, png->img_width * png->img_height)); // to revise
}

ud_png_filter	ud_img_png_get_filter_type(unsigned char filter_byte)
{
	if (filter_byte == 0) return UD_PNG_F_NONE;
	if (filter_byte == 1) return UD_PNG_F_SUB;
	if (filter_byte == 2) return UD_PNG_F_UP;
	if (filter_byte == 3) return UD_PNG_F_AVERAGE;
	if (filter_byte == 4) return UD_PNG_F_PAETH;
	return UD_PNG_F_NONE; // idk default value ?
}
/*
static unsigned char	ud_img_png_clip_pixel(int val)
{
	if (val > 255) return (255);
	if (val < 0) return (0);
	return ((unsigned char)val);
}
*/

void			ud_img_png_update_bit_shift(int *bit_shift, int *index, unsigned char bit_depth)
{
	*bit_shift -= bit_depth;
	if (*bit_shift < 0)
	{
		if (bit_depth == 16)
		{
			*bit_shift = 8;
			*index += 2;
		}
		else
		{
			*bit_shift = 8 - bit_depth;
			(*index)++;
		}
	}
}

unsigned short	ud_img_png_assign_pixel(unsigned char *pix_stream, int *index, int mask, int *bit_shift, unsigned char bit_depth)
{
	if (bit_depth == 16)
	{
		return (((pix_stream[*index] << 8) | pix_stream[*index + 1]));// * 255 / ((1 << bit_depth) - 1));
		//return ((((pix_stream[*index] << 8) | pix_stream[*index + 1])) / 256);
	}
	return ((pix_stream[*index] >> *bit_shift) & mask);// * 255 / ((1 << bit_depth) - 1));
}

unsigned char	ud_img_png_convert_pixel(unsigned short value, unsigned char bit_depth)
{
	return (value * 255 / ((1 << bit_depth) - 1));
}

void			ud_img_png_assign_rgb(void *pixel_addr, size_t pix_index, unsigned char *pix_stream, int *index, ud_img_pix_rgba *palette, ud_img_png_spix *transp_tab, unsigned char bit_depth, int mask, int *bit_shift)
{
	ud_img_pix_rgb *pixel = (ud_img_pix_rgb*)pixel_addr;

	(void)palette;
	(void)transp_tab;
	pixel[pix_index].red = ud_img_png_convert_pixel(ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth), bit_depth);
	ud_img_png_update_bit_shift(bit_shift, index, bit_depth);
	pixel[pix_index].green = ud_img_png_convert_pixel(ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth), bit_depth);
	ud_img_png_update_bit_shift(bit_shift, index, bit_depth);
	pixel[pix_index].blue = ud_img_png_convert_pixel(ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth), bit_depth);
	ud_img_png_update_bit_shift(bit_shift, index, bit_depth);
}

void			ud_img_png_assign_rgba(void *pixel_addr, size_t pix_index, unsigned char *pix_stream, int *index, ud_img_pix_rgba *palette, ud_img_png_spix *transp_tab, unsigned char bit_depth, int mask, int *bit_shift)
{
	ud_img_pix_rgba *pixel = (ud_img_pix_rgba*)pixel_addr;

	(void)palette;
	unsigned short	red = ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth);
	ud_img_png_update_bit_shift(bit_shift, index, bit_depth);
	unsigned short	green = ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth);
	ud_img_png_update_bit_shift(bit_shift, index, bit_depth);
	unsigned short	blue = ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth);
	ud_img_png_update_bit_shift(bit_shift, index, bit_depth);
	if (!transp_tab)
	{
		pixel[pix_index].alpha = ud_img_png_convert_pixel(ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth), bit_depth);
		ud_img_png_update_bit_shift(bit_shift, index, bit_depth);
	}
	else
	{
		if (transp_tab->red == red && transp_tab->blue == blue && transp_tab->green == green)
			pixel[pix_index].alpha = 0;
		else
			pixel[pix_index].alpha = 255;
	}
	pixel[pix_index].red = ud_img_png_convert_pixel(red, bit_depth);
	pixel[pix_index].green = ud_img_png_convert_pixel(green, bit_depth);
	pixel[pix_index].blue = ud_img_png_convert_pixel(blue, bit_depth);
}

void			ud_img_png_assign_greyscale(void *pixel_addr, size_t pix_index, unsigned char *pix_stream, int *index, ud_img_pix_rgba *palette, ud_img_png_spix *transp_tab, unsigned char bit_depth, int mask, int *bit_shift)
{
	ud_img_pix_greyscale *pixel = (ud_img_pix_greyscale*)pixel_addr;

	(void)palette;
	(void)transp_tab;
	pixel[pix_index].greyscale = ud_img_png_convert_pixel(ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth), bit_depth);
	ud_img_png_update_bit_shift(bit_shift, index, bit_depth);
}

void			ud_img_png_assign_greyscalea(void *pixel_addr, size_t pix_index, unsigned char *pix_stream, int *index, ud_img_pix_rgba *palette, ud_img_png_spix *transp_tab, unsigned char bit_depth, int mask, int *bit_shift)
{
	ud_img_pix_greyscalea *pixel = (ud_img_pix_greyscalea*)pixel_addr;

	(void)palette;
	unsigned short greyscale = ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth);
	ud_img_png_update_bit_shift(bit_shift, index, bit_depth);
	if (!transp_tab)
	{
		pixel[pix_index].alpha = ud_img_png_convert_pixel(ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth), bit_depth);
		ud_img_png_update_bit_shift(bit_shift, index, bit_depth);
	}
	else
	{
		if (transp_tab->red == greyscale)
			pixel[pix_index].alpha = 0;
		else
			pixel[pix_index].alpha = 255;
	}
	pixel[pix_index].greyscale = ud_img_png_convert_pixel(greyscale, bit_depth);
}

void			ud_img_png_assign_palette(void *pixel_addr, size_t pix_index, unsigned char *pix_stream, int *index, ud_img_pix_rgba *palette, ud_img_png_spix *transp_tab, unsigned char bit_depth, int mask, int *bit_shift)
{
	(void)transp_tab;
	//int	ind_s = *index;
//	printf("% d \n", (int)bit_depth);
	ud_img_pix_rgba *pixel = (ud_img_pix_rgba*)pixel_addr;
	unsigned char	pal_index = ((pix_stream[*index] >> *bit_shift) & mask);
	pixel[pix_index].red = palette[pal_index].red;
	pixel[pix_index].green = palette[pal_index].green;
	pixel[pix_index].blue = palette[pal_index].blue;
	pixel[pix_index].alpha = palette[pal_index].alpha;
	ud_img_png_update_bit_shift(bit_shift, index, bit_depth);
	//if (*index != ind_s)
	//	printf("%02.2hhx ", pix_stream[*index]);
}

static void		(*ud_img_png_get_assign_function(ud_img_color_space color_space, ud_png_color_type png_color_type))(void *, size_t, unsigned char *, int *, ud_img_pix_rgba *, ud_img_png_spix *, unsigned char, int, int *)
{
	if (png_color_type == UD_PNG_PALETTE) return &ud_img_png_assign_palette;
	if (color_space == UD_CS_RGB) return &ud_img_png_assign_rgb;
	if (color_space == UD_CS_RGBA) return &ud_img_png_assign_rgba;
	if (color_space == UD_CS_GREYSCALE) return &ud_img_png_assign_greyscale;
	if (color_space == UD_CS_GREYSCALEA) return &ud_img_png_assign_greyscalea;
	return NULL;
}

unsigned char	ud_img_png_reverse_filtering(unsigned char *pix_stream, int index, unsigned char bit_depth, ud_img *img, ud_png_filter filter_type, int filter_index)
{
	if (filter_type == UD_PNG_F_NONE)
		return pix_stream[index];
	else if (filter_type == UD_PNG_F_SUB)
	{
		int bpp = (img->comp_nbr * bit_depth + 7) / 8;
		if (index - bpp <= filter_index)
			return pix_stream[index];
		return ((pix_stream[index] + pix_stream[index - bpp]) % 256);
	}
	else if (filter_type == UD_PNG_F_UP)
	{
		int prior = (img->comp_nbr * bit_depth + 7) / 8  * img->width + 1;
		if (index - prior < 1)
			return pix_stream[index];
		return ((pix_stream[index] + pix_stream[index - prior]) % 256);
	}
	else if (filter_type == UD_PNG_F_AVERAGE)
	{
		int bpp = (img->comp_nbr * bit_depth + 7) / 8;
		int prior = bpp * img->width + 1;
		int	average = 0;
		if (index - prior > 0) average += pix_stream[index - prior];
		if (index - bpp > filter_index) average += pix_stream[index - bpp];
		average >>= 1;
		return ((pix_stream[index] + average) % 256);
	}
	else if (filter_type == UD_PNG_F_PAETH)
	{
		int bpp = (img->comp_nbr * bit_depth + 7) / 8;
		int prior = bpp * img->width + 1;
		int	prior_bpp = prior + bpp;
		int	p, a, b, c, pa, pb, pc;
		a = index - bpp <= filter_index ? 0 : pix_stream[index - bpp];
		b = index - prior < 1 ? 0 : pix_stream[index - prior];
		c = index - bpp <= filter_index || index - prior < 1 ? 0 : pix_stream[index - prior_bpp];
		p = a + b - c;
		pa = p > a ?  p - a : a - p;
		pb = p > b ?  p - b : b - p;
		pc = p > c ?  p - c : c - p;
		if (pa <= pb && pa <= pc) return ((pix_stream[index] + a) % 256);
		else if (pb <= pc) return ((pix_stream[index] + b) % 256);
		return ((pix_stream[index] + c) % 256);
	}
	return pix_stream[index]; //default ?
}

ud_img		*ud_img_png_build_image(ud_png *png)
{
	ud_img	*img;
	unsigned char	*pix_stream = png->pix_stream;
	int				index = 0;
	ud_png_filter	filter_type;
	//if (png->color_type == UD_PNG_PALETTE)
	//	png->bit_depth = 8;
	unsigned char	bit_depth = png->bit_depth;
	size_t			pix_index = 0;
	ud_ut_prot_malloc(img = ud_ut_malloc(sizeof(ud_img)));
	ud_img_png_define_color_space(img, png);
	img->width = png->img_width;
	img->height = png->img_height;
	printf("bit_depth = %hhu\n", bit_depth);
	int		mask = ud_img_png_get_bit_depth_mask(bit_depth);
	void	(*assign_pix)(void *, size_t, unsigned char *, int *, ud_img_pix_rgba *, ud_img_png_spix *, unsigned char, int, int *) = ud_img_png_get_assign_function(img->color_space, png->color_type);
	int		bit_shift = 8 - bit_depth;
	int		filter_index;
	int		bit_depth_div = bit_depth == 16 ? 1 : 8 / bit_depth;
	size_t	scanline_size = png->color_type == UD_PNG_PALETTE ? img->width / bit_depth_div : png->chann_nbr * img->width * ((bit_depth + 7) / 8) / bit_depth_div;
	for (ud_ut_count row = 0; row < img->height; ++row)
	{
		printf("\nscanline[%03.3zu]: filter type: %03.3hhu stream : ", row, pix_stream[index]);
		filter_index = index++;
		filter_type = ud_img_png_get_filter_type(pix_stream[filter_index]);

		printf("%02.2hhx ", pix_stream[index]);
		//printf("f_type = %d\n", filter_type);
		for (ud_ut_count i = 0; i < scanline_size; ++i)
			pix_stream[index + i] = ud_img_png_reverse_filtering(pix_stream, index + i, bit_depth, img, filter_type, filter_index);
		for (ud_ut_count col = 0; col < img->width; ++col, ++pix_index)
		{
			/*if (bit_depth == 16)
			{
				//////////////
				printf("bit_depth = 16 => idk how to handle this\n");
				exit(0);
			}*/
			//if (png->color_type != UD_PNG_PALETTE)
			assign_pix(img->pixels->val, pix_index, pix_stream, &index, png->palette, png->transp_tab, bit_depth, mask, &bit_shift);
			//else
			//	ud_img_png_assign_palette(img->pixels->val, pix_index, pix_stream, &index, png->palette, bit_depth, mask, &bit_shift);
			/*for (ud_ut_count i = 0; i < img->comp_nbr; ++i)
			{
				
			}
			for (ud_ut_count i = 0; i < 8 && (size_t)col < png->img_width * img->comp_nbr; i += bit_depth, ++col)
			{
				printf("%03.3d ", ((pix_stream[index] >> (8 - i - bit_depth)) & mask));
			}*/
		}
		if (bit_depth != 16 && bit_shift != 8 - bit_depth)
		{
			bit_shift = 8 - bit_depth;
			index++;
		}
		//printf("\n");
	}
	ud_ut_prot_malloc(img->background = ud_ut_malloc(sizeof(ud_img_pix_rgb)));
	unsigned char bkgd_bit_depth = png->color_type == UD_PNG_PALETTE && png->bkgd_flag ? 8 : 16;
	img->background->red = (png->background.red * 255 / ((1 << bkgd_bit_depth) - 1));
	img->background->green = (png->background.green * 255 / ((1 << bkgd_bit_depth) - 1));
	img->background->blue = (png->background.blue * 255 / ((1 << bkgd_bit_depth) - 1));
		//;ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth);
	//img->background->green = ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth);
//	img->background->blue = ud_img_png_assign_pixel(pix_stream, index, mask, bit_shift, bit_depth);
	printf("%zu = %d ?\n", png->stream_index, index);
	return img;
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
	png.chann_nbr = ud_img_png_get_chann_nbr(png.color_type);
	ud_ut_prot_malloc(png.pix_stream = ud_ut_malloc(sizeof(unsigned char) * png.img_height * png.img_width * ((png.bit_depth + 7) / 8) * png.chann_nbr + png.img_height + 15555555));
	printf("%ld\n", png.img_height * png.img_width * ((png.bit_depth + 7) / 8) * png.chann_nbr + png.img_height);
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
	return ud_img_png_build_image(&png);
	//printf("pixel stream nb val: %zu\n", png.stream_index);
	/*for (ud_ut_count r = 0, int byte_i = 0; r < png.stream_index / 4 + png.img_height; ++r)
	{
		if (!(r % png.img_width) && ++r)
			printf("\nf= %02.2x ", png.pix_stream[byte_i++]);
		printf("%02.2x %02.2x %02.2x %02.2x ", (png.pix_stream[byte_i] >> 6), ((png.pix_stream[byte_i] >> 4) & 2),((png.pix_stream[r] >> 2) & 2),(png.pix_stream[r] & 2));
	}*/
	//return (NULL);
}

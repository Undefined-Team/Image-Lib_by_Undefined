#ifndef UD_IMAGE_H
# define UD_IMAGE_H

// Lib

#include <ud_file.h>
#include <ud_array.h>

// Macro

# define UD_M_1SQRT2			0.70710678118

# define UD_IMG_JPG_SOI			0xd8		// Start Of Image JPEG FILE EXTENSION

//	SOF_MARKERS

# define UD_IMG_JPG_SOF0		0xc0		// Start Of Frame (Baseline DCT, Huffman coding)
# define UD_IMG_JPG_SOF1		0xc1		// Start Of Frame (Extended sequential DCT, Huffman coding)
# define UD_IMG_JPG_SOF2		0xc2		// Start Of Frame (Progressive DCT, Huffman coding)
# define UD_IMG_JPG_SOF3		0xc3		// Start Of Frame (Lossless sequential, Huffman coding)
# define UD_IMG_JPG_SOF5		0xc5		// Start Of Frame (Differencial sequential DCT, Huffman coding)
# define UD_IMG_JPG_SOF6		0xc6		// Start Of Frame (Differencial progressive DCT, Huffman coding)
# define UD_IMG_JPG_SOF7		0xc7		// Start Of Frame (Differencial lossless sequential, Huffman coding)
# define UD_IMG_JPG_SOF9		0xc9		// Start Of Frame (Extended sequential DCT, arithmetic coding)
# define UD_IMG_JPG_SOF10		0xca		// Start Of Frame (Progressive DCT, arithmetic coding)
# define UD_IMG_JPG_SOF11		0xcb		// Start Of Frame (Lossless sequential, arithmetic coding)
# define UD_IMG_JPG_SOF13		0xcd		// Start Of Frame (Differencial sequential DCT, arithmetic coding)
# define UD_IMG_JPG_SOF14		0xce		// Start Of Frame (Differencial progressive DCT, arithmetic coding)
# define UD_IMG_JPG_SOF15		0xcf		// Start Of Frame (Differencial lossless sequential, arithmetic coding)

# define UD_IMG_JPG_DHT			0xc4		// Define Huffman Table(s)
# define UD_IMG_JPG_DQT			0xdb		// Define Quantization Table(s)
# define UD_IMG_JPG_DRI			0xdd		// Define Restart Interval
# define UD_IMG_JPG_SOS			0xda		// Start Of Scan
# define UD_IMG_JPG_RST_MIN		0xd0		// Restart n (From d0 to d7) (associed with DRI)
# define UD_IMG_JPG_RST_MAX		0xd7		// Restart n ...
# define UD_IMG_JPG_APP_MIN		0xe0		// Application specific n (From e0 to ef)
# define UD_IMG_JPG_APP_MAX		0xef		// Application specific n ...
# define UD_IMG_JPG_COM			0xfe		// Commment
# define UD_IMG_JPG_EOI			0xd9		// Endo Of Image

# define UD_IMG_JPG_MAX_HUFF_TABLE	4
# define UD_IMG_JPG_MAX_QUANT_MAT	4

# define UD_IMG_PNG_STATICHUFF_SIZE	572
//	SIGNATURES

# define UD_IMG_JPG_SIGN		0xffd8		// JPG Signature
# define UD_IMG_PNG_SIGN_1		0x89504e47	// PNG Signature Pt 1
# define UD_IMG_PNG_SIGN_2		0x0d0a1a0a	// PNG Signature Pt 2
# define UD_IMG_BM_SIGN			0x424d		// Bit Map Signature
# define UD_IMG_SVG_SIGN_1		0x3c3f786d	// SVG Signature Pt 1
# define UD_IMG_SVG_SIGN_2		0x6c		// SVG Signature Pt 2


# define ud_img_jpg_check_marker_start(unsigned_char)		unsigned_char == 0xff ? 1 : 0
# define ud_prot_overflow(val)								val > 255 ? 255 : val

// Enum
typedef enum				{UD_CS_RGB, UD_CS_RGBA, UD_CS_GREYSCALE, UD_CS_GREYSCALEA, UD_CS_YCBCR} ud_img_color_space;
typedef enum				{UD_IT_JPG, UD_IT_PNG, UD_IT_BM, UD_IT_SVG} ud_image_type;

// JPG Enum
typedef enum				{UD_DU_JPG_UNKNOWN, UD_DU_JPG_PBINCH, UD_DU_JPG_PBCM} ud_density_unit;
typedef enum				{UD_HC_DC, UD_HC_AC} ud_huff_class;

// PNG Enum
//typedef enum				{UD_CHUNK_IHDR, UD_CHUNK_PLTE, UD_CHUNK_IDAT, UD_CHUNK_IEND, UD_CHUNK_tRNS, UD_CHUNK_cHRM, UD_CHUNK_gAMA, UD_CHUNK_iCCP, UD_CHUNK_sBIT, UD_CHUNK_sRGB, UD_CHUNK_tEXt, UD_CHUNK_zTXt, UD_CHUNK_iTXt, UD_CHUNK_bKGD, UD_CHUNK_hIST, UD_CHUNK_pHYs, UD_CHUNK_sPLT, UD_CHUNK_tIME, UD_CHUNK_UNKN} ud_chunk_id;
typedef enum				{UD_PNG_GREYSCALE, UD_PNG_RGB, UD_PNG_PALETTE, UD_PNG_GREYSCALE_A, UD_PNG_RGB_A} ud_png_color_type;
typedef enum				{UD_PNG_RAW, UD_PNG_STATICHUFF, UD_PNG_SUPPHUFF, UD_PNG_RESERVED} ud_png_encoding;
typedef enum				{UD_PNG_F_NONE, UD_PNG_F_SUB, UD_PNG_F_UP, UD_PNG_F_AVERAGE, UD_PNG_F_PAETH} ud_png_filter;
// Structures

typedef struct			uds_img_pix_rgb
{
	unsigned char		red;
	unsigned char		green;
	unsigned char		blue;
}						ud_img_pix_rgb;

typedef struct			uds_img_pix_greyscale
{
	unsigned char		greyscale;
}						ud_img_pix_greyscale;

typedef struct			uds_img_pix_greyscalea
{
	unsigned char		greyscale;
	unsigned char		alpha;
}						ud_img_pix_greyscalea;

typedef struct			uds_img_png_spix
{
	unsigned short		red;
	unsigned short		green;
	unsigned short		blue;
}						ud_img_png_spix;

typedef struct			uds_img_pix_rgba
{
	unsigned char		red;
	unsigned char		green;
	unsigned char		blue;
	unsigned char		alpha;
}						ud_img_pix_rgba;

typedef struct			uds_img_pix_ycbcr
{
	unsigned char		luminance;
	unsigned char		chroma_blue;
	unsigned char		chroma_red;
}						ud_img_pix_ycbcr;

typedef struct			uds_mcu
{
	int					*val;
	int					nb;
}						ud_mcu;

typedef struct			uds_huff
{
	struct uds_huff		*right_1;
	struct uds_huff		*left_0;
	unsigned char		val;
	unsigned char		val_len;
}						ud_huff;

typedef struct			uds_png_huff
{
	struct uds_png_huff	*right_1;
	struct uds_png_huff	*left_0;
	unsigned short		val;
	unsigned short		tmp_huff_code;
	unsigned char		code_len;
}						ud_png_huff;

typedef struct			uds_jpg_comp
{
	ud_huff				*ac_table;
	ud_huff				*dc_table;
	ud_mcu				*mcu_lst;
	ud_mcu				*mcu_first;
	int					dc_prev;
	unsigned char		comp_id; 
	unsigned char		hor_sampling;	// 1, 2, 3 or 4
	unsigned char		ver_sampling;	// 1, 2, 3 or 4
	unsigned char		quant_mat_id;	// 0, 1, 2 or 3
	unsigned char		nbr_by_mcu;
}						ud_jpg_comp;

typedef struct			uds_jpg
{
	ud_huff				*ac_huff_tables[4];
	ud_huff				*dc_huff_tables[4];
	ud_arr				*quantization_mat;
	ud_jpg_comp			*components;
	ud_mcu				*mcu_lst;
	unsigned short		jfif_seg_len;
	int					mcu_nbr;
	ud_density_unit		density_unit;
	unsigned short		x_pixel_by_unit;
	unsigned short		y_pixel_by_unit;
	unsigned short		mcu_by_interval;
	unsigned short		img_height;			//in pixel
	unsigned short		img_width;			//in pixel
	unsigned short		mcu_height;			//in pixel
	unsigned short		mcu_width;			//in pixel
	unsigned char		data_precision;
	unsigned char		restart_interval;	// boolean
	unsigned char		comp_nbr;
}						ud_jpg;

/*
typedef struct			uds_png_chunk
{
	ud_chunk_id			id;
}						ud_png_chunk;
*/
typedef struct			uds_img_png_chroma
{
	float				white_p_x;
	float				white_p_y;
	float				red_x;
	float				red_y;
	float				green_x;
	float				green_y;
	float				blue_x;
	float				blue_y;
}						ud_img_png_chroma;

typedef struct			uds_png
{
	int					img_height;			//in pixel
	int					img_width;			//in pixel
	unsigned char		bit_depth;			//1, 2, 4, 8 or 16
	ud_png_color_type	color_type;			//0, 2, 3, 4 or 6
	unsigned char		compression_method;	//At present, png use only compression method 0 (deflate/inflate)
	unsigned char		filter_method;		//At present, png use only filter method 0 (with five basic filter types)
	unsigned char		interlace_method;	//0 (no interlace) or 1 (Adam7 interlace)
	unsigned char		palette_entries;	//from 1 to 256 (0 - 255)
	unsigned int		pix_by_unit_x;
	unsigned int		pix_by_unit_y;
	unsigned char		unit;				//0: unknown unit, 1: meter unit
	float				gamma;
	ud_img_pix_rgba		*palette;
	ud_img_png_chroma	*chroma;
	ud_img_png_spix		background;
	int					bkgd_flag; //1 or 0
	ud_img_png_spix		*transp_tab;
	ud_png_huff			*litlen_tree;
	ud_png_huff			*dist_tree;
	unsigned char		*pix_stream;
	size_t				chann_nbr;
	size_t				stream_index;
}						ud_png;

typedef struct			uds_img
{
	ud_arr				*pixels;
	ud_img_color_space	color_space;
	size_t				comp_nbr;
	size_t				width;
	size_t				height;
	ud_img_pix_rgb		*background;
}						ud_img;

// Prototypes

ud_img	*ud_img_parse_image(char *s);
ud_img	*ud_img_jpg_decryption(unsigned char *img);
ud_img	*ud_img_png_decryption(unsigned char *img);
void	ud_img_free_img(ud_img *img);
void	mlx_print_img(ud_img *img); // temporary function

#endif

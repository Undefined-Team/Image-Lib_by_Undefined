#ifndef UD_IMAGE_H
# define UD_IMAGE_H

// Lib

#include <ud_file.h>
#include <ud_array.h>

// Macro
# define UD_M_1SQRT2			0.70710678118
# define UD_M_PI				3.14159265359
# define UD_M_PI_16				0.19634954084
# define UD_IMG_JPG_SOI			0xd8		// Start Of Image JPEG FILE EXTENSION
# define UD_IMG_JPG_SOF_BD		0xc0		// Start Of Frame (Baseline DCT)
# define UD_IMG_JPG_SOF_PD		0xc2		// Start Of Frame (Progressive DCT)
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

# define UD_IMG_JPG_SIGN		0xffd8		// JPG Signature
# define UD_IMG_PNG_SIGN_1		0x89504e47	// PNG Signature Pt 1
# define UD_IMG_PNG_SIGN_2		0x0d0a1a0a	// PNG Signature Pt 2
# define UD_IMG_BM_SIGN			0x424d		// Bit Map Signature
# define UD_IMG_SVG_SIGN_1		0x3c3f786d	// SVG Signature Pt 1
# define UD_IMG_SVG_SIGN_2		0x6c		// SVG Signature Pt 2

# define ud_img_jpg_check_marker_start(unsigned_char)		unsigned_char == 0xff ? 1 : 0
//# define ud_round(float_val)								float_val - (int)float_val > 0.5 ? float_val + 1 : float_val
# define ud_round(float_val)								float_val
# define ud_prot_overflow(val)								val > 255 ? 255 : val
// Structures

typedef enum				{UD_IT_JPG, UD_IT_PNG, UD_IT_BM, UD_IT_SVG} ud_image_type;
typedef enum				{UD_DU_JPG_UNKNOWN, UD_DU_JPG_PBINCH, UD_DU_JPG_PBCM} ud_density_unit;
typedef enum				{UD_CS_RGB, UD_CS_YCBCR} ud_img_color_space;
typedef enum				{UD_HC_DC, UD_HC_AC} ud_huff_class;

/*typedef struct		uds_jfif
{
	
}					ud_jfif;
*/

typedef struct			uds_mcu
{
	int					***val; //need to be changed for opti to many allocation
	struct uds_mcu		*next;
	int	nb;
}						ud_mcu;

typedef struct			uds_huff
{
	struct uds_huff		*right_1;
	struct uds_huff		*left_0;
	unsigned char		val;
	unsigned char		val_len;
}						ud_huff;

typedef struct			uds_jpg_comp
{
	unsigned char		comp_id; // JPGEG DEFINIED FOR 0-255 BUT JFIF NORM ACTUALLY USE 1,2,3 ONLY
	unsigned char		hor_sampling;	// 1, 2, 3 or 4
	unsigned char		ver_sampling;	// same as hor
	unsigned char		quant_mat_id;	// 0, 1, 2 or 3
	ud_huff				*ac_table;
	ud_huff				*dc_table;
	ud_mcu				*mcu_lst;
	ud_mcu				*mcu_first;
	int					dc_prev;
}						ud_jpg_comp;

typedef struct			uds_jpg
{
	unsigned short		jfif_seg_len;
	ud_density_unit		density_unit;
	unsigned short		x_pixel_by_unit;
	unsigned short		y_pixel_by_unit;
	unsigned char		thumbnail_width;
	unsigned char		thumbnail_height;
	int					**thumbnail;
	ud_arr				*quantization_mat;
	//unsigned char		*lum_quantization_mat;
	//unsigned char		*chrom_quantization_mat;
	unsigned char		data_precision; // ???
	unsigned short		img_height; //in pixel
	unsigned short		img_width; //in pixel
	unsigned short		mcu_height; //in pixel
	unsigned short		mcu_width; //in pixel
	unsigned char		comp_nbr; //components nbr
	ud_jpg_comp			*components;
	ud_huff				*ac_huff_tables[4];
	ud_huff				*dc_huff_tables[4];
	ud_mcu				*mcu_lst;
}						ud_jpg;

typedef struct			uds_img
{
	ud_img_color_space	color_space;
	ud_arr				*pixels;
	size_t				width;
	size_t				height;
}						ud_img;

typedef struct			uds_img_pix_rgb
{
	unsigned char		red;
	unsigned char		green;
	unsigned char		blue;
}						ud_img_pix_rgb;

typedef struct			uds_img_pix_ycbcr
{
	unsigned char		luminance;
	unsigned char		chroma_blue;
	unsigned char		chroma_red;
}						ud_img_pix_ycbcr;

// Prototypes

void	ud_img_parse_image(char *s);
void	mlx_print_this_shit(ud_img *img);
ud_img		*ud_img_decryption_jpg_to_rgb(unsigned char *img); //a changer en ud arr *


#endif

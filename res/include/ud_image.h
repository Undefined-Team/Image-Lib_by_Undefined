#ifndef UD_IMAGE_H
# define UD_IMAGE_H

// Lib

#include <ud_file.h>
#include <ud_array.h>

// Macro

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

# define UD_IMG_JPG_SIGN		0xffd8ffe0	// JPG Signature
# define UD_IMG_PNG_SIGN_1		0x89504e47	// PNG Signature Pt 1
# define UD_IMG_PNG_SIGN_2		0x0d0a1a0a	// PNG Signature Pt 2
# define UD_IMG_BM_SIGN			0x424d		// Bit Map Signature
# define UD_IMG_SVG_SIGN_1		0x3c3f786d	// SVG Signature Pt 1
# define UD_IMG_SVG_SIGN_2		0x6c		// SVG Signature Pt 2

# define ud_img_jpg_check_marker_start(unsigned_char)		unsigned_char == 0xff ? 1 : 0

// Structures

typedef enum				{UD_IT_JPG, UD_IT_PNG, UD_IT_BM, UD_IT_SVG} ud_image_type;
typedef enum				{UD_DU_JPG_UNKNOWN, UD_DU_JPG_PBINCH, UD_DU_JPG_PBCM} ud_density_unit;

/*typedef struct		uds_jfif
{
	
}					ud_jfif;
*/

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
}						ud_jpg;

typedef struct		uds_img
{
	ud_image_type	image_type;
	
	
}					ud_img;
// Prototypes

void	ud_img_parse_image(char *s);

int		**ud_img_decryption_jpg_to_rgb(unsigned char *img); //a changer en ud arr *


#endif

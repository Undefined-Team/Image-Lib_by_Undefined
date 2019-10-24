#include "ud_image.h"

/*void	print_char(unsigned char *str)
{
	printf(" |");
	for (int i = 0 ; i < 16; ++i, ++str)
	{
		printf("%c", *str > ' ' && *str < 127 ? *str : '.');
		
	}
	printf("|\n");
}*/

int				ud_img_check_jpg_signature(unsigned char *img)
{
	if (*img++ != ((UD_IMG_JPG_SIGN >> 24) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_JPG_SIGN >> 16) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_JPG_SIGN >> 8) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_JPG_SIGN & 0xff))) return 0;
	return 1;
}
int				ud_img_check_png_signature(unsigned char *img)
{
	if (*img++ != ((UD_IMG_PNG_SIGN_1 >> 24) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_PNG_SIGN_1 >> 16) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_PNG_SIGN_1 >> 8) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_PNG_SIGN_1 & 0xff))) return 0;
	if (*img++ != ((UD_IMG_PNG_SIGN_2 >> 24) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_PNG_SIGN_2 >> 16) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_PNG_SIGN_2 >> 8) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_PNG_SIGN_2 & 0xff))) return 0;
	return 1;
}

int				ud_img_check_bm_signature(unsigned char *img)
{
	if (*img++ != ((UD_IMG_BM_SIGN >> 8) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_BM_SIGN & 0xff))) return 0;
	return 1;
}

int				ud_img_check_svg_signature(unsigned char *img)
{
	if (*img++ != ((UD_IMG_SVG_SIGN_1 >> 24) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_SVG_SIGN_1 >> 16) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_SVG_SIGN_1 >> 8) & 0xff)) return 0;
	if (*img++ != ((UD_IMG_SVG_SIGN_1 & 0xff))) return 0;
	if (*img++ != ((UD_IMG_SVG_SIGN_2 & 0xff))) return 0;
	return 1;
}

ud_image_type	ud_img_parse_signature(unsigned char *img)
{
	//ud_ut_count		i = 0;
	//unsigned char	*p_img = *img;

	if (ud_img_check_jpg_signature(img))
		return (UD_IT_JPG);
	if (ud_img_check_png_signature(img))
		return (UD_IT_PNG);
	if (ud_img_check_bm_signature(img))
		return (UD_IT_BM);
	if (ud_img_check_svg_signature(img))
		return (UD_IT_SVG);
	return (8);
}

void			ud_img_parse_image(char *img_path)
{
	char	*img = ud_file_read(img_path);
	//ud_arr	*i
	int		**rgb_img;
	ud_image_type	image_type = ud_img_parse_signature((unsigned char *)img);
	
	if (image_type == UD_IT_JPG)
		rgb_img = ud_img_decryption_jpg_to_rgb((unsigned char *)img);
	//rgbprintf("JPG \n");
	else if (image_type == UD_IT_PNG)
		printf("PNG \n");
	else if (image_type == UD_IT_BM)
		printf("BM \n");
	else if (image_type == UD_IT_SVG)
		printf("SVG \n");
	else
		printf("IDK \n");

	/*int		i = 0;


	unsigned char	*str = (unsigned char *)img;
	while (++i)
	{
		unsigned short	app2 = (((unsigned short)*str << 8) | (unsigned short)*(str + 1));
		if (app2 == 0xffe2)
		{
			printf("%02.2hhx\n\n\n", *str);
			break;
		}
		printf("%02.2hhx ", *str++);
		if (!(i %16))
			print_char(str - 16);
	}
	//printf("%hhx %hhx %hhx %hhx ", str[0], str[1], str[2], str[3]);

	//printf("%s\n", s);*/
	//return (0);
}

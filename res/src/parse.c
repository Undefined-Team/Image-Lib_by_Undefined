#include "ud_image.h"

int				ud_img_check_jpg_signature(unsigned char *img)
{
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

ud_img		*ud_img_parse_image(char *img_path)
{
	char	*img_str = ud_file_read(img_path);
	ud_img	*img = NULL;
	if (!img_str)
	{
		printf("%s is not a valid file\n", img_path);
		return (NULL);
	}
	ud_image_type	image_type = ud_img_parse_signature((unsigned char *)img_str);	
	if (image_type == UD_IT_JPG)
		img = ud_img_jpg_decryption((unsigned char *)img_str);
	// ---- to do ---- //
	else if (image_type == UD_IT_PNG)
		img = ud_img_png_decryption((unsigned char *)img_str);
		//printf("PNG FILE\n");
	else if (image_type == UD_IT_BM)
		printf("BM FILE\n");
	else if (image_type == UD_IT_SVG)
		printf("SVG FILE\n");
	else
		printf("%s is not a valid file\n", img_path);
	free(img_str);
	return (img);
}

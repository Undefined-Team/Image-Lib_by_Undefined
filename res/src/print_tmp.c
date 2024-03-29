#include "ud_image.h"
#include "../../unit test/mlx.h" //

void		mlx_print_img(ud_img *img)
{
	void	*mlx_ptr = mlx_init();
	size_t		win_w = img->width > 128 ? img->width : 128;
	size_t		win_h = img->height > 128 ? img->height : 128;
	int		img_pos_x = win_w / 2 - (img->width + 1) / 2;
	int		img_pos_y = win_h / 2 - (img->height + 1) / 2;
	void	*win_ptr = mlx_new_window(mlx_ptr, win_w, win_h, "");
	void	*img_ptr = mlx_new_image(mlx_ptr, img->width, img->height);
	void	*bkgd_ptr = mlx_new_image(mlx_ptr, win_w, win_h);
	int		bpp, s_l, endian;
	int		*img_str = (int *)mlx_get_data_addr(img_ptr, &bpp, &s_l, &endian);
	int		*bkgd_str = (int *)mlx_get_data_addr(bkgd_ptr, &bpp, &s_l, &endian);
	float	r, g, b, a;
	if (img->background)
		for (ud_ut_count i = 0; i < win_w; ++i)
			for (ud_ut_count j = 0; j < win_h; ++j, ++bkgd_str)
				*bkgd_str = (((int)(img->background->red) << 16) | ((int)(img->background->green) << 8) | ((int)img->background->blue));
	for (ud_ut_count i = 0; i < img->width * img->height; ++i, ++img_str)
	{
		if (img->color_space == UD_CS_YCBCR)
		{
			ud_img_pix_ycbcr	*pix = (ud_img_pix_ycbcr *)img->pixels->val;
			if (img->comp_nbr == 3)
			{
				r = (float)pix[i].luminance + 1.402 * (float)(pix[i].chroma_red - 128);
				g = (float)pix[i].luminance - 0.34414 * (float)(pix[i].chroma_blue - 128) - 0.71414 * (float)(pix[i].chroma_red - 128);
				b = (float)pix[i].luminance + 1.772 * (float)(pix[i].chroma_blue - 128);
				a = 0;
			}
			else if (img->comp_nbr == 1)
			{
				r = pix[i].luminance;
				g = pix[i].luminance;
				b = pix[i].luminance;
				a = 0;
			}
		}
		else if (img->color_space == UD_CS_RGB)
		{
			ud_img_pix_rgb	*pix = (ud_img_pix_rgb *)img->pixels->val;
			r = pix[i].red;
			g = pix[i].green;
			b = pix[i].blue;
			a = 0;
		}
		else if (img->color_space == UD_CS_GREYSCALE)
		{
			ud_img_pix_greyscale	*pix = (ud_img_pix_greyscale *)img->pixels->val;
			r = pix[i].greyscale;
			g = pix[i].greyscale;
			b = pix[i].greyscale;
			a = 0;
		}
		else if (img->color_space == UD_CS_GREYSCALEA)
		{
			ud_img_pix_greyscalea	*pix = (ud_img_pix_greyscalea *)img->pixels->val;
			r = pix[i].greyscale;
			g = pix[i].greyscale;
			b = pix[i].greyscale;
			a = 255 - pix[i].alpha;
		}
		else if (img->color_space == UD_CS_RGBA)
		{
			ud_img_pix_rgba	*pix = (ud_img_pix_rgba *)img->pixels->val;
			r = pix[i].red;
			g = pix[i].green;
			b = pix[i].blue;
			a = 255 - pix[i].alpha;
		}
		/*if (img->comp_nbr == 1)
		{
			r = pix[i].greyscale;//(float)pix[i].luminance + 1.402 * (float)(pix[i].chroma_red - 128);
			g = pix[i].greyscale;//(float)pix[i].luminance - 0.34414 * (float)(pix[i].chroma_blue - 128) - 0.71414 * (float)(pix[i].chroma_red - 128);
			b = pix[i].greyscale;//(float)pix[i].luminance + 1.772 * (float)(pix[i].chroma_blue - 128);
		}*/
	/*	
	*/	if (r < 0) r = 0;
		else if (r > 255) r = 255;
		if (g < 0) g = 0;
		else if (g > 255) g = 255;
		if (b < 0) b = 0;
		else if (b > 255) b = 255;
		if (a < 0) a = 0;
		else if (a > 255) a = 255;
		//printf("r %f g %f b %f a %f\n", r, g, b, a);
		*img_str = ((int)a << 24) | ((int)r << 16) | ((int)g << 8) | ((int)b);
	}
	if (img->background)
		mlx_put_image_to_window(mlx_ptr, win_ptr, bkgd_ptr, 0, 0);
	mlx_put_image_to_window(mlx_ptr, win_ptr, img_ptr, img_pos_x, img_pos_y);
	mlx_loop(mlx_ptr);
}

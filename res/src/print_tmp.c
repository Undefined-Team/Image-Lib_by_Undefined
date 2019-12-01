#include "ud_image.h"
#include "../../unit test/mlx.h" //

void		mlx_print_jpg(ud_img *img)
{
	void	*mlx_ptr = mlx_init();
	void	*win_ptr = mlx_new_window(mlx_ptr, img->width, img->height, "");
	void	*img_ptr = mlx_new_image(mlx_ptr, img->width, img->height);
	int		bpp, s_l, endian;
	int		*img_str = (int *)mlx_get_data_addr(img_ptr, &bpp, &s_l, &endian);
	float	r, g, b;
	ud_img_pix_ycbcr	*pix = (ud_img_pix_ycbcr *)img->pixels->val;
	for (ud_ut_count i = 0; i < img->width * img->height; ++i, ++pix, ++img_str)
	{
		if (img->comp_nbr == 3)
		{
			r = (float)pix->luminance + 1.402 * (float)(pix->chroma_red - 128);
			g = (float)pix->luminance - 0.34414 * (float)(pix->chroma_blue - 128) - 0.71414 * (float)(pix->chroma_red - 128);
			b = (float)pix->luminance + 1.772 * (float)(pix->chroma_blue - 128);
		}
		else if (img->comp_nbr == 1)
		{
			r = pix->luminance;
			g = pix->luminance;
			b = pix->luminance;
		}
		if (r < 0) r = 0;
		else if (r > 255) r = 255;
		if (g < 0) g = 0;
		else if (g > 255) g = 255;
		if (b < 0) b = 0;
		else if (b > 255) b = 255;
		*img_str = ((int)r << 16) | ((int)g << 8) | ((int)b);
	}
	mlx_put_image_to_window(mlx_ptr, win_ptr, img_ptr, 0, 0);
	mlx_loop(mlx_ptr);
}

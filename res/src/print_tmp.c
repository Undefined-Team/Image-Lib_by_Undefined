#include "ud_image.h"
# include "/Users/pduhard-/Desktop/fdf/minilibx_macos/mlx.h"

void		mlx_print_this_shit(ud_img *img)
{
	void	*mlx_ptr = mlx_init();
	void	*win_ptr = mlx_new_window(mlx_ptr, img->width, img->height, "ca");
	float	r, g, b;
	ud_img_pix_ycbcr	*pix = (ud_img_pix_ycbcr *)img->pixels->val;
	for (ud_ut_count i = 0; i < img->width * img->height; ++i)
	{
		if (img->comp_nbr == 3)
		{
			r = (float)pix->luminance + 1.402 * (float)(pix->chroma_red - 128);
			g = (float)pix->luminance - 0.34414 * (float)(pix->chroma_blue - 128) - 0.71414 * (float)(pix->chroma_red - 128);
			b = (float)pix->luminance + 1.772 * (float)(pix->chroma_blue - 128);
		}
		else if (img->comp_nbr == 1)
		{
			r = pix->luminance;// + 1.402 * (float)(pix->chroma_red - 128);
			g = pix->luminance;// - 0.34414 * (float)(pix->chroma_blue - 128) - 0.71414 * (float)(pix->chroma_red - 128);
			b = pix->luminance;
		}
		/*	
		float r = pix->luminance + (2 - 2 * 0.299) * (pix->chroma_red - 128) + 128;
		float b = pix->luminance + (2 - 2 * 0.114) * (pix->chroma_blue - 128) + 128;
		float g = (pix->luminance - 0.114 * b - 0.299 * r) / 0.587 + 128;
	*/	if (r < 0)
			r = 0;
		if (g < 0)
			g = 0;
		if (b < 0)
			b = 0;
		if (r > 255)
			r = 255;
		if (g > 255)
			g = 255;
		if (b > 255)
			b = 255;
		//if ((int)r == 255 && (int)g == 255 && (int)b == 255)
		printf("lum %d chroma_bue %d chroma_red %d r %d g %d b %d row %lu col %lu\n", pix->luminance, pix->chroma_blue, pix->chroma_red, (int)r, (int)g, (int)b, i % img->width, i / img->width);
		mlx_pixel_put(mlx_ptr, win_ptr, i % img->width, i / img->width, ((int)r << 16) | ((int)g << 8) | ((int)b));
		++pix;
	}
	mlx_loop(mlx_ptr);
}

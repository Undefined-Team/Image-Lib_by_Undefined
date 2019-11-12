#include "ud_image.h"
# include "/Users/pduhard-/Desktop/fdf/minilibx_macos/mlx.h"

void		mlx_print_this_shit(ud_img *img)
{
	printf("ferme la %zu\n", img->width);
	void	*mlx_ptr = mlx_init();
	void	*win_ptr = mlx_new_window(mlx_ptr, img->width, img->height, "ca");
	ud_img_pix_ycbcr	*pix = (ud_img_pix_ycbcr *)img->pixels->val;
	for (ud_ut_count i = 0; i < img->width * img->height; ++i)
	{
		float r = pix->luminance + 1.402 * (pix->chroma_red - 128);
		float g = pix->luminance - 3.4414 * (pix->chroma_blue - 128) - 0.71414 * (pix->chroma_red - 128);
		float b = pix->luminance + 1.772 * (pix->chroma_blue - 128);
		mlx_pixel_put(mlx_ptr, win_ptr, i % img->width, i / img->width, ((int)r << 24) + ((int)g << 16) + ((int)b << 8));
		++pix;
	}
	mlx_loop(mlx_ptr);
}

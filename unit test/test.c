#include "../res/include/ud_image.h"

int	ud_img_png_read_bits(unsigned char **img_str_addr, unsigned char *bit_pos, size_t nb_bits);

int main(int ac, char **av)
{
	unsigned char	*im = malloc(2);
	im[0]= 0b01010001;
	im[1]= 0b10101010;
	unsigned char	bit_pos = 7 ;
	printf("%d = 2?\n", ud_img_png_read_bits(&im, &bit_pos, 1));
	printf("%d = 2?\n", ud_img_png_read_bits(&im, &bit_pos, 4));
	printf("%d == %d\n", 0b100000000, 0b10000000 << 1 >> 1 << 1);
	if (ac == 1)
		return 0;
	ud_img *img = ud_img_parse_image(av[1]);
	if (img)
	{
		if (ac == 3 && !ud_str_cmp(av[2], "--print"))
			mlx_print_img(img);
		ud_img_free_img(img);
	}
	return 0;
}

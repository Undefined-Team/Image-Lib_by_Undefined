#include "../res/include/ud_image.h"

int main(int ac, char **av)
{
	if (ac == 1)
		return 0;
	ud_img *img = ud_img_parse_image(av[1]);
	if (img)
	{
		if (ac == 3 && !ud_str_cmp(av[2], "--print"))
			mlx_print_jpg(img);
		ud_img_free_img(img);
	}
	return 0;
}

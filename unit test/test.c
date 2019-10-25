#include "ud_image.h"

void	ud_img_parse_zig_zag(char **new_mat, unsigned char *img, size_t size_y, size_t size_x);

int main(int ac, char **av)
{
	if (ac)
		;
	ud_img_parse_image(av[1]);
/*	ud_img_parse_zig_zag(NULL, NULL, 10, 1);
	ud_img_parse_zig_zag(NULL, NULL, 3, 5);
	ud_img_parse_zig_zag(NULL, NULL, 5, 3);
	ud_img_parse_zig_zag(NULL, NULL, 2, 2);

	ud_img_parse_zig_zag(NULL, NULL, 8, 8);
	ud_img_parse_zig_zag(NULL, NULL, 16, 16);
*/	return 0;
}

#include "ud_image.h"

int main(int ac, char **av)
{
	if (ac)
		;
	ud_img_parse_image(av[1]);
	return 0;
}

#include "../res/include/ud_image.h"

void	ud_img_parse_zig_zag(char **new_mat, unsigned char *img, size_t size_y, size_t size_x);
void	ud_img_jpg_compute_idct(int a[8][8], int b[8][8]);

int main(int ac, char **av)
{
	if (ac)
		;
	short sh = 0b1111111100101111;
	printf("%hd == -208 ? \n", sh);
	ud_img_parse_image(av[1]);
	int a[8][8] =	{{-477, 24, 7, -25, -6, -28, -1, 7},
					{-66, -23, -5, 15, 16, -13, 12, 8},
					{8, -5, 14, 74, 4, 16, 13, -2},
					{45, -25, -25, -14, 3, 47, -34, 14},
					{-14, 23, 23, -31, -53, 22, -22, 20},
					{11, -33, -65, 40, 18, -11, 12, -3},
					{10, 7, 63, 9, 30, 17, 26, -23},
					{42, -31, -4, -36, 0, 29, 10, -27}};

	int c[8][8] =	{{1260, -1, -12, -5, 2, -2, -3, 1},
					{-23, -17, -6, -3, -3, 0, 0, -1},
					{-11, -9, -2, 2, 0, -1, -1, 0},
					{-7, -2, 0, 1, 1, 0, 0, 0},
					{-1, -1, 1, 2, 0, -1, 1, 1},
					{2, 0, 2, 0, -1, 1, 1, -1},
					{-1, 0, 0, -1, 0, 2, 1, -1},
					{-3, 2, -4, -2, 2, 1, -1, 0}};
	int	b[8][8];
	ud_img_jpg_compute_idct(c, b);
	for (int i = 0 ; i < 8 ; ++i)
	{
		for (int j = 0 ; j < 8 ; ++j)
		{
			printf("%-4d ", b[i][j]);
		}
		printf("\n");	
	}
	ud_img_jpg_compute_idct(a, b);
	for (int i = 0 ; i < 8 ; ++i)
	{
		for (int j = 0 ; j < 8 ; ++j)
		{
			printf("%-4d ", b[i][j]);
		}
		printf("\n");	
	}
/*	ud_img_parse_zig_zag(NULL, NULL, 10, 1);
	ud_img_parse_zig_zag(NULL, NULL, 3, 5);
	ud_img_parse_zig_zag(NULL, NULL, 5, 3);
	ud_img_parse_zig_zag(NULL, NULL, 2, 2);

	ud_img_parse_zig_zag(NULL, NULL, 8, 8);
	ud_img_parse_zig_zag(NULL, NULL, 16, 16);
*/	return 0;
}

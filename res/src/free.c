#include "ud_image.h"

void		ud_img_free_img(ud_img *img)
{
	free(img->pixels->val);
	free(img->pixels);
	free(img);
}

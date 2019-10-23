#include "ud_image.h"

void	ud_img_parse_image(char *img_path)
{
	ud_arr	*img = ud_file_read(img_path);

	
	char	*str = (char *)img->val;
	printf("%x %x %x %x ", str[0], str[1], str[2], str[3]);
	//printf("%s\n", s);
}

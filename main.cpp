#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <vector>
#include <direct.h>

int main(int argc, char** argv)
{
	// load the source image
	int w, h, c;
	std::vector<unsigned char> srcImage;
	{
		unsigned char* pixels = stbi_load("03ts001_peanutbsrm_cs-cropped.jpg", &w, &h, &c, 0);
		srcImage.resize(w * h * c);
		memcpy(srcImage.data(), pixels, w * h * c);
		stbi_image_free(pixels);
	}

	_mkdir("out");

	int ijkl = 0;
}
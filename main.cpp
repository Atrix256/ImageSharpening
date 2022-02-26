#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <vector>
#include <direct.h>

unsigned char ConvolvePixel(const std::vector<unsigned char>& src, int w, int h, int c, const std::vector<float>& kernel, int kernelw, int px, int py, int pc)
{
	int kernelh = int(kernel.size() / kernelw);
	int radiusx = kernelw / 2;
	int radiusy = kernelh / 2;

	float retf = 0.0f;
	float totalWeight = 0.0f;

	for (int iy = -radiusy; iy <= radiusy; ++iy)
	{
		int ready = (py + iy + h) % h;
		for (int ix = -radiusx; ix <= radiusx; ++ix)
		{
			int readx = (px + ix + w) % w;
			
			float pixelValue = float(src[(ready * w + readx) * c + pc]) / 255.0f;

			float kernelValue = kernel[(iy + radiusy) * kernelw + ix + radiusx];

			retf += pixelValue * kernelValue;
			totalWeight += kernelValue;
		}
	}

	retf /= totalWeight;

	return (unsigned char)(std::max(std::min(retf * 256.0f, 255.0f),0.0f));
}

std::vector<unsigned char> Convolve(const std::vector<unsigned char>& src, int w, int h, int c, const std::vector<float>& kernel, int kernelw)
{
	std::vector<unsigned char> ret(w * h * c);
	size_t outIndex = 0;
	for (int iy = 0; iy < h; ++iy)
	{
		for (int ix = 0; ix < w; ++ix)
		{
			for (int ic = 0; ic < c; ++ic)
			{
				ret[outIndex] = ConvolvePixel(src, w, h, c, kernel, kernelw, ix, iy, ic);
				outIndex++;
			}
		}
	}

	return ret;
}

std::vector<float> MakeHPFFromLPF(const std::vector<float>& lpf)
{
	std::vector<float> hpf(lpf.size());
	for (size_t i = 0; i < hpf.size(); ++i)
		hpf[i] = -lpf[i];
	hpf[hpf.size() / 2] += 1.0f;
	return hpf;
}

std::vector<float> MakeSharpenFromLPF(const std::vector<float>& lpf)
{
	std::vector<float> hpf(lpf.size());
	for (size_t i = 0; i < hpf.size(); ++i)
		hpf[i] = -lpf[i];
	hpf[hpf.size() / 2] += 2.0f;
	return hpf;
}

int main(int argc, char** argv)
{
	_mkdir("out");

	// load the source image
	int w, h, c;
	std::vector<unsigned char> src;
	{
		unsigned char* pixels = stbi_load("03ts001_peanutbsrm_cs-cropped.jpg", &w, &h, &c, 0);
		src.resize(w * h * c);
		memcpy(src.data(), pixels, w * h * c);
		stbi_image_free(pixels);
		stbi_write_png("out/src.png", w, h, c, src.data(), 0);
	}

	std::vector<float> boxLPF = {
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
	};

	// Gaussian kernels calculated from http://demofox.org/gauss.html
	std::vector<float> gaussianLPFSigma03 = {
		0.0023f,	0.0432f,	0.0023f,
		0.0432f,	0.8180f,	0.0432f,
		0.0023f,	0.0432f,	0.0023f,
	};

	std::vector<float> gaussianLPFSigma1 = {
		0.0000f,	0.0000f,	0.0000f,	0.0001f,	0.0001f,	0.0001f,	0.0000f,	0.0000f,	0.0000f,
		0.0000f,	0.0000f,	0.0004f,	0.0014f,	0.0023f,	0.0014f,	0.0004f,	0.0000f,	0.0000f,
		0.0000f,	0.0004f,	0.0037f,	0.0146f,	0.0232f,	0.0146f,	0.0037f,	0.0004f,	0.0000f,
		0.0001f,	0.0014f,	0.0146f,	0.0584f,	0.0926f,	0.0584f,	0.0146f,	0.0014f,	0.0001f,
		0.0001f,	0.0023f,	0.0232f,	0.0926f,	0.1466f,	0.0926f,	0.0232f,	0.0023f,	0.0001f,
		0.0001f,	0.0014f,	0.0146f,	0.0584f,	0.0926f,	0.0584f,	0.0146f,	0.0014f,	0.0001f,
		0.0000f,	0.0004f,	0.0037f,	0.0146f,	0.0232f,	0.0146f,	0.0037f,	0.0004f,	0.0000f,
		0.0000f,	0.0000f,	0.0004f,	0.0014f,	0.0023f,	0.0014f,	0.0004f,	0.0000f,	0.0000f,
		0.0000f,	0.0000f,	0.0000f,	0.0001f,	0.0001f,	0.0001f,	0.0000f,	0.0000f,	0.0000f
	};

	// large gaussian low pass filter
	{
		printf("LPF\n");
		std::vector<unsigned char> result = Convolve(src, w, h, c, gaussianLPFSigma1, 9);
		stbi_write_png("out/lpf.png", w, h, c, result.data(), 0);
	}

	// large gaussian high pass filter
	{
		printf("HPF\n");
		std::vector<unsigned char> result = Convolve(src, w, h, c, MakeHPFFromLPF(gaussianLPFSigma1), 9);
		stbi_write_png("out/hpf.png", w, h, c, result.data(), 0);
	}

	// large gaussian sharpen
	{
		printf("Sharpen\n");
		std::vector<unsigned char> result = Convolve(src, w, h, c, MakeSharpenFromLPF(gaussianLPFSigma1), 9);
		stbi_write_png("out/sharpen.png", w, h, c, result.data(), 0);
	}

	// box sharpen
	{
		printf("Box Sharpen\n");
		std::vector<unsigned char> result = Convolve(src, w, h, c, MakeSharpenFromLPF(gaussianLPFSigma03), 3);
		stbi_write_png("out/boxsharpen.png", w, h, c, result.data(), 0);
	}

	// gauss sharpen
	{
		printf("Gauss Sharpen\n");
		std::vector<unsigned char> result = Convolve(src, w, h, c, MakeSharpenFromLPF(boxLPF), 3);
		stbi_write_png("out/gausssharpen.png", w, h, c, result.data(), 0);
	}

	int ijkl = 0;

	return 0;
}
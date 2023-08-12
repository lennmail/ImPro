#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Image.h"
#include "stb_image.h"
#include "stb_image_write.h"

const int STEG_HEADER_SIZE = sizeof(uint32_t) * 8;

namespace ImPro {

	Image::Image(const char *filename) {

		if(read(filename)) {
			printf("Read file %s\n", filename);
			size = width * height * channels;
		}
		else {
			printf("Failed to read file %s\n", filename);
		}
	}

	Image::Image(int width, int height, int channels) : width(width), height(height), channels(channels) {

		size = static_cast<size_t>(width) * height * channels;
		data = new uint8_t[size];

	}

	Image::Image(const Image &img) : Image(img.width, img.height, img.channels) {
		memcpy(data, img.data, img.size);
	}

	Image::~Image() {
		stbi_image_free(data);
	}

	bool Image::read(const char *filename) {

		data = stbi_load(filename, &width, &height, &channels, 0);
		return data != nullptr;

	}

	bool Image::write(const char *filename) {

		ImageType type = getImageType(filename);
		int state = 0;

		switch(type) {
			case PNG:
				state = stbi_write_png(filename, width, height, channels, data, width * channels);
				break;
			case BMP:
				state = stbi_write_bmp(filename, width, height, channels, data);
				break;
			case JPG:
				state = stbi_write_jpg(filename, width, height, channels, data, 100);
				break;
			case TGA:
				state = stbi_write_tga(filename, width, height, channels, data);
				break;
		}

		return state;
	}

	ImageType Image::getImageType(const char *filename) {

		const char *ext = strrchr(filename, '.');

		if(ext != nullptr) {
			if(!strcmp(ext, ".png"))
				return PNG;
			else if(!strcmp(ext, ".jpg"))
				return JPG;
			else if(!strcmp(ext, ".bmp"))
				return BMP;
			else if(!strcmp(ext, ".tga"))
				return TGA;
		}
		return UNKNOWN;
	}

	Image &Image::grayscale() {
		/* Grayscale function based on averaging RGB values */
		if(channels < 3) {
			printf("Image %p has less than 3 channels. It will not be processed", data);
		}
		else {
			for(int i = 0; i < size; i += channels) {
				int g = (data[i] + data[i + 1] + data[i + 2]) / channels;
				memset(data + i, g, channels);
			}
		}
		return *this;
	}

	Image &Image::grayscale_luminescence() {
		/* Grayscale function that preserves luminescence, based on weighted linear combination of RGB values */
		if(channels < 3) {
			printf("Image %p has less than 3 channels. It will not be processed", data);
		}
		else {
			for(int i = 0; i < size; i += channels) {
				int g = 0.2126f * data[i] + 0.7152f * data[i + 1] + 0.0722f * data[i + 2];
				memset(data + i, g, 3);
			}
		}
		return *this;
	}

	Image &Image::color_mask(float red, float green, float blue) {

		if(channels < 3) {
			printf("Image %p has less than 3 channels. It will not be processed", data);
		}
		for(int i = 0; i < size; i += channels) {
			data[i] *= red;
			data[i + 1] *= green;
			data[i + 2] *= blue;
		}
		return *this;
	}

	Image &Image::encode_message(const char *message) {

		uint32_t len = strlen(message) * 8;

		if(len + STEG_HEADER_SIZE > size) {
			printf("\e[31m[ERROR] Message too large (%lu bits / %zu bits)\e[0m\n", len + STEG_HEADER_SIZE, size);
			return *this;
		}

		for(uint8_t i = 0; i < STEG_HEADER_SIZE; i++) {
			data[i] &= 0xFE;
			data[i] |= (len >> (STEG_HEADER_SIZE - 1 - i)) & 1UL;
		}

		for(uint32_t i = 0; i < len; i++) {
			data[i + STEG_HEADER_SIZE] &= 0xFE;
			data[i + STEG_HEADER_SIZE] |= (message[i / 8] >> ((len - 1 - i) % 8)) & 1;
		}
		return *this;
	}

	Image &Image::decode_message(char *buffer, size_t *messageLength) {

		uint32_t len = 0;

		for(uint8_t i = 0; i < STEG_HEADER_SIZE; i++) {
			len = (len << 1) | (data[i] & 1);
		}

		*messageLength = len / 8;

		for(uint32_t i = 0; i < len; i++) {
			buffer[i / 8] = (buffer[i / 8] << 1) | (data[i + STEG_HEADER_SIZE] & 1);
		}

		return *this;
	}

	Image &Image::diffmap(Image &img) {

		int cmp_width = fmin(width, img.width);
		int cmp_height = fmin(height, img.height);
		int cmp_channels = fmin(channels, img.channels);

		for(size_t i = 0; i < cmp_height; i++) {
			for(size_t j = 0; j < cmp_width; j++) {
				for(size_t k = 0; k < cmp_channels; k++) {
					data[(i * width + j) * channels + k] = BYTE_BOUND(abs(data[(i * width + j) * channels + k] - img.data[(i * width + j) * channels + k]));
				}
			}
		}
		return *this;
	}

	Image &Image::diffmap_scale(Image &img, uint8_t scale = 0) {

		int cmp_width = fmin(width, img.width);
		int cmp_height = fmin(height, img.height);
		int cmp_channels = fmin(channels, img.channels);

		uint8_t max = 0;

		for(size_t i = 0; i < cmp_height; i++) {
			for(size_t j = 0; j < cmp_width; j++) {
				for(size_t k = 0; k < cmp_channels; k++) {
					data[(i * width + j) * channels + k] = BYTE_BOUND(abs(data[(i * width + j) * channels + k] - img.data[(i * width + j) * channels + k]));
					max = fmax(max, data[(i * width + j) * channels + k]);
				}
			}
		}

		/*Expression rescales range between 0 and largest to range between 0 and 255.
		Inner fmax is to make sure no scaling over 255 is possible.
		Outer fmax is to prevent passing a 0 or a negative number.*/
		scale = 255 / fmax(1, fmax(scale, max));
		for(uint32_t i = 0; i < size; i++)
			data[i] *= scale;

		return *this;
	}

}


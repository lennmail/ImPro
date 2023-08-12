#include <cstdio>
#include <stdint.h>

extern const int STEG_HEADER_SIZE;

namespace ImPro {

	enum ImageType {
		PNG, JPG, BMP, TGA, UNKNOWN
	};

	struct Image {

		uint8_t *data = nullptr;
		size_t size = 0;

		int width = 0, height = 0, channels = 0;

		Image(const char *filename);
		Image(int width, int height, int channels);
		Image(const Image &img);
		~Image();

		bool read(const char *filename);
		bool write(const char *filename);

		ImageType getImageType(const char *filename);

		Image &grayscale();
		Image &grayscale_luminescence();

		Image &color_mask(float red, float green, float blue);

		Image &encode_message(const char *message);
		Image &decode_message(char *buffer, size_t *messageLength);

		Image &diffmap(Image &img);
		Image &diffmap_scale(Image &img, uint8_t scale);


	};

	inline int BYTE_BOUND(int val) {
		return val < 0 ? 0 : (val > 255 ? 255 : val);
	}

}

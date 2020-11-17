#include <gui/FontLoader.h>

#include <misc/Crash.h>

#include <lodepng/lodepng.h>
#include <stdio.h>
#include <stdlib.h>

void FontLoader_Init(Font* font, const char* filename) {
	uint32_t* image = NULL;
	unsigned int width = 255, height = 255;
	uint32_t error = lodepng_decode32_file((uint8_t**)&image, &width, &height, filename);
	if (error == 0 && image != NULL) {
		uint16_t* imgInLinRam = (uint16_t*)linearAlloc(width * height * sizeof(uint16_t));
		for (int i = 0; i < width * height; i++) {
			uint16_t r = (image[i] & 0xff) >> 3;
			uint16_t g = ((image[i] >> 8) & 0xff) >> 3;
			uint16_t b = ((image[i] >> 16) & 0xff) >> 3;
			uint16_t a = ((image[i] >> 24) & 0xff) >> 7;
			imgInLinRam[i] = (r << 11) | (g << 6) | (b << 1) | (a);
		}

		int c = 0;
		for (int y = 0; y < 128; y += 8) {
			for (int x = 0; x < 128; x += 8) {
				int length = 2;
				bool foundPixel = true;
				for (int i = 2; i < 8 && foundPixel; i++) {
					foundPixel = false;
					length++;
					for (int j = 0; j < 8; j++) foundPixel |= !!image[(y + j) * 128 + (x + i)];
				}

				font->fontWidth[c++] = length;
			}
		}

		GSPGPU_FlushDataCache(imgInLinRam, width * height * 4);
		free(image);

		C3D_TexInitVRAM(&font->texture, width, height, GPU_RGBA5551);

		// Hiermit zu konvertieren funktioniert irgendwie nicht richtig
		C3D_SyncDisplayTransfer(
		    (uint32_t*)imgInLinRam, GX_BUFFER_DIM(width, height), font->texture.data, GX_BUFFER_DIM(width, height),
		    (GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGB5A1) |
		     GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB5A1) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO)));

		linearFree(imgInLinRam);
	} else {
		Crash("Failed to load font %s\n", filename);
	}
}
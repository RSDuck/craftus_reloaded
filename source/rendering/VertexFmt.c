#include <rendering/VertexFmt.h>

#define HUE_DEGREE 512

int16_t RGB16_ToHSV(uint16_t rgb) {
	int16_t out_h, out_s, out_v;

	int r = SHADER_R(rgb);
	int g = SHADER_G(rgb);
	int b = SHADER_B(rgb);
	int m = MIN(r, MIN(g, b));
	int M = MAX(r, MAX(g, b));
	int delta = M - m;

	if (delta == 0) {
		/* Achromatic case (i.e. grayscale) */
		out_h = 0; /* undefined */
		out_s = 0;
	} else {
		int h;

		if (r == M)
			h = ((g - b) * 60 * HUE_DEGREE) / delta;
		else if (g == M)
			h = ((b - r) * 60 * HUE_DEGREE) / delta + 120 * HUE_DEGREE;
		else /*if(b == M)*/
			h = ((r - g) * 60 * HUE_DEGREE) / delta + 240 * HUE_DEGREE;

		if (h < 0) h += 360 * HUE_DEGREE;

		out_h = h;

		/* The constatnt 8 is tuned to statistically cause as little
		 * tolerated mismatches as possible in RGB -> HSV -> RGB conversion.
		 * (See the unit test at the bottom of this file.)
		 */
		out_s = (0x1f * delta - 8) / M;
	}
	out_v = M;

	return SHADER_RGB(out_h, out_s, out_v);
}

int16_t HSV16_ToRGB(int16_t hsv) {
	int16_t r, g, b;
	int16_t in_h = SHADER_R(hsv), in_s = SHADER_G(hsv), in_v = SHADER_B(hsv);

	if (in_s == 0) {
		r = g = b = in_v;
	} else {
		int h = in_h;
		int s = in_s;
		int v = in_v;
		int i = h / (60 * HUE_DEGREE);
		int p = (0x1f * v - s * v) / 0x1f;

		if (i & 1) {
			int q = (0x1f * 60 * HUE_DEGREE * v - h * s * v + 60 * HUE_DEGREE * s * v * i) / (0x1f * 60 * HUE_DEGREE);
			switch (i) {
				case 1:
					r = q;
					g = v;
					b = p;
					break;
				case 3:
					r = p;
					g = q;
					b = v;
					break;
				case 5:
					r = v;
					g = p;
					b = q;
					break;
			}
		} else {
			int t = (0x1f * 60 * HUE_DEGREE * v + h * s * v - 60 * HUE_DEGREE * s * v * (i + 1)) / (0x1f * 60 * HUE_DEGREE);
			switch (i) {
				case 0:
					r = v;
					g = t;
					b = p;
					break;
				case 2:
					r = p;
					g = v;
					b = t;
					break;
				case 4:
					r = t;
					g = p;
					b = v;
					break;
			}
		}
	}

	return SHADER_RGB(r, g, b);
}
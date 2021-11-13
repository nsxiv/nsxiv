#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <Imlib2.h>

/* macros */
#define ARRLEN(a) (sizeof(a) / sizeof((a)[0]))

/* globals */
static unsigned long palette[16] = {0};
static unsigned int palette_size = 0;
static unsigned int run_column = 0;
static unsigned int icon_sizes[16] = {0};
static unsigned int icon_sizes_size = 0;

/* functions */
static void error(int eval, int err, const char *fmt, ...)
{
	va_list ap;

	fflush(stdout);
	va_start(ap, fmt);
	if (fmt)
		vfprintf(stderr, fmt, ap);
	va_end(ap);
	if (err)
		fprintf(stderr, "%s%s", fmt ? ": " : "", strerror(err));
	fputc('\n', stderr);

	if (eval)
		exit(eval);
}

static unsigned long color_to_ulong(Imlib_Color color)
{
	return (color.alpha << 24 & 0xff000000) |
		(color.red   << 16 & 0x00ff0000) |
		(color.green <<  8 & 0x0000ff00) |
		(color.blue        & 0x000000ff);
}

static int to_palette(unsigned long color)
{
	unsigned int i;

	for (i = 0; i < palette_size; i++) {
		if (palette[i] == color)
			return i;
	}
	if (palette_size + 1 == 16)
		error(EXIT_FAILURE, 0, "Error: More than 16 colors in palette");
	palette[palette_size] = color;

	return palette_size++;
}

static void print_run(int color, unsigned int run_length)
{
	while (run_length > 0) {
		int x = run_length / 16 >= 1 ? 16 : run_length;

		printf("0x%02x, ", (x - 1) << 4 | color);
		run_length -= x;
		if (++run_column % 12 == 0)
			printf("\n\t");
	}
}

static void print_palette(void)
{
	unsigned int i;

	printf("static const unsigned long icon_colors[] = {\n\t");
	for (i = 0; i < palette_size; i++) {
		printf("0x%08lx, ", palette[i]);
		if (i % 4 == 3)
			printf("\n\t");
	}
	printf("\n};\n\n");
}

static void print_icon_array(void)
{
	unsigned int i;

	printf("static const icon_data_t icons[] = {\n");
	for (i = 0; i < icon_sizes_size; i++)
		printf("\tICON_(%d),\n", icon_sizes[i]);
	printf("};\n\n");
}

static unsigned int print_encoded_image(const char *path)
{
	Imlib_Image image;
	Imlib_Color color;
	unsigned int width, height, x, y;
	unsigned int run_length = 1;
	int currentcolor = 0;
	int lastcolor = -1;

	image = imlib_load_image(path);

	if (image == NULL)
		error(EXIT_FAILURE, 0, "Error loading image: %s", path);

	imlib_context_set_image(image);

	width = imlib_image_get_width();
	height = imlib_image_get_height();

	if (width != height)
		error(EXIT_FAILURE, 0, "Image is not square: %s", path);

	printf("static const unsigned char icon_data_%d[] = {\n\t", width);
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			imlib_image_query_pixel(x, y, &color);
			currentcolor = to_palette(color_to_ulong(color));
			if (currentcolor != lastcolor) {
				if (lastcolor != -1)
					print_run(lastcolor, run_length);
				run_length = 1;
			} else {
				run_length++;
			}
			lastcolor = currentcolor;
		}
	}
	print_run(lastcolor, run_length);
	printf("\n};\n\n");

	imlib_free_image();

	return width;
}

int main(int argc, char **argv)
{
	unsigned int img_size = 0;
	unsigned int i;

	if (argc < 2)
		error(EXIT_FAILURE, 0, "No icons provided");
	else if (argc-1 > ARRLEN(icon_sizes))
		error(EXIT_FAILURE, 0, "Too many icons");

	for (i = 1; i < argc; i++) {
		img_size = print_encoded_image(argv[i]);
		run_column = 0;
		icon_sizes[icon_sizes_size++] = img_size;
	}
	print_palette();
	print_icon_array();

	return EXIT_SUCCESS;
}

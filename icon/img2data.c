#include <stdio.h>
#include <stdlib.h>

#include <Imlib2.h>

void die(const char *message)
{
	fprintf(stderr, "%s\n", message);
	exit(EXIT_FAILURE);
}

void die_arg(const char *message, const char *arg)
{
	fprintf(stderr, "%s: %s\n", message, arg);
	exit(EXIT_FAILURE);
}

unsigned int color_to_uint(Imlib_Color color)
{
	return (color.alpha << 24 & 0xff000000) |
		(color.red   << 16 & 0x00ff0000) |
		(color.green <<  8 & 0x0000ff00) |
		(color.blue        & 0x000000ff);
}

unsigned int palette[16] = {0};
unsigned int palette_size = 0;

int to_palette(unsigned int color)
{
	for (unsigned int i = 0; i < palette_size; i++) {
		if (palette[i] == color)
			return i;
	}

	if (palette_size + 1 == 16)
		die("Error: More than 16 colors in palette");

	palette[palette_size] = color;
	return palette_size++;
}

unsigned int run_column = 0;

void print_run(int color, unsigned int run_length)
{
	while (run_length > 0) {
		int x = run_length / 16 >= 1 ? 16 : run_length;
		printf("0x%02x, ", (x - 1) << 4 | color);
		run_length -= x;

		if (++run_column % 12 == 0)
			printf("\n\t");
	}
}

void print_palette(void)
{
	unsigned int width = 0;

	printf("static const unsigned int icon_colors[] = {\n\t");
	for (unsigned int i = 0; i < palette_size; i++) {
		printf("0x%08x, ", palette[i]);
		if (++width % 4 == 0)
			printf("\n\t");
	}
	printf("\n};\n\n");
}

unsigned int icon_sizes[16] = {0};
unsigned int icon_sizes_size = 0;

void print_icon_array(void)
{
	printf("static const icon_data_t icons[] = {\n");
	for (unsigned int i = 0; i < icon_sizes_size; i++)
		printf("\tICON_(%d),\n", icon_sizes[i]);
	printf("};\n\n");
}

unsigned int print_encoded_image(const char *path)
{
	Imlib_Image image;

	unsigned int width = 0;
	unsigned int height = 0;

	image = imlib_load_image(path);

	if (!image)
		die_arg("Error loading image", path);

	imlib_context_set_image(image);

	width = imlib_image_get_width();
	height = imlib_image_get_height();

	if (width != height)
		die_arg("Image is not square", path);

	unsigned int run_length = 1;
	int currentcolor = 0;
	int lastcolor = -1;

	printf("static const unsigned char icon_data_%d[] = {\n\t", width);
	for (unsigned int y = 0; y < height; y++) {
		for (unsigned int x = 0; x < width; x++) {
			Imlib_Color color;
			imlib_image_query_pixel(x, y, &color);

			currentcolor = to_palette(color_to_uint(color));

			if (currentcolor != lastcolor) {
				if (lastcolor != -1)
					print_run(lastcolor, run_length);
				run_length = 1;
			}
			else {
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
	if (argc < 2)
		die("No icons provided");
	else if (argc > 1 + (sizeof(icon_sizes) / sizeof(icon_sizes[0])))
		die("Too many icons");

	unsigned int img_size = 0;

	for (int i = 1; i < argc; i++) {
		img_size = print_encoded_image(argv[i]);
		run_column = 0;

		icon_sizes[icon_sizes_size++] = img_size;
	}

	print_palette();

	print_icon_array();
}


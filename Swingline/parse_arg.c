#include "swingline.h"
#include "getopt.h"
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

void config_set_aspect_ratio(Config* c)
{
	if (c->width > c->height)
	{
		c->sx = 1;
		c->sy = (float)c->width / (float)c->height;
	}
	else
	{
		c->sx = (float)c->height / (float)c->width;
		c->sy = 1;
	}
}

void print_usage(char* prog)
{
	fprintf(stderr, "Usage: %s [-n samples] [-r radius] [-o output] "
		"[-i iterations] image\n", prog);
}

Config* parse_args(int argc, char** argv)
{
	unsigned n = 1000;
	float r = 0.01f;
	int iter = -1;
	const char* out = NULL;

	while (1)
	{
		char c = getopt(argc, argv, "r:n:o:i:");
		if (c == -1) { break; }

		switch (c)
		{
		case 'n':
			n = atoi(optarg);
			break;
		case 'i':
			iter = atoi(optarg);
			break;
		case 'o':
			out = optarg;
			break;
		case 'r':
			r = 0.01f * (float)atof(optarg);
			break;
		default:
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		};
	}

	if (optind >= argc)
	{
		fprintf(stderr, "%s: expected filename after options\n", argv[0]);
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}
	else if (n > UINT16_MAX)
	{
		fprintf(stderr, "Error: too many points (%i)\n", n);
		exit(-1);
	}

	int x, y;
	stbi_set_flip_vertically_on_load(1);
	stbi_uc* img = stbi_load(argv[optind], &x, &y, NULL, 1);

	if (img == NULL)
	{
		fprintf(stderr, "Error loading image: %s\n", stbi_failure_reason());
		exit(-1);
	}
	else if ((unsigned)x > UINT16_MAX || (unsigned)y > UINT16_MAX)
	{
		fprintf(stderr, "Error: image is too large (%i x %i)\n", x, y);
		exit(-1);
	}

	if (out)
	{
		size_t len = strlen(out);
		if (len >= 4 && strcmp(out + len - 4, ".svg"))
		{
			fprintf(stderr, "Error: output file should end in .svg (%s)\n", out);
			exit(-1);
		}
	}

	Config* c = (Config*)calloc(1, sizeof(Config));
	(*c) = (Config) {
		.img = img,
			.width = (uint16_t)x,
			.height = (uint16_t)y,
			.samples = (uint16_t)n,
			.resolution = 256,
			.radius = r,
			.iter = iter,
			.out = out
	};

	config_set_aspect_ratio(c);
	return c;
}

#pragma once

#include <stdint.h>

typedef struct Config_ {
	unsigned char* img;     /*  Pointer to raw image data  */

	uint16_t width, height; /*  Image size   */
	uint16_t samples;       /*  Number of Voronoi cells */
	uint16_t resolution;    /*  Resolution of Voronoi cones  */

	float sx, sy;           /*  Scale (used to adjust for aspect ratio) */
	float radius;           /*  Stipple radius (in arbitrary units)     */

	int iter;               /*  Number of iterations; -1 if interactive */
	const char* out;        /*  Output file name  */
} Config;

Config* parse_args(int argc, char** argv);

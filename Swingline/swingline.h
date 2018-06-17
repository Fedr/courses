#pragma once

#include <stdint.h>

typedef struct Config_ {
	unsigned char* img;     /*  Pointer to raw image data  */

	uint16_t width, height; /*  Image size   */
	uint16_t samples;       /*  Number of Voronoi cells */
	uint16_t resolution;    /*  Resolution of Voronoi cones  */

    int stipple_resolution; /*  Number of points on a cap half-circle of every stipple */
    float stipple_half_axis;/*  This controls stipple elongation, 1 for circular stipple */

	float sx, sy;           /*  Scale (used to adjust for aspect ratio) */
	float radius;           /*  Stipple radius (in arbitrary units)     */

	int iter;               /*  Number of iterations; -1 if interactive */
	const char* out;        /*  Output file name  */
    const char* in_points;  /*  Input file name with points */
    int show_voronoi;       /*  bool: whether to show Voronoi cells */
} Config;

Config* parse_args(int argc, char** argv);

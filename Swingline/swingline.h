#pragma once

#include <stdint.h>

typedef struct Config_ {
	unsigned char* img;     /*  Pointer to raw image data  */

	uint16_t width, height; /*  Image size   */
	uint16_t samples;       /*  Number of Voronoi cells */
	uint16_t resolution;    /*  Number of points in each half-side of a Voronoi cone  */

    int stipple_resolution; /*  Number of points on a cap half-circle of every stipple */
    float stipple_half_axis;/*  This controls stipple elongation, 0 for circular stipple */

	float sx, sy;           /*  Scale (used to adjust for aspect ratio) */
	float radius;           /*  Stipple radius (in arbitrary units)     */

	int iter;               /*  Number of iterations; -1 if interactive */
	const char* out;        /*  Output file name  */
    const char* in_points;  /*  Input file name with points */
    int show_voronoi;       /*  bool: whether to show Voronoi cells */
    int weighted_voronoi;   /*  bool: whether to give more weight to points in light cells */
} Config;

Config* parse_args(int argc, char** argv);

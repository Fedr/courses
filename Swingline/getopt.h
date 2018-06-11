#pragma once

extern char	*optarg;		// global argument pointer
extern int	optind;			// global argv index

int getopt(int argc, char *argv[], char *optstring);

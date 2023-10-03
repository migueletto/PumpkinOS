//
// optget.c - a simple version of getopt
//
// ld80 needs the GNU version of getopt which isn't readily available on OSX.
// It's argument parsing needs are so modest that there's no point in dragging
// in external code that is excessively fancy.

#include <stdio.h>
#include <string.h>

#include "ld80.h"

int optget_ind;
static int pos;

int optget(int argc, char **argv, char *options, char **arg)
{
	char *opt;

	// Initialize if argument index is 0.
	if (optget_ind == 0) {
		optget_ind = 1;
		pos = 0;
	}

	// Return done if we've gone through all the arguments.
	if (optget_ind >= argc)
		return -1;

	// At start of a command line word?
	if (pos == 0) {
		// Return the string if it isn't an argument.
		// That is, doesn't start with a dash or is only a dash.
		if (argv[optget_ind][0] != '-' || !argv[optget_ind][1]) {
			*arg = argv[optget_ind];
			optget_ind++;
			return 1;
		}
		// Otherwise, start looking at the argument characters.
		pos++;
	}

	// Look up in our list of options.  Return if unknown.
	opt = strchr(options, argv[optget_ind][pos]);
	if (!opt || *opt == ':') {
		fprintf(stderr, "Unknown option '%c'\n", argv[optget_ind][pos]);
		// Return done if called again.
		optget_ind = argc;
		return '?';
	}

	// Skip over the option character and move to the next word
	// if we're at the end of this one.
	pos++;
	if (!argv[optget_ind][pos]) {
		pos = 0;
		optget_ind++;
	}

	// If the option takes an argument then find it.
	if (opt[1] == ':') {
		// Return error if we don't have an argument for it.
		if (optget_ind >= argc)
			return '?';

		*arg = argv[optget_ind] + pos;

		// Move to the next word.
		pos = 0;
		optget_ind++;
	}

	// Finally, return the option we found.
	return *opt;
}

#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>
#include "alloc.h"
	#ifndef NO_MEMORY
		#define MALLOC(x) memmalloc(x)
		#define CALLOC(x, y) memcalloc(x, y)
		#define REALLOC(x, y) memrealloc(x, y)
		#define FREE(x) memfree(x)
	#else
		#define MALLOC(x) malloc(x)
		#define CALLOC(x, y) calloc(x, y)
		#define REALLOC(x, y) realloc(x, y)
		#define FREE(x) free(x)
	#endif

int total = 0; //total number of bytes allocated at the moment
int maxtotal = -1; //max value achieved by total

void * memmalloc(int x) {
	int * y = (int *) malloc(x + sizeof(int));
	*y = x;
	total += x;
	if (total > maxtotal) maxtotal = total;
	return (void *) (y + 1);
}

void * memcalloc(int x, int xx) {
	int * y = (int *) calloc(1, x*xx + sizeof(int));
	*y = x*xx;
	total += x*xx;
	if (total > maxtotal) maxtotal = total;
	return (void *) (y + 1);
}

void * memrealloc(void * z, int x) {
	int t, * y;
	if (z == NULL) return memmalloc(x);
	t = *(((int *) z)-1);
	y = (int *) realloc(((int *) z)-1, x + sizeof(int));
	*y = x;
	total += x-t;
	if (total > maxtotal) maxtotal = total;
	return (void *) (y + 1);
}

void memfree(void * z) {
	int t = *(((int *) z)-1);
	free(((int *) z)-1);
	total += -t;
	if (total > maxtotal) maxtotal = total;
}

int maxmemusage() {
	return maxtotal;
}

#endif

/*
**  Prototypes for bsearch()
**
**  by Charles Hannum
*/

void *bsearch(void *key, void *base, int n, size_t width,
	      int (*cmp)(const void*, const void*));

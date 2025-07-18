/*
**  A portable version of the ANSI standard bsearch() function
**
**  by Charles Hannum
*/

#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include "bsearch.h"

void *bsearch(void *key, void *base, int n, size_t width,
	      int (*cmp)(const void*, const void*))
{
  register void *middle;
  register int size = n;

  middle = base + (size / 2 * width);

  while (size)
    {
      int val = cmp(key, middle);

#ifdef DEBUG
      fprintf(stderr, "bsearch(): %c, %c, %d, %d, %d\n", *((char *)key),
	      *((char *)middle), size, n, val);
#endif

      if (val < 0)
	{
	  middle -= ((size /= 2) + 1) / 2 * width;
	}
      else if (val > 0)
	{
	  --size;
	  middle += ((size /= 2) + 2) / 2 * width;
	}
      else
	return(middle);
    }

    return(NULL);
}

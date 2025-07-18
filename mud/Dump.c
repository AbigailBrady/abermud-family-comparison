/*
**  Dump the uaf_rand file in a human-readable form
**
**  Another fine hack by Mycroft the Maintainer (Charles Hannum).
*/

#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fcntl.h>
#include "kernel.h"
extern int errno;

main (int argc, char **argv)
{
  int fd, od;
  PERSONA d;

  if (!--argc) {
    fprintf(stderr, "Usage:  Dump <infile> [outfile]\n");
    exit(1);
  }

  if ((fd = open(*++argv, O_RDONLY)) == -1) {
    perror("open(infile)");
    exit(errno);
  }

  if (--argc)
    if ((od = open(*++argv, O_WRONLY | O_CREAT, 0644)) == -1) {
      perror("open(outfile)");
      exit(errno);
    }

  printf("Name          Score       Strength  Sex     Level\n");
  printf("------------  ----------  --------  ------  -----\n");

  while (read(fd, &d, sizeof(d)) == sizeof(d))
  {
    if (ntohl (d.p_strength) == -1)
      continue;
    if (!ntohl (d.p_score))
      continue;

    printf ("%-12s  %10d  %8d  %-6s  %5d\n", d.p_name, ntohl(d.p_score),
	    ntohl(d.p_strength), (ntohl(d.p_sex)&1) ? "Female":"Male",
	    ntohl(d.p_level));

    if (od)
      if (write(od, &d, sizeof(d)) == -1) {
        perror("write()");
        exit(errno);
      }
  }
  close(od);
  close(fd);
  fflush(stdout);

  if (errno)
    perror("read()");

  exit(errno);
}

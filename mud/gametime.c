/*
**  Game time functions
*/

#include <time.h>
#include "kernel.h"
#include "bprintf.h"
#include "gametime.h"

static char sccsid[] = "@(#)gametime.c	4.100.0 (IthilMUD)	6/02/90";

/*
**  Returns the game time in seconds
**
**  by Mycroft the Maintainer (Charles Hannum)
*/

int gametime(void)
{
  FILE *fl;
  int rt;
  time_t ct;

  if ((fl = fopen(RESET_N, "r")) == NULL)
    return TIME_NEVER;
  fscanf(fl, "%d", &rt);
  fclose(fl);

  time(&ct);
  if (ct == -1)
    return TIME_UNAVAIL;

  /*  WARNING!!  The following code will break on a system where time_t is
      not an int.  */
  return ((int)ct - rt);
}

/*
**  Prints out the game time elapsed in English
**
**  by Edheler (Mike Barthelemy)
*/

void eltime()
{
  int et, i;

  switch (et = gametime())
  {
    case TIME_NEVER:
      bprintf("AberMUD has yet to ever start!");
      break;
    case TIME_UNAVAIL:
      bprintf("Current time is unavailable for computation!");
      break;
    default:
      if (et > 24 * 3600)
        bprintf("Over a day!");
      else {
        if (et >= 3600) {
          i = et / 3600;
          bprintf("%d hour%s", i, (i == 1) ? "" : "s");
          if (et -= (i * 3600))
            bprintf(", ");
	}
	if (et >= 60) {
          i = et / 60;
          bprintf("%d minute%s", i, (i == 1) ? "" : "s");
          if (et -= (i * 60))
	    bprintf(", ");
	}
        if (et)
          bprintf("%d second%s", et, (et == 1) ? "" : "s");
      }
      break;
  }
  bprintf("\n");
}

/*
**  Some basic functions.
*/

#include <strings.h>
#include <stdarg.h>
#include <time.h>
#include "kernel.h"
#include "macros.h"
#include "pflags.h"
#include "oflags.h"
#include "objects.h"
#include "support.h"
#include "blib.h"
#include "tk.h"
#include "mud.h"

#define CHAR1	0x00FF
#define CHAR2	0xFF00

static void
_addtolog (char *file, char *format, va_list args)
{
  time_t log_time;
  FILE *log_file;
  char *time_string;

  (void) time(&log_time);
  time_string = ctime(&log_time);
  *index(time_string, '\n') = 0;
  if ((log_file = openlock(file, "a")) == NULL)
    {
      loseme();
      crapup("Log fault : Access Failure");
    }
  fprintf(log_file, "%s: ", time_string);
  _doprnt(format, args, log_file);
  fprintf(log_file, "\n");
  closelock(log_file);
}

void
addtolog (char *file, char *format, ...)
{
  va_list args;

  va_start(args, format);
  _addtolog (file, format, args);
  va_end(args);
}

void
mudlog (char *format, ...)
{
  va_list args;

  if (spawned)				/* Don't record normal log entries */
    return;				/* if spawned.			   */
  va_start(args, format);
  _addtolog (LOG_FILE, format, args);
  va_end(args);
}

void
byte_put (int *x, int y, int z)
{
  if (y == 1)
    {
      *x &= ~CHAR1;
      *x |= z;
    }
  else
    {
      *x &= ~CHAR2;
      *x |= ((z << 8) & CHAR2);
    }
}

int
byte_fetch(int x, int y)
{
    return (y == 1 ? x : x >> 8) & CHAR1;
}

int
bit_fetch(int x, int y)
{
    return (x >> y) & 1;
}

void
bit_set(int *x, int y)
{
    *x |= (1 << y);
}

void
bit_clear(int *x, int y)
{
    *x &= ~(1 << y);
}

void
setoloc(int ob, int l, int c)
{
    oloc(ob) = l;
    ocarrf(ob) = c;
}

int
ohany(int mask)
{
    int a;

    for (a = 0; a < numobs; a++)
	if ((iscarrby(a, mynum) || ishere(a)) && (oflags(a) & mask))
	    return 1;
    return 0;
}

void
setpsex(int chr, int v)
{
    if (v)
	psetflg(chr, pfl(Female));
    else
	pclrflg(chr, pfl(Female));
}

int
ptothlp(int pl)
{
    int ct;

    for (ct = 0; ct < MAX_USERS; ct++) {
	if (ploc(ct) != ploc(pl) || phelping(ct) != pl)
	    continue;
	return ct;
    }
    return -1;
}

int
willhold(int x, int y)
{
    int a, b;

    for (a = 0, b = 0; a < numobs; a++)
	if (iscontin(a, x))
	    b += osize(a);
    b += osize(y);
    return b > osize(x) ? 0 : 1;
}

void
setpfighting(int x, int y)
{
    int ct;

    ublock[16 * x + 14] = y;
    if (y == -1) {
	for (ct = 0; ct < MAX_CHARS; ct++)
	    if (pfighting(ct) == x)
		ublock[16 * ct + 14] = -1;
    }
    else
	ublock[16 * y + 14] = x;
}

/* Can't be macros because of multiple evaluations */

int
otstmask(int ob, int v)
{
    v <<= 16;
    return (oflags(ob) & v) == v;
}

int
isavl(int ob)
{
    return ishere(ob) ? 1 : iscarrby(ob, mynum);
}

int
ospare(int ob)
{
    return otstbit(ob, ofl(Destroyed)) ? -1 : 0;
}

char *
oname(int ob)
{
    return objects[ob].o_name;
}

char *
pname(int chr)
{
    return (char *)&ublock[16 * chr];
}

/*
** Zone based name generator
*/

#include <strings.h>
#include "kernel.h"
#include "macros.h"
#include "zones.h"
#include "loctable.h"
#include "exec.h"

int
findzone (int x, char *str)
{
  int a, b, c;

  x = -x;
  if (x <= 0) {
    strcpy(str, "TCHAN");
    return 0;
  }
  for (a = 0, b = 0; a < x; b++) {
    strcpy(str, zoname[b].z_name);
    c = a;
    a = zoname[b].z_loc;
  }
  return x - c;
}

void
exits()
{
  int a, v, newch;
  int drnum, droff;
  int b = 0;
  char st[64];

  if (isdark()) {
    bprintf("It is dark.....\n");
    return;
  }
  bprintf("Obvious exits are:\n");
  for (a = 0; a < 6; a++) {
    newch = getexit(ploc(mynum), a);
    if (newch > 999 && newch < 2000) { /* look through special exits */
      drnum = newch - 1000;
      droff = drnum ^ 1;	/* other side */
      if (!state(drnum))
	newch = oloc(droff);
    }
    if (newch >= 0)
      continue;
    if (plev(mynum) < LVL_WIZARD)
      bprintf("%s : %s\n", dirns[a], sdesc(newch));
    else {
      v = findzone(newch, st);
      bprintf("%s : %-45s : %s%d\n", dirns[a], sdesc(newch), st, v);
    }
    b = 1;
  }
  if (b == 0)
    bprintf("None....\n");
}

int
getroomnum()
{
  int a, lr, off;
  char w[80], *s1;
  ZONE *p;

  /*  Get room name  */
  if (brkword() == -1)
    return(0);
  /*  Maybe it's a player name?  */
  strcpy(w, wordbuf);
  if ((a = fpbn(w)) != -1)
    return ploc(a);
  /*  Get room offest  */
  off = 1;
  s1 = w;
  while (1) {
    if (!*s1) {
      if (brkword() != -1)
	off = atoi(wordbuf);
      break;
    } else if (isdigit(*s1) || *s1 == '-') {
      off = abs(atoi(s1));
      *s1 = '\0';
      break;
    } else
      s1++;
  }
  /*  Search zones list  */
  for (lr = 0, a = 0, p = zoname; a < numzon; a++, p++) {
    if (strncasecmp(w, p->z_name, strlen(w)) == 0)
      return(-(lr + off));
    else
      lr = p->z_loc;
  }
  return(0);
}

void
showname(int loc)
{
  char a[64];
  int b;

  if (plev(mynum) < LVL_WIZARD)
    bprintf("%s\n",sdesc(loc));
  else {
    b = findzone(loc, a);
    bprintf("%s%d", a, b);
    if (plev(mynum) >= LVL_ARCHWIZARD)
      bprintf("[ %d ]", loc);
    bprintf("\n");
  }
}

void
loccom()
{
  int a;

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  bprintf("\nKnown Zones Are\n\n");
  a = 0;
  for (a = 0; a < numzon; a++) {
    bprintf("%-20s", zoname[a].z_name);
    if (a % 4 == 3)
      bprintf("\n");
  }
  bprintf("\n");
}

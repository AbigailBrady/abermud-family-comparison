/*
**  Show/edit flags.
*/

#include "kernel.h"
#include "macros.h"
#include "pflags.h"
#include "pflagnames.h"
#include "lflags.h"
#include "lflagnames.h"
#include "flags.h"
#include "exec.h"
#include "support.h"

void
show_priv (int pl1)
{
  int ct;
  int c2 = 0;

  for (ct = 0; ct < NUM_PFLAGS; ct++) {
    if (_ptstflg(pl1, ct)) {
      if (!(c2 % 5))
	bprintf("\n");
      bprintf("%-16s", Pflags[ct]);
      c2++;
    }
  }
  if (!c2)
    bprintf("\nNone!\n");
  else
    bprintf("\n");
}

void
show_lflags()
{
  int ct;
  int c2 = 0;

  if (plev(mynum) < LVL_WIZARD) {
    bprintf("Pardon?\n");
    return;
  }
  bprintf("LFLAGS:");
  for (ct = 0; ct < NUM_LFLAGS; ct++) {
    if (_ltstflg(ploc(mynum), ct)) {
      if (!(c2 % 5))
	bprintf("\n");
      bprintf("%-16s", Lflags[ct]);
      c2++;
    }
  }
  if (!c2)
    bprintf("\nNone!\n");
  else
    bprintf("\n");
}

void
pflagcom ()
{
  int a, b, c;

  if (brkword() == -1) {
    bprintf("Whose Pflags?\n");
    return;
  }
  if ((a = fpbn(wordbuf)) == -1) {
    bprintf("Who's that?\n");
    return;
  }
  if (brkword() == -1) {
    show_priv (a);
    return;
  }
  if (!ptstflg(mynum, pfl(PflagEdit)) && plev(mynum) < LVL_GOD) {
    erreval();
    return;
  }
  b = atoi(wordbuf);
  if (brkword() == -1) {
    bprintf("%s is %s\n", Pflags[b], _ptstflg(a, b) ? "on" : "off");
    return;
  }
  c = atoi(wordbuf);
  if (b < 0 || b >= NUM_PFLAGS) {
    bprintf("\001A\033[31m\377PFLAG must be between 0 and %d.\001A\033[0m\377\n",
	    NUM_PFLAGS - 1);
    return;
  }
  if (c < 0 || c > 1) {
    bprintf("\001A\033[31m\377Set flag to 0 or 1.\001A\033[0m\377\n");
    return;
  }
  mudlog("PFLAG: %s %s for %s by %s", Pflags[b], c ? "ON" : "OFF",
	 pname(a), pname(mynum));
  if (c)
    _psetflg(a, b);
  else
    _pclrflg(a, b);
}

void
setcom ()
{
  int a, b, c;

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
#ifdef BIG_BROTHER
  mudlog("WIZCOM: %s %s", pname(mynum), wordbuf);
#endif
  if (brkword() == -1) {
    bprintf("Set what?\n");
    return;
  }
  if ((a = fobn(wordbuf)) == -1) {
    if ((a = fpbn(wordbuf)) == -1)
      bprintf("Set what?\n");
    else if (a < MAX_USERS)
      bprintf("Mobiles only.\n");
    else if (brkword() == -1)
      bprintf("To what value?\n");
    else {
      if (*wordbuf < '0' || *wordbuf > '9')
	bprintf("Stamina should be a number.\n");
      else {
	b = atoi(wordbuf);
	setpstr(a, b);
      }
    }
    return;
  }
  if (brkword() == -1) {
    bprintf("Set to what value?\n");
    return;
  }
  if (EQ(wordbuf, "bit")) {
    if (brkword() == -1) {
      bprintf("Which bit?\n");
      return;
    }
    b = atoi(wordbuf);
    if (brkword() == -1) {
      bprintf("This bit is %s\n", _otstbit(a, b) ? "TRUE" : "FALSE");
      return;
    }
    c = atoi(wordbuf);
    if (c < 0 || c > 1 || b < 0 || b > 31) {
      bprintf("Number out of range.\n");
      return;
    }
    if (c == 0)
      _oclrbit(a, b);
    else
      _osetbit(a, b);
    return;
  }
  if (EQ(wordbuf, "byte")) {
    if (brkword() == -1) {
      bprintf("Which byte?\n");
      return;
    }
    b = atoi(wordbuf);
    if (brkword() == -1) {
      bprintf("Current Value is : %d\n", obyte(a, b));
      return;
    }
    c = atoi(wordbuf);
    if (b < 0 || b > 1 || c < 0 || c > 255) {
      bprintf("Number out of range.\n");
      return;
    }
    osetbyte(a, b, c);
    return;
  }
  if (!isdigit(*wordbuf)) {
    bprintf("I don't know how to change that.\n");
    return;
  }
  if ((b = atoi(wordbuf)) > omaxstate(a)) {
    bprintf("Sorry, max state for that is %d.\n", omaxstate(a));
    return;
  }
  if (b < 0)
    bprintf("States start at 0.\n");
  else
    setobjstate(a, b);
}

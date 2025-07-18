/*
**  The other stuff in here handles time, and also game timelocks.
*/

#include <fcntl.h>
#include <sys/time.h>
#include <strings.h>
#include "kernel.h"
#include "actions.h"
#include "macros.h"
#include "oflags.h"
#include "oflagnames.h"
#include "lflags.h"
#include "locdir.h"
#include "blib.h"
#include "support.h"
#include "exec.h"
#include "tk.h"
#include "parse.h"
#include "new2.h"
#include "extra.h"
#include "objsys.h"
#include "gametime.h"
#include "pflags.h"

static char sccsid[] = "@(#)locdir.c	4.100.0 (IthilMUD)	6/02/90";

static char folnam[16];
static int trace_carrf;
static int i_folcode = -1;
static int trace_class = -1;
static int trace_item = -1;
static int trace_loc;

void showplayer()
{
  int b;

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  if ((b = fpbn(item1)) == -1) {
/*    bprintf("Who's that?\n");    SEE showplayer_not_in_game  - BMR */
      showplayer_not_in_game();
	  return;
  }
  else {
    bprintf("Name       : %s\n", pname(b));
    bprintf("Title      : ");
    displevel(plev(b), psex(b), "<name>");
    bprintf("\nLevel      : %d\n", plev(b));
    bprintf("Visibility : %d\n", pvis(b));
    if (b < MAX_USERS)
      bprintf("Strength   : %d / %d\n", pstr(b), maxstrength(b));
    else
      bprintf("Strength   : %d\n", pstr(b));
    if (b > MAX_USERS)
      bprintf("Damage     : %d\n", damof(b));
    bprintf("Sex        : %s\n", (psex(b) == 0) ? "Male" : "Female");
    bprintf("Score      : %d\n", pscore(b));
    bprintf("Location   : ");
    showname(ploc(b));
    bprintf("Condition  :\n");
    if (phelping(b) != -1)
      bprintf("    Helping %s\n", pname(phelping(b)));
    if (pfighting(b) != -1)
      bprintf("    Fighting %s\n", pname(pfighting(b)), pfighting(b));
    if (psitting(b))
      bprintf("    Sitting\n");
    bprintf("\n");
  }
}

/* 
** I want to be able to get certain stats on a person even when they aren't
** in the game.  -BMR
*/
void showplayer_not_in_game()
{
  PERSONA b;

  if (findpers(item1, &b) == 0) {
	bprintf("No such persona in system.\n");
	return;
  }
  bprintf("Name       : %s\n", b.p_name);
  bprintf("Level      : %d\n", b.p_level);
  bprintf("Strength   : %d\n", b.p_strength);
  bprintf("Score      : %d\n", b.p_score);
  bprintf("Location   : NOT IN GAME\n");
}

void showlocation(int o)
{
  int uc, fl;

  if (otstbit(o, ofl(Unlocatable)) && (plev(mynum) < LVL_GOD))
  {
    bprintf(" Unknown\n");
    return;
  }

  uc = 1;
  fl = 0;
  while (!fl) {
    switch (ocarrf(o)) {
    case IN_ROOM:
      o = oloc(o);
      fl = 1;
      break;
    case CARRIED_BY:
      bprintf(" %carried by %s", (uc ? 'C' : 'c'), pname(oloc(o)));
      uc = 0;
      o = ploc(oloc(o));
      fl = 1;
      break;
    case WORN_BY:
      bprintf(" %corn by %s", (uc ? 'W' : 'w'), pname(oloc(o)));
      uc = 0;
      o = ploc(oloc(o));
      fl = 1;
      break;
    case CONTAINED_IN:
      bprintf(" %cnside the %s", (uc ? 'I' : 'i'), oname(oloc(o)));
      uc = 0;
      o = oloc(o);
      break;
    default:
      bprintf("\n");
      return;
    }
    if (plev(mynum) < LVL_WIZARD)
      if (!uc) {
        bprintf("\n");
        return;
      }
      else
        break;
  }
  bprintf(" %cn ", (uc ? 'I' : 'i'));
  showname(o);
}

void showitem()
{
  int x, i;

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  if (EMPTY(item1)) {
    bprintf("Show what?\n");
    return;
  }
  if ((x = fobn(item1)) == -1) {
    bprintf("What's that?\n");
    return;
  }
  bprintf("\nItem:     %s", oname(x));
  if (!EQ(oaltname(x), "<null>"))
    bprintf(" (%s)", oaltname(x));
  bprintf("\nLocation:");
  showlocation(x);
  bprintf("\nState: %d\tMax State: %d\t\tFlannel: %d\n",
	  state(x), omaxstate(x), oflannel(x));
  bprintf("Damage: %d\tArmor Class: %d\t\tSize: %d\n",
	  obyte(x, 0), obyte(x, 1), osize(x));
  bprintf("Base Value: %-4d\t\tCurrent Value: %d\n\n",
	  obaseval(x), ovalue(x));
  bprintf("Properties: ");
  for (i = 0; i < 32; i++)
    pdobit(x, i);
  bprintf("\nDescriptions:\n%s\n%s\n%s\n%s\n",
	  olongt(x, 0), olongt(x, 1), olongt(x, 2), olongt(x, 3));
}

void pdobit(int o, int x)
{
  static int ct = 0;

  if (x == 0)
    ct = 0;
  if (_otstbit(o, x)) {
    ct += strlen(Oflags[x]) + 1;
    if (ct > 79) {
      bprintf("\n");
      ct = 0;
    }
    bprintf("%s ", Oflags[x]);
  }
}

void wizlock()
{
  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  if (shimge[1] == 127) {
    bprintf("Sorry, the game is already ARCH-WIZARD locked.\n");
    return;
  }
  shimge[1] = 12;
  broad("** SYSTEM : The Game is now WIZARD locked.\007\007\n");
  mudlog("LOCK by %s", pname(mynum));
}

void wizunlock()
{
  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  if (plev(mynum) < LVL_ARCHWIZARD && shimge[1] == 127) {
    bprintf("Sorry, only Arch-Wizards can undo AWIZLOCK.\n");
    return;
  }
  shimge[1] = 0;
  broad("** SYSTEM : The Game is now unlocked.\007\007\n");
  mudlog("UNLOCK by %s", pname(mynum));
}

void awizlock()
{
  if (plev(mynum) < LVL_ARCHWIZARD) {
    erreval();
    return;
  }
  shimge[1] = 127;
  broad("** SYSTEM : The Game is now ARCH-WIZARD locked.\007\007\n");
  mudlog("ARCHLOCK by %s", pname(mynum));
}

void timecom()
{
  FILE *a;
  time_t x;

  bprintf("Elapsed Time : ");
  eltime();  /* get the game time elapsed */

  time(&x);
  bprintf("Current Time : %s", ctime(&x));
  closeworld();
  /* have to be careful with snoop and world reset or can deadlock */
  if (snoopd != -1)
    bprintf(" %c", 8);
  if ((a = openlock(RESET_N, "r")) == NULL)
    return;
  fscanf(a, "%d", &x);
  closelock(a);
  bprintf("Last Reset   : %s", ctime(&x));
}

void warcom()
{
  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  shimge[0] = 0;
  broad("The air of peace and friendship lifts.\n");
}

void peacecom()
{
  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  shimge[0] = 1;
  broad("Peace and tranquility descend upon the world.\n");
}

void awizcom()
{
  char x[128], y[140];

  if (plev(mynum) < LVL_ARCHWIZARD) {
    erreval();
    return;
  }
  getreinput(x);
  sprintf(y, "\001c%s\377# %s\n", pname(mynum), x);
  sendsys(pname(mynum), pname(mynum), -10125, 0, y);
  rd_qd = True;
}

void followcom()
{
  if (i_follow) {
    bprintf("Stopped following \001P%s\377.\n", folnam);
    i_follow = False;
    strcpy(folnam, "");
  }
  if (in_fight) {
    bprintf("Not in a fight!\n");
    return;
  }
  if (EMPTY(item1)) {
    bprintf("Who?\n");
    return;
  }
  if ((i_folcode = pl1) == -1) {
    bprintf("Who?\n");
    return;
  }
  if (ploc(i_folcode) != ploc(mynum)) {
    bprintf("They aren't here.\n");
    return;
  }
  if (mynum == i_folcode) {
    bprintf("You run round in circles!\n");
    return;
  }
  strcpy(folnam, pname(i_folcode));
  i_follow = True;
  if (plev(mynum) <= LVL_ARCHWIZARD) {
    sillytp(i_folcode, "is following you.\n");
  }
  bprintf("Ok\n");
}

void losecom()
{
  if (i_follow == False) {
    bprintf("You aren't following anyone.\n");
    return;
  }
  i_follow = False;
  strcpy(folnam, "");
  i_folcode = -1;
  bprintf("Ok\n");
}

void on_follow()
{
  int ct;

  on_trace();
  if (!i_follow)
    return;
  if (in_fight || fpbn(folnam) != i_folcode) {
    bprintf("You can no longer follow \001P%s\377.\n", folnam);
    i_follow = False;
    return;
  }
  if (ploc(mynum) == ploc(i_folcode))		/* Now to find the guy */
    return;
  if (ltstflg(ploc(i_folcode), lfl(OnWater)))	/* Don't follow to sea! */
    return;
  for (ct = 0; ct < 6; ct++) {
    if (getexit(ploc(mynum), ct) == ploc(i_folcode)
	|| doorthru(getexit(ploc(mynum), ct)) == ploc(i_folcode)) {
      dodirn(ct + 2);
      return;
    }
  }
  bprintf("You can no longer follow \001P%s\377.\n", folnam);
  strcpy(folnam, "");
  i_folcode = -1;
  i_follow = False;
}

int doorthru(int x)
{
  if (x < 1000 || x > 9999)
    return x;
  if (state(x - 1000))
    return 0;
  return oloc((x - 1000) ^ 1);
}

void tracecom()
{
  int i;

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  if (trace_item != -1) {
    bprintf("Stopped tracing %s.\n",
	    trace_class == 1 ? oname(trace_item) : pname(trace_item));
    trace_item = -1;
    if (brkword() == -1)
      return;
  }
  else if (EMPTY(item1)) {
    bprintf("Trace what?\n");
    return;
  }
  if ((i = fpbn(item1)) != -1) {
    trace_class = 2;
    trace_item = i;
    trace_loc = 0;
    trace_carrf = 0;
    bprintf("Ok\n");
    return;
  }
  if ((i = fobn(item1)) != -1) {
    trace_class = 1;
    trace_item = i;
    trace_loc = 0;
    trace_carrf = 0;
    bprintf("Ok\n");
    return;
  }
  bprintf("What's that?\n");
}

void on_trace()
{
  if (trace_item == -1)
    return;
  if (trace_class == 1) {
    if (trace_loc != oloc(trace_item)
	|| trace_carrf != ocarrf(trace_item)) {
      bprintf(">>%s now:", oname(trace_item));
      trace_loc = oloc(trace_item);
      trace_carrf = ocarrf(trace_item);
      showlocation(trace_item);
    }
    return;
  }
  if (EMPTY(pname(trace_item))) {
    bprintf(">>Can no longer trace %s\n", pname(trace_item));
    trace_item = -1;
    return;
  }
  if (ploc(trace_item) != trace_loc) {
    bprintf(">>\001p%s\377 now: ", pname(trace_item));
    trace_loc = ploc(trace_item);
    showploc(trace_item);
  }
}

void mobilecom()
{
  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  bprintf("Mobiles");
  if (shimge[3])
    bprintf("   [Currently STOPped]");
  bprintf("\n\n");
  whop2(1, MAX_USERS, MAX_CHARS);
}

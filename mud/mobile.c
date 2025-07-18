/*
**  Timing event stuff
*/

#include <strings.h>
#include "kernel.h"
#include "macros.h"
#include "objects.h"
#include "mobiles.h"
#include "sendsys.h"
#include "pflags.h"
#include "lflags.h"
#include "mobile.h"
#include "support.h"
#include "blib.h"
#include "blood.h"
#include "parse.h"
#include "exec.h"

static char sccsid[] = "@(#)mobile.c	4.100.0 (IthilMUD)	6/02/90";

static char *exnam[6] = {
  "north", "east", "south", "west", "up", "down"
};

void on_timing()
{
  int ct;

  for (ct = MAX_USERS; ct < MAX_CHARS; ct++) {
    if (!ptstflg(ct, pfl(Possessed)) && pstr(ct) > 0)
      consid_move(ct);		/* Maybe move it */
  }
  if (randperc() < 20)
    onlook();
}

/*
**  Look and timing fight controller
*/
void onlook()
{
  int i;

  for (i = MAX_USERS; i < MAX_CHARS; i++)
    chkfight(i);
  if (iscarrby(OBJ_RUNESWORD, mynum))
    dorune();
  if (phelping(mynum) != -1)
    helpchkr();
}

void chkfight(int x)
{
  int i;

  if (ptstflg(x, pfl(Possessed)) || x < 0 || EMPTY(pname(x))
      || ploc(x) != ploc(mynum) || pstr(x) < 0 || pvis(mynum)
      || ptstflg(mynum, pfl(NoHassle)) || testpeace())
    return;
  /* IF UNDEAD (LEVELS -900 TO -999) DON'T ATTACK CROSS-BEARER */
  if (plev(x) >= -999 && plev(x) <= -900 && iscarrby(OBJ_CROSS, mynum))
    return;
  /* LEVELS 0 to -199 NEVER ATTACK */
  if (plev(x) >= -199 && plev(x) < -1)
    return;
  if (x == MOB_YETI && ohany((1 << 13)) && pfighting(x) == -1) {
    bprintf("The yeti seems disturbed by naked flame, and keeps its distance.\n");
    return;
  }
  i = randperc();
  if (i >= abs(plev(x) % 100))
    return;
  if (pfighting(x) == -1)
    woundmn(x, 0);
}

void consid_move(int x)
{
  char msg[128];
  int  ct;

  if (EMPTY(pname(x)) || randperc() > 1)
    return;
  for (ct = 0; ct < MAX_CHARS; ct++)
    if (pfighting(ct) == x)
      return;
  if (x == MOB_CHICKEN && randperc() < 20)
    sendsys("Chicken Licken", "Chicken Licken", SYS_SAY, ploc(x), "The sky is about to fall in.");
  if (x == MOB_GHOST && randperc() < 15)
    sendsys("", "", SYS_TXT_TO_R, ploc(x), "The Ghost moans, sending chills down your spine.\n");
  if (x == MOB_ANDRE && randperc() < 30) {
	switch (rand() % 3) {
	  case 0: strcpy(msg, "I do not believe in ghosts, this is just a practical joke.\n");
		break;
	  case 1: strcpy(msg, "He was found hanging next to a set from the Roi de Lahore.\n");
		break;
	  case 2: strcpy(msg, "Who is this Opera Ghost, anyway?\n");
		break;
	  }
    sendsys("Mr. Andre", "Mr. Andre", SYS_SAY, ploc(x), msg);
  }
  if (x == MOB_JOEPA && randperc() < 20) {
    switch (rand() % 2) {
      case 0: strcpy(msg, "SHOWER!");
		break;
	  case 1: strcpy(msg, "Sweat, sweat!");
		break;
	  }
    sendsys("Saint Joepa", "Saint Joepa", SYS_SAY, ploc(x), msg);
  }
  if (x == MOB_PUFF && randperc() < 20) {
    switch (rand() % 7) {
      case 0: strcpy(msg, "Rassilon hasn't fed me today.");
	    break;
      case 1: strcpy(msg, "Look at the pretty colors!");
	    break;
      case 2: strcpy(msg, "My god!  It's full of stars!");
	    break;
      case 3: strcpy(msg, "To iterate is human.  To recurse, divine.");
	    break;
      case 4: strcpy(msg, "How'd those fish get up there?");
	    break;
      case 5: strcpy(msg, "Hi!");
	    break;
      case 6: strcpy(msg, "I feel sleepy.  I think I'll go take a nap.");
	    break;
      }
    sendsys("Puff", "Puff", SYS_SAY, ploc(x), msg);
  }
  /* MOBILE LEVELS -100 TO -299 DON'T MOVE, NOR DOES WRAITH */
  if (plev(x) >= -299 && plev(x) <= -100 || x == MOB_WRAITH)
    return;
  if (x == MOB_MINOTAUR)
    if (randperc() < 50) {
      shiftstuff(x);
      return;
    }
  movemob(x);
}    

void crashcom()
{
  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  bprintf("Bye Bye Cruel World...\n");
  sendsys("", "", SYS_CRASH, 0, "");
  mudlog("CRASH by %s", pname(mynum),"");
  rescom();
}

void sys_reset()
{
  FILE *fl;
  int i;
  time_t t;
  long u;

  if (tscale() != 1) {
    bprintf("There are other people on, so it won't work.\n");
    return;
  }
  time(&t);
  if (fl = openlock(RESET_N, "r")) {
    fscanf(fl, "%ld", &u);
    closelock(fl);
    if (u < t && t < u + 3600) {
      bprintf("Sorry, at least an hour must pass between resets.\n");
      return;
    }
  }
  i = plev(mynum);
  setplev(mynum, LVL_WIZARD);
  rescom();
  setplev(mynum, i);
}

/* Handle Runesword */
void dorune()
{
  int ct;

  if (in_fight || testpeace())
    return;
  for (ct = 0; ct < MAX_CHARS; ct++) {
    if (ct == mynum || EMPTY(pname(ct)) || plev(ct) >= LVL_WIZARD)
      continue;
    if (ploc(ct) == ploc(mynum))
      break;
  }
  if (ct == MAX_CHARS)
    return;
  if (randperc() < 9 * plev(mynum) || fpbns(pname(ct)) == -1)
    return;
  bprintf("The Runesword twists in your hands, lashing out savagely!\n");
  hitplayer(ct, OBJ_RUNESWORD);
}

dragget()
{
  int l;

  if (plev(mynum) >= LVL_WIZARD)
    return 0;
  if ((l = alive(MOB_DRAGON)) != -1 && ploc(l) == ploc(mynum)) {
    bprintf("The dragon makes it quite clear that he doesn't want his treasure borrowed!\n");
    return 1;
  }
  if ((l = alive(MOB_COSIMO)) != -1 && ploc(l) == ploc(mynum)) {
    bprintf("Cosimo guards his treasure jealously.\n");
    woundmn(l, 0);
    return 1;
  }
  return 0;
}

void helpchkr()
{
  int x;

  if (!i_setup)
    return;
  x = phelping(mynum);
  if (EMPTY(pname(x)) || ploc(x) != ploc(mynum)) {
    bprintf("You can no longer help \001c%s.\377\n", pname(x));
    setphelping(mynum, -1);
  }
}

void movemob(int x)
{
  int n, r;
  char v[80];

  if (shimge[3])
    return;
  r = randperc() % 6;
  if ((n = getexit(ploc(x),r)) > 9999)
    return;
  if (n > 999) {
    if (state(n - 1000))
      return;
    n = oloc((n - 1000) ^ 1);
  }
  if (n >= 0 || ltstflg(n, lfl(NoMobiles) | lfl(Death)))
    return;
  sprintf(v, "\001s%s\377%s has gone %s.\n\377", pname(x), pname(x), exnam[r]);
  sendsys(pname(x), pname(x), SYS_TXT_TO_R, ploc(x), v);
  setploc(x, n);
  sprintf(v, "\001s%s\377%s has arrived.\n\377", pname(x), pname(x));
  sendsys(pname(x), pname(x), SYS_TXT_TO_R, ploc(x), v);
}

void stopcom()
{
  char ms[20];

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  shimge[3] = 1;
  sprintf(ms, "[Mobiles STOPped]\n");
  sendsys("", "", SYS_WIZ, ploc(mynum), ms);
}

void startcom()
{
  char ms[20];

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  shimge[3] = 0;
  sprintf(ms, "[Mobiles STARTed]\n");
  sendsys("", "", SYS_WIZ, ploc(mynum), ms);
}

void shiftstuff(int m)
{
  int a;
  char x[200];

  for (a = 0; a < numobs; a++)
    if (ocarrf(a) == 0 && oloc(a) == ploc(m) && !oflannel(a)) {
      sprintf(x, "\001sminotaur\377The Minotaur takes the %s.\n\377", oname(a));
      sendsys("", "", SYS_TXT_TO_R, ploc(m), x);
      setoloc(a, m, 1);
      return;
    }
}

/*
** Calculate damage done by monster
*/
char damof(int n)
{
  switch (n) {
  case MOB_RAT:
  case MOB_GHOST:
  case MOB_PRISONER:
    return 6;
  case MOB_AGNES:
  case MOB_BOMBER:
  case MOB_OWIN:
  case MOB_GLOWIN:
  case MOB_SMYTHE:
  case MOB_DIO:
  case MOB_SKELETON:
  case MOB_VIOLA:
  case MOB_COYOTE:
  case MOB_ROTTY:
  case MOB_SUCCUBUS:
  case MOB_PRIEST:
    return 8;
  case MOB_BISON:
  case MOB_WOLF:
  case MOB_EFREET:
  case MOB_LAVAMAN:
  case MOB_SHADOW:
  case MOB_COMMANDER:
  case MOB_GRUEL:
  case MOB_PATCH:
  case MOB_PEGLEG:
    return 12;
  case MOB_WATCHMAN:
  case MOB_USHER:
  case MOB_CERBERUS:
  case MOB_GARGOYLE:
    return 13;
  case MOB_BEAR:
  case MOB_GIANT:
  case MOB_OGRE:
  case MOB_FIGURE:
  case MOB_HOOK:
  case MOB_SCURGE:
    return 14;
  case MOB_TROLL:
  case MOB_VALKYRIE:
  case MOB_ODIN:
  case MOB_THOR:
  case MOB_RIATHA:
  case MOB_BEAST:
  case MOB_DEFENDER:
  case MOB_ASMODEUS:
  case MOB_LICH:
    return 15;
  case MOB_PETER:
  case MOB_JOEPA:
  case MOB_TERRANCE:
  case MOB_COSIMO:
  case MOB_KING:
  case MOB_REDBEARD:
    return 20;
  case MOB_TRUFFLE:
	return 25;
  case MOB_DRAGON:
  case MOB_PUFF:
    return 32;
  case MOB_UNICORN:
    return 40;
  default:
    return 10;
  }
}



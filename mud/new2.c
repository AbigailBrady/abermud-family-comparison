/*
** Extensions section 2
*/

#include <strings.h>
#include "kernel.h"
#include "macros.h"
#include "objects.h"
#include "mobiles.h"
#include "locations.h"
#include "sendsys.h"
#include "pflags.h"
#include "oflags.h"
#include "lflags.h"
#include "new2.h"
#include "support.h"
#include "parse.h"
#include "uaf.h"
#include "objsys.h"
#include "tk.h"
#include "mud.h"
#include "weather.h"
#include "bprintf.h"
#include "exec.h"
#include "verbs.h"

int vichere(int *x)
{
  int a;

  if ((a = vicbase(x)) == -1)
    return -1;
  if (ploc(*x) != ploc(mynum)) {
    bprintf("They aren't here.\n");
    return -1;
  }
  return a;
}

void sillytp(int per, char *msg)
{
  char bk[256];

  sprintf(bk, "\001p%s\377 %s", pname(mynum), msg);
  sendsys(pname(per), pname(mynum), SYS_MSG_TO_P, ploc(mynum), bk);
}

void destroy(int ob)
{
  osetbit(ob, ofl(Destroyed));
  setoloc(ob, RM_DEAD3, 0);
}

void eat(int ob)
{
  osetbit(ob, ofl(Destroyed));
  setoloc(ob, RM_DEAD2, 0);
}

void create(int ob)
{
  oclrbit(ob, ofl(Destroyed));
}

int tscale()
{
  int a = 0;
  int b;

  for (b = 0; b < MAX_USERS; b++)
    if (!EMPTY(pname(b)) && plev(b) < LVL_WIZARD)
      a++;
  if (a < 2)
    return 1;
  else
    return (a > 9 ? 9 : a);
}

int chkdumb()
{
  if (!ptstflg(mynum, pfl(Dumb)))
    return 0;
  bprintf("You are mute.\n");
  return 1;
}

int chkcrip()
{
  if (!ptstflg(mynum, pfl(Crippled)))
    return 0;
  bprintf("You are crippled.\n");
  return 1;
}

int chksitting()
{
  if (!psitting(mynum))
    return 0;
  bprintf("You'll have to stand up, first.\n");
  return 1;
}

void wounded(int n)
{
  char ms[128];

  if (plev(mynum) >= LVL_WIZARD)
    return;
  setpstr(mynum, pstr(mynum) - n);
  me_cal = True;
  if (pstr(mynum) >= 0)
    return;
  closeworld();
  mudlog("%s slain magically", pname(mynum), "");
  delpers(pname(mynum));
  openworld();
  dumpitems();
  sprintf(ms, "%s has died.\n", pname(mynum));
  sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, ploc(mynum), ms);
  sprintf(ms, "[%s has died]\n", pname(mynum));
  sendsys(pname(mynum), pname(mynum), SYS_WIZ, ploc(mynum), ms);
  zapped = True;
  loseme();
  crapup("Oh dear, you seem to have died.\n");
}

void resetplayers()
{
  int a, c;

  for (c = 0, a = MAX_USERS; a < numchars; a++, c++) {
    strcpy(pname(a), pinit[c].p_name);
    setploc(a, pinit[c].p_loc);
    setpstr(a, pinit[c].p_str);
    setphelping(a, -1);
    setpflags(a, pinit[c].p_flags);
    setpsitting(a, 0);
    setpwpn(a, -1);
    setpvis(a, 0);
    setplev(a, pinit[c].p_lev);
  }
  for (; a < MAX_CHARS; a++)
    strcpy(pname(a), "");
}

void wearcom()
{
  int a;

  if (ohereandget(&a) == -1)
    return;
  if (!iscarrby(a, mynum)) {
    bprintf("You don't have it.\n");
    return;
  }
  if (iswornby(a, mynum)) {
    bprintf("You're already wearing it.\n");
    return;
  }
  if ((iswornby(OBJ_SHIELD, mynum) || iswornby(OBJ_SHIELD_2, mynum) || iswornby(OBJ_SHIELD_3, mynum) || iswornby(OBJ_SHIELD_4, mynum) || iswornby(OBJ_SHIELD_5, mynum)) && (a == OBJ_SHIELD || a == OBJ_SHIELD_2 || a == OBJ_SHIELD_3 || a == OBJ_SHIELD_4 || a == OBJ_SHIELD_5)) {
    bprintf("You can't use two shields at once.\n");
    return;
  }
  if ((iswornby(OBJ_ARMOR, mynum) || iswornby(OBJ_CHAINMAIL, mynum)) && (a == OBJ_ARMOR || a == OBJ_CHAINMAIL)) {
    bprintf("You can't wear two suits of armor at once.\n");
    return;
  }
  if (!otstbit(a, ofl(Wearable))) {
    bprintf("Is this a new fashion?\n");
    return;
  }
  setcarrf(a, 2);
  bprintf("Ok\n");
}

void removecom()
{
  int a;

  if (ohereandget(&a) == -1)
    return;
  if (!iswornby(a, mynum)) {
    bprintf("You're not wearing it.\n");
    return;
  }
  setcarrf(a, 1);
  bprintf("Ok\n");
}

void setcarrf(int o, int n)
{
  ocarrf(o) = n;
}

int iswornby(int item, int chr)
{
  if (!iscarrby(item, chr) || ocarrf(item) != 2)
    return 0;
  return 1;
}

void addforce(char *x)
{
  if (forf == 1)
    bprintf("The compulsion to %s is overridden.\n", acfor);
  forf = 1;
  strcpy(acfor, x);
}

void forchk()
{
  isforce = True;
  if (forf == 1)
    gamecom(acfor);
  isforce = False;
  forf = 0;
}

int iam(char *x)
{
  char a[64], b[64];

  strcpy(a, x);
  strcpy(b, pname(mynum));
  if (EQ(a, b))
    return 1;
  if (strncasecmp(b, "the ", 4) == 0) {
    if (EQ(a, b + 4))
      return 1;
  }
  return 0;
}

void teletrap(int newch)
{
  char block[200];

  sprintf(block, "\001s%s\377%s\n\377", pname(mynum), pname(mynum),
	  " has left.");
  sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, ploc(mynum), block);
  sprintf(block, "\001s%s\377%s\n\377", pname(mynum), pname(mynum),
	  " has arrived.");
  sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, newch, block);
  trapch(newch);
}

void on_flee_event()
{
  int ct;
    
  for (ct = 0; ct < numobs; ct++)
    if (iscarrby(ct, mynum) && !iswornby(ct, mynum) && pwpn(mynum) != ct)
      setoloc(ct, ploc(mynum), 0);
}

void on_flee_mob(int n)
{
  int ct;
    
  for (ct = 0; ct < numobs; ct++)
    if (iscarrby(ct, n) && !iswornby(ct, n) && pwpn(n) != ct)
      setoloc(ct, ploc(n), 0);
}

void sitcom()
{
  char x[80];

  if (psitting(mynum)) {
    bprintf("You're already sitting.\n");
    return;
  }
  else if (in_fight) {
    bprintf("You want to sit down while fighting?  Do you have a death wish or something?\n");
    return;
  }
  else{
    bprintf("You assume the lotus position.\n");
    sprintf(x, "\001s%s\377%s sits down.\n\377", pname(mynum), pname(mynum));
    sillycom(x);
    setpsitting(mynum, 1);
  }
}

void standcom()
{
  char x[80];

  if (!psitting(mynum)) {
     bprintf("You're already standing.\n");
     return;
  }
  else {
     bprintf("You clamber to your feet.\n");
     sprintf(x, "\001s%s\377%s clambers to %s feet.\n\377", pname(mynum), pname(mynum), his_or_her(mynum));
     sillycom(x);
     setpsitting(mynum, 0);
  }
}

void dircom()
{
  int a, c, oc = 0;
  char b[40], d[40];

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  for (a = 0; a < numobs; a++) {
    if (otstbit(a, ofl(Unlocatable))  && (plev(mynum) < LVL_GOD))
      continue;
    oc++;
    c = findzone(oloc(a), b);
    sprintf(d, "%s%d", b, c);
    if (ocarrf(a))
      strcpy(d, "CARRIED");
    if (ocarrf(a) == 3)
      strcpy(d, "IN ITEM");
    bprintf("%-13s%-13s", oname(a), d);
    if (a % 3 == 2)
      bprintf("\n");
    if (a % 18 == 17)
      pbfr();
  }
  bprintf("\nTotal of %d objects.\n", oc);
}

void treasurecom()
{
  int a, c, tc = 0;
  char b[40], d[40];

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  for (a = 0; a < numobs; a++) {
    if (!oflannel(a) && !otstbit(a, ofl(Destroyed)) && !ocarrf(a)) {
      tc++;
      c = findzone(oloc(a), b);
      sprintf(d, "%s%d", b, c);
      bprintf("%-13s%-13s", oname(a), d);
      if (tc % 3 == 0)
	bprintf("\n");
      if (tc % 18 == 17)
	pbfr();
    }
  }
  bprintf("\nApproximately %d treasures remain.\n", tc);
}

void levelscom()
{
  int i;

  bprintf("Level   Points           Male            	 Female\n");
  bprintf("=====   ======           ====            	 ======\n");
  for (i = 1; i <= LVL_WIZARD; i++) {
    bprintf("%-5d   %6d          ", i, levels[i]);
    displevel(i, 0, "");
    bprintf("\t");
    if ((i != LVL_WIZARD) && (i != LVL_NECROMANCER))
      bprintf("\t");
    displevel(i, 1, "");
    bprintf("\n");
  }
  bprintf("\n");
}

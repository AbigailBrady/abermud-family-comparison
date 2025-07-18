/*
**  Magic!
*/

#include "kernel.h"
#include "actions.h"
#include "locdir.h"
#include "macros.h"
#include "objects.h"
#include "mobiles.h"
#include "sendsys.h"
#include "pflags.h"
#include "oflags.h"
#include "lflags.h"
#include "magic.h"
#include "support.h"
#include "exec.h"
#include "parse.h"
#include "blood.h"
#include "extra.h"
#include "new2.h"
#include "weather.h"
#include "tk.h"

static char sccsid[] = "@(#)magic.c	4.100.0 (IthilMUD)	6/02/90";

int randperc()
{
  return (rand() % 100);
}

/* Brian Preble -- shows current set*in/set*out settings */
void reviewcom()
{
  char ms[128];

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  bprintf("Current travel messages:\n");
  bprintf("------------------------\n");
  bprintf("SETIN   : %s %s\n", pname(mynum), in_ms);
  sprintf(ms, out_ms, "<dir>");
  bprintf("SETOUT  : %s %s\n", pname(mynum), ms);
  bprintf("SETMIN  : %s %s\n", pname(mynum), min_ms);
  bprintf("SETMOUT : %s %s\n", pname(mynum), mout_ms);
  bprintf("SETVIN  : %s %s\n", pname(mynum), vin_ms);
  bprintf("SETVOUT : %s %s\n", pname(mynum), vout_ms);
}

void cripplecom()
{
  int a;

  if (vicf2(&a, SPELL_REFLECTS, 4) != -1)
    sendsys(pname(a), pname(mynum), SYS_CRIPPLE, ploc(mynum), "");
}

/* Brian Preble -- Heals a player */
void healcom()
{
  int a;
    
  if (EMPTY(item1)) {
    bprintf("Heal who?\n");
    return;
  }
  if (plev(mynum) < LVL_WIZARD) {
    bprintf("The spell fails.\n");
    return;
  }
  if (vicf2(&a, SPELL_PEACEFUL, 100) != -1) {
    if (a == mynum) {
      bprintf("You feel much better.\n");
      setpstr(mynum, maxstrength(mynum));
      return;
    }
    sendsys(pname(a), pname(mynum), SYS_HEAL, ploc(mynum), "");
    bprintf("Power radiates from your hands to heal %s.\n", pname(a));
  }
}

void curecom()
{
  int a;

  if (EMPTY(item1)) {
    bprintf("Cure who?\n");
    return;
  }
  if (vicf2(&a, SPELL_PEACEFUL, 8) != -1) {
    sendsys(pname(a), pname(mynum), SYS_CURE, ploc(mynum), "");
    if (a != mynum)
      bprintf("With a laying on of hands, you miraculously cure %s.\n", pname(a));
  }
}

void dumbcom()
{
  int a;

  if (vicf2(&a, SPELL_REFLECTS, 4) != -1)
    sendsys(pname(a), pname(mynum), SYS_DUMB, ploc(mynum), "");
}

void forcecom()
{
  int a;
  char z[128];

  if (vicf2(&a, SPELL_REFLECTS, 4) == -1)
    return;
  if (a >= MAX_USERS) {
    bprintf("You can only force players to do things.\n");
    return;
  }
  getreinput(z);
  sendsys(pname(a), pname(mynum), SYS_FORCE, ploc(mynum), z);
}

int castspell(int m)
{
  if (mynum == m) {
    bprintf("You're supposed to be killing others, not yourself.\n");
    error();
    return 0;
  }
  if (ptstflg(m, pfl(NoHassle))) {
    bprintf("The magic flashes harmlessly past.\n");
    error();
    return 0;
  }
  return -1;
}

void damagebyspell(int m, int v)
{
  if (m >= MAX_USERS)
    woundmn(m, v);
  if (plev(m) <= LVL_ARCHWIZARD && pstr(m) < 0) {
    bprintf("Your last spell did the trick.\n");
    if (m < MAX_USERS)
      if (plev(m) > 2)
	setpscore(mynum, pscore(mynum) +
		  (plev(m) * plev(m) * plev(m) * 10));
      else
	setpscore(mynum, pscore(mynum) + 10 * damof(m));
    setpstr(m, -1);
    in_fight = 0;
    fighting = -1;
    setpfighting(mynum, -1);
  }
  else
    if (plev(m) > 2 || plev(m) < 0) /* no points for wimps */
      setpscore(mynum, pscore(mynum) + v * 2);
}

void missilecom()
{
  int a;
  char ar[8];

  if (vichfb(&a, 5) == -1)
    return;
  if (castspell(a)) {
    sprintf(ar, "%d", plev(mynum) * 2);
    sendsys(pname(a), pname(mynum), SYS_MISSILE, ploc(mynum), ar);
    damagebyspell(a, plev(mynum) * 2);
  }
}

void fireballcom()
{
  int a, damage;
  char ar[8];

  if (vichfb(&a, 5) == -1)
    return;
  if (castspell(a)) {
    damage = (iscarrby(OBJ_SULPHUR, mynum) ? 3 : 2);
	if (iscarrby(OBJ_SULPHUR_1, mynum)) {
	  damage = 3;
	}
    damage = (a == MOB_YETI || a == MOB_GUARDIAN ? 6 : damage) * plev(mynum);
    sprintf(ar, "%d", damage);
    sendsys(pname(a), pname(mynum), SYS_FIREBALL, ploc(mynum), ar);
    damagebyspell(a, damage);
  }
}

void shockcom()
{
  int a, damage;
  char ar[8];

  if (vichfb(&a, 5) == -1)
    return;
  if (castspell(a)) {
    damage = (iscarrby(OBJ_LODESTONE, mynum) ? 3 : 2);
    damage = damage * plev(mynum);
    sprintf(ar, "%d", damage);
    sendsys(pname(a), pname(mynum), SYS_SHOCK, ploc(mynum), ar);
    damagebyspell(a, damage);
  }
}

void frostcom()
{
  int a, damage;
  char ar[8];

  if (vichfb(&a, 5) == -1)
    return;
  if (castspell(a)) {
    damage = iscarrby(OBJ_WINTERGREEN, mynum) ? 3 : 2;
    damage = (a == MOB_FLAME || a == MOB_EFREET || a == MOB_LAVAMAN
	      ? 6 : damage) * plev(mynum);
    sprintf(ar, "%d", damage);
    sendsys(pname(a), pname(mynum), SYS_FROST, ploc(mynum), ar);
    damagebyspell(a, damage);
  }
}

void sumcom()
{
  sumb(1);
}

void transcom()
{
  if (plev(mynum) < LVL_NECROMANCER)
    erreval();
  else
    sumb(0);
}

void sumb(int n)
{
  int a;
  char seg[128];
  int c, d, x;
  FILE *fl;
  time_t t;
  long u;

  if (plev(mynum) < LVL_CONJURER)
  {
      bprintf("Pardon?\n");
      return;
  }

  if (EMPTY(item1)) {
    bprintf("Summon who?\n");
    return;
  }
  if ((a = fpbn(item1)) != -1) {
    if ((a == mynum) && (plev(mynum) < LVL_GOD)) {
      bprintf("Sorry, you can't summon yourself.\n");
      return;
    }
    if (plev(mynum) < LVL_WIZARD) {
      if (pstr(mynum) < 10) {
	bprintf("You're too weak.\n");
	return;
      }
      if (plev(mynum) < LVL_WIZARD)
	setpstr(mynum, pstr(mynum) - 1);
      c = plev(mynum) * 2;
      if (plev(mynum) >= LVL_WIZARD)
	c = 101;
      if (iscarrby(OBJ_POWERSTONE, mynum) ||
	  iscarrby(OBJ_POWERSTONE_1, mynum) ||
	  iscarrby(OBJ_POWERSTONE_2, mynum))
	c += plev(mynum);
      d = randperc();
      if (ltstflg(ploc(mynum), lfl(OnePerson))) {
	bprintf("It's too restricted in here.\n");
	return;
      }
      if (ltstflg(ploc(mynum), lfl(NoSummon))) {
	bprintf("Something prevents your summoning from succeeding.\n");
	return;
      }
      if (ltstflg(ploc(mynum), lfl(OnWater))) {
	bprintf("The boat is rolling too much.\n");
	return;
      }
      if (c < d) {
	bprintf("The spell fails.\n");
	return;
      }
/* 
**  If NO_SUMMON is defined, then mortals may not summon mobiles unless
**  NO_SUMMON minutes have passed since the reset. -- Rassilon
*/
#ifdef NO_SUMMON
      if (a >= MAX_USERS && plev(mynum) < LVL_WIZARD) {
	if (gametime() < NO_SUMMON * 60) {
	  bprintf("No summoning until 1.5 hours have passed ");
	  bprintf("from reset time.\n");
	  return;
	}
      }  /*  LEVEL CHECK */
#endif
      if (plev(mynum) < LVL_WIZARD && (
	  iswornby(OBJ_SHIELD_4, a) || iswornby(OBJ_AMULET, a) || 
	  iscarrby(OBJ_RUNESWORD, a) || iswornby(OBJ_TALISMAN, a) || 
	  iswornby(OBJ_PENDANT, a) || plev(a) >= LVL_WIZARD ||
	  ptstflg(a, pfl(Unsummonable)))) {
	  bprintf("Something blocks your summoning.\n");
	  return;
      }
      if (a == mynum) {
	    bprintf("You're already here.\n");
	    return;
      }
    }
    if ((plev(mynum) < LVL_ARCHWIZARD) && (plev(a) >= LVL_WIZARD)) {
      bprintf("You can't summon Wizards, Arch-Wizards, or Gods.\n");
      sendsys(pname(a), pname(mynum), SYS_SUMMON, ploc(mynum), n ? "1" : "0");
      return;
    }
    bprintf("You cast the summoning......\n");
    if (a < MAX_USERS) {	/* Is this a mobile or a player? */
      if (plev(mynum) >= LVL_ARCHWIZARD && plev(mynum) < LVL_GOD)
	sendsys(pname(a), pname(mynum), SYS_WIZ_SUMMON, ploc(mynum), n ? "1" : "0");
      else if (plev(mynum) >= LVL_GOD)
	sendsys(pname(a), pname(mynum), SYS_GOD_SUMMON, ploc(mynum), n ? "1" : "0");
	  else
	sendsys(pname(a), pname(mynum), SYS_SUMMON, ploc(mynum), n ? "1" : "0");
      return;
    }
    if (n)
      on_flee_mob(a);
    sprintf(seg, "\001s%s\377%s disappears in a puff of smoke.\n\377",
	    pname(a), pname(a));
    sendsys("", "", SYS_TXT_TO_R, ploc(a), seg);
    setploc(a, ploc(mynum));
    sprintf(seg, "\001s%s\377%s arrives.\n\377", pname(a), pname(a));
    sendsys("", "", SYS_TXT_TO_R, ploc(a), seg);
    if (a == MOB_SKELETON) {
      sprintf(seg, "\001c%s disintegrates, leaving behind a pile of bones which quickly\ncrumble into dust and are blown away by the wind.\n\377",
	      pname(a));
      sendsys("", "", SYS_TXT_TO_R, ploc(mynum), seg);
      woundmn(a, 10000);
    }
    if (plev(mynum) < LVL_WIZARD)
      /* Monster regards summoning as a hostile action */
      woundmn(a, 0);
    return;
  }
  /* Summon object... also by number now */
  else if ((a = fobn(item1)) != -1) {
    if (plev(mynum) < LVL_WIZARD) {
      bprintf("You can only summon people and monsters.\n");
      return;
    }
    if (otstbit(a, ofl(Unlocatable)) && (plev(mynum) < LVL_GOD))
    {
      bprintf("Strange...  You can't seem to find it.\n");
      return;
    }
    x = oloc(a);
    if (ocarrf(a) > 0)
      x = ploc(x);
    sprintf(seg, "The %s vanishes!\n", oname(a));
    sendsys("", "", SYS_MSG_TO_Rd, x, seg);
    if(!ptstflg(mynum, pfl(NoArms))) {
       bprintf("The %s flies into your hand.\nIt was:", oname(a));
       showlocation(a);
       setoloc(a, mynum, CARRIED_BY);
       return;
       }
    else {
       bprintf("The %s appears where your hand should be.\nIt was:",oname(a));
       showlocation(a);
       setoloc(a, ploc(mynum), IN_ROOM);
       return;
       }
  }
  else {
    bprintf("Who or what is that?\n");
    return;
  }
}

void goloccom(int tiptoe)
{
  char bf[128];
  int a, i, pc;

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  if ((a = getroomnum()) == 0 || !exists(a)) {
    bprintf("Unknown Player Or Room\n");
    return;
  }
  if (plev(mynum) < LVL_GOD) {
    if (ltstflg(a, lfl(Private))) {
      pc = 0;
      for (i = 0; i < MAX_USERS; i++)
        if (!EMPTY(pname(i)) && ploc(i) == a)
	  pc++;
      if (pc > 1) {
        bprintf("I'm sorry, there's a private conference in that location.\n");
        return;
      }
    }
  }
  if (!tiptoe) {
    sprintf(bf, "\001s%%s\377%%s %s\n\377", mout_ms);
    sillycom(bf);
  }
  setploc(mynum, a);
  trapch(a);
  if (!tiptoe) {
    sprintf(bf, "\001s%%s\377%%s %s\n\377", min_ms);
    sillycom(bf);
  }
}

void wizcom()
{
  char bf[128];

  if (plev(mynum) < LVL_WIZARD) {
    bprintf("Such advanced conversation is beyond you.\n");
    return;
  }
  getreinput(wordbuf);
  sprintf(bf, "\001p%s\377: %s\n", pname(mynum), wordbuf);
  sendsys(pname(mynum), pname(mynum), SYS_WIZ, ploc(mynum), bf);
  rd_qd = True;
}

void viscom()
{
  char bf[128];
  int ar[4];

  if (plev(mynum) < LVL_WIZARD) {
    bprintf("You can't do that sort of thing at will, you know.\n");
    return;
  }
  if (!pvis(mynum)) {
    bprintf("You're already visible.\n");
    return;
  }
  setpvis(mynum, 0);
  ar[0] = mynum;
  ar[1] = pvis(mynum);
  sendsys("", "", SYS_VIS, 0, (char *)ar);
  bprintf("Ok\n");
  pclrflg(mynum, pfl(InvisStart));
  sprintf(bf, "\001s%%s\377%%s %s\n\377", vin_ms);
  sillycom(bf);
}

void inviscom()
{
  char bf[128];
  int x;
  int ar[4];

  if (plev(mynum) < LVL_WIZARD) {
    bprintf("You can't turn invisible at will, you know.\n");
    return;
  }
  if (plev(mynum) >= LVL_GOD)
    x = LVL_GOD;
  else
    x = plev(mynum) >= LVL_ARCHWIZARD ? LVL_GOD : LVL_ARCHWIZARD;
  if (op(mynum) && brkword() != -1)
    x = atoi(wordbuf);
  if (pvis(mynum) == x) {
    bprintf("You're already invisible.\n");
    return;
  }
  setpvis(mynum, x);
  ar[0] = mynum;
  ar[1] = pvis(mynum);
  sendsys("", "", SYS_VIS, 0, (char *)ar);
  bprintf("Ok\n");
  psetflg(mynum, pfl(InvisStart));
  sprintf(bf, "\001c%%s %s\n\377", vout_ms);
  sillycom(bf);
}

void resurcom()
{
  char bf[80];
  int a;

  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  if (EMPTY(item1)) {
    bprintf("Resurrect what?\n");
    return;
  }
  if ((a = fobn(item1)) == -1) {
    bprintf("You can only resurrect objects.\n");
    return;
  }
  if (ospare(a) != -1) {
    bprintf("It already exists.\n");
    return;
  }
  oclrbit(a, ofl(Destroyed));
  setoloc(a, ploc(mynum), 0);
  sprintf(bf, "The %s suddenly appears.\n", oname(a));
  sendsys("", "", SYS_TXT_TO_R, ploc(mynum), bf);
}

int vicf2(int *x, int f1, int i)
{
  int a;

  if (ltstflg(ploc(mynum), lfl(NoMagic)) && plev(mynum) < LVL_WIZARD) {
    bprintf("Something about this location has drained your mana.\n");
    return -1;
  }
  if (f1 >= SPELL_VIOLENT && plev(mynum) < LVL_WIZARD && testpeace()) {
    bprintf("No, that's violent!\n");
    return -1;
  }
  if ((a = vicbase(x)) == -1)
    return -1;
  if (pstr(mynum) < 10) {
    bprintf("You are too weak to cast magic.\n");
    return -1;
  }
  if (plev(mynum) < LVL_WIZARD)
    setpstr(mynum, pstr(mynum) - 1);
  if (iscarrby(OBJ_POWERSTONE, mynum))
    i++;
  if (iscarrby(OBJ_POWERSTONE_1, mynum))
    i++;
  if (iscarrby(OBJ_POWERSTONE_2, mynum))
    i++;
  if (plev(mynum) < LVL_WIZARD && randperc() > i * plev(mynum)) {
    bprintf("You fumble the magic.\n");
    if (f1 == SPELL_REFLECTS) {
      *x = mynum;
      bprintf("The spell reflects back.\n");
      return a;
    }
    return -1;
  }
  if (plev(mynum) < LVL_WIZARD) {
    if ((f1 >= SPELL_VIOLENT) && iswornby(OBJ_SHIELD_4, pl1)) {
      bprintf("The spell is absorbed by %s shield.\n", his_or_her(pl1));
      return -1;
    }
    else
      bprintf("The spell succeeds!\n");
  }
  else
    bprintf("The spell succeeds!\n");
  return a;
}

int vichfb(int *x, int cth)
{
  int a;

  if ((a = vicf2(x, SPELL_VIOLENT, cth)) == -1)
    return -1;
  if (ploc(*x) != ploc(mynum)) {
    bprintf("%s isn't here.\n", psex(*x) ? "She" : "He");
    return -1;
  }
  return a;
}

void deafcom()
{
  int a;

  if (vicf2(&a, SPELL_REFLECTS, 4) != -1)
    sendsys(pname(a), pname(mynum), SYS_DEAF, ploc(mynum), "");
}

void blindcom()
{
  int a;

  if (vicf2(&a, SPELL_REFLECTS, 4) != -1)
    sendsys(pname(a), pname(mynum), SYS_BLIND, ploc(mynum), "");
}

void zapcom()
{
  int vic;

  if (plev(mynum) < LVL_WIZARD) {
    bprintf("The spell fails.\n");
    return;
  }
  if (brkword() == -1) {
    bprintf("Zap who?\n");
    return;
  }
  if ((vic = pl1) == -1) {
    bprintf("There is no one on with that name.\n");
    return;
  }
  if (plev(vic) >= LVL_GOD) {
    bprintf("The spell fails.\n");
    return;
  }
  sendsys(pname(vic), pname(mynum), SYS_ZAP, ploc(vic), "");
  mudlog("%s zapped %s", pname(mynum), pname(vic));
  if (vic >= MAX_USERS)
    woundmn(vic, 10000);	/* if mobile, kill it */
  broad("\001dYou hear an ominous clap of thunder in the distance.\n\377");
}

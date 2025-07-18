/*
**  The combat system, this is one piece of code where mobile/player
**  differences still show up a lot.
**
**  in_fight and pfighting should be player properties for quick reference.
*/

#include <strings.h>
#include "kernel.h"
#include "blood.h"
#include "bprintf.h"
#include "exec.h"
#include "locations.h"
#include "lflags.h"
#include "macros.h"
#include "magic.h"
#include "sendsys.h"
#include "mobiles.h"
#include "mud.h"
#include "new1.h"
#include "new2.h"
#include "uaf.h"
#include "objects.h"
#include "objsys.h"
#include "oflags.h"
#include "parse.h"
#include "pflags.h"
#include "support.h"
#include "tk.h"
#include "weather.h"

static char sccsid[] = "@(#)blood.c	4.100.0 (IthilMUD)	6/02/90";

#define	plevme	plev(mynum)
#define plevvic	plev(victim)

/*
**  Calculate damage done by an item
*/

int dambyitem(int it)
{
  if (ptstflg(mynum, pfl(NoArms)))
    return 3;
  if (it == -1)
    return 8;
  return !otstbit(it, ofl(Weapon)) ? -1 : obyte(it, 0) + 8;
}

/*
**  Command function block to handle wielding of weapons.  On full
**  database version of system, you should use a parameter for the
**  item to wield.
*/

void weapcom()
{
  int a;

  if (ptstflg(mynum, pfl(NoArms))) {
    bprintf("You can't wield anything, you're unarmed.\n");
    return;
  }
  if ((a = ob1) == -1) {
    bprintf("What's that?\n");
    return;
  }
  else if (dambyitem(a) < 0) {
    bprintf("It's not a weapon.\n");
    setpwpn(mynum, -1);
    error();
    return;  /* This should fix the wield anything bug. */
  }
  setpwpn(mynum, a);
  bprintf("You are now wielding the %s.\n", oname(a));
}

/*
**  Routine to hit a player, not just the kill command - lower level
*/

void hitplayer(int victim, int wpn)
{
  int x[4];
  int cth, ddn, res, i;
  char ms[80];

  /* Catch case of victim being ex'd or quiting etc */
  if (EMPTY(pname(victim)))
    return;

  /* Bloody special cases, they get everywhere! */
  /* CASE_SPECIAL  */

  if (plevme < LVL_WIZARD && pl1 == MOB_PETER) {
    bprintf("Peter waves, and the floor vanishes beneath you.\n");
    bprintf("You don't feel so good anymore.\n\n");
    teletrap(RM_WASTE1);
    bprintf("You hear a voice saying 'Bad move.'\n\n");
    return;
  }

  if (plevme < LVL_WIZARD && victim == MOB_CAT && !ptstflg(mynum, pfl(Blind))) {
    bprintf("The Cat scratches your eyes out!  You are blind!\n");
    psetflg(mynum, pfl(Blind));
  }

  if (plevme < LVL_WIZARD && victim == MOB_CHICKEN && !psex(victim)) {
    bprintf("As you look up you see the sky is somehow different.\n");
    bprintf("The next second you're crushed to death as the sky hits you.\n");
    sprintf(ms, "%s is squashed as %s attacks Chicken Licken.\n", pname(mynum), psex(mynum) ? "she" : "he");
    sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, ploc(mynum), ms);
    loseme();
    crapup("Chicken Licken was right\n");
  }

  if (plevme < LVL_WIZARD && victim == MOB_SERAPH) {
    bprintf("The Seraph is amused by your foolhardiness.  For your impudence, he draws his\nflaming sword and slays you with a single blow.\n");
    sprintf(ms, "\001s%s\377%s is sliced in half by the Seraph!\n", pname(mynum), pname(mynum));
    sendsys("", "", SYS_TXT_TO_R, ploc(mynum), ms);
    loseme();
    crapup("Never pick a fight with an angel\n");
  }

  if (plevme < LVL_WIZARD && victim == MOB_BEGGAR) {
    bprintf("You drop everything as you are summoned by The Seraph.\n");
    bprintf("The Seraph scowls at you contemptuously.  For your lack of compassion, he\n");
    bprintf("draws his flaming sword and slays you with a single blow.\n");
    sprintf(ms, "\001s%s\377%s is sliced in half by the Seraph!\n", pname(mynum), pname(mynum));
    sendsys("", "", SYS_TXT_TO_R, ploc(mynum), ms);
    loseme();
    crapup("Remember the ten commandments: Thou Shalt Not Kill\n");
  }

  if (testpeace())
    return;

  if (!iscarrby(wpn, mynum) && wpn != -1) {
    bprintf("You belatedly realize you don't have the %s,\nand are forced to use your hands instead.\n", oname(wpn));
    if (pwpn(mynum) == wpn)
      setpwpn(mynum, -1);
    wpn = -1;
  }

  setpwpn(mynum, wpn);
  if (wpn == OBJ_RUNESWORD && iscarrby(OBJ_STAFF, victim)) {
    bprintf("The Runesword flashes back from its target, growling in anger!\n");
    return;
  }

  if (ptstflg(mynum, pfl(Assassin)) && plevvic > -2) {
    bprintf ("%s hits you with a mighty blow!\n", pname(victim));
    wounded(50);
    return;
  }

  if (dambyitem(wpn) < 0) {
    bprintf("That's not a weapon.\n");
    setpwpn(mynum, -1);
    error();
    return;
  }

  if (in_fight) {
    bprintf("You're already fighting!\n");
    error();
    return;
  }

  if (psitting(mynum))
    standcom();

  fighting = victim;
  setpfighting(mynum, victim);
  in_fight = 300;
  res = randperc();

  if (mynum < MAX_USERS)
    cth = 4 * plevme + 30;
  else
    cth = 57;

  if (iscarrby(OBJ_HORSESHOE, mynum))
    cth += 13;

  for (i = 0; i < numobs; i++)
    if (iswornby(i, victim) && otstbit(i, ofl(Armour)))
      cth -= obyte(i, 1);

  if (cth < 0)
    cth = 0;

  if (cth > res)
    ddn = rand() % (1 + dambyitem(wpn));
  else
    ddn = -1;

  combatmessage(victim, mynum, wpn, ddn);

  if (cth > res) {
    x[0] = mynum;
    x[1] = ddn;
    x[2] = wpn;
    if (pstr(victim) - ddn < 0) {
      if ((pstr(victim) >= 0) && (victim < MAX_USERS) && (plevvic > 2))
	setpscore(mynum, pscore(mynum) + (plevvic * plevvic * plevvic * 10));
      else
        setpscore(mynum, pscore(mynum) + 10 * damof(victim));
      setpstr(victim, -1);
      in_fight = 0;
      fighting = -1;
      setpfighting(mynum, -1);
    }
    sendsys(pname(victim), pname(mynum), SYS_ATTACK, ploc(mynum), (char *) x);
    if (victim >= MAX_USERS)
      woundmn(victim, ddn);
    if (plevvic > 2 || plevvic < 0)      /* no points for wimps */
      setpscore(mynum, pscore(mynum) + ddn * 2);
    calibme();
    return;
  }
  x[0] = mynum;
  x[1] = -1;
  x[2] = wpn;
  sendsys(pname(victim), pname(mynum), SYS_ATTACK, ploc(mynum), (char *) x);
  if (victim >= MAX_USERS)
    woundmn(victim, 0);
}

/*
**  The killing and hitting command
*/

void killcom()
{
  if (pl1 == -1 && ob1 == -1) {
    bprintf("Kill who?\n");
    error();
    return;
  }
  if (EQ(item1, "door")) {
    if (plevme < LVL_ARCHWIZARD) {
      bprintf("Who do you think you are?  An arch-wizard?\n");
      error();
    }
    else
      wreckdoor();
    return;
  }
  if (ob1 != -1) {
    breakitem(ob1);
    return;
  }
  if (pl1 == -1) {
    bprintf("You can't do that.\n");
    error();
    return;
  }
  if (pl1 == mynum) {
    bprintf("Come on, it'll look better tomorrow...\n");
    error();
    return;
  }
  if (ploc(pl1) != ploc(mynum)) {
    bprintf("They aren't here.\n");
    error();
    return;
  }
  if (testpeace()) {
    bprintf("Sorry, it's too peaceful for violence.\n");
    error();
    return;
  }
  if (ptstflg(pl1, pfl(NoHassle))) {
    bprintf("Something interferes with your attack.\n");
    error();
    return;
  }
  if (ob2 == -1) {
    hitplayer(pl1, pwpn(mynum));
    return;
  }
  hitplayer(pl1, ob2);
}

/*
**  Receive a combat event
*/

void bloodrcv(int *array, int isme)
{
  char ms[128];

  if (!isme)
    return;			/* Hit views handled elsewhere */
  if (array[0] < 0)
    return;			/* Silly value dump */
  if (EMPTY(pname(array[0])))
    return;			/* Enemy has been exorcised */
  fighting = array[0];
  setpfighting(mynum, array[0]);
  in_fight = 300;
  if (ploc(mynum) != ploc(array[0]))
    return;
  combatmessage(mynum, array[0], array[2], array[1]);
  if (array[1] == -1)
    return;
  if (plevme < LVL_WIZARD) {
    setpstr(mynum, pstr(mynum) - array[1]);
    if (array[0] == MOB_WRAITH) {
      setpscore(mynum, pscore(mynum) - 100 * array[1]);
      bprintf("You feel weaker, as %s's icy touch drains your very life force!\n", pname(array[0]));
      if (pscore(mynum) < 0)
        setpstr(mynum, -1);
    }
  }
  if (pstr(mynum) < 0) {
    mudlog("%s slain by %s", pname(mynum), pname(array[0]));
    dumpitems();
    closeworld();
    delpers(pname(mynum));
    openworld();
    sprintf(ms, "\001p%s\377 has just died.\n", pname(mynum));
    sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, ploc(mynum), ms);
    sprintf(ms, "[\001p%s\377 has been slain by \001p%s\377]\n", pname(mynum),
	    pname(array[0]));
    sendsys(pname(mynum), pname(mynum), SYS_WIZ, ploc(mynum), ms);
    setpstr(mynum, 40);
    setpscore(mynum, pscore(mynum) / 2);
    calibme();
    loseme();
    crapup("Oh dear... you seem to be slightly dead.\n");
  }

#ifdef WIMPY
  if (pstr(mynum) < 20 && ptstflg(mynum, pfl(Wimp)))
    wimpyflee();
#endif

  me_cal = True;			/* Queue an update when ready */
}

void breakitem(int x)
{
  switch (x) {
    default:
      bprintf("You can't do that.\n");
      break;
    case OBJ_STONE:
      sys_reset();
      break;
    case OBJ_ROCK:
      bprintf("You smash it apart to reveal a gem inside.\n");
      create(OBJ_GEM);
      setoloc(OBJ_GEM, oloc(OBJ_ROCK), ocarrf(OBJ_ROCK));
      destroy(OBJ_ROCK);
      break;
    case -1:
      bprintf("What's that?\n");
      break;
  }
}

void wreckdoor()
{
  int a;
  char ms[60];

  if ((a = fobna("door")) == -1)
    bprintf("Sorry sir, no doors here.\n");
  else if (state(a) == 0)
    bprintf("It's already open.\n");
  else {
    setobjstate(a, 0);
    bprintf("The door flies open!\n");
    sprintf(ms, "%s hits the door...  It flies open!\n", pname(mynum));
    sillycom(ms);
    if (plevme >= LVL_ARCHWIZARD && plevme < LVL_GOD)
      broad("\001dYou hear the distinctive crunch of an Arch-Wizard meeting door.\n\377");
    else if (plevme >= LVL_GOD)
      broad("\001dYou hear the distinctive crunch of a god meeting door.\n\377");
  }
}

Boolean testpeace()
{
  return (shimge[0] ? True : False) || ltstflg(ploc(mynum), lfl(Peaceful));
}

void woundmn(int mon, int am)
{
  int a;
  char ms[128];

  a = pstr(mon) - am;
  setpstr(mon, a);
  if (a >= 0)
    mhitplayer(mon);
  else {
    dumpstuff(mon, ploc(mon));
    sprintf(ms, "%s has died.\n", pname(mon));
    sendsys(" ", " ", SYS_TXT_TO_R, ploc(mon), ms);
    sprintf(ms, "[%s has died]\n", pname(mon));
    setploc(mon, RM_DEAD1);
    pclrflg(mon, pfl(Possessed));
    sendsys(pname(mynum), pname(mynum), SYS_WIZ, ploc(mon), ms);
  }
}

void fleecom()
{
  char s[128];

  if (!in_fight)
    dogocom();
  else {
    if (iscarrby(OBJ_RUNESWORD, mynum)) {
      bprintf("The sword won't let you!\n");
      return;
    }
    sprintf(s,"\001s%s\377%s drops things as %s make a frantic attempt to escape.\n\377", pname(mynum), pname(mynum), psex(mynum) ? "she" : "he");
    sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, ploc(mynum), s);
    sendsys(pname(mynum), pname(mynum), SYS_FIGHT_OVER, ploc(mynum), "");
    setpscore(mynum, pscore(mynum) - pscore(mynum) / 33); /* lose 3% */
    calibme();
    in_fight = 9999;
    if (i_follow) {
      bprintf("You stopped following.\n");
      i_follow = False;
    }
  on_flee_event();
  dogocom();
  }
}

void mhitplayer(int mon)
{
  int a, cth, i, x[4];
  
  if (testpeace() || ploc(mon) != ploc(mynum) || mon < 0)
    return;
  if (mon == MOB_PRIEST && randperc() < 25) {
    bprintf("The Dark Elven Priest fires a magic missile at you.\n");
    bprintf("You are struck!\n");
    wounded(10);
    return;
  }
  if (mon == MOB_HERMIT && randperc() < 20) {
    bprintf("Bolts of fire leap from the fingers of The Hermit.\n");
    bprintf("You are struck!\n");
    wounded(10);
    return;
  }
  if (mon == MOB_SHAZARETH && randperc() < 25) {
    bprintf("Shazareth fires an icy ray at you.\n");
    bprintf("You are struck!\n");
    wounded(10);
    return;
  }
  if (mon == MOB_SNAKE && randperc() < 30) {
    bprintf("The Snake bites you with its fangs!\n");
    bprintf("OUCH!\n");
    wounded(25);
    return;
  }
  a = randperc();
  cth = 57;
  if (iscarrby(OBJ_HORSESHOE, mon))
    cth += 13;
  for (i = 0; i < numobs; i++)
    if (iswornby(i, mynum) && otstbit(i, ofl(Armour)))
      cth -= obyte(i, 1);
  x[0] = mon;
  x[1] = (a < cth) ? rand() % damof(mon) : -1;
  x[2] = -1;
  sendsys(pname(mynum), pname(mon), SYS_ATTACK, ploc(mon), (char *)x);
}

static char *wordl1[] = {
  "counter",
  "retributive",
  "follow-up",
  "retaliatory",
  "return",
  "follow-through"
};

static char *wordl2[] = {
  "whack",
  "thrust",
  "stab",
  "punch",
  "blow",
  "swing",
  "swipe",
  "stab",
  "lunge",
  "clout",
  "jab",
  "cut"
};

static char *wordl3[] = {
  "reeling",
  "sprawling",
  "spinning",
  "staggering",
  "to the floor",
  "backwards",
  "to the ground"
};

static char *wordl4[] = {
  "hurt",
  "numbed",
  "jolted",
  "dazed",
  "bruised",
  "wounded",
  "winded",
  "stunned",
  "jarred",
  "troubled"
};

static char *wordl5[] = {
  "viciousness",
  "fierceness",
  "savageness",
  "speed",
  "power",
  "energy",
  "weight",
  "violence",
  "strength",
  "vigour",
  "momentum",
  "might"
};

static char *wordl6[] = {
  "hasty",
  "pathetic",
  "tame",
  "bungled",
  "weak",
  "feeble",
  "poor",
  "limp",
  "clumsy"
};

static char *wordl7[] = {
  "comfortably",
  "narrowly",
  "easily",
  "simply",
  "effortlessly"
};

static char *wordl8[] = {    
  "side-stepped",
  "dodged",
  "shrugged off",
  "absorbed",
  "parried",
  "ducked",
  "eluded",
  "matched"
};

static char *wordl9[] = {
  "side-step",
  "dodge",
  "shrug off",
  "absorb",
  "parry",
  "duck",
  "elude",
  "evade",
  "avoid"
};

static char *wordl10[] = {
  "aggressive",
  "wild",
  "dreadful",
  "mighty",
  "severe",
  "punishing",
  "weighty",
  "furious",
  "vicious",
  "tremendous",
  "savage",
  "forceful"
};

static char *wordl11[] = {
  "hit",
  "strike",
  "strike home at",
  "thrash",
  "wallop",
  "smite",
  "beat",
  "take aim at",
  "bash",
  "catch"
};

static char *wordl12[] = {
  "But",
  "Yet",
  "Dazedly",
  "Groggily",
  "With renewed vigour",
  "Indomitably",
  "With a vast effort",
  "Summoning strength",
  "But courageously",
  "Gritting your teeth"
};

static char *wordl13[] = {
  "revitalise",
  "bear up",
  "pull together",
  "pull through",
  "compose",
  "recover",
  "concentrate",
  "carry on"
};

static char *wordl14[] = {
  "charge back",
  "head back",
  "stagger forward",
  "press forward",
  "wade",
  "start",
  "advance",
  "hurl yourself",
  "launch yourself",
  "throw yourself"
};

static char *wordl15[] = {
  "fight",
  "slaughter",
  "carnage",
  "engagement",
  "melee",
  "contest",
  "battle",
  "action",
  "tussle",
  "fray",
  "combat"
};

static char *wordl16[] = {
  "murdered",
  "finished",
  "slaughtered",
  "exterminated",
  "killed",
  "crushed",
  "made history of",
  "made away with"
};
	    
#define RNDWORD(w) (w[rand() % (sizeof(w) / sizeof(char *))])

void combatmessage(int victim, int attacker, int wpn, int ddn)
{
  char name[128], *ptr;

  if (victim == -1 || attacker == -1)
    return;
  if (randperc() > 60 && mynum == attacker)
    strcpy(name, psex(victim) ? "her" : "him");
  else
    sprintf(name, "\001p%s\377", pname(victim != mynum ? victim : attacker));
  if (attacker == mynum && ddn != -1) {
    ptr = RNDWORD(wordl2);
    if (wpn == -1) {
      if (randperc() > 40)
	bprintf("Your");
      else
        bprintf("The %s of your", RNDWORD(wordl5));
      bprintf(" %s %s sends %s %s!", RNDWORD(wordl1), ptr, name, RNDWORD(wordl3));
    }
    else
      if (randperc() > 40)
	bprintf("You %s %s with a %s %s!", RNDWORD(wordl11), name, RNDWORD(wordl10), ptr);
      else
        bprintf("You give %s a %s %s with the %s!", name, RNDWORD(wordl10), ptr, oname(wpn));
    if (pstr(victim) - ddn < 0)
      bprintf("\nYour last %s %s %s!", ptr, RNDWORD(wordl16), name);
  }
  if (attacker == mynum && ddn == -1) {
    if (randperc() > 20)
      bprintf("Your %s %s %s at %s is %s %s.", RNDWORD(wordl6), RNDWORD(wordl1), RNDWORD(wordl2), name, RNDWORD(wordl7), RNDWORD(wordl8));
    else
      bprintf("Your %s %s %s at %s is %s.", RNDWORD(wordl10), RNDWORD(wordl1), RNDWORD(wordl2), name, RNDWORD(wordl8));
  }
  if (victim == mynum && ddn != -1) {
    if (wpn != -1 && randperc() > 60)
      bprintf("%s gives you a %s %s with %s %s!", name, RNDWORD(wordl10), RNDWORD(wordl2), his_or_her(attacker), oname(wpn));
    else
      bprintf("You are %s by the %s of a %s %s %s!", RNDWORD(wordl4), RNDWORD(wordl5), RNDWORD(wordl2), randperc() > 50 ? "by" : "from", name);
    bprintf("\n%s you %s, and %s into the %s.", RNDWORD(wordl12), RNDWORD(wordl13), RNDWORD(wordl14), RNDWORD(wordl15));
  }
  if (victim == mynum && ddn == -1) {
    if (randperc() > 20)
      bprintf("You %s %s a %s %s %s %s.", RNDWORD(wordl7), RNDWORD(wordl9), RNDWORD(wordl6), RNDWORD(wordl2), randperc() > 50 ? "by" : "from", name);
    else
      bprintf("You %s a %s %s %s %s.", RNDWORD(wordl9), RNDWORD(wordl10), RNDWORD(wordl2), randperc() > 50 ? "by" : "from", name);
  }
  if (attacker != mynum && victim != mynum && ddn != -1) {
    if (wpn == -1) {
      if (randperc() > 50)
        bprintf("\001p%s\377 gives %s a %s %s that sends %s %s!", pname(attacker), name, RNDWORD(wordl10), RNDWORD(wordl2), psex(victim) ? "her" : "him", RNDWORD(wordl3));
      else
        bprintf("%s is %s by a %s %s from \001p%s\377!", name, RNDWORD(wordl4), RNDWORD(wordl10), RNDWORD(wordl2), pname(attacker));
    }
    else
      if (randperc() > 40) {
	do {
          ptr = RNDWORD(wordl11);
        } while (index(ptr,' ') != (char *) 0);
        bprintf("\001p%s\377 %s%s %s with a %s %s!", pname(attacker), ptr, ptr[strlen(ptr)-1] == 'h' ? "es" : "s", name, RNDWORD(wordl10), RNDWORD(wordl2));
      }
      else
	bprintf("\001p%s\377 gives %s a %s %s with the %s!", pname(attacker), name, RNDWORD(wordl10), RNDWORD(wordl2), oname(wpn));
    }
    if (attacker != mynum && victim != mynum && ddn == -1) {
      do {
        ptr = RNDWORD(wordl9);
      } while (index(ptr,' ') != (char *) 0 || ptr[strlen(ptr)-1] == 'y');
      if (randperc() > 20)
        bprintf("%s %s %ss a %s %s %s \001p%s\377.", name, RNDWORD(wordl7),
		ptr, RNDWORD(wordl6), RNDWORD(wordl2),
		randperc() > 50 ? "by" : "from", pname(attacker));
      else
        bprintf("%s %ss a %s %s %s \001p%s\377.", name, ptr, RNDWORD(wordl10),
		RNDWORD(wordl2), randperc() > 50 ? "by" : "from",
		pname(attacker));
    }
    bprintf("\n");
}

#ifdef WIMPY
/*
 * Wimpy mode flee.  Force the player to flee through one of the room
 * exits.  If no exits, oh well...
 */
wimpyflee()
{
  int i;
  static char *dn[] = {"north", "east", "south", "west", "up", "down"};
  char c[30];

  for (i=0; i<6; i++) {
	if (getexit(ploc(mynum), i)) {
		bprintf("You feel your legs run %s!\n", dn[i]);
		sprintf(c, "flee %s", dn[i]);
		addforce(c);
		return;
	}
  }
  bprintf("Your legs try to run away but fail!\n");
}
#endif

/*
**  Movement routines (north, south, go, etc.)
*/

#include <strings.h>
#include "kernel.h"
#include "parse.h"
#include "macros.h"
#include "objects.h"
#include "mobiles.h"
#include "locations.h"
#include "lflags.h"
#include "move.h"
#include "support.h"
#include "blood.h"
#include "new2.h"
#include "tk.h"
#include "blib.h"
#include "pflags.h"

static char sccsid[] = "@(#)move.c	4.100.0 (IthilMUD)	6/02/90";

static char *exittxt[] = {
  "north", "east", "south", "west", "up", "down",
  "n", "e", "s", "w", "u", "d",
  0
};

static int exitnum[] = {
  1, 2, 3, 4, 5, 6,
  1, 2, 3, 4, 5, 6
};

int dodirn(int n)
{
  char block[128], ms[128];
  int i, pc;
  int newch, drnum, droff;

  if (in_fight > 0 && in_fight != 9999) {
    bprintf("You can't just stroll out of a fight!\n");
    bprintf("If you wish to leave, you must FLEE in a direction.\n");
    return -1;
  }
  if (iscarrby(OBJ_CUP, mynum) && (i = alive(MOB_SERAPH)) != -1
      && ploc(i) == ploc(mynum)) {
    bprintf("The Seraph says 'I cannot allow you to leave this place with the Holy Relic.'\n");
    return -1;
  }
  if (iscarrby(OBJ_RUNESWORD, mynum) && ploc(MOB_GOLEM) == ploc(mynum)
      && !EMPTY(pname(MOB_GOLEM))) {
    bprintf("\001cThe Golem\377 bars the doorway!\n");
    return -1;
  }
  n -= 2;
  if (chkcrip() || chksitting())
    return -1;
  newch = getexit(ploc(mynum), n);
  if (newch == 10200) {
    if (state(OBJ_PIT) != 0) {
      bprintf("That doesn't look like a very good idea!\n");
      return -1;
    }
    newch = -162;
  }
  if (newch == 10000)
    newch = -2445;
  if (newch == 10001)
    newch = -2444;
  if (newch > 999 && newch < 2000) {
    drnum = newch - 1000;
    droff = drnum ^ 1;		/* other door side */
    if (state(drnum)) {
      if (!EQ(oname(drnum), "door") || isdark()
	  || EMPTY(olongt(drnum, state(drnum))))
	bprintf("You can't go that way.\n");
      else
	bprintf("The door is closed.\n");
      return -1;
    }
    newch = oloc(droff);
  }
  if (ltstflg(newch, lfl(Private) | lfl(OnePerson))) {
    pc = 0;
    for (i = 0; i < MAX_USERS; i++)
      if (!EMPTY(pname(i)) && ploc(i) == newch)
	pc++;
    if (pc > (ltstflg(newch, lfl(Private)) ? 1 : 0)) {
      bprintf("I'm sorry, that room is currently full.\n");
      return -1;
    }
  }
  if (newch == -139) {
    if (!iswornby(OBJ_SHIELD_2, mynum) && !iswornby(OBJ_SHIELD_3, mynum)
	&& !iswornby(OBJ_SHIELD, mynum) && !iswornby(OBJ_SHIELD_4, mynum)) {
      bprintf("The intense heat drives you back.\n");
      return -1;
    }
    bprintf("The shield protects you from the worst of the lava's heat.\n");
  }
  if (state(OBJ_STONE_1) == 1 && newch == RM_OPERA19) {
    bprintf("You fall through a trapdoor!\n");
    teletrap(RM_OPERA36);
    return -1;
  }
  /* to deal with auto-transpot from heaven - JSM  11 March 1990 */
  if (newch == RM_HEAVEN14 || newch == RM_HEAVEN36 || newch == RM_HEAVEN27
	       || newch == RM_HEAVEN15 || newch == RM_HEAVEN16 || 
	       newch == RM_HEAVEN17 || newch == RM_HEAVEN18) {
    bprintf("You faintly hear voices, then feel an irresistable tugging.\n");
    teletrap(RM_VILLAGE25);
    bprintf("A nurse tells you that they almost didn't pull you through.\n");
    return 0;
  }
  if (n == 0) {
    if ((i = alive(MOB_DEFENDER)) != -1 &&plev(mynum) < LVL_WIZARD
	&& ploc(i) == ploc(mynum)) {
      bprintf("\001pThe Defender\377 says 'None shall pass!'\n");
      woundmn(i, 0);
      return -1;
    }
  }
  if (n == 5) {	       /* can't go down unless empty-handed */
    if ((ploc(mynum) == RM_CATACOMB31 || ploc(mynum) == RM_VALLEY93)
	&& !unencumbered())
      if (ploc(mynum) == RM_CATACOMB31) {
	bprintf("A mysterious force blocks your passage.\n");
	if (ploc(MOB_BEGGAR) == ploc(mynum))
	  sendsys("The Beggar", "The Beggar", -10003, ploc(mynum),
		  "To continue on, you must forego all worldly possessions.");
	return -1;
      }
      else {
	bprintf("The steep and slippery sides of the pool make it impossible to climb down\nwithout dropping everything first.\n");
	return -1;
      }
  }
  if (n == 0) {
	if (iscarrby(OBJ_HOPE, mynum) && ploc(mynum) == RM_EFOREST33) {
	  bprintf("A mysterious force prevents you from going that way.\n");
	  return -1;
	}
  }
  if (n == 3) {
	if ((i = alive(MOB_CERBERUS)) != -1 && plev(mynum) < LVL_WIZARD &&
		   ploc(i) == ploc(mynum)){
	   bprintf("\001pCerberus\377 gives a warning growl.\n");
	   bprintf("\001pCerberus\377 won't let you go West!\n");
	   return -1;
    }
  }
  if ((n == 0) || (n == 1) || (n == 2)) {
       if ((i = alive(MOB_ASMADEUS)) != -1 && plev(mynum) < LVL_WIZARD &&
		  ploc(i) == ploc(mynum)){
	     bprintf("\001pAsmadeus\377 refuses to let you enter his museum.\n");
	     return -1;
       }
  }
  if (newch == RM_OPERA8 && n == 0 &&  state(OBJ_RUBBLE) == 0) {
	bprintf("You can't get through the rubble and construction.\n");
	return -1;
  }
  if (newch == RM_OPERA16 && n == 1 && !iscarrby(OBJ_PISTOL, mynum)) {
	bprintf("It's too dangerous to proceed without a gun.\n");
	return -1;
  }
  if (n == 2 || n == 5) {
    if ((i = alive(MOB_FIGURE)) != mynum && i != -1
	&& ploc(i) == ploc(mynum) && !iswornby(OBJ_ROBE, mynum)) {
      bprintf("\001pThe Figure\377 holds you back!\n");
      bprintf("\001pThe Figure\377 says 'Only true sorcerors may pass.'\n");
      return -1;
    }
  }
  if (n == 1) {
    if ((i = alive(MOB_VIOLA)) != mynum && i != -1
	&& ploc(i) == ploc(mynum) && !iscarrby(OBJ_FAN, i)) {
      bprintf("\001pViola\377 says 'How dare you come to our land!  Leave at once!'\n");
      return -1;
    }
  }
  if (ltstflg(newch, lfl(OnWater))) {
    if (plev(mynum) < LVL_WIZARD && !iscarrby(OBJ_RAFT, mynum) &&
	!iscarrby(OBJ_BOAT, mynum) && !iscarrby(OBJ_BOAT_1, mynum) &&
	!iscarrby(OBJ_BOAT_2, mynum)) {
      bprintf("You need a boat to go to sea!\n");
      return -1;
    }
  }
  if (ltstflg(newch, lfl(InWater))) {
    if (plev(mynum) < LVL_WIZARD) {
      bprintf("You'd surely drown!\n");
      return -1;
    }
  }
  if (newch >= 0) {
    bprintf("You can't go that way.\n");
    return -1;
  }
  if (n == 5 && ploc(mynum) == RM_OAKTREE8) {
    bprintf("You slide down the banister.  Wheee!\n");
    sprintf(block, "\001s%s\377%s%s%s\377",
	    pname(mynum), pname(mynum),
	    " slides down the banister shouting 'Yippeee...'\n");
    sendsys(pname(mynum), pname(mynum), -10000, ploc(mynum), block);
    sprintf(block, "\001s%s\377%s%s\377",
	    pname(mynum), pname(mynum),
	    " slides down the banister and lands at your feet.\n");
    sendsys(pname(mynum), pname(mynum), -10000, newch, block);

    if (oloc(OBJ_BUST) == newch) {
      bprintf("On your way down, you smash a valuable bust.\n");
      sprintf(block, "\001s%s\377%s%s%s%s\377",
	      pname(mynum), pname(mynum),
	      " smashed a valuable bust on ", his_or_her(mynum),
	      " way down.\n");
      sendsys(pname(mynum), pname(mynum), -10000, newch, block);
      destroy(OBJ_BUST);
      create(OBJ_BUST_1);
    }
    in_fight = 0;
    trapch(newch);
    return 0;
  }
  else {
    sprintf(ms, out_ms, exittxt[n]);
    sprintf(block, "\001s%s\377%s %s\n\377",
	    pname(mynum), pname(mynum), ms);
  }
  sendsys(pname(mynum), pname(mynum), -10000, ploc(mynum), block);
  sprintf(block, "\001s%s\377%s %s\n\377", pname(mynum),
	  pname(mynum), in_ms);
  sendsys(pname(mynum), pname(mynum), -10000, newch, block);
  in_fight = 0;
  trapch(newch);
  return 0;
}

int dogocom()
{
  int a;

  if (brkword() == -1) {
    bprintf("Go where?\n");
    return -1;
  }
  if (EQ(wordbuf, "rope"))
    strcpy(wordbuf, "up");
  if (EQ(wordbuf, "well"))
    strcpy(wordbuf, "down");
  if (EQ(wordbuf, "maiden"))
    strcpy(wordbuf, "east");
  if (EQ(wordbuf, "tree") && ploc(mynum) == -2300)
    strcpy(wordbuf, "up");
  if (EQ(wordbuf, "archway") && ploc(mynum) == -2441)
    strcpy(wordbuf, "south");
  if (EQ(wordbuf, "window")) {
    if (ploc(mynum) == -102)
      strcpy(wordbuf, "west");
    if (ploc(mynum) == -103)
      strcpy(wordbuf, "east");
  }
  if (EQ(wordbuf, "cauldron")) {
    bprintf("You'd be boiled alive!\n");
    return -1;
  }
  if ((a = chklist(wordbuf, exittxt, exitnum)) == -1) {
    bprintf("That's not a valid direction.\n");
    return -1;
  }
  return dodirn(a + 1);
}

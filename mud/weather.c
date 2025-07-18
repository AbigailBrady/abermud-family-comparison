/*
**  The game functions controlling the weather.  (Ones that do not need moved.)
*/

#include <strings.h>
#include "kernel.h"
#include "objects.h"
#include "pflags.h"
#include "oflags.h"
#include "lflags.h"
#include "weather.h"
#include "macros.h"
#include "new1.h"
#include "new2.h"
#include "support.h"
#include "parse.h"
#include "exec.h"
#include "tk.h"

/*
** Weather Routines
** Current weather defined by state of OBJ_SYS_WEATHER; states are
**	0 Sunny   1 Rain   2 Stormy   3 Snowing   4 Blizzard   5 Hailing
*/

void adjust_weather(weather_type new_setting)
{
  int x;

  x = state(OBJ_WEATHER);
  setobjstate(OBJ_WEATHER, new_setting);
  if (x != new_setting)
    sendsys(pname(mynum), pname(mynum), -10030, new_setting, "");
}

void set_weather(weather_type new_setting)
{
  if (plev(mynum) < LVL_WIZARD) {
    erreval();
    return;
  }
  adjust_weather(new_setting);
}

void longwthr()
{
  int a;

  if (randperc() < 70)
    return;
  a = randperc();
  if (a < 10)
      adjust_weather(hailing);
  else if (a < 50)
    adjust_weather(rainy);
  else if (a > 90)
    adjust_weather(stormy);
  else
    adjust_weather(sunny);
  return;
}

void wthrrcv(weather_type type)
{
  if (!outdoors())
    return;
  switch (modifwthr(type)) {
  case sunny:
    bprintf("\001cThe sun comes out of the clouds.\n\377");
    break;
  case rainy:
    bprintf("\001cIt has started to rain.\n\377");
    break;
  case stormy:
    bprintf("\001cDark clouds boil across the sky as a heavy storm breaks.\n\377");
    break;
  case snowing:
    bprintf("\001cIt has started to snow.\n\377");
    break;
  case blizzard:
    bprintf("\001cYou are half blinded by drifting snow, as a white, icy blizzard sweeps across\nthe land.\n\377");
    break;
  case hailing:
    bprintf("\001cYou run for cover as golf-ball sized hailstones begin to fall.\n\377");
  }
}

void showwthr()
{
  if (!outdoors())
    return;
  switch (modifwthr (state(OBJ_WEATHER))) {
  case rainy:
    if (ploc(mynum) > -199 && ploc(mynum) < -178) {
      bprintf("It is raining, a gentle mist of rain, which sticks to everything around\n");
      bprintf("you making it glisten and shine.  High in the skies above you is a rainbow.\n");
    }
    else
      bprintf("\001cIt is raining.\n\377");
    break;
  case stormy:
    bprintf("\001cThe skies are dark and stormy.\n\377");
    break;
  case snowing:
    bprintf("\001cIt is snowing.\377\n");
    break;
  case blizzard:
    bprintf("\001cA blizzard is howling around you.\377\n");
    break;
  case hailing:
    bprintf("\001cIt is hailing.\377\n");
  }
}

int
outdoors()
{
  return (ltstflg(ploc(mynum), lfl(Outdoors)));
}

/* Silly Section */

void sillycom(char *txt)
{
  char bk[256];
  
  sprintf(bk, txt, pname(mynum), pname(mynum));
  sendsys(pname(mynum), pname(mynum), -10000, ploc(mynum), bk);
}

void posecom()
{
  char x[128];

  if (plev(mynum) < LVL_MAGICIAN) {
    bprintf("You're not up to this yet.\n");
	return;
  }
  if (pstr(mynum) > 2) {
    switch (randperc() % 5) {
    case 0:
      sprintf(x, "\001s%%s\377%%s raises %s arms in mighty magical invocations.\n\377", his_or_her(mynum));
	  sillycom(x);
      bprintf("You make mighty magical gestures.\n");
      setpstr(mynum, pstr(mynum) - 2);
	  break;
    case 1:
      sillycom("\001s%s\377%s throws out one arm and sends a huge bolt of fire high into the sky.\n\377");
      bprintf("You toss a fireball high into the sky.\n");
      broad("\001cA massive ball of fire explodes high up in the sky.\n\377");
      setpstr(mynum, pstr(mynum) - 5);
	  break;
    case 2:
      sillycom("\001s%s\377%s turns casually into a hamster before resuming normal shape.\n\377");
      bprintf("You casually turn into a hamster before resuming normal shape.\n");
      setpstr(mynum, pstr(mynum) - 2);
      break;
    case 3:
      sillycom("\001s%s\377%s starts sizzling with magical energy.\n\377");
      bprintf("You sizzle with magical energy.\n");
      setpstr(mynum, pstr(mynum) - 2);
      break;
    case 4:
      sillycom("\001s%s\377%s begins to crackle with magical fire.\n\377");
      bprintf("You crackle with magical fire.\n");
      setpstr(mynum, pstr(mynum) - 2);
      break;
    }
  }
}

/* (C) Jim Finnis  (Yes, he really did write one or two routines.) */
void emotecom()
{
  char buf[100];

  if (plev(mynum) < LVL_NECROMANCER && !ptstflg(mynum, pfl(Emote))
      && pscore(mynum) != 1234 && !ltstflg(ploc(mynum), lfl(Party))) {
    bprintf("Your emotions are strictly limited!\n");
    return;
  }
  if (EMPTY(item1)) {
    bprintf("What do you want to emote?\n");
    return;
  }
  strcpy(buf, "\001P%s\377 ");
  getreinput(buf + 6);
  strcat(buf, "\n");
  sillycom(buf);
  bprintf("Ok\n");
}

/* (C) Rassilon (Brian Preble) */
void emotetocom()
{
  int a;

  if (plev(mynum) < LVL_NECROMANCER && !ptstflg(mynum, pfl(Emote))
      && !ltstflg(ploc(mynum), lfl(Party))) {
    bprintf("Your emotions are strictly limited!\n");
    return;
  }
  if (EMPTY(item1)) {
    bprintf("Emote to who?\n");
    return;
  }
  if ((a = pl1) == -1) {
    bprintf("No one with that name is playing.\n");
    return;
  }
  if (a == mynum) {
    bprintf("Good trick, that.\n");
    return;
  }
  if(EMPTY(txt2)) {
    bprintf("Emote what?\n");
    return;
  }
  sillytp(a, txt2);
  bprintf("Ok\n");
}

/* (C) Rassilon (Brian Preble) */
void echocom()
{
  char x[128];

  if (plev(mynum) < LVL_ARCHWIZARD && !ptstflg(mynum, pfl(Echo))) {
    bprintf("You hear echos.\n");
    return;
  }
  getreinput(x);
  if (EMPTY(x)) {
    bprintf("ECHO what?\n");
    return;
  }
  sendsys(pname(mynum), pname(mynum), -10005, ploc(mynum), x);
  bprintf("Ok\n");
}

/* (C) Rassilon (Brian Preble) */
void echoallcom()
{
  char x[128], a[128];

  if (plev(mynum) < LVL_ARCHWIZARD && !ptstflg(mynum, pfl(Echo))) {
    bprintf("You hear echos.\n");
    return;
  }
  getreinput(x);
  if (EMPTY(x)) {
    bprintf("Echo what?\n");
    return;
  }
  sprintf(a, "\001c%s\n\377", x);
  broad(a);
  bprintf("Ok\n");
}

/* (C) Rassilon (Brian Preble) */
void echotocom()
{
    int b;

    if (plev(mynum) < LVL_ARCHWIZARD && !ptstflg(mynum, pfl(Echo))) {
        bprintf("You hear echos.\n");
        return;
    }
    if (EMPTY(item1)) {
	bprintf("Echo to who?\n");
	return;
    }
    if ((b = pl1) == -1) {
	bprintf("No one with that name is playing.\n");
	return;
    }
    if (b == mynum) {
	bprintf("What's the point?\n");
	return;
    }
    if (EMPTY(txt2)) {
        bprintf("What do you want to echo to them?\n");
        return;
    }
    sendsys(pname(b), pname(mynum), -10007, ploc(mynum), txt2);
    bprintf("Ok\n");
}

void praycom()
{
  sillycom("\001s%s\377%s falls down and grovels in the dirt.\n\377");
  bprintf("You fall down and grovel in the dirt.\n");
}

int cancarry(int plyr)
{
  int a;
  int b = 0;

  if (plev(plyr) >= LVL_WIZARD || plev(plyr) < 0)
    return 1;
  for (a = 0; a < numobs; a++)
    if (iscarrby(a, plyr) && !iswornby(a, plyr) &&
	!otstbit(a, ofl(Destroyed)))
      b++;
  return b < plev(plyr) + 5;
}

int
roomdark()
{
  int c;
  int dark = 0;

  if (ltstflg(ploc(mynum), lfl(Dark)))
    dark = 1;

  /* Do any objects in this location shed light? */
  if (dark)
    for (c = 0; c < numobs; c++)
      if (otstbit(c, ofl(Lit)))
	if (ishere(c) || ((ocarrf(c) == 1 || ocarrf(c) == 2)
			  && ploc(oloc(c)) == ploc(mynum)))
	  dark = 0;

  /* Are any players glowing here? */
  if (dark)
    for (c = 0; c < MAX_CHARS; c++)
      if (!EMPTY(pname(c)) && ploc(c) == ploc(mynum) && ptstflg(c, pfl(Glowing)))
	dark = 0;

  return dark;
}

int
isdark()
{
  if (roomdark() && plev(mynum) < LVL_WIZARD)
    return 1;
  else
    return 0;
}

int
modifwthr (int n)
{
  if (!ltstflg(ploc(mynum), lfl(Cold)))
    return n == snowing || n == blizzard ? n - 2 : n;
  else
    return n == rainy || n == stormy ? n + 2 : n;
}

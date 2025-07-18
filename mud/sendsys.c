/*
**  Send messages to the other AberMUD processes using the world file
*/

#include <strings.h>
#include "kernel.h"
#include "macros.h"
#include "objects.h"
#include "mobiles.h"
#include "verbs.h"
#include "sendsys.h"
#include "pflags.h"
#include "oflags.h"
#include "version.h"
#include "weather.h"
#include "parse.h"
#include "support.h"
#include "blib.h"
#include "tk.h"
#include "exec.h"
#include "new1.h"
#include "opensys.h"
#include "mud.h"
#include "new2.h"
#include "objsys.h"
#include "magic.h"
#include "uaf.h"
#include "extra.h"
#include "blood.h"
#include "change.h"
#include "zones.h"
#include "bprintf.h"
#include "frob.h"
#include "god.h"
#include "locdir.h"
#include "key.h"
#include "lflags.h"

static int iraq;

void
sendsys(char *to, char *from, int codeword, int chan, char *text)
{
  int block[128];
  int *i;

  block[1] = codeword;
  block[0] = chan;
  sprintf((char *)&block[2], "%s\001%s\001", to, from);
  if (codeword != SYS_VIS && codeword != SYS_ATTACK)
    strcpy((char *)&block[64], text);
  else {
    i = (int *)text;
    block[64] = i[0];
    block[65] = i[1];
    block[66] = i[2];
  }
  send2(block);
}

void gamrcv(int *blok)
{
  char zb[150], to[40], from[40];
  char text[256], nameme[40];
  int  *i, isme;

  lowercase(strcpy(nameme, pname(mynum)));
  isme = split(blok, to, from, text, nameme);
  if (blok[1] == SYS_FIGHT_OVER && fpbns(to) == fighting) {
    in_fight = 0;
    fighting = -1;
    setpfighting(mynum, -1);
  }
  i = (int *)text;
  switch (blok[1]) {
  case SYS_VIS:
    setpvis(i[0], i[1]);
    break;
  case SYS_CRASH:
    bprintf("Something Very Evil Has Just Happened...\n");
    loseme();
    crapup("Bye Bye Cruel World....");
  case SYS_FROB:
    if (isme) {
      sscanf(text, "%d.%d.%d.", &my_lev, &my_sco, &my_str);
      setpstr(mynum, my_str);
      setplev(mynum, my_lev);
      setpscore(mynum, my_sco);
      calibme();
    }
    break;
  case SYS_SNOOP_OFF:
    if (isme)
      snoopd = -1;
    break;
  case SYS_SNOOP_ON:
    if (isme)
      snoopd = fpbns(from);
    break;
  case SYS_TXT_TO_R:
    if (isme != 1 && blok[0] == ploc(mynum))
      bprintf("%s", text);
    break;
  case SYS_WEATHER:
    wthrrcv(blok[0]);
    break;
  case SYS_ATTACK:
    if (ploc(mynum) == blok[0])
      {
	if (isme != 1)
	  {
	    if (i[0] == mynum)
	      break;
	    if (EMPTY(pname(i[0])))
	      return;
	    combatmessage(fpbns(to), i[0], i[2], i[1]);
	  }
      }
    bloodrcv(i, isme);
    break;
  case SYS_SUMMON:
    if (isme) {
      ades = blok[0];
      sscanf(text, "%d", &iraq);
      if (plev(mynum) >= LVL_WIZARD) {
	bprintf("\001p%s\377 tried to summon you.\n", from);
	return;
      }
      if (iraq)
	bprintf("You drop everything as you are summoned by \001p%s.\377\n", from);
      else
	bprintf("You are summoned by \001p%s.\377\n", from);
      tdes = True;
    }
    break;
  case SYS_WIZ_SUMMON:
    if (isme) {
      ades = blok[0];
      sscanf(text, "%d", &iraq);
      if (iraq)
	bprintf("You drop everything as you are summoned by the Arch-Wizard \001p%s.\377\n", from);
      else
	bprintf("You are summoned by the Arch-Wizard \001p%s.\377\n", from);
      tdes = True;
    }
    break;
  case SYS_GOD_SUMMON:
    if (isme) {
      ades = blok[0];
      sscanf(text, "%d", &iraq);
      if (iraq)
	bprintf("You drop everything as you are summoned by the God \001p%s.\377\n", from);
      else
	bprintf("You are summoned by the God \001p%s.\377\n", from);
      tdes = True;
    }
    break;
  case SYS_ZAP:
    if (isme) {
      if ((plev(mynum) >= LVL_WIZARD && plev(fpbns(from)) < LVL_ARCHWIZARD) ||
	  (plev(mynum) >= LVL_ARCHWIZARD && plev(fpbns(from)) < LVL_GOD) ||
	  (plev(mynum) >= LVL_GOD && plev(fpbns(from)) >= LVL_GOD))
	bprintf("\001p%s\377 cast a lightning bolt at you.\n", from);
      else {
	bprintf("\001A\033[1m\377A massive lightning bolt arcs down out of ");
	bprintf("the sky to strike you between\nthe eyes.\001A\033[0m\377\n");
	sprintf(zb, "[\001p%s\377 zapped \001p%s\377]\n", from, pname(mynum));
	sendsys(pname(mynum), pname(mynum), SYS_WIZ, ploc(mynum), zb);
	zapped = True;
	delpers(pname(mynum));
	sprintf(zb, "\001s%s\377%s has just died.\n\377",
		pname(mynum), pname(mynum));
	sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, ploc(mynum), zb);
	loseme();
	bprintf("You have been utterly destroyed by %s.\n", from);
	crapup("Bye Bye.... Slain by a Thunderbolt");
      }
    }
    else if (blok[0] == ploc(mynum))
      bprintf("\001A\033[1m\377\001cA massive lightning bolt strikes \377\001D%s\377\001c.\n\377\001A\033[0m\377", to);
    break;
  case SYS_SHOUT:
    if (!isme) {
      if (ptstflg(mynum, pfl(NoShout)))
	return;
      if (blok[0] == ploc(mynum) || plev(mynum) >= LVL_WIZARD)
	bprintf("\001P%s\377\001d shouts '%s'\n\377", from, text);
      else
	bprintf("\001dA voice shouts '%s'\n\377", text);
    }
    break;
  case SYS_SAY:
    if (!isme && blok[0] == ploc(mynum))
      bprintf("\001P%s\377\001d says '%s'\n\377", from, text);
    break;
  case SYS_TELL:
    if (isme == 1)
      bprintf("\001P%s\377\001d tells you '%s'\n\377", from, text);
    break;
  case SYS_MSG_TO_Rd:
    if (isme != 1 && blok[0] == ploc(mynum))
      bprintf("\001d%s\n\377", text);
    break;
  case SYS_MSG_TO_PP:
    if (isme == 1)
      bprintf("\001P%s\377\001d %s\n\377", from, text);
    break;
  case SYS_MSG_TO_Pd:
    if (isme == 1)
      bprintf("\001d%s\n\377", text);
    break;
  case SYS_EXORCISE:
    if (isme == 1)
      crapup("You have been kicked off!");
    else
      bprintf("%s has been kicked off.\n", to);
    break;
  case SYS_CURE:
    if (isme == 1) {
      bprintf("All your ailments have been cured.\n");
      pclrflg(mynum, pfl(Dumb) | pfl(Crippled) | pfl(Blind) | pfl(Deaf));
    }
    break;
  case SYS_CRIPPLE:
    if (isme == 1) {
      if (plev(mynum) < LVL_WIZARD) {
	bprintf("You have been magically crippled.\n");
	psetflg(mynum, pfl(Crippled));
      }
      else
	bprintf("\001p%s\377 tried to cripple you.\n", from);
    }
    break;
  case SYS_DUMB:
    if (isme == 1) {
      if (plev(mynum) < LVL_WIZARD) {
	bprintf("You have been struck magically dumb.\n");
	psetflg(mynum, pfl(Dumb));
      }
      else
	bprintf("\001p%s\377 tried to mute you.\n", from);
    }
    break;
  case SYS_FORCE:
    if (fpbns(from) != -1)
      if ((fpbns(from) >= LVL_GOD) ||
	  ((plev(fpbns(from)) >= LVL_ARCHWIZARD &&
	    plev(mynum) < LVL_ARCHWIZARD))) {
	if (isme)
	  addforce(text);
	break;
      }
    if (isme == 1) {
      if (plev(mynum) < LVL_WIZARD) {
	bprintf("\001p%s\377 has forced you to %s\n", from, text);
	addforce(text);
      }
      else
	bprintf("\001p%s\377 tried to force you to %s\n", from, text);
    }
    else if (plev(mynum) >= LVL_WIZARD)
      bprintf("[\001p%s\377 forced \001p%s\377]\n[Force:%s]\n",
	      from, to, text);
    break;
  case SYS_WIZ_SHOUT:
    if (ptstflg(mynum, pfl(NoShout)))
      break;
    if (isme != 1)
      bprintf("\001p%s\377 shouts '%s'\n", from, text);
    break;
  case SYS_BLIND:
    if (isme == 1) {
      if (plev(mynum) < LVL_WIZARD) {
	bprintf("You have been struck magically blind.\n");
	psetflg(mynum, pfl(Blind));
      }
      else
	bprintf("\001p%s\377 tried to blind you.\n", from);
    }
    break;
  case SYS_DEAF:
    if (isme == 1) {
      if (plev(mynum) < LVL_WIZARD) {
	bprintf("You have been magically deafened.\n");
	psetflg(mynum, pfl(Deaf));
      }
      bprintf("\001p%s\377 tried to deafen you.\n", from);
    }
    break;
  case SYS_MISSILE:
    if (iam(from))
      break;
    if (ploc(mynum) == blok[0]) {
      bprintf("Bolts of fire leap from the fingers of \001p%s\377\n", from);
      if (isme == 1) {
	bprintf("You are struck!\n");
	wounded(atoi(text));
      }
      else
	bprintf("\001p%s\377 is struck.\n", to);
    }
    break;
  case SYS_CH_SEX:
    if (isme == 1) {
      bprintf("Your sex has been magically changed!\n");
      setpsex(mynum, 1 - psex(mynum));
      bprintf("You are now ");
      if (psex(mynum))
	bprintf("female.\n");
      else
	bprintf("male.\n");
      calibme();
    }
    break;
  case SYS_FIREBALL:
    if (iam(from))
      break;
    if (ploc(mynum) == blok[0]) {
      bprintf("\001A\033[31m\377\001p%s\377 casts a fireball.\001A\033[0m\377\n", from);
      if (isme == 1) {
	bprintf("You are struck!\n");
	wounded(atoi(text));
      }
      else
	bprintf("\001p%s\377 is struck.\n", to);
    }
    break;
  case SYS_SHOCK:
    if (iam(from))
      break;
    if (isme == 1) {
      bprintf("\001A\033[1;36m\377\001p%s\377 touches you, giving you a strong electric shock!\001A\033[0m\377\n", from);
      wounded(atoi(text));
    }
    break;
  case SYS_MSG_TO_P:
    if (isme == 1)
      bprintf("%s\n", text);
    break;
  case SYS_HEAL:
    if (iam(from))
      break;
    if (isme == 1) {
      bprintf("\001p%s\377 heals all your wounds.\n", from);
      setpstr(mynum, maxstrength(mynum));
    }
    break;
  case SYS_WIZ:
    if (text[0] == '[' && ptstflg(mynum, pfl(Quiet)))
      break;
    if (plev(mynum) >= LVL_WIZARD)
      bprintf("\001A\033[1;33m\377%s\001A\033[0m\377", text);
    break;
  case SYS_FROST:
    if (iam(from))
      break;
    if (ploc(mynum) == blok[0]) {
      bprintf("\001A\033[1;37m\377\001p%s\377 fires an icy ray!\001A\033[0m\377\n", from);
      if (isme == 1) {
	bprintf("You are chilled to the bone!\n");
	wounded(atoi(text));
      }
      else
	bprintf("\001p%s\377 is struck.\n", to);
    }
    break;
  case SYS_AWIZ:
    if (plev(mynum) >= LVL_ARCHWIZARD)
      bprintf("\001A\033[1;33m\377%s\001A\033[0m\377", text);
    break;
  }
}

void dosumm(int loc)
{
  char ms[128];

  sprintf(ms, "\001s%s\377%s vanishes in a puff of smoke.\n\377",
	  pname(mynum), pname(mynum));
  sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, ploc(mynum), ms);
  sprintf(ms, "\001s%s\377%s appears, looking bewildered.\n\377",
	  pname(mynum), pname(mynum));
  if (iraq)
    on_flee_event();
  sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, loc, ms);
  trapch(loc);
}

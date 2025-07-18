#include "kernel.h"
#include "locdir.h"
#include "key.h"
#include "macros.h"
#include "objects.h"
#include "mobiles.h"
#include "locations.h"
#include "sendsys.h"
#include "oflags.h"
#include "extra.h"
#include "new1.h"
#include "new2.h"
#include "blib.h"
#include "support.h"
#include "bprintf.h"
#include "mud.h"
#include "exec.h"
#include "zones.h"
#include "tk.h"
#include "parse.h"

static int jumtb[] = {
  /* FROM,	TO */
    -1050,	-662,
    -1082,	-1053,
    -1600,	-620,
/*
    -4,		RM_PIT1,
    -5,		RM_PIT1,
    -2431,	RM_PIT1,
    -2432,	RM_PIT1,
    -2433,	RM_PIT1,
    -2434,	RM_PIT1,
*/
    -702,	-2207,
    -102,	-103,
    -103,	-102,
    0, 0
};

/*
** Brian Preble (rassilon@eddie.mit.edu)
** (Mostly rewritten by Charles Hannum)
** Gives detailed help on commands (well, fairly detailed)
*/
int
showhelp(char *verb)
{
  char line[80];
  int  scanreturn, prefix;
  char ch;
  FILE *fp;
  long pos;

  if ((fp = openlock(FULLHELP, "r")) == NULL) {
    bprintf("Someone's editing the help file.\n");
    return -1;
  }
  uppercase(verb);
  do {
    do {
      if ((fgets(line, 80, fp) == NULL)) {
	fclose(fp);
	return 0;
      }
    } while(strcmp(line, "^\n"));
    pos = ftell(fp);
    do {
      if ((fgets(line, 80, fp) == NULL)) {
        fclose(fp);
        return 0;
      }
      if (!strcmp(line, "\n"))
        break;
      scanreturn = strncmp(line, verb, strlen(verb));
    } while (scanreturn);
  } while (scanreturn);

  fseek(fp, pos, 0);
  prefix = True;
  while ((fgets(line, 80, fp) != NULL) && strcmp(line, "\n"))
    if (prefix) {
      bprintf("\nUsage:\t%s", line);
      prefix = False;
    }
    else
      bprintf("\t%s", line);

  do {
    bprintf("%s", line);
  } while (fgets(line, 80, fp) && strcmp(line, "^\n"));

  bprintf("\n");
  fclose(fp);
  return -1;
}

void
helpcom()
{
  int a;

  if (item1[0])
    {
      if (showhelp(item1) == -1)
	return;
      if (vichere(&a) == -1)
	return;
      if (phelping(mynum) != -1)
        {
	  sillytp(a, "stopped helping you.");
	  bprintf("Stopped helping \001p%s\377.\n", pname(phelping(mynum)));
        }
      if (a == mynum)
        {
	  bprintf("You are beyond help.\n");
	  return;
        }
      setphelping(mynum, a);
      sillytp(a, "has offered to help you.");
      bprintf("Started helping \001p%s\377.\n", pname(phelping(mynum)));
      return;
    }
  closeworld();
  bprintf("\n\001f%s\377", HELP1);
  if (plev(mynum) >= LVL_WIZARD)
    {
      sig_alon();
      pwait();
      sig_aloff();
      bprintf("\n\001f%s\377", HELP2);
    }
  if (plev(mynum) >= LVL_ARCHWIZARD)
    {
      sig_alon();
      pwait();
      sig_aloff();
      bprintf("\n\001f%s\377", HELP3);
    }
  if (plev(mynum) >= LVL_GOD)
    {
      sig_alon();
      pwait();
      sig_aloff();
      bprintf("\n\001f%s\377", HELP4);
    }
  bprintf("Type 'HELP [cmd]' for detailed help on a specific command.\n");
}

int
maxstrength(int p)
{
  return (plev(p) * 8 + 50);
}

int
ovalue(int ob)
{
  return (tscale() * obaseval(ob) / 9);
}

int
pvalue(int p)
{
  if (p >= MAX_USERS)
    return (damof(p) * 10 + pstr(p) * 2);
  else
    return (plev(p) * plev(p) * plev(p) * 10 + pstr(p) * 2);
}

void
valuecom()
{
  int a;

  if (((a = fpbns(item1)) != -1) && (pvis(a) <= plev(mynum)))
    {
      if (plev(mynum) >= LVL_WIZARD)
        {
	  bprintf("%s : %d points\n", pname(a), pvalue(a));
	  return;
	}
      else
        {
	  bprintf("Sorry, you can't tell.\n");
	  return;
	}
    }
  if ((a = fobn(item1)) == -1)
    {
      bprintf("It's not here.\n");
      return;
    }
  bprintf("%s : %d points\n", oname(a), ovalue(a));
}

void
examcom()
{
  int a;
  FILE *x;
  char ch;
  char text[80];

  if (!item1[0])
    {
      bprintf("Examine what?\n");
      return;
    }
  if ((a = fpbns(item1)) != -1)
    {
      if (pvis(a) < plev(mynum))
	{
	  if (ploc(mynum) == ploc(a))
	    {
	      sprintf(text, "%s%s", DESCFILE, pname(a));
	      if ((x = fopen(text, "r")) == NULL)
		{
		  bprintf("A typical, run of the mill %s.\n", pname(a));
		  return;
		}
	    }
	  else
	    {
	      bprintf("They aren't here!\n");
	      return;
	    }
	}
      else
	{
	  bprintf("You see nothing special.\n");
	  return;
	}
      fclose(x);
      closeworld();
      bprintf("\001f%s\377", text);
      return;
    }
  if ((a = ob1) == -1)
    {
      bprintf("You see nothing special.\n");
      return;
    }
  switch (a) {
  case OBJ_TREE_2:
    if (obyte(OBJ_TREE_2, 0) == 0)
      {
	osetbyte(OBJ_TREE_2, 0, 1);
	bprintf("You find a door in the tree.\n");
	setobjstate(OBJ_TREE_2, 0);
	return;
      }
    break;
  case OBJ_DESK:
    if (obyte(OBJ_DESK, 0) == 0)
      {
	osetbyte(OBJ_DESK, 0, 1);
	bprintf("You find a button hidden in a recess of the desk.\n");
	create(OBJ_BUTTON);
	return;
      }
    break;
  case OBJ_THRONE_8:
    if (obyte(OBJ_THRONE_8, 0) == 0)
      {
	osetbyte (OBJ_THRONE_8, 0, 1);
	bprintf("You find a crown hidden under the throne.\n");
	create(OBJ_CROWN_1);
	return;
      }
    break;
  case OBJ_COAT:
    if (obyte(OBJ_COAT, 0) == 0)
      {
	osetbyte (OBJ_COAT, 0, 1);
	bprintf("You find a coin in the pocket.\n");
	create(OBJ_COIN);
	setoloc(OBJ_COIN, mynum, 1);
	return;
      }
    break;
  case OBJ_TABLE_1:
    if (obyte(OBJ_TABLE_1, 0) == 0)
      {
	osetbyte (OBJ_TABLE_1, 0, 1);
	bprintf("You find some gauntlets admidst the rubble of this table.\n");
	create(OBJ_GAUNTLETS);
	return;
      }
    break;
  case OBJ_PAINTING:
    if (obyte(OBJ_PAINTING, 0) == 0)
      {
	osetbyte (OBJ_PAINTING, 0, 1);
	bprintf("There was a note behind the painting.\n");
	create(OBJ_NOTE_1);
	return;
      }
    break;
  case OBJ_CHAIR:
    if (obyte(OBJ_CHAIR, 0) == 0)
      {
	osetbyte (OBJ_CHAIR, 0, 1);
	bprintf("A ring was between the cushions of that chair!\n");
	create(OBJ_RING_4);
	return;
      }
    break;
  case OBJ_RACK:
    if (obyte(OBJ_RACK, 0) == 0)
      {
	osetbyte (OBJ_RACK, 0, 1);
	bprintf("You found a scarab in the rack.\n");
	create(OBJ_SCARAB);
	return;
      }
    break;
  case OBJ_PAPERS:
    if (obyte(OBJ_RACK, 0) == 0)
      {
	osetbyte (OBJ_PAPERS, 0, 1);
	bprintf("Among the papers, you find a treaty.\n");
	create(OBJ_TREATY);
	return;
      }
    break;
  case OBJ_DESK_1:
    if (obyte(OBJ_DESK_1, 0) == 0)
      {
	osetbyte (OBJ_DESK_1, 0, 1);
	bprintf("Inside the desk is a beautiful emerald.\n");
	create(OBJ_EMERALD);
	return;
      }
    break;
  case OBJ_ALTAR_2:
    if (obyte(OBJ_ALTAR_2, 0) == 0)
      {
	osetbyte (OBJ_ALTAR_2, 0, 1);
	bprintf("Inside the altar is a statue of a dark elven deity.\n");
	create(OBJ_STATUE_2);
	return;
      }
    break;
  case OBJ_MATTRESS:
    if (obyte(OBJ_MATTRESS, 0) == 0)
      {
	osetbyte (OBJ_MATTRESS, 0, 1);
	bprintf("Hidden under the mattress is a purse.\n");
	create(OBJ_PURSE);
	return;
      }
    break;
  case OBJ_TRASH:
    if (obyte(OBJ_TRASH, 0) == 0)
      {
	osetbyte (OBJ_TRASH, 0, 1);
	bprintf("In the trash is a silver coin.\n");
	create(OBJ_COIN_2);
	return;
      }
    break;
  case OBJ_TUBE:
    if (obyte(OBJ_TUBE, 0) == 0)
      {
	osetbyte(OBJ_TUBE, 0, 1);
	bprintf("You take a scroll from the tube.\n");
	create(OBJ_SCROLL_1);
	setoloc(OBJ_SCROLL_1, mynum, 1);
	return;
      }
    break;
  case OBJ_SCROLL_1:
    if (iscarrby(OBJ_CUP, mynum))
      {
	bprintf("Funny, I thought this was a teleport scroll, but nothing happened.\n");
	return;
      }
    bprintf("As you read the scroll you are teleported!\n");
    destroy(OBJ_SCROLL_1);
    trapch(-114);
    return;
  case OBJ_ROBE:
    if (obyte(OBJ_ROBE, 0) == 0)
      {
	bprintf("You take a key from one pocket.\n");
	osetbyte(OBJ_ROBE, 0, 1);
	create(OBJ_KEY_2);
	setoloc(OBJ_KEY_2, mynum, 1);
	return;
      }
    break;
  case OBJ_WAND:
    if (obyte(OBJ_WAND, 1) != 0)
      {
	bprintf("It seems to be charged.");
	return;
      }
    break;
  case OBJ_BALL:
    setobjstate(OBJ_BALL, randperc() % 3 + 1);
    switch (state(OBJ_BALL)) {
    case 1:
      bprintf("It glows red.");
      break;
    case 2:
      bprintf("It glows blue.");
      break;
    case 3:
      bprintf("It glows green.");
      break;
    }
    bprintf("\n");
    return;
  case OBJ_SCROLL:
    if (state(OBJ_BALL) != 0)
      {
	if (iscarrby(3 + state(OBJ_BALL), mynum)
	    && otstbit(3 + state(OBJ_BALL), ofl(Lit)))
	  {
	    bprintf("Everything shimmers and then solidifies into a different view!\n");
	    destroy(OBJ_SCROLL);
	    teletrap(-1074);
	    return;
	  }
      }
    break;
  case OBJ_BED:
    if (!obyte(OBJ_BED, 0))
      {
	create(OBJ_LOAF);
	create(OBJ_PIE);
	osetbyte(OBJ_BED, 0, 1);
	if (ishere(OBJ_LOAF) && ishere(OBJ_PIE))
	  bprintf("Aha!  Under the bed you find a loaf and a rabbit pie.\n");
	else
	  bprintf("You find nothing.\n");
	return;
      }
    break;
  case OBJ_GARBAGE:
    if (!state(OBJ_GARBAGE))
      {
	if (alive(MOB_MAGGOT) != -1)
	  {
	    bprintf("In the garbage you find a gold plate... with a maggot on it!\n");
	    sprintf(text, "A maggot leaps out of the garbage onto %s!\n",
		    pname(mynum));
	    sillycom(text);
	    setploc(MOB_MAGGOT, ploc(mynum));
	    /* Make sure maggot attacks the right person. */
	    woundmn(a, 0);
	  }
	else
	  bprintf("In the garbage you find a gold plate with some slime on it.\n");
	setoloc(OBJ_PLATE_1, ploc(mynum), 0);
	create(OBJ_PLATE_1);
	setobjstate(OBJ_GARBAGE, 1);
	return;
      }
    break;
  case OBJ_BEDDING:
    if (!obyte(OBJ_AMULET, 0))
      {
	create(OBJ_AMULET);
	bprintf("You pull an amulet from the bedding.\n");
	osetbyte(OBJ_AMULET, 0, 1);
	return;
      }
    break;
  case OBJ_BOARSKIN:
    if (!obyte(OBJ_WHISTLE, 0))
      {
	create(OBJ_WHISTLE);
	bprintf("Under the boarskin you find a silver whistle.\n");
	osetbyte(OBJ_WHISTLE, 0, 1);
	return;
      }
    break;
  }
  if ((x = fopen(OBJECTS, "r")) == NULL || objects[a].o_examine == 0)
    {
      bprintf("You see nothing special.\n");
      return;
    }
  fseek(x, objects[a].o_examine, 0);
  while ((ch = fgetc(x)) != '^')
    bprintf("%c", ch);
  fclose(x);
}

void incom()
{
  char st[80];
  int x, y;

  if (plev(mynum) < LVL_WIZARD)
    {
      erreval();
      return;
    }
  if ((x = getroomnum()) == 0 || !exists(x))
    {
      bprintf("Unknown Player Or Room\n");
      return;
    }
  getreinput(st);
  y = ploc(mynum);
  setploc(mynum, x);
  gamecom(st);
  openworld();
  setploc(mynum, y);
}

void
jumpcom()
{
  int a, b, i;
  char ms[128];

  if (psitting(mynum))
    {
      bprintf("You have to stand up first.\n");
      error();
      return;
    }
  if (ploc(mynum) == -2320)
    {
      bprintf("You fall...\n\n\001S2\377");
      bprintf("             and fall...\n\n\001S2\377");
      bprintf("                          and fall...\n\n\001S2\377");
      bprintf("                                       and fall...\n\n\001S2\377");
      bprintf("                                                    and die.\n");
      loseme();
      crapup("You impaled yourself on a construction sign");
    }
  for (a = 0, b = 0; jumtb[a]; a += 2)
    if (jumtb[a] == ploc(mynum))
      {
	b = jumtb[a + 1];
	break;
      }
  for (i = OBJ_PIT_1; i <= OBJ_PIT_6; i++)
    if (oloc(i) == ploc(mynum))
      break;
  if ((i <= OBJ_PIT_4 && !state(i)))
    b = -2429;
  else if (i <= OBJ_PIT_6 || oloc(OBJ_HOLE_6) == ploc(mynum))
    b = RM_PIT1;
  if (b == 0)
    {
      bprintf("Wheeeeee....\n");
      return;
    }
  if (iscarrby(OBJ_UMBRELLA, mynum) && state(OBJ_UMBRELLA) != 0)
    {
      sprintf(ms, "\001s%s\377%s jumps off the ledge.\n\377",
	      pname(mynum), pname(mynum));
      bprintf("You grab hold of the umbrella and fly down like Mary Poppins.\n");
    }
  else if (plev(mynum) < LVL_WIZARD)
    {
      sprintf(ms, "\001s%s\377%s makes a perfect swan dive off the ledge.\n\377",
	      pname(mynum), pname(mynum));
      if (b != RM_PIT1)
	{
	  sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, ploc(mynum), ms);
	  setploc(mynum, b);
	  bprintf("Wheeeeeeeeeeeeeeeee	 <<<<SPLAT>>>>\n");
	  bprintf("You seem to be splattered all over the place.\n");
	  loseme();
	  crapup("I suppose you could be scraped up with a spatula.");
	}
    }
  else
    sprintf(ms, "\001s%s\377%s dives off the ledge and floats down.\n\377",
	    pname(mynum), pname(mynum));
  sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, ploc(mynum), ms);
  setploc(mynum, b);
  if (iscarrby(OBJ_UMBRELLA, mynum))
    sprintf(ms, "\001s%s\377%s flys down, clutching an umbrella.\n\377",
	    pname(mynum), pname(mynum));
  else
    sprintf(ms, "\001s%s\377%s has just dropped in.\n\377",
	    pname(mynum), pname(mynum));
  sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, ploc(mynum), ms);
  trapch(b);
}

void
wherecom()
{
  int cha, rnd;

  if (pstr(mynum) < 10)
    {
      bprintf("You're too weak to cast any spells.\n");
      return;
    }
  if (plev(mynum) < LVL_WIZARD)
    setpstr(mynum, pstr(mynum) - 2);
  rnd = randperc();
  cha = 6 * plev(mynum);
  if (iscarrby(OBJ_POWERSTONE, mynum))
    cha += 12;
  if (iscarrby(OBJ_POWERSTONE_1, mynum))
    cha += 12;
  if (iscarrby(OBJ_POWERSTONE_2, mynum))
    cha += 12;
  if (plev(mynum) >= LVL_NECROMANCER)
    cha = 100;
  closeworld();
  if (rnd > cha)
    {
      bprintf("Your spell fails.\n");
      return;
    }
  if (!item1[0])
    {
      bprintf("What's that?\n");
      return;
    }
  for (cha = 0, rnd = 0; cha < numobs; cha++)
    {
      if (EQ(oname(cha), item1)
	  || (plev(mynum) >= LVL_WIZARD && EQ(oaltname(cha), item1)))
	if (otstbit(cha, ofl(Unlocatable)) && (plev(mynum) < LVL_GOD))
	  {
	    if (!rnd)
	      bprintf("Strange...  You can't seem to find it.\n");
	    rnd = 1;
	  }
	else
	  {
	    rnd = 1;
	    if (plev(mynum) >= LVL_ARCHWIZARD)
	      bprintf("[%3d]", cha);
	    bprintf("%13s:", oname(cha));
	    if (plev(mynum) < LVL_WIZARD && ospare(cha) == -1)
	      bprintf(" Nowhere\n");
	    else
	      showlocation(cha);
	  }
    }
  if ((cha = pl1) != -1)
    {
      rnd++;
      bprintf("%-15s", pname(cha));
      showploc(cha);
    }
  if (!rnd)
    bprintf("I don't know what it is.\n");
}

void
showploc(int a)
{
  if (plev(mynum) < LVL_WIZARD && ploc(a) > -5)
    {
      bprintf("Somewhere...\n");
      return;
    }
  if (!exists(ploc(a)))
    {
      if (plev(mynum) < LVL_ARCHWIZARD)
	bprintf("Out in the void\n");
      else
	bprintf("NOT IN UNIVERSE[%d]\n", ploc(a));
      return;
    }
  bprintf("%-35s", sdesc(ploc(a)));
  if (plev(mynum) >= LVL_WIZARD)
    {
      bprintf(" | ");
      showname(ploc(a));
    }
  else
    bprintf("\n");
}

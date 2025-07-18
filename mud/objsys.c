/*
**  Routines to keep track of objects
*/

#include <strings.h>
#include "kernel.h"
#include "objects.h"
#include "mobiles.h"
#include "locations.h"
#include "pflags.h"
#include "oflags.h"
#include "lflags.h"
#include "aflags.h"
#include "objsys.h"
#include "macros.h"
#include "support.h"
#include "blib.h"
#include "bprintf.h"
#include "new1.h"
#include "new2.h"
#include "parse.h"
#include "tk.h"
#include "weather.h"
#include "exec.h"
#include "extra.h"
#include "blood.h"

static LEVEL u;

static char *classnam[] =
{
  "clothing", "weapons", "containers", "food", "keys", "all", NULL
};

static short class_data[] = {
  ofl(Wearable), -2,
  ofl(Weapon), -1,
  ofl(Container), -1,
  ofl(Food), -1,
  ofl(Key), -1,
  0, -1
};

void
inventory()
{
  bprintf("You are carrying:\n");
  lobjsat(mynum);
}

void
lobjsat(int loc)
{
  aobjsat(loc, 1, 0);
}

void
mlobjsat(int x, int m)
{
  aobjsat(x, 4, m);
}

void
aobjsat (int loc, int mode, int marg)
{
  int ostack[16];
  int ostackp;
  int a, c, e, f, g;

  ostackp = 0;
  e = 0;
  for (f = 0; f < marg; f++)
    bprintf(" ");
  for (c = 0; c < numobs; c++)
    {
      g = 0;
      if ((iscarrby(c, loc) && (mode == 1 || mode == 4))
	  || (iscontin(c, loc) && mode == 3))
	{
	  e = g = 1;
	  f += strlen(oname(c)) + 1;
	  if (otstbit(c, ofl(Destroyed)))
	    f += 2;
	  if (iswornby(c, loc))
	    f += 7;
	  else if (pwpn(loc) == c)
	    f += 10;
	  else if (otstbit(c, ofl(Lit)))
	    f += 6;
	  if (f > 78)
	    {
	      bprintf("\n");
	      for (f = 0; f < marg; f++)
		bprintf(" ");
	    }
	  if (otstbit(c, ofl(Destroyed)))
	    bprintf("(");
	  bprintf("%s", oname(c));
	  if (iswornby(c, loc))
	    bprintf(" <worn>");
	  else if (pwpn(loc) == c)
	    bprintf(" <wielded>");
	  else if (otstbit(c, ofl(Lit)))
	    bprintf(" <lit>");
	  if (otstbit(c, ofl(Destroyed)))
	    bprintf(")");
	  bprintf(" ");
	  if (otstbit(c, ofl(Container)) && g
	      && (!otstbit(c, ofl(Openable)) || state(c) == 0))
	    {
	      ostack[ostackp++] = c;
	      f++;
	    }
	}
    }
  if (!e)
    bprintf("Nothing");
  bprintf("\n");
  if (mode != 4) {
      for (f = 0; f < ostackp; f++) {
	  for (a = 0; a < marg; a++) {
	      bprintf(" ");
	  }
	  bprintf("    The %s contains:\n", oname(ostack[f]));
	  pbfr();
	  aobjsat(ostack[f], 3, marg + 8);
      }
  }
}

int
unencumbered()
{
  int ob;

  for (ob = 0; ob < numobs; ob++)
    if (iscarrby(ob, mynum))
      return 0;
  return 1;
}

int
iscontin(int o1, int o2)
{
  if (ocarrf(o1) != 3 || oloc(o1) != o2)
    return 0;
  if (plev(mynum) < LVL_WIZARD && otstbit(o1, ofl(Destroyed)))
    return 0;
  return 1;
}

void
brkwordw(char *x)
{
  strcpy(x, brkword() == -1 ? "<nothing>" : wordbuf);
}

int
fobnsys(char *nam, int ctrl, int ct_inf)
{
  int a;

  for (a = 0; a < numobs; a++) {
    if (EQ(nam, oname(a)) || EQ(nam, oaltname(a))) {
      strcpy(wd_it, nam);
      switch (ctrl) {
      case 0:
	return a;
      case 1:			/* Patch for shields */
	if (a == OBJ_SHIELD_1 && iscarrby(OBJ_SHIELD_2, mynum))
	  return OBJ_SHIELD_2;
	if (a == OBJ_SHIELD_1 && iscarrby(OBJ_SHIELD_3, mynum))
	  return OBJ_SHIELD_3;
	if (isavl(a))
	  return a;
	break;
      case 2:
	if (iscarrby(a, mynum))
	  return a;
	break;
      case 3:
	if (iscarrby(a, ct_inf))
	  return a;
	break;
      case 4:
	if (ishere(a))
	  return a;
	break;
      case 5:
	if (iscontin(a, ct_inf))
	  return a;
	break;
      default:
	return a;
      }
    }
  }
  return -1;
}

int
fobn(char *word)
{
  int x;

  if ((x = atoi(word)) > 0 && x <= numobs)
    return x;
  if ((x = fobna(word)) != -1)
    return x;
  return fobnsys(word, 0, 0);
}

int
fobna(char *word)
{
  return fobnsys(word, 1, 0);
}

int
fobnin(char *word, int ct)
{
  return fobnsys(word, 5, ct);
}

int
fobnc(char *word)
{
  return fobnsys(word, 2, 0);
}

int
fobncb(char *word, int by)
{
  return fobnsys(word, 3, by);
}

int
fobnh(char *word)
{
  return fobnsys(word, 4, 0);
}

int
getobj()
{
  int a, i, l;
  int des_inf = -1;
  char bf[81];

  if (brkword() == -1) {
    bprintf("Get what?\n");
    return -1;
  }
  if (isdark()) {
    bprintf("It's dark!\n");
    return -1;
  }
  if (findclass(wordbuf) != -1) {
    getall();
    return 0;
  }
  a = fobnh(wordbuf);
  /* Hold */
  i = stp;
  strcpy(bf, wordbuf);
  if (brkword() != -1 && (EQ(wordbuf, "from") || EQ(wordbuf, "out"))) {
    if (brkword() == -1) {
      bprintf("From what?\n");
      return -1;
    }
    if ((des_inf = fobna(wordbuf)) == -1) {
      bprintf("You can't take things from that!  It's not here!\n");
      return -1;
    }
    a = fobnin(bf, des_inf);
  }
  stp = i;
  if (a == -1) {
    bprintf("It's not here.\n");
    return -1;
  }
  if (a == OBJ_SHIELD_1 && ishere(OBJ_SHIELD_2))
    a = OBJ_SHIELD_2;
  if (a == OBJ_SHIELD_1 && ishere(OBJ_SHIELD_3))
    a = OBJ_SHIELD_3;
  if (a == OBJ_SHIELD_1 && des_inf == -1) {
    if (otstbit(OBJ_SHIELD_2, ofl(Destroyed)))
      a = OBJ_SHIELD_2;
    else if (otstbit(OBJ_SHIELD_3, ofl(Destroyed)))
      a = OBJ_SHIELD_3;
    if (a == OBJ_SHIELD_2 || a == OBJ_SHIELD_3)
      create(a);
    else {
      bprintf("The shields are too firmly secured to the walls.\n");
      return 0;
    }
  }
  if (oflannel(a) == 1) {
    bprintf("You can't take that!\n");
    return -1;
  }
  if (ptstflg(mynum, pfl(NoArms))) {
    bprintf("You can't take anything, you have no arms.\n");
    return -1;
  }
  if (des_inf == -1 || !iscarrby(des_inf, mynum))
    if (dragget())
      return -1;
  if (!cancarry(mynum)) {
    bprintf("You can't carry any more.\n");
    return -1;
  }
  if ((a == OBJ_GRASSHOPPER) && (state(OBJ_GRASSHOPPER) != 0)) {
	bprintf("\nYou hear an explosion and see a blinding flash of light.\n");
    broad("\001dYou hear a loud explosion coming from town.\n\377");
    teletrap(-2700);
	if (alive(MOB_WORKMAN)) {
	  setploc(MOB_WORKMAN, RM_VILLAGE12);
	}
  }
  if (a == OBJ_VASE) {
    destroy(OBJ_VASE);
    bprintf("The vase slips out of you hands and crashes to the floor!\n");
    bprintf("The reverberations caused by the crash cause the ceiling to");
    bprintf(" cave in!\n");
    broad("\001dYou hear ambulance sirens in the distance.\n\377");
    teletrap(-2700);
    return 0;
  }
  if (a == OBJ_RUNESWORD && state(a) == 1)
    if (ptothlp(mynum) == -1) {
      bprintf("It's too well embedded to shift alone.\n");
      return 0;
    }
  if (a == OBJ_ARK)
    if (ptothlp(mynum) == -1) {
      bprintf("It's too heavy to lift alone.\n");
      return 0;
    }
  if ((l = alive(MOB_SERAPH)) != -1 && ploc(l) == ploc(mynum)
      && a == OBJ_CUP) {
    bprintf("\001pThe Seraph\377 says 'Well done, my %s.  Truly you are a %s of virtue.'\n", psex(mynum) ? "daughter" : "son", psex(mynum) ? "woman" : "man");
  }
  setoloc(a, mynum, 1);
  sprintf(bf, "\001p%s\377 takes the %s.\n", pname(mynum), oname(a));
  if (des_inf != -1)
    sprintf(bf, "\001p%s\377 takes the %s from the %s.\n",
	    pname(mynum), oname(a), oname(des_inf));
  sendsys(pname(mynum), pname(mynum), -10000, ploc(mynum), bf);
  if (otstbit(a, ofl(GetFlips)))
    setobjstate(a, 0);
  if ((ploc(mynum) == -1081) && (state(OBJ_DOOR) == 0)) {
    setobjstate(OBJ_DOOR, 1);
    bprintf("The door clicks shut....\n");
  }
  if ((l = alive(MOB_SERAPH)) != -1 && ploc(l) == ploc(mynum)
      && a == OBJ_CUP) {
    bprintf("The Seraph gestures and you are transported to ...\n");
    sprintf(bf, "The Seraph vanishes, taking %s with him!\n", pname(mynum));
    sendsys("The Seraph", "The Seraph", -10000, ploc(mynum), bf);
    sprintf(bf, "%s appears, accompanied by an angel!\n", pname(mynum));
    sendsys(pname(mynum), pname(mynum), -10000, -5, bf);
    setploc(MOB_SERAPH, -5);
    trapch(-5);
    return 0;
  }
  bprintf("Ok\n");
  return 0;
}

void
getall()
{
  int x, cl;

  cl = findclass(wordbuf);
  if (brkword() != -1) {
    getallfr();
    return;
  }
  for (x = 0; x < numobs; x++) {
    if (ishere(x) && !oflannel(x) && classmatch(x, cl)) {
      sprintf(strbuf, "%s", oname(x));
      stp = 0;
      bprintf("%s: ", oname(x));
      if (getobj() == -1)
	break;
    }
  }
}

void
getallfr()
{
  int a, x, cl;

  cl = findclass(wordbuf);
  if (brkword() == -1) {
    bprintf("From what?\n");
    return;
  }
  if ((a = fobna(wordbuf)) == -1) {
    bprintf("That isn't here.\n");
    return;
  }
  /* Do items */
  for (x = 0; x < numobs; x++) {
    if (iscontin(x, a) && !oflannel(x) && classmatch(x, cl)) {
      sprintf(strbuf, "%s from %s", oname(x), oname(a));
      stp = 0;
      bprintf("%s: ", oname(x));
      if (getobj() == -1)
	break;
    }
  }
}

void
dropall()
{
  int x, cl;

  for (cl = findclass(wordbuf), x = 0; x < numobs; x++) {
    if (iscarrby(x, mynum) && classmatch(x, cl)) {
      sprintf(strbuf, "%s", oname(x));
      stp = 0;
      bprintf("%s: ", oname(x));
      if (dropitem() == -1)
	return;
    }
  }
}

int
ishere(int item)
{
  if (plev(mynum) < LVL_WIZARD && otstbit(item, ofl(Destroyed)))
    return 0;
  if (ocarrf(item) == 1 || oloc(item) != ploc(mynum))
    return 0;
  return 1;
}

int
iscarrby(int item, int user)
{
  if (plev(mynum) < LVL_WIZARD && otstbit(item, ofl(Destroyed)))
    return 0;
  if (ocarrf(item) != 1 && ocarrf(item) != 2)
    return 0;
  if (oloc(item) != user)
    return 0;
  return 1;
}

int
dropitem()
{
  int a, i, l;
  char bf[80];

  if (brkword() == -1) {
    bprintf("Drop what?\n");
    return -1;
  }
  if (findclass(wordbuf) != -1) {
    dropall();
    return 0;
  }
  if ((a = fobnc(wordbuf)) == -1) {
    bprintf("You don't have it.\n");
    return -1;
  }
  if (plev(mynum) < LVL_WIZARD && a == OBJ_RUNESWORD) {
    bprintf("You can't let go of it!\n");
    return 0;
  }
  if (a == OBJ_STAFF_2 && ploc(mynum) != -4 && ploc(mynum) != -5 &&
      !ishere(OBJ_HOLE_5))
    {
      destroy(OBJ_STAFF_2);
      l = ploc(mynum);
      setploc(MOB_SNAKE, l);
      bprintf("As you drop the staff, it comes alive and starts wiggling!\n");
      return;
    }
  if (a == OBJ_CUP && ploc(MOB_SERAPH) == -5)
    setplev(MOB_SERAPH, -2);
  l = ploc(mynum);

  /* MINE LADDER */
  if (l == RM_QUARRY4 || l == RM_QUARRY5) {
    bprintf("The %s falls down the ladder.\n", oname(a));
    l = RM_QUARRY9;
  }
  if ((l >= RM_QUARRY16 && l <= RM_QUARRY14) ||
      (l >= RM_QUARRY11 && l <= RM_QUARRY9)) {
    bprintf("The %s falls down the ladder.\n", oname(a));
    l = RM_QUARRY8;
  }

  /* ALL AT SEA */
  if (ltstflg(l, lfl(OnWater)) && a != OBJ_BOAT && a != OBJ_RAFT) {
    bprintf("The %s sinks into the sea.\n", oname(a));
    l = RM_SEA7;
  }

  /* OAKTREE */
  if ((l >= RM_OAKTREE6 && l <= RM_OAKTREE2) || l == RM_OAKTREE9 ||
      (l >= RM_OAKTREE18 && l <= RM_OAKTREE13))
    {
      bprintf("The %s falls through the leaves to the ground far below.\n",
	      oname(a));
      l = RM_OAKTREE1;
      sendsys("", "", -10000, l, "Something falls to the ground.\n");
    }
  for (i = OBJ_PIT_1; i <= OBJ_PIT_6; i++)
    if (oloc(i) == l)
      break;
  if ((i <= OBJ_PIT_6 && !state(i)) || oloc(OBJ_HOLE_5) == l)
    {
      bprintf("It disappears into the bottomless pit.....\n");
      sprintf(bf, "\001p%s\377 drops the %s into the pit.\n",
	      pname(mynum), oname(a));
      sendsys(pname(mynum), pname(mynum), -10000, ploc(mynum), bf);
      dropinpit(a);
      return 0;
    }
  else if (i <= OBJ_PIT_6)
    {
      bprintf("It disappears into the bottomless pit.....and hits bottom.\n");
      sprintf(bf, "\001p%s\377 drops the %s into the pit.\n",
	      pname(mynum), oname(a));
      sendsys(pname(mynum), pname(mynum), -10000, ploc(mynum), bf);
      l = RM_CATACOMB45;
      setoloc(a, l, 0);		/* to spherical room in CATACOMB section */
      sendsys("", "", -10000, l, "Something falls to the ground.\n");
      return 0;
    }
  setoloc(a, l, 0);
  sprintf(bf, "\001p%s\377 drops the %s.\n", pname(mynum), oname(a));
  sendsys(pname(mynum), pname(mynum), -10000, ploc(mynum), bf);
  if (l = ploc(mynum))
    bprintf("Ok\n");
  return 0;
}

void
dropinpit(int o)
{
  int i;

  setpscore(mynum, pscore(mynum) + ovalue(o));
  calibme();
  osetbit(o, ofl(Destroyed));
  setoloc(o, RM_PIT1, 0);
  if (otstbit(o, ofl(Container)))
    for (i = 0; i < numobs; i++)
      if (iscontin(i, o))
	dropinpit(i);
}

void
lisobs()
{
  lojal2(1);
  showwthr();
  lojal2(0);
}

void
lojal2(int n)
{
  int a;

  for (a = 0; a < numobs; a++) {
    if (ishere(a) && oflannel(a) == n) {
      if (state(a) > 3)
	continue;
      if (!EMPTY(olongt(a, state(a)))) {
	if (otstbit(a, ofl(Destroyed)))
	  bprintf("--");
	oplong(a);
	strcpy(wd_it, oname(a));
      }
      else if (my_lev > 9999)
	bprintf("<marker>%s\n", oname(a));
    }
  }
}

void
dumpitems()
{
  dumpstuff(mynum, ploc(mynum));
}

void
dumpstuff(int n, int loc)
{
  int b;

  for (b = 0; b < numobs; b++)
    if (iscarrby(b, n)) {
      if (loc == RM_PIT1)
	dropinpit(b);
      else
	setoloc(b, loc, 0);
    }
}

void
whocom()
{
  whop2(0, 0, MAX_USERS);
}

void
mwhocom()
{
  if (plev(mynum) >= LVL_WIZARD) 
    whop2(0, MAX_USERS, MAX_CHARS);
  else
    erreval();
}

void
whop2(int mode, int st, int end)
{
  int a, bas;

  bas = end;
  a = st;
  if (a == 0 && !mode)
    bprintf("Players\n-------\n");
  if (a == MAX_USERS && !mode)
    bprintf("Mobiles\n-------\n");
  for (; a < bas; a++) {
    if (EMPTY(pname(a)))
      continue;
    if (seeplayer(a)) {
      if (mode) {
	bprintf("%-14s", pname(a));
	bprintf(ptstflg(a, pfl(Possessed)) ? "*" : " ");
      }
      else
	dispuser(a);
    }
    if (mode && seeplayer(a)) {
      if (plev(mynum) >= LVL_WIZARD)
	showploc(a);
      else {
	if (pvis(a))
	  bprintf(")");
	bprintf("\n");
      }
    }
  }
  bprintf("\n");
}

void
dispuser(int ubase)
{
  if (pstr(ubase) < 0 ||
      (ubase != mynum && pvis(ubase) > 0 && pvis(ubase) > plev(mynum)))
    return;
  if (pvis(ubase))
    bprintf("(");
  displevel(plev(ubase), psex(ubase), pname(ubase));
  if (pvis(ubase))
    bprintf(")");
  if (plev(mynum) >= LVL_WIZARD && ppos(ubase) == -2)
    bprintf(" [out of %s mind]", psex(ubase) ? "her" : "his");
  bprintf("\n");
}

void
displevel(int l, int s, char *name)
{
  FILE *fp;
  char line[150];
  int flag;

  if ((fp = fopen(LEVEL_FILE, "r")) == NULL) {
    bprintf("%s The Unknown", name);
    return;
  }
  for (flag = 0; fgets(line, sizeof line, fp); ) {
    unpack(line);
    if (l == u.level) {
      flag = 1;
      break;
    }
  }
  fclose(fp);
  if (!flag) {
    if (l >= 0)
      bprintf("%s The Cardboard Box", name);
    return;
  }
  if (!*u.m_name || s)
    bprintf(u.f_name, name);
  else
    bprintf(u.m_name, name);
}

#define LEVEL_SCRATCH "titles.tmp"

void
savelevel (int l, char *title)
{
  FILE *fp;			/* Old titles file */
  FILE *np;			/* New titles file */
  char line[BUFSIZ], l2[BUFSIZ], *ptr;

  /* Create scratch file. */
  if ((np = openlock(LEVEL_SCRATCH, "w+")) == NULL)
    {
      bprintf("Error: Cannot open scratch pad.\n");
      return;
    }

  /* Open titles file. */
  if ((fp = openlock(LEVEL_FILE, "r")) == NULL)
    {
      bprintf("Error: Cannot open %s.\n", LEVEL_FILE);
      return;
    }

  /* Scan for old title, copying to scratch file as we go. */
  while (ptr = fgets(line, sizeof line, fp))
    {
      strcpy(l2, line);
      unpack(line);
      if (l <= u.level)
	break;		/* Don't copy current title. */
      fputs(l2, np);
    }

  /* Save new title */
  fprintf(np, "%0d:%s\n", l, title);
  if (l < u.level)
    fputs(l2, np);
  u.level = l;

  /* Copy the rest of file if necessary */
  while (fgets(line, sizeof line, fp))
    fputs(line, np);
  fclose(fp);
  unlink(fp);
  rename(LEVEL_SCRATCH, LEVEL_FILE);
  closelock(np);
}

void unpack (char *line)
{
  char c, *ptr2, *ptr = line;

  while (*ptr++ != ':')
    ;
  *(ptr - 1) = '\0';
  u.level = atoi(line);
  ptr2 = u.f_name;
  while ((c = *ptr++) != ':')
    {
      if (c == '\n')
	{
	  *u.m_name = '\0';
	  *ptr2 = '\0';
	  return;
	}
      *ptr2++ = c;
    }
  *ptr2 = '\0';
  ptr2 = u.m_name;
  while ((*ptr2++ = *ptr++) != '\n')
    ;
  *--ptr2 = '\0';
}

int
fpbn (char *name)
{
  int s;

  if ((s = fpbns(name)) == -1)
    return s;
  return seeplayer(s) ? s : -1;
}

int
fpbns (char *name)
{
  char n1[90], n2[90];
  register int a;

  for (a = 0; a < MAX_CHARS; a++)
    {
      lowercase(strcpy(n1, name));
      lowercase(strcpy(n2, pname(a)));
      lowercase(n2);
      if (strncmp(n2, "the ", 4) == 0)
	sprintf(n2, n2 + 4);
      if (!EMPTY(n2) && !EMPTY(n1) && EQ(n1, n2))
	return a;
    }
  return -1;
}

int
alive(int i)
{
  if (pstr(i) < 1 || EMPTY(pname(i)))
    return -1;
  else
    return i;
}

/*
**  Replaces special codes in string with appropriate pronouns
*/
void
pnstring(char *s, int pl)
{
  char *i;

  while (i = index(s, '~'))
    {
      *i++ = 'h';
      if (psex(pl))
	{
	  insertch(s, 'e', i++ - s);
	  insertch(s, 'r', i - s);
	}
      else
	{
	  insertch(s, 'i', i++ - s);
	  insertch(s, 'm', i - s);
	}
    }
  while (i = index(s, '^'))
    {
      *i++ = 'h';
      if (psex(pl))
	{
	  insertch(s, 'e', i++ - s);
	  insertch(s, 'r', i - s);
	}
      else
	{
	  insertch(s, 'i', i++ - s);
	  insertch(s, 's', i - s);
	}
    }
  while (i = index(s, '@'))
    {
      *i++ = '\001';
      insertch(s, 'N', i++ - s);
      insertch(s, '%', i++ - s);
      insertch(s, 's', i++ - s);
      insertch(s, '\377', i - s);
    }
  while (i = index(s, '&'))
    {
      *i++ = '\001';
      insertch(s, 'n', i++ - s);
      insertch(s, '%', i++ - s);
      insertch(s, 's', i++ - s);
      insertch(s, '\377', i - s);
    }
}

/*
**  Brian Preble (rassilon@eddie.mit.edu)
**  Handles external commands such as Kiss, Hug, Tickle, etc.
*/
int
fextern (char *verb)
{
  int i, a;
  int found = -1;
  char x[81], line[81];

  for (i = 0; i < numactions; i++)
    if (EQ(verb, action[i]))
      {
	found = i;
	break;
      }
  if (found == -1)		/* command not found in extern list */
    return -1;
  if (in_fight)
    {
      bprintf("Not in a fight!\n");
      return 0;
    }

  if (tables(1) == 2)
    return 0;

  /* Is this a message action?  If so, parse accordingly.  */
  if (!EMPTY(item1) && atstflg(i, ACT_MESSAGE))
    {
      if ((a = fpbn(item1)) != -1 && atstflg(i, ACT_TARGET))
	if (!EMPTY(txt2))
	  {
	    strcpy(x, action_msg_to[i]);
	    pnstring(x, mynum);
	    sprintf(line, "%s '%s'", x, txt2);
	    sillytp(a, line);
	    bprintf("Ok\n");
	    return 0;
	  }
	else
	  {}
      else if (atstflg(i, ACT_ALL))
	{
	  strcpy(x, action_msg_all[i]);
	  pnstring(x, mynum);
	  sprintf(line, "\001s%%s\377%%s %s '%s'\n\377", x, txt1);
	  sillycom(line);
	  bprintf("Ok\n");
	  return 0;
	}
    }

  if (EMPTY(item1) || !atstflg(i, ACT_TARGET))
    {
      if (atstflg(i, ACT_ALL))
	{
	  strcpy(x, action_all[i]);
	  pnstring(x, mynum);
	  sprintf(line, "\001s%%s\377%%s %s\n\377", x);
	  sillycom(line);
	  bprintf("%s\n", action_all_rsp[i]);
	  return 0;
	}
      bprintf("To whom do you wish to do this thing?\n");
      return 0;
    }
  if (vichere(&a) == -1)
    return 0;
  if (a == mynum)
    {
      bprintf("Good trick, that.\n");
      return 0;
    }
  if (atstflg(i, ACT_HOSTILE))
    if (testpeace())
      {
	bprintf("Nah, that's violent.\n");
	return 0;
      }
  if (ptstflg(a, pfl(Aloof)) && plev(mynum) < LVL_WIZARD)
    {
      bprintf("%s thinks %s's too good for mere mortals.\n",
	      pname(a), (psex(a) ? "she" : "he"));
      return 0;
    }
  strcpy(x, action_to[i]);
  pnstring(x, mynum);
  sprintf(line, x, pname(a));
  sprintf(x, "\001s%%s\377%%s %s\n\377", line);
  sillycom(x);
  strcpy(x, action_to_rsp[i]);
  pnstring(x, a);
  bprintf("%s\n", x);
  if (atstflg(i, ACT_HOSTILE) && a >= MAX_USERS) /* hostile action? */
    woundmn(a, 0);
  return 0;
}

/*
**  Brian Preble (rassilon@eddie.mit.edu)
**  Displays list of external commands such as Kiss, Hug, Tickle, etc.
*/
void
lisextern()
{
  int i;

  bprintf("The following actions are defined:\n");
  for (i = 0; i < numactions; i++)
    {
      if (i % 8 == 0)
	bprintf("\n");
      bprintf("%-10s", uppercase(action[i]));
    }
  if (i % 8 == 0)
    bprintf("\n");
  bprintf("%-10s", "FLOWERS");
  i++;
  if (i % 8 == 0)
    bprintf("\n");
  bprintf("%-10s", "PET");
  i++;
  if (i % 8 == 0)
    bprintf("\n");
  bprintf("%-10s", "PRAY");
  i++;
  if (i % 8 == 0)
    bprintf("\n");
  bprintf("%-10s", "TICKLE");
  i++;
  if (i % 8 == 0)
    bprintf("\n");
  bprintf("WAVE\n\n");
}

void lispeople()
{
  int a;

  for (a = 0; a < MAX_CHARS; a++)
    {
      if (a == mynum)
	continue;
      if (!EMPTY(pname(a)) && ploc(a) == ploc(mynum) && seeplayer(a))
	{
	  if (a >= MAX_USERS)
	    {
	      if (plev(mynum) >= LVL_WIZARD && pvis(a))
		bprintf("(");
	      bprintf("%s", pftxt[a - MAX_USERS]);
	      if (plev(mynum) >= LVL_WIZARD && pvis(a))
		bprintf(")");
	      if (gotanything(a))
		bprintf("\n%s is", pname(a));
	      else
		bprintf("\n");
	    }
	  else
	    {
	      if (plev(mynum) >= LVL_WIZARD && pvis(a))
		bprintf("(");
	      displevel(plev(a), psex(a), pname(a));
	      if (psex(a))
		strcpy(wd_her, pname(a));
	      else
		strcpy(wd_him, pname(a));
	      strcpy(wd_them, pname(a));
	      bprintf(" is %shere", psitting(a) ? "sitting " : "");
	      bprintf("%s", ptstflg(a, pfl(Glowing))
		      ? " (providing light)" : "");
	      if (!gotanything(a))
		{
		  bprintf(".");
		  if (plev(mynum) >= LVL_WIZARD && pvis(a))
		    bprintf(")");
		  bprintf("\n");
		}
	    }
	  if (gotanything(a))
	    {
	      bprintf(" carrying:");
	      if (plev(mynum) >= LVL_WIZARD && pvis(a))
		bprintf(")");
	      bprintf("\n");
	      mlobjsat(a, 4);
	    }
	}
    }
}

void usercom()
{
  whop2(1, 0, MAX_USERS);
}

void oplong(int x)
{
  if (!EMPTY(olongt(x, state(x))))
    bprintf("%s\n", olongt(x, state(x)));
}

int gotanything(int x)
{
  int ct;
    
  for (ct = 0; ct < numobs; ct++)
    if (iscarrby(ct, x))
      return 1;
  return 0;
}

int findclass(char *n)
{
  int a;
    
  for (a = 0; classnam[a]; a++)
    if (EQ(classnam[a], n))
      return a;
  return -1;
}

int classmatch(int ob, int cl)
{
  int neg;

  if (cl == -1)
    return 1;
  neg = 0;
  if (class_data[cl * 2 + 1] >= 0 && class_data[cl * 2 + 1] != state(ob))
    return neg;
  if (otstmask(ob, class_data[cl * 2]) == 0)
    return neg;
  return 1 - neg;
}

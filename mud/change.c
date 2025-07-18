/*
**  CHANGE XXX routines.
*/

#include <strings.h>
#include "maint.h"
#include "kernel.h"
#include "macros.h"
#include "parse.h"
#include "parser.h"
#include "support.h"
#include "change.h"
#include "pflags.h"
#include "magic.h"
#include "blib.h"
#include "sendsys.h"

void changecom()
{
  if (brkword() == -1)
    {
      bprintf("Change what?\n");
      return;
    }

  /* CHANGE VIS */
  if (EQ(wordbuf, "vis"))
    chvis();

  /* CHANGE SEX */
  else if (EQ(wordbuf, "sex"))
    chsex();

  /* CHANGE PASSWORD */
  else if (EQ(wordbuf, "passwd"))
    chpwd();

  /* CHANGE TITLE */
  else if (EQ(wordbuf, "title"))
    chtitle();

  /* Unknown argument */
  else
    bprintf("I don't know how to change that.\n");
}

void
chtitle()
{
  int a, vic_lev;
  char *newtitle = "";
  int ar[4];

  if ((plev(mynum) < LVL_GOD) && !ptstflg(mynum, pfl(ChangeTitle)))
    {
      bprintf("I don't know how to change that.\n");
      return;
    }
  if (vicf2(&a, SPELL_PEACEFUL, 100) == -1)
    return;
  vic_lev = plev(a);
  if ((a != mynum) && (plev(mynum) < LVL_GOD))
    {
      bprintf("You can only change your own title.\n");
      return;
    }
  newtitle = markpos();

  if (vic_lev <= LVL_WIZARD)
    {
      bprintf("Sorry, this is a generic level and cannot be changed.\n");
      return;
    }
  if (EMPTY(newtitle))
    {
      bprintf("What do you want to change your title to?\n");
      return;
    }
  savelevel(vic_lev, newtitle);
}

void
chpwd()
{
  char block[BUFSIZ], data[BUFSIZ], pwd[14], pv[14], *user;
  int x;
  FILE *fl;

  if (plev(mynum) < LVL_GOD)
    user = pname(mynum);
  else {
    if (brkword() == -1) {
      bprintf("Change whose password?\n");
      return;
    }
    if ((x = fpbn(wordbuf)) == -1)
      if (UserExists(user = wordbuf, block)) {
        bprintf("That player does not exist.\n");
        return;
      }
      else ;
    else
      (void)UserExists(user = pname(x), block);
  }

  GetFields(block, data, pwd);
  if (plev(mynum) < LVL_GOD) {
    mygetpass("Old Password: ", data);
    Crypt(data, pwd);
    if (!EQ(data, pwd)) {
      bprintf("Incorrect Password\n");
      return;
    }
  }

  mygetpass("Enter Password: ", pwd);
  if (EMPTY(pwd)) {
    bprintf("Password not changed.\n");
    return;
  }
  mygetpass("Verify Password: ", pv);
  if (!EQ(pv, pwd)) {
    bprintf("Passwords don't match.\n");
    return;
  }
  if (EMPTY(pv)) {
    bprintf("Password not changed.\n");
    return;
  }

  /* delete me and tack me on end */
  Crypt(pwd, salt());
  sprintf(block, "%s\001%s\001", user, pwd);
  delu2(user);
  if ((fl = openlock(PASSWORDFILE, "a")) == NULL)
    return;
  fprintf(fl, "%s\n", block);
  closelock(fl);
  bprintf("Password changed.\n");
}

void
chsex()
{
  int a, vic_lev;
  int ar[4];

  if (vicf2(&a, SPELL_VIOLENT, 7) == -1)
    return;
  vic_lev = plev(a);
  if (vic_lev > LVL_GOD)
    vic_lev = LVL_GOD;
  else if (vic_lev > LVL_ARCHWIZARD)
    vic_lev = LVL_ARCHWIZARD;
  else if (vic_lev > LVL_WIZARD)
    vic_lev = LVL_WIZARD;
  if (vic_lev > plev(mynum))
    {
      bprintf("You can't do that to them.\n");
      return;
    }
  sendsys(pname(a), pname(mynum), SYS_CH_SEX, ploc(mynum), "");
  if (a >= MAX_USERS)
    setpsex(a, 1 - psex(a));
}

void
chvis()
{
  int a, vic_lev, new_vis;
  char vislev[81];
  int ar[4];

  if (plev(mynum) < LVL_WIZARD)
    {
      bprintf("I don't know how to change that.\n");
      return;
    }
  if (vicf2(&a, SPELL_PEACEFUL, 100) == -1)
    return;
  vic_lev = plev(a);
  if (vic_lev > LVL_GOD)
    vic_lev = LVL_GOD;
  else if (vic_lev > LVL_ARCHWIZARD)
    vic_lev = LVL_ARCHWIZARD;
  else if (vic_lev > LVL_WIZARD)
    vic_lev = LVL_WIZARD;
  if (op(mynum))
    vic_lev = 0;
  if (vic_lev >= plev(a))
    {
      bprintf("You can't change their visibility, silly!\n");
      return;
    }
  getreinput(vislev);
  sscanf(vislev, "%d", &new_vis);
  if (new_vis < 0)
    new_vis = 0;
  else if (plev(mynum) < LVL_ARCHWIZARD && new_vis > LVL_WIZARD)
    new_vis = LVL_WIZARD;
  else if (op(mynum))
    new_vis = new_vis;
  else if (new_vis > LVL_ARCHWIZARD)
    new_vis = LVL_ARCHWIZARD;
  sprintf(vislev, "%d", new_vis);
  bprintf("Setting players visibility to %s.\n", vislev);
  setpvis(a, new_vis);
  ar[0] = mynum;
  ar[1] = pvis(mynum);
  sendsys("", "", SYS_VIS, 0, (char *)ar);
  return;
}

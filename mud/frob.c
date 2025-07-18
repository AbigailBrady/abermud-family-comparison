/*
**  The routines for frobbing players.
*/

#include <strings.h>
#include "kernel.h"
#include "macros.h"
#include "sendsys.h"
#include "frob.h"
#include "exec.h"
#include "mud.h"
#include "parse.h"
#include "support.h"
#include "uaf.h"
#include "key.h"

void
frobnicate ()
{
  int x;
  char ary[128], bf1[8], bf2[8], bf3[8];

  if (plev(mynum) < LVL_ARCHWIZARD)
    {
      erreval();
      return;
    }
  if (brkword() == -1)
    {
      bprintf("Frob whom?\n");
      return;
    }
  if ((x = fpbn(wordbuf)) == -1)
    {
      frobinfile();
      return;
    }
  if (x >= MAX_USERS)
    {
      bprintf("Can't frob mobiles, old bean.\n");
      return;
    }
  if ((plev(x) >= LVL_ARCHWIZARD) && (plev(mynum) < LVL_GOD))
    {
      bprintf("You can't frob %s!\n", pname(x));
      return;
    }
  bprintf("Level is: %d\n", plev(x));
  sig_alon();
  key_input("New Level: ", 6);
  sig_aloff();
  strcpy(bf1, key_buff);
  if (atoi(bf1) >= LVL_ARCHWIZARD && plev(mynum) < LVL_GOD)
    {
      bprintf("You can't do that.\n");
      return;
    }
  bprintf("Score is: %d\n", pscore(x));
  sig_alon();
  key_input("New Score: ", 8);
  sig_aloff();
  strcpy(bf2, key_buff);
  bprintf("Strength is: %d\n", pstr(x));
  sig_alon();
  key_input("New Strength: ", 7);
  sig_aloff();
  strcpy(bf3, key_buff);
  sprintf(ary, "%s.%s.%s.", bf1, bf2, bf3);
  sendsys(pname(x), pname(x), SYS_FROB, 0, ary);
  closeworld();
  mudlog("FROB: %s by %s", pname(x), pname(mynum));
  mudlog("  FROB: t%s %s", ary, "");
  bprintf("Ok\n");
}

void
frobinfile ()
{
  PERSONA x;
  char ary[128];
  char bf1[8], bf2[8], bf3[8];

  if (findpers(wordbuf, &x) == 0)
    {
      bprintf("No such persona in system.\n");
      return;
    }
  if (x.p_level >= LVL_ARCHWIZARD && plev(mynum) < LVL_GOD)
    {
      bprintf("You can't frob %s!\n", wordbuf);
      return;
    }
  bprintf("Level is: %d\n", x.p_level);
  sig_alon();
  key_input("New Level: ", 6);
  sig_aloff();
  strcpy(bf1, key_buff);
  if (atoi(bf1) >= LVL_ARCHWIZARD && plev(mynum) < LVL_GOD)
    {
      bprintf("You can't do that.\n");
      return;
    }
  bprintf("Score is: %d\n", x.p_score);
  sig_alon();
  key_input("New Score: ", 7);
  sig_aloff();
  strcpy(bf2, key_buff);
  bprintf("Strength is: %d\n", x.p_strength);
  sig_alon();
  key_input("New Strength: ", 7);
  sig_aloff();
  strcpy(bf3, key_buff);
  sprintf(ary, "%s.%s.%s.", bf1, bf2, bf3);
  mudlog("FROB: %s by %s", wordbuf, pname(mynum));
  mudlog("FROB: %s %s", ary, "");
  sscanf(bf1, "%d", &(x.p_level));
  sscanf(bf2, "%d", &(x.p_score));
  sscanf(bf3, "%d", &(x.p_strength));
  putpers(wordbuf, &x);
  bprintf("Ok\n");
}

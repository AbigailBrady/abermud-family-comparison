/*
**  This file contains commands only usable by gods
*/

#include "kernel.h"
#include "bprintf.h"
#include "macros.h"
#include "exec.h"
#include "blib.h"
#include "god.h"

void
spawncom()
{
  if (plev(mynum) < LVL_ARCHWIZARD)
    {
      bprintf("You feel an irresistable urge to swim upstream.\n");
      return;
    }
  if (spawned)
    {
      openworld();
      bprintf("Welcome back to the real world.\n");
    }
  else
    {
      bprintf("Creating the universe in six days...\n");
      closeworld();
    }
  spawned = !spawned;
}

void maxusercom()
{
  if ((brkword() == -1) || (plev(mynum) < LVL_GOD)) {
    bprintf("Up to %d players may be on at once.\n", shimge[4]);
    return;
  }
  shimge[4] = atoi(wordbuf);
  if (shimge[4] < 1)
    shimge[4] = 1;
  if (shimge[4] > MAX_USERS)
    shimge[4] = MAX_USERS;
  bprintf("Setting maximum users to %d.\n", shimge[4]);
}

void shutdowncom()
{
  FILE *nologin_file;
  char s[128];

  if (plev(mynum) < LVL_GOD) {
    erreval();
    return;
  }
  getreinput(s);
  if (EMPTY(s))
    sprintf(s, "%s\n", "AberMUD is closed for hacking.  Please try again later.");
  mudlog("SHUTDOWN by %s", pname(mynum));
  if ((nologin_file = openlock(NOLOGIN, "w")) == NULL) {
    loseme();
    crapup("Unauthorized SHUTDOWN attempt.");
  }
  fprintf(nologin_file, "%s\n", s);
  closelock(nologin_file);
  bprintf("AberMUD is now closed.\n");
}

void deluser(char *user)
{
  char name[80], block[BUFSIZ];

  if (UserExists(user, block) == -1)
    bprintf("Cannot delete non-existant user\n");
  else {
    delu2(user);
    bprintf("Ok\n");
  }
}

void delu2(char *name)
{
  char b2[128], buff[128], b3[128];
  FILE *a, *b;

  if ((a = openlock(PASSWORDFILE, "r")) == NULL)
    return;
  if ((b = openlock(PASSTEMP, "w")) == NULL) {
    fclose(a);
    return;
  }
  strcpy(b3, name);
  while (fgets(buff, sizeof buff, a)) {
    GetFields(buff, b2, (char *)NULL);
    if (!EQ(b3, b2))
      fprintf(b, "%s", buff);
  }
  closelock(a);
  closelock(b);
  if ((a = openlock(PASSWORDFILE, "w")) == NULL)
    return;
  if ((b = openlock(PASSTEMP, "r")) == NULL) {
    fclose(a);
    return;
  }
  while (fgets(buff, sizeof buff, b))
    fprintf(a, "%s", buff);
  closelock(a);
  closelock(b);
}

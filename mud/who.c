#define DATA_DEF
#include <stdio.h>
#include "kernel.h"

crapup(char *m)
{
  closeworld();
  fprintf(stderr, "fatal: %s\n", m);
  exit(1);
}

char *pname(int chr)
{
  return((char *)&ublock[16 * chr]);
}

main()
{
  int a;

  if (chdir(GAME_DIR) < 0) {
	crapup("Could not change to game directory.");
  }
  filrf = (FILE *)0;
  openworld();
  for (a=0; a<MAX_USERS; a++) {
	if (EMPTY(pname(a))) {
		continue;
	}
	printf("%s\n", pname(a));
  }
  closeworld();
}



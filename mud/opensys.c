/*
**  Open and close the world file (with locking)
*/

#include "kernel.h"
#include "macros.h"
#include "blib.h"
#include "opensys.h"
#include "mud.h"

static char sccsid[] = "@(#)opensys.c	4.100.0 (IthilMUD)	6/02/90";

#define UBLOCK 201

void closeworld()
{
  if (spawned || filrf == NULL)
    return;
  sec_write(filrf, objinfo, UBLOCK + 41, o_ofs(numobs));
  sec_write(filrf, (int*)shimge, UBLOCK + 40, 16);
  sec_write(filrf, ublock, UBLOCK, 16 * MAX_CHARS);
  closelock(filrf);
  filrf = NULL;
}

FILE *openworld()
{
  if (spawned)
    return NULL;
  if (filrf != NULL)
    return filrf;
  if ((filrf = openlock(UNIVERSE, "r+")) == NULL)
    crapup("Cannot find World file");
  sec_read(filrf, objinfo, UBLOCK + 41, o_ofs(numobs));
  sec_read(filrf, (int*)shimge, UBLOCK + 40, 16);
  sec_read(filrf, ublock, UBLOCK, 16 * MAX_CHARS);
  return filrf;
}

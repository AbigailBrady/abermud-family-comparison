/*
**  The executive to drive the tables
*/

#include <setjmp.h>
#include "kernel.h"
#include "macros.h"
#include "condact.h"
#include "exec.h"
#include "action.h"
#include "support.h"
#include "mud.h"
#include "pflags.h"

static char sccsid[] = "@(#)exec.c	4.100.0 (IthilMUD)	6/02/90";

#define MAX_NEST	8
static jmp_buf	 escape_bf[MAX_NEST];	/* Table escape			*/
static int	 ct;			/* Offset Counter		*/
static LINE	*crlin;			/* Current Line			*/
static int	 doing_table = 0; 	/* True if doing table		*/
static int	 matchq;

void jumpaway()
{
  longjmp(escape_bf[--doing_table], 1);
}

/*
**  Routine to execute a table
*/
int exectable(LINE *v)
{
  matchq = 0;
  if (setjmp(escape_bf[doing_table])) {
    return matchq;
  }
  if (++doing_table > MAX_NEST) {
    crapup("exectable(): Nested too deep!");
  }
  for (; ; v++) {
    crlin = v;
    if (v->verb == -1)
      break;
    if (v->verb && v->verb != verbcode)
      continue;
    if (v->item1 >= 0) {
      if (v->item1 > 9999) {
	if (v->item1 != P(pl1))
	  continue;
      }
      else if (v->item1 != ob1)
	continue;
    }
    else {
      switch (v->item1) {
      case PL:
	if (pl1 == -1)
	  continue;
	break;
      case OB:
	if (ob1 == -1)
	  continue;
	break;
      case NONE:
	if (pl1 != -1 || ob1 != -1)
	  continue;
      }
    }
    if (v->item2 >= 0) {
      if (v->item2 > 9999) {
	if (v->item2 != P(pl2))
	  continue;
      }
      else if (v->item2 != ob2)
	continue;
    }
    else {
      switch (v->item2) {
      case PL:
	if (pl2 == -1)
	  continue;
	break;
      case OB:
	if (ob2 == -1)
	  continue;
	break;
      case NONE:
	if (pl2 != -1 || ob2 != -1)
	  continue;
      }
    }
    /* The item matches on vocab */
    matchq = 1;
    for (ct = 0, pptr = 0; ct < 6; ct++) {
      if (!condeval(v->conditions[ct].cnd_code, v->conditions[ct].cnd_data))
	break;
    }
    if (ct < 6)
      continue;
    pptr = 0;
    matchq = 2;
    stdaction(v->actions[0]);
    stdaction(v->actions[1]);
    stdaction(v->actions[2]);
    stdaction(v->actions[3]);
  }
  --doing_table;
  return matchq;
}

int getnext()
{
  return ct == 6 ? 0 : prmmod(crlin->conditions[++ct].cnd_data);
}

int getpar()
{
  switch (parameters[pptr++]) {
  case FL_PL1:
    return pl1;
  case FL_PL2:
    return pl2;
  case FL_OB1:
    return ob1;
  case FL_OB2:
    return ob2;
  case FL_CURCH:
    return ploc(mynum);
  case FL_PREP:
    return prep;
  case FL_LEV:
    return plev(mynum);
  case FL_STR:
    return pstr(mynum);
  case FL_SEX:
    return psex(mynum);
  case FL_SCORE:
    return pscore(mynum);
  case FL_MYNUM:
    return mynum;
  }
  pptr--;
/*
  if (parameters[pptr] > 9999)
    return globaldata[parameters[pptr++] - 10000];
*/
  return parameters[pptr++];
}

int prmmod(int p)
{
  switch (p) {
  case FL_PL1:
    return pl1;
  case FL_PL2:
    return pl2;
  case FL_OB1:
    return ob1;
  case FL_OB2:
    return ob2;
  case FL_CURCH:
    return ploc(mynum);
  case FL_PREP:
    return prep;
  case FL_LEV:
    return plev(mynum);
  case FL_STR:
    return pstr(mynum);
  case FL_SEX:
    return psex(mynum);
  case FL_SCORE:
    return pscore(mynum);
  case FL_MYNUM:
    return mynum;
  }
  return p;
/*  return (p > 9999 ? globaldata[p - 10000] : p); */
}

int getparnz()
{
  int a;
    
  if ((a = getpar()) == 0) {
    mudlog("zero operand trap","","");
    crapup("Everything dissolves in a haze of chaos....");
  }
  return a;
}

int tables(int v)
{
  parse_2(v);
  exectable(event);
  return matchq;
}

void error()
{
  if (doing_table)
    jumpaway();
}

void erreval()
{
  if (plev(mynum) < LVL_GOD)
    bprintf("You can't do that now.\n");
  else {
    bprintf("I am sorry, oh great and wise One, but I could not understand.\n");
  }
}

void dostatus()
{
  exectable(status);
}

void dointerrupt()
{
  exectable(inter);
}

/*
**  Condition evaluator for the system
*/

#include "kernel.h"
#include "support.h"
#include "condact.h"
#include "oflags.h"
#include "pflags.h"
#include "macros.h"
#include "mud.h"

static char sccsid[] = "@(#)condition.c	4.100.0 (IthilMUD)	6/02/90";

condeval(int n, int p2)
{
  int a, b, r, p;

  p = prmmod(p2);
  a = n & NOT;

  switch (n & COND) {
  case LEVEQ:
    r = plev(mynum) == p;
    break;
  case LEVLT:
    r = plev(mynum) < p;
    break;
  case IN:
    r = ploc(mynum) == p;
    break;
  case GOT:
    r = iscarrby(p, mynum);
    break;
  case AVL:
    r = isavl(p);
    break;
  case IFDEAF:
    r = ptstflg(mynum, pfl(Deaf));
    break;
  case IFDUMB:
    r = ptstflg(mynum, pfl(Dumb));
    break;
  case IFBLIND:
    r = ptstflg(mynum, pfl(Blind));
    break;
  case IFCRIPPLED:
    r = ptstflg(mynum, pfl(Crippled));
    break;
  case IFFIGHTING:
    r = in_fight;
    break;
  case IFSITTING:
    r = psitting(mynum);
    break;
  case PFLAG:
    r = ptstflg(p, getnext());
    break;
  case STATE:
    r = state(p) == getnext();
    break;
/*
  case EQc:
    r = globaldata[p] == getnext();
    break;
  case LT:
    r = globaldata[p] < getnext();
    break;
  case GT:
    r = globaldata[p] > getnext();
    break;
*/
  case VISLT:
    r = pvis(p) < getnext();
    break;
  case VISEQ:
    r = pvis(p) == getnext();
    break;
  case CANSEE:
    r = pvis(p) <= plev(mynum);
    break;
  case STRLT:
    r = pstr(mynum) < p;
    break;
  case STREQ:
    r = pstr(mynum) == p;
    break;
  case ISIN:
    r = iscontin(p, getnext());
    break;
  case HERE:
    r = ishere(p);
    break;
  case PLGOT:
    r = iscarrby(p, getnext());
    break;
  case DESTROYED:
    r = otstbit(p, ofl(Destroyed));
    break;
  case CHANCE:
    r = randperc() < p;
    break;
  case OBIT:
    r = otstbit(p, getnext());
    break;
  case INLT:
    r = ploc(mynum) < p;
    break;
  case INGT:
    r = ploc(mynum) > p;
    break;
  case ISME:
    r = p == mynum;
    break;
  case DARK:
    r = isdark();
    break;
  case OBJAT:
    r = oloc(p) == getnext() && ocarrf(p) == 0;
    break;
  case CHANCELEV:
    r = randperc() < p * plev(mynum);
    break;
  case WIZ:
    r = plev(mynum) >= LVL_WIZARD;
    break;
  case AWIZ:
    r = plev(mynum) >= LVL_ARCHWIZARD;
    break;
  case OP:
    r = op(mynum);
    break;
  case PLWORN:
    r = iswornby(p, getnext());
    break;
  case PAR:
    parameters[pptr++] = p2;
    r = 1;
    break;
  case TSTSEX:
    r = psex(mynum) == p;
    break;
  case WPN:
    r = pwpn(mynum) == p;
    break;
  case ENEMY:
    r = fighting == p;
    break;
  case CANGO:
    r = cango(p);
    break;
  case PREP:
    r = prep == p;
    break;
  case ALONE:
    r = tscale() == 100;
    break;
  case WILLFITIN:
    r = willhold(p, getnext());
    break;
  case ISMONSTER:
    r = p >= MAX_USERS;
    break;
  case CANCARRY:
    r = cancarry(p);
    break;
  case OBYTEEQ:
    b = getnext();
    r = obyte(p, b) == getnext();
    break;
  case OBYTEGT:
    b = getnext();
    r = obyte(p, b) > getnext();
    break;
  case OBYTEZ:
    r = obyte(p, getnext()) == 0;
    break;
  case HASANY:
    r = ohany(p) == 1;
    break;
  case HELPED:
    r = ptothlp(mynum) > 0;
    break;
  case PHERE:
    r = ploc(p) == ploc(mynum);
    break;
  default:
    mudlog("bad condition - code %d", (char *)n, "");
    crapup("Something unexpectedly nasty has happened");
  }
  return a ? !r : r;
}

cango(int d)
{
  if (getexit(ploc(mynum), d) == 0)
    return 0;
  if (getexit(ploc(mynum), d) < 0)
    return 1;
  if (getexit(ploc(mynum), d) < 1000)
    return 0;
  if (state(getexit(ploc(mynum), d) - 1000) == 0)
    return 1;
  return 0;
}

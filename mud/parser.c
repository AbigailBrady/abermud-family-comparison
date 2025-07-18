/*
**  The Parser
*/

#include <strings.h>
#include "kernel.h"
#include "macros.h"
#include "parser.h"
#include "support.h"
#include "pflags.h"

static char sccsid[] = "@(#)parser.c	4.100.0 (IthilMUD)	6/02/90";

char *markpos()
{
    for (; strbuf[stp]; stp++)
	if (!isspace(strbuf[stp]) && strbuf[stp] != ',')
	    break;
    return strbuf + stp;
}

void
parse_2(int vb)
{
  int savstp;

  if ((verbcode = vb) == -1)
    return;
  savstp = stp;
  txt1 = markpos();
  strcpy(item1, "");
  strcpy(item2, "");
  if (brkword() == -1) {
    ob1 = -1;
    ob2 = -1;
    prep = 0;
    pl1 = -1;
    pl2 = -1;
    stp = savstp;
    return;
  }
  strcpy(item1, wordbuf);
  if (findprep(item1) != -1) {
    ob1 = -1;
    pl1 = -1;
  }
  else {
    txt2 = markpos();
    pl1 = fpbn(item1);
    if ((ob1 = fobnc(item1)) == -1)
      ob1 = fobna(item1);
    if (pl1 != -1)
      setthem(pl1);
    if (ob1 != -1)
      strcpy(wd_it, oname(ob1));
    if (brkword() == -1) {
      ob2 = -1;
      pl2 = -1;
      prep = 0;
      stp = savstp;
      return;
    }
  }
  if ((prep = findprep(wordbuf)) == -1)
    prep = 0;
  else if (brkword() == -1) {
    pl2 = -1;
    ob2 = -1;
    stp = savstp;
    return;
  }
  strcpy(item2, wordbuf);
  pl2 = fpbn(wordbuf);
  if ((ob2 = fobnc(wordbuf)) == -1)
    ob2 = fobna(wordbuf);
  if (pl2 != -1)
    setthem(pl2);
  if (ob2 != -1)
    strcpy(wd_it, oname(ob2));
  if (txt2 == NULL)
    txt2 = markpos();
  stp = savstp;
}

void setthem(int p)
{
    switch (psex(p)) {
    case 0:
	strcpy(wd_him, pname(p));
	break;
    case 1:
	strcpy(wd_her, pname(p));
	break;
    case 2:
	strcpy(wd_it, pname(p));
	break;
    }
    strcpy(wd_them, pname(p));
}

int findprep(char *x)
{
    if (EQ(x, "at"))
	return 1;
    if (EQ(x, "on"))
	return 2;
    if (EQ(x, "with"))
	return 3;
    if (EQ(x, "to"))
	return 4;
    if (EQ(x, "in"))
	return 5;
    if (EQ(x, "into"))
	return 5;
    if (EQ(x, "from"))
	return 6;
    if (EQ(x, "out"))
	return 7;
    return -1;
}

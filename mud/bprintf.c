/*
**  This module and key.c together handle all of the I/O in the game
*/

#include <strings.h>
#include <stdarg.h>
#include "kernel.h"
#include "blib.h"
#include "pflags.h"
#include "lflags.h"
#include "bprintf.h"
#include "macros.h"
#include "mud.h"
#include "support.h"
#include "exec.h"
#include "parse.h"
#include "bsearch.h"
#include "sendsys.h"

#define SYSBUFSIZE 4096
#define MAX_ARGS   2

#define prnt(x, f) if (x.nest) dcprnt(x.str, f); else kiputs(x.str, f);

static char *bufptr, *sysbuf;
static int buflen;

struct arg
{
  char *str;
  int nest;
};

void pansi (struct arg [], FILE *);
void pcls (struct arg [], FILE *);
void ppnblind (struct arg [], FILE *);
void pryou (struct arg [], FILE *);
void pryour (struct arg [], FILE *);
void ppndeaf (struct arg [], FILE *);
void psleep (struct arg [], FILE *);
void pndark (struct arg [], FILE *);
void pndeaf (struct arg [], FILE *);
void pfile (struct arg [], FILE *);
void pnotkb (struct arg [], FILE *);
void prname (struct arg [], FILE *);
void pcansee (struct arg [], FILE *);


struct _code
{
  char name;
  int args;
  void (*func)(struct arg [], FILE *);
};
static struct _code codes[] =
  {{'A', 1, pansi   },
   {'C', 0, pcls    },
   {'D', 1, ppnblind},
   {'N', 1, pryou   },
   {'P', 1, ppndeaf },
   {'S', 1, psleep  },
   {'c', 1, pndark  },
   {'d', 1, pndeaf  },
   {'f', 1, pfile   },
   {'l', 1, pnotkb  },
   {'n', 1, pryour  },
   {'p', 1, prname  },
   {'s', 2, pcansee }};

#define NUM_CODES (sizeof codes / sizeof(struct _code))


void makebfr()
{
  if ((bufptr = sysbuf = NEW(char, SYSBUFSIZE+BUFSIZ)) == NULL)
    crapup("Out Of Memory");
  sysbuf[0] = 0;
  buflen = 0;
}

int
_code_cmp (const void *c, const void *code)
{
  return (*((const char *)c) - ((const struct _code *)code)->name);
}

static void
kiputs (char *s, FILE *file)
{
  if (pr_qcr) {
    putchar('\n');
    pr_qcr = False;
  }
  fputs(s, file);
  pr_due = True;
}

int
tocontinue(char **ct)
{
  register char *s = *ct;
  register int n = 1;
  register int nest = 0;
  struct _code *code;

  for (; n; s++) {
    switch (*s) {
      case '\002':
      case '\377':
	--n;
#ifdef DEBUG
	fprintf(stderr, "tocontinue(): one arg found, %d left\n", n);
#endif
	break;
      case '\001':
	code = bsearch((void *)(++s), (void *)codes, NUM_CODES,
		       sizeof(struct _code), _code_cmp);
	if (code == NULL) {
	  mudlog("ERROR: tocontinue(): Unknown control code\n");
	  fprintf(stderr, "ERROR: tocontinue(): Unknown control code\n");
	  _exit(2);
	}
#ifdef DEBUG
	fprintf(stderr, "tocontinue(): code %c found; %d args left\n", *ct, n);
#endif
	n += code->args;
	nest = 1;
	break;
      case '\0':
	mudlog("ERROR: tocontinue(): Buffer overrun\n");
	fprintf(stderr, "ERROR: tocontinue(): Buffer overrun\n");
	_exit(2);
	break;
    }
  }

  *ct = s;
  return(nest);
}

void
dcprnt (char *ct, FILE *file)
{
  char *str;
  struct arg args[MAX_ARGS+1];
  struct _code *code;
  int n;

  while (*(str = ct)) {
    if ((ct = index(str, '\001')) == NULL) {
      kiputs(str, file);
      break;
    }
    *ct = '\0';
    kiputs(str, file);
    *(ct++) = '\001';
#ifdef DEBUG
    fprintf(stderr, "dcprnt(): lookup code %c\n", *ct);
#endif
    code = bsearch((void *)ct, (void *)codes, NUM_CODES,
		   sizeof(struct _code), _code_cmp);
    if (code == NULL) {
      mudlog("ERROR: dcprnt(): Unknown control code\n");
      fprintf(stderr, "ERROR: dcprnt(): Unknown control code\n");
      _exit(2);
    } else {
#ifdef DEBUG
      fprintf(stderr, "dcprnt(): code found; %d args\n", code->args);
#endif
      args[0].str = ++ct;
      for (n = 1; n <= code->args; n++) {
#ifdef DEBUG
	fprintf(stderr, "dcprnt(): parsing argument %d\n", n);
	fprintf(stderr, "dcprnt(): %s\n", ct);
#endif
	args[n-1].nest = tocontinue(&ct);
#ifdef DEBUG
	fprintf(stderr, "dcprnt(): %d, %d\n", n, args[n-1].nest);
#endif
	*(--ct) = '\0';
	args[n].str = ++ct;
      }
#ifdef DEBUG
      fprintf(stderr, "dcprnt(): executing %c with first arg %s\n", code->name,
	      args[0]);
#endif
      code->func(args, file);
      for (n = 1; n <= code->args; n++)
	args[n].str[-1] = '\377';
    }
  }
}

void pwait(void)
{
  key_input("Press <Return> to continue...", 0);
}

void pfile(struct arg *args, FILE *file)
{
  FILE *a;
  char x[BUFSIZ];

  if ((a = fopen(args[0].str, "r")) == NULL)
    fprintf(stderr, "[Cannot find file ->%s]\n", args[0]);
  else {
    while (fgets(x, sizeof(x), a))
      dcprnt(x, file);
    fclose(a);
  }
}

void pndeaf(struct arg *args, FILE *file)
{
  if (!ptstflg(mynum, pfl(Deaf)))
    prnt(args[0], file);
}

void pcls(struct arg *args, FILE *file)
{
  cls();
}

void pcansee(struct arg *args, FILE *file)
{
  int a;

  a = fpbns(args[0].str);
  if (seeplayer(a) && (a != -1))
    prnt(args[1], file);
}

void prname(struct arg *args, FILE *file)
{
  kiputs(seeplayer(fpbns(args[0].str)) ? args[0].str : "Someone", file);
}

void
pryou (struct arg *args, FILE *file)
{
  kiputs((fpbns(args[0].str) == mynum) ? "you" : args[0].str, file);
}

/*
  Prints "your" if player is the receiver, otherwise prints the
  possessive form of the receiver's name
*/

void
pryour (struct arg *args, FILE *file)
{
  if (fpbns(args[0].str) == mynum)
    kiputs("your", file);
  else
    {
      kiputs(args[0].str, file);
      kiputs("'s", file);
    }
}

void pndark(struct arg *args, FILE *file)
{
  if ((!isdark()) && (!ptstflg(mynum, pfl(Blind))))
    prnt(args[0], file);
}

void pansi(struct arg *args, FILE *file)
{
  if (ptstflg(mynum, pfl(Color)))
    prnt(args[0], file);
}

int seeplayer(int x)
{
  if (x == -1 || x == mynum)
    return 1;
  if (pvis(x) > 0 && plev(mynum) < pvis(x))
    return 0;
  if (ptstflg(mynum, pfl(Blind)))
    return 0;
  if (ploc(mynum) == ploc(x) && isdark())
    return 0;
  setname(x);
  return 1;
}

void ppndeaf(struct arg *args, FILE *file)
{
  if (!ptstflg(mynum, pfl(Deaf)))
    kiputs(seeplayer(fpbns(args[0].str)) ? args[0].str : "Someone", file);
}

void ppnblind(struct arg *args, FILE *file)
{
  if (!ptstflg(mynum, pfl(Blind)))
    kiputs(seeplayer(fpbns(args[0].str)) ? args[0].str : "Someone", file);
}

void logcom()
{
  if (log_fl) {
    fprintf(log_fl, "\nEnd of log....\n\n");
    fclose(log_fl);
    log_fl = NULL;
    bprintf("End of log\n");
    return;
  }
  bprintf("Commencing Logging of Session\n");
  setreuid(-1, userid);
  if ((log_fl = fopen("mud_log", "a")) == NULL)
    log_fl = fopen("mud_log", "w");
  setreuid(-1, sys_uid);
  if (log_fl == NULL) {
    bprintf("Cannot open log file mud_log\n");
    return;
  }
  bprintf("The log will be written to the file 'mud_log'\n");
}

void
pbfr()
{
  FILE *fln;

  if (!sysbuf)
    makebfr();

  block_alarm();
  if (buflen)
    {
      if (!key_mode)
	pr_qcr = False;
      if (log_fl) {
	iskb = False;
	dcprnt(sysbuf, log_fl);
	fflush(log_fl);
      }
      if (iamon)
	if (snoopd != -1 && (fln = opensnoop(pname(snoopd), "a")) != NULL)
	  {
	    iskb = False;
	    dcprnt(sysbuf, fln);
	    closelock(fln);
	  }
      iskb = True;
      dcprnt(sysbuf, stdout);
      sysbuf[0] = 0;		/* clear buffer */
      bufptr = sysbuf;
      buflen = 0;
    }
  pr_qcr = False;
  unblock_alarm();
}

void bflush(void)
{
  pbfr();
  fflush(stdout);
}

void psleep(struct arg *args, FILE *file)
{
  fflush(stdout);
  sleep(atoi(args[0].str));
}

void pnotkb(struct arg *args, FILE *file)
{
  if (!iskb)
    prnt(args[0], file);
}

FILE *opensnoop(char *user, char *per)
{
  char x[BUFSIZ];

  sprintf(x, "%s%s", SNOOPFILE, user);
  return openlock(x, per);
}

void
snoopcom()
{
  FILE *fx;
  int x;

  if (plev(mynum) < LVL_WIZARD)
    {
      erreval();
      return;
    }
  if (snoopt != -1)
    {
      bprintf("Stopped snooping on %s.\n", sntn);
      snoop_off();
    }
  if (pl1 == -1 && ob1 == -1)
    return;
  if ((x = pl1) == -1)
    {
      bprintf("Who's that?\n");
      return;
    }
  if ((plev(mynum) < LVL_ARCHWIZARD && plev(x) >= LVL_WIZARD)
      || (plev(mynum) < LVL_GOD && ptstflg(x, pfl(NoSnoop))))
    {
      bprintf("Your magical vision is obscured.\n");
      snoopt = -1;
      return;
    }
  /* Is this a PRIVATE room?  If so, don't let anyone snoop. */
  if (ltstflg(ploc(x), lfl(Private)) && plev(mynum) < LVL_GOD)
    {
      bprintf("I'm sorry, sir, but that room has been soundproofed.\n");
      snoopt = -1;
      return;
    }
  strcpy(sntn, pname(x));
  snoopt = x;
  bprintf("Started to snoop on %s.\n", pname(x));
  sendsys(sntn, pname(mynum), SYS_SNOOP_ON, 0, "");
  fx = opensnoop(pname(mynum), "w");
  closelock(fx);
  mudlog("%s snooped %s", pname(mynum), pname(x));
}

void
viewsnoop()
{
  FILE *fx;
  char x[BUFSIZ+1] = {'|'};

  if (snoopt == -1 || (fx = opensnoop(pname(mynum), "r+")) == NULL)
    return;
  while (!feof(fx) && fgets(&x[1], sizeof(x), fx))
    if (x[1])
      dcprnt(x, stdout);
  ftruncate(fileno(fx), 0L);
  closelock(fx);
}

void
snoop_off()
{
  if (snoopt != -1)
    sendsys(sntn, pname(mynum), SYS_SNOOP_OFF, 0, "");
  snoopt = -1;
}

/* Assign him or her according to who it is */
void
setname(int x)
{
  char *p;

  p = pname(x);
  if (psex(x))
    strcpy(wd_her, p);
  else
    strcpy(wd_him, p);
  strcpy(wd_them, p);
}

void
bprintf(char *format, char *a1, char *a2, char *a3, char *a4, char *a5,
	char *a6, char *a7, char *a8, char *a9)
{
  register int len;

  if (!sysbuf)
    makebfr();

  sprintf(bufptr, format, a1, a2, a3, a4, a5, a6, a7, a8, a9);
  len = strlen(bufptr);
  buflen += len;
  if (buflen >= SYSBUFSIZE)
    pbfr();
  else
    bufptr += len;
}

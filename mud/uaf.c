/*
**  User access files routines.
*/

#include <strings.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "kernel.h"
#include "macros.h"
#include "blib.h"
#include "uaf.h"
#include "mud.h"
#include "support.h"
#include "bprintf.h"
#include "key.h"
#include "pflags.h"

#define PCTL_GET	0
#define PCTL_FIND	1

#define PL_GOD		0
#define PL_ARCHWIZARD	1
#define PL_WIZARD	2
#define PL_APPRWIZARD	3
#define PL_TOTAL	4

typedef struct plist {
  PERSONA	*pl_persona;
  struct plist	*pl_next;
} PLIST;


static FILE *openuaf(char *perm)
{
  FILE *i;
  
  if ((i = openlock(UAF_RAND, perm)) == NULL)
    crapup("Cannot access UAF\n");
    return i;
}

static FILE *personactl(char *name, PERSONA *d, int act)
{
  FILE *uaf_file;
  char c[32], e[32];
  
  uaf_file = openuaf("r+");
  lowercase(strcpy(e, name));
  while (fread((char *)d, sizeof *d, 1, uaf_file) == 1)
    if (EQ(strcpy(c, d->p_name), e))
      switch (act) {
      case PCTL_GET:
	closelock(uaf_file);
	d->p_strength = ntohl(d->p_strength);
	d->p_score = ntohl(d->p_score);
	d->p_level = ntohl(d->p_level);
	d->p_sex = ntohl(d->p_sex);
	/* Hack, return pointer to closed file. */
		return uaf_file;
      case PCTL_FIND:
	/* Back up one spot. */
	fseek(uaf_file, ftell(uaf_file) - sizeof *d, 0);
	return uaf_file;
      }
  closelock(uaf_file);
  return NULL;
}

int findpers(char *name, PERSONA *d)
{
  return personactl(name, d, PCTL_GET) != NULL;
}

void delpers(char *name)
{
  FILE *i;
  PERSONA d;
  
  /* Was while loop but something happened and this looped forever... */
  if (i = personactl(name, &d, PCTL_FIND)) {
    mudlog("Deleted %s", name,"");
    if (!EQ(d.p_name, name))
      crapup("Panic: Invalid Persona Delete");
    d.p_name[0] = '\0';
    d.p_level = -1;
    fwrite((char *)&d, sizeof d, 1, i);
    closelock(i);
    }
}

void putpers(char *name, PERSONA *d)
{
  FILE *i;
  off_t flen;
  PERSONA s;

  d->p_strength = htonl(d->p_strength);
  d->p_level = htonl(d->p_level);
  d->p_sex = htonl(d->p_sex);
  d->p_score = htonl(d->p_score);
  if ((i = personactl(name, &s, PCTL_FIND)) == NULL) {
	flen = -1;
	if ((i = personactl("", &s, PCTL_FIND)) == NULL) {
	  i = openuaf("a");
	  flen = ftell(i);
	}
	if (fwrite((char *)d, sizeof *d, 1, i) != 1) {
	  bprintf("Save Failed - Device Full?\n");
	  if (flen != -1)
	    ftruncate(fileno(i), (off_t)flen);
	}
      }
    else
	fwrite((char *)d, sizeof *d, 1, i);
  closelock(i);
}

void decpers(PERSONA *d, char *name, int *str, int *score, int *lev, int *sex)
{
  strcpy(name, d->p_name);
  *str = d->p_strength;
  *score = d->p_score;
  *lev = d->p_level;
  *sex = d->p_sex;
}

void initme()
{
  PERSONA d;
  char s[32];

  zapped = False;
  if (findpers(globme, &d)) {
	decpers(&d, s, &my_str, &my_sco, &my_lev, &my_sex);
	if (my_str >= 0)
	    return;
  }
  d.p_score = 0;
  bprintf("Creating character....\n");
  my_sco = 0;
  my_str = 40;
  my_lev = 1;
  for (my_sex = -1; my_sex < 0; ) {
    bprintf("\nSex (M/F) : ");
    pbfr();
    key_input(">", 2);
    switch (key_buff[0]) {
    case 'M': case 'm':
      my_sex = 0;
      break;
    case 'F': case 'f':
      my_sex = 1;
      break;
    default:
      bprintf("M or F");
    }
  }
  strcpy(d.p_name, globme);
  d.p_strength = my_str;
  d.p_level = my_lev;
  d.p_sex = my_sex;
  d.p_score = my_sco;
  putpers(globme, &d);
}

void
saveme ()
{
  PERSONA d;
  
  strcpy(d.p_name, globme);
  d.p_strength = pstr(mynum);
  d.p_level = plev(mynum);
  d.p_sex = psexall(mynum);
  d.p_score = pscore(mynum);
  if (! zapped)
    {
      bprintf("\nSaving %s\n", pname(mynum));
      bflush();
      putpers (globme, &d);
    }
}

void
findwizards(PLIST *pl[])
{
  FILE *uaf_file;
  PERSONA *pers;
  PLIST *cur[PL_TOTAL], *new;
  int pos;

  bzero((char *)pl, sizeof(PLIST *)*PL_TOTAL);
  bzero((char *)cur, sizeof(PLIST *)*PL_TOTAL);
  pers = (PERSONA *)malloc(sizeof(PERSONA));
  uaf_file = openuaf ("r");

  while (fread ((char *) pers, sizeof (PERSONA), 1, uaf_file) == 1) {
	pers->p_level = ntohl(pers->p_level);
	pers->p_sex = ntohl(pers->p_sex);
	if (pers->p_level >= LVL_WIZARD) {
		if (pers->p_level >= LVL_GOD) {
			pos = PL_GOD;
		} else if (pers->p_level >= LVL_ARCHWIZARD) {
			pos = PL_ARCHWIZARD;
		} else if (pers->p_level >= LVL_WIZARD+1) {
			pos = PL_WIZARD;
		} else if (pers->p_level >= LVL_WIZARD) {
			pos = PL_APPRWIZARD;
		}
		new = (PLIST *)malloc(sizeof(PLIST));
		new->pl_persona = pers;
		new->pl_next = (PLIST *)0;
		pers = (PERSONA *)malloc(sizeof(PERSONA));
		if (!cur[pos]) {
			pl[pos] = cur[pos] = new;
		} else {
			cur[pos]->pl_next = new;
			cur[pos] = new;
		}
	}
  }

  free(pers);
  closelock(uaf_file);
}

void
diswizards(char *header, PLIST *list, Boolean show_title)
{
  PLIST *cur;
  int cnt;

  if (list) {
	bprintf(header);
  }
  cnt = 0;
  while (list) {
	if (show_title) {
		displevel(list->pl_persona->p_level,
		  list->pl_persona->p_sex & pfl(Female),
		  list->pl_persona->p_name);
		bprintf("\n");
	} else {
		bprintf("%-19.19s", list->pl_persona->p_name);
		if (++cnt >= 4) {
			bprintf("\n");
			cnt = 0;
		}
	}
	cur = list;
	list = list->pl_next;
	free(cur->pl_persona);
	free(cur);
  }
  if (cnt) {
	bprintf("\n");
  }
}

void
wizlistcom ()
{
  PLIST *plists[PL_TOTAL];

  findwizards(plists);
  bprintf ("\nThe following people have risen to power in the Underground:\n");
  diswizards("\nA[1mGodsA[0m\n====\n", plists[PL_GOD], True);
  diswizards("\nA[1mArch-WizardsA[0m\n============\n",
	   plists[PL_ARCHWIZARD], True);
  diswizards("\nA[1mWizardsA[0m\n=======\n", plists[PL_WIZARD],
	False);
  diswizards("\nA[1mApprentice WizardsA[0m\n==================\n",
	plists[PL_APPRWIZARD], False);
  bprintf("\n");
}


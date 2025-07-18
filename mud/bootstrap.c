/*
**  Read the data files.
*/

#include <strings.h>
#include "kernel.h"
#include "blib.h"
#include "bootstrap.h"
#include "macros.h"
#include "aflags.h"

static char* get_string (FILE *file);
static int boot_messages (FILE *a);
static int boot_levels (FILE *a);
static int boot_objects (FILE *a);
static int boot_mobiles (FILE *a);
static int boot_zones (FILE *a);
static int boot_verbs (FILE *a);
static int boot_actions (FILE *a);

int
bootstrap()
{
  FILE *a, *b;
  int mem_used;
  char *y, x[128];

  if ((a = fopen(BOOTSTRAP, "r")) == NULL)
    {
      fprintf(stderr, "Bootstrap failed\n");
      exit(1);
    }
  while (fgets(x, sizeof x, a))
    {
      if (y = index(x, '\n'))
	*y = '\0';
      if (isawiz)
	{
	  bprintf("Bootstrap... %s ", x);
	  bflush();
	}
      if ((y = index(x, ':')) == NULL)
	{
	  fprintf(stderr, "Invalid bootstrap declaration\n");
	  exit(1);
	}
      y++;
      if ((b = fopen(y, "r")) == NULL)
	{
	  bprintf("Can't open %c file %s.\n", x[0], y);
	  exit(1);
	}
      switch (x[0]) {
      case 'C':
	mem_used = boot_mobiles(b);
	break;
      case 'M':
	mem_used = boot_messages(b);
	break;
      case 'O':
	mem_used = boot_objects(b);
	break;
      case 'V':
	mem_used = boot_verbs(b);
	break;
      case 'Z':
	mem_used = boot_zones(b);
	break;
      case 'E':
	mem_used = boot_levels(b);
	break;
      case 'A':
	mem_used = boot_actions(b);
	break;
      }
      fclose(b);
      if (isawiz)
	bprintf("used %d bytes\n", mem_used);
    }
  fclose(a);
}

static char *
get_string(FILE *file)
{
  char *y;
  char x[1024];

  for (y = x; (*y = fgetc(file)) != '^'; y++)
    ;
  *y = 0;
  fgetc(file);
  return COPY(x);
}

static int
boot_messages(FILE *a)
{
  int v, mem_used, ct;
  char x[32];

  fgets(x, sizeof x, a);
  if (!sscanf(x, "%d", &v)) {
    fprintf(stderr, "Illegal message number\n");
    exit(1);
  }
  messages = NEW(char*, v);
  mem_used = v * sizeof (char *);
  for (ct = 0; ct < v; ct++) {
    messages[ct] = get_string(a);
    mem_used += strlen(messages[ct]) + 1;
  }
  return mem_used;
}

static int
boot_levels(FILE *a)
{
  int i, v, ct;

  fscanf(a, "%d", &ct);
  fgetc(a);
  for (i = 1; i <= ct; i++)
    {
      fscanf(a, "%d", &v);
      fgetc(a);
      levels[i] = v;
    }
  return sizeof(levels);
}

static int
boot_objects(FILE *a)
{
  int  j, s1, v, ct, mem_used;
  char x[32], y[32];

  fscanf(a, "%d", &v);
  fgetc(a);
  objects = NEW(OBJECT, v);
  objinfo = NEW(int, o_ofs(v));
  mem_used = sizeof (OBJECT) * v;
  mem_used += sizeof (int) * o_ofs(v);
  numobs = v;
  for (ct = 0; ct < v; ct++) {
    fscanf(a, "%s %s %d %d %d %d",
	   x, y,
	   &objects[ct].o_maxstate, &objects[ct].o_value,
	   &objects[ct].o_flannel, &s1);
    objects[ct].o_name = COPY(x);
    objects[ct].o_altname = COPY(y);
    objects[ct].o_size = s1;
    mem_used += strlen(x) + strlen(y);
    fgetc(a);

    objects[ct].o_desc[0] = get_string(a);
    mem_used += strlen(objects[ct].o_desc[0]);

    objects[ct].o_desc[1] = get_string(a);
    mem_used += strlen(objects[ct].o_desc[1]);

    objects[ct].o_desc[2] = get_string(a);
    mem_used += strlen(objects[ct].o_desc[2]);

    objects[ct].o_desc[3] = get_string(a);
    mem_used += strlen(objects[ct].o_desc[3]);

    objects[ct].o_examine = ftell(a);
    for (j = 1; fgetc(a) != '^'; j++)
      ;
    if (j == 1)
      objects[ct].o_examine = 0;
  }
  return mem_used;
}

static int
boot_mobiles(FILE *a)
{
  int v, ct, mem_used;
  char x[64];

  fgets(x, sizeof x, a);
  sscanf(x, "%d", &v);
  pinit = NEW(PLAYER, v);
  mem_used = sizeof (PLAYER) * v;
  pftxt = NEW(char*, v);
  mem_used += sizeof (char *) * v;
  for (ct = 0; ct < v; ct++)
    {
      pinit[ct].p_name = get_string(a);
      fscanf(a, "%d %d %o %d",
	     &pinit[ct].p_loc, &pinit[ct].p_str,
	     &pinit[ct].p_flags, &pinit[ct].p_lev);
      mem_used += strlen(pinit[ct].p_name) + 1;
      fgetc(a);
      pftxt[ct] = get_string(a);
      mem_used += strlen(pftxt[ct]) + 1;
    }
  numchars = v + MAX_USERS;
  return mem_used;
}

static int
boot_zones(FILE *a)
{
  int ct, mem_used;
  char x[64];

  fgets(x, sizeof x, a);
  sscanf(x, "%d", &numzon);
  zoname = NEW(ZONE, numzon);
  mem_used = sizeof (ZONE) * numzon;
  for (ct = 0; ct < numzon; ct++)
    {
      fscanf(a, "%s %d", x, &zoname[ct].z_loc);
      zoname[ct].z_name = COPY(x);
      mem_used += strlen(x) + 1;
    }
  return mem_used;
}

static int
boot_verbs(FILE *a)
{
  int v, ct, mem_used;
  char x[64];

  fscanf(a, "%d", &v);
  mem_used = sizeof (char *) * (v + 1);
  mem_used += sizeof (int) * (v + 1);
  verbtxt = NEW(char*, v + 1);
  verbnum = NEW(int, v + 1);
  for (ct = 0; ct < v; ct++)
    {
      fscanf(a, "%s %d", x, &verbnum[ct]);
      verbtxt[ct] = COPY(x);
      mem_used += strlen(x) + 1;
    }
  verbtxt[ct] = NULL;
  return mem_used;
}

static int
boot_actions (FILE *a)
{
  int v, ct = 0;
  int mem_used, scanreturn = 0;
  char x[81], verb[81], t[6];

  fscanf(a, "%d", &v);		/* How many actions are there? */
  mem_used = sizeof (char *) * (v + 1) * 7;
  mem_used += sizeof (int) * (v + 1);
  numactions = v;

  action = NEW(char*, v + 1);		/* Allocate space for pointers. */
  action_all = NEW(char*, v + 1);
  action_all_rsp = NEW(char*, v + 1);
  action_to = NEW(char*, v + 1);
  action_to_rsp = NEW(char*, v + 1);
  action_msg_all = NEW(char*, v + 1);
  action_msg_to = NEW(char*, v + 1);
  action_flags = NEW(int, v + 1);

  /* Scan action table */
  do {
    if ((fgets(x, 80, a) == NULL))
      {
	scanreturn = EOF;
	break;
      }
    scanreturn = sscanf(x, ":%s %s", verb, t);
    if (scanreturn == 2)
      {
	action[ct] = COPY(verb);
	mem_used += strlen(verb) + 1;

	action_flags[ct] = 0;
	if (index(t, 'h'))
	  asetflg(ct, ACT_HOSTILE);
	if (index(t, 's'))
	  asetflg(ct, ACT_TARGET);
	if (index(t, 'a'))
	  asetflg(ct, ACT_ALL);
	if (index(t, 'm'))
	  asetflg(ct, ACT_MESSAGE);

	fgets(x, sizeof x, a);	/* read messages */
	*index(x, '\n') = 0;
	action_all[ct] = COPY(x);
	mem_used += strlen(x) + 1;

	fgets(x, sizeof x, a);
	*index(x, '\n') = 0;
	action_all_rsp[ct] = COPY(x);
	mem_used += strlen(x) + 1;

	fgets(x, sizeof x, a);
	*index(x, '\n') = 0;
	action_to[ct] = COPY(x);
	mem_used += strlen(x) + 1;

	fgets(x, sizeof x, a);
	*index(x, '\n') = 0;
	action_to_rsp[ct] = COPY(x);
	mem_used += strlen(x) + 1;

	fgets(x, sizeof x, a);
	*index(x, '\n') = 0;
	action_msg_all[ct] = COPY(x);
	mem_used += strlen(x) + 1;

	fgets(x, sizeof x, a);
	*index(x, '\n') = 0;
	action_msg_to[ct] = COPY(x);
	mem_used += strlen(x) + 1;

	ct++;
      }
  } while (1);
  action[ct] = NULL;
  return mem_used;
}

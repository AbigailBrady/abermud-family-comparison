/*
**  Utility program to build the data files.
*/

#include <strings.h>
#include "kernel.h"
#include "blib.h"

int line = 0;

void crapup(char *p)
{
  perror(p);
  exit(1);
}

/*
**  Parse a flags line, a line like:
**	int1:int2:bits
**  and turn it into a four-byte int like:
**	bits:int1:int2
*/
static int *flags(char *buff)
{
  int i[2];
  char *p;
  char c, save[120];

  strcpy(save, buff);
  if (p = index(buff, ':')) {
    *p++ = '\0';
    i[0] = atoi(buff) << 8;
    buff = p;
    if (p = index(buff, ':')) {
      *p++ = '\0';
      i[0] |= atoi(buff);
      for (i[1] = 0; (c = *p) && (c == '0' || c == '1'); p++)
	  i[1] = (i[1] << 1) | (c == '1' ? 1 : 0);
      return i;
    }
  }
  fprintf(stderr, "Badly-formed input, line %d:\n%s\n", line, save);
  exit(1);
}

/*
**  Open file for read/write or die trying
*/
static FILE *Do_fopen(char *name, char *mode)
{
  FILE *file;

  if (*name == '-' && !name[1])
    return *mode == 'w' ? stdout : stdin;
  if (!(file = fopen(name,mode))) {
    perror(name);
    (void)fprintf(stderr, "Unable to open file for %s\n",
		  *mode == 'w' ? "write" : "read");
    exit(1);
  }
  return file;
}

int fndzone(int x, char **str, ZONE *ztable)
{
        char   *s;
	int	a, b;
	int	c;

	x = -x;
	if (x <= 0) {
	        *str = "TCHAN";
		return 0;
	}
	for (a = 0, b = 0; a < x; b++) {
		c = a;
	        s = ztable[b].z_name;
		a = ztable[b].z_loc;
	}
	*str = s;
	return x - c;
}

/*
**  Make header file locations.h
*/
static void
make_locations(argc, argv)
int	argc;
char	*argv[];
{
    /* generate locations zones locations locations.h */

	register FILE  *In, *Out, *Zone;
	int	i, j, v, z, numz, rcode;
	ZONE	*zon_table;
	char	*znames, *zp;
	char	name[40], buff[128];

	if (argc < 4) {
		(void)fprintf(stderr, "Usage error: See Makefile\n");
		exit(1);
	}
/* First initialize zones table */

	Zone = Do_fopen(argv[1], "r");
	(void)fgets(buff, sizeof(buff), Zone);
	sscanf(buff, "%d", &numz);
	zon_table = NEW(ZONE,numz);
	zp = znames = NEW(char,numz*sizeof name);

	for (z = 0; z < numz; z++) {
	    (void)fgets(buff, sizeof(buff), Zone);
	    (void)sscanf(buff, "%s%d", name, &i);
	    zon_table[z].z_name = strcpy(zp,name);
	    zon_table[z].z_loc = i;
	    zp += strlen(zp) + 1;
	}
	fclose(Zone);

	In = Do_fopen(argv[2], "r");
	Out = Do_fopen(argv[3], "w");
	(void)fgets(buff, sizeof(buff), In);
	sscanf(buff, "%d", &v);

	/* Print out header */
	(void)fprintf(Out, "\
/*\n\
**\tLocations file header generated from zone file %s\n\
**\t and location file %s\n\
**\tDON'T MAKE CHANGES HERE -- THEY WILL GO AWAY!\n\
*/\n\n",
     	     	     	     	     	     	     argv[1], argv[2]);

/* Read thru locations file, creating #define's for each location */

	for (i = 0; i < v; i++) {
		fscanf(In, "%d %*d %*d %*d %*d %*d %*d", &rcode );
		fgetc(In);
		fscanf(In, "%s", buff);
		fgetc(In);
		while (fgetc(In) != '^');	/* Skip short description */
		while (fgetc(In) != '^');	/* Skip long description */
		fgetc(In);
/*
** Now we have read a room description and got a room code. Now get zone
** name and number to put after zone name.
**
*/
		j = fndzone(-rcode,&zp,zon_table);
		(void)uppercase(strcpy(name,zp));
		(void)fprintf(Out,"#define RM_%s%d\t%d\n", name, j, -rcode);
	}

	(void)fclose(In);
	(void)fclose(Out);
	exit(0);
}

/*
**  Create world file.
*/
static void make_world(int argc, char *argv[])
{
  FILE *F;
  int b, x[64];

  F = Do_fopen(argc == 2 ? argv[1] : UNIVERSE,"w");
  x[0] = 1;
  x[1] = 1;
  sec_write(F, x, 0, sizeof x / sizeof x[0]);
  for (b = 0; b < sizeof x / sizeof x[0]; )
    x[b++] = 0;
  for (b = 1; b < 600 - 149 - 9; b++, x[0] = 0)
    sec_write(F, x, b, sizeof x / sizeof x[0]);
  (void)fclose(F);
  exit(0);
}

/*
**  Create the user activity file.
*/
static void make_uaf(int argc, char *argv[])
{
  FILE *F;
  PERSONA P;

  F = Do_fopen(argc == 2 ? argv[1]: UAF_RAND,"w");

  P.p_sex = 0;
  P.p_level = 10001;
  P.p_score = 150000;
  P.p_strength = 58040;
  (void)strcpy(P.p_name, "Root");
  (void)fwrite((char *)&P, 1, sizeof P, F);

  exit(0);
}

/*
**  Make the data file that shows what starts where, and their states.
*/
static void make_reset(int argc, char *argv[])
{
    FILE	*In, *Out;
    int	*ip, *p;
    int	i, blob[10000];
    char	buff[128], *cp;

    line = 1;
    if (argc < 3) {
	(void)fprintf(stderr, "Usage error\n");
	exit(1);
    }
    In = Do_fopen(argv[1], "r");
    Out = Do_fopen(argv[2], "w");

    /* Read thru object file, ignoring nonnumeric lines and collecting */
    /* others into blob[] for write to output file... */
    for (ip = blob, i = 0; fgets(buff, sizeof(buff), In); line++) {
	for (cp = buff; isspace(*cp); )
	    ++cp;
	if (*cp == '-' || isdigit(*cp)) {
	    if ((i & 3) == 2) {
		p = flags(cp);
		*ip++ = *p++;
		*ip++ = *p;
	    } else {
		*ip++ = atoi(cp);
	    }
	    ++i;
	}
    }

    sec_write(Out, blob, 0, ip - blob);
    (void)fclose(In);
    (void)fclose(Out);
    exit(0);
}

/*
**  Make the user file.
*/
static void make_user(int argc, char *argv[])
{
  (void)fclose(Do_fopen(argc == 2 ? argv[1] : PASSWORDFILE, "w"));
  exit(0);
}

/*
**	Search for word in internal storage, return zero if was already
**	there; else, return true and add to to the storage.
*/
int used(char *word)
{
  int p;
  /* Maximum number of objects is 1000.  Feel free to make it larger. */
  static char *storage[1000];
  static int  xused[1000];
  static int  store_index;

  for (p = 0; p < store_index; p++)
    if (EQ(storage[p], word))
      return(++xused[p]);
  storage[p] = (char *)malloc(35);	/* 35 bytes for object name */
  (void)strcpy(storage[p], word);
  store_index++;
  return(0);
}

/*
**  Make header file objects.h
*/
static void make_objects(int argc, char *argv[])
{
  FILE	*In, *Out;
  char	*p, *q;
  int	 i, j, nr;
  char	 buff[128], objname[2][30];

  if (argc < 3) {
    (void)fprintf(stderr, "Usage error\n");
    exit(1);
  }
  In = Do_fopen(argv[1], "r");
  Out = Do_fopen(argv[2], "w");

  /* Print out header */
  (void)fprintf(Out, "\
/*\n\
**\tObject file header generated from %s\n\
**\tDON'T MAKE CHANGES HERE -- THEY WILL GO AWAY!\n\
*/\n\n", argv[1]);

  (void)fgets(buff, sizeof(buff), In);

  /* Read thru object file, creating #define:s for each object */
  for (i = 0; fgets(buff, sizeof(buff), In); i++) {
    if (sscanf(buff, "%s%s%d", objname[0], objname[1], &j) != 3) {
      (void)fprintf(stderr, "Badly-formed input line:\n%s\n", buff);
      exit(1);
      /* NOTREACHED */
    }

    for (j = 0; j < 2; j++)	{
      if (!*(p = uppercase(objname[j])))
	continue;
      for (q = p; *q; q++)
	if (!isalpha(*q) && *q != '$' && *q != '_') {
	  *p='\0';
	  continue;
	}
    }
    for (j=0; *objname[j] && j < 2; j++) {
      *buff='\0';
      (void)strcpy(buff, objname[j]);
      if (j == 1) {
	(void)strcat(buff, "_");
	(void)strcat(buff, objname[0]);
      }
      if (!(nr = used(buff))) {
	(void)fprintf(Out, "#define\tOBJ_%s\t%d\n", buff, i);
	break;
      } else {
	(void)fprintf(Out, "#define\tOBJ_%s_%d\t%d\n", buff, nr, i);
	break;
      }
    }
    for (j = 0; j < 5; j++)	/* skip past object descriptions */
      while (fgetc(In) != '^')
	;
    while (fgetc(In) != '\n')
      ;
  }
  (void)fclose(In);
  (void)fclose(Out);
  exit(0);
}

/*
**  Make header file verbs.h
*/
static void make_verbs(int argc, char *argv[])
{
  FILE	*In, *Out;
  char	*p, *q;
  int	j;
  char	buff[128], verb[30];

  if (argc < 3) {
    (void)fprintf(stderr, "Usage error: See Makefile\n");
    exit(1);
  }
  In = Do_fopen(argv[1], "r");
  Out = Do_fopen(argv[2], "w");

  /* Print out header */
  (void)fprintf(Out,"\
/*\n\
**\tVerb file header generated from %s\n\
**\tDON'T MAKE CHANGES HERE -- THEY WILL GO AWAY!\n\
*/\n\n", argv[1]);
  (void)fgets(buff, sizeof(buff), In);

  /* Read thru verb file, creating #define's for each verb */

  while (fgets(buff, sizeof(buff), In)) {
    if (sscanf(buff, "%s%d", verb, &j) != 2)	{
      (void)fprintf(stderr, "Badly-formed input line:\n%s\n", buff);
      exit(1);
      /* NOTREACHED */
    }
    if (!*(p = uppercase(verb)))
      continue;
    for (q = p; *q; q++)
      if (!isalpha(*q) && *q!='$' && *q!='_')
	continue;
    (void)fprintf(Out, "#define\tVERB_%s\t%d\n", p, j);
  }
  (void)fclose(In);
  (void)fclose(Out);
  exit(0);
}

/*
**  Make header file mobiles.h
*/
static void make_mobiles(int argc, char *argv[])
{
  FILE	*In, *Out;
  int	i, j, v;
  char	name[40], buff[128];

  if (argc < 3) {
    (void)fprintf(stderr, "Usage error: See Makefile\n");
    exit(1);
  }
  In = Do_fopen(argv[1], "r");
  Out = Do_fopen(argv[2], "w");

  /* Print out header */
  (void)fprintf(Out,"\
/*\n\
**\tMobiles file header generated from %s\n\
**\tDON'T MAKE CHANGES HERE -- THEY WILL GO AWAY!\n\
*/\n\n", argv[1]);
  (void)fgets(buff, sizeof(buff), In);
  sscanf(buff, "%d", &v);

  /* Read thru mobiles file, creating #define's for each mobile */
  for (i = 0; i < v; i++) {
    (void)fgets(name, sizeof(name), In);
    name[strlen(name) - 2] = '\0';
    uppercase(name);
    if (strncmp(name, "THE ", 4) == 0)
      sprintf(name, name + 4);
    (void)fprintf(Out, "#define\tMOB_%s\t%d\n", name, i + MAX_USERS);
    (void)fgets(buff, sizeof(buff), In);
    for (j = 1; (fgetc(In)) != '^'; j++)
      ;
    fgetc(In);
  }
  (void)fclose(In);
  (void)fclose(Out);
  exit(0);
}

main(int argc, char *argv[])
{
  argc--;
  argv++;

  if (argc)
    if (EQ(*argv, "world"))
      make_world(argc, argv);
    else if (EQ(*argv, "uaf"))
      make_uaf(argc, argv);
    else if (EQ(*argv, "reset"))
      make_reset(argc, argv);
    else if (EQ(*argv, "userfile"))
      make_user(argc, argv);
    else if (EQ(*argv, "objects"))
      make_objects(argc, argv);
    else if (EQ(*argv, "verbs"))
      make_verbs(argc, argv);
    else if (EQ(*argv, "mobiles"))
      make_mobiles(argc, argv);
    else if (EQ(*argv, "locations"))
      make_locations(argc,argv);

  (void)fprintf(stderr, "Usage error: See Makefile\n");
  exit(1);
}

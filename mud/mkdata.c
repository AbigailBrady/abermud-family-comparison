/*
 *  Alf Salte  (Alf)  June 1990
 */

#include <stdio.h>
#include <ctype.h>

#include "kernel.h"

#ifdef VARGS
#include <stdarg.h>
#endif

#define get_oref(z,F) ((XOBJ *)get_ref('O',z,F))
#define get_lref(z,F) get_ref('L', z, F)
#define get_mref(z,F) ((XMOB *)get_ref('M', z, F))

#include "cflags.h"

#include "mflagnames.h"
#include "pflagnames.h"
#include "sflagnames.h"
#include "lflagnames.h"
#include "oflagnames.h"
#include "exitnames.h"
#include "lflags.h"

#define UNKNOWN         ((XOBJ *)(-1))
#define BAD             ((XOBJ *)(-2))
#define GOOD            ((XOBJ *)(-3))
#define REFERRED        ((XOBJ *)(-4))
#define DEFINED         ((XOBJ *)(-5))

#define IS_LINK         0
#define IS_CONTAINED    1
#define IS_CHECK        2

typedef struct _XZON {
    char                *name;
    char                *fname;
    int                 zone;   /* This zone's number */
    int                 loc;    /* The loc number of room ZONE0 */
    int                 n_loc;  /* Number of locations in this zone */
    int                 mob;    /* The mob number of first mobile in zone */
    int                 n_mob;  /* Number of mobiles in this zone */
    struct _XLOC        *locs;
    struct _XOBJ        *objs;
    struct _XMOB        *mobs;
    struct _XLOC        *rlocs;
    struct _XMOB        *rmobs;
} XZON;

typedef struct _XMOB {
    struct _XOBJ        *aux;
    struct _XZON        *zone;
    struct _XMOB        *next;
    char                *name;
    char                *pname;
    int                 mob;
    struct _XLOC        *loc;
    int                 str;
    int                 armor;
    int                 damage;
    int			agg;
    int                 speed;
    int                 vis;
    int                 wimpy;
    PFLAGS              pflags;
    SFLAGS              sflags;
    MFLAGS              mflags;
    int                 attitude;
    int                 att_param;
    char                *desc;
    char                *exam;
} XMOB;

typedef struct _XOBJ {
    struct _XOBJ        *aux;
    struct _XZON        *zone;
    struct _XOBJ        *next;
    char                *name;
    struct _XOBJ        *the_next;
    struct _XOBJ        *linked;
    char                *pname;
    char                *aname;
    int                 obj;
    int                 cflag;
    struct _XLOC        *loc;
    unsigned char       damage;
    unsigned char       armor;
    OFLAGS              oflags;
    int                 state;
    int                 mstate;
    int                 bvalue;
    int                 osize;
    int                 oweight;
    char                *desc[4];
    char                *examine;
    int                 vis;
} XOBJ;

typedef struct _XLOC {
    struct _XOBJ        *aux;
    struct _XZON        *zone;
    struct _XLOC        *next;
    char                *name;
    char                *pname;
    int                 loc;
    LFLAGS              lflags;
    char                exit_types[NEXITS];
    struct _XLOC        *exits[NEXITS];
    char                *description;
} XLOC;

char *strchr();

char    *main_file;
char    *log_file;
char    *zone_file;
char    *mobh_file;
char    *mob_file;
char    *objh_file;
char    *obj_file;
char    *reset_list;
char    *loch_file;
char    *loc_file;
FILE    *LogFile;
int     n_logs;

char    dir[512];
XZON    zones[400];
XMOB    mobs[500];
XOBJ    objs[2000];
XLOC    locs[5000];
char    texts[350000];

char    *textsp = texts;
int     num_z = 0;
int     num_m = 0;
int     num_o = 0;
int     num_l = 0;
int     num_linked = 0;
int     num_unlinked = 0;

/*
XOBJ    *not_linked_l = (XOBJ *)NULL;
XOBJ    *linked_objs_l = (XOBJ *)NULL;
XOBJ    *not_linked = (XOBJ *)NULL;
XOBJ    *linked_objs = (XOBJ *)NULL;
*/
XOBJ    *obj_list = NULL;
XOBJ    *obj_list_l = NULL; /* last */

#define T_NAME 0
#define T_PNAME 1
#define T_LOC 2
#define T_DESC 3
#define T_END 4

#define TMOB_STR 5
#define TMOB_DAM 6
#define TMOB_ARMOR 7
#define TMOB_AGG 8
#define TMOB_SFLAGS 9
#define TMOB_PFLAGS 10
#define TMOB_MFLAGS 11
#define TMOB_SPEED 12
#define TMOB_EXAM  13
#define TMOB_VIS  14
#define TMOB_WIMPY 15

#define TOBJ_ANAME 5
#define TOBJ_OFLAGS 6
#define TOBJ_ARMOR 7
#define TOBJ_DAMAGE 8
#define TOBJ_MAX_STATE 9
#define TOBJ_STATE 10
#define TOBJ_BVALUE 11
#define TOBJ_SIZE 12
#define TOBJ_WEIGHT 13
#define TOBJ_EXAM 14
#define TOBJ_LINKED 15
#define TOBJ_VIS 16

char *Mob_tab[] = {
    "Name",     "Pname",        "Location",     "Description",  "End",
    "Strength", "Damage",	"Armor",        "Aggression",   "SFlags",
    "PFlags",   "MFlags",       "Speed",        "Examine",      "Visibility",
    "Wimpy",    TABLE_END
};

char *Obj_tab[] = {
    "Name",     "Pname",        "Location",     "Description", "End",
    "AltName",  "Oflags",       "Armor",        "Damage",
    "MaxState", "State",        "BValue",       "Size",
    "Weight",   "Examine",      "Linked",       "Visibility",
    TABLE_END
};


char *Cflags[] = {
    "In room", "In container", "Carried by", "Worn by",
    "Wielded by", "Both worn and wielded by"
};

void xexit(code)
    int code;
{
    printf("\n\n");
    exit(code);
}

#ifdef VARGS
void log(char t,XOBJ *O,XZON *Z,char *f, ...);
#endif


/*
**  Open file for read/write or die trying
*/
static FILE    *
Do_fopen(char *name, char *mode)
{
    char           *m;
    FILE           *file;
    Boolean            b;

    b = False;
    if (*(m = mode) == 'p') {
        b = True;
        ++m;
    }
    if (*name == '-' && !name[1])
        return *mode == 'w' ? stdout : stdin;
    file = (b ? popen(name, m) : fopen(name, m));
    if (!file) {
        perror(name);
        (void) fprintf(stderr, "Unable to open file for %s(%s)\n",
           (*m == 'w' ? "write" : "read"), mode);
        xexit(1);
    }
    return file;
}

char *xfgets(char *b,int s, FILE *f)
{
    int k;
    int c;
    char *t;

    for (t = b, k = s; --k >= 0 && (c = fgetc(f)) != EOF && c != '\n';)
        *t++ = c;
    if (c == EOF && t == b)
        return NULL;
    *t = 0;
    return b;
}

char *alloc_text(int s)
{
    char *p;

    textsp = (p = textsp) + s;
    return p;
}

char *save_text(char *t)
{
    int s;
    char *u;

    s = strlen(t) + 1;
    u = alloc_text(s);
    bcopy(t,u,s);
    return u;
}

char *get_fname(FILE *a,char *d)
{
  char *p;
  register char *q;
  int   c, k;
    
  while ((c = getc(a)) == ' ' || c == '\t');
  q = (p = textsp);
  if (c != '/') {
    k = strlen(d);
    bcopy(d,q,k);
    q += k;
  }
  do {
    *q++ = c;
    c = getc(a);
  } while (!isspace(c));
  while (c != '\n') c = getc(a);
  *q++ = 0;
  textsp = q;
  return p;
}

char *get_text(FILE *a)
{
    char *p;
    register char *q;
    int   c, d;

    q = (p = textsp);
    while ((c = getc(a)) == ' ' || c == '\t');
    if (ispunct(c)) {
        d = c;
    } else {
        d = 0;
        ungetc(c,a);
    }
    if ((c = getc(a)) != '\n')
        ungetc(c,a);
    while((c = getc(a)) != d) {
        if (d == 0 && isspace(c))
            break;
        *q++ = c;
    }
    if (d == 0)
        ungetc(c,a);
    *q++ = 0;
    textsp = q;
    return p;
}

char *get_exam(FILE *a)
{
    char *p;
    register char *q;
    int   c, d;

    q = (p = textsp);
    c = getc(a);
    while (isspace(c)) c = getc(a);
    d = c;
    if ((c = getc(a)) != '\n') ungetc(c,a);
    while((c = getc(a)) != d && c != EOF) *q++ = c;
    if (q[-1] != '\n') *q++ = '\n';
    *q++ = 0;
    textsp = q;
    return p;
}

char *get_description(FILE *a)
{
    char *p;
    register char *q;
    int   c, d;

    q = (p = textsp);
    while (True) {
        while ((c = getc(a)) != '^') {
            do {
                *q++ = c;
            } while ((c = getc(a)) != '\n');
            *q++ = '\n';
        }
        if ((c = getc(a)) == '\n')
            break;
        *q++ = '^';
        *q++ = c;
        while ((*q++ = c = getc(a)) != '\n');
    }
    *q++ = 0;
    textsp = q;
    return p;
}

FILE *xopen(char *f,char *m)
{
    FILE *x;

    if ((x = fopen(f,m)) == 0) {
        perror(f);
        xexit(1);
    }
    return x;
}


int read_main(char *mf)
{
    char b[128];
    char n[128];
    FILE *f;
    XZON *p;
    int c;
    int z;
    int k;
    char *s;
    char *t;
    char *u;

    f = xopen(mf,"r");
    main_file = mf;
    for (s = dir, t = main_file, u = NULL; *t != 0;++s, ++t) {
      if ((*s = *t) == '/')
	u = s;
    }
    if (u == NULL)
      *(u = s = dir) = 0;
    else
      *++u = 0;
    log_file = get_fname(f,dir);
    zone_file = get_fname(f,dir);
    mobh_file = get_fname(f,dir);
    mob_file = get_fname(f,dir);
    objh_file = get_fname(f,dir);
    obj_file = get_fname(f,dir);
    reset_list = get_fname(f,dir);
    loch_file = get_fname(f,dir);
    loc_file = get_fname(f,dir);
    if ((LogFile = fopen(log_file,"w")) == NULL) {
        perror(log_file);
        exit(1);
    }
    n_logs = 0;

    for (z = 0, p = zones;xfgets(b,sizeof(b),f) != NULL;) {
        for (s = b; isalpha(*s); ++s);
        if ((k = s - b) == 0)
            continue;
        t = alloc_text(k + 1);
        bcopy(b,t,k);
        t[k] = 0;
        p->name = t;
        while (*s == ' ' || *s == '\t') ++s;
        if (isalpha(*s) || *s == '/' || *s == '.') {
            for (t = s; isalpha(*s) || *s == '/' || *s == '.'; ++s);
            *s = 0;
	    if (*t != '/') {
	        sprintf(n, "%s%s", dir, t);
		t = n;
	    }
        } else {
            sprintf(n,"%s%s.zone", dir, t);
            t = n;
        }
        k = strlen(t);
        s = alloc_text(k+1);
        bcopy(t,s,k);
        s[k] = 0;
        p->fname = s;
        p->loc = 0;
        p->locs = (XLOC *)0;
        p->mobs = (XMOB *)0;
        p->objs = (XOBJ *)0;
	p->zone = z;
        ++z;
        ++p;
    }
    fclose(f);
    return num_z = z;
}


FILE *zopen(char *fn, char *zn, Boolean *m, char *b, int bs)
{
    FILE *f;
    char *s;
    char *t;
    char buff[128];

    sprintf( buff, "/lib/cpp -P -R -I../include %s", fn);
    f = popen(buff, "r");
    if (f == NULL) {
        fprintf(stderr, "\nError when opening file %s for zone %s\n",
                fn, zn);
        perror(fn);
        xexit(1);
    }
    *m = False;
    do {
        if (xfgets(b,bs,f) == NULL) {
            pclose(f);
            return NULL;
        }
    } while (b[0] != '%');
    if (strncasecmp(&b[1],"zone:",5) == 0) {
        *m = True;
        for (;;) {
            for (s = &b[6]; *s == ' ' || *s == '\t'; ++s);
            for (t = s; isalpha(*t); ++t);
            *t = 0;
            if (strcasecmp(s,zn) == 0) {
                while (xfgets(b,bs,f)) {
                    if (b[0] == '%')
                        return f;
                }
                pclose(f);
                return NULL;
            }
            do {
                if (xfgets(b,bs,f) == NULL) {
                    pclose(f);
                    return NULL;
                }
            } while (strncasecmp(b,"%zone:",6) != 0);
        }
    }
    return f;
}

int lookup(char *s, char **t)
{
    char **u;
    int l;
    int x;

    l = strlen(s);
    for (u = t, x = 0; *u != TABLE_END; u++, x++) {
        if (*u == NULL)
            continue;
        if (strncasecmp(s,*u,l) == 0)
            return x;
    }
    return -1;
}

XMOB *find_mob(XZON *z, char *name)
{
    XMOB *m, *q = NULL;
    char *s;

    for (m = z->mobs; m != NULL; m = m->next) {
        if (strcasecmp(m->name,name) == 0)
            return m;
    }
    for (m = z->rmobs; m != NULL; m = m->next) {
        if (strcasecmp(m->name,name) == 0)
            return m;
        q = m;
    }
    m = &mobs[num_m++];
    bzero((char *)m,sizeof(XMOB));
    m->next = NULL;
    if (q == NULL)
        z->rmobs = m;
    else
        q->next = m;

    m->name = s = save_text(name);
    lowercase(s);
    *s = toupper(*s);
    if (strncmp(m->name,"The ", 4) == 0) {
        s += 4;
        *s = toupper(*s);
    }
    m->zone = z;
    m->mob = -1;
    m->aux = REFERRED;
    return m;
}

XOBJ *find_obj(XZON *z, char *name)
{
    XOBJ *o, *q = NULL;
    char *s;

    for (o = z->objs; o != NULL; o = o->next) {
        if (strcasecmp(o->name,name) == 0)
            return o;
        q = o;
    }
    o = &objs[num_o++];
    bzero((char *)o,sizeof(XOBJ));
    o->next = NULL;
    if (q == NULL)
        z->objs = o;
    else
        q->next = o;

    o->name = s = save_text(name);
    lowercase(s);
    o->zone = z;
    o->obj  = -1;
    o->aux = REFERRED;
    return o;
}

XLOC *find_loc(XZON *z, char *name)
{
    XLOC *l, *q = NULL;

    for (l = z->locs; l != NULL; l = l->next) {
        if (strcasecmp(l->name,name) == 0)
            return l;
    }
    for (l = z->rlocs; l != NULL; l = l->next) {
        if (strcasecmp(l->name,name) == 0)
            return l;
        q = l;
    }
    l = &locs[num_l++];
    bzero((char *)l,sizeof(XLOC));
    l->next = NULL;
    if (q == NULL)
        z->rlocs = l;
    else
        q->next = l;

    l->name = save_text(name);
    l->zone = z;
    l->loc = -1;
    l->aux = REFERRED;
    return l;
}

XLOC *get_ref(char t, XZON *zo, FILE *F)
{
    char n[128];
    char zone[128];
    char *s;
    XZON *z;
    int c;
    int x;

    for (s = n, c = getc(F); isalnum(c) || c == '_'; c = getc(F)) {
        *s++ = c;
    }
    *s = 0;
    z = zo;
    if (c == '@') {
        for (s = zone, c = getc(F); isalpha(c); c = getc(F)) {
            *s++ = c;
        }
        *s = 0;
        for (z = zones, x = 0; x < num_z; ++z, ++x) {
            if (strcasecmp(z->name,zone) == 0)
                break;
        }
        if (x == num_z) {
            ungetc(c,F);
            return 0;   /* Couldn't find entry */
        }
    }
    ungetc(c,F);

    switch(t) {
    case 'L': return (XLOC *)find_loc(z,n); break;
    case 'O': return (XLOC *)find_obj(z,n); break;
    case 'M': return (XLOC *)find_mob(z,n); break;
    }
    return 0;
}

int get_int(FILE *F)
{
    int i;
    int c;
    Boolean neg;

    c = getc(F);
    while (c == ' ' || c == '\t')
        c = getc(F);
    i = 0;
    if (neg = c == '-') {
        c = getc(F);
    }
    while (isdigit(c)) {
        i = i * 10 + (c - '0');
        c = getc(F);
    }
    ungetc(c,F);
    if (neg)
        i = -i;
    return i;
}

void get_flags(XZON *z, XMOB *m, char ty, int *f, int s, char **t, FILE *F)
{
    char n[128];
    int c;
    int light = -1;
    int temp = -1;
    char *p;
    int k;
    int bno;
    int cno;

    c = getc(F);
    while (c != '{')
        c = getc(F);
    bzero(f,s);

    for (c = getc(F);c != '}';c = getc(F)) {
        if (!isalpha(c)) {
            continue;
        }
        for (p = n; isalpha(c); c = getc(F)) {
            *p++ = c;
        }
        ungetc(c,F);

        *p = 0;
        if (ty == 'L') {
            if ((k = lookup(n,L_Light)) >= 0) {
                light = k;
                continue;
            } else if ((k = lookup(n,L_Temp)) >= 0) {
                temp = k;
                continue;
            }
        }
        if ((k = lookup(n,t)) == -1) {
            fprintf(stderr,
                    "\nError in %s@%s, %cFLAGS: unknown flag %s.\n",
                    m->name, z->name, ty, n);
            xexit(1);
        }
        f[s-k/32-1] |= (1<< (k%32));
    }
    if (temp >= 0) temp <<= LFL_T_SHIFT;
    else temp = LFL_T_ORDINARY;
    if (light >= 0) light <<= LFL_L_SHIFT;
    else light = LFL_LIGHT;

    *f |= temp|light;
}

void mkdef_mob(XZON *z, XMOB *m)
{
    XMOB *p;

    if (m->aux != REFERRED) {
        fprintf(stderr,"\nMobile %s@%s is defined twice!\n", m->name, z->name);
        xexit(1);
    }
    if ((p = z->rmobs) == m) {
        z->rmobs = m->next;
    } else {
        while (p->next != m) p = p->next;
        p->next = m->next;
    }
    m->aux = DEFINED;
    m->mob = -1;
    m->pname = m->name;
    m->next = NULL;
    if ((p = z->mobs) == NULL) z->mobs = m;
    else {
        while (p->next != NULL) p = p->next;
        p->next = m;
    }
}

Boolean read_mob(FILE *F, XZON *z, char *b, int bs)
{
    XMOB *m;
    char s[128];
    char n[128];
    char r[128];
    int c;
    int d;
    char *t;
    char *v;
    int k;
    int i;

    m = NULL;
    for (;;) {
        do {
            c = getc(F);
        }
        while (isspace(c));
        for (t = s; isalnum(c) || c == '_';c = getc(F))
            *t++ = c;
        *t = 0;
        if (t == s) {
            if (c == EOF) {
                return False;
            }
            if (c != '%')
                continue;
            ungetc(c,F);
            break;
        }
        i = -1;
        while (c == ' ' || c == '\t' || c == '=')
            c = getc(F);
        ungetc(c,F);
        k = lookup(s,Mob_tab);
        if (k != T_NAME && m == NULL) {
            fprintf(stderr,
                    "\nError in zone mob:%s: Entry %s is out of sequence.\n",
                    z->name, s);
            xexit(1);
        }
        switch (k) {
        case T_NAME:
            d = 0;
            c = getc(F);
            for (t = n; isalnum(c) || c == '_'; c = getc(F)) {
                *t++ = c;
            }
            *t = 0;
            ungetc(c,F);
            m = find_mob(z,n);
            mkdef_mob(z,m);
            m->mob = -1;
            m->pname = m->name;
            m->loc = 0;
            m->str = 0;
            m->armor = 0;
            m->damage = 10;
	    m->vis = 0;
	    m->agg = 0;
	    m->speed = -1;
	    m->wimpy = 0;
            m->pflags.h = 0;
            m->pflags.l = 0;
            m->mflags.h = 0;
            m->mflags.l = 0;
            m->sflags = 0;
            m->attitude = 0;
            m->att_param = 0;
            m->desc = NULL;
	    m->exam = NULL;
            break;
        case T_PNAME:
            m->pname = v = get_text(F);
            lowercase(v);
            *v = toupper(*v);
            if (strncmp(m->pname,"The ", 4) == 0) {
                v += 4;
                *v = toupper(*v);
            }
            break;
        case T_LOC:
            m->loc = get_lref(z, F);
            break;
        case T_DESC:
            m->desc = get_text(F);
            break;
        case T_END:
            d = 0;
            c = getc(F);
            for (t = n; isalnum(c) || c == '_'; c = getc(F)) {
                *t++ = c;
            }
            *t = 0;
            ungetc(c,F);
	    if (strcasecmp(n,m->name) != 0) {
	      log( 'M', (XOBJ *)m,z,"Wrong arg to end: %s.", n);
	      m->aux = BAD;
	    }
	    m = NULL;
	    break;
        case TMOB_STR:
            m->str = get_int(F);
            break;
        case TMOB_ARMOR:
            m->armor = get_int(F);
            break;
        case TMOB_DAM:
            m->damage = get_int(F);
            break;
        case TMOB_WIMPY:
            m->wimpy = get_int(F);
            break;
        case TMOB_VIS:
            m->vis = get_int(F);
            break;
        case TMOB_AGG:
            m->agg = get_int(F);
            break;
	case TMOB_SPEED:
	    m->speed = get_int(F);
	    break;
	case TMOB_EXAM:
	    m->exam  = get_exam(F);
	    break;
        case TMOB_SFLAGS:
            get_flags(z,m,'S', (int *)&(m->sflags), 
                      sizeof(SFLAGS)/sizeof(int),Sflags,F);
            break;
        case TMOB_PFLAGS:
            get_flags(z,m,'P', (int *)&(m->pflags), 
                      sizeof(PFLAGS)/sizeof(int),Pflags,F);
            break;
        case TMOB_MFLAGS:
            get_flags(z,m,'M', (int *)&(m->mflags), 
                      sizeof(MFLAGS)/sizeof(int),Mflags,F);
            break;
        case -1:
            fprintf(stderr,
                    "\nError in zone mob:%s: %s is not a legal entry.\n",
                    z->name, s);
            xexit(1);
        default:
            fprintf(stderr,
                    "\nError in zone mob:%s: %s isn't implemented yet.\n",
                    z->name, s);
            xexit(1);
        }
        while ((c = getc(F)) != '\n') {
            if (c == EOF)
                return False;
        }
    }
    xfgets(b,bs,F);
    return True;
}

Boolean read_obj(FILE *F, XZON *z, char *b, int bs)
{
    XOBJ *o;
    XOBJ *g;
    char s[128];
    char n[128];
    char r[128];
    int c;
    int d;
    char *t;
    int k;
    int i;
    int cf;
    char cft;
    int x;

    o = NULL;
    for (;;) {
        do {
            c = getc(F);
        }
        while (isspace(c));
        for (t = s; isalnum(c) || c == '_';c = getc(F))
            *t++ = c;
        *t = 0;
        if (t == s) {
            if (c == EOF) {
                return False;
            }
            if (c != '%')
                continue;
            ungetc(c,F);
            break;
        }
        i = -1;
        if (c == '[') {
            c = getc(F);
            for (i = 0;isdigit(c);i = i * 10 + (c - '0'), c = getc(F));
            if (c != ']') {
                fprintf(stderr,
                        "\nError in zone obj:%s: not number in [..]\n",
                        z->name);
            }
            c = getc(F);
        }
        while (c == ' ' || c == '\t' || c == '=')
            c = getc(F);
        ungetc(c,F);
        k = lookup(s,Obj_tab);
        if (k != T_NAME && o == NULL) {
            fprintf(stderr,
                    "\nError in zone obj:%s: Entry %s is out of sequence.\n",
                    z->name, s);
            xexit(1);
        }
        switch (k) {
        case T_NAME:
            d = 0;
            c = getc(F);
            for (t = n; isalnum(c) || c == '_'; c = getc(F)) {
                *t++ = c;
            }
            *t = 0;
            ungetc(c,F);
            o = find_obj(z,n);
            if (o->aux != REFERRED) {
                fprintf(stderr, "\nObject %s@%s is defined twice!\n",
                        n, z->name);
                xexit(1);
            }
            o->aux = DEFINED;
            o->obj = -1;
            o->pname = o->name;
            o->linked = NULL;
            o->aname = NULL;
            o->cflag = -1;
            o->loc = 0;
            o->damage = 0;
            o->armor = 0;
            o->oflags = 0;
            o->state = -1;
            o->mstate = 0;
            o->bvalue = 0;
            o->osize = 0;
            o->oweight = 0;
            o->desc[0] = NULL;
            o->desc[1] = NULL;
            o->desc[2] = NULL;
            o->desc[3] = NULL;
            o->examine = NULL;
	    o->vis = 0;
            break;
        case T_PNAME:
            o->pname = get_text(F);
            lowercase(o->pname);
            break;
        case T_LOC:

	    fscanf(F, "%d\t:", &cf); /* */
	    /* fscanf(F, "%d%*[\t ]:", &cf); */

            switch(cf) {
            case IN_ROOM: cft = 'L'; break;
            case IN_CONTAINER: cft = 'O'; break;
            case CARRIED_BY:
            case WIELDED_BY:
            case WORN_BY:
            case BOTH_BY:
                cft = 'M';
                break;
            default:
                fprintf(stderr,
                        "\nError in obj:%s@%s: Illegal carry flag %d.\n",
                        o->name, z->name, cf);
                xexit(1);
            }
            o->cflag = cf;
            o->loc = get_ref(cft, z, F);
            break;
        case T_DESC:
            if (i < 0 || i >= 4) {
                fprintf(stderr,
                        "\nError in obj:%s@%s: Illegal index in Desc %d.\n",
                        o->name, z->name, i);
            }
            o->desc[i] = get_text(F);
            break;
        case T_END:
            d = 0;
            c = getc(F);
            for (t = n; isalnum(c) || c == '_'; c = getc(F)) {
                *t++ = c;
            }
            *t = 0;
            ungetc(c,F);
	    if (strcasecmp(n,o->name) != 0) {
	      log( 'O', o, z,"Wrong arg to end: %s.", n);
	      o->aux = BAD;
	    }
	    o = NULL;
	    break;
        case TOBJ_ANAME:
            o->aname = get_text(F);
            break;
        case TOBJ_OFLAGS:
            get_flags(z,(XMOB *)o,'O',
                     (int *)&(o->oflags), sizeof(OFLAGS)/sizeof(int),Oflags,F);
            break;
        case TOBJ_ARMOR:
            o->armor = get_int(F);
            break;
        case TOBJ_DAMAGE:
            o->damage = get_int(F);
            break;
        case TOBJ_MAX_STATE:
            o->mstate = get_int(F);
            break;
        case TOBJ_STATE:
            o->state = get_int(F);
            break;
        case TOBJ_BVALUE:
            o->bvalue = get_int(F);
            break;
        case TOBJ_SIZE:
            o->osize = get_int(F);
            break;
        case TOBJ_WEIGHT:
            o->oweight = get_int(F);
            break;
        case TOBJ_LINKED:
            if ((o->linked = get_oref(z,F)) == NULL)
                o->linked = (XOBJ *)(-1);
            break;
        case TOBJ_EXAM:
            o->examine = get_exam(F);
            break;
        case TOBJ_VIS:
            o->vis = get_int(F);
            break;
        case -1:
            fprintf(stderr,
                    "\nError in zone obj:%s: %s is not a legal entry.\n",
                    z->name, s);
            xexit(1);
        default:
            fprintf(stderr,
                    "\nError in zone obj:%s: %s isn't implemented yet.\n",
                    z->name, s);
            xexit(1);
        }
        while ((c = getc(F)) != '\n') {
            if (c == EOF)
                return False;
        }
    }
    xfgets(b,bs,F);
    return True;
}

void mkdef_loc(XZON *z, XLOC *l)
{
    XLOC *p, *q;

    if (l->aux != REFERRED) {
        fprintf(stderr,"\nRoom %s@%s is defined twice!\n", l->name, z->name);
        xexit(1);
    }
    if ((p = z->rlocs) == l) {
        z->rlocs = l->next;
    } else {
        while (p->next != l) p = p->next;
        p->next = l->next;
    }
    l->aux = DEFINED;
    l->loc = 0;
    l->next = NULL;
    if ((p = z->locs) == NULL) z->locs = l;
    else {
        while (p->next != NULL) p = p->next;
        p->next = l;
    }
}

Boolean read_loc(FILE *F, XZON *z, char *b, int bs)
{
    XLOC *l;
    char s[128];
    char n[128];
    char r[128];
    int c;
    int d;
    int g;
    char *t;
    int k;
    int i;

    l = NULL;
    for (;;) {
        do {
            c = getc(F);
        }
        while (isspace(c));
        for (t = s; isalnum(c) || c == '_';c = getc(F))
            *t++ = c;
        *t = 0;
        if (t == s) {
            if (c == EOF) {
                return False;
            }
            if (c == '%') {
                ungetc(c,F);
                break;
            }
            continue;
        }

        l = find_loc(z,s);
        mkdef_loc(z,l);

        while (c != ';') {
            while (isspace(c))
                c = getc(F);
            for (t = n; isalpha(c); c = getc(F)) {
                *t++ = c;
            }
            *t = 0;
            if (t == n) {
                if (c == ';')
                    break;
                if (t == n && c != ';') {
                    fprintf(stderr,"\n Expected ';' in room %s@%s.\n",
                            s, z->name);
                    xexit(1);
                }
            }
            if (c != ':') {
                fprintf(stderr,"\n Expected ':' in room %s@%s.\n",
                        s, z->name);
                xexit(1);
            }
            if ((k = lookup(n,Exits)) == -1) {
                fprintf(stderr,"\n Illegal exit %s in room %s@%s.\n",
                        n, s, z->name);
                xexit(1);
            }
	    if (l->exit_types[k] != 0) {
	      log('L', (XOBJ *)l, z,
		  "Multiple defineds of exit %s.", Exits[k]);
	    }

            for (c = getc(F);isspace(c); c = getc(F));
            switch(c) {
            case '#':
                l->exit_types[k] = '#';
                l->exits[k] = (XLOC *)get_int(F);
                break;
            case '^':
                l->exit_types[k] = '^';
                l->exits[k] = (XLOC *)get_oref(z, F);
                break;
    default:
                l->exit_types[k] = ' ';
                ungetc(c,F);
                l->exits[k] = get_lref(z, F);
            }
            c = getc(F);
        }
        while (getc(F) != '\n');
        c = getc(F);
        while (isspace(c))
            c = getc(F);
        for (t = n; isalpha(c); c = getc(F)) {
            *t++ = c;
        }
        *t = 0;
        if ((g = t - n) == 0 || strncasecmp(n,"Lflags", g) != 0) {
            fprintf(stderr,"\nError in %s@%s, Lflags entry required.\n",
                    s, z->name);
            xexit(1);
        }
        ungetc(c,F);
        get_flags(z,(XMOB *)l,'L',
                  (int *)&(l->lflags), sizeof(LFLAGS)/sizeof(int),Lflags,F);
        while (getc(F) != '\n');
        c = getc(F);
        t = n;
        while (c != '^') {
            *t++ = c;
            c = getc(F);
        }
        *t = 0;
        while (getc(F) != '\n');
        l->pname = save_text(n);
        l->description = get_description(F);
    }
}

void read_zones(XZON *zo, int nz)
{
    int z;
    XZON *zon;
    FILE *F;
    Boolean multiple;
    char buff[128];
    char buff2[128];

    printf("\n");
    for (z = 0, zon = zo; z < nz; ++z, ++zon, pclose(F)) {
        sprintf(buff2, "Zone %d(%s) from %s", z, zon->name, zon->fname);
        printf("    %-75s\r", buff2);
        fflush(stdout);
        if ((F = zopen(zon->fname,
                       zon->name,&multiple,buff,sizeof(buff))) == NULL) {
            fprintf(stderr,"\nZone not found: %s in file %s.\n",
                    zon->name, zon->fname);
            xexit(1);
        }
        if (strncasecmp(buff,"%mobiles",8) == 0) {
            if (!read_mob(F,zon,buff,sizeof(buff)))
                continue;
        }
        if (strncasecmp(buff, "%objects", 8) == 0) {
            if (!read_obj(F,zon,buff,sizeof(buff)))
                continue;
        }
        if (strncasecmp(buff, "%locations", 10) == 0) {
            read_loc(F,zon,buff,sizeof(buff));
            continue;
        } else if (strncasecmp(buff, "%zone:", 6) == 0) {
            if (!multiple) {
                fprintf(stderr,"\nError in zone file, %s unexpected.\n", buff);
                xexit(1);
            }
            continue;
        } else {
            fprintf(stderr, "\n Error in zone file: unknown sequence %s.",
                    buff);
            xexit(1);
        }

    }
}

Boolean obj_ok(XOBJ *O, XOBJ *F, char t)
{
    XOBJ *L;
    XOBJ *G;
    XLOC *R;
    int k;

    if (O == 0 || O == (XOBJ *)(-1) || O->aux == BAD) {
        return False;
    } else if (O->aux == F && t == IS_LINK) {
        return True;
    } else if (O->aux == F) {
        log('O',O,O->zone,"Recursive contents in object.");
        O->aux = BAD;
        return False;
    } else if (O->loc == NULL) {
        log('O',O,O->zone,"Location does not exist.");
        O->aux = BAD;
        return False;
    } else if (O->obj != -1) {
        log('O',O,O->zone,"O->obj == %d, should be -1");
        O->aux = BAD;
        return False;
    } else if (O->aux != UNKNOWN) {
        return True;
    }

    O->aux = F;
    if (O->cflag == IN_CONTAINER && !obj_ok((XOBJ *)(O->loc),F,IS_CONTAINED)) {
        G = (XOBJ *)(O->loc);
        log('O',O,O->zone,"Container %s@%s[%s] is bad",
            G->name, G->zone->name, G->pname);
        O->aux = BAD;
        return False;
    }

    if ((L = O->linked) != NULL) {
        if (L == (XOBJ *)(-1)) {
            log('O',O,O->zone,"Linked object doesn't exist.");
            O->aux = BAD;
            return False;
        } else if (L == O) {
            log('O',O,O->zone,"Linked object to itself.");
            O->aux = BAD;
            return False;
        } else if (L->linked != O) {
            log('O',O,O->zone,"Linked object %s@%s[%s] isn't linked back.",
                L->name, L->zone->name, L->pname);
            O->aux = BAD;
            return False;
        } else if (L->aux == BAD) {
            log('O',O,O->zone,"Linked object %s@%s[%s] is bad.",
                L->name, L->zone->name, L->pname);
            O->aux = BAD;
            return False;
        } else if ((G = L->aux) != GOOD) {
            if (G == UNKNOWN)
                G = L;

            if (!obj_ok(L,G,IS_LINK)) {
                log('O',O,O->zone,"Error with linked object %s@%s[%s].",
                    L->name, L->zone->name, L->pname);
                L->aux = BAD;
                O->aux = BAD;
                return False;
            }
        }

        if (O->state == -1) {
            if ((O->state = L->state) == -1) {
                O->state = L->state = 0;
            }
        } else if (L->state == -1) {
            L->state = O->state;
        } else if (L->state != O->state) {
            log('O',O,O->zone,
                "Initial states on linked objects %s@%s[%s] are not the same.",
                    L->name, L->zone->name, L->pname);
            L->aux = BAD;
            O->aux = BAD;
            return False;
        }

    } else if (O->state == -1) {
        O->state = 0;
    }

    if ((R = O->loc) != NULL) {
        switch(O->cflag) {
        case IN_ROOM:
            if (((int)(R->loc)) <= 0 || R->aux != GOOD)
                R = NULL;
            break;
        case IN_CONTAINER:
            break;
        case CARRIED_BY:
        case WORN_BY:
        case WIELDED_BY:
        case BOTH_BY:
            if (((XMOB *)R)->mob < 0 || ((XMOB *)R)->aux != GOOD)
                R = NULL;
            break;
        default:
            R = NULL;
        }
    }
    if (R == NULL) {
        log('O',O,O->zone,"Invalid location.");
        O->aux = BAD;
        return False;
    }

/*
    if (O->linked == NULL) {
        if (not_linked_l == NULL) {
            not_linked = not_linked_l = O;
        } else {
            not_linked_l->the_next = O;
            not_linked_l = O;
        }
        ++num_unlinked;
    } else if (O < O->linked) {
        if (linked_objs_l == NULL) {
            linked_objs = linked_objs_l = O;
        } else {
            linked_objs_l->the_next = O;
            linked_objs_l = O;
        }
        ++num_linked;
    }
*/

    if (obj_list == NULL) {
	    obj_list_l = obj_list = O;
    }
    else {
	    obj_list_l->the_next = obj_list_l = O;
    }

    if (O->linked == NULL)
      ++num_unlinked;
    else if (O < O->linked)
      ++num_linked;


    O->aux = GOOD;
    return True;
}

void clean_up(XZON *zon, int nz, int *mm, int *oo, int *lo, int *ll)
{
    XOBJ *O;
    XOBJ *F;
    XMOB *M;
    XLOC *L;
    XLOC *E;
    XZON *Z;
    int x;
    int k;
    int l;
    int m;
    int o;
    int z;
    char et;

    for (M = mobs, x = 0; x < num_m; M++, x++) {
        if (M->aux == BAD)
	  continue;
        if (M->aux != DEFINED) {
            M->aux = BAD;
            log('M',(XOBJ *)M,M->zone,"Mobile isn't properly defined..");
        } else {
	    char *p = M->pname;
            M->aux = UNKNOWN;
	    if (M->agg == -1) {
	      M->agg = 10;     /*((k = abs(M->lev)) <= 200 ? 0 : k%100);*/
	    }
	    if (M->speed == -1) {
	      M->speed = 5;
	    }
	    while (isalpha(*p) || *p == ' ') p++;
	    if (*p != '\0')
	      log('M',(XOBJ *)M,M->zone,
	      "Mobile in-game name must consist of letters only.");
        }
    }
    for (L = locs, x = 0; x < num_l; L++, x++) {
        if (L->aux != DEFINED) {
            log('L',(XOBJ *)L,L->zone,"Location isn't properly defined..");
            L->aux = BAD;
        } else {
            L->aux = UNKNOWN;
            L->loc = -2;
        }
    }
    for (O = objs, x = 0; x < num_o; O++, x++) {
        if (O->aux == BAD)
	    continue;
        if (O->aux != DEFINED) {
            log('O',O,O->zone,"Object isn't properly defined..");
            O->aux = BAD;
        } else {
	    char *p = O->pname;
            O->obj = -2;
            O->the_next = NULL;
            O->aux = UNKNOWN;
	    while (isalpha(*p) || *p == ' ') p++;
	    if (*p != '\0')
	      log('O',O,O->zone,
	      "Object in-game name must consist of letters only.");
        }
    }

    for (Z = zon, z = 0, o = 0; z < nz; ++z, ++Z) {
        Z->n_loc = 0;
        Z->n_mob;
        for (O = Z->objs; O != NULL; O = O->next) {
            if (O->aux == UNKNOWN) {
                O->obj = -1;
                O->zone = Z;
            }
        }

        for (L = Z->locs; L != NULL; L = L->next) {
            if (L->aux == UNKNOWN) {
                L->loc = -1;
            }
        }
    }

    for (Z = zon, z = 0, l = 0; z < nz; ++z, ++Z) {
        Z->loc = l;
        for (L = Z->locs; L != NULL; L = L->next) {
            if (L->aux == UNKNOWN && L->pname != NULL
                && L->description != NULL) {
                L->loc = ++l;
                L->aux = GOOD;
                Z->n_loc++;
            }
        }
    }

    for (Z = zon, z = 0, m = 0; z < nz; ++z, ++Z) {
        Z->mob = m;
        for (M = Z->mobs; M != NULL; M = M->next) {
            if (M->aux == UNKNOWN && M->loc != NULL && M->loc->aux == GOOD) {
                M->mob = m++;
                M->aux = GOOD;
                Z->n_mob++;
            }
        }
    }

    for (Z = zon, z = 0, o = 0; z < nz; ++z, ++Z) {
        for (O = Z->objs; O != NULL; O = O->next) {
            if ((F = O->aux) == BAD || F == GOOD)
                continue;
            if (F ==  REFERRED || F == DEFINED) {
                log('O',O,O->zone,"Object isn't properly checked.");
                O->aux = BAD;
                continue;
            }
            if (F == UNKNOWN)
                F = O;
            if (!obj_ok(O,F,IS_CHECK)) {
                log('O',O,O->zone,"General error with object.");
                O->aux = BAD;
            } else {
                O->aux = GOOD;
            }
        }
    }

    for (Z = zon, z = 0; z < nz; ++z, ++Z) {
        for (L = Z->locs; L != NULL; L = L->next) {
            if (L->aux != GOOD)
                continue;
            for (k = 0; k < NEXITS; k++) {
                E = L->exits[k];
                switch(L->exit_types[k]) {
                case '^':
                    O = (XOBJ *)E;
                    if (O->cflag != IN_ROOM
                        || O->loc != L
                        || O->aux != GOOD
                        || (F = O->linked) == NULL
                        || F->cflag != IN_ROOM) {
                        L->exits[k] = NULL;
                        L->exit_types[k] = 0;
                    }
                    break;
                case '#':
                    break;
                case ' ':
                    if (E == NULL || E->aux != GOOD) {
                        L->exits[k] = NULL;
			L->exit_types[k] = 0;
		    }
                    break;
		case 0:
		    if (L->exits[k] != NULL) {
		      log('L', (XOBJ *)L, L->zone,
			  "Garbage in exit %s, should be none.", Exits[k]);
		      L->exits[k] = NULL;
		    }
		    break;
                default:
		    log('L', (XOBJ *)L, L->zone,
			"Garbage in exit %s, exit type = %c[%03o]",
			Exits[k], L->exit_types[k],  L->exit_types[k]);
                    L->exits[k] = NULL;
                    L->exit_types[k] = 0;
                    break;
               }
            }
        }
    }

/*
    for (O = linked_objs, o = 0; O != NULL; O = O->the_next) {
        if (O->aux != GOOD || O->linked == NULL
            || O->linked->aux != GOOD) {
            printf("\nPanic Object %s@%s were supposed to be good!\n",
                   O->name, O->zone->name);
            xexit(1);
        }
        O->obj = o++;
        O->linked->obj = o++;
    }
    *lo = o;

    for (O = not_linked; O != NULL; O = O->the_next) {
        if (O->aux != GOOD) {
            printf("\nPanic Object %s@%s were supposed to be good!\n",
                   O->name, O->zone->name);
            xexit(1);

        }
        O->obj = o++;
    }
*/

    for (O = obj_list, o = *lo = 0; O != NULL; O = O->the_next) {
	    if (O->aux != GOOD || O->linked != NULL && O->linked->aux != GOOD){
		    printf("\nPanic Object %s@%s were supposed to be good!\n",
			   O->name, O->zone->name);
		    xexit(1);
	    }

	    O->obj = o++;

	    if (O->linked != NULL) (*lo)++;
    }

    *mm = m;
    *oo = o;
    *ll = l;
}

void write_zone(XZON *ZON, int numz)
{
    FILE *F;
    int z;
    XZON *Z;

    F = Do_fopen(zone_file,"w");
    fprintf(F,"%d\n", numz);
    for (Z = ZON, z = 0; z < numz; ++z, ++Z) {
        fprintf(F,"%s\t%d\n", Z->name, Z->loc);
    }
    fclose(F);
}

void write_mob(XZON *ZON, int numz,int m)
{
    FILE *F;
    FILE *H;
    int z;
    int i = 0;
    XZON *Z;
    XMOB *M;
    char zn[32];
    char n[64];
    char b[128];

    F = Do_fopen(mob_file,"w");
    H = Do_fopen(mobh_file, "w");
    fprintf(F,"%d\n", m);

   /* Print out header */
    (void) fprintf(H, "\
/*\n\
**\tMobiles file header generated from %s\n\
**\tDON'T MAKE CHANGES HERE -- THEY WILL GO AWAY!\n\
*/\n\n",
       main_file);

    for (Z = ZON, z = 0; z < numz; ++z, ++Z) {
        strcpy(zn,Z->name);
        uppercase(zn);
        fprintf(H, "#define MOBMIN_%s\t%d\n", zn, Z->mob);
        fprintf(H, "#define MOBMAX_%s\t%d\n", zn, Z->mob + Z->n_mob);
        for (M = Z->mobs; M != NULL; M = M->next, i++) {
            if (M->aux != GOOD) {
	      log('M',(XOBJ *)M,Z,"Mobile not found good.");
	      continue;
            }
            fprintf(F,"%s^\n", M->pname);
            fprintf(F,"%d %d %d %d %d %d %d %d %d %d %d\n",
                    i, i, z, -(M->loc->loc), M->str,
		    M->damage, M->agg, M->armor, M->speed, M->vis, M->wimpy);
            fprintf(F,"0x%08x 0x%08x:0x%08x\n",
                    M->sflags, M->pflags.h, M->pflags.l);
            fprintf(F,"0x%08x:0x%08x\n", M->mflags.h, M->mflags.l);
            fprintf(F, "%s^\n", M->desc);
	    fprintf(F, "%s^\n\n", M->exam == NULL ? "" : M->exam);
            strcpy(n,M->name);
            uppercase(n);
            fprintf(H, "#define MOB_%s_%s\t%d\n", zn, n, M->mob);
        }
    }
    fclose(F);
    fclose(H);
}

void write_xobj(FILE *F, FILE *H, FILE *X, XOBJ *O, int obj_num)
{
    XLOC *L;
    XOBJ *C;
    XMOB *M;
    XZON *Z;
    int x;
    int y;
    int j;
    OFLAGS f;
    char *s;
    int obj_loc;
    char n[64];
    char z[32];
    char b[128];

    char                *name;
    int                 obj;

    L = O->loc;
    switch(O->cflag) {
    case IN_ROOM: Z = L->zone; obj_loc = -(L->loc); break;
    case IN_CONTAINER: C = (XOBJ *)L; Z = C->zone; obj_loc = C->obj; break;
    case CARRIED_BY:
    case WIELDED_BY:
    case WORN_BY:
    case BOTH_BY: M = (XMOB *)L; Z = M->zone; obj_loc = M->mob; break;
    default:
        printf("\nPanic carry flag is %d!\n", O->cflag);
        xexit(1);
    }

    f = O->oflags;


    fprintf(F,"%s %s %d %d %d %d %d %d %d %d %d %d %ld %d %d %d %d\n",
            O->pname, (O->aname == NULL?"<null>":O->aname),
	    O->zone->zone,
	    obj_num, obj_num,
	    O->linked != NULL ? O->linked->obj : -1,
	    O->vis, O->cflag, obj_loc,
            O->state, O->damage,
            O->armor, O->oflags,
            O->mstate, O->bvalue,
            O->osize, O->oweight);
    for (x = 0; x < 4; ++x) {
        if ((s = O->desc[x]) == NULL)
            s = "";
        fprintf(F,"%s^\n", s);
    }
    if ((s = O->examine) == NULL)
        s = "";
    fprintf(F,"%s^\n\n", s);
    strcpy(n,O->name);
    uppercase(n);
    strcpy(z,O->zone->name);
    uppercase(z);

    fprintf(H,"#define OBJ_%s_%s\t%d\n", z, n, O->obj);
    fprintf(X,"\n\nName = %s@%s, PName = %s\nLocation = ",
            O->name, O->zone->name, O->pname);
    switch(O->cflag) {
    case IN_ROOM:
        fprintf(X, "In room %s@%s[%s](%d)",
                L->name, Z->name, L->pname, obj_loc);
        break;
    case IN_CONTAINER:
        fprintf(X, "In container %s@%s[%s](%d)",
                C->name, Z->name, C->pname, obj_loc);
        break;
    case CARRIED_BY:
    case WIELDED_BY:
    case WORN_BY:
    case BOTH_BY:
        fprintf(X, "%s %s@%s[%s](%d)",
                Cflags[O->cflag], M->name, Z->name, M->pname, obj_loc);
        break;
    }

    if (O->linked != NULL)
      fprintf(X, "\nLinked to object %d (%s@%s)\n", O->linked->obj,
	      O->linked->name, O->linked->zone->name);

    fprintf(X,"\nState = %d, Damage = %d, Armor = %d, Flags = 0x%08x\n",
            O->state, O->damage, O->armor, f);
    if (f != 0) {
        fprintf(X,"OFlags:");
        for (y = 31, j = 0; y >= 0; --y, f <<= 1) {
            if (f >= 0)
                continue;
            if (Oflags[y] != NULL)
                fprintf(X, " %s", Oflags[y]);
            else
                fprintf(X, " <spare %d>", y);
            if (++j != 5)
                continue;
            j = 1;
            putc('\n', X);
        }
        putc('\n', X);
    }
}

void write_obj(XZON *ZON, int numz, int o, int l)
{
    FILE *F;
    FILE *H;
    FILE *X;
    int   i, z;
    XZON *Z;
    XOBJ *O;

    F = Do_fopen(obj_file,"w");
    H = Do_fopen(objh_file, "w");
    X = Do_fopen(reset_list, "w");

/*    fprintf(F,"%d/%d\n", o, l);*/
    fprintf(F,"%d\n", o);

   /* Print out header */
    (void) fprintf(H, "\
/*\n\
**\tObject file header generated from %s\n\
**\tDON'T MAKE CHANGES HERE -- THEY WILL GO AWAY!\n\
*/\n\n",
       main_file);

/*
    fprintf(X,"\n**************\n\n Linked objects\n\n**************\n");
    for (O = linked_objs; O != NULL; O = O->the_next) {
        fprintf(X,"\n ===========================\n");
        write_xobj(F,H,X,O);
        write_xobj(F,H,X,O->linked);
    }
    fprintf(X,"\n **************\n\n Unlinked objects\n\n**************\n");
    for (O = not_linked; O != NULL; O = O->the_next) {
        write_xobj(F,H,X,O);
    }
*/
    fprintf(X,"\n**************\n\n Objects\n\n**************\n");
    for (O = obj_list, i = 0; O != NULL; O = O->the_next, i++) {
        fprintf(X,"\n ===========================\n");
        write_xobj(F,H,X,O,i);
    }


    fprintf(X, "\n *************\n\n The End\n\n**************\n");
    fclose(F);
    fclose(H);
    fclose(X);
}

static char *c_ex(char *s,int x, int *Ex, Boolean *ex,Boolean neg)
{
    char c;

    if (ex[x]) {
        ex[x] = False;
    } else if ((Ex[x] == 0) == neg) {
        ex[x] = True;
    } else {
        c = s[-1];
        while (*s != 0) {
            if (*s == '^' && s[1] == c)
                return s + 2;
            ++s;
        }
    }
    return s;
}

static char *c_zo(char *s,Boolean neg)
{
    XZON *Z;
    char zname[64];
    char *t, *q;
    int z;

    if (*s != '(')
        return s;
    for (t = zname, q = s; *++q != 0 && *q != ')';)
        *t++ = *q;
    if (*q == 0)
        return s;
    ++q;
    if (t == zname)
        return q;
    *t = 0;
    for (z = 0, Z = zones; z < num_z; ++z, ++Z) {
        if (strcasecmp(zname,Z->name) == 0)
            continue;
        neg = !neg;
        break;
    }
    if (neg)
        return q;
    while (*q != 0 && (*q != '^' || q[1] != 'z')) ++q;
    if (*q == 0)
        return q;
    q += 2;
    if (*q == '(' && q[1] == ')')
        q += 2;
    return q;
}

void write_loc(XZON *ZON,int numz,int l)
{
    FILE *F;
    FILE *H;
    int z;
    int x;
    int y;
    int i = -1;
    Boolean neg;
    Boolean ex[NEXITS];   /* True if in condition ^<exit> */
    int Ex[NEXITS]; /* The calculated exit number.. */
    XZON *Z;
    XLOC *L;
    XLOC *E;
    char *s;
    char c;
    char zn[32];
    char n[64];
    char b[128];

    F = Do_fopen(loc_file,"w");
    H = Do_fopen(loch_file, "w");
    fprintf(F,"%d\n", l);

   /* Print out header */
    (void) fprintf(H, "\
/*\n\
**\tLocations file header generated from %s\n\
**\tDON'T MAKE CHANGES HERE -- THEY WILL GO AWAY!\n\
*/\n\n",
       main_file);

    for (Z = ZON, z = 0; z < numz; ++z, ++Z) {
        strcpy(zn,Z->name);
        uppercase(zn);
        fprintf(H, "#define LOCMIN_%s\t%d\n", zn, -(Z->loc));
        fprintf(H, "#define LOCMAX_%s\t%d\n", zn, -(Z->loc + Z->n_loc));
        for (L = Z->locs; L != NULL; L = L->next, i--) {
            if (L->aux != GOOD) {
                log('L',(XOBJ *)L,Z,"Location not found good.");
                continue;
            }
            fprintf(F,"%d %d", /*L->loc,*/ i, z);
            for (x = 0; x < NEXITS; x++) {
                E = L->exits[x];
                ex[x] = False;
                switch (L->exit_types[x]) {
                case '^': y = DOOR + ((XOBJ *)L->exits[x])->obj; break;
                case '#': y = (int)(L->exits[x]); break;
                case ' ': y = -(L->exits[x]->loc); break;
                case 0: y = 0; break;
                default:
                    printf("\nPanic Room %d has illegal exit type %s: %c!\n",
                           L->loc, Exits[x], L->exit_types[x]);
                    xexit(1);
                }
                Ex[x] = y;
                fprintf( F, " %d", y);
            }
            fprintf(F, "\n0x%08x\n%s^\n", L->lflags, L->pname);
            s = L->description;
            while (*s != 0) {
                if ((c = *s++) != '^') {
                    putc(c,F);
                    continue;
                }
                neg = False;
                if (*s == '!') {
                    neg = True;
                    ++s;
                }

                switch(c = *s++) {
                case 'n':       s = c_ex(s,0,Ex,ex,neg); continue;
                case 'e':       s = c_ex(s,1,Ex,ex,neg); continue;
                case 's':       s = c_ex(s,2,Ex,ex,neg); continue;
                case 'w':       s = c_ex(s,3,Ex,ex,neg); continue;
                case 'u':       s = c_ex(s,4,Ex,ex,neg); continue;
                case 'd':       s = c_ex(s,5,Ex,ex,neg); continue;
                case 'z':       s = c_zo(s,neg); continue;
                default:        putc(c,F); continue;
                }
            }
            fprintf(F,"^\n");

            strcpy(n,L->name);
            uppercase(n);
            fprintf(H, "#define LOC_%s_%s\t%d\n", zn, n, -(L->loc));
        }
    }
    fclose(F);
    fclose(H);
}

void write_files(XZON *ZON, int numz, int l, int o, int lo, int m)
{
    write_zone(ZON,numz);
    write_mob(ZON,numz,m);
    write_obj(ZON,numz,o,lo);
    write_loc(ZON,numz,l);
}

void make_data(int argc,char **argv)
{
    int numz;
    int numl;
    int numo;
    int numm;
    int numlo;

    if (argc < 2) {
        fprintf(stderr,"\nToo few arguments.\n");
        xexit(1);
    }
    numz = read_main(argv[1]);
    read_zones(zones,numz);
    clean_up(zones,numz,&numm,&numo,&numlo,&numl);

    write_files(zones,numz,numl,numo,numlo,numm);
    printf("\nMade %d zones with %d locations,\n%d objects (%d linked) and %d mobiles.\nUsed %d of text space out of a total of %d (%d unused)\n",
           numz, numl, numo, numlo, numm, textsp - texts, sizeof(texts),
           sizeof(texts) - (textsp - texts));
    if (n_logs > 0) {
        printf("\nNumber of errors is %d, check log file %s.\n\n",
               n_logs, log_file);
        fprintf(LogFile, "Total number of logs is %d.\n", n_logs);
    } else {
        printf("\nNo errors, log file %s is empty.\n\n", log_file);
    }
    fclose(LogFile);
}



#ifdef VARGS

void log(char t,XOBJ *O,XZON *Z,char *f, ...)
{
    va_list pvar;
    char *n;
    char *z;
    char *p;
    char *q;
    char b[1024];

    va_start(pvar,f);
    vsprintf(b,f,pvar);
    z = Z->name;
    ++n_logs;
    switch (t) {
    case 'L':
        n = ((XLOC *)O)->name;
        p = ((XLOC *)O)->pname;
        q = "Loc";
        break;
    case 'M':
        n = ((XMOB *)O)->name;
        p = ((XMOB *)O)->pname;
        q = "Mob";
        break;
    case 'O':
        n = O->name;
        p = O->pname;
        q = "Obj";
        break;
    default:
        fprintf(stderr,"\nIllegal type %c.\n", t);
        exit(1);
    }
    fprintf(LogFile,"%s: %s@%s[%s] %s\n", q, n, z, p, b);
}

#else

void log(char t,XOBJ *O,XZON *Z,char *f,
         char *arg1, char *arg2, char *arg3, char *arg4)
{
    char *n;
    char *z;
    char *p;
    char *q;
    char b[1024];

    sprintf(b, f, arg1, arg2, arg3, arg4);

    z = Z->name;
    ++n_logs;
    switch (t) {
    case 'L':
        n = ((XLOC *)O)->name;
        p = ((XLOC *)O)->pname;
        q = "Loc";
        break;
    case 'M':
        n = ((XMOB *)O)->name;
        p = ((XMOB *)O)->pname;
        q = "Mob";
        break;
    case 'O':
        n = O->name;
        p = O->pname;
        q = "Obj";
        break;
    default:
        fprintf(stderr,"\nIllegal type %c.\n", t);
        exit(1);
    }
    fprintf(LogFile,"%s: %s@%s[%s] %s\n", q, n, z, p, b);
}
#endif


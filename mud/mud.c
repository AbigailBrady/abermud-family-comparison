#define DATA_DEF
#include <signal.h>
#include <sys/errno.h>
#include <strings.h>
#include "kernel.h"
#include "mud.h"
#include "key.h"
#include "macros.h"
#include "support.h"
#include "blib.h"
#include "bprintf.h"
#include "parse.h"
#include "exec.h"
#include "tk.h"

extern int errno;
extern char* sys_errlist[];
 
char	*dirns[6] = {"North", "East ", "South", "West ", "Up   ", "Down "};
char	*txt1 = "";
char	*txt2 = "";
char	localname[32];
char	in_ms[81] = "has arrived.";
char	out_ms[81] = "has gone %s.";
char	o_in_ms[81] = "";
char	o_out_ms[81] = "";
char	min_ms[81] = "appears with an ear-splitting bang.";
char	mout_ms[81] = "vanishes in a puff of smoke.";
char	vin_ms[81] = "suddenly appears!";
char	vout_ms[81] = "has vanished!";
char	prm_str[81] = ">";
char	wd_her[16] = " ";
char	wd_him[16] = " ";
char	wd_them[16] = " ";
char	wd_it[16] = " ";
char	wordbuf[128] = " ";
int	cms = 1;
Boolean	iskb = True;
int	fighting = -1;
int	mynum = -1;
int	ob1 = -1;
int	ob2 = -1;
int	pl1 = -1;
int	pl2 = -1;
int	prep = -1;
int	snoopd = -1;
int	snoopt = -1;

static int sig_active;

int op(int plnum)
{
  return (plev(plnum) >= LVL_GOD);
}

int master(char *name)
{
  return MASTERUSER(name);
}

void mudprog(char *name)
{
  time_t a;
  char username[10];
  char localtty[16];

  userid = getuid();
  sys_uid = geteuid();
  polymorphed = -1;
  get_username(username);
  get_ttynum(localtty);
  time(&a);
  srand((int)a);
  sig_init();
  bootstrap();
  bprintf("Entering Game ...\n");
  strcpy(globme, name);
  bprintf("Hello %s\n", globme);
  bflush();
  sprintf(localname, "%s %s", username, localtty + 5);
  setstdin();
  talker(globme);
}

void crapup(char *str)
{
  char x[80];
  static char *dashes =
    "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-";

  if (aliased) {
    bprintf("%s\n", str);
    unalias();
    error();
    return;
  }
  if (polymorphed != -1) {
    bprintf("%s\n", str);
    unpolymorph();
    error();
    return;
  }
  key_mode = pr_due = 0;	/* So we dont get a prompt after the exit */
  bprintf("\n%s\n\n%s\n\n%s\n\001S2\377", dashes, str, dashes);
  sprintf(x, "%s", t_username);
  loseme();
  closeworld();
  bflush();
  keysetback();
  execl(EXE_FILE, EXE_FILE, x, "-m", (char *)NULL);
  fprintf(stderr,"%s: Couldn't execute \"%s\" (%s).\n",
	  exe_file, EXE_FILE, sys_errlist[errno]);
}

int sig_occurred = 0;

void sig_wait()
{
  sig_occurred = 1;
  if (sig_active)
    alarm(IO_INTER);
}

void sig_alon()
{
  sig_active = 1;
  signal(SIGALRM, sig_occur);
  alarm(IO_INTER);
}

void unblock_alarm()
{
  signal(SIGALRM, sig_occur);
  if (sig_occurred)
    sig_occur();
}

void block_alarm()
{
  sig_occurred = 0;
  signal(SIGALRM, sig_wait);
}

void sig_aloff()
{
  sig_active = 0;
  alarm(0);
  signal(SIGALRM, SIG_IGN);
}

void sig_occur()
{
  if (!sig_active)
    return;
  sig_aloff();
  openworld();
  interrupt = 1;		/* What's this for? */
  rte();			/* Process messages */
  interrupt = 0;
  on_timing();			/* Move mobiles */
  closeworld();
  key_reprint();
  sig_alon();
}

void ohdear()
{
  setplev(mynum, LVL_ARCHWIZARD);
  zapped = True;
  crashcom();
  bprintf("Something very unpleasant has just happened.\n");
  exit(0);
}

void sig_init()
{
  signal(SIGHUP, sig_oops);
  signal(SIGSEGV, ohdear);
  signal(SIGINT, SIG_IGN);
  signal(SIGTERM, sig_ctrlc);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGQUIT, sig_ctrlc);
}

void sig_oops()
{
  sig_aloff();
  loseme();
  exit(255);
}

void sig_ctrlc()
{
  printf("^C\n");
}

void setstdin() {}

void fetchprmpt(int level)
{
  FILE *a;
  char c, x[128];
  int l;

  if ((a = fopen(LEV_PRM, "r")) == NULL) {
    strcpy(prm_str, "?");
    return;
  }
  while (fgets(x, sizeof x, a)) {
    sscanf(x, "%c:%d:%s", &c, &l, prm_str);
    if (l == level) {
      if (c == 'Y') {
	fgets(x, sizeof x, a);
	*index(x, '\n') = 0;
	if (!EMPTY(x))
	  strcpy(in_ms, x);

	fgets(x, sizeof x, a);
	*index(x, '\n') = 0;
	if (!EMPTY(x))
	  strcpy(out_ms, x);

	fgets(x, sizeof x, a);
	*index(x, '\n') = 0;
	if (!EMPTY(x))
	  strcpy(min_ms, x);

	fgets(x, sizeof x, a);
	*index(x, '\n') = 0;
	if (!EMPTY(x))
	  strcpy(mout_ms, x);

	fgets(x, sizeof x, a);
	*index(x, '\n') = 0;
	if (!EMPTY(x))
	  strcpy(vin_ms, x);

	fgets(x, sizeof x, a);
	*index(x, '\n') = 0;
	if (!EMPTY(x))
	  strcpy(vout_ms, x);
      }
      fclose(a);
      return;
    }
  }
  fclose(a);
  strcpy(prm_str, ">");
}

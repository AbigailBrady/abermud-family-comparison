/*
**  The login and menu driver.
*/

#include <time.h>
#include <signal.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "kernel.h"
#include "macros.h"
#include "mudprog.h"
#include "blib.h"
#include "mud.h"
#include "gametime.h"
#include "maint.h"
#include "pflags.h"
#include "shutdown.h"

char t_username[81];
char t_password[20];
char exe_file[40];
Boolean isawiz;
Boolean ismonitored;
Boolean candetach;

void exit(int rc)
{
  bflush();
  keysetback();
  _exit(rc);
}

#ifdef CLOSEHOUR
static void leave_game()
{
  printf("AberMUD has now closed.\n\n");
  printf("Thank you for playing....\n");
  exit(0);
}

static void explode_game()
{
  signal(SIGALRM, leave_game);
  printf("\007\007AberMUD is closing in 4 minutes.\n");
  alarm(240);
}

static void setalrm()
{
  struct tm *x;
  time_t i;
  unsigned int j;

  time(&i);
  x = localtime(&i);
  /* Now how long is it till kicking off time */
  j = 0;
  if (x->tm_hour > CLOSEHOUR)
    {
      /* Its tomorrow */
      j = (23 - x->tm_hour) * 60 * 60;
      x->tm_hour = 0;
    }
  j += (60 - x->tm_min) * 60;	/* Minutes */
  j += (CLOSEHOUR - x->tm_hour) * 3600 - 4;
  signal(SIGALRM, explode_game);
  alarm(j);
}
#endif	/* CLOSEHOUR */

#ifdef OPENHOUR
static int mudopen()
{
  time_t x;
  struct tm *a;

  time(&x);
  a = localtime(&x);
  return (a->tm_hour >= OPENHOUR || a->tm_hour < CLOSEHOUR
	  || a->tm_wday == 6 || a->tm_wday == 0);
}
#endif

Boolean infile(FILE *fl, char *user)
{
  register char *p;
  char a[80];

  while (fgets(a, sizeof a, fl))
    {
      if (p = index(a, '\n'))
	*p = 0;
      if (EQ(user, a))
	return True;
    }
  return False;
}

/*
**
**  The whole login system is called from this
**
*/

void login(char *user, int password)
{
  register char *p;
  FILE *fl, *f2;
  char a[80], pwd[32], pv[32], block[128], u[80];
  int i, match, check;

  pclrflg(mynum, pfl(Color));	/* Eliminate bug on non-ansi
				   terminals on start up */
  keyinit();
  for ( ; ; )
    {
      check = password;
      password = 1;

      if (EMPTY(user))
        {
	  bprintf("\nWho do you want to be? ");
	  readline(user, 12);
	  if ((user[0] == '\0') || (user[0] == EOF))
            {
	      bprintf("Ok, bye then.\n");
	      exit(0);
            }
        }

      for (p = user; *p; p++)
	if (!isalnum(*p) && !(*p == '-'))
	  break;
      if (*p)
        {
	  bprintf("Sorry, the name may only contain alphanumeric characters.\n");
	  user[0] = '\0';
	  continue;
        }
      if (p - user > 12)
        {
	  bprintf("Please choose a name with 12 characters or less.\n");
	  user[0] = '\0';
	  continue;
        }
      if (islower(user[0]))
	user[0] = toupper(user[0]);

      if ((fl = fopen(ILLEGAL_FILE, "r")) != NULL)
	if (infile(fl, user))
	  {
	    fclose(fl);
	    bprintf("Sorry, I can't call you that.\n");
	    user[0] = '\0';
	    continue;
	  }
        else
	  fclose(fl);

      if (UserExists(user, a) == -1) {
	  bprintf("\nDid I get the name right, %s?", user);
	  readline(a, 1);
	  if (a[0] == 'n' || a[0] == 'N') {
	      bprintf("\n");
	      user[0] = '\0';
	      continue;
	  }
      }

      if (fl = fopen(PRIVED_FILE, "r")) {
	  isawiz = infile(fl, user);
	  fclose(fl);
      } else {
	  isawiz = False;
      }

      /* Is this player being monitored?  */
      if (fl = fopen(MONITOR_FILE, "r")) {
	  ismonitored = infile(fl, user);
	  fclose(fl);
      } else {
	  ismonitored = False;
      }

      /* Now see if user has been banned.  */
      if (fl = fopen(BAN_FILE, "r")) {
	  if (infile(fl, user)) {
	      bprintf("You've been banned from the game!\n");
	      fclose(fl);
	      exit(3);
	  }
	  fclose(fl);
      }
      break;
  }

  if (!check)
    return;

  i = UserExists(user, block);
  strcpy(pwd, user);		/* save for new user */
  if (i == 1)
    {
      GetFields(block, user, pwd);
      for (i = 0; i < 2; i++)
        {
	  mygetpass("Code word: ", block);
	  if ((block[0] == '\0') || (block[0] == EOF))
            {
	      bprintf("Ok, bye then.\n");
	      exit(0);
            }
	  Crypt(block, pwd);
	  if (EQ(block, pwd))
	    break;
	  bprintf("Incorrect password\n");
        }
      if (i == 2)
        {
	  bprintf("Bad password\n");
	  exit(1);
        }
    }
  else
    {
      /* this bit registers the new user */
      bprintf("Creating new persona...\n");
      for ( ; ; )
        {
	  mygetpass("Give me a password for this persona:", pv);
	  if (EMPTY(pv))
	    continue;
	  mygetpass("Please enter your password again:", block);
	  if (!EQ(block, pv))
            {
	      bprintf("Passwords don't match.  Please try again.\n");
	      continue;
            }
	  break;
        }
      Crypt(block, salt());
      strcpy(user, pwd);
      strcpy(pwd, block);
      sprintf(block, "%s\001%s\001", user, pwd);
      if ((fl = openlock(PASSWORDFILE, "a")) == NULL)
        {
	  bprintf("No persona file\n");
	  exit(1);
        }
      fprintf(fl, "%s\n", block);
      closelock(fl);
    }
}

#ifndef SETVBUF
char iobuf[BUFSIZ];
#endif

int main(int argc, char *argv[])
{
  char z[60], u[80];
  int r, i;
  int is_update, is_become, goto_menu;
  FILE *a;
  struct stat sb;
  time_t ct;

#ifdef SETVBUF
  setvbuf(stdout, NULL, _IOFBF, BUFSIZ);
#else
  setbuf(stdout, iobuf, BUFSIZ);
#endif
  keysetup();

  if (chdir(GAME_DIR))
    {
      bprintf("Cannot change to game directory!\n");
      exit(1);
    }

  strcpy(exe_file, argv[0]);
  t_username[0] = '\0';
  t_password[0] = '\0';
  is_update = 0;
  is_become = 0;
  goto_menu = 0;
  for (i = 1; i < argc; i++)
    {
      if (EQ(argv[i], "-u"))
	is_update = 1;
      else if (EQ(argv[i], "-b"))
	is_become = 1;
      else if (EQ(argv[i], "-m"))
	goto_menu = 1;
      else
	strcpy(t_username, argv[i]);
    }

#ifdef HOST_MACHINE
  /* Ensure that we are running on the correct host; see the     */
  /* notes about the use of flock(); and the affects of lockf(); */
  gethostname(u, 33);
  if (!EQ(u, HOST_MACHINE))
    {
      fprintf(stderr, "AberMUD is only available on %s, not on %s.\n",
	      HOST_MACHINE, u);
      exit(1);
    }
#endif			/* HOST_MACHINE */

  get_username(u);
  candetach = (EQ(u, "jaf") || EQ(u, "lad") || EQ(u, "hogan"));

  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);

#ifdef CLOSEHOUR
  setalrm();
#endif

  if (is_become)
    {
      login(t_username, 1);
      mudprog(t_username);
    }
  else if (is_update)
    {
      login(t_username, 0);
      mudprog(t_username);
    }
  else if (goto_menu)
    login(t_username, 0);
  else
    {
      /* Show date created, motd, etc. */
      bprintf("\001C\n\t\t\t   A B E R M U D    I V\n\n");
      bprintf("\t\t  By Alan Cox, Richard Acott, Jim Finnis\n\n");
      bprintf("This AberMUD was created: %s", stat(EXE_FILE, &sb) < 0 ? "<unknown>\n" : ctime(&sb.st_mtime));

      bprintf("Game Time Elapsed: ");
      eltime();			/* Get the game time elapsed */
      login(t_username, 1);	/* Does all of the login stuff */

      /* list the message of the day */
      bprintf("\001C\001f%s\377", MOTD);
      pwait();
    }

  if (!is_update && !is_become)
    {
      for ( ; ; )
        {
	  bprintf("\001C\nWelcome To AberMUD IV [Unix]\n");
	  bprintf("Options:\n");
	  bprintf("\t0]  Exit AberMUD\n");
	  bprintf("\t1]  Enter the game\n");
	  bprintf("\t2]  Enter description\n");
	  bprintf("\n\nSelect> ");
	  readline(z, 1);
	  switch (z[0])
	    {
	    case '0':
	      exit(0);
	    case '1':
	      bprintf("\001CThe Hallway\t\tElapsed Time: ");
	      eltime();
	      bprintf("\nYou stand in a long dark hallway, which echoes to the tread of your booted\n");
	      bprintf("feet.  You stride on down the hall, choose your masque, and enter the worlds\n");
	      bprintf("beyond the known...\n\n");
	      bflush();
	      mudprog(t_username);
	      break;
	    case '2':
	      setchardesc(t_username);
	      break;
	    default:
	      bprintf("Bad option\n\001S2\377");
	      break;
            }
        }
    }
}  

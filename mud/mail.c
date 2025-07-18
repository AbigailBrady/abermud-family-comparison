/*
**  The MAIL command.
*/

#include <strings.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include "kernel.h"
#include "macros.h"
#include "support.h"
#include "uaf.h"
#include "bprintf.h"
#include "blib.h"
#include "pflags.h"
#include "mail.h"

void
mailcom()
{
  time_t log_time;
  char *time_string, file[80], line[80], *name;
  extern int errno;
  FILE *fl;
  PERSONA junk;

  if (EMPTY(item1))
    {
      sprintf (file, "%s%s", MSGFILE, pname(mynum));
      if (fl = fopen(file, "r"))
	{
	  bprintf ("\n");
	  while (fgets(line, sizeof line, fl))
	    bprintf ("%s", line);
	  fclose (fl);
	  unlink (file);
	  bprintf ("\n");
	  return;
	}
      switch (errno)
	{
	case EACCES :
	  bprintf("Your answering machine seems to be broken.\n");
	  return;
	  default :
	  bprintf("Sorry, you have no messages!\n");
	  return;
	}
    }
  if (ptstflg(mynum, pfl(CannotMSG)))
    {
      bprintf("You have no phone.\n");
      return;
    }
  else if (findpers(item1, &junk) == 0)
    {
      bprintf("That player is not in the AberMUD phone book.\n");
      return;
    }

  convflg = 2;
  name = item1;
  if (islower(*name))
    *name = toupper(*name);

  sprintf (file, "%s%s", MSGFILE, name);

  if ((mail_fl = openlock (file, "a")) == NULL)
    switch (errno)
      {
      case EACCES :
	bprintf("There is a ring, but no answer.\n");
	return;
	default :
	bprintf("Your phone is not functioning properly.\n");
	mudlog("ERROR in msgcom() at fopen(message-file, append): %d",
	       errno);
	return;
      }
  else
    {
      (void) time(&log_time);
      time_string = ctime(&log_time);
      *index(time_string, '\n') = 0;

      fprintf(mail_fl, "From %s at %s\n", pname(mynum), time_string);
      bprintf("Composing mail.  Type '*help mail' for help.\n");
    }
}

/* Checks for new mail. */

void
checkmail()
{
  char file[BUFSIZ];
  FILE *fl;
  struct stat msgstatus;
  static time_t last_modified = 0;
  extern int errno;

  sprintf (file, "%s%s", MSGFILE, pname(mynum));
  if (stat (file, &msgstatus) != -1)		/* see when last modified */
    if (last_modified != msgstatus.st_mtime)
      {
	bprintf ("\007You have new mail.  Type MAIL to read.\n");
	last_modified = msgstatus.st_mtime;
      }
}

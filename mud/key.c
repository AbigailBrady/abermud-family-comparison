/*
**  Keyboard input functions
*/

#include <sgtty.h>
#include "kernel.h"
#include "key.h"
#include "bprintf.h"
#include "pflags.h"
#include "macros.h"

static char *pr_bf;
static long save_flag = -1;
int key_echo = False;

void keysetup()
{
  struct sgttyb	x;

  gtty(fileno(stdin), &x);
  save_flag = x.sg_flags;
  x.sg_flags &= ~ECHO;
  x.sg_flags |= CBREAK;
  stty(fileno(stdin), &x);
}

void keysetback()
{
  struct sgttyb	 x;

  if (save_flag != -1) {
    gtty(fileno(stdin), &x);
    x.sg_flags = save_flag;
    stty(fileno(stdin), &x);
  }
}

int keyecho(int n)
{
  if (isatty (fileno (stdin)))
    return (key_echo = n);
  else {
    key_echo = False;
    return (True);
  }
}

void keyinit(void)
{
  keyecho (True);
}

char hilite[] = "\001A\033[36m\377";
char lolite[] = "\001A\033[37m\377";

static void key_prompt(void)
{
  dcprnt (hilite, stdout);
  dcprnt (pr_bf, stdout);
  dcprnt (lolite, stdout);
}

void key_reprint()
{
  int tmp;

  if (pr_due && key_mode) {
    tmp = snoopd;
    snoopd = 0;
    key_prompt();
    dcprnt(key_buff, stdout);
    fflush(stdout);
    snoopd = tmp;
  }
  pr_due = False;
}

void
key_input (char *ppt, int len_max)
{
  int len_cur, x, tmp;

  bflush();
  key_mode = True;
  pr_bf = ppt;
  key_prompt();
  key_buff[len_cur = 0] = '\0';

  while (1)
    {
      pr_due = False;
      fflush(stdout);

      if (feof(stdin))
	{
	  loseme();
	  exit(2);
	}

      switch (x = getchar()) {

      case 'H' & 0x1F:
      case 127:
	if (len_cur)
	  {
	    if (key_echo)
	      dcprnt("\b \b", stdout);
	    key_buff[--len_cur] = '\0';
	  }
	continue;

	/* SOCKET HACK */
      case '\r':
	break;

      case '\n':
	if (key_echo)
	  {
	    dcprnt("\n", stdout);
	    bflush();
	  }
	key_mode = False;
#ifdef FLUSH_INPUT		/* disables mouse/file buffered input */
	if (stdin->_cnt > FLUSH_INPUT)
	  stdin->_cnt = 0;
#endif
	return;

      case 'R' & 0x1F:
	if (key_echo)
	  {
	    for (tmp = len_cur; tmp; tmp--)
	      dcprnt("\b \b", stdout);
	    bprintf("%s", key_buff);
	  }
	continue;

      case 'W' & 0x1F:
	while (len_cur && key_buff[len_cur - 1] == ' ')
	  {
	    if (key_echo)
	      dcprnt("\b \b", stdout);
	    key_buff[--len_cur] = '\0';
	  }
	while (len_cur && key_buff[len_cur - 1] != ' ')
	  {
	    if (key_echo)
	      dcprnt("\b \b", stdout);
	    key_buff[--len_cur] = '\0';
	  }
	continue;

      case 'U' & 0x1F:
	if (key_echo)
	  for (tmp = len_cur; tmp; tmp--)
	    dcprnt("\b \b", stdout);
	key_buff[len_cur = 0] = '\0';
	continue;

      case 255:
	if ((x = getchar()) >= 4)
	  getchar();
	if (x != 255)
	  continue;

      default:
	if (x < 32)
	  continue;

	if (len_cur < len_max)
	  {
	    key_buff[len_cur++] = x;
	    key_buff[len_cur] = '\0';
	    if (key_echo)
	      dcprnt(&key_buff[len_cur-1], stdout);
	  }
	else
	  dcprnt("\a", stdout);
      }
    }
}

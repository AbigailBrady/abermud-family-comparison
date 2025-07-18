/*
**  Maintenance functions.
*/

#include "maint.h"
#include "kernel.h"
#include "macros.h"
#include "bprintf.h"
#include "blib.h"

static char sccsid[] = "@(#)maint.c	4.100.0 (IthilMUD)	6/02/90";

static char saltch[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789./";

char *salt(void)
{
    static char str[3];

    srandom(time(NULL));
    str[0] = saltch[random() & 63];
    str[1] = saltch[random() & 63];
    str[2] = '\0';
    return(str);
}

void
Crypt (char *pwd, char *s)
{
  register char *tmp;
  tmp = crypt(pwd, s);
  strcpy(pwd, tmp);
}

void
mygetpass (char *prompt, char *str)
{
    int echo;

    if (echo = keyecho (False))
      bprintf ("WARNING:  Your password will be displayed as you type it.\n");
    bprintf ("%s", prompt);
    readline (str, 8);
    if (!echo)
      bprintf ("\n");
    keyecho (True);
}

int
UserExists (char *uid, char *block)
{
    FILE *unit;
    char namefromfile[128], wk2[128];

    if ((unit = openlock(PASSWORDFILE, "r")) == NULL)
    {
        bprintf("No persona file\n");
        exit(1);
    }
    strcpy(wk2, uid);
    while (fgets(block, 255, unit))
    {
        GetFields(block, namefromfile, (char *)NULL);
        if (EQ(namefromfile, wk2))
        {
            closelock(unit);
            return 1;
        }
    }
    closelock(unit);
    return -1;
}

void setchardesc(char *user)
{
    FILE *fl;
    char file[60], s[80];

    sprintf(file, "%s%s", DESCFILE, user);
    if ((fl = openlock(file, "w")) == NULL)
    {
        bprintf("The description editor is closed right now.  Please try again later.\n");
        return;
    }      
    bprintf("Enter the description you'd like other people to see when they examine you.\n");
    bprintf("End with a blank line.  If you make a mistake you'll have to reenter the\n");
    bprintf("whole thing.  Sorry.\n");
    do
    {
        bprintf("#>");
        readline(s, 79);
        fprintf(fl, "%s\n", s);
    } while (!EMPTY(s));
    closelock(fl);
    bprintf("Description Set.\n");
}

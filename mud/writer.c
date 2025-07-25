#include "kernel.h"
#include "writer.h"
#include "bprintf.h"

/*
 * MUD writer
 *
 * Note that the mud writer is reentrant, you may start one writer
 * while another is active, the writer will save the previous writer's state
 * prompt et.c the writer tries to save all info about the previous
 * input handler, cprompt is also saved.
 * This means that for example while doing 'mail xyzzy' in mud
 * you may do '*mail zyxxy' inside there, to mail another person.
 * by typing '**' you will then terminate the last mail and you will
 * get back to the previous mail and yet another ** will terminate your
 * mail to xyzzy and get you back to whereever you were when you did
 * mail xyzzy.
 *
 *
 * Also note that it is not an editor, you may write text and the text
 * can replace old text or may be appended to old text, but you don't have
 * the old text available while inside the writer.
 *
 *
 * The MUD writer is used like this:
 *
 * example a simple mail writer for mud.
 *
 * mailcom(void)
 * {
 *    if (brkword() != -1) { /* send mail
 *        start_writer( "Write your mail, terminate with '**'",
 *                      "MAIL>",
 *                      wordbuf,  /* The name who we want to send mail to
 *                      strlen(wordbuf) + 1, /* length of argument
 *                      mail_handler,
 *                      WR_CMD|'*',          /* allow commands
 *                      500);                /* Max 500 lines
 *
 *        return;
 *    } else { /* read mail
 *      read_mail();
 *   }
 * }
 *
 * mail_handler(void *w,void *ad, int adlen)
 * {
 *    FILE *f;
 *    char b[100];
 *    char a[100];
 *
 *    strcpy(b,"MAIL/");
 *    strcpy(a,ad); /* save the address, wgets will destroy ad
 *    strcat(b,a);
 *    if ((f = fopen(b,"a")) == NULL) {
 *       progerror(b);
 *       terminate_writer(w);
 *       return;
 *    } else {
 *       while (wgets(b,sizeof(b),w) != NULL) {
 *           fputs(b,f);
 *       }
 *       /* notify the person he has mail
 *       notify_mail(a); /* can't use ad here as it is destroyed by wgets
 *    }
 * }
 *
 */

typedef struct _wr_line {
  struct _wr_line *next;
  char            *s;
} WrLine;

typedef struct {
  void              *previous;
  char              *prevprompt;
  void              (*exit_handler)(void *w,void *arg,int arglen);
  void              (*old_inp_h)(char *str);
  char              *prompt;
  void              *arg;
  int		    arglen;
  int               flags;
  int               max_lines;
  int               num_lines;
  WrLine            *first, *last;
} WrHead;

static void write_handler(char *line);

void start_writer(char *h,char *p,void *arg,int arglen,
		  void handler(void *x,void *arg,int arglen),
		  int flags,int max_lines)
{
  WrHead *w;

  if (handler == NULL || h == NULL || p == NULL || max_lines <= 0) return;
  w = NEW(WrHead,1);
  w->previous      = cur_player->writer;
  w->prevprompt    = COPY(cur_player->cprompt);
  w->exit_handler  = handler;
  w->old_inp_h     = cur_player->inp_handler->inp_handler;
  w->prompt        = COPY(p);
  w->arg           = BCOPY(arg,arglen);
  w->arglen        = arglen;
  w->flags         = flags;
  w->max_lines     = max_lines;
  w->num_lines     = 0;
  w->first         = NULL;
  w->last          = NULL;
  cur_player->writer = w;
  strcpy(cur_player->cprompt,p);
  bprintf( "%s\n\n", h );
  write_handler(NULL);
}
  
static void write_handler(char *line)
{
  WrHead *w = (WrHead *)cur_player->writer;
  WrLine *l;
  int len;
  int k;

  if ((k = (w->flags & WR_CMDCH)) == 0) k = '*';
  if (line == NULL) {
    replace_input_handler(write_handler);
  } else if (*line == k) {
    if (line[1] == k && line[2] == '\0') {
      /* Exit writer. */
      replace_input_handler(w->old_inp_h); /* Back to old handler */
      free(w->prompt);         /* Don't need the prompt any more */
      if (w->first == NULL) {  /* Let it point to text instead */
	w->prompt = NULL;
      } else {
	w->prompt = w->first->s;
      }
      cur_player->writer = w->previous;
      strcpy(cur_player->cprompt,w->prevprompt);
      free(w->prevprompt);

/* Don't need max lines anymore, use it for unget buffer */

      w->max_lines = EOF;
      w->exit_handler(w,w->arg,w->arglen);
      bprintf( "\r%s", cur_player->cprompt);
      return; /* Don't increment num_lines. */
    } else if ((w->flags & WR_CMD) != 0) {
      gamecom(line+1,True);
    } else {
      bprintf( "You cannot execute any commands now.\n");
    }
  } else if (w->num_lines == w->max_lines) {
    bprintf( "You cannot type in more lines.\n" );
  } else {
    l = NEW(WrLine,1);
    l->s = COPY(line);
    l->next = NULL;
    if (w->first == NULL) { /* First line. */
      w->first = w->last = l;
    } else {
      w->last->next = l;
      w->last = l;
    }
    ++w->num_lines;
  }
  bprintf( "\r%s", cur_player->cprompt);
}

int wnum_lines(void *x)
{
   WrHead *w = x;

   return w == 0 ? 0 : w -> num_lines;
}

int wnum_chars(void *x)
{
   WrHead *w = x;
   WrLine *n;
   int k = 0;

   if (w == 0) return 0;
   for (n = w -> first; n != 0; n = n -> next) k += strlen(n -> s);
   return k;
}

int wgetc(void *x)
{
  WrHead *w = x;
  WrLine *l;
  WrLine *n;
  char *s;
  int k;

  if (w == NULL) return EOF;
  if ((k = w->max_lines) != EOF) {
    w->max_lines = EOF;
    return k; /* return unget character */
  }
  if (w->prompt == NULL) {
    free(w->arg);
    free(w);
    return EOF;
  }
  if ((k = *w->prompt++) == '\0') {
    /* End of this line, kill it and return '\n' */
    l = w->first;
    if ((n = w->first = l->next) == NULL) {
      w->prompt = NULL;
    } else {
      w->prompt = n->s;
    }
    w->num_lines--;
    free(l->s);
    free(l);
    return '\n';
  } else {
    return k;
  }
}

int wungetc(int c, void *x)
{
  WrHead *w;
  WrLine *l;
  char *s;

  if ((w = x) == NULL) return EOF;
  if (c == EOF) {
    while ((l = w->first) != NULL) {
      w->first = l->next;
      free(l->s);
      free(l);
    }
    w->prompt = NULL;
    w->num_lines = 0;
    return EOF;
  }
  if ((l = w->first) != NULL && w->prompt > l->s) {
    *--(w->prompt) = c;
    return c;
  }
  /* Try to put it into max_lines */
  if (w->max_lines != EOF) return EOF; /* Unable to push character back */
  w->max_lines = c;
  return c;
}

char *wgets(char *buf,int buflen,void *w)
{
  int i, k;
  char *s = buf;

  for (i = 1; i < buflen; i++) {
    if ((k = wgetc(w)) == EOF) {
      *s = 0;
      return NULL;
    }
    if ((*s++ = k) == '\n') break;
  }
  *s = 0;
  return buf;
}

void terminate_writer(void *w)
{

  if (w != NULL) {
    (void)wungetc(EOF,w);
    (void)wgetc(w);
  }
}

void terminate_all_writers(int plx)
{
  WrHead *w;
  WrHead *x;

  if (plx >= 0 && plx < max_players) {
    w = players[plx].writer;
    while ((x = w) != NULL) {
      w = w->previous;
      terminate_writer(x);
    }
  }
}


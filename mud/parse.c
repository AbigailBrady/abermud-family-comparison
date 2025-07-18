/*
**  Parse command-line
*/

#include <strings.h>
#include <signal.h>
#include "kernel.h"
#include "macros.h"
#include "objects.h"
#include "locations.h"
#include "mobiles.h"
#include "verbs.h"
#include "sendsys.h"
#include "pflags.h"
#include "oflags.h"
#include "version.h"
#include "weather.h"
#include "parse.h"
#include "support.h"
#include "blib.h"
#include "tk.h"
#include "flags.h"
#include "exec.h"
#include "new1.h"
#include "opensys.h"
#include "mud.h"
#include "new2.h"
#include "objsys.h"
#include "magic.h"
#include "uaf.h"
#include "extra.h"
#include "blood.h"
#include "change.h"
#include "zones.h"
#include "bprintf.h"
#include "frob.h"
#include "god.h"
#include "locdir.h"
#include "key.h"
#include "lflags.h"
#include "mail.h"
#include "maint.h"

#define NOISECHAR(c)	((c) == ' ' || (c) == '.' || (c) == ',' || (c) == '%')

extern int errno;
extern char* sys_errlist[];

static int tmynum;		/* Who am I really if aliased	*/

/*
**  Test if string contains a substring
*/
static int comparethem(char *a, char *b, int n)
{
  char c, d;

  while (--n >= 0) {
    c = *a++;
    d = *b++;
    if (isupper(c))
      c = tolower(c);
    if (c != d)
      return 0;
  }
  return 1;
}

int contains(char *s1, char *s2)
{
  register int b, c;

  b = strlen(s2);
  c = strlen(s1) - b;
  if (c >= 0)
    for (; c; c--)
      if (comparethem(s1 + c, s2, b - 1))
	return 1;
  return -1;
}

void pncom()
{
  bprintf("Current pronouns are:\n");
  bprintf("Me    : %s\n", globme);
  bprintf("It    : %s\n", wd_it);
  bprintf("Him   : %s\n", wd_him);
  bprintf("Her   : %s\n", wd_her);
  bprintf("Them  : %s\n", wd_them);
}

int gamecom(char *str)
{
  int a;

  if (!EQ(str, "!")) {
    strcpy(strbuf, str);
    if (EMPTY(str))
      return 0;
  }
  else if (EQ(str, "!"))
    strcpy(str, strbuf);
  stp = 0;
  if (brkword() == -1) {	/* if line contains nothing but  */
    erreval();			/* pn's or articles (a, an, the) */
    return -1;
  }
  if ((a = chkverb()) != -1) {
    doaction(a);
    return 0;
  }
  if (fextern(wordbuf) != -1) {	/* handle external commands */
    stp = 0;
    return 0;
  }
  if ((a = fpbn(wordbuf)) != -1) { /* translate NAME to TELL NAME */
    stp = 0;
    a = VERB_TELL;
    doaction(a);
    return 0;
  }
  if (plev(mynum) >= LVL_GOD)
	bprintf("Oh great and mighty One, I, your humble servant, did not understand.\n");
  else {
    bprintf("Pardon?\n");
  }
  return -1;
}

int
brkword()
{
  static int neword = 0;
  int worp;

  if (!stp)
    neword = 0;
  if (neword) {
    neword = 0;
    return 0;
  }
  for ( ; ; ) {
    while (NOISECHAR(strbuf[stp]))
      stp++;
    for (worp = 0; strbuf[stp] && !NOISECHAR(strbuf[stp]); )
      wordbuf[worp++] = strbuf[stp++];
    wordbuf[worp] = 0;
    lowercase(wordbuf);
    if (EQ(wordbuf, "the") || EQ(wordbuf, "a") || EQ(wordbuf, "an")
	|| EQ(wordbuf, "of") || EQ(wordbuf, "with"))
      continue;
    if (EQ(wordbuf, "it"))
      strcpy(wordbuf, wd_it);
    if (EQ(wordbuf, "him"))
      strcpy(wordbuf, wd_him);
    if (EQ(wordbuf, "her"))
      strcpy(wordbuf, wd_her);
    if (EQ(wordbuf, "them"))
      strcpy(wordbuf, wd_them);
    if (EQ(wordbuf, "me"))
      strcpy(wordbuf, globme);
    break;
  }
  return (worp ? 0 : -1);
}

int chklist(char *word, char *lista[], int listb[])
{
  int a, b, c;
  int d = -1;

  b = c = 0;
  for (a = 0; lista[a]; a++)
    if ((b = Match(word, lista[a])) > c) {
      c = b;
      d = listb[a];
    }
  return (c < 5 ? -1 : d);
}

int Match(char *x, char *y)
{
  if (strncasecmp(x, y, strlen(x)) == 0)
    return 10000;
  return 0;
}

int chkverb()
{
  return chklist(wordbuf, verbtxt, verbnum);
}

void doaction(int n)
{
  char xx[128];

  openworld();
  if (tables(n) == 2)
    return;
  if (n > 1 && n < 8) {
    if (i_follow) {
      bprintf("You stopped following.\n");
      i_follow = False;
    }
    dodirn(n);
    return;
  }
  if (isforce)
    switch (n) {
      case VERB_QUIT:
      case VERB_LOG:
      case VERB_UPDATE:
      case VERB_BECOME:
      case VERB_BUG:
      case VERB_TYPO:
      case VERB_CLS:
      case VERB_FORCE:
      case VERB_CHANGE:
      case VERB_PET:
        bprintf("You can't be forced to do that.\n");
        return;
    }
  if (in_fight)
    switch (n) {
      case VERB_QUIT:
      case VERB_UPDATE:
      case VERB_BECOME:
        bprintf("Not in the middle of a fight!\n");
	return;
    }
  switch (n) {
  case VERB_DELETEUSER:
    if (plev(mynum) >= LVL_GOD)
      if (!EMPTY(item1))
	deluser(item1);
      else
	bprintf("Who do you want to delete?\n");
    else
      bprintf("Tsk, tsk!  I don't think so.\n");
    break;
  case VERB_MAIL:
    mailcom();
    break;
  case VERB_GO:
    if (i_follow)
      {
	bprintf("You stopped following.\n");
	i_follow = False;
      }
    dogocom();
    break;
  case VERB_FLOWERS:
    flowercom();
    break;
  case VERB_TICKLE:
    ticklecom();
    break;
  case VERB_PET:
    petcom();
    break;
  case VERB_QUIT:
    rte();
    openworld();
    if (aliased)
      {
	unalias();
	break;
      }
    if (polymorphed != -1)
      {
	bprintf("A mysterious force won't let you quit.\n");
	break;
      }
    sprintf(xx, "%s has left the game\n", pname(mynum));
    bprintf("Ok");
    sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, ploc(mynum), xx);
    sprintf(xx, "[Quitting Game: %s]\n", pname(mynum));
    sendsys(pname(mynum), pname(mynum), SYS_WIZ, 0, xx);
    loseme();
    crapup("                                    Goodbye");
    break;
  case VERB_SIT:
    sitcom();
    break;
  case VERB_STAND:
    standcom();
    break;
  case VERB_GET:
    getobj();
    break;
  case VERB_CRASH:
    crashcom();
    break;
  case VERB_DROP:
    dropitem();
    break;
  case VERB_WHO:
    whocom();
    break;
  case VERB_MWHO:
    mwhocom();
    break;
  case VERB_RESET:
    rescom();
    break;
  case VERB_ZAP:
    zapcom();
    break;
  case VERB_EAT:
    eatcom();
    break;
  case VERB_SAVE:
    saveme();
    break;
  case VERB_STEAL:
    stealcom();
    break;
  case VERB_REVIEW:
    reviewcom();
    break;
  case VERB_LEVELS:
    levelscom();
    break;
  case VERB_NEWS:
    closeworld();
    bprintf("\001f%s\377", NEWS);
    break;
  case VERB_MUDLIST:
    closeworld();
    bprintf("\001f%s\377", MUDLIST);
    break;
  case VERB_COUPLES:
    closeworld();
    bprintf("\001f%s\377", COUPLES);
    break;
  case VERB_WIZLIST:
    wizlistcom();
    break;
  case VERB_BULLETIN:
    if (plev(mynum) < LVL_WIZARD)
      erreval();
    else
      {
	closeworld();
	bprintf("\001f%s\377", BULLETIN);
      }
    break;
  case VERB_INFO:
    closeworld();
    bprintf("\001f%s\377", INFO);
    break;
  case VERB_HELP:
    helpcom();
    break;
  case VERB_STATS:
    showplayer();
    break;
  case VERB_EXAMINE:
    examcom();
    break;
  case VERB_SUMMON:
    sumcom();
    break;
  case VERB_WIELD:
    weapcom();
    break;
  case VERB_KILL:
    killcom();
    break;
  case VERB_POSE:
    posecom();
    break;
  case VERB_SET:
    setcom();
    break;
  case VERB_PRAY:
    praycom();
    break;
  case VERB_STORM:
    set_weather(stormy);
    break;
  case VERB_RAIN:
    set_weather(rainy);
    break;
  case VERB_SUN:
    set_weather(sunny);
    break;
  case VERB_SNOW:
    set_weather(snowing);
    break;
  case VERB_HAIL:
    set_weather(hailing);
    break;
  case VERB_TIPTOE:
    goloccom(1);
    break;
  case VERB_GOTO:
    goloccom(0);
    break;
  case VERB_WEAR:
    wearcom();
    break;
  case VERB_REMOVE:
    removecom();
    break;
  case VERB_PUT:
    putcom();
    break;
  case VERB_WAVE:
    wavecom();
    break;
  case VERB_BLIZZARD:
    set_weather(blizzard);
    break;
  case VERB_FORCE:
    forcecom();
    break;
  case VERB_LIGHT:
    lightcom();
    break;
  case VERB_EXTINGUISH:
    extinguishcom();
    break;
  case VERB_CRIPPLE:
    cripplecom();
    break;
  case VERB_CURE:
    curecom();
    break;
  case VERB_HEAL:
    healcom();
    break;
  case VERB_MUTE:
    dumbcom();
    break;
  case VERB_CHANGE:
    changecom();
    break;
  case VERB_MISSILE:
    missilecom();
    break;
  case VERB_SHOCK:
    shockcom();
    break;
  case VERB_FIREBALL:
    fireballcom();
    break;
  case VERB_FROST:
    frostcom();
    break;
  case VERB_BLOW:
    blowcom();
    break;
  case VERB_WIZ:
    wizcom();
    break;
  case VERB_EXITS:
    exits();
    break;
  case VERB_OBJECTS:
    dircom();
    break;
  case VERB_PUSH:
    pushcom();
    break;
  case VERB_IN:
    incom();
    break;
  case VERB_INVISIBLE:
    inviscom();
    break;
  case VERB_VISIBLE:
    viscom();
    break;
  case VERB_DEAFEN:
    deafcom();
    break;
  case VERB_RESURRECT:
    resurcom();
    break;
  case VERB_LOG:
    logcom();
    break;
  case VERB_ZONES:
    loccom();
    break;
  case VERB_USERS:
    usercom();
    break;
  case VERB_UPDATE:
    updcom();
    break;
  case VERB_BECOME:
    becom();
    break;
  case VERB_SYSTAT:
    if (plev(mynum) >= LVL_WIZARD)
      system("uptime; who");
    else
      system("finger");
    break;
  case VERB_CONVERSE:
    convcom();
    break;
  case VERB_SNOOP:
    snoopcom();
    break;
  case VERB_RAW:
    rawcom();
    break;
  case VERB_ROLL:
    rollcom();
    break;
  case VERB_CREDITS:
    bprintf("\001f%s\377", CREDITS);
    break;
  case VERB_BRIEF:
    bprintf("Brief mode %s.\n", pxorflg(mynum, pfl(Brief)) ? "on" : "off");
    break;
  case VERB_JUMP:
    jumpcom();
    break;
  case VERB_WHERE:
    wherecom();
    break;
  case VERB_FLEE:
    fleecom();
    break;
  case VERB_BUG:
    bugcom();
    break;
  case VERB_TYPO:
    typocom();
    break;
  case VERB_ACTIONS:
    lisextern();
    break;
  case VERB_PN:
    pncom();
    break;
  case VERB_BLIND:
    blindcom();
    break;
  case VERB_PFLAGS:
    pflagcom();
    break;
  case VERB_FROB:
    frobnicate();
    break;
  case VERB_MAXUSERS:
    maxusercom();
    break;
  case VERB_SHUTDOWN:
    shutdowncom();
    break;
  case VERB_SPAWN:
    spawncom();
    break;
  case VERB_EMOTE:
    emotecom();
    break;
  case VERB_EMOTETO:
    emotetocom();
    break;
  case VERB_EMPTY:
    emptycom();
    break;
  case VERB_TIME:
    timecom();
    break;
  case VERB_LOCATIONS:
    locdir();
    break;
  case VERB_TREASURES:
    treasurecom();
    break;
  case VERB_WAR:
    warcom();
    break;
  case VERB_PEACE:
    peacecom();
    break;
  case VERB_QUIET:
    if (plev(mynum) < LVL_WIZARD) {
      erreval();
      break;
    }
    bprintf("Quiet mode %s.\n", pxorflg(mynum, pfl(Quiet)) ? "on" : "off");
    break;
  case VERB_NOSHOUT:
    if (plev(mynum) < LVL_WIZARD) {
      erreval();
      break;
    }
    if (pxorflg(mynum, pfl(NoShout)))
      bprintf("From now on you won't hear shouts.\n");
    else
      bprintf("You can hear shouts again.\n");
    break;
  case VERB_COLOR:
    if (pxorflg(mynum, pfl(Color)))
      bprintf("Welcome to the wonderful world of \033[1;31mC\033[32mO\033[33mL\033[34mO\033[35mR\033[36m!\033[0m\n");
    else
      bprintf("Back to black and white.\n");
    break;
  case VERB_ALOOF:
    if (plev(mynum) < LVL_WIZARD) {
      erreval();
      break;
    }
    if (pxorflg(mynum, pfl(Aloof)))
      bprintf("From now on you'll ignore mortals' actions.\n");
    else
      bprintf("You feel less distant.\n");
    break;
  case VERB_SHOW:
    showitem();
    break;
  case VERB_WIZLOCK:
    wizlock();
    break;
  case VERB_WIZUNLOCK:
    wizunlock();
    break;
  case VERB_AWIZLOCK:
    awizlock();
    break;
  case VERB_AWIZ:
    awizcom();
    break;
  case VERB_FOLLOW:
    followcom();
    break;
  case VERB_LOSE:
    losecom();
    break;
  case VERB_CLS:
    cls();
    break;
  case VERB_ECHO:
    echocom();
    break;
  case VERB_ECHOALL:
    echoallcom();
    break;
  case VERB_ECHOTO:
    echotocom();
    break;
  case VERB_WISH:
    wishcom();
    break;
  case VERB_TRACE:
    tracecom();
    break;
  case VERB_START:
    startcom();
    break;
  case VERB_STOP:
    stopcom();
    break;
  case VERB_MOBILES:
      mobilecom();
      break;
  case VERB_UNVEIL:
      unveilcom();
      break;
  case VERB_BANG:
      if (plev(mynum) < LVL_WARLOCK)
          erreval();
      else
	  if (plev(mynum) >= LVL_WIZARD)
              broad("\001dA huge crash of thunder echoes through the land.\n\377");
          else if (pstr(mynum) >= 2)
          {
              broad("\001dA huge crash of thunder echoes through the land.\n\377");
	      setpstr(mynum, pstr(mynum) - 2);
	  }
	  else
          {
	      bprintf("You haven't the energy.\n");
	  }
      break;
  case VERB_TRANSLOCATE:
      transcom();
      break;
  case VERB_ALIAS:
    aliascom();
    return;
  case VERB_LFLAGS:
    show_lflags();
    break;
  case VERB_VERSION:
    bprintf(VERSION);
    bprintf("\n");
    break;

  default:
    if (plev(mynum) < LVL_GOD)
      bprintf("You can't do that now.\n");
    else
      bprintf("Oh great one, truly I apologize, but I do not understand.\n");
    break;
  }
}

void
aliascom()
{
  if (plev(mynum) < LVL_WIZARD || aliased)
    erreval();
  else
    {
      if (pl1 == -1)
	{
	  bprintf("Who?\n");
	  error();
	  return;
	}
      if (polymorphed != -1)
	{
	  bprintf("A mysterious force stops you.\n");
	  error();
	  return;
	}
      if (ptstflg(pl1, pfl(Possessed)))
	{
	  bprintf("Already occupied!\n");
	  error();
	  return;
	}
      if (pl1 < MAX_USERS && plev(mynum) < LVL_ARCHWIZARD)
	{
	  bprintf("You can't become another player!\n");
	  error();
	  return;
	}
      if (aliased)
	pclrflg(mynum, pfl(Possessed));
      if (!aliased)
	{
	  tmynum = mynum;
	  cms = -2;
	  update();
	}
      mynum = pl1;
      cms = -1;
      update();
      bprintf("Aliased to %s.\n", pname(pl1));
      aliased = True;
      psetflg(mynum, pfl(Possessed));
      strcpy(o_in_ms, in_ms);
      strcpy(o_out_ms, out_ms);
      sprintf(in_ms, "has arrived.");
      sprintf(out_ms, "has gone %%s.");
    }
}

void eorte()
{
  static time_t last_io_interrupt;
  time_t ctm;

  if (EMPTY(pname(mynum))) {
/*    zapped = True; */
    crapup("You have timed out.");
  }
  if (snoopt != -1 && EMPTY(pname(snoopt))) {
    snoopt = -1;
    bprintf("You can no longer snoop on %s.\n", sntn);
  }
  time(&ctm);
  interrupt = 0;
  if (ctm - last_io_interrupt > 2)
    interrupt = 1;
  if (interrupt)
    last_io_interrupt = ctm;
  if (me_ivct)
    me_ivct--;
  if (me_ivct == 1)
    setpvis(mynum, 0);
  if (interrupt)
    dointerrupt();
  dostatus();
  if (me_cal)
    {
      me_cal = False;
      calibme();
    }
  if (tdes)
    dosumm(ades);
  if (in_fight)
    {
      if (ploc(fighting) != ploc(mynum))
	{
	  fighting = -1;
	  in_fight = 0;
	  setpfighting(mynum, -1);
	}
      if (EMPTY(pname(fighting)))
	{
	  fighting = -1;
	  in_fight = 0;
	  setpfighting(mynum, -1);
	}
      if (in_fight && interrupt)
	{
	  in_fight = 0;
	  hitplayer(fighting, pwpn(mynum));
	}
    }
  if (iswornby(OBJ_RING, mynum) || randperc() < 20
      || (psitting(mynum) && randperc() < 40))
    {
      if (!in_fight && interrupt)
	{
	  setpstr(mynum, pstr(mynum) + 1);
	  if (i_setup)
	    calibme();
	}
    }
  if (polymorphed == 0)
    unpolymorph();
  if (polymorphed != -1)
    polymorphed--;
  forchk();
  interrupt = 0;
  if (ploc(mynum) == RM_CASTLE18 && state(OBJ_DOOR_4) == 1)
    {
      loseme();
      broad("\001dThere is a long drawn out scream in the distance\377");
      crapup("The iron maiden closes....... S Q U I S H !");
    }
  if (plev(mynum) < LVL_WIZARD && ltstflg(ploc(mynum), lfl(OnWater)))
    {
      if (!iscarrby(OBJ_RAFT, mynum) && !iscarrby(OBJ_BOAT, mynum))
	{
	  loseme();
	  crapup("You plunge beneath the waves....");
	}
    }
  on_follow();
  if (pstr(mynum) < 0 && (aliased || polymorphed != -1))
    {
      bprintf("You've just died.\n");
      if (polymorphed != -1)
	unpolymorph();
      else
	unalias();
    }
}

void rescom()
{
  FILE *a;
  int d, et;

  if (plev(mynum) < LVL_WIZARD)
    {
      erreval();
      return;
    }

  et = gametime();
  if ((et > 0) && (et < RESET_TIME * 60) && (plev(mynum) < LVL_GOD))
    {
      bprintf("Resets are not allowed until %d minutes game time.\n", RESET_TIME);
      return;
    }

  mudlog("RESET by %s", pname(mynum), "");
  broad("\001A\033[31m\377Reset in progress....\nReset Completed....\001A\033[0m\377\n");
  a = openlock(RESET_DATA, "r");
  sec_read(a, objinfo, 0, o_ofs(numobs));
  closelock(a);
  a = fopen(RESET_N, "w");

  /*  WARNING!!  The following code will break on a system where time_t is not an int. */
  fprintf(a, "%d\n", (int)time(NULL));

  fclose(a);
  resetplayers();
}

void eatcom()
{
  int b;
  char s[100];

  if (brkword() == -1) {
    bprintf("Eat what?\n");
    return;
  }
  if (EQ(wordbuf, "water"))
    strcpy(wordbuf, "spring");
  if ((b = ob1 == -1 ? ob2 : ob1) == -1) {
    bprintf("It isn't here.\n");
    return;
  }
  switch (b) {
  case OBJ_TART:
  case OBJ_CAKES:
  case OBJ_TOAST:
    bprintf("That was delicious, but not very filling.\n");
    eat(b);
    setpstr(mynum, pstr(mynum) + 6);
    calibme();
    break;
  case OBJ_SOUP:
    bprintf("As you finish off the last of the caterpillar consume\n");
    bprintf("you notice a small diamond in the bottom of the cup.\n");
    eat(b);
    create(OBJ_DIAMOND_1);
    setoloc(OBJ_DIAMOND_1, mynum, 1);
    create(OBJ_CUP_1);
    setoloc(OBJ_CUP_1, mynum, 1);
    setpstr(mynum, pstr(mynum) + 6);
    calibme();
    break;
  case OBJ_CAULDRON:
    bprintf("You feel funny and pass out....\n");
    bprintf("You wake up elsewhere....\n");
    teletrap(-1076);
    break;
  case OBJ_SPRING:
    bprintf("Very refreshing.\n");
    break;
  case OBJ_POTION:
    setpstr(mynum, 1000);
    calibme();
    bprintf("You feel much much stronger!\n");
    setoloc(OBJ_POTION, RM_DEAD2, 0);
    destroy(OBJ_POTION);
    break;
  case OBJ_WAYBREAD:
  case OBJ_WAYBREAD_1:
  case OBJ_MUSHROOM:
    if (plev(mynum) < LVL_WIZARD && !aliased && polymorphed == -1)
      {
	pl1 = (rand() >> 3) % (MAX_CHARS - 1);
	if (ptstflg(pl1, pfl(Possessed)) || pstr(pl1) < 0 || pl1 < MAX_USERS)
	  {
	    bprintf("There is a sudden feeling of failure...\n");
	    break;
	  }
	polymorphed = 25;	/* aliased for 25 moves */
	tmynum = mynum;
	cms = -2;
	update();
	mynum = pl1;
	cms = -1;
	update();
	bprintf("You pass out.....\n..... and wake up.\n");
	psetflg(mynum, pfl(Possessed));
      }
    setpstr(mynum, pstr(mynum) + 16);
    eat(b);
    break;
  case OBJ_FOUNTAIN_1:
    if (plev(mynum) < 3) {
      setpscore(mynum, pscore(mynum) + 40);
      calibme();
      bprintf("You feel a wave of energy sweeping through you.\n");
    }
    else {
      bprintf("Faintly magical by the taste.\n");
      if (pstr(mynum) < 8)
	setpstr(mynum, pstr(mynum) + 2);
      calibme();
    }
    break;
  default:
    if (otstbit(b, ofl(Food))) {
      eat(b);
      bprintf("Delicious!\n");
      setpstr(mynum, pstr(mynum) + 12);
      calibme();
      sprintf(s, "\001P%s\377 greedily devours the %s.\n", pname(mynum), oname(b));
      sillycom(s);
    }
    else {
      bprintf("I think I've lost my appetite.\n");
      return;
    }
    break;
  }
}

/*
**  Routine to update player in user file
*/
void calibme()
{
  int b;
  char sp[128];

  if (!i_setup || aliased || polymorphed != -1)
    return;
  if ((b = levelof(pscore(mynum))) != plev(mynum) && plev(mynum) > 0) {
    setplev(mynum, b);
    bprintf("You are now ");
    displevel(b, psex(mynum), pname(mynum));
    bprintf("\n");
    mudlog("%s to level %d", pname(mynum), b);
    sprintf(sp, "[\001p%s\377 is now level %d]\n", pname(mynum), plev(mynum));
    sendsys(pname(mynum), pname(mynum), SYS_WIZ, ploc(mynum), sp);
    switch (b) {
    case LVL_CONJURER:
      bprintf("\nWelcome, Conjurer.  You may now use the SUMMON command.\n");
      break;
    case LVL_MAGICIAN:
      bprintf("\nWelcome, Magician.  You may now use the POSE command.\n");
      break;
    case LVL_WARLOCK:
      bprintf("\nNice work, Warlock.  You may now use the BANG command.\n");
      break;
    case LVL_NECROMANCER:
      bprintf("\nCongratulations Necromancer!  You may now use TRANSLOCATE.\n");
      bprintf("In addition, you may now use EMOTE anywhere.\n");
      break;
    case LVL_WIZARD:
      bprintf("\001f%s\377", GWIZ);
      break;
    }
  }
  b = maxstrength(mynum);
  if (pstr(mynum) > b)
    setpstr(mynum, b);
}

int levelof(int score)
{
  int i;

  if (plev(mynum) > LVL_WIZARD || plev(mynum) < LVL_NOVICE)
    return plev(mynum);
  for (i = LVL_WIZARD; i > LVL_GUEST; i--)
    if (score >= levels[i])
      return i;
  return 0;
}

void getreinput(char *blob)
{
  while (NOISECHAR(strbuf[stp]))
    stp++;
  strcpy(blob, &strbuf[stp]);
}

void shoutcom()
{
  char blob[200];

  if (plev(mynum) < LVL_WIZARD && ptstflg(mynum, pfl(NoShout))) {
    bprintf("I'm sorry, you can't shout anymore.\n");
    error();
  }
  getreinput(blob);
  if (EMPTY(blob)) {
    bprintf("What do you want to shout?\n");
    error();
  }
  if (plev(mynum) >= LVL_WIZARD)
    sendsys(pname(mynum), pname(mynum), SYS_WIZ_SHOUT, ploc(mynum), blob);
  else if (pstr(mynum) >= 2) {
    sendsys(pname(mynum), pname(mynum), SYS_SHOUT, ploc(mynum), blob);
    setpstr(mynum, pstr(mynum) - 2);
  }
  else {
    bprintf("You gasp for breath, but haven't the energy to shout.\n");
    return;
  }
}

void
tellcom()
{
  int b;

  if (EMPTY(item1)) {
    bprintf("Tell who?\n");
    return;
  }
  if ((b = pl1) == -1) {
    bprintf("No one with that name is playing.\n");
    error();
  }
  if (b == mynum) {
    bprintf("You talk to yourself.\n");
    return;
  }
  if (EMPTY(txt2)) {
    bprintf("What do you want to tell them?\n");
    error();
  }
  sendsys(pname(b), pname(mynum), SYS_TELL, ploc(mynum), txt2);
/*
  if (b == MOB_SERAPH) {
    if (index(txt2, '?')) {
      switch (rand() % 4) {
      case 0: sprintf(txt2, "Charity");
	break;
      case 1: sprintf(txt2, "Faith");
	break;
      case 2: sprintf(txt2, "Wisdom");
	break;
      case 3: sprintf(txt2, "Courage");
	break;
      }
    }
    else
      sprintf(txt2, "A blessing be upon your house.");
    sendsys(pname(mynum), pname(b), SYS_TELL, ploc(mynum), txt2);
  }
*/
}

void scorecom()
{
  int p;

  if (mynum >= MAX_USERS)
    bprintf("Your strength is %d.\n", pstr(mynum));
  else {
    p = (int) (pstr(mynum) * 10 / maxstrength(mynum));
    bprintf("You ");
    switch (p) {
    case 10:
    case 9: bprintf("are in exceptional health.");
      break;
    case 8: bprintf("are in better than average condition.");
      break;
    case 7: bprintf("feel a bit dazed.");
      break;
    case 6:
    case 5: bprintf("have minor cuts and abrasions.");
      break;
    case 4: bprintf("have some wounds, but are still fairly strong.");
      break;
    case 3: bprintf("are seriously wounded.");
      break;
    case 2: bprintf("are mortally wounded.");
      break;
    case 1:
    case 0: bprintf("are near death.");
    }
    bprintf("\n");
    bprintf("Your strength is %d (from %d), your score is %d\n",
	    pstr(mynum), maxstrength(mynum), pscore(mynum));
    bprintf("This ranks you as ");
    displevel(plev(mynum), psex(mynum), pname(mynum));
    bprintf("\n");
  }
}

void exorcom()
{
  int x;

  if (EMPTY(item1)) {
    bprintf("Exorcise who?\n");
    error();
  }
  if ((x = pl1) == -1) {
    bprintf("They aren't playing.\n");
    error();
  }
  if (plev(mynum) < LVL_ARCHWIZARD && plev(x) >= LVL_ARCHWIZARD) {
    bprintf("You can't exorcise them.\n");
    return;
  }
  mudlog("%s exorcised %s", pname(mynum), pname(x));
  dumpstuff(x, ploc(x));
  sendsys(pname(x), pname(mynum), SYS_EXORCISE, ploc(mynum), "");
  pname(x)[0] = 0;
  if (x == mynum)
    crapup("You have been exorcised from the game");
}

void givecom()
{
  int a, c;

  if (EMPTY(item1)) {
    bprintf("Give what to who?\n");
    error();
  }
  if (pl1 != -1) {
    if ((a = pl1) == -1) {
      bprintf("Who's that?\n");
      error();
    }
    if (mynum == a) {
      bprintf("Cheap skate!\n");
      error();
    }
    if (EMPTY(item2)) {
      bprintf("Give them what?\n");
      error();
    }
    if ((c = ob2) == -1) {
      bprintf("You don't have it.\n");
      error();
    }
    dogive(c, a);
    return;
  }
  if ((a = ob1) == -1) {
    bprintf("You don't have it.\n");
    error();
  }
  if (EMPTY(item2)) {
    bprintf("To who?\n");
    error();
  }
  if ((c = pl2) == -1) {
    bprintf("Who's that?\n");
    error();
  }
  if (mynum == c) {
    bprintf("Cheap skate!\n");
    error();
  }
  dogive(a, c);
}

void dogive(int ob, int pl)
{
  char z[60];
  int i, o;

  if (plev(mynum) < LVL_WIZARD && ploc(pl) != ploc(mynum)) {
    bprintf("They aren't here.\n");
    return;
  }
  if (!iscarrby(ob, mynum)) {
    bprintf("You don't have it.\n");
    return;
  }
  if (!cancarry(pl)) {
    bprintf("They can't carry it.\n");
    return;
  }
  if (pl == MOB_HERMIT && otstbit(ob, ofl(Food))) {
    bprintf("The Hermit thanks you.\n");
    sendsys("", "", SYS_TXT_TO_R, ploc(mynum), "The Hermit has left the game.\n");
    setpscore(mynum, pscore(mynum) + 50);
    dumpstuff(pl, ploc(pl));
    strcpy(pname(pl), "");
    eat(ob);
    return;
  }
  if (pl == MOB_BEGGAR && otstbit(ob, ofl(Food))) {
    bprintf("The Beggar thanks you and greedily devours the %s.\n", oname(ob));
    setpscore(mynum, pscore(mynum) + 50);
    eat(ob);
    bprintf("After finishing his meal, the beggar stares at you and says '");
    switch (rand() % 4) {
    case 0: bprintf("Charity");
      o = OBJ_PIT_1;
      break;
    case 1: bprintf("Faith");
      o = OBJ_PIT_2;
      break;
    case 2: bprintf("Wisdom");
      o = OBJ_PIT_3;
      break;
    case 3: bprintf("Courage");
      o = OBJ_PIT_4;
      break;
    }
    bprintf("'\n");
    for (i = OBJ_PIT_1; i <= OBJ_PIT_4; i++)
      setobjstate(i, 0);
    setobjstate(o, 1);
    sendsys(pname(mynum), pname(mynum), SYS_TXT_TO_R, oloc(o),
	    "A faint glow emanates from the pit.\n");
    return;
  }
  if (pl == MOB_VIOLA && ob == OBJ_FAN) {
    bprintf("Viola kisses you%s.\n", psex(mynum) == 0 ? "" : " on the cheek");
    bprintf("Viola says 'Thank you, %s.  Won't you please come in?'\n",
	    psex(mynum) == 0 ? "kind sir" : "madame");
    setpscore(mynum, pscore(mynum) + 50);
  }
  if (ob == OBJ_ROSE && psex(mynum) != psex(pl)) {
    bprintf("You give %s the rose.  It suddenly disappears!\n",
	    psex(pl) == 0 ? "him" : "her");
    setpscore(mynum, pscore(mynum) + 60);
    setpscore(pl, pscore(pl) + 50);
    destroy(OBJ_ROSE);
    sprintf(z, "\001p%s\377 gives you the %s.\n", pname(mynum), oname(ob));
    sendsys(pname(pl), pname(mynum), SYS_MSG_TO_P, ploc(mynum), z);
    sprintf(z, "It vanishes!\n");
    sendsys(pname(pl), pname(mynum), SYS_MSG_TO_P, ploc(mynum), z);
    return;
  }
  setoloc(ob, pl, 1);
  sprintf(z, "\001p%s\377 gives you the %s.\n", pname(mynum), oname(ob));
  sendsys(pname(pl), pname(mynum), SYS_MSG_TO_P, ploc(mynum), z);
  return;
}

void stealcom()
{
  int a, c, e, f;
  char x[128], tb[128];

  if (EMPTY(item1)) {
    bprintf("Steal what?\n");
    return;
  }
  strcpy(x, item1);
  if (EMPTY(item2)) {
    bprintf("From who?\n");
    return;
  }
  if ((c = pl2) == -1) {
    bprintf("Who is that?\n");
    return;
  }
  if (mynum == c) {
    bprintf("A true kleptomaniac.\n");
    return;
  }
  if ((a = fobncb(x, c)) == -1) {
    bprintf("They don't have it.\n");
    return;
  }
  if (plev(mynum) < LVL_WIZARD && ploc(c) != ploc(mynum)) {
    bprintf("They're not here!\n");
    return;
  }
  if (ocarrf(a) == 2) {
    bprintf("They're wearing it.\n");
    return;
  }
  if (pwpn(c) == a) {
    bprintf("They have it firmly to hand ... for KILLING people with!\n");
    return;
  }
  if (c == MOB_THOR) {
	bprintf("Thor simply grins wildly at you.\n");
	bprintf("You get the feeling he is challenging you.\n");
	return;
  }
  if (c == MOB_JOEPA) {
	bprintf("Joepa frowns and tells you to take a shower.\n");
	return;
  }
  if (c == MOB_VIOLA) {
	bprintf("You can't break her tight grip.\n");
	return;
  }
  if (c == MOB_HERMIT) {
    bprintf("He is too watchful.\n");
    return;
  }
  if (c == MOB_DRAGON || c == MOB_CHICKEN || c == MOB_PIPER) {
  bprintf("He is too alert.\n");
    return;
  }
  if (c == MOB_COSIMO) {
    bprintf("He is too crafty.\n");
    return;
  }
  if (c == MOB_DEFENDER) {
    sprintf(tb, "How dare you steal from me, %s!", pname(mynum));
    sendsys("The Defender", "The Defender", SYS_SAY, ploc(c), tb);
    woundmn(c, 0);
    return;
  }
  if (!cancarry(mynum)) {
    bprintf("You can't carry any more.\n");
    return;
  }
  f = randperc();
  e = (10 + plev(mynum) - plev(c)) * 5;
  if (f < e) {
    sprintf(tb, "\001p%s\377 steals the %s from you!", pname(mynum), oname(a));
    bprintf("Got it!\n");
    sendsys(pname(c), pname(mynum), SYS_MSG_TO_P, ploc(mynum), tb);
    if ((f & 1) && (c >= MAX_USERS))
      woundmn(c, 0);
    setoloc(a, mynum, 1);
    return;
  }
  bprintf("Your attempt fails.\n");
}

void updcom()
{
  char x[128];
  
  if (polymorphed != -1) {
    bprintf("Not while polymorphed.\n");
    return;
  }
  if (aliased) {
    bprintf("Not when aliased.");
    return;
  }
  sprintf(x, "[%s has updated]\n", pname(mynum));
  sendsys(pname(mynum), pname(mynum), SYS_WIZ, 0, x);
  sprintf(x, "%s", pname(mynum));
  loseme();
  closeworld();
  bflush();
  keysetback();
  execl(EXE_FILE, EXE_FILE, x, "-u", (char *)NULL);
  fprintf(stderr,"%s: Couldn't execute \"%s\" (%s).\n",
	  exe_file, EXE_FILE, sys_errlist[errno]);
}

void becom()
{
  char x[128];

  if (polymorphed != -1 || EMPTY(item1)) {
    bprintf("Become what?  Inebriated?\n");
    return;
  }
  if (aliased) {
    bprintf("Not when aliased.");
    error();
    return;
  }
  sprintf(x, "[%s has quit, via BECOME]\n", pname(mynum));
  sendsys("", "", SYS_WIZ, 0, x);
  mudlog("%s has BECOME %s", pname(mynum), item1);
  keysetback();
  loseme();
  closeworld();
  sprintf(x, "%s", item1);
  bflush();
  keysetback();
  execl(EXE_FILE, EXE_FILE, x, "-b", item1, (char *)NULL);
  fprintf(stderr,"%s: Couldn't execute \"%s\" (%s).\n",
	  exe_file, EXE_FILE, sys_errlist[errno]);
}

void
convcom()
{
  convflg = 1;
  bprintf("Type '*help conv' for help.\n");
}

void rawcom()
{
  char x[100], y[100];

  if (!ptstflg(mynum, pfl(Operator))) {
    erreval();
    return;
  }
  getreinput(x);
  sprintf(y, "%s%s%s", "** SYSTEM : ", x, "\n\007\007");
  broad(y);
}

void rollcom()
{
  int a;

  if (ohereandget(&a) == -1)
    return;
  switch (a) {
  case OBJ_PILLAR:
  case OBJ_PILLAR_1:
    gamecom("push pillar");
    break;
  case OBJ_BOULDER:
    gamecom("push boulder");
    break;
  default:
    bprintf("You can't roll that.\n");
  }
}

void bugcom()
{
  char x[120];

  getreinput(x);
  if (EMPTY(txt1))
    {
      bprintf("What do you want to bug me about?\n");
      return;
  }
  addtolog(BUG_LOG, "%s: %s", pname(mynum), x);
}

void typocom()
{
  char x[120], y[80];

  sprintf(y, "%s in %d", pname(mynum), ploc(mynum));
  getreinput(x);
  if(EMPTY(txt1))
    {
      bprintf("What typo do you wish to inform me of?\n");
      return;
  }
  addtolog(BUG_LOG, "%s: %s", y, x);
}

void
lookcom()
{
  int a, brhold;

  if (brkword() == -1)
    {
      brhold = ptstflg(mynum, pfl(Brief)) ? 1 : 0;
      pclrflg(mynum, pfl(Brief));
      lookin(ploc(mynum));
      if (brhold)
	psetflg(mynum, pfl(Brief));
      return;
    }
  if (EQ(wordbuf, "at"))
    {
      /* is this due to a parser error? ("at" is in item1, same for ob1) */
      strcpy(item1, item2);
      ob1 = ob2;
      examcom();
      return;
    }
  if (!EQ(wordbuf, "in") && !EQ(wordbuf, "into"))
    {
      bprintf("Are you trying to LOOK, LOOK AT, or LOOK IN something?\n");
      return;
    }
  if (EMPTY(item2))
    {
      bprintf("In what?\n");
      return;
    }
  if ((a = ob2) == -1)
    {
      bprintf("What?\n");
      return;
    }
  if (a == OBJ_TUBE || a == OBJ_BEDDING  || a == OBJ_COVER)
    {
      strcpy(item1, item2);
      ob1 = ob2;
      examcom();
      return;
    }
  if (!otstbit(a, ofl(Container)))
    {
      bprintf("That isn't a container.\n");
      return;
    }
  if (otstbit(a, ofl(Openable)) && state(a))
    {
      bprintf("It's closed!\n");
      return;
    }
  bprintf("The %s contains:\n", oname(a));
  aobjsat(a, 3, 8);
}

void
unveilcom()
{
  char *ptr;

  if (!isawiz)
    {
      erreval();
      return;
    }
  psetflg(mynum, pfl(PflagEdit));
  if (brkword() == -1) 
    {
      bprintf("You are now seen in Godlike splendour!\n");
      setplev(mynum, LVL_GOD);
      return;
    }
  for (ptr = wordbuf; *ptr; ptr++)
    {
      if (*ptr == '-')
        {
	  bprintf("I hope you know what you're doing!\n");
	  ptr++;
        }
      if (!isdigit(*ptr))
        {
	  bprintf("That's not a number!\n");
	  return;
        }
    }
  bprintf("You are now seen in your true from!\n");
  setplev(mynum, atoi(wordbuf));
}

void
emptycom()
{
  int a, b;
  char x[81];

  if ((b = ohereandget(&a)) == -1)
    return;
  for (b = 0; b < numobs; b++)
    {
      if (iscontin(b, a))
	{
	  setoloc(b, mynum, 1);
	  bprintf("You empty the %s from the %s.\n", oname(b), oname(a));
	  sprintf(x, "drop %s", oname(b));
	  gamecom(x);
	  pbfr();
	  openworld();
	}
    }
}

void unalias()
{
  aliased = False;
  pclrflg(mynum, pfl(Possessed));
  mynum = tmynum;
  bprintf("Returning to self....\n");
  cms = -1;
  in_fight = 0;
  fighting = -1;
  strcpy(in_ms, o_in_ms);
  strcpy(out_ms, o_out_ms);
  update();
}

void unpolymorph()
{
  if (polymorphed == -1) return;
  polymorphed = -1;
  pclrflg(mynum, pfl(Possessed));
  mynum = tmynum;
  bprintf("Suddenly you awake... were you dreaming or what?\n");
  cms = -1;
  in_fight = 0;
  fighting = -1;
  update();
}

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include "kernel.h"
#include "macros.h"
#include "pflags.h"
#include "lflags.h"
#include "tk.h"
#include "support.h"
#include "opensys.h"
#include "parse.h"
#include "bprintf.h"
#include "mud.h"
#include "key.h"
#include "blib.h"
#include "weather.h"
#include "objsys.h"
#include "uaf.h"
#include "zones.h"
#include "loctable.h"
#include "locations.h"

void vcpy(int *dest, int *source, int len)
{
  while (--len >= 0)
    *dest++ = *source++;
}

void mstoout(int *block)
{
  /* Print appropriate stuff from data block */
  if (block[1] < -3)
    gamrcv(block);
  else
    bprintf("%s", (char *)&block[2]);
}

int sendamsg(char *name)
{
  char prmpt[32];
  char work[200];
  char w2[200];

  for ( ; ; ) {
    strcpy(prmpt, "");
    if (pvis(mynum))
      strcat(prmpt, "(");
    if (aliased)
      strcat(prmpt, "@");
    strcat(prmpt, prm_str);
    switch (convflg) {
    case 0:
      break;
    case 1:
      prmpt[strlen(prmpt) - 1] = 0;
      strcat(prmpt, "\"");
      break;
    case 2:
      prmpt[strlen(prmpt) - 1] = 0;
      strcpy(prmpt, "MSG> ");
      break;
    default:
      prmpt[strlen(prmpt) - 1] = 0;
      strcat(prmpt, "->");
    }
    if (pvis(mynum))
      strcat(prmpt, ")");
    sprintf(work, exe_file, name);
    sig_alon();
    key_input(prmpt, 80);
    sig_aloff();
    strcpy(work, key_buff);
    bprintf("\001l%s%s\n\377", prmpt, work);
    rte();
    closeworld();
    if (convflg && EQ(work, "**")) {
      if (convflg == 2)
	fclose(mail_fl);
      convflg = 0;
    }
    else
      break;
  }
  if (!EMPTY(work)) {
    if (work[0] == '\"' || work[0] == '\'') {
      strcpy(w2, work);
      sprintf(work, "say %s", w2 + 1);
    }
    if (work[0] == '/') {
      strcpy(w2, work);
      sprintf(work, "emote %s", w2 + 1);
    }
    if (!EQ(work, "*") && work[0] == '*')
      work[0] = 32;
    else if (convflg == 1) {
      strcpy(w2, work);
      sprintf(work, "say %s", w2);
    }
    else if (convflg == 2) {
      fprintf(mail_fl, "   %s\n", work);
      return -1;
    }
  }
  if (curmode == 1)
    gamecom(work);
  if (fighting > -1) {
    if (EMPTY(pname(fighting))) {
      in_fight = 0;
      fighting = -1;
      setpfighting(mynum, -1);
    }
    if (ploc(fighting) != ploc(mynum)) {
      in_fight = 0;
      fighting = -1;
      setpfighting(mynum, -1);
    }
  }
  if (in_fight)
    in_fight -= 1;
  return -1;
}

void send2(int *block)
{
  FILE *unit;
  int number;
  int inpbk[128];

  if (spawned)
    return;
  if ((unit = openworld()) == NULL) {
    loseme();
    crapup("\nAberMUD: FILE_ACCESS : Access failed\n");
  }
  sec_read(unit, inpbk, 0, 64);
  number = 2 * inpbk[1] - inpbk[0];
  inpbk[1]++;
  sec_write(unit, block, number, 128);
  sec_write(unit, inpbk, 0, 64);
  if (number >= 199)
    cleanup(inpbk);
  if (number >= 199)
    longwthr();
}

void readamsg(FILE *channel, int *block, int num)
{
  int buff[64], actnum;

  sec_read(channel, buff, 0, 64);
  actnum = num * 2 - buff[0];
  sec_read(channel, block, actnum, 128);
}

void rte()
{
  FILE *unit;
  int too, ct, block[128];

  if (spawned)
    return;
  if ((unit = openworld()) == NULL)
    crapup("AberMUD: FILE_ACCESS : Access failed\n");
  too = findend(unit);
  if (cms == -1)
    cms = too;
  if (cms < findstart(unit) / 2) {
/*
**  Attempt to solve the timeout bug which was caused when old processes
**  are not killed.  crapup() was killing *all* active processes, instead
**  of the offending one.  exit() should solve this -- Rassilon
*/
    loseme();
    bprintf("\nYou've been idle too long.\n");
    exit();
  }
  pbfr();
  pr_qcr = True;
  for (ct = cms; ct < too; ct++) {
    readamsg(unit, block, ct);
    mstoout(block);
  }
  cms = ct;
  update();
  eorte();
  tdes = 0;
  if (iamon && snoopt != -1)
    viewsnoop();
  bflush();
}

int
findstart(FILE *unit)
{
  int bk[2];

  sec_read(unit, bk, 0, 1);
  return bk[0];
}

int
findend(FILE *unit)
{
  int bk[3];

  sec_read(unit, bk, 0, 2);
  return bk[1];
}

void
talker(char *name)
{
    char msg[80], *hname;
    struct sockaddr_in addr;
    struct hostent *host;
    int len;
    extern int errno;

    len = sizeof(addr);
    if (getpeername(fileno(stdin), (struct sockaddr *)&addr, &len) >= 0) {
	if (host = gethostbyaddr((char *)&addr.sin_addr, sizeof(struct in_addr),
				 addr.sin_family)) {
	    hname = host->h_name;
	} else {
	    hname = inet_ntoa(addr.sin_addr);
	}
    } else {
	mudlog("error = %d", errno);
	hname = "<direct>";
    }
    cms = -1;
    putmeon(name);
    if (openworld() == NULL)
	crapup("Sorry, AberMUD is currently unavailable.");
    if (mynum == -1) {
	bprintf("\nSorry, AberMUD is full at the moment.\n");
	exit();
    }
    strcpy(globme, name);
    rte();
    closeworld();
    cms = -1;
    entergame(name, hname);
    mudlog("GAME ENTRY: %s [level %d] [from %s]", name, my_lev, hname);
    if (shimge[1] && my_lev < shimge[1]) {
	loseme();
	sprintf(msg, "I'm sorry, the game is currently %s locked--please try later.\n",
		(shimge[1] == 127) ? "ARCHWIZARD" : "WIZARD");
	crapup(msg);
    }
    if (ismonitored) {
	sprintf(msg, "DATA/logs/.%s", name);
	log_fl = fopen(msg, "a");
	if (log_fl != NULL)
	    fprintf(log_fl, "Commencing Logging of %s\n", name);
    }
    i_setup = True;
    while (True) {
	closeworld ();
	pbfr ();
	sendamsg (name);
	if (rd_qd)
	    rte ();
	rd_qd = False;
    }
}

void cleanup(int *inpbk)
{
  int *bk;
  FILE *unit;

  unit = openworld();
  bk = NEW(int, 1280);
  sec_read(unit, bk, 101, 1280);
  sec_write(unit, bk, 1, 1280);
  sec_read(unit, bk, 121, 1280);
  sec_write(unit, bk, 21, 1280);
  sec_read(unit, bk, 141, 1280);
  sec_write(unit, bk, 41, 1280);
  sec_read(unit, bk, 161, 1280);
  sec_write(unit, bk, 61, 1280);
  sec_read(unit, bk, 181, 1280);
  sec_write(unit, bk, 81, 1280);
  free((char *)bk);
  inpbk[0] += 100;
  sec_write(unit, inpbk, 0, 64);
  revise(inpbk[0]);
}

int entergame(char *name, char *hname)
{
  char xx[128];
  char xy[128];
  int startloc;

  curmode = 1;
  curch = RM_START1;
  initme();
  openworld();
  setpstr(mynum, my_str);
  setplev(mynum, my_lev);
  if (ptstflg(mynum, pfl(InvisStart)) && my_lev >= LVL_WIZARD)
    if (my_lev >= LVL_GOD)
      setpvis(mynum, LVL_GOD);
    else
      setpvis(mynum, (my_lev >= LVL_ARCHWIZARD) ? LVL_ARCHWIZARD : LVL_WIZARD);
  else
    setpvis(mynum, 0);
  setpwpn(mynum, -1);
  setpsexall(mynum, my_sex);
  setphelping(mynum, -1);
  setpsitting(mynum, 0);
  setpscore(mynum, my_sco);
  my_sex = my_sex & 1;
  fetchprmpt(my_lev);
  sprintf(xx, "\001s%s\377[%s [%s] has entered the game]\n\377", name, name,
	  hname);
  sendsys(name, name, -10113, ploc(mynum), xx);
  if (shimge[1])
    bprintf("The game is currently %s locked.\n",
	    shimge[1] == 127 ? "ARCHWIZARD" : "WIZARD");
  if (shimge[0])
    bprintf("Everything is peaceful.\n");
  rte();
  if (plev(mynum) < LVL_GOD) {
    if (randperc() > 50)
      trapch(RM_START1);
    else {
      curch = -5;
      trapch(RM_START2);
    }
  }
  else {
	curch = -1217;
	trapch(RM_HOME18);
  }
  if (plev(mynum) < LVL_GOD) {
    sprintf(xy, "\001s%s\377%s has entered the game.\n\377", name, name);
    sendsys(name, name, -10000, ploc(mynum), xy);
  }
  return 1;
}

void broad(char *mesg)
{
  char bk2[256];
  int block[128];

  strcpy(bk2, mesg);
  block[1] = -1;
  vcpy(&block[2], (int *)bk2, 126);
  rd_qd = True;
  send2(block);
}

char split(int *block, char *nam1, char *nam2, char *work, char *luser)
{
  char q[40];
  int wkblock[128];

  vcpy(wkblock, &block[2], 126);
  vcpy((int *)work, &block[64], 64);
  GetFields((char *)(block + 2), nam1, nam2);
  strcpy(q, nam1);
  return EQ(q, luser);
}

void trapch(int chan)
{
  openworld();
  setploc(mynum, chan);
  lookin(chan);
}

void putmeon(char *name)
{
  int ct;

  iamon = False;
  openworld();
  if ((ct = fpbns(name)) == -1) {
    if (master(name))			/* reserved slot for MASTERUSER */
      ct = 0;
    else {
      for (ct = 1; ct < shimge[4]; ct++)
	if (EMPTY(pname(ct)))
	  break;
      if (ct >= shimge[4]) {
	mynum = -1;
	return;
      }
      if (ct >= MAX_USERS)
         crapup("Sorry, you can't come in as a mobile.");
    }
  }
  else if (!master(name))
    crapup("You're already on the system...");
  strcpy(pname(ct), name);
  setploc(ct, curch);
  setppos(ct, -1);
  setplev(ct, 1);
  setpvis(ct, 0);
  setpstr(ct, -1);
  setpwpn(ct, -1);
  setpsex(ct, 0);
  setpsitting(ct, 0);
  setpfighting(ct, -1);
  mynum = ct;
  iamon = True;
}

void loseme()
{
  char bk[128];

  if (aliased || polymorphed != -1)
    return;
  if (mynum == -1)
    return;
  /* No interruptions while you are busy dying */
  sig_aloff();
  iamon = False;
  /* THIS BIT IS A CRITICAL SECTION - A USER TOTALLY CRASHING HERE */
  /* WILL GET STUCK ON THE USERLIST FOR ABOUT 2 MINUTES OR SO */
  i_setup = False;
  strcpy(bk, pname(mynum));
  openworld();
  if (EQ(globme, bk)) {
    dumpitems();
    if (plev(mynum) >= LVL_WIZARD && shimge[1])
      bprintf("\nDon't forget the game is locked....\n");
    if (pvis(mynum) < 10000) {
      sprintf(bk, "[%s has departed from AberMUD IV]\n", pname(mynum));
      if (!EMPTY(pname(mynum)))
	sendsys(pname(mynum), pname(mynum), -10113, 0, bk);
    }
    mudlog("GAME EXIT: %s [level %d]", pname(mynum), plev(mynum));
    setpfighting(mynum, -1);
    setpsitting(mynum, 0);
    setpvis(mynum, 0);
    if (!zapped)
      saveme();
    pname(mynum)[0] = 0;
  }
  if (log_fl)
    fclose(log_fl);
  if (mail_fl)
    fclose(mail_fl);
  snoop_off();
  /* We rely on world locking to assure no one nicks */
  /* our place when we leave before we save. */
  closeworld();
}

void update()
{
  int xp;

  if ((xp = cms - lasup) < 0)
    xp = -xp;
  if (xp >= 10) {
    (void)openworld();
    setppos(mynum, cms);
    lasup = cms;
  }
}

void revise(int cutoff)
{
  char mess[128];
  int ct;

  (void)openworld();
  for (ct = 0; ct < MAX_USERS; ct++) {
    if (pname(ct)[0] && ppos(ct) < cutoff / 2 && ppos(ct) != -2) {
      sprintf(mess, "[%s%s", pname(ct), " has timed out]\n");
      broad(mess);
      dumpstuff(ct, ploc(ct));
      pname(ct)[0] = 0;
    }
  }
}

void lookin(int room)
{
  int brmode, loc;

  loc = ploc(mynum);
  brmode = ptstflg(mynum, pfl(Brief));
  if (ptstflg(mynum, pfl(Blind))) {
    bprintf("You're blind, you can't see a thing!\n");
    onlook();
  }
  else {
    if (plev(mynum) >= LVL_WIZARD) {
      showname(room);
      if (roomdark())
	bprintf("[DARK]\n");
    }
    if (isdark()) {
      bprintf("It's dark.  Be careful or you may be eaten by a Grue!\n");
      onlook();
      return;
    }
    bprintf("%s\n", sdesc(loc));
  }
  if (ltstflg(loc, lfl(Death))) {
    if (plev(mynum) < LVL_WIZARD) {
      bprintf(" %s\n", ldesc(loc) + 1);
      loseme();
      crapup("You seem to have died...");
    }
    else
      bprintf("[DEATH]\n");
  }
  if ((plev(mynum) >= LVL_WIZARD) && ltstflg(loc, lfl(Private)))
    bprintf("[PRIVATE]\n");
  if ((plev(mynum) >= LVL_WIZARD) && ltstflg(loc, lfl(Party)))
    bprintf("[PARTY]\n");
  if (!ptstflg(mynum, pfl(Blind))) {
    if (!brmode)
      bprintf("%s", ldesc(loc));
    lisobs();
    if (curmode == 1)
      lispeople();
  }
  bprintf("\n");
  onlook();
}

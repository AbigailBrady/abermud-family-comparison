/* AberMUD 4.100.4
   Kernel definitions.
   Copyright (C) 1988 Alan Cox
   All Rights Reserved.
   
   Extensive UNIX mods by Rich Salz
   Major rewrite by Rassilon (Brian Preble). */

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include "levels.h"

/*
**  Global configuration section
*/

#define HOST_MACHINE	"psykoman.hut.fi"	/* Host machine name	     */
#define FLUSH_INPUT	80			/* Flush typeahead?	     */
#define MAX_USERS	20			/* Maximum users	     */
#define MAX_CHARS	140			/* Maximum users + mobiles   */
#define BSD_WAIT				/* BSD-style wait call?	     */
#define MASTERUSER(n)	(EQ(n, "jaf"))
#define RESET_TIME	60			/* Time before reset allowed */
#define NO_SUMMON	90			/* Mortals can't summon mobiles
						   for NO_SUMMON minutes     */
#define SNOOPFILE	"SNOOP/"		/* Snoop directory	     */
#define DESCFILE	"DESC/"			/* Character descriptions    */
#define MSGFILE		"MSGS/"			/* Messages file for players */
#define UNIVERSE	"world_file"		/* Game working image block  */

/*
**  Priviledge information
*/

#define BAN_FILE       "DATA/priv/banned"       /* List of banned usernames  */
#define	ILLEGAL_FILE   "DATA/priv/illegal"      /* Illegal player names      */
#define MONITOR_FILE   "DATA/priv/monitor"	/* Players to monitor	     */
#define PRIVED_FILE    "/home/fsf/daz/gnu/gnu.private"       /* Prived player names       */

/*
**  System files
*/

#define EXTERNS	       "DATA/system/actions"    /* Hug, kiss, tickle, ...    */
#define BOOTSTRAP      "DATA/system/bootstrap"	/* Points to text files      */
#define OBJECTS	       "DATA/system/objects"    /* Object names and desc.    */
#define RESET_DATA     "DATA/system/reset_data" /* The data needed to reset  */
#define RESET_N	       "DATA/system/reset_n"	/* Time of last reset        */

/*
**  Informational files for commands and conditions of the game
*/

#define BULLETIN       "DATA/info/bulletin"	/* Messages for wizards      */
#define COUPLES	       "DATA/info/couples"	/* Who's been married?       */
#define CREDITS	       "DATA/info/credits"	/* Author credits            */
#define FULLHELP       "DATA/info/fullhelp"	/* Detailed command desc.    */
#define HELP1	       "DATA/info/help1"        /* General helps for Players */
#define HELP2	       "DATA/info/help2"        /*    Wizards                */
#define HELP3	       "DATA/info/help3"        /*    Awizes                 */
#define HELP4	       "DATA/info/help4"        /*    Gods                   */
#define INFO	       "DATA/info/info"	        /* Brief information on game */
#define MOTD	       "DATA/info/motd"	        /* Message of the day        */
#define	MUDLIST	       "DATA/info/mudlist"	/* List of known AberMUDs    */
#define NEWS	       "DATA/info/news"	        /* AberMUD news              */
#define NOLOGIN	       "DATA/info/nologin"	/* If exists, no logins      */
#define GWIZ	       "DATA/info/gwiz"		/* "Welcome to Wizard" msg.  */

/*
**  Player data files 
*/

#define LEV_PRM	       "DATA/player/promptlist" /* Player's travel messages  */
#define LEVEL_FILE     "DATA/player/titles"	/* Player's titles           */
#define UAF_RAND       "DATA/player/uaf_rand"   /* User action file          */
#define PASSWORDFILE   "DATA/player/user_file"	/* Password file             */
#define PASSTEMP       "DATA/player/user_file.b"/* Temp password file        */

/*
**  System logs
*/

#define LOG_FILE       "DATA/logs/syslog"       /* The system log            */
#define BUG_LOG	       "BUGS"			/* Bug and typo log	     */

/*
**  End of configuration section.  DO NOT EDIT BELOW THIS LINE!
*/

typedef enum { False, True } Boolean;

extern char exe_file[];

#ifdef BSD_WAIT
#include <sys/wait.h>
typedef union wait	 WAITER;
#else
typedef int		 WAITER;
#endif /* BSD_WAIT */

#define NEW(t, c)		(t *)xmalloc((c) * sizeof (t))
#define COPY(s)			strcpy(NEW(char, strlen(s) + 1), s)
#define EMPTY(p)		(*(p) == '\0')
#define EQ(a, b)		(strcasecmp((a), (b)) == 0)

#define closelock(f)		((void)fclose(f))

typedef struct _LEVEL {
    int		level;
    char	f_name[50];
    char	m_name[50];
} LEVEL;

typedef struct _CONDITION {
    short	cnd_code;
    short	cnd_data;
} CONDITION;

typedef struct _LINE {
    short	verb;
    short	item1;
    short	item2;
    CONDITION	conditions[6];
    short	actions[4];
} LINE;

typedef struct _TABLE {
    char	*t_name;
    int		t_code;
} TABLE;

typedef struct _PERSONA {
    char	p_name[16];
    int		p_score;
    int		p_strength;
    int		p_sex;
    int		p_level;
} PERSONA;

typedef struct _LOCDATA {
    int		r_code;
    int		r_exit[6];
    short	r_flags;
    char	*r_short;
    char	*r_long;
} LOCDATA;

typedef struct _OBJECT {
    char	*o_name;
    char	*o_desc[4];
    long	o_examine;
    int		o_maxstate;
    int		o_value;
    int		o_flannel;
    char	*o_altname;
    int		o_size;
} OBJECT;

typedef struct _PLAYER {
    char	*p_name;
    int		p_loc;
    int		p_str;
    int		p_flags;
    int		p_lev;
} PLAYER;

typedef struct _ZONE {
    char	*z_name;
    int		z_loc;
} ZONE;

/*
** Global data.
** Data is declared everywhere, lives oncewhere.   Note the difference
** bewteen EXTERN and extern; "extern" data is initialized in main,
** EXTERN data is in BSS area, and thus set to (the right kind of) zero.
*/

#ifdef	DATA_DEF
#define EXTERN /* NULL */
#else
#define EXTERN extern
#endif

EXTERN FILE	*filrf;		/* NULL if not open	*/
EXTERN FILE	*log_fl;	/* NULL if no logging	*/
EXTERN FILE	*mail_fl;	/* NULL if not sending mail */
EXTERN LINE	event[];
EXTERN LINE	inter[];
EXTERN LINE	status[];
extern const LOCDATA	room_data[];
EXTERN OBJECT	*objects;
EXTERN PLAYER	*pinit;
EXTERN ZONE	*zoname;
EXTERN char	**messages;
EXTERN char	**pftxt;	/* Mobiles text table	*/
EXTERN char	**verbtxt;	/* Verb table */

EXTERN char	**action;	/* Action table */
EXTERN int	*action_flags;
EXTERN char	**action_all;
EXTERN char	**action_all_rsp;
EXTERN char	**action_to;
EXTERN char	**action_to_rsp;
EXTERN char	**action_msg_all;
EXTERN char	**action_msg_to;

extern char	*dirns[6];
EXTERN char	localname[32];
extern char	*txt1;
extern char	*txt2;
EXTERN char	acfor[128];
EXTERN char	globme[40];	/* Your name			*/
EXTERN char	item1[128];
EXTERN char	item2[128];
EXTERN char	key_buff[BUFSIZ];
extern char	min_ms[81];
extern char	mout_ms[81];
extern char	vin_ms[81];
extern char	vout_ms[81];
extern char	in_ms[81];
extern char	out_ms[81];
extern char	o_in_ms[81];	/* duplicate sets for aliased wizards */
extern char	o_out_ms[81];
extern char	prm_str[81];
extern char	t_username[81];
EXTERN int	levels[LVL_WIZARD + 1];
EXTERN Boolean	isawiz;		/* is this player a system's wizard?  */
EXTERN Boolean	ismonitored;	/* is this player being monitored?  */
EXTERN Boolean	candetach;	/* can this account use tss/detach?  */
EXTERN char	shimge[64];	/* global variables, PEACE, LOCK, etc.  */
EXTERN char	sntn[32];
EXTERN char	strbuf[128];
extern char	wd_her[16];
extern char	wd_him[16];
extern char	wd_them[16];
extern char	wd_it[16];
extern char	wordbuf[128];
EXTERN short	parameters[6];	/* Parameter store for PAR		*/
EXTERN int	*objinfo;
EXTERN int	*verbnum;
EXTERN int	ades;
EXTERN Boolean	aliased;	/* Are we aliased?			*/
EXTERN int      polymorphed;	/* if polymorphed then how many rounds? */
extern int	cms;		/* Current message			*/
EXTERN int	convflg;
EXTERN int	curch;
EXTERN int	curmode;	/* Mode data				*/
EXTERN int	in_fight;	/* Are we in a fight?			*/
extern int	fighting;	/* Who we are beating up		*/
EXTERN int	forf;
EXTERN int	interrupt;
EXTERN Boolean	iamon;		/* Am I on?				*/
EXTERN Boolean	isforce;
extern Boolean	iskb;
EXTERN Boolean	i_follow;	/* Am I following someone?		*/
EXTERN Boolean	i_setup;	/* Am I set up?				*/
EXTERN Boolean	me_cal;
EXTERN int	lasup;
EXTERN int	me_ivct;
extern int	mynum;
EXTERN int	my_lev;
EXTERN int	my_sco;
EXTERN int	my_sex;
EXTERN int	my_str;
EXTERN int	numchars;
EXTERN int	numobs;
EXTERN int	numactions;
EXTERN int      numzon;		/* Number of zones in the world		*/
extern int	ob1;
extern int	ob2;
extern int	pl1;
extern int	pl2;
EXTERN int	pptr;		/* The parameter pointer		*/
extern int	prep;
EXTERN Boolean	pr_due;		/* I/O redraw pending?			*/
EXTERN Boolean	key_mode;
EXTERN Boolean	pr_qcr;		/* CR queue				*/
EXTERN Boolean	tdes;		/* Teleport request in queue		*/
EXTERN Boolean	rd_qd;
extern int	snoopd;
extern int	snoopt;		/* Snoop control			*/
EXTERN Boolean	spawned;
EXTERN int	stp;
EXTERN int	sys_uid;	/* UserID of the game owner		*/
EXTERN int	userid;		/* UserID of the game player		*/
EXTERN int	ublock[16 * (MAX_CHARS + 1)];
EXTERN int	verbcode;
EXTERN Boolean	zapped;		/* Player dead, no save flag		*/

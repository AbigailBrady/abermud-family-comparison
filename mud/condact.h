#define P(n)		(10000 + (n))

/*
**  Words, etc.
*/
#define PAR		0
#define LEVEQ		1
#define LEVLT		2
#define IN		3
#define GOT		4
#define AVL		5
#define IFDEAF		6
#define IFDUMB		7
#define IFBLIND		8
#define IFCRIPPLED	9
#define IFFIGHTING	10
#define PFLAG		11
#define STATE		12
#define EQc		13
#define LT		14
#define GT		15
#define VISLT		17
#define VISEQ		18
#define CANSEE		19
#define STRLT		20
#define STREQ		21
#define ISIN		22
#define HERE		23
#define PLGOT		24
#define DESTROYED	25
#define CHANCE		26
#define OBIT		27
#define INLT		29
#define INGT		30
#define ISME		31
#define DARK		32
#define OBJAT		33
#define CHANCELEV	34
#define WIZ		35
#define AWIZ		36
#define PHERE           37
#define ISENT		38	/* Message transer block unimplemented */
#define PLWORN		39
#define TSTSEX		40
#define WPN		41
#define ENEMY		42
#define CANGO		44
#define PREP		45
#define ALONE		46
#define WILLFITIN	47
#define CANCARRY	48
#define ELAPSED		49	/* Unimplemented */
#define ISMONSTER	50
#define OBYTEEQ         51
#define OBYTEGT         52
#define OBYTEZ          53
#define HASANY          54
#define HELPED          55
#define OP              56
#define IFSITTING	57
#define COND		127
#define NOT		128

/*
**	Null to 0 not 0L
*/
#ifdef NULL
#undef NULL
#endif

#define NULL		0
#define GAIN		1
#define LOSE		2
#define WOUND		3
#define HEAL		4
#define INC		5
#define DEC		6
#define ADD		7
#define SUB		8
#define MUL		9
#define DIV		10
#define MOD		11
#define LET		12
#define CLR		13
#define SET		14
#define PUT		15
#define PUTIN		16
#define PUTCARR		17
#define DONE		18
#define DESTROY		19
#define QUIT		20
#define DIE		21
#define PUTWITH		22
#define GOBY		23
#define DISPROOM	24
#define OK		25
#define GOTO		26
#define SWAP		27
#define POBJ		28
#define PPLAYER		29
#define PNUMBER		30
#define SETLEVEL	31
#define SETVIS		32
#define SENDALL		33	/* Send functions not yet inplemented */
#define SENDPL		34
#define SENDRM		35
#define COPYFL		37
#define LOBJAT		38	/* all list actions unimplemented */
#define LOBJIN		39
#define LOBJCARR	40
#define CREATE		41
#define DOOBJS		42	/* unimplemented */
#define DOPLS		43	/* unimplemented */
#define PSENDER		44	/* unimplemented */
#define SETSTATE	45
#define PUTWORN		46
#define CHECK_NOLOGIN	47
#define CHECK_MAIL	48	/* check for mail */
/*
**    These kernel actions will be gradually replaced with database
**    stuff with no checking
*/
#define TELL		50
#define SAY		51
#define WIZACT		52
#define SHOUT		53
#define ARCH		54
#define RESET		55
#define CRASH		56
#define UNVEIL		57	/* unimplemented */
#define SAVE		58
#define BUG		59
#define TYPO		60
#define FLEE		61
#define MOVE		62
#define EXITS		63
#define BLIND		64
#define DEAF		65
#define DUMB		66
#define CRIPPLE		67
#define FORCE		68
#define LISTFILE	69
#define PRONOUNS	70
#define FIGHT		71
#define CURE		72
#define FROB		73
#define ZAP		74
#define WHO		75
#define USERS		76
#define PLAYERS		77
#define MOBILES		78
#define FOLLOW		79
#define EXORCISE	80
#define BECOME		81
#define DEBUGMODE	82	/* unimp */
#define EXAMINE		83
#define GOTOACT		84
#define LOOK		85
#define GET		86
#define DROP		87
#define WEAR		88
#define REMOVE		89
#define GIVE		90
#define STEAL		91
#define INVEN		92
#define SUMMON		93
#define WRITE		94	/* unimp */
#define SCORE		95
#define VALUE		96
#define STATS		97
#define SETACT		98
#define EMPTYc		99	/* unimp */
#define ROOMS		100
#define DIR		101
#define WEAPON		102
#define WHERE		103
#define PFLAGS		104
#define SNOOP		105	/* unimp */
#define LOG		106	/* unimp */
#define TIME		107
#define EXEC		111	/* unimp */
#define BRIEF		112
#define SETMIN		113
#define SETMOUT		114
#define SETIN		115
#define SETOUT		116
#define INACT		117
#define UPDATE		118
#define TRACE		119
#define SENDTEXT	120	/* unimp */
#define SENDPLTEXT	121	/* unimpo */
#define EMOTE		122
#define PATCH		123	/* unimp */
#define RMEDIT		124	/* unimp */
#define SHOW		125	/* unimp */
#define LOCNODES	126
#define DROPIN		127
#define GETOUT		128	/* unimp */
#define NORMAL		129	/* unimplemented - hardwired version */
#define BEG		130
#define SETSEX		131
#define ROOMNAME	132	/* unimp */
#define GETOLOC		133
#define GETPLOC		134
#define	FOB		135
#define DECOBYTE        136
#define INCOBYTE        137
#define SETOBYTE        138
#define PROVOKE         139
#define MOVEMON         140
#define WOUNDMON        141
#define KILL            142
#define BROAD           143
#define EAT		144
#define SETVIN		145
#define SETVOUT		146
#define SETPROMPT	147

#define AMASK		255
#define PL		(-32767)
#define OB		(-32766)
#define ANY		(-32765)
#define NONE		(-32764)

#define FL_PL1		20000
#define FL_PL2		20001
#define FL_OB1		20002
#define FL_OB2		20003
#define FL_CURCH	20004
#define FL_PREP		20005
#define FL_LEV		20006
#define FL_STR		20007
#define FL_SEX		20008
#define FL_SCORE	20009
#define FL_SENDER	20010
#define FL_MYNUM	20011

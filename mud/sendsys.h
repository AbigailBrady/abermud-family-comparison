/*
** MSG refers to text ended with \n and TXT refer to text not ended with \n
*/
#define SYS_SNOOP_OFF	-400
#define SYS_SNOOP_ON	-401
#define SYS_CH_LEV	-598
#define SYS_FROB	-599
#define SYS_CRASH	-666
#define SYS_HALT	-750
#define SYS_VIS		-9900
#define SYS_TXT_TO_R	-10000	/* Text to room */
#define SYS_ZAP		-10001
#define SYS_SHOUT	-10002
#define SYS_SAY		-10003
#define SYS_TELL	-10004
#define SYS_MSG_TO_Rd	-10005	/* Msg to room with \001d...\001 */
#define SYS_MSG_TO_PP	-10006	/* Msg to person with \001P...\001 */
#define SYS_MSG_TO_Pd	-10007	/* Msg to person with \001d...\001 */
#define SYS_EXORCISE	-10010
#define SYS_SUMMON	-10020
#define SYS_ATTACK	-10021
#define SYS_WIZ_SUMMON	-10022
#define SYS_GOD_SUMMON  -10023
#define SYS_WEATHER	-10030
#define SYS_CURE	-10100
#define SYS_CRIPPLE	-10101
#define SYS_DUMB	-10102
#define SYS_FORCE	-10103
#define SYS_WIZ_SHOUT	-10104
#define SYS_BLIND	-10105
#define SYS_MISSILE	-10106
#define SYS_CH_SEX	-10107
#define SYS_FIREBALL	-10109
#define SYS_SHOCK	-10110
#define SYS_MSG_TO_P	-10111	/* Message to person */
#define SYS_HEAL	-10112
#define SYS_WIZ		-10113
#define SYS_FROST	-10114
#define SYS_DEAF	-10120
#define SYS_AWIZ	-10125
#define SYS_FIGHT_OVER	-20000

void gamrcv(int *);
void sendsys(char *, char *, int, int, char *);
void dosumm(int);

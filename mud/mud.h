#define IO_INTER	2		/* How often to scan for updates */

int op(int plnum);
int master(char *name);
void mudprog(char *name);
void crapup(char *str);
void sig_alon();
void unblock_alarm();
void block_alarm();
void sig_aloff();
void tisyschk();
void sig_occur();
void ohdear();
void sig_init();
void sig_oops();
void sig_ctrlc();
void setstdin();
void fetchprmpt(int level);

void GetFields(char *p, char *Data1, char *Data2);
void sec_read(FILE *unit, int *block, int pos, int len);
void sec_write(FILE *unit, int *block, int pos, int len);
void cls();
char *lowercase(char *str);
char *uppercase(char *str);
int *xmalloc(int i);
/* void closelock(FILE *file); */
FILE *openlock(char *file, char *perm);
void get_username(char *username);
void insertch(char *s, char ch, int i);

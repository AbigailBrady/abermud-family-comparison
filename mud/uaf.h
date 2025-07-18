/* static FILE* openuaf(char *perm);
   static FILE* personactl(char *name, PERSONA *d, int act); */
int findpers(char *name, PERSONA *d);
void delpers(char *name);
void putpers(char *name, PERSONA *d);
void decpers(PERSONA *d, char *name, int *str, int *score, int *lev, int *sex);
void initme();
void saveme();

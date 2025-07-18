/*
**  Random functions and utilities
**  This emulates some useful parts of the B system library, etc.
**  Not everyone needs everything here, but it's too hard to split
**  things up into pieces...
*/

#include <errno.h>
#include <sys/file.h>
#include <strings.h>
#include <pwd.h>
#include "kernel.h"
#include "blib.h"
#include "mud.h"

static char sccsid[] = "@(#)blib.c	4.100.0 (IthilMUD)	6/02/90";

/* NOTE:  I will mark the end of the Berkeley file.  */

/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific written prior permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static u_char charmap[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

int strcasecmp(s1, s2)
	char *s1, *s2;
{
	register u_char	*cm = charmap,
			*us1 = (u_char *)s1,
			*us2 = (u_char *)s2;

	while (cm[*us1] == cm[*us2++])
		if (*us1++ == '\0')
			return(0);
	return(cm[*us1] - cm[*--us2]);
}

int strncasecmp(s1, s2, n)
	char *s1, *s2;
	register int n;
{
	register u_char	*cm = charmap,
			*us1 = (u_char *)s1,
			*us2 = (u_char *)s2;

	while (--n >= 0 && cm[*us1] == cm[*us2++])
		if (*us1++ == '\0')
			return(0);
	return(n < 0 ? 0 : cm[*us1] - cm[*--us2]);
}

/*
** 
**   END OF BERKELEY INCLUDE.  I added it because of the strcasecmp
**   and strncasecmp function calls that our system does not have.
**
*/

void GetFields(char *p, char *Data1, char *Data2)
{
    while ((*Data1 = *p++) != '\001')
        Data1++;
    *Data1 = '\0';
    if (Data2)
    {
        while (*p && *p != '\001')
        *Data2++ = *p++;
        *Data2 = '\0';
    }
}

/* Emulate Honeywell random-access files. */

void sec_read(FILE *unit, int *block, int pos, int len)
{
    (void)fseek(unit, (long)(pos * 64 * sizeof (int)), 0);
    (void)fread((char *) block, len * sizeof (int), 1, unit);
}

void sec_write(FILE *unit, int *block, int pos, int len)
{
    (void)fseek(unit, (long)(pos * 64 * sizeof(int)), 0);
    (void)fwrite((char *) block, len * sizeof(int), 1, unit);
}

/* Clear screen */

void
cls()
{
    fflush(stdout);
    (void)system("exec clear");
}

/* Convert a string to lowercase */

char *lowercase(char *str)
{
    char *p;

    for (p = str; *p; p++)
        if (isupper(*p))
            *p = tolower(*p);
    return str;
}

/* Convert a string to uppercase. */

char *uppercase(char *str)
{
    char *p;

    for (p = str; *p; p++)
        if (islower(*p))
            *p = toupper(*p);
    return str;
}

/* Get memory */

int *xmalloc(int i)
{
    int	*p;

    /* NOSTRICT "warning: possible pointer alignment problem" */
    if ((p = (int *)malloc((unsigned int)i)) == NULL)
    {
        (void)fprintf(stderr, "No room to allocate %d bytes.\n", i);
        abort();
    }
    return p;
}

FILE *openlock(char *file, char *perm)
{
    FILE *unit;
    int errno;

    for (errno = 0; (unit = fopen(file, perm)) == NULL && errno == EINTR; )
        ;				/* INTERRUPTED SYSTEM CALL CATCH */
    if (unit == NULL || EQ(perm, "r"))
        return unit;
    while (flock(fileno(unit), LOCK_EX) == -1 && errno == EINTR)
        ;				/* INTERRUPTED SYSTEM CALL CATCH */
    if (errno == ENOSPC)
        crapup("PANIC exit device full");
    #ifdef ESTALE
        if (errno == ESTALE || errno == EHOSTUNREACH || errno == EHOSTDOWN)
        crapup("PANIC exit access failure, NFS gone for a snooze");
    #endif /* ESTALE */
    return unit;
}

void get_username(char *username)
{
    struct passwd *entry;

    entry = getpwuid(getuid());
    strcpy(username, entry->pw_name);
}

char *ttyname(int);

void get_ttynum(char *ttynum)
{
    char *tty;

    if (tty = ttyname(0))
	strcpy(ttynum, tty);
    else
	strcpy(ttynum, "/dev/tty??");
}

/* Inserts ch into s at position i */

void insertch(char *s, char ch, int i)
{
    int j;

    j = strlen(s) + 1;
    while (j > i)
    {
        s[j] = s[j - 1];
        j--;
    }
    s[i] = ch;
}

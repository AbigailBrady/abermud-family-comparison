/*
**
*/

#include "kernel.h"
#include "loctable.h"
#include "macros.h"
#include "exec.h"
#include "bprintf.h"

static char sccsid[] = "@(#)loctable.c	4.100.0 (IthilMUD)	6/02/90";

int convroom(int x)
{
    int ct;
    
    for (ct = 0; ; ct++) {
	if (x == -room_data[ct].r_code)
	    return ct;
	if (room_data[ct].r_code == 0)
	    return -9999;
    }
}

char *sdesc(int room)
{
    if (!exists(room))
	return "Where no man has gone before";
    return room_data[convroom(room)].r_short;
}

char *ldesc(int room)
{
    if (!exists(room))
	return "";
    return room_data[convroom(room)].r_long;
}

int getexit(int room, int ex)
{
    return room_data[convroom(room)].r_exit[ex];
}

int exists(int room)
{
    return convroom(room) != -9999;
}

int locdir()
{
    int locnum = 0;
    int posync = 0;
    int ct, i;
    char rt[32];
    char rs[32];
    
    if (plev(mynum) < LVL_WIZARD) {
	erreval();
	return;
    }
    for (ct = 0; room_data[ct].r_code; ct++) {
	i = findzone(-room_data[ct].r_code, rt);
	sprintf(rs, "%s%d", rt, i);
	bprintf("%-11s", rs);
	locnum++;
	posync++;
	if (posync % 7 == 0)
	    bprintf("\n");
	if (posync % 21 == 0)
	    pbfr();
    }
    bprintf("\nTotal of %d rooms.\n", locnum);
}

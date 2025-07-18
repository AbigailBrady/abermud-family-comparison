/*
**  standard actions file
*/

#include <time.h>
#include "kernel.h"
#include "condact.h"
#include "oflags.h"
#include "action.h"
#include "parse.h"
#include "locdir.h"
#include "weather.h"
#include "extra.h"
#include "zones.h"
#include "magic.h"
#include "objsys.h"
#include "new2.h"
#include "tk.h"
#include "blood.h"
#include "new1.h"
#include "mud.h"
#include "exec.h"
#include "support.h"
#include "bprintf.h"
#include "uaf.h"
#include "move.h"
#include "macros.h"
#include "locations.h"
#include "pflags.h"
#include "mail.h"

/*
**  This function processes a database table entry action statement
**  of any form.  Return may be jumpaway().
*/

void stdaction(int n)
{
    int a, b, c, x, y;

    switch (n)
    {
        case NULL:		        /* Do nothing */
            break;
        case GAIN:			/* Gain points */
            setpscore(mynum, pscore(mynum) + getpar());
            calibme();
            break;
        case LOSE:			/* Lose points */
            setpscore(mynum, pscore(mynum) - getpar());
            calibme();
            break;
        case WOUND:			/* Wounded and take damage */
            setpstr(mynum, pstr(mynum) - getpar());
            calibme();
            break;
        case HEAL:			/* Healed damage */
            setpstr(mynum, pstr(mynum) + getpar());
            calibme();
            break;
        case INC:			/* Increase state of object */
            a = getpar();
            setobjstate(a, state(a) + 1);
            if (state(a) > omaxstate(a))
                setobjstate(a, omaxstate(a));
            break;
        case DEC:			/* Decrease state of object */
            a = getpar();
            if (state(a))
                setobjstate(a, state(a) - 1);
            break;
        case PUT:			/* Put object in room */
            a = getpar();
            setoloc(a, getpar(), 0);
            break;
        case PUTIN:			/* Put object in container */
            a = getpar();
            setoloc(a, getpar(), 3);
            break;
        case PUTCARR:			/* Put object carried by user */
            a = getpar();
            setoloc(a, getpar(), 1);
            break;
        case PUTWORN:			/* Put object worn by user */
            a = getpar();
            setoloc(a, getpar(), 2);
            break;
        case OK:			/* Print Ok */
            bprintf("Ok\n");
        /* FALLTHROUGH */
        case DONE:			/* Exit table */
            jumpaway();
        case DESTROY:			/* Destroy object */
            osetbit(getpar(), ofl(Destroyed));
            break;
        case CHECK_NOLOGIN:		/* NOLOGIN check */
            check_nologin();
            break;
        case CHECK_MAIL:		/* New mail check */
            checkmail();
            break;
        case EAT:			/* Destroy object */
            eat(getpar());
            break;
        case QUIT:			/* Message exit */
            loseme();
            crapup(messages[getpar()]);
        case DIE:			/* Die, but don't save (hard death) */
            delpers(pname(mynum));
            zapped = True;
            loseme();
            crapup(messages[getpar()]);
        case PUTWITH:			/* put obj by obj */
            a = getpar();
            b = getpar();
            setoloc(a, oloc(b), ocarrf(b));
            break;
        case GOBY:			/* Goto object */
            teletrap(oloc(getpar()));
            break;
	case DISPROOM:		        /* Describe current room */
            lookin(ploc(mynum));
            break;
        case GOTO:			/* Jump to room */
            teletrap(getpar());
            break;
        case SWAP:			/* Swap two object positions */
            a = getpar();
            b = getpar();
            x = oloc(a);
            y = ocarrf(a);
            setoloc(a, oloc(b), ocarrf(b));
            setoloc(b, x, y);
            break;
        case POBJ:			/* Print object name */
            bprintf("%s", oname(getpar()));
            break;
        case PPLAYER:			/* Print player name */
            bprintf("%s", pname(getpar()));
            break;
        case SETLEVEL:		        /* Set level */
            setplev(mynum, getpar());
            calibme();
            break;
            /* Setlevel to mortal results in calibration resetting! */
        case SETVIS:
            setpvis(mynum, getpar());
            break;
        case LOBJAT:
            aobjsat(getpar(), 0, 0);
            break;
        case LOBJIN:
            aobjsat(getpar(), 3, 0);
            break;
        case LOBJCARR:		        /* List items in things */
            aobjsat(getpar(), 1, 0);
            break;
        case CREATE:
            oclrbit(getpar(), ofl(Destroyed));
            break;
        case DOOBJS:			/* UNIMP */
            break;
        case DOPLS:			/* UNIMP */
            break;
        case SETSTATE:  		/* Set object state */
            a = getpar();
            setobjstate(a, (int) getpar());
            break;

/*
**  Most of the actions below this point do major sections of code.
**  Quite a few still contain hard coded special cases which want
**  removing so be careful.
*/

        case INVEN:
            inventory();
            break;
        case EXITS:
            exits();
            break;
        case MOVE:			/* 0=n 1=e etc... */
            dodirn(getpar());
            break;
        case SETIN:
            set_ms(in_ms);
            break;
        case SETOUT:
            set_ms(out_ms);
            break;
        case SETMIN:
            set_ms(min_ms);
            break;
        case SETMOUT:
            set_ms(mout_ms);
            break;
        case SETVIN:
            set_ms(vin_ms);
            break;
        case SETVOUT:
            set_ms(vout_ms);
            break;
        case SETPROMPT:
            set_ms(prm_str);
            break;
        case TELL:
            tellcom();
            break;
        case SHOUT:
            shoutcom();
            break;
        case WIZACT:
            wizcom();
            break;
        case ARCH:
            awizcom();
            break;
        case BUG:
            bugcom();
            break;
        case TYPO:
            typocom();
            break;
        case BLIND:
            psetflg(mynum, pfl(Blind));
            break;
        case DEAF:
            psetflg(mynum, pfl(Deaf));
            break;
        case DUMB:
            psetflg(mynum, pfl(Dumb));
            break;
        case CRIPPLE:
            psetflg(mynum, pfl(Crippled));
            break;
        case CURE:
            pclrflg(mynum, pfl(Blind) | pfl(Deaf) | pfl(Crippled) | pfl(Dumb));
            break;
        case LISTFILE:	/* Print a file - using bprintf file include */
            bprintf("\001f%s\377", messages[getpar()]);
            break;
        case USERS:
            usercom();
            break;
        case SETSEX:
            a = getpar();
            setpsex(a, getpar());
            break;
        case PRONOUNS:
            pncom();
            break;
        case WHO:
            whocom();
            break;
        case GET:
            getobj();
            break;
        case DROP:
            dropitem();
            break;
        case WEAR:
            wearcom();
            break;
        case REMOVE:
            removecom();
            break;
        case GIVE:
            givecom();
            break;
        case STEAL:
            stealcom();
            break;
        case VALUE:
            valuecom();
            break;
        case SCORE:
            scorecom();
            break;
        case WEAPON:
            weapcom();
            break;
        case DROPIN:
            putcom();
            break;
        case FORCE:
            forcecom();
            break;
        case EXORCISE:
            exorcom();
            break;
        case TIME:
            timecom();
            break;
        case SETACT:
            setcom();
            break;
        case EXAMINE:
            examcom();
            break;
        case LOCNODES:
            loccom();
            break;
        case DIR:
            dircom();
            break;
        case RESET:
            rescom();
            break;
        case INACT:		/* Action after IN is parsed seperately */
            incom();
            break;
        case GOTOACT:
            goloccom();
            break;
        case PFLAGS:
            pflagcom();
            break;
        case PLAYERS:
            usercom();
            break;
        case MOBILES:
            mobilecom();
            break;
        case WHERE:
            wherecom();
            break;
        case DECOBYTE:
            a = getpar();
            b = getpar();
            osetbyte(a, b, obyte(a, b) - 1);
            break;
        case INCOBYTE:		/* Incremenbt object byte flag */
            a = getpar();
            b = getpar();
            osetbyte(a, b, obyte(a, b) + 1);
            break;
        case SETOBYTE:
            a = getpar();
            b = getpar();
            c = getpar();
            osetbyte(a, b, c);
            break;
        case PROVOKE:			/* Do monster 0 point hit - starts fight */
            woundmn(getpar(), 0);
            break;
        case MOVEMON:			/* Move player with no message */
            a = getpar();
            setploc(a, getpar());
            break;
        case WOUNDMON:		/* Wound a mobile - cannot apply to players */
            a = getpar();
            woundmn(a, getpar());
            break;
        case KILL:
            killcom();
            break;
        case LOOK:
            lookcom();
            break;
        case BROAD:
            broad(messages[getpar()]);
            break;
        default:			/* Analyse message groups */
            if (n > 1999)
                bprintf("%s", messages[n - 2000]);
            else if (n > 199)
                bprintf("%s\n", messages[n - 200]);
            else
                bprintf("**Invalid Action\n");
    }
}

void check_nologin()
/* Check if there is a NOLOGIN file */
{
    FILE *a;
    char u[80];

    if (!isawiz && (a = fopen(NOLOGIN, "r")))
    {
        bprintf("\n\n");
        while (fgets(u, sizeof u, a))
            bprintf("%s", u);
        bprintf("\n\n");
        fclose(a);
        setploc(mynum, RM_PIT1);
        dumpitems();
        loseme();
        closeworld();
        exit(0);
    }
}

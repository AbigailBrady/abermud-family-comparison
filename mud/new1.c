/*
** Extensions section 1
*/

#include <strings.h>
#include "kernel.h"
#include "macros.h"
#include "objects.h"
#include "mobiles.h"
#include "pflags.h"
#include "oflags.h"
#include "new1.h"
#include "new2.h"
#include "objsys.h"
#include "parse.h"
#include "support.h"
#include "weather.h"
#include "tk.h"
#include "mud.h"
#include "locations.h"

void flowercom()
{
    int a;
    char s[80];

    if (vichere(&a) == -1)
	return;
    sillytp(a, "sends you flowers.\n\n         \001A\033[1;33m\377(*)\n     \001A\033[35m\377*    \001A\033[32m\377|    \001A\033[1;36m\377*\n    \001A\033[32m\377\\|/  \\|/  \\|/\n  \001A\033[0;33m\377 ---------------\001A\033[0m\377");
    sprintf(s, "You send ~ flowers.\n");
    pnstring(s, a);
    bprintf(s);
}

void ticklecom()
{
    int a;
    char s[80];

    if (vichere(&a) == -1)
	return;
    if (a == mynum) {
	bprintf("You tickle yourself.\n");
	return;
    }
    sillytp(a, "tickles you.");
    sprintf(s, "You tickle ~.\n");
    pnstring(s, a);
    bprintf(s);
}

void petcom()
{
    int a;
    char s[80];
    static char master = 0;

    if (vichere(&a) == -1)
	return;
    if (a == mynum) {
	switch (master)
	{case 0:	
	     bprintf("You begin to pet yourself.\n");
	     strcpy(s, "\001P%s\377 begins to play with ~self!\n");
	     pnstring(s, mynum);
	     sillycom(s);
	     break;
	 case 1:case 2:case 3:case 4:
	     bprintf("You fondle yourself vigorously.\n");
	     strcpy(s, "\001P%s\377 fondles ~self vigorously.\n");
	     pnstring(s, mynum);
	     sillycom(s);
	     break;
	 case 5:case 6:case 7:case 8:
	     bprintf("You're really getting into fondling yourself!\n");
	     strcpy(s,"\001P%s\377 is fondling ^ genitals madly while "
			"breathing heavily.\n");
	     pnstring(s,mynum);
	     sillycom(s);
	     break;
	 case 9:
	     bprintf("You're about to have an orgasm!!!!!!\n");
	     sillycom("\001P%s\377 is about to have an orgasm!!!!\n"
		      "(Quick!  Have your children avert their eyes!)\n");
	     break;
	 case 10:
	     bprintf("You have an orgasm!!!!!!!!!\n"
		     "God does that feel good!!!!\n"
		     "You wipe up after yourself.\n");
	     mudlog(" came.\n");
	     sillycom("\001P%s\377 just had an orgasm!!  "
			"Fluid goes everywhere.\n"
		        "(What a show!)\n");
	     if (pscore(mynum) < 1234)
		 setpscore(mynum, 1234);
	     break;	
	 case 11:
	     bprintf("Are you sure?  You're kinda tired.\n");
	     break;
	 default:
	     bprintf("You clean up and relax in anticipation of "
		     "another round.\n");	
	     sillycom("\001P%s\377 is going to go at it again, "
			"me thinks.\n");
	     master = 0;
	     return;
	 }
	master++;
	return;
    }
    sillytp(a, "pats you on the head.");
    sprintf(s, "You pat ~ on the head.\n");
    pnstring(s, a);
    bprintf(s);
}

void wishcom()
{
    char x[128];
    char z[128];
    char y[160];

    if (EMPTY(item1)) {
	bprintf("Wish for what?\n");
	return;
    }
    strcpy(z, "[");
    getreinput(x);
    strcat(z, x);
    strcat(z, "]\n");
    sprintf(y, "[Wish from \001p%s\377]\n", pname(mynum));
    sendsys(pname(mynum), pname(mynum), -10113, ploc(mynum), y);
    sendsys(pname(mynum), pname(mynum), -10113, ploc(mynum), z);
    sillycom("\001s%s\377%s begs and grovels to the powers that be.\n\377");
    bprintf("Ok\n");
}

int ohereandget(int *onm)
{
    if (EMPTY(item1)) {
	bprintf("Tell me more?\n");
	return -1;
    }
    openworld();
    if ((*onm = ob1) == -1) {
	bprintf("It isn't here.\n");
        return -1;
    }
    return 1;
}

void setobjstate(int o, int v)
{
    state(o) = v;
    if (otstbit(o, ofl(Linked)))
	state(o ^ 1) = v;
}

void wavecom()
{
    if (in_fight) {
        bprintf("What are you trying to do?  Make 'em die laughing?\n");
        return;
    }
    if (EMPTY(item1)) {
	sillycom("\001s%s\377%s waves happily.\n\377");
	bprintf("You wave happily.\n");
    }
    switch (ob1) {
    case -1:
	return;
    case OBJ_WAND_1:
	if ((state(OBJ_BRIDGE_1) == 1) && (oloc(OBJ_BRIDGE_1) == ploc(mynum))) {
	    setobjstate(OBJ_BRIDGE, 0);
	    bprintf("The drawbridge is lowered!\n");
	    return;
	}
	break;
    case OBJ_ROD:
	if (iscarrby(OBJ_CUP, mynum)) {
	    bprintf("Something prevents the rod's functioning.\n");
	    return;
	}
	if (obyte(OBJ_ROD, 1) == 3) {
	    bprintf("The rod crumbles to dust!\n");
	    destroy(OBJ_ROD);
	    return;
	}
	osetbyte(OBJ_ROD, 1, obyte(OBJ_ROD, 1) + 1);
	bprintf("You are teleported!\n");
	teletrap(-114);
	return;
    case OBJ_WAND:
	if (obyte(OBJ_WAND, 1) != 0) {
	    osetbyte(OBJ_WAND, 1, obyte(OBJ_WAND, 1) - 1);
	    me_ivct = 30;
	    setpvis(mynum, 12);
	    bprintf("You seem to shimmer and blur.\n");
	    return;
	}
    }
    bprintf("Nothing happens.\n");
}

void blowcom()
{
    int a;
    char x[80];

    if (ohereandget(&a) == -1)
	return;
    if (a == OBJ_BAGPIPES) {
	broad("\001dA hideous wailing sounds echos all around.\n\377");
	return;
    }
    if (a == OBJ_WHISTLE) {
	broad("\001dA strange ringing fills your head.\n\377");
	if (alive(MOB_OTTIMO) != -1) {
/*   destroy(OBJ_WHISTLE);
		bprintf("You blew so hard that you destroyed the whistle, but\n");
*/   bprintf("a small dachshund bounds into the room and leaps on you playfully.\n");
		sprintf(x, "A small dachshund bounds into the room and leaps on %s playfully.\n", pname(mynum));
	    sillycom(x);
	    setploc(MOB_OTTIMO, ploc(mynum));
	}
	return;
    }
    if (a == OBJ_HORN) {
	broad("\001dA mighty horn blast echoes around you.\n\377");
	if (ploc(mynum) >= -999 && ploc(mynum) <= -900 &&
            obyte(OBJ_EXCALIBUR, 1) == 0) {
	    setoloc(OBJ_EXCALIBUR, ploc(mynum), 0);
	    setobjstate(OBJ_EXCALIBUR, 1);
	    bprintf("A hand breaks through the water holding up the sword Excalibur!\n");
	    osetbyte(OBJ_EXCALIBUR, 1, 1);
	}
	return;
    }
    bprintf("You can't blow that.\n");
}

void putcom()
{
    int a;
    char ar[128];
    int c;

    if (ohereandget(&a) == -1)
	return;
    if (EMPTY(item2)) {
	bprintf("Where?\n");
	return;
    }
    if ((c = ob2) == -1) {
	bprintf("It isn't here.\n");
	return;
    }
/***  Generating errors   - DAZ  (Put it back when you're ready
    if (c == OBJ_TABLE_2) {
      if (a == OBJ_MAP) {
	  if (state(OBJ_LADDER_2)) {
	     bprintf("Nothing happens...\n");
	  }
	  else if (state(OBJ_SAIL) == 1) {
	     bprintf("A ghostly pirate fades into view.  After he carefully studying the map, he\n");
	     bprintf("goes outside, and sails the ship to a new land...\n");
	     setobjstate(OBJ_LADDER_2, 1);
	     setobjstate(OBJ_PLANK_1, 0);
	     destroy(OBJ_MAP);
	     }
	  else {
	     bprintf("A ghostly pirate fades into view.  After testing the wind carefully, he\n");
	     bprintf("shakes his head sadly, and fades away.\n");
	     }
       }
    }
 **************/
    if (c == OBJ_HOLE_9) {
      if (state(c) == 1) {
          if (a == OBJ_RUBY || a == OBJ_SAPPHIRE || a == OBJ_DIAMOND ||
		    	 a == OBJ_GEM || a == OBJ_GEM_1 || a == OBJ_AMETHYST ||
		    	 a == OBJ_HOPE) {
      	     bprintf("The gem clicks into place...\n");
      	     bprintf("...and the door opens!\n");
      	     setobjstate(c, 0);
      	  }
      	  else {
	         bprintf ("It doesn't fit!\n");
	      }
      }
      else {
	 	  bprintf ("Nothing happens.\n");
      }
      return;
    }
    if (c == OBJ_LAKE) {
     if (state(c) == 1) {
        if (a == OBJ_SPONGE) {
             bprintf("The Sponge seems to miraculously suck up the");
		     bprintf(" water in the lake!\n");
             bprintf("It has dried the entire lake...wow!\n");
             setobjstate(OBJ_LAKE, 0);
        }
        else {
		     bprintf("OK - it gets wet.  Now what?\n");
        }
     }
     else {
	    bprintf("What lake? It is dried up!\n");
     }
     return;
    }
	if (a == OBJ_GRASSHOPPER) {
	  bprintf("\nYou hear an explosion and see a blinding flash of light!\n");
	  broad("\001dYou hear a loud explosion coming from town.\n\377");
	  teletrap(RM_HEAVEN1);
	  if (alive(MOB_WORKMAN)) {
		setploc(MOB_WORKMAN, RM_VILLAGE12);
	  }
	}
    if (a == OBJ_VASE) {
      destroy(OBJ_VASE);
      bprintf("The vase slips out of you hands and crashes to the floor!\n");
      bprintf("The reverberations caused by the crash cause the ceiling to");
      bprintf(" cave in!\n");
      broad("\001dYou hear ambulance sirens in the distance.\n\377");
      teletrap(RM_HEAVEN1);
      return;
    }
    if (c == OBJ_CANDLESTICK) {
      if (a != OBJ_CANDLE && a != OBJ_CANDLE_1 && a != OBJ_CANDLE_2) {
	    bprintf("You can't do that.\n");
	    return;
	  }
	  if (state(OBJ_CANDLESTICK) != 2) {
		bprintf("There's already a candle in it!\n");
	    return;
  	  }
	  bprintf("The candle fixes firmly into the candlestick.\n");
	  setpscore(mynum, pscore(mynum) + 50);
	  destroy(a);
	  osetbyte(OBJ_CANDLESTICK, 1, a);
	  osetbit(OBJ_CANDLESTICK, ofl(Lightable));
	  osetbit(OBJ_CANDLESTICK, ofl(Extinguishable));
	  if (otstbit(a, ofl(Lit))) {
	      osetbit(OBJ_CANDLESTICK, ofl(Lit));
	      setobjstate(OBJ_CANDLESTICK, 0);
	      return;
  	  }
	  setobjstate(OBJ_CANDLESTICK, 1);
	  oclrbit(OBJ_CANDLESTICK, ofl(Lit));
	  return;
    }
    if (c == OBJ_BALL_1) {
	if (a == OBJ_WAND && !obyte(a, 1)) {
	    bprintf("The wand seems to soak up energy.\n");
	    osetbyte(a, 1, 4);
	    return;
	}
	bprintf("Nothing happens.\n");
	return;
    }
    if (c == OBJ_PIT) {
	if (state(c) == 0) {
	    setoloc(a, -162, 0);
	    bprintf("Ok\n");
	    return;
	}
	destroy(a);
	bprintf("It dissappears with a fizzle into the slime.\n");
	if (a == OBJ_SOAP) {
	    bprintf("The soap dissolves the slime away!\n");
	    setobjstate(OBJ_PIT, 0);
	}
	return;
    }
    if (c == OBJ_CHUTE_1) {
	bprintf("You can't do that, the chute leads up from here!\n");
	return;
    }
    if (c == OBJ_CHUTE) {
	if (a == OBJ_RUNESWORD) {
	    bprintf("You can't let go of it!\n");
	    return;
	}
	bprintf("It vanishes down the chute....\n");
	sprintf(ar, "The %s comes out of the chute.\n", oname(a));
	sendsys("", "", -10000, oloc(OBJ_CHUTE_1), ar);
	setoloc(a, oloc(OBJ_CHUTE_1), 0);
	return;
    }
    if (c == OBJ_HOLE) {
	if (a == OBJ_SCEPTRE && state(OBJ_DOOR_1) == 1) {
	    setobjstate(OBJ_DOOR, 0);
	    sprintf(ar, "The door clicks open!\n");
	    sendsys("", "", -10000, oloc(OBJ_DOOR), ar);
	    sendsys("", "", -10000, oloc(OBJ_DOOR_1), ar);
	    return;
	}
	bprintf("Nothing happens.\n");
	return;
    }
    if (c == a) {
	bprintf("What do you think this is, the goon show?\n");
	return;
    }
    if (otstbit(c, ofl(Container)) == 0) {
	bprintf("You can't do that.\n");
	return;
    }
    if (state(c) != 0) {
	bprintf("It's not open.\n");
	return;
    }
    if (oflannel(a)) {
	bprintf("You can't take that!\n");
	return;
    }
    if ((ishere(a)) && (dragget()))
	return;
    if (a == OBJ_RUNESWORD) {
	bprintf("You can't let go of it!\n");
	return;
    }
    if (otstbit(a, ofl(Lit))) {
	bprintf("I'd try putting it out first!\n");
	return;
    }
    if (!willhold(c, a)) {
	bprintf("It won't fit.\n");
	return;
    }
    setoloc(a, c, 3);
    bprintf("Ok\n");
    sprintf(ar, "\001D%s\377\001c puts the %s in the %s.\n\377",
    pname(mynum), oname(a), oname(c));
    sendsys(pname(mynum), pname(mynum), -10000, ploc(mynum), ar);
    if (otstbit(a, ofl(GetFlips)))
	setobjstate(a, 0);
    if (ploc(mynum) == -1081 && state(OBJ_DOOR) == 0 && ishere(a)) {
	setobjstate(OBJ_DOOR, 1);
	sprintf(ar, "The door clicks shut....\n");
	sendsys("", "", -10000, oloc(OBJ_DOOR), ar);
	sendsys("", "", -10000, oloc(OBJ_DOOR_1), ar);
    }
}

void lightcom()
{
    int a;

    if (ohereandget(&a) == -1)
	return;
    if (!ohany(1 << 13)) {
	  bprintf("You have nothing to light things from.\n");
	  return;
    }
    if ((a == OBJ_THORNS) && (state(a) == 1)) {
       bprintf("You burn the wall of thorns away!\n");
       bprintf("Beyond them you see the enterance to a cave.\n");
       setobjstate(OBJ_THORNS, 0);
    }
    if ((a == OBJ_THORNS_1) && (state(a) == 1)) {
       bprintf("You burn the wall of thorns away!\n");
       bprintf("Beyond them you see the enterance to a cave.\n");
       setobjstate(OBJ_THORNS_1, 0);
    }
    if (!otstbit(a, ofl(Lightable)))
	bprintf("You can't light that!\n");
    else if (otstbit(a, ofl(Lit)))
	bprintf("It's already lit.\n");
    else {
	setobjstate(a, 0);
	osetbit(a, ofl(Lit));
	bprintf("Ok\n");
    }
}

void extinguishcom()
{
  int a;

  if (ohereandget(&a) == -1)
    return;
  if (!otstbit(a, ofl(Lit)))
    bprintf("It's not lit!\n");
  else if (!otstbit(a, ofl(Extinguishable)))
    bprintf("You can't extinguish that!\n");
  else {
    setobjstate(a, 1);
    oclrbit(a, ofl(Lit));
    bprintf("Ok\n");
  }
}

void pushcom()
{
  int x;

  if (brkword() == -1) {
    bprintf("Push what?\n");
    return;
  }
  if ((x = fobna(wordbuf)) == -1) {
    bprintf("That is not here.\n");
    return;
  }
  switch (x) {
  case OBJ_BUTTON:
     if (state(OBJ_THRONE_6)==1) {
    	 bprintf("You hear a grinding sound from near the entrance ");
     	 bprintf("of these caves.\n");
    	 setobjstate(OBJ_THRONE_6, 0);
     }
     else {
     	bprintf("You hear a click, but nothing seems to happen.\n");
	 }
     return;
  case OBJ_TRIPWIRE:
    bprintf("The tripwire moves and a huge stone crashes down from above!\n");
    broad("\001dYou hear a thud and a squelch in the distance.\n\377");
    loseme();
    crapup("             S   P    L      A         T           !");
  case OBJ_BOOKCASE:
    if (in_fight) {
      bprintf("You're too busy to do that right now!\n");
    } else {
      bprintf("A trapdoor opens at your feet and you plumment downwards!\n");
      sillycom("\001P%s\377 disappears through a trapdoor!\n");
      trapch(-140);
    }
    return;
  case OBJ_BAR:
    if (state(OBJ_PASSAGE) == 1) {
      setobjstate(OBJ_PASSAGE_1, 0);
      bprintf("A secret panel opens in the east wall!\n");
      break;
    }
    bprintf("Nothing happens.\n");
    break;
  case OBJ_BOULDER:
    bprintf("With a mighty heave you manage to move the boulder a few feet\n");
    if (state(OBJ_HOLE_1) == 1) {
      bprintf("uncovering a hole behind it.\n");
      setobjstate(OBJ_HOLE_1, 0);
    }
    else {
      bprintf("covering a hole behind it.\n");
      setobjstate(OBJ_HOLE_1, 1);
    }
    break;
  case OBJ_MIRROR:
	bprintf("You swing the mirror aside, ");
	if (state(OBJ_HOLE_7) == 1) {
	  bprintf("uncovering a hole behind it.\n");
	  setobjstate(OBJ_HOLE_7, 0);
    }
    else {
      bprintf("covering a hole behind it.\n");
      setobjstate(OBJ_HOLE_7, 1);
    }
    break;
  case OBJ_LEVER_1:
    if (ptothlp(mynum) == -1) {
      bprintf("It's too stiff.  Maybe you need help.\n");
      return;
    }
    bprintf("Ok\n");
    if (state(OBJ_PIT) != 0) {
      sillycom("\001s%s\377%s pulls the lever.\n\377");
      sendsys("", "", -10000, oloc(OBJ_LEVER_1),
	      "\001dYou hear a gurgling noise and then silence.\n\377");
      setobjstate(OBJ_PIT, 0);
      sendsys("", "", -10000, oloc(OBJ_PIT),
	      "\001cThere is a muffled click and the slime drains away.\n\377");
    }
    break;
  case OBJ_CURTAINS:
  case OBJ_CURTAINS_1:
    setobjstate(OBJ_CURTAINS, 1 - state(OBJ_CURTAINS));
    bprintf("Ok\n");
    break;
  case OBJ_LEVER:
    setobjstate(OBJ_PORTCULLIS, 1 - state(OBJ_PORTCULLIS));
    if (state(OBJ_PORTCULLIS)) {
      sendsys("", "", -10000, oloc(OBJ_PORTCULLIS),
	      "\001cThe portcullis falls.\n\377");
      sendsys("", "", -10000, oloc(OBJ_PORTCULLIS_1),
	      "\001cThe portcullis falls.\n\377");
    }
    else {
      sendsys("", "", -10000, oloc(OBJ_PORTCULLIS),
	      "\001cThe portcullis rises.\n\377");
      sendsys("", "", -10000, oloc(OBJ_PORTCULLIS_1),
	      "\001cThe portcullis rises.\n\377");
    }
    break;
  case OBJ_LEVER_2:
    setobjstate(OBJ_BRIDGE, 1 - state(OBJ_BRIDGE));
    if (state(OBJ_BRIDGE)) {
      sendsys("", "", -10000, oloc(OBJ_BRIDGE),
	      "\001cThe drawbridge rises.\n\377");
      sendsys("", "", -10000, oloc(OBJ_BRIDGE_1),
	      "\001cThe drawbridge rises.\n\377");
    }
    else {
      sendsys("", "", -10000, oloc(OBJ_BRIDGE),
	      "\001cThe drawbridge is lowered.\n\377");
      sendsys("", "", -10000, oloc(OBJ_BRIDGE_1),
	      "\001cThe drawbridge is lowered.\n\377");
    }
    break;
  case OBJ_TORCH:
    if (state(OBJ_DOOR_2) == 1) {
      setobjstate(OBJ_DOOR_2, 0);
      bprintf("A secret door slides quietly open in the south wall!\n");
    }
    else
      bprintf("It moves but nothing seems to happen.\n");
    return;
  case OBJ_ROPE:
    if (obyte(OBJ_ROPE, 1) > 7)
      bprintf("\001dChurch bells ring out around you.\n\377");
    else {
      broad("\001dChurch bells ring out around you.\n\377");
      osetbyte(OBJ_ROPE, 1, obyte(OBJ_ROPE, 1) + 1);
      if (obyte(OBJ_ROPE, 1) == 8) {
	bprintf("A strange ghostly guitarist shimmers briefly before you.\n");
	setpscore(mynum, pscore(mynum) + 300);
	broad("\001dA faint ghostly guitar solo floats through the air.\n\377");
      }
    }
    break;
  case OBJ_DUST:
    bprintf("Great clouds of dust billow up, causing you to sneeze horribly.\n");
    bprintf("When you're finished sneezing, you notice a message carved into one wall.\n");
    broad("\001dA loud sneeze echoes through the land.\n\377");
    destroy(OBJ_DUST);
    create(OBJ_KOAN);
    break;
  case OBJ_COVER_1:
    bprintf("You can't seem to get enough leverage to move it.\n");
    return;
  case OBJ_COVER:
    if (ptothlp(mynum) == -1) {
      bprintf("You try to shift it, but it's too heavy.\n");
      break;
    }
    sillytp(ptothlp(mynum), "pushes the cover aside with your help.");
    setobjstate(x, 1 - state(x));
    oplong(x);
    return;
  case OBJ_SWITCH:
    if (state(x)) {
      bprintf("A hole slides open in the north wall!\n");
      setobjstate(x,0);
    }
    else
      bprintf("You hear a little 'click' sound.\n");
    return;
  case OBJ_VASE:
    destroy(OBJ_VASE);
    bprintf("The vase crashes to the floor!\n");
    bprintf("The reverberations caused by the crash cause the ceiling to");
    bprintf(" cave in!\n");
    broad("\001dYou hear ambulance sirens in the distance.\n\377");
    teletrap(-2700);
    break;
  case OBJ_STATUE:
    if (ptothlp(mynum) == -1) {
      bprintf("You can't shift it alone, maybe you need help.\n");
      break;
    }
    sillytp(ptothlp(mynum), "pushes the statue with your help.");
    /* FALLTHROUGH */
  default:
    if (otstbit(x, ofl(Pushable))) {
      setobjstate(x, 0);
      oplong(x);
      return;
    }
    if (otstbit(x, ofl(PushToggle))) {
      setobjstate(x, 1 - state(x));
      oplong(x);
      return;
    }
    bprintf("Nothing happens.\n");
  }
}

int vicbase(int *x)
{
  int a, b;

  do {
    if (brkword() == -1) {
      bprintf("Who?\n");
      return -1;
    }
    b = openworld() != NULL;
  } while (EQ(wordbuf, "at"));
  if ((a = fpbn(wordbuf)) == -1) {
    bprintf("That person isn't playing now.\n");
    return -1;
  }
  *x = a;
  return b;
}



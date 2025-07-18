/*
**  AberMUD IV Database Definitions
*/

#include "macros.h"
#include "kernel.h"
#include "condact.h"
#include "objects.h"
#include "locations.h"
#include "oflags.h"
#include "mobiles.h"
#include "verbs.h"

/*
** Events
**
** The order of objects with same names should NOT be changed.
*/

LINE event[] = {

/* Random "floorboards squeek" message in cellar */
0, ANY, ANY,
   HERE,OBJ_FLOORBOARDS, CHANCE,30, PAR,0, PAR,0, PAR,0, PAR,0,
   227, NULL, NULL, NULL,

/* Drink from chalice */
VERB_EAT, OBJ_CHALICE, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   260, DONE, NULL, NULL,

/* Put chalice on altar */
VERB_PUT, OBJ_CHALICE, OBJ_ALTAR,
   PAR,OBJ_CHALICE, PAR,OBJ_ALTAR, PAR,1, PAR,0, PAR,0, PAR,0,
   DESTROY, SETSTATE, OK, NULL,

/* Pray at altar + chalice */
VERB_PRAY, ANY, ANY,
   HERE,OBJ_ALTAR, STATE,OBJ_ALTAR, NULL,1, PAR,-2500, PAR,OBJ_ALTAR, PAR,0,
   261, GOTO, SETSTATE, DONE,

/* Praying in Wasteland brings you behind the altar */
VERB_PRAY, ANY, ANY,
   INLT,-2500, NOT|INLT,-2600, PAR,-1420, CHANCE,50, NOT|IFFIGHTING,0, PAR,0,
   264, GOTO, DONE, NULL,

/* Get or put stuff on altar */
VERB_GET, ANY, OBJ_ALTAR,
   STATE,OBJ_ALTAR, NULL,1, PAR,0, PAR,0, PAR,0, PAR,0,
   262, DONE, NULL, NULL,

/* North to trapdoor in ORCHOLD */
VERB_NORTH, ANY, ANY,
   IN,-2603, PAR,-2605, PAR,0, PAR,0, PAR,0, PAR,0,
   274, GOTO, DONE, NULL,

/* Same for the remove */
VERB_REMOVE, ANY, OBJ_ALTAR,
   STATE,OBJ_ALTAR, NULL,1, PAR,0, PAR,0, PAR,0, PAR,0,
   262, DONE, NULL, NULL,

/* go raft */
VERB_GO, OBJ_RAFT, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   233, DONE, NULL, NULL,

/* go boat */
VERB_GO, OBJ_BOAT, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   233, DONE, NULL, NULL,

/* give seamas the pipes */
VERB_GIVE, P(MOB_SEAMAS), OBJ_PIPES,
   GOT,OBJ_PIPES, PHERE,MOB_SEAMAS, PAR,48, PAR,0, PAR,0, PAR,0,
   GIVE, 247, BROAD, DONE,

/* give pipes to seamas */
VERB_GIVE, OBJ_PIPES, P(MOB_SEAMAS),
   GOT,OBJ_PIPES, PHERE,MOB_SEAMAS, PAR,48, PAR,0, PAR,0, PAR,0,
   GIVE, 247, BROAD, DONE,

/* give Piper the bagpipes */
VERB_GIVE, P(MOB_PIPER), OBJ_BAGPIPES,
   GOT,OBJ_BAGPIPES, PHERE,MOB_PIPER, PAR,68, PAR,50, PAR,0, PAR,0,
   GIVE, 255, BROAD, GAIN,

/* give Piper the bagpipes */
VERB_GIVE, P(MOB_PIPER), OBJ_BAGPIPES,
   GOT,OBJ_BAGPIPES, PHERE,MOB_PIPER, PAR,0, PAR,0, PAR,0, PAR,0,
   DONE, 0, 0, 0,

/* give bagpipes to Piper */
VERB_GIVE, OBJ_BAGPIPES, P(MOB_PIPER),
   GOT,OBJ_BAGPIPES, PHERE,MOB_PIPER, PAR,68, PAR,50, PAR,0, PAR,0,
   GIVE, 255, BROAD, GAIN,

/* give Piper the bagpipes */
VERB_GIVE, OBJ_BAGPIPES, P(MOB_PIPER),
   GOT,OBJ_BAGPIPES, PHERE,MOB_PIPER, PAR,0, PAR,0, PAR,0, PAR,0,
   DONE, 0, 0, 0,

/* give umbrella to chicken */
VERB_GIVE, OBJ_UMBRELLA, P(MOB_CHICKEN),
   GOT,OBJ_UMBRELLA, PHERE,MOB_CHICKEN, PAR,50, PAR,0, PAR,0, PAR,0,
   GIVE, 230, GAIN, DONE,

/* give chicken the umbrella */
VERB_GIVE, P(MOB_CHICKEN), OBJ_UMBRELLA,
   GOT,OBJ_UMBRELLA, PHERE,MOB_CHICKEN, PAR,50, PAR,0, PAR,0, PAR,0,
   GIVE, 230, GAIN, DONE,

/* give runesword to player */
VERB_GIVE, OBJ_RUNESWORD, ANY,
   NOT|WIZ,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   205, DONE, NULL, NULL,

/* give player runesword */
VERB_GIVE, ANY, OBJ_RUNESWORD,
   NOT|WIZ,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   205, DONE, NULL, NULL,

/* give */
VERB_GIVE, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   GIVE, OK, NULL, NULL,

/* play the pipes */
VERB_PLAY, OBJ_PIPES, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   249, DONE, NULL, NULL,

/* play the harp */
VERB_PLAY, OBJ_HARP, ANY,
   PAR,65, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   BROAD, DONE, NULL, NULL,

/* value */
VERB_VALUE, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   VALUE, OK, NULL, NULL,

/* cut ladder */
VERB_CUT, OBJ_LADDER, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   240, DONE, NULL, NULL,

/* cut ladder 1 */
VERB_CUT, OBJ_LADDER_1, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   240, DONE, NULL, NULL,

/* cut rope 1 */
VERB_CUT, OBJ_ROPE_1, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   240, DONE, NULL, NULL,

/* cut rope 2 */
VERB_CUT, OBJ_ROPE_2, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   240, DONE, NULL, NULL,

/* cut rope 3 */
VERB_CUT, OBJ_ROPE_3, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   240, DONE, NULL, NULL,

/* cut rope 4 */
VERB_CUT, OBJ_ROPE_4, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   240, DONE, NULL, NULL,

/* cut rope 5 */
VERB_CUT, OBJ_ROPE_5, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   240, DONE, NULL, NULL,

/* tie ladder down tree (rope 5) */
VERB_TIE, OBJ_ROPE_5, ANY,
   IN,-1600, PAR,OBJ_LADDER, PAR,OBJ_ROPE_5, PAR,OBJ_LADDER_1, PAR,0, PAR,0,
   CREATE, DESTROY, CREATE, OK,

/* untie rope 5 */
VERB_UNTIE, OBJ_ROPE_5, ANY,
   STATE,OBJ_ROPE_5, NULL,0, PAR,OBJ_ROPE_3, PAR,0, PAR,0, PAR,0,
   241, INC, NULL, NULL,           

/* turn ladder into pure rope (rope 5) */
VERB_UNTIE, OBJ_ROPE_5, ANY,
   STATE,OBJ_ROPE_5, NULL,0, PAR,OBJ_ROPE_5, PAR,OBJ_LADDER, PAR,OBJ_ROPE_1, PAR,0,
   INC, INC, INC, DONE,

/* untie (rope 5) */
VERB_UNTIE, OBJ_ROPE_5, ANY,
   STATE,OBJ_ROPE_5, NULL,2, PAR,OBJ_ROPE_3, PAR,0, PAR,0, PAR,0,
   NULL, DEC, NULL, NULL,           

/* untie rope knot (rope 5) */
VERB_UNTIE, OBJ_ROPE_5, ANY,
   STATE,OBJ_ROPE_5, NULL,2, PAR,OBJ_ROPE_5, PAR,OBJ_LADDER, PAR,OBJ_ROPE_1, PAR,0,
   DEC, DEC, DEC, OK,

/* knot (rope 5) */
VERB_KNOT, OBJ_ROPE_5, ANY,
   STATE,OBJ_ROPE_5, NULL,2, PAR,0, PAR,0, PAR,0, PAR,0,
   243, DONE, NULL, NULL,

/* knot (rope 5) */
VERB_KNOT, OBJ_ROPE_5, ANY,
   STATE,OBJ_ROPE_5, NULL,0, PAR,0, PAR,0, PAR,0, PAR,0,
   242, DONE, NULL, NULL,

/* knot (rope 5) */
VERB_KNOT, OBJ_ROPE_5, ANY,
   STATE,OBJ_ROPE_5, NULL,1, PAR,OBJ_ROPE_3, PAR,0, PAR,0, PAR,0,
   INC, NULL, NULL, NULL,

/* knot (rope 5) */
VERB_KNOT, OBJ_ROPE_5, ANY,
   STATE,OBJ_ROPE_5, NULL,1, PAR,OBJ_ROPE_1, PAR,OBJ_LADDER, PAR,OBJ_ROPE_5, PAR,0,
   INC, INC, INC, OK,

/* up */
VERB_UP, ANY, ANY,
   HERE,OBJ_LADDER, PAR,OBJ_LADDER_1, NOT|IFFIGHTING,0, PAR,0, PAR,0, PAR,0,
   GOBY, DONE, NULL, NULL,

/* down */
VERB_DOWN, ANY, ANY,
   HERE,OBJ_LADDER_1, PAR,OBJ_LADDER, NOT|IFFIGHTING,0, PAR,0, PAR,0, PAR,0,
   GOBY, DONE, NULL, NULL,

/* knot (rope 5) */
VERB_KNOT, OBJ_ROPE_5, ANY,
   IN,-620, PAR,OBJ_ROPE_5, PAR,OBJ_LADDER, PAR,OBJ_LADDER_1, PAR,0, PAR,0,
   DESTROY, CREATE, CREATE, 244,

/* knot (rope 5) */
VERB_KNOT, OBJ_ROPE_5, ANY,
   IN,-620, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   DONE, NULL, NULL, NULL,

/* knot (rope 5) */
VERB_KNOT, OBJ_ROPE_5, ANY,
   IN,-1600, PAR,OBJ_ROPE_5, PAR,OBJ_LADDER, PAR,OBJ_LADDER_1, PAR,0, PAR,0,
   DESTROY, CREATE, CREATE, 244,

/* knot (rope 5) */
VERB_KNOT, OBJ_ROPE_5, ANY,
   IN,-1600, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   DONE, NULL, NULL, NULL,

/* untie ladder */
VERB_UNTIE, OBJ_LADDER, ANY,
   PAR,OBJ_ROPE_5, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   CREATE, NULL, NULL, NULL,            

/* untie ladder */
VERB_UNTIE, OBJ_LADDER, ANY,
   PAR,OBJ_LADDER, PAR,OBJ_LADDER_1, PAR,OBJ_ROPE_5, PAR,FL_CURCH, PAR,0, PAR,0,
   DESTROY, DESTROY, PUT, OK,

/* untie ladder 1 */
VERB_UNTIE, OBJ_LADDER_1, ANY,
   PAR,OBJ_ROPE_5, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   CREATE, NULL, NULL, NULL,            

/* untie ladder 1 */
VERB_UNTIE, OBJ_LADDER_1, ANY,
   PAR,OBJ_LADDER, PAR,OBJ_LADDER_1, PAR,OBJ_ROPE_5, PAR,FL_CURCH, PAR,0, PAR,0,
   DESTROY, DESTROY, PUT, OK,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-620, PAR,OBJ_ROPE_5, PAR,OBJ_LADDER, PAR,OBJ_LADDER_1, PAR,0, PAR,0,
   DESTROY, CREATE, CREATE, 244,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-620, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   DONE, NULL, NULL, NULL,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-1600, PAR,OBJ_ROPE_5, PAR,OBJ_LADDER, PAR,OBJ_LADDER_1, PAR,0, PAR,0,
   DESTROY, CREATE, CREATE, 244,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-1600, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   DONE, NULL, NULL, NULL,                      

/* knot (rope 5) */
VERB_KNOT, OBJ_ROPE_5, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   245, DONE, NULL, NULL,

/* west */
VERB_WEST, ANY, ANY,
   IN,-2200, HERE,OBJ_ROPE_4, PAR,-696, NOT|IFFIGHTING,0, PAR,0, PAR,0,
   GOTO, DONE, NULL, NULL,

/* east */
VERB_EAST, ANY, ANY,
   IN,-696, HERE,OBJ_ROPE_3, PAR,-2200, NOT|IFFIGHTING,0, PAR,0, PAR,0,
   GOTO, DONE, NULL, NULL,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-2200, CHANCE,50, PAR,0, PAR,0, PAR,0, PAR,0,
   252, DONE, NULL, NULL,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-2200, PAR,0, PAR,0, PAR,0, PAR,0,PAR,0,
   251, NULL, NULL, NULL,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-2200, PAR,OBJ_ROPE_3, PAR,OBJ_ROPE_4, PAR,OBJ_ROPE_5, PAR,0, PAR,0,
   CREATE, CREATE, DESTROY, DONE,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-696, CHANCE,50, PAR,0, PAR,0, PAR,0, PAR,0,
   252, DONE, NULL, NULL,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-696, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   251, NULL, NULL, NULL,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-2207, PAR,OBJ_ROPE_1, PAR,OBJ_ROPE_2, PAR,OBJ_ROPE_5, PAR,0, PAR,0,
   CREATE, CREATE, DESTROY, 257,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-2207, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   DONE, NULL, NULL, NULL,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-702, PAR,OBJ_ROPE_2, PAR,OBJ_ROPE_1, PAR,OBJ_ROPE_5, PAR,0, PAR,0,
   CREATE, CREATE, DESTROY, OK,

/* throw (rope 5) */
VERB_THROW, OBJ_ROPE_5, ANY,
   IN,-696, PAR,OBJ_ROPE_3, PAR,OBJ_ROPE_4, PAR,OBJ_ROPE_5, PAR,0, PAR,0,
   CREATE, CREATE, DESTROY, OK,
            
/* tie (rope 5) */
VERB_TIE, OBJ_ROPE_5, ANY,
   IN,-702, PAR,OBJ_ROPE_1, PAR,OBJ_ROPE_2, PAR,OBJ_ROPE_5, PAR,0, PAR,0,
   CREATE, CREATE, DESTROY, 258,

/* tie (rope 5) */
VERB_TIE, OBJ_ROPE_5, ANY,
   IN,-702, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   DONE, NULL, NULL, NULL,

/* untie (rope 1) */
VERB_UNTIE, OBJ_ROPE_1, ANY,
   PAR,230, PAR,231, PAR,234, PAR,234, PAR,FL_MYNUM, PAR,0,
   DESTROY, DESTROY, CREATE, PUTCARR,

/* untie (rope 1) */
VERB_UNTIE, OBJ_ROPE_1, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   OK, NULL, NULL, NULL,

/* untie (rope 2) */
VERB_UNTIE, OBJ_ROPE_2, ANY,
   PAR,230, PAR,231, PAR,234, PAR,234, PAR,FL_MYNUM, PAR,0,
   DESTROY, DESTROY, CREATE, PUTCARR,

/* untie (rope 2) */
VERB_UNTIE, OBJ_ROPE_2, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   OK, NULL, NULL, NULL,

/* tie (rope 3) */
VERB_UNTIE, OBJ_ROPE_3, ANY,
   PAR,232, PAR,233, PAR,234, PAR,234, PAR,FL_MYNUM, PAR,0,
   DESTROY, DESTROY, CREATE, PUTCARR,

/* tie (rope 3) */
VERB_UNTIE, OBJ_ROPE_3, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   OK, NULL, NULL, NULL,

/* untie (rope 4) */
VERB_UNTIE, OBJ_ROPE_4, ANY,
   PAR,232, PAR,233, PAR,234, PAR,234, PAR,FL_MYNUM, PAR,0,
   DESTROY, DESTROY, CREATE, PUTCARR,

/* untie (rope 4) */
VERB_UNTIE, OBJ_ROPE_4, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   OK, NULL, NULL, NULL, 

/* turn (rope 3) */
VERB_TURN, OBJ_ROPE_3, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   253, DONE, NULL, NULL,

/* turn (rope 4) */
VERB_TURN, OBJ_ROPE_4, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   253, DONE, NULL, NULL,

/* down */
VERB_DOWN, ANY, ANY,
   IN,-702, HERE,OBJ_ROPE_1, PAR,-2207, NOT|IFFIGHTING,0, PAR,0, PAR,0,
   GOTO, DONE, NULL, NULL,

/* up */
VERB_UP, ANY, ANY,
   IN,-2207, HERE,OBJ_ROPE_2, PAR,-702, NOT|IFFIGHTING,0, PAR,0, PAR,0,
   GOTO, DONE, NULL, NULL,

/* get ladder */
VERB_GET, OBJ_LADDER, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   266, DONE, NULL, NULL,

/* get ladder 1 */
VERB_GET, OBJ_LADDER_1, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   266, DONE, NULL, NULL,

/* get rope 1 */
VERB_GET, OBJ_ROPE_1, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   266, DONE, NULL, NULL,

/* get rope 2 */
VERB_GET, OBJ_ROPE_2, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   266, DONE, NULL, NULL,

/* get rope 3 */
VERB_GET, OBJ_ROPE_3, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   266, DONE, NULL, NULL,

/* get rope 4 */
VERB_GET, OBJ_ROPE_4, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   266, DONE, NULL, NULL,

/* setin */
VERB_SETIN, ANY, ANY,
   NOT|WIZ,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   204, DONE, NULL, NULL,

/* setout */
VERB_SETOUT, ANY, ANY,
   NOT|WIZ,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   204, DONE, NULL, NULL,

/* setmin */
VERB_SETMIN, ANY, ANY,
   NOT|WIZ,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   204, DONE, NULL, NULL,

/* setmout */
VERB_SETMOUT, ANY, ANY,
   NOT|WIZ,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   204, DONE, NULL, NULL,

/* setvin */
VERB_SETVIN, ANY, ANY,
   NOT|WIZ,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   204, DONE, NULL, NULL,

/* setvout */
VERB_SETVOUT, ANY, ANY,
   NOT|WIZ,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   204, DONE, NULL, NULL,

/* setin */
VERB_SETIN, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   SETIN, OK, NULL, NULL,

/* setout */
VERB_SETOUT, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   SETOUT, OK, NULL, NULL,

/* setmin */
VERB_SETMIN, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   SETMIN, OK, NULL, NULL,

/* setmout */
VERB_SETMOUT, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   SETMOUT, OK, NULL, NULL,

/* setvin */
VERB_SETVIN, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   SETVIN, OK, NULL, NULL,

/* setvout */
VERB_SETVOUT, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   SETVOUT, OK, NULL, NULL,

/* setvout */
VERB_SETPROMPT, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   SETPROMPT, OK, NULL, NULL,

/* dig */
VERB_DIG, ANY, ANY,
   HERE,OBJ_TUNNEL, STATE,OBJ_TUNNEL, NULL,1, PAR,OBJ_TUNNEL, PAR,0, PAR,0,
   SETSTATE, 256, DONE, NULL,

/* dig */
VERB_DIG, ANY, ANY,
   HERE,OBJ_TUNNEL_1, STATE,OBJ_TUNNEL_1, NULL,1, PAR,OBJ_TUNNEL_1, PAR,0, PAR,0,
   SETSTATE, 256, DONE, NULL,

/* dig */
VERB_DIG, ANY, ANY,
   DESTROYED,OBJ_SLAB, IN,RM_CHURCH18, PAR,OBJ_SLAB, PAR,0, PAR,0, PAR,0,
   200, CREATE, DONE, NULL,

/* dig */
VERB_DIG, ANY, ANY,
   HERE,OBJ_FOXHOLE, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   259, DONE, NULL, NULL,

/* dig OBJECT */
/*
VERB_DIG, ANY, ANY,
   HERE,OBJ_HOLE_7, STATE,OBJ_HOLE_7, NULL,1, PAR,OBJ_HOLE_7, PAR,0, PAR,0,
   276, DEC, DONE, NULL,
*/

/* dig OBJECT */
/*
VERB_DIG, ANY, ANY,
   HERE,OBJ_HOLE_8, STATE,OBJ_HOLE_8, NULL,1, PAR,OBJ_HOLE_8, PAR,77, PAR,0,
   DEC, QUIT, NULL, NULL,
*/

/*---------------------------------------------------------------------------*/

/* dig failed (widen hole) */
VERB_DIG, ANY, ANY,
   NOT|HERE,OBJ_HOLE_3, NOT|HERE,OBJ_HOLE_4, PAR,0, PAR,0, PAR,0, PAR,0,
   201, DONE, NULL, NULL,

/* dig with OBJECT */
VERB_DIG, ANY, OB,
   OBIT,FL_OB2, NULL,ofl(Weapon), STATE,OBJ_HOLE_3, NULL,1, PAR,OBJ_HOLE_3, PAR,0,
   203, DEC, DONE, NULL,

/* dig OBJECT */
VERB_DIG, OB, ANY,
   OBIT,FL_OB1, NULL,ofl(Weapon), STATE,OBJ_HOLE_3, NULL,1, PAR,OBJ_HOLE_3, PAR,0,
   203, DEC, DONE, NULL,

/* dig */
VERB_DIG, ANY, ANY,
   STATE,OBJ_HOLE_3, NULL,0, PAR,0, PAR,0, PAR,0, PAR,0,
   202, DONE, NULL, NULL,

/*---------------------------------------------------------------------------*/

/* dig */
VERB_DIG, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   254, DONE, NULL, NULL,

/* open tree */
VERB_OPEN, OBJ_TREE, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   238, DONE, NULL, NULL,

/* open tree 1 */
VERB_OPEN, OBJ_TREE_1, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   238, DONE, NULL, NULL,

/* light tree 1 */
VERB_LIGHT, OBJ_TREE_1, ANY,
   HASANY,ofl(Lit), PAR,OBJ_TREE, PAR,-524, PAR,100, PAR,0, PAR,0,
   239, DESTROY, GOTO, GAIN,

/* tickle tree 1 */
VERB_TICKLE, OBJ_TREE_1, ANY,
   PAR,OBJ_TREE, PAR,-524, PAR,100, PAR,0, PAR,0, PAR,0,
   246, DESTROY, GOTO, GAIN,

/* light tree 1 */
VERB_LIGHT, OBJ_TREE_1, ANY,
   HASANY,ofl(Lit), PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   DONE, NULL, NULL, NULL,

/* pet ottimo */
VERB_PET, P(MOB_OTTIMO), ANY,
   PHERE,MOB_OTTIMO, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   269, DONE, NULL, NULL,

/* jump from lower ledge with umbrella */
VERB_JUMP, ANY, ANY,
   IN,-2441, GOT,OBJ_UMBRELLA, PAR,-2443, PAR,0, PAR,0, PAR,0,
   270, GOTO, DONE, NULL,

/* jump from lower ledge with umbrella */
VERB_JUMP, ANY, ANY,
   IN,-2442, GOT,OBJ_UMBRELLA, PAR,-2443, PAR,0, PAR,0, PAR,0,
   270, GOTO, DONE, NULL,

/* jump from upper ledge */
VERB_JUMP, ANY, ANY,
   IN,-2437, PAR,-2439, PAR,0, PAR,0, PAR,0, PAR,0,
   271, GOTO, DONE, NULL,

/* jump from upper ledge */
VERB_JUMP, ANY, ANY,
   IN,-2438, PAR,-2440, PAR,0, PAR,0, PAR,0, PAR,0,
   271, GOTO, DONE, NULL,

/* jump from lower ledge */
VERB_JUMP, ANY, ANY,
   IN,-2441, PAR,-2439, PAR,0, PAR,0, PAR,0, PAR,0,
   272, GOTO, DONE, NULL,

/* jump from lower ledge */
VERB_JUMP, ANY, ANY,
   IN,-2442, PAR,-2440, PAR,0, PAR,0, PAR,0, PAR,0,
   272, GOTO, DONE, NULL,

/* look */
VERB_LOOK, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   LOOK, DONE, NULL, NULL,

/* look */
VERB_LOOK, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   DISPROOM, DONE, NULL, NULL,

/* inv */
VERB_INVENTORY, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   INVEN, DONE, NULL, NULL,

/* inv */
VERB_INVENTORY, ANY, ANY, 
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   WHO, DONE, NULL, NULL,

/* summon */
VERB_SUMMON, ANY, ANY,
   IN,-500, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   234, DONE, NULL, NULL,

/* shout */
VERB_SHOUT, ANY, ANY,
   IFDUMB,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   207, DONE, NULL, NULL,

/* tell */
VERB_TELL, ANY, ANY,
   IFDUMB,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   207, DONE, NULL, NULL,

/* shout */
VERB_SHOUT, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   SHOUT, OK, NULL, NULL,

/* tell */
VERB_TELL, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   TELL, OK, NULL, NULL,

/* score */
VERB_SCORE, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   SCORE, DONE, NULL, NULL,

/* exorcise */
VERB_EXORCISE, ANY, ANY,
   NOT|WIZ,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   208, DONE, NULL, NULL,

/* exorcise */
VERB_EXORCISE, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   EXORCISE, DONE, NULL, NULL,

/* put */
VERB_PUT, OBJ_UMBRELLA, ANY,
   STATE,1, NULL,1, PAR,0, PAR,0, PAR,0, PAR,0,
   232, DONE, NULL, NULL,

/* open floorboards */
VERB_OPEN, OBJ_FLOORBOARDS, ANY,
   STATE,OBJ_FLOORBOARDS, NULL,1, PAR,OBJ_FLOORBOARDS, PAR,0, PAR,0, PAR,0,
   229, SETSTATE, DONE, NULL,

/* open */
VERB_OPEN, OBJ_FLOORBOARDS_1, ANY,
   STATE,OBJ_FLOORBOARDS_1, NULL,1, PAR,OBJ_FLOORBOARDS_1, PAR,0, PAR,0, PAR,0,
   229, SETSTATE, DONE, NULL,

/* open */
VERB_PUSH, OBJ_FLOORBOARDS, ANY,
   STATE,OBJ_FLOORBOARDS, NULL,1, PAR,OBJ_FLOORBOARDS, PAR,0, PAR,0, PAR,0,
   229, SETSTATE, DONE, NULL,

/* open */
VERB_PUSH, OBJ_FLOORBOARDS_1, ANY,
   STATE,OBJ_FLOORBOARDS_1, NULL,1, PAR,OBJ_FLOORBOARDS_1, PAR,0, PAR,0, PAR,0,
   229, SETSTATE, DONE, NULL,

/* open */
VERB_OPEN, OBJ_DOOR_1, ANY,
   STATE,OBJ_DOOR_1, NULL,0, PAR,0, PAR,0, PAR,0, PAR,0,
   210, DONE, NULL, NULL,

/* open */
VERB_OPEN, OBJ_DOOR_1, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   209, DONE, NULL, NULL,

/* open */
VERB_OPEN, OBJ_UMBRELLA, ANY,
   STATE,OBJ_UMBRELLA, NULL,1, PAR,0, PAR,0, PAR,0, PAR,0,
   210, DONE, NULL, NULL,

/* open */
VERB_OPEN, OBJ_UMBRELLA, ANY,
   PAR,OBJ_UMBRELLA, PAR,1, PAR,0, PAR,0, PAR,0, PAR,0,
   SETSTATE, 211, DONE, NULL,

/* open */
VERB_OPEN, OBJ_DOOR, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   212, DONE, NULL, NULL,

/* open (can't open) */
VERB_OPEN, OB, ANY,
   NOT|OBIT,FL_OB1, NULL,2, PAR,0, PAR,0, PAR,0, PAR,0,
   213, DONE, NULL, NULL,

/* open */
VERB_OPEN, OB, ANY,
   STATE,FL_OB1, NULL,0, PAR,0, PAR,0, PAR,0, PAR,0,
   214, DONE, NULL, NULL,

/* open x with key */
VERB_OPEN, OB, ANY,
   STATE,FL_OB1, NULL,2, HASANY,ofl(Key), PAR,FL_OB1, PAR,0, PAR,0,
   DEC, NULL, NULL, NULL,

/* open (locked) */
VERB_OPEN, OB, ANY,
   STATE,FL_OB1, NULL,2, PAR,0, PAR,0, PAR,0, PAR,0,
   215, DONE, NULL, NULL,

/* open */
VERB_OPEN, OB, ANY,
   PAR,FL_OB1, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   DEC, OK, NULL, NULL,

/* close */
VERB_CLOSE, OBJ_UMBRELLA, ANY,
   STATE,OBJ_UMBRELLA, NULL,0, PAR,0, PAR,0, PAR,0, PAR,0,
   216, DONE, NULL, NULL,

/* close */
VERB_CLOSE, OBJ_UMBRELLA, ANY,
   PAR,OBJ_UMBRELLA, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   SETSTATE, OK, NULL, NULL,

/* close */
VERB_CLOSE, OB, ANY,
   NOT|OBIT,FL_OB1, NULL,2, PAR,0, PAR,0, PAR,0, PAR,0,
   217, DONE, NULL, NULL,

/* close */
VERB_CLOSE, OB, ANY,
   NOT|STATE,FL_OB1, NULL,0, PAR,0, PAR,0, PAR,0, PAR,0,
   218, DONE, NULL, NULL,

/* close */
VERB_CLOSE, OB, ANY,
   PAR,FL_OB1, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   INC, OK, NULL, NULL,

/* lock */
VERB_LOCK, OB, ANY,
   NOT|HASANY,ofl(Key), PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   222, DONE, NULL, NULL,

/* lock */
VERB_LOCK, OB, ANY,
   NOT|OBIT,FL_OB1, NULL,3, PAR,0, PAR,0, PAR,0, PAR,0,
   223, DONE, NULL, NULL,

/* lock */
VERB_LOCK, OB, ANY,
   STATE,FL_OB1, NULL,2, PAR,0, PAR,0, PAR,0, PAR,0,
   224, DONE, NULL, NULL,

/* lock */
VERB_LOCK, OB, ANY,
   PAR,FL_OB1, PAR,2, PAR,0, PAR,0, PAR,0, PAR,0,
   SETSTATE, OK, NULL, NULL,

/* unlock */
VERB_UNLOCK, OB, ANY,
   NOT|HASANY,ofl(Key), PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   222, DONE, NULL, NULL,

/* unlock */
VERB_UNLOCK, OB, ANY,
   NOT|OBIT,FL_OB1, NULL,3, PAR,0, PAR,0, PAR,0, PAR,0,
   226, DONE, NULL, NULL,

/* unlock */
VERB_UNLOCK, OB, ANY,
   NOT|STATE,FL_OB1, PAR,2, PAR,0, PAR,0, PAR,0, PAR,0,
   225, DONE, NULL, NULL,

/* unlock */
VERB_UNLOCK, OB, ANY,
   PAR,FL_OB1, PAR,1, PAR,0, PAR,0, PAR,0, PAR,0,
   SETSTATE, OK, NULL, NULL,

/* dragon code; icy dagger */
VERB_HIT, P(MOB_DRAGON), OBJ_DAGGER_1,
   PAR,MOB_DRAGON, PAR,1000, PAR,300, PHERE,MOB_DRAGON, PAR,0, PAR,0,
   221, WOUNDMON, GAIN, DONE,

/* hit dragon */
VERB_HIT, P(MOB_DRAGON), NONE,
   WPN,OBJ_DAGGER_1, PAR,MOB_DRAGON, PAR,1000, PAR,300, PHERE,MOB_DRAGON, PAR,0,
   221, WOUNDMON, GAIN, DONE,

/* read (bone) */
VERB_READ, OBJ_BONE, ANY,
   IN,-212, PAR,-420, PAR,0, PAR,0, PAR,0, PAR,0,
   GOTO, 220, DONE, NULL,

/* read (bone) */
VERB_READ, OBJ_BONE, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   219, DONE, NULL, NULL,

/* hit the bell; bong! */
VERB_HIT, OBJ_BELL, ANY,
   PAR,28, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   BROAD, DONE, NULL, NULL,

/* ring the bell; bong! */
VERB_RING, OBJ_BELL, ANY,
   PAR,28, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   BROAD, DONE, NULL, NULL,

/* hit pebble */
VERB_HIT, OBJ_PEBBLE, ANY,
   PAR,OBJ_PEBBLE, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   231, DESTROY, DONE, NULL,

/* sit on throne */
VERB_SIT, OBJ_THRONE_3, ANY,
   PAR,OBJ_THRONE_3, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   273, SETSTATE, DONE, NULL,

/* null entry */
-1, 0, 0,
  PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
  0, 0, 0, 0,
};

/*
** Interrupts (called every two seconds via alarm).
*/
LINE inter[] = {

/* 15% chance of the tree swallowing you */
0, ANY, ANY,
   HERE,OBJ_TREE_1, CHANCE,15, NOT|WIZ,0, PAR,OBJ_TREE, PAR,37, PAR,36,
   DEC, BROAD, QUIT, NULL,

/* Occasionally display "Fox barks" message at fox hole */
0, ANY, ANY,
   HERE,OBJ_FOXHOLE, CHANCE,5, PAR,0, PAR,0, PAR,0, PAR,0,
   267, NULL, NULL, NULL,

/* Ice dagger melts if you take it into Hell */
0, ANY, ANY,
   INLT,-2499, NOT|INLT,-2599, AVL,OBJ_DAGGER_1, CHANCE,25, PAR,OBJ_DAGGER_1, PAR,RM_PIT1,
   278, PUT, NULL, NULL,

/* Check for NOLOGIN file */
0, ANY, ANY,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   CHECK_NOLOGIN, 0, 0, 0,

/* Check for new mail */
0, ANY, ANY,
   CHANCE,5, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   CHECK_MAIL, NULL, NULL, NULL,

-1, 0, 0,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   0, 0, 0, 0
};

/*
** Status (called after each action is done).
*/
LINE status[] = {

/* 10% chance that tree will suck you in */
0, ANY, ANY,
   HERE,OBJ_TREE, STATE,OBJ_TREE, NULL,0, CHANCE,10, PAR,-500, PAR,0,
   235, GOTO, NULL, NULL,

/* If Ottimo's by the fox hole and has a witness, dig it up */
0, ANY, ANY,
   HERE,OBJ_FOXHOLE, PHERE,MOB_OTTIMO, STATE,OBJ_FOXHOLE, NULL,1, PAR,OBJ_FOXHOLE, PAR,0,
   250, SETSTATE, NULL, NULL,

/* Crack the altar if the cross is brought near it */
0, ANY, ANY,
   HERE,OBJ_ALTAR, AVL,OBJ_CROSS, NOT|WIZ,0, PAR,OBJ_ALTAR, PAR,100, PAR,0,
   DESTROY, GAIN, 263, NULL,

-1, 0, 0,
   PAR,0, PAR,0, PAR,0, PAR,0, PAR,0, PAR,0,
   0, 0, 0, 0, 0
};

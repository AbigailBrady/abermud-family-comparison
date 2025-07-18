# Makefile for Phoenix

EXEC    = abermud
HOMEDIR	= /p/src/games/AberMud/abermud-new
UNIVERSE= $(HOMEDIR)/world_file
FFLAGS	= -DEXE_FILE="\"${EXEC}\"" -DGAME_DIR="\"${HOMEDIR}\""

CC	= gcc
CPP	= $(CC) -E
LFLAGS	=
CFLAGS	= -O -g

S_GEN = generate.c blib.c
O_GEN = $(S_GEN:.c=.o)

S_MUD = action.c blood.c bootstrap.c bprintf.c change.c condition.c \
	database.c exec.c extra.c flags.c frob.c gametime.c god.c key.c \
	locdir.c loctable.c magic.c maint.c mobile.c move.c mail.c mud.c \
	mudprog.c new1.c new2.c objsys.c opensys.c parse.c parser.c sendsys.c \
	support.c tk.c uaf.c weather.c zones.c blib.c bsearch.c crypt.c
O_MUD = $(S_MUD:.c=.o) rooms.o

SRC = ${S_GEN} ${S_MUD}
OBJ = $(SRC:.c=.o)

DATA  = DATA/system/locations \
	DATA/system/reset_data

WORLD =	DATA/player/uaf_rand \
	DATA/player/user_file

all:	generate ${DATA} ${WORLD} ${EXEC} abermudd goaway
#	mkdirs DATA/logs MSGS DESC SNOOP

generate: $(O_GEN) $(LIBS)
${EXEC}: $(O_MUD) $(LIBS)
abermudd: abermudd.c
goaway: goaway.c
convertloc: convertloc.c

generate $(EXEC) abermudd goaway convertloc:
	rm -f $@
	${CC} ${LFLAGS} -o $@ $^

pflags.h pflagnames.h lflags.h lflagnames.h oflags.h oflagnames.h: %: %.awk
	awk -f $^ >$@
pflags.h pflagnames.h: DATA/system/pflags
lflags.h lflagnames.h: DATA/system/lflags
oflags.h oflagnames.h: DATA/system/oflags

data:	${WORLD} ${DATA}

clean:
	rm -f DATA/system/locations $(OBJ) rooms.c verbs.h locations.h \
	mobiles.h locations.h generate convertloc {p,l,o}flag*.h

install: all
	chown mud.abermud abermud
	chmod 770 abermud
	
world: ${WORLD}

strip:
	strip abermud

depend: $(SRC)
	touch Makefile.depend
	mkdep -f Makefile.depend $(CFLAGS) $(FFLAGS) $(SRC)
Makefile.depend:

.c.o:
	${CC} ${CFLAGS} $(FFLAGS) -c $*.c


rooms.c: DATA/system/locations convertloc
	./convertloc

mobiles.h: DATA/system/mobiles generate
	./generate $* DATA/system/mobiles $@

locations.h: DATA/system/zones DATA/system/locations generate
	./generate $* DATA/system/zones DATA/system/locations $@

objects.h: DATA/system/objects generate
	./generate $* DATA/system/objects $@

verbs.h: DATA/system/verbs generate
	./generate $* DATA/system/verbs $@

universe: $(UNIVERSE)
${UNIVERSE}: DATA/system/reset_data generate
	./generate world ${UNIVERSE}

DATA/system/locations:	DATA/system/locations.[1-6]
	cat DATA/system/locations.[1-6] > DATA/system/locations

DATA/system/reset_data: DATA/system/reset.txt generate
	./generate reset DATA/system/reset.txt DATA/system/reset_data

DATA/system/reset.txt: DATA/system/reset.c objects.h mobiles.h locations.h \
	generate
	$(CPP) DATA/system/reset.c > DATA/system/reset.txt


DATA/player/uaf_rand:
	./generate uaf DATA/player/uaf_rand

DATA/player/user_file:
	./generate userfile DATA/player/user_file

aberwho: who.o opensys.o blib.o
	gcc -o aberwho who.o opensys.o blib.o

include Makefile.depend

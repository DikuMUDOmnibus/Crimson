# all includes checked against makefile as of Jan 31/95
# -Cryogen 
#
# further everything compiles with the -Wall option (whew!)
# no warnings, no errors (no kidding)
CC=gcc
CFLAGS=-g

dmserver : comm.o act_comm.o act_info.o act_move.o act_obj1.o \
	act_obj2.o act_offensive.o act_other.o act_social.o act_wizard.o \
	handler.o db.o interpreter.o utility.o spec_assign.o shop.o limits.o \
	mobact.o fight.o modify.o weather.o spell_parser.o spells1.o \
	spells2.o reception.o constants.o silver.o ban.o g_hill.o citadel.o \
	tuscany.o academy.o spec_procs.o signals.o boards.o act_create.o               magic.o 

#comm.o :  structs.h utility.h comm.h interpreter.h handler.h db.h \
#	limits.h ban.h ansi.h weather.h reception.h signals.h
#	$(CC) -c $(CFLAGS) 
ban.o : ban.c structs.h utility.h comm.h interpreter.h handler.h db.h \
	constants.h ban.h
	$(CC) -c $(CFLAGS) ban.c
act_comm.o : act_comm.c structs.h utility.h comm.h db.h handler.h \
	 interpreter.h constants.h spells.h ansi.h act.h
	$(CC) -c $(CFLAGS) act_comm.c
act_info.o : act_info.c structs.h utility.h comm.h db.h handler.h\
  	limits.h interpreter.h spells.h constants.h modify.h ansi.h\
	weather.h act.h
	$(CC) -c $(CFLAGS) act_info.c
act_move.o : act_move.c structs.h utility.h comm.h db.h handler.h \
  	limits.h interpreter.h constants.h spells.h ansi.h fight.h ansi.h \
	act.h
	$(CC) -c $(CFLAGS) act_move.c
act_obj1.o : act_obj1.c structs.h utility.h comm.h db.h handler.h \
  	interpreter.h constants.h spells.h act.h
	$(CC) -c $(CFLAGS) act_obj1.c
act_obj2.o : act_obj2.c structs.h utility.h comm.h db.h handler.h \
  	limits.h interpreter.h spells.h constants.h act.h
	$(CC) -c $(CFLAGS) act_obj2.c
act_offensive.o : act_offensive.c structs.h utility.h comm.h interpreter.h \
  	handler.h db.h limits.h constants.h spells.h fight.h act.h
	$(CC) -c $(CFLAGS) act_offensive.c
act_other.o : act_other.c structs.h utility.h comm.h db.h handler.h \
  	limits.h interpreter.h fight.h spells.h constants.h act.h
	$(CC) -c $(CFLAGS) act_other.c
act_social.o : act_social.c structs.h utility.h comm.h interpreter.h \
  	handler.h db.h spells.h act.h
	$(CC) -c $(CFLAGS) act_social.c
act_wizard.o : act_wizard.c structs.h utility.h comm.h db.h handler.h \
  	limits.h interpreter.h constants.h spells.h reception.h act.h
	$(CC) -c $(CFLAGS) act_wizard.c
handler.o : handler.c structs.h utility.h interpreter.h comm.h db.h \
	handler.h fight.h spells.h constants.h ansi.h
	$(CC) -c $(CFLAGS) handler.c 
db.o : db.c structs.h utility.h db.h comm.h handler.h limits.h constants.h \
	ban.h weather.h
	$(CC) -c $(CFLAGS) db.c
interpreter.o : interpreter.c structs.h utility.h comm.h db.h handler.h \
  	limits.h constants.h act.h ban.h interpreter.h
	$(CC) -c $(CFLAGS) interpreter.c 
utility.o : utility.c structs.h utility.h
	$(CC) -c $(CFLAGS) utility.c
spec_assign.o : spec_assign.c structs.h db.h
	$(CC) -c $(CFLAGS) spec_assign.c
spec_procs.o : spec_procs.c structs.h utility.h comm.h interpreter.h \
  	handler.h limits.h db.h constants.h spells.h fight.h act.h
	$(CC) -c $(CFLAGS) spec_procs.c
limits.o : limits.c limits.h structs.h utility.h spells.h comm.h
	$(CC) -c $(CFLAGS) limits.c
fight.o : fight.c structs.h utility.h comm.h db.h handler.h limits.h \
  	interpreter.h constants.h spells.h ansi.h act.h fight.h
	$(CC) -c $(CFLAGS) fight.c
weather.o : weather.c structs.h utility.h comm.h
	$(CC) -c $(CFLAGS) weather.c
shop.o : shop.c structs.h comm.h handler.h db.h interpreter.h utility.h \
	constants.h act.h
	$(CC) -c $(CFLAGS) shop.c
spells1.o : spells1.c structs.h utility.h comm.h db.h interpreter.h spells.h \
  	handler.h
	$(CC) -c $(CFLAGS) spells1.c 
spells2.o : spells2.c structs.h utility.h comm.h db.h interpreter.h spells.h \
  	handler.h
	$(CC) -c $(CFLAGS) spells2.c 
magic.o : magic.c structs.h utility.h comm.h db.h handler.h limits.h \
  	constants.h fight.h act.h spells.h
	$(CC) -c $(CFLAGS) magic.c 
spell_parser.o : spell_parser.c structs.h utility.h comm.h db.h interpreter.h \
  	spells.h handler.h constants.h
	$(CC) -c $(CFLAGS) spell_parser.c 
mobact.o : mobact.c structs.h utility.h comm.h db.h handler.h constants.h \
	fight.h act.h
	$(CC) -c $(CFLAGS) mobact.c
modify.o : modify.c structs.h utility.h interpreter.h handler.h db.h comm.h \
	modify.h
	$(CC) -c $(CFLAGS) modify.c
reception.o : reception.c structs.h utility.h comm.h db.h handler.h \
  	interpreter.h spells.h constants.h act.h reception.h
	$(CC) -c $(CFLAGS) reception.c
constants.o : constants.c limits.h structs.h constants.h
	$(CC) -c $(CFLAGS) constants.c
boards.o : boards.c structs.h utility.h comm.h db.h interpreter.h \
	handler.h constants.h modify.h boards.h
	$(CC) -c $(CFLAGS) boards.c
signals.o : signals.c structs.h utility.h signals.h act.h
	$(CC) -c $(CFLAGS) signals.c
act_create.o : act_create.c structs.h utility.h comm.h db.h handler.h \
  	limits.h interpreter.h spells.h constants.h modify.h act.h
	$(CC) -c $(CFLAGS) act_create.c

# special code for areas
citadel.o : citadel.c structs.h utility.h comm.h interpreter.h \
  	handler.h db.h spells.h limits.h constants.h act.h
	$(CC) -c $(CFLAGS) citadel.c
g_hill.o : g_hill.c structs.h utility.h comm.h interpreter.h \
  	handler.h db.h spells.h limits.h constants.h act.h
	$(CC) -c $(CFLAGS) g_hill.c
silver.o : silver.c structs.h utility.h comm.h interpreter.h \
  	handler.h db.h spells.h limits.h constants.h act.h
	$(CC) -c $(CFLAGS) silver.c
tuscany.o : tuscany.c structs.h utility.h comm.h interpreter.h handler.h db.h 	           limits.h constants.h act.h ansi.h	
	    $(CC) -c $(CFLAGS) tuscany.c
academy.o : academy.c structs.h utility.h comm.h interpreter.h handler.h db.h 	           limits.h constants.h act.h spells.h ansi.h	
	    $(CC) -c $(CFLAGS) academy.c

# utilities code - I've moved this code to its own dir
list.o : list.c structs.h
	$(CC) $(CFLAGS) -o list list.c
delplay.o : delplay.c structs.h
	$(CC) $(CFLAGS) -o delplay delplay.c


# put it all together
dmserver :  act_comm.c act_info.c act_move.c act_obj1.c act_obj2.c act_offensive.c act_other.c act_social.c act_wizard.c handler.c db.c interpreter.c utility.c spec_assign.c shop.c limits.c mobact.c fight.c modify.c weather.c spells1.c spells2.c spell_parser.c reception.c constants.c ban.c silver.c g_hill.c citadel.c tuscany.c academy.c spec_procs.c signals.c boards.c act_create.c magic.c
	$(CC) -o dmserver $(CFLAGS) comm.o act_comm.o act_info.o act_move.o act_obj1.o act_obj2.o act_offensive.o act_other.o act_social.o act_wizard.o handler.o db.o interpreter.o utility.o spec_assign.o shop.o limits.o mobact.o fight.o modify.o weather.o spells1.o spells2.o spell_parser.o reception.o constants.o ban.o silver.o g_hill.o citadel.o tuscany.o academy.o spec_procs.o signals.o boards.o act_create.o magic.o 

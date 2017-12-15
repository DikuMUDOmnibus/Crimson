/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "constants.h"
#include "act.h"


#define SWORD_ACT \
	"Somewhere somehow you can hear the gods laughing at the sheer audacity\
\n\rof praying for evil intent in this bastion of goodness. A deep voice\
\n\rgrowls 'Such daring shall not go unrewarded!'. A cloud of darkness fills\
\n\rthe room and when it lifts moments later you see the fabled storm blade\
\n\rlying on the alter.\
\n\r"



int citadel_altar(struct char_data *ch, int cmd, char *arg)
{
	/*
	Altar room : 14670

	Special items:
	14611  first book
	14612  second book
	14613  third book
	14614  hilt of sword
	14615  middle of sword
	14616  tip of sword

	Items when dropped in altar room:
	14617  first book
	14618  second book
	14619  third book
	14620  hilt of sword
	14621  middle of sword
	14622  tip of sword
	*/

	struct obj_data *obj;
	static int sword = -1;
	static int book1,  book2,  book3,  hilt,  middle,  tip;
	static int abook1, abook2, abook3, ahilt, amiddle, atip;
	char buf[100];

	if (sword == -1) {
		book1  = 14611;
		book2  = 14612;
		book3  = 14613;
		hilt   = 14614;
		middle = 14615;
		tip    = 14616;

		abook1 = 14617;
		abook2 = 14618;
		abook3 = 14619;
		ahilt  = 14620;
		amiddle= 14621;
		atip   = 14622;

		sword  = 14610;
	}

   	switch(cmd)
   	{
		case CMD_GET:    /* get */
		case CMD_TAKE:    /* get */
			do_get(ch, arg, cmd); /* perform the get */
			/* if got any of the altar ones, switch her */
			if ((obj = get_obj_in_list_virtual(abook1, ch->carrying)))
			{
				extract_obj(obj);
				obj = read_object(book1, VIRTUAL);
				obj_to_char(obj, ch);
         		}
			if ((obj = get_obj_in_list_virtual(abook2, ch->carrying)))
			{
				extract_obj(obj);
				obj = read_object(book2, VIRTUAL);
				obj_to_char(obj, ch);
       	 		}
			if ((obj = get_obj_in_list_virtual(abook3, ch->carrying)))
			{
				extract_obj(obj);
				obj = read_object(book3, VIRTUAL);
				obj_to_char(obj, ch);
         		}
			if ((obj = get_obj_in_list_virtual(ahilt, ch->carrying)))
			{
				extract_obj(obj);
				obj = read_object(hilt, VIRTUAL);
				obj_to_char(obj, ch);
         		}
			if ((obj = get_obj_in_list_virtual(amiddle, ch->carrying)))
			{
				extract_obj(obj);
				obj = read_object(middle, VIRTUAL);
				obj_to_char(obj, ch);
         		}
			if ((obj = get_obj_in_list_virtual(atip, ch->carrying)))
			{
				extract_obj(obj);
				obj = read_object(tip, VIRTUAL);
				obj_to_char(obj, ch);
			}
         		return(1);
		break;

		/* case 67:  put */
		case CMD_DROP: /* drop */
			do_drop(ch, arg, cmd); /* perform the drop */
			/* if got any of the altar ones, switch her */
			if ((obj = get_obj_in_list_virtual(book1, world[ch->in_room].contents)))
			{
				send_to_char("You place the book on the altar\n\r",ch);
				extract_obj(obj);
				obj = read_object(abook1, VIRTUAL);
				obj_to_room(obj, ch->in_room);
			}
			if ((obj = get_obj_in_list_virtual(book2, world[ch->in_room].contents)))
			{
				send_to_char("You place the book on the altar\n\r",ch);
				extract_obj(obj);
				obj = read_object(abook2, VIRTUAL);
				obj_to_room(obj, ch->in_room);
			}
			if ((obj = get_obj_in_list_virtual(book3, world[ch->in_room].contents)))
			{
				send_to_char("You place the book on the altar\n\r",ch);
				extract_obj(obj);
				obj = read_object(abook3, VIRTUAL);
				obj_to_room(obj, ch->in_room);
			}
			if ((obj = get_obj_in_list_virtual(hilt, world[ch->in_room].contents)))
			{
				send_to_char("You place the hilt on the altar\n\r",ch);
				extract_obj(obj);
				obj = read_object(ahilt, VIRTUAL);
				obj_to_room(obj, ch->in_room);
			}
			if ((obj = get_obj_in_list_virtual(middle, world[ch->in_room].contents)))
			{
				send_to_char("You place the sword fragment on the altar\n\r",ch);
				extract_obj(obj);
				obj = read_object(amiddle, VIRTUAL);
				obj_to_room(obj, ch->in_room);
			}
			if ((obj = get_obj_in_list_virtual(tip, world[ch->in_room].contents)))
			{
				send_to_char("You place the sword tip on the altar\n\r",ch);
				extract_obj(obj);
				obj = read_object(atip, VIRTUAL);
				obj_to_room(obj, ch->in_room);
			}
			return(1);
		break;

		case CMD_PRAY: /* pray */
			if ((obj = get_obj_in_list_virtual(abook1, world[ch->in_room].contents)))
			if ((obj = get_obj_in_list_virtual(abook2, world[ch->in_room].contents)))
			if ((obj = get_obj_in_list_virtual(abook3, world[ch->in_room].contents)))
			if ((obj = get_obj_in_list_virtual(ahilt, world[ch->in_room].contents)))
			if ((obj = get_obj_in_list_virtual(amiddle, world[ch->in_room].contents)))
			if ((obj = get_obj_in_list_virtual(atip, world[ch->in_room].contents)))
			{
         		do_action(ch, arg, 176);  /* pray */
			act(SWORD_ACT, TRUE, ch, 0, 0, TO_ROOM);
			act(SWORD_ACT, TRUE, ch, 0, 0, TO_CHAR);

			for (obj = world[ch->in_room].contents;
				  obj;
				  obj=world[ch->in_room].contents
			)
				extract_obj(obj);

			obj = read_object(sword, VIRTUAL);
			obj_to_room(obj, ch->in_room);
			do_gossip(ch,"Hurray! I have solved the Elfin quest!", 232);
			sprintf(buf,"QUEST: %s got a new storm blade.", GET_NAME(ch));
			log(buf);
			return(1);
			}
			else return(0);
      		break;
      
		default:
         		return(0);
      		break;
	}
	return(0);
}



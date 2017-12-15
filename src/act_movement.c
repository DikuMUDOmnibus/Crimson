/* ************************************************************************
*  file: act.movement.c , Implementation of commands      Part of DIKUMUD *
*  Usage : Movement commands, close/open & lock/unlock doors.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "constants.h"
#include "ansi.h"

/*   external vars  */

extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list; 
extern struct index_data *obj_index;
extern int rev_dir[];
extern char *dirs[]; 
extern int movement_loss[];

/* external functs */

int special(struct char_data *ch, int cmd, char *arg);
void death_cry(struct char_data *ch);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name,
	struct obj_data *list);


int do_simple_move(struct char_data *ch, int cmd, int following)
/* Assumes, 
	1. That there is no master and no followers.
	2. That the direction exists. 

   Returns :
   1 : If succes.
   0 : If fail
  -1 : If dead.
*/
{
	char tmp[MAX_INPUT_LENGTH];
	int was_in, to_room;
	int need_movement;
	struct obj_data *obj;
	bool has_boat;
	struct affected_type *hjp, *next_hjp;
	struct char_data *tmp_ch;
	int found = FALSE;

	int special(struct char_data *ch, int cmd, char *arg);

	if (special(ch, cmd+1, ""))  /* Check for special routines (North is 1) */
		return(FALSE);

	need_movement = (movement_loss[world[ch->in_room].sector_type]+
	movement_loss[world[world[ch->in_room].dir_option[cmd]->to_room].sector_type]) / 2;

        if ((world[ch->in_room].sector_type == SECT_WATER_NOSWIM) ||
        (world[world[ch->in_room].dir_option[cmd]->to_room].sector_type == SECT_UNDERWATER) ||
        (world[world[ch->in_room].dir_option[cmd]->to_room].sector_type == SECT_WATER_NOSWIM)) {
                has_boat = FALSE;
                /* See if char is carrying a boat */
                for (obj=ch->carrying; obj; obj=obj->next_content)
                        if (obj->obj_flags.type_flag == ITEM_BOAT)
                                has_boat = TRUE;
                if (!has_boat && world[world[ch->in_room].dir_option[cmd]->to_room].sector_type == SECT_WATER_NOSWIM) {
                        ansi(CLR_ERROR, ch);
                        send_to_char("You need a boat to go there.\n\r", ch);
                        ansi(END, ch);
                        return(FALSE);
                } else
                if (has_boat && world[world[ch->in_room].dir_option[cmd]->to_room].sector_type == SECT_UNDERWATER) {
                        ansi(CLR_ERROR, ch);
                        send_to_char("You can't take a boat underwater.\n\r", ch);
                        ansi(END, ch);
                        return(FALSE);
                }
        }
        if (GET_LEVEL(ch) < IMO_LEV && (
        (IS_SET(world[world[ch->in_room].dir_option[cmd]->to_room].room_flags, LAWFULL) && IS_GOOD(ch)) ||
        (IS_SET(world[world[ch->in_room].dir_option[cmd]->to_room].room_flags, NEUTRAL) && IS_NEUTRAL(ch)) ||
        (IS_SET(world[world[ch->in_room].dir_option[cmd]->to_room].room_flags, CHAOTIC) && IS_EVIL(ch))  )) {
	        ansi(CLR_ERROR, ch);
                send_to_char("An overpowering urge not to enter seizes your mind.\n\r",ch);
		ansi(END, ch);
                return(FALSE);
        }

	if(GET_MOVE(ch)<need_movement && !IS_NPC(ch))
	{
	        ansi(CLR_ERROR, ch);
		if(!following)
			send_to_char("You are too exhausted.\n\r",ch);
		else
			send_to_char("You are too exhausted to follow.\n\r",ch);
		ansi(END, ch);

		return(FALSE);
	}

	if(GET_LEVEL(ch)<IMO_LEV2 && !IS_NPC(ch))
		GET_MOVE(ch) -= need_movement;

	if (!IS_AFFECTED(ch, AFF_SNEAK)) {
		sprintf(tmp, "$n leaves %s.", dirs[cmd]);
		ansi_act(tmp, TRUE, ch, 0,0,TO_ROOM,CLR_ACTION);
	}

	was_in = ch->in_room;
	to_room =  world[was_in].dir_option[cmd]->to_room;
        /* are trying to go to a NOENTER zone */
        if (world[ch->in_room].zone != world[to_room].zone) {
          if (IS_SET(zone_table[world[to_room].zone].flags, ZONE_NOENTER)){
	    ansi(CLR_ERROR, ch);
            send_to_char("You cant go that way (Zone is NOENTER).\n\r",ch);
	    ansi(END, ch);
            return (FALSE);
          }
        }
	char_from_room(ch);
	char_to_room(ch, to_room);
	if (!IS_AFFECTED(ch, AFF_SNEAK))
		ansi_act("$n has arrived.", TRUE, ch, 0,0, TO_ROOM,CLR_ACTION);
	do_look(ch, "\0",15);
        /* are we leaving a testing zone */
        if (world[ch->in_room].zone != world[was_in].zone) {
          if (IS_SET(zone_table[world[was_in].zone].flags, ZONE_TESTING))
            char_del_zone_objs(ch);
        }

        if ((world[ch->in_room].sector_type == SECT_UNDERWATER)
         && GET_LEVEL(ch) < IMO_LEV && !IS_AFFECTED(ch,AFF_BREATHWATER)) {
                ansi_act("You're starting to drown!.",FALSE,ch,0,0,TO_CHAR,CLR_DAM);
                GET_HIT(ch) -= number(1,6);
                update_pos(ch);
        }
        if (IS_SET(world[ch->in_room].room_flags, DRAIN_MAGIC) && GET_LEVEL(ch) < IMO_LEV) {
                GET_MANA(ch) >>= 1;
                for(hjp = ch->affected; hjp; hjp = next_hjp){
                        next_hjp = hjp->next;
                        affect_remove( ch, hjp );
                }
        }
        if (IS_SET(world[ch->in_room].room_flags, HARMFULL1) && GET_LEVEL(ch) < IMO_LEV) {
                ansi_act("Ouch! that hurt.",FALSE,ch,0,0,TO_CHAR,CLR_DAM);
                GET_HIT(ch) -= number(1,4);
                update_pos(ch);
        }
        if (IS_SET(world[ch->in_room].room_flags, HARMFULL2) && GET_LEVEL(ch) < IMO_LEV) {
                ansi_act("Ouch! that really hurt.",FALSE,ch,0,0,TO_CHAR,CLR_DAM);
                GET_HIT(ch) -= number(1,8);
                update_pos(ch);
        }

        switch (GET_POS(ch)) {
                case POSITION_MORTALLYW:
                        ansi_act("$n is mortally wounded, and will die soon, if not aided.", TRUE, ch, 0, 0, TO_ROOM,CLR_HIT);
                        ansi_act("You are mortally wounded, and will die soon, if not aided.", FALSE, ch, 0, 0, TO_CHAR,CLR_DAM);
                        break;
                case POSITION_INCAP:
                        ansi_act("$n is incapacitated and will slowly die, if not aided.", TRUE, ch, 0, 0, TO_ROOM,CLR_HIT);
                        ansi_act("You are incapacitated an will slowly die, if not aided.", FALSE, ch, 0, 0, TO_CHAR,CLR_DAM);
                        break;
                case POSITION_STUNNED:
                        ansi_act("$n is stunned, but will probably regain conscience again.", TRUE, ch, 0, 0, TO_ROOM,CLR_HIT);
                        ansi_act("You're stunned, but will probably regain conscience again.", FALSE, ch, 0, 0, TO_CHAR,CLR_DAM);
                        break;
                case POSITION_DEAD:
                        ansi_act("$n is dead! R.I.P.", TRUE, ch, 0, 0, TO_ROOM,CLR_HIT);
                        ansi_act("You are dead!  Sorry...", FALSE, ch, 0, 0, TO_CHAR,CLR_DAM);
                        return(-1);
                        break;
                default:  
                        if (GET_HIT(ch) < (hit_limit(ch)/5))
                                ansi_act("You wish that your wounds would stop BLEEDING that much!",FALSE,ch,0,0,TO_CHAR,CLR_DAM);
                        break;
        }

	if (IS_SET(world[ch->in_room].room_flags, DEATH) && GET_LEVEL(ch) < IMO_LEV && isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord)) {
		was_in = ch->in_room;
		death_cry(ch);
		extract_char(ch);
		for (obj = world[was_in].contents; obj; obj = world[was_in].contents)
			extract_obj(obj);
		return(-1);
	}

	/* check for auto-aggr to mobs in room */
	if (!IS_NPC(ch)){
		if (IS_SET(ch->specials.act, PLR_AUTOAGGR)) {
			for (tmp_ch = world[ch->in_room].people; 
			     tmp_ch && !found; 
			     tmp_ch=tmp_ch->next_in_room)
				if (IS_NPC(tmp_ch) && IS_SET(tmp_ch->specials.act, ACT_AGGRESSIVE) && !IS_SET(tmp_ch->specials.act, ACT_WIMPY)){
					found = TRUE;
					hit(ch, tmp_ch, TYPE_UNDEFINED);
					WAIT_STATE(ch, PULSE_VIOLENCE);
				}
		}
	} else { /* maybe somebody in the room is autoaggr to us */
		if (IS_SET(ch->specials.act, ACT_AGGRESSIVE) && !IS_SET(ch->specials.act, ACT_WIMPY)) {
			for (tmp_ch = world[ch->in_room].people; 
			     tmp_ch && !found; 
			     tmp_ch=tmp_ch->next_in_room)
				if (!tmp_ch->specials.fighting && !IS_NPC(tmp_ch) && IS_SET(tmp_ch->specials.act, PLR_AUTOAGGR)){
					found = TRUE;
					hit(tmp_ch, ch, TYPE_UNDEFINED);
					WAIT_STATE(tmp_ch, PULSE_VIOLENCE);
				}
		}
	}
	return(1);
}

void do_move(struct char_data *ch, char *argument, int cmd)
{
	char tmp[MAX_INPUT_LENGTH];
	int was_in;
	struct char_data *tch1;
	struct follow_type *k, *next_dude;

	--cmd;

	if (!world[ch->in_room].dir_option[cmd]) {
	        ansi(CLR_ERROR, ch);
		send_to_char("Alas, you cannot go that way...\n\r", ch);
		ansi(END, ch);
	      } 
	else {          /* Direction is possible */

		if (IS_SET(EXIT(ch, cmd)->exit_info, EX_CLOSED)) {
			if (EXIT(ch, cmd)->keyword) {
				sprintf(tmp, "The %s seems to be closed.\n\r",
					fname(EXIT(ch, cmd)->keyword));
				send_to_char(tmp, ch);
			} else {
				send_to_char("It seems to be closed.\n\r", ch);
			}
		} else if (EXIT(ch, cmd)->to_room == NOWHERE) {
                        ansi(CLR_ERROR, ch);
			send_to_char("Alas, you can't go that way.\n\r", ch);
                        ansi(END, ch);
		      }
		else if (!ch->followers && !ch->master)
			do_simple_move(ch,cmd,FALSE);
		else {

			if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master) && 
			   (ch->in_room == ch->master->in_room)) {
			        ansi(CLR_ERROR, ch);
				send_to_char("The thought of leaving your master makes you weep.\n\r", ch);
				ansi(CLR_ACTION, ch);
				act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
				ansi(END, ch);
			} else {
				was_in = ch->in_room;
				if (do_simple_move(ch, cmd, TRUE) == 1) { /* Move the character */
					if (ch->followers) {							      /* If succes move followers */

						for(k = ch->followers; k; k = next_dude) {
							next_dude = k->next;
							if ((was_in == k->follower->in_room) &&
							    (GET_POS(k->follower) >= POSITION_STANDING)) {
								ansi_act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR,CLR_ACTION);
								cmd++;
								send_to_char("\n\r", k->follower);
								do_move(k->follower, argument, cmd);
								cmd--;
							}
						}
					}
				}
			}
		}
	}
}

int find_door(struct char_data *ch, char *type, char *dir)
{
	char buf[MAX_STRING_LENGTH];
	int door;
	char *dirs[] = 
	{
		"north",
		"east",
		"south",
		"west",
		"up",
		"down",
		"\n"
	};

	if (*dir) /* a direction was specified */
	{
		if ((door = search_block(dir, dirs, FALSE)) == -1) /* Partial Match */
		{
                        ansi(CLR_ERROR, ch);
			send_to_char("That's not a direction.\n\r", ch);
                        ansi(END, ch);
			return(-1);
		}

		if (EXIT(ch, door))
			if (EXIT(ch, door)->keyword)
				if (isname(type, EXIT(ch, door)->keyword))
					return(door);
				else
				{
					sprintf(buf, "I see no %s there.\n\r", type);
					ansi(CLR_ERROR, ch);
					send_to_char(buf, ch);
					ansi(END, ch);
					return(-1);
				}
			else
				return(door);
		else
		{
                        ansi(CLR_ERROR, ch);
			send_to_char(
				"I really don't see how you can close anything there.\n\r", ch);
                        ansi(END, ch);
			return(-1);
		}
	}
	else /* try to locate the keyword */
	{
		for (door = 0; door <= 5; door++)
			if (EXIT(ch, door))
				if (EXIT(ch, door)->keyword)
					if (isname(type, EXIT(ch, door)->keyword))
						return(door);

		sprintf(buf, "I see no %s here.\n\r", type);
		ansi(CLR_ERROR, ch);
		send_to_char(buf, ch);
		ansi(END, ch);
		return(-1);
	}
}

void do_open(struct char_data *ch, char *argument, int cmd)
{
	int door, other_room, bits;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	struct obj_data *obj;
	struct char_data *victim;

	argument_interpreter(argument, type, dir);

	if (!*type) {
	        ansi(CLR_ERROR, ch);
		send_to_char("Open what?\n\r", ch);
		ansi(END, ch);
	} else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
		ch, &victim, &obj))

		/* this is an object */

		if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
                        ansi(CLR_ERROR, ch);
			send_to_char("That's not a container.\n\r", ch);
                        ansi(END, ch);
 		} else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
                        ansi(CLR_ERROR, ch);
			send_to_char("But it's already open!\n\r", ch);
                        ansi(END, ch);
                } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
                        ansi(CLR_ERROR, ch);
			send_to_char("You can't do that.\n\r", ch);
			ansi(END, ch);
		} else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("It seems to be locked.\n\r", ch);
			ansi(END, ch);
		} else {
			REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
			ansi_act("You opens $p.", FALSE, ch, obj, 0, TO_CHAR,CLR_ACTION);
			ansi_act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM,CLR_ACTION);
		}
	else if ((door = find_door(ch, type, dir)) >= 0)

		/* perhaps it is a door */

		if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
                        ansi(CLR_ERROR, ch);
			send_to_char("That's impossible, I'm afraid.\n\r", ch);
			ansi(END, ch);
		      } 
		else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
                        ansi(CLR_ERROR, ch);
			send_to_char("It's already open!\n\r", ch);
                        ansi(END, ch);
		      }
		else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
                        ansi(CLR_ERROR, ch);
			send_to_char("It seems to be locked.\n\r", ch);
                        ansi(END, ch);
		      }
		else
		{
			REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
			if (EXIT(ch, door)->keyword)
			        ansi_act("$n opens the $F.", FALSE, ch, 0, EXIT(ch, door)->keyword,TO_ROOM,CLR_ACTION);
			else
				ansi_act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTIOn);
                        send_to_char("Ok.\n\r", ch); 
			/* now for opening the OTHER side of the door! */
			if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
				if (back = world[other_room].dir_option[rev_dir[door]])	
					if (back->to_room == ch->in_room)
					{
						REMOVE_BIT(back->exit_info, EX_CLOSED);
						if (back->keyword)
						{
							sprintf(buf,
								"The %s is opened from the other side.\n\r",
								fname(back->keyword));
							send_to_room(buf, EXIT(ch, door)->to_room);
						}
						else
							send_to_room(
							"The door is opened from the other side.\n\r",
							EXIT(ch, door)->to_room);
					}						 
		}
}


void do_close(struct char_data *ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	struct obj_data *obj;
	struct char_data *victim;


	argument_interpreter(argument, type, dir);

	if (!*type) {
	        ansi(CLR_ERROR, ch);
		send_to_char("Close what?\n\r", ch);
                ansi(END, ch);
	      }
	else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
		ch, &victim, &obj))

		/* this is an object */

		if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
		        ansi(CLR_ERROR, ch);
			send_to_char("That's not a container.\n\r", ch);
			ansi(END, ch);
		      }
 		else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("But it's already closed!\n\r", ch);
			ansi(END, ch);
		      }
		else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE)) {
                        ansi(CLR_ERROR, ch);
			send_to_char("That's impossible.\n\r", ch);
			ansi(END, ch);
		} 
		else
		{
			SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
			send_to_char("Ok.\n\r", ch);
			ansi_act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM,CLR_ACTION);
		}
	else if ((door = find_door(ch, type, dir)) >= 0)

		/* Or a door */

		if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("That's absurd.\n\r", ch);
			ansi(END, ch);
		      }
		else if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("It's already closed!\n\r", ch);
			ansi(END, ch);
		      }
		else
		{
			SET_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
			if (EXIT(ch, door)->keyword)
				ansi_act("$n closes the $F.", 0, ch, 0, EXIT(ch, door)->keyword,TO_ROOM,CLR_ACTION);
			else
			        ansi_act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
			send_to_char("Ok.\n\r", ch);
			/* now for closing the other side, too */
			if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
				if (back = world[other_room].dir_option[rev_dir[door]])
					if (back->to_room == ch->in_room)
					{
						SET_BIT(back->exit_info, EX_CLOSED);
						if (back->keyword)
						{
							sprintf(buf,
								"The %s closes quietly.\n\r", back->keyword);
							send_to_room(buf, EXIT(ch, door)->to_room);
						}
						else
							send_to_room(
								"The door closes quietly.\n\r",
								EXIT(ch, door)->to_room);
					}						 
		}
}


int has_key(struct char_data *ch, int key)
{
	struct obj_data *o;

	for (o = ch->carrying; o; o = o->next_content)
		if (obj_index[o->item_number].virtual == key)
			return(1);

	if (ch->equipment[HOLD])
		if (obj_index[ch->equipment[HOLD]->item_number].virtual == key)
			return(1);

	return(0);
}


void do_lock(struct char_data *ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	struct obj_data *obj;
	struct char_data *victim;

	argument_interpreter(argument, type, dir);

	if (!*type) {
	        ansi(CLR_ERROR, ch);
		send_to_char("Lock what?\n\r", ch);
	        ansi(END, ch);
	      }
	else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
		ch, &victim, &obj))

		/* this is an object */

		if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
		        ansi(CLR_ERROR, ch);
			send_to_char("That's not a container.\n\r", ch);
             	        ansi(END, ch);
		      }
 		else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("Maybe you should close it first...\n\r", ch);
			ansi(END, ch);
		      }
		else if (obj->obj_flags.value[2] < 0) {
		        ansi(CLR_ERROR, ch);
			send_to_char("That thing can't be locked.\n\r", ch);
			ansi(END, ch);
		      }
		else if (!has_key(ch, obj->obj_flags.value[2])) {
		        ansi(CLR_ERROR, ch);
			send_to_char("You don't seem to have the proper key.\n\r", ch);	
          	        ansi(END, ch);
		      }
		else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("It is locked already.\n\r", ch);
			ansi(END, ch);
		      }
		else
		{
			SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
			ansi(CLR_ACTION, ch);
			send_to_char("*Cluck*\n\r", ch);
			ansi(END, ch);
			ansi_act("$n locks $p - 'cluck', it says.", FALSE, ch, obj, 0, TO_ROOM,CLR_ACTION);
		}
	else if ((door = find_door(ch, type, dir)) >= 0)

		/* a door, perhaps */

		if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("That's absurd.\n\r", ch);
			ansi(END, ch);
		      }
		else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("You have to close it first, I'm afraid.\n\r", ch);
			ansi(END, ch);
		      }
		else if (EXIT(ch, door)->key < 0) {
		        ansi(CLR_ERROR, ch);
			send_to_char("There does not seem to be any keyholes.\n\r", ch);
			ansi(END, ch);
		      }
		else if (!has_key(ch, EXIT(ch, door)->key)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("You don't have the proper key.\n\r", ch);
			ansi(END, ch);
		      }
		else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("It's already locked!\n\r", ch);
			ansi(END, ch);
	      }
		else
		{
			SET_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
			if (EXIT(ch, door)->keyword)
			        ansi_act("$n locks the $F.", 0, ch, 0,  EXIT(ch, door)->keyword,TO_ROOM,CLR_ACTION);
			else
			        ansi_act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
			ansi(CLR_ACTION, ch);
			send_to_char("*Click*\n\r", ch);
			ansi(END, ch);
			/* now for locking the other side, too */
			if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
				if (back = world[other_room].dir_option[rev_dir[door]])
					if (back->to_room == ch->in_room)
						SET_BIT(back->exit_info, EX_LOCKED);
		}
}


void do_unlock(struct char_data *ch, char *argument, int cmd)
{
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	struct obj_data *obj;
	struct char_data *victim;


	argument_interpreter(argument, type, dir);

	if (!*type) {
	        ansi(CLR_ERROR, ch);
		send_to_char("Unlock what?\n\r", ch);
	        ansi(END, ch);
	      } 
	else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
		ch, &victim, &obj))

		/* this is an object */

		if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
		        ansi(CLR_ERROR, ch);
			send_to_char("That's not a container.\n\r", ch);
			ansi(END, ch);
		      }
 		else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("Silly - it ain't even closed!\n\r", ch);
			ansi(END, ch);
		      }
		else if (obj->obj_flags.value[2] < 0) {
		        ansi(CLR_ERROR, ch);
			send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
			ansi(END, ch);
		      }
		else if (!has_key(ch, obj->obj_flags.value[2])) {
		        ansi(CLR_ERROR, ch);
			send_to_char("You don't seem to have the proper key.\n\r", ch);	
			ansi(END, ch);
		      }
		else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("Oh.. it wasn't locked, after all.\n\r", ch);
			ansi(END, ch);
		      }
		else
		{
			REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
			ansi(CLR_ACTION, ch);
			send_to_char("*Click*\n\r", ch);
			ansi(END, ch);
			ansi_act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM,CLR_ACTIOn);
		}
	else if ((door = find_door(ch, type, dir)) >= 0)

		/* it is a door */

		if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
                        ansi(CLR_ERROR, ch);
			send_to_char("That's absurd.\n\r", ch);
                        ansi(END, ch);
		      } 
		else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
                        ansi(CLR_ERROR, ch);
			send_to_char("Heck.. it ain't even closed!\n\r", ch);
			ansi(END, ch);
		      }
		else if (EXIT(ch, door)->key < 0) {
		        ansi(CLR_ERROR, ch);
			send_to_char("You can't seem to spot any keyholes.\n\r", ch);
			ansi(END, ch);
		      }
		else if (!has_key(ch, EXIT(ch, door)->key)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("You do not have the proper key for that.\n\r", ch);
			ansi(END, ch);
		      }
		else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("It's already unlocked, it seems.\n\r", ch);
			ansi(END, ch);
		      }
		else
		{
			REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
			if (EXIT(ch, door)->keyword)
				ansi_act("$n unlocks the $F.", 0, ch, 0, EXIT(ch, door)->keyword,TO_ROOM,CLR_ACTION);
			else
				ansi_act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
                        ansi(CLR_ACTION, ch);
			send_to_char("*click*\n\r", ch);
                        ansi(END, ch);
			/* now for unlocking the other side, too */
			if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
				if (back = world[other_room].dir_option[rev_dir[door]])
					if (back->to_room == ch->in_room)
						REMOVE_BIT(back->exit_info, EX_LOCKED);
		}
}


void do_pick(struct char_data *ch, char *argument, int cmd)
{
   signed char percent;
	int door, other_room;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	struct room_direction_data *back;
	struct obj_data *obj;
	struct char_data *victim;

	argument_interpreter(argument, type, dir);

   percent=number(1,101); /* 101% is a complete failure */

   if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) {
      ansi(CLR_ERROR, ch);
      send_to_char("You failed to pick the lock.\n\r", ch);
      ansi(END, ch);
      return;
	}

	if (!*type) {
	        ansi(CLR_ERROR, ch);
		send_to_char("Pick what?\n\r", ch);
		ansi(END, ch);
	      }
	else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
		ch, &victim, &obj))

		/* this is an object */

		if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
		        ansi(CLR_ERROR, ch);
			send_to_char("That's not a container.\n\r", ch);
			ansi(END, ch);
		      }
 		else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("Silly - it ain't even closed!\n\r", ch);
			ansi(END, ch);
		      }
		else if (obj->obj_flags.value[2] < 0) {
		        ansi(CLR_ERROR, ch);
			send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
			ansi(END, ch);
		      }
		else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
			ansi(CLR_ERROR, ch);
			send_to_char("Oho! This thing is NOT locked!\n\r", ch);
			ansi(END, ch);
		      }
		else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) {
			ansi(CLR_ERROR, ch);
			send_to_char("It resists your attempts at picking it.\n\r", ch);
			ansi(END, ch);
		      }
		else
		{
			REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
			ansi(CLR_ACTION, ch);
			send_to_char("*Click*\n\r", ch);
			ansi(END, ch);
			ansi_act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM,CLR_ACTION);
		}
	else if ((door = find_door(ch, type, dir)) >= 0)
		if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
			ansi(CLR_ERROR, ch);
			send_to_char("That's absurd.\n\r", ch);
			ansi(END, ch);
		      }
		else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
                        ansi(CLR_ERROR, ch);
			send_to_char("You realize that the door is already open.\n\r", ch);
                        ansi(END, ch);
		      }
		else if (EXIT(ch, door)->key < 0) {
			ansi(CLR_ERROR, ch);
			send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
			ansi(END, ch);
		      }
		else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
			ansi(CLR_ERROR, ch);
			send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
			ansi(END, ch);
		      }
		else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)) {
			ansi(CLR_ERROR, ch);
			send_to_char("You seem to be unable to pick this lock.\n\r", ch);
			ansi(END, ch);
		      }
		else
		{
			REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
			if (EXIT(ch, door)->keyword)
				ansi_act("$n skillfully picks the lock of the $F.", 0, ch, 0,EXIT(ch, door)->keyword, TO_ROOM,CLR_ACTION);
			else
				ansi_act("$n picks the lock of the.", TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
			ansi(CLR_ACTION, ch);
			send_to_char("The lock quickly yields to your skills.\n\r", ch);
			ansi(END, ch);
			/* now for unlocking the other side, too */
			if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
				if (back = world[other_room].dir_option[rev_dir[door]])
					if (back->to_room == ch->in_room)
						REMOVE_BIT(back->exit_info, EX_LOCKED);
		}
}


void do_enter(struct char_data *ch, char *argument, int cmd)
{
	int door;
	char buf[MAX_INPUT_LENGTH], tmp[MAX_STRING_LENGTH];

	void do_move(struct char_data *ch, char *argument, int cmd);

	one_argument(argument, buf);

	if (*buf)  /* an argument was supplied, search for door keyword */
	{
		for (door = 0; door <= 5; door++)
			if (EXIT(ch, door))
				if (EXIT(ch, door)->keyword)
					if (!str_cmp(EXIT(ch, door)->keyword, buf))
					{
						do_move(ch, "", ++door);
						return;
					}
		sprintf(tmp, "There is no %s here.\n\r", buf);
		ansi(CLR_ERROR, ch);
		send_to_char(tmp, ch);
		ansi(END, ch);
 	}
	else
		if (IS_SET(world[ch->in_room].room_flags, INDOORS)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("You are already indoors.\n\r", ch);
			ansi(END, ch);
		      }
		else
		{
			/* try to locate an entrance */
			for (door = 0; door <= 5; door++)
				if (EXIT(ch, door))
					if (EXIT(ch, door)->to_room != NOWHERE)
						if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
							IS_SET(world[EXIT(ch, door)->to_room].room_flags,
							INDOORS))
						{
							do_move(ch, "", ++door);
							return;
						}
			ansi(CLR_ERROR, ch);
			send_to_char("You can't seem to find anything to enter.\n\r", ch);
			ansi(END, ch);
		}
}


void do_leave(struct char_data *ch, char *argument, int cmd)
{
	int door;

	void do_move(struct char_data *ch, char *argument, int cmd);

	if (!IS_SET(world[ch->in_room].room_flags, INDOORS)) {
	        ansi(CLR_ERROR, ch);
		send_to_char("You are outside.. where do you want to go?\n\r", ch);
		ansi(END, ch);
	      }
	else
	{
		for (door = 0; door <= 5; door++)
			if (EXIT(ch, door))
				if (EXIT(ch, door)->to_room != NOWHERE)
					if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
						!IS_SET(world[EXIT(ch, door)->to_room].room_flags, INDOORS))
					{
						do_move(ch, "", ++door);
						return;
					}
		ansi(CLR_ERROR, ch);
		send_to_char("I see no obvious exits to the outside.\n\r", ch);
		ansi(END, ch);
	}
}


void do_stand(struct char_data *ch, char *argument, int cmd)
{
	char buffer[MAX_STRING_LENGTH];

	switch(GET_POS(ch)) {
		case POSITION_STANDING : { 
			ansi_act("You are already standing.",FALSE, ch,0,0,TO_CHAR,CLR_ERROR);
		} break;
		case POSITION_SITTING	: { 
			ansi_act("You stand up.", FALSE, ch,0,0,TO_CHAR,CLR_ACTION);
			ansi_act("$n clambers on $s feet.",TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
			GET_POS(ch) = POSITION_STANDING;
		} break;
		case POSITION_RESTING	: { 
			ansi_act("You stop resting, and stand up.", FALSE, ch,0,0,TO_CHAR,CLR_ACTION);
			ansi_act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
			GET_POS(ch) = POSITION_STANDING;
		} break;
		case POSITION_SLEEPING : { 
			ansi_act("You have to wake up first!", FALSE, ch, 0,0,TO_CHAR,CLR_ERROR);
		} break;
		case POSITION_FIGHTING : { 
			ansi_act("Do you not consider fighting as standing?",FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
		} break;
		default : { 
			ansi_act("You stop floating around, and put your feet on the ground.", FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
			ansi_act("$n stops floating around, and puts $s feet on the ground.",  TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
		} break;
	}
}


void do_sit(struct char_data *ch, char *argument, int cmd)
{
	char buffer[MAX_STRING_LENGTH];

	switch(GET_POS(ch)) {
		case POSITION_STANDING : {
			ansi_act("You sit down.", FALSE, ch, 0,0, TO_CHAR,CLR_ACTION);
			ansi_act("$n sits down.", FALSE, ch, 0,0, TO_ROOM,CLR_ACTION);
			GET_POS(ch) = POSITION_SITTING;
		} break;
		case POSITION_SITTING	: {
			ansi(CLR_ERROR, ch);
			send_to_char("You'r sitting already.\n\r", ch);
			ansi(END, ch);
		} break;
		case POSITION_RESTING	: {
			ansi_act("You stop resting, and sit up.", FALSE, ch,0,0,TO_CHAR,CLR_ACTION);
			ansi_act("$n stops resting.", TRUE, ch, 0,0,TO_ROOM,CLR_ACTION);
			GET_POS(ch) = POSITION_SITTING;
		} break;
		case POSITION_SLEEPING : {
		        ansi_act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
		} break;
		case POSITION_FIGHTING : {
			ansi_act("Sit down while fighting? are you MAD?", FALSE, ch,0,0,TO_CHAR,CLR_ERROR);
		} break;
		default : {
		        ansi_act("You stop floating around, and sit down.", FALSE, ch,0,0,TO_CHAR,CLR_ACTION);
			ansi_act("$n stops floating around, and sits down.", TRUE, ch,0,0,TO_ROOM,CLR_ACTION);
			GET_POS(ch) = POSITION_SITTING;
		} break;
	}
}
			

void do_rest(struct char_data *ch, char *argument, int cmd) {
char buffer[MAX_STRING_LENGTH];

	switch(GET_POS(ch)) {
		case POSITION_STANDING : {
			ansi_act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
			ansi_act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
			GET_POS(ch) = POSITION_RESTING;
		} break;
		case POSITION_SITTING : {
			ansi_act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
			ansi_act("$n rests.", TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
			GET_POS(ch) = POSITION_RESTING;
		} break;
		case POSITION_RESTING : {
			ansi_act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
		} break;
		case POSITION_SLEEPING : {
			ansi_act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
			} break;
		case POSITION_FIGHTING : {
			ansi_act("Rest while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
		} break;
		default : {
			ansi_act("You stop floating around, and stop to rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
			ansi_act("$n stops floating around, and rests.", FALSE, ch, 0,0, TO_ROOM,CLR_ACTION);
			GET_POS(ch) = POSITION_SITTING;
		} break;
	}
}


void do_sleep(struct char_data *ch, char *argument, int cmd) {
char buffer[MAX_STRING_LENGTH];

	switch(GET_POS(ch)) {
		case POSITION_STANDING : 
		case POSITION_SITTING  :
		case POSITION_RESTING  : {
			ansi(CLR_ACTION, ch);
			send_to_char("You go to sleep.\n\r", ch);
			ansi(END, ch);
			ansi_act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
			GET_POS(ch) = POSITION_SLEEPING;
		} break;
		case POSITION_SLEEPING : {
			ansi(CLR_ERROR, ch);
			send_to_char("You are already sound asleep.\n\r", ch);
			ansi(END, ch);
		} break;
		case POSITION_FIGHTING : {
			ansi(CLR_ERROR, ch);
			send_to_char("Sleep while fighting? are you MAD?\n\r", ch);
			ansi(END, ch);
		} break;
		default : {
			ansi_act("You stop floating around, and lie down to sleep.", FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
			ansi_act("$n stops floating around, and lie down to sleep.", TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
			GET_POS(ch) = POSITION_SLEEPING;
		} break;
	}
}


void do_wake(struct char_data *ch, char *argument, int cmd)
{
	char buffer[MAX_STRING_LENGTH];
	struct char_data *tmp_char;
	char arg[MAX_STRING_LENGTH];


	one_argument(argument,arg);
	if (*arg) {
		if (GET_POS(ch) == POSITION_SLEEPING) {
			ansi_act("You can't wake people up if you are asleep yourself!", FALSE, ch,0,0,TO_CHAR,CLR_ERROR);
		} else {
			tmp_char = get_char_room_vis(ch, arg);
			if (tmp_char) {
				if (tmp_char == ch) {
					ansi_act("If you want to wake yourself up, just type 'wake'", FALSE, ch,0,0,TO_CHAR,CLR_ERROR);
				} else {
					if (GET_POS(tmp_char) == POSITION_SLEEPING) {
						if (IS_AFFECTED(tmp_char, AFF_SLEEP)) {
							ansi_act("You can not wake $M up!", FALSE, ch, 0, tmp_char, TO_CHAR,CLR_ERROR);
						} else {
							ansi_act("You wake $M up.", FALSE, ch, 0, tmp_char, TO_CHAR,CLR_ACTION);
							GET_POS(tmp_char) = POSITION_SITTING;
							ansi_act("You are awakened by $n.", FALSE, ch, 0, tmp_char, TO_VICT,CLR_ACTION);
						}
					} else {
						ansi_act("$N is already awake.",FALSE,ch,0,tmp_char, TO_CHAR,CLR_ERROR);
					}
				}
			} else {
			        ansi(CLR_ERROR, ch);
				send_to_char("You do not see that person here.\n\r", ch);
				ansi(END, ch);
			}
		}
	} else {
		if (IS_AFFECTED(ch,AFF_SLEEP)) {
			ansi(CLR_ERROR, ch);
			send_to_char("You can't wake up!\n\r", ch);
			ansi(END, ch);
		} else {
			if (GET_POS(ch) > POSITION_SLEEPING) {
			        ansi(CLR_ERROR, ch);
				send_to_char("You are already awake...\n\r", ch);
				ansi(CLR_ERROR, ch);
			      }
			else {
			        ansi(CLR_ACTION, ch);
				send_to_char("You wake, and sit up.\n\r", ch);
				ansi(END ch);
				ansi_act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
				GET_POS(ch) = POSITION_SITTING;
			}
		}
	}
}


void do_follow(struct char_data *ch, char *argument, int cmd)
{
	char name[160], buf[160];
	struct char_data *leader, *k;

	void stop_follower(struct char_data *ch);
	void add_follower(struct char_data *ch, struct char_data *leader);

	one_argument(argument, name);

	if (*name) {
		if (!(leader = get_char_room_vis(ch, name))) {
			ansi(CLR_ERROR, ch);
			send_to_char("I see no person by that name here!\n\r", ch);
			ansi(END, ch);
			return;
		}
	} else {
	        ansi(CLR_ERROR, ch);
		send_to_char("Who do you wish to follow?\n\r", ch);
		ansi(END, ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {
	        ansi_act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR,CLR_ERROR);
	      } else { /* Not Charmed follow person */
		if (leader == ch) {
			if (!ch->master) {
			        ansi(CLR_ERROR, ch);
				send_to_char("You are already following yourself.\n\r", ch);
				ansi(END, ch);
				return;
			}
			stop_follower(ch);
		} else {
			if (circle_follow(ch, leader)) {
				ansi_act("Sorry, but following in 'loops' is not allowed", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
				return;
			}
			if (ch->master)
				stop_follower(ch);

			add_follower(ch, leader);
		}
	}
}



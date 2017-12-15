/* ************************************************************************
*  file: mobact.c , Mobile action module.                 Part of DIKUMUD *
*  Usage: Procedures generating 'intelligent' behavior in the mobiles.    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "constants.h"
#include "fight.h"
#include "act.h"

void mobile_activity(void)
{
	int magic_user(struct char_data *ch, int cmd, char *arg);
	int cleric(struct char_data *ch, int cmd, char *arg);
	int thief(struct char_data *ch, int cmd, char *arg);

	struct char_data *ch;
	struct char_data *tmp_ch;
	struct obj_data *obj, *best_obj;
	int door, found, max;

	extern int no_specials;

	for (ch = character_list; ch; ch = ch->next)
		if (IS_MOB(ch))
		{
			/* Examine call for special procedure */
			if (IS_SET(ch->specials.act, ACT_SPEC) && !no_specials) {
				if (!mob_index[ch->nr].func) {
					fprintf(stderr, "Attempting to call a non-existant MOB funct. (mob %d)\n\r", mob_index[ch->nr].virtual);
					REMOVE_BIT(ch->specials.act, ACT_SPEC);
				} else {
			   	if ((*mob_index[ch->nr].func)	(ch, 0, ""))
		      		continue;
				}
			}
			if (AWAKE(ch) && (ch->specials.fighting)) {
				if (IS_SET(ch->specials.act,ACT_MAGE))
					if (magic_user(ch,0,"")) continue;
			}
			if (AWAKE(ch)) {
				if (IS_SET(ch->specials.act,ACT_CLERIC))
					if (cleric(ch,0,"")) continue;
			}
			if (AWAKE(ch) && !(ch->specials.fighting)) {
				if (IS_SET(ch->specials.act,ACT_THIEF))
					if (thief(ch,0,"")) continue;

				if (IS_SET(ch->specials.act, ACT_SCAVENGER)) {
					if (world[ch->in_room].contents && !number(0,10)) {
						for (max = 1, best_obj = 0, obj = world[ch->in_room].contents;
					       obj; obj = obj->next_content) {
							if (CAN_GET_OBJ(ch, obj)) {
								if (obj->obj_flags.cost > max) {
									best_obj = obj;
									max = obj->obj_flags.cost;
								}
							}
						} /* for */

						if ((best_obj) && (max>0)) { /* ZCMDS worth -1, DONT pickup */
							obj_from_room(best_obj);
							obj_to_char(best_obj, ch);
							act("$n gets $p.",FALSE,ch,best_obj,0,TO_ROOM);
						}
					}
				} /* Scavenger */

				if (!IS_SET(ch->specials.act, ACT_SENTINEL) && 
					(GET_POS(ch) == POSITION_STANDING) &&
					((door = number(0, 45)) <= 5) && CAN_GO(ch,door) &&
					!IS_SET(world[EXIT(ch, door)->to_room].room_flags, NO_MOB) &&
					 !IS_SET(world[EXIT(ch, door)->to_room].room_flags, DEATH)) {
					if (ch->specials.last_direction == door) {
						ch->specials.last_direction = -1;
					} else {
						if (!IS_SET(ch->specials.act, ACT_STAY_ZONE)) {
							ch->specials.last_direction = door;
							do_move(ch, "", ++door);
						} else {
							if (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone) {
								ch->specials.last_direction = door;
								do_move(ch, "", ++door);
							}
						}
					}
				} /* if can go */


				if (IS_SET(ch->specials.act,ACT_AGGRESSIVE)) {
					found = FALSE;
					for (tmp_ch = world[ch->in_room].people; tmp_ch && !found;
					     tmp_ch = tmp_ch->next_in_room) {
						if (!number(0,2) && !IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch) &&!IS_SET(tmp_ch->specials.act, PLR_NOHASSLE)) {
							if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
								hit(ch, tmp_ch, 0);
								found = TRUE;
							}
						}
					}
				}
			} /* If AWAKE(ch)   */
		}   /* If IS_MOB(ch)  */
}








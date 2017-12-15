/* ************************************************************************
*  file: act.other.c , Implementation of commands.        Part of DIKUMUD *
*  Usage : Other commands.                                                *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "limits.h"
#include "interpreter.h"
#include "fight.h"
#include "spells.h"
#include "constants.h"
#include "ansi.h"
#include "act.h"


void do_visible(struct char_data *ch, char *argument, int cmd)
{
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    	affect_from_char(ch, SPELL_INVISIBLE);
  if (affected_by_spell(ch, SPELL_IMPROVED_INVIS))
    	affect_from_char(ch, SPELL_IMPROVED_INVIS);

  REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
  ansi_act("$n slowly fade into existence.", FALSE, ch,0,0,TO_ROOM,CLR_ACTION);
}

/* should guard against cursed items */
void do_junk(struct char_data *ch, char *argument, int cmd)
{
	struct obj_data *tmp_obj;
	char arg[MAX_INPUT_LENGTH];
 
	argument=one_argument(argument, arg);
	if (*arg) 
	{	tmp_obj = get_obj_in_list_vis(ch,arg, ch->carrying);
		if (!tmp_obj) 
		{	ansi_act("You don't have anything like that.",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
			return;
		}

		if (IS_SET(tmp_obj->obj_flags.extra_flags, ITEM_NODROP))
		  {
		    ansi(CLR_ERROR, ch);
		    send_to_char("You can't let go of it! Yeech!!\n\r", ch);
		    ansi(END, ch);
		    return;
		  }

		sprintf(arg, "The Gods award you %d coins!.\n\r", tmp_obj->obj_flags.cost/100);
		ansi(CLR_ACTION, ch);
		send_to_char(arg, ch);
		ansi(END, ch);  
		GET_GOLD(ch) += tmp_obj->obj_flags.cost/100;
  		extract_obj(tmp_obj);
	} else  {
		ansi(CLR_ERROR, ch);
		send_to_char("Junk! Fine! But junk what?\n\r", ch);
		ansi(END, ch);
		}
}

void do_qui(struct char_data *ch, char *argument, int cmd)
{
	ansi(CLR_ERROR, ch);
	send_to_char("You have to write quit - no less, to quit!\n\r",ch);
	send_to_char("Warning: You will lose all your equipment if you do not rent in the Inn.\n\r",ch);
	ansi(END, ch);
	return;
}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
	void do_save(struct char_data *ch, char *argument, int cmd);
	void die(struct char_data *ch);
	void del_char_objs(struct char_data *ch);
	
	char buf[100];

	if (IS_NPC(ch) || !ch->desc)
		return;

	if (GET_POS(ch) == POSITION_FIGHTING) {
	        ansi(CLR_ERROR, ch);
		send_to_char("No way! You are fighting.\n\r", ch);
		ansi(END, ch);
		return;
	}

	if (GET_POS(ch) < POSITION_STUNNED) {
	        ansi(CLR_DAM, ch);
		send_to_char("You die before your time!\n\r", ch);
		ansi(END, ch);
		die(ch);
		return;
	}

	ansi_act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR,WHITE);
	ansi_act("$n has left the game.", TRUE, ch,0,0,TO_ROOM,CLR_ACTION);
	ansi(CLR_ERROR, ch);
	send_to_char("Warning: By quitting the game you forfeit your equipment.\n\r          To keep your equipment, RENT out at the Inn.\n\r", ch);
	send_to_char("         (Its currently in a big pile on the ground where you quit).\n\r", ch);
	sprintf(buf,"SYSTEM: %s left the game via QUIT.", GET_NAME(ch));
	log(buf);
	ansi(END, ch);
	extract_char(ch); /* Char is saved in extract char */
	del_char_objs(ch);
}


void do_save(struct char_data *ch, char *argument, int cmd)
{
	char buf[100];

	if (IS_NPC(ch) || !ch->desc)
		return;

	sprintf(buf, "Saving %s.\n\r", GET_NAME(ch));
	ansi(CLR_ACTION, ch);
	send_to_char(buf, ch);
	ansi(END, ch);
	save_char(ch, NOWHERE);
}


void do_not_here(struct char_data *ch, char *argument, int cmd)
{
	ansi(CLR_ERROR, ch);
	send_to_char("Sorry, but you cannot do that here!\n\r",ch);
	ansi(END, ch);
}


void do_sneak(struct char_data *ch, char *argument, int cmd)
{
	struct affected_type af;
	signed char percent;

	ansi(CLR_ACTION, ch);
	send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
	ansi(END, ch);
	if (IS_AFFECTED(ch, AFF_SNEAK))
		affect_from_char(ch, SKILL_SNEAK);

	percent=number(1,101); /* 101% is a complete failure */

	if (percent > ch->skills[SKILL_SNEAK].learned +
	    dex_app_skill[GET_DEX(ch)].sneak)
		return;

	af.type = SKILL_SNEAK;
	af.duration = GET_LEVEL(ch);
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_SNEAK;
	affect_to_char(ch, &af);
}
 
void do_title(struct char_data *ch, char *arguement, int cmd)
{
  	char buf[MAX_INPUT_LENGTH];

	for (; *arguement  == ' '; arguement++);
	if (!*arguement) {
	        ansi(CLR_ERROR, ch);
		send_to_char("Okay now what was that you wanted for a title?\n\r",ch);
	        ansi(END, ch);
		return;
 	} else if (strlen(arguement) + strlen(GET_NAME(ch))>80) {
	        ansi(CLR_ERROR, ch);
		send_to_char("I'm afraid thats a bit lengthy.\n\r",ch);
	        ansi(END, ch);
		return;
	} else {
		sprintf(buf,"Ok, you are now:  %s %s",GET_NAME(ch),arguement);
	        ansi(CLR_ACTION, ch);
		send_to_char(buf,ch);
	        ansi(END, ch);
		sprintf(buf,"%s",arguement);
		
		if (GET_TITLE(ch))
			RECREATE(GET_TITLE(ch),char,strlen(buf)+1);
		else
			CREATE(GET_TITLE(ch),char,strlen(buf)+1);

		strcpy(GET_TITLE(ch), buf);
 	}
 }

void do_hide(struct char_data *ch, char *argument, int cmd)
{
	signed char percent;

	ansi(CLR_ACTION, ch);
	send_to_char("You attempt to hide yourself.\n\r", ch);
        ansi(END, ch);

	if (IS_AFFECTED(ch, AFF_HIDE))
		REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

	percent=number(1,101); /* 101% is a complete failure */

	if (percent > ch->skills[SKILL_HIDE].learned +
	    dex_app_skill[GET_DEX(ch)].hide)
		return;

	SET_BIT(ch->specials.affected_by, AFF_HIDE);
}


void do_steal(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	struct obj_data *obj;
	char victim_name[240];
	char obj_name[240];
	char buf[240];
	int percent;
	int gold, eq_pos;
	bool ohoh = FALSE;

	argument = one_argument(argument, obj_name);
	one_argument(argument, victim_name);

	if (!(victim = get_char_room_vis(ch, victim_name))) {
                ansi(CLR_ERROR, ch);
		send_to_char("Steal what from who?\n\r", ch);
                ansi(END, ch);
		return;
	} else if (victim == ch) {
                ansi(CLR_ACTION, ch);
		send_to_char("You stealthily slip a coin from your pocket.\n\r", ch);
		ansi(END, ch);
		return;
	}

	/* 101% is a complete failure */
	percent=number(1,101) - dex_app_skill[GET_DEX(ch)].p_pocket + GET_LEVEL(victim);

	if (GET_POS(victim) < POSITION_SLEEPING)
		percent = -1; /* ALWAYS SUCCESS */

	if (GET_LEVEL(victim)>IMO_LEV) /* NO NO With God's and Shopkeepers! */
		percent += (2*GET_LEVEL(victim)); /* Failure */

	if (str_cmp(obj_name, "coins") && str_cmp(obj_name,"gold")) {

		if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {

			for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
				if (victim->equipment[eq_pos] &&
				   (isname(obj_name, victim->equipment[eq_pos]->name)) &&
						CAN_SEE_OBJ(ch,victim->equipment[eq_pos])) {
					obj = victim->equipment[eq_pos];
					break;
				}

			if (!obj) {
				ansi_act("$E has not got that item.",FALSE,ch,0,victim,TO_CHAR,CLR_ERROR);
				return;
			} else { /* It is equipment */
				if ((GET_POS(victim) > POSITION_STUNNED)) {
				        ansi(CLR_ERROR, ch);
					send_to_char("Steal the equipment now? Impossible!\n\r", ch);
					ansi(END, ch);
					return;
				} else {
					ansi_act("You unequip $p and steal it.",FALSE, ch, obj ,0, TO_CHAR,CLR_ACTION);
					ansi_act("$n steals $p from $N.",FALSE,ch,obj,victim,TO_NOTVICT,CLR_ACTION);
					obj_to_char(unequip_char(victim, eq_pos), ch);
				}
			}
		} else {  /* obj found in inventory */

			percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */

			if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
				ohoh = TRUE;
				ansi_act("Oops..", FALSE, ch,0,0,TO_CHAR,CLR_ERROR);
				ansi_act("$n tried to steal something from you!",FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
				ansi_act("$n tries to steal something from $N.", TRUE, ch, 0, victim, TO_NOTVICT,CLR_ACTION);
			} else { /* Steal the item */
				if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
					if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
						obj_from_char(obj);
						obj_to_char(obj, ch);
						ansi(CLR_ACTION, ch);
						send_to_char("Got it!\n\r", ch);
						ansi(END, ch);
					}
				} else {
				        ansi(CLR_ERROR, ch);
					send_to_char("You cannot carry that much.\n\r", ch);
					ansi(END, ch);
				      }
			}
		}
	} else { /* Steal some coins */
		if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
			ohoh = TRUE;
			ansi_act("Oops..", FALSE, ch,0,0,TO_CHAR,CLR_ERROR);
			ansi_act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
			ansi_act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT,CLR_ACTION);
		} else {
			/* Steal some gold coins */
			gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
			gold = MINV(2000, gold);
			if (gold > 0) {
				GET_GOLD(ch) += gold;
				GET_GOLD(victim) -= gold;
				sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
                                ansi(CLR_ACTION, ch);
				send_to_char(buf, ch);
				ansi(END, ch);
			} else
			{
                                ansi(CLR_ERROR, ch);
				send_to_char("No luck...\n\r",ch);
				ansi(END, ch);
			}
		}
	}

	if (ohoh && IS_NPC(victim) && AWAKE(victim))
		if (IS_SET(victim->specials.act, ACT_NICE_THIEF)) {
			sprintf(buf, "%s is a bloody thief.", GET_NAME(ch));
			do_shout(victim, buf, 0);
			log(buf);
			ansi(CLR_ERROR, ch);
			send_to_char("Don't you ever do that again!\n\r", ch);
			ansi(END, ch);
		} else {
			hit(victim, ch, TYPE_UNDEFINED);
		}

}

 
void do_practice(struct char_data *ch, char *arg, int cmd) {
	int tmp;
	int guild(struct char_data *ch, int cmd, char *arg);
/*
	send_to_char("You can only practise in a guild.\n\r", ch);
*/	tmp = guild(ch,164,0); 
}


void do_idea(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
	{
                ansi(CLR_ERROR, ch);
		send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
                ansi(END, ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)
	{
                ansi(CLR_ERROR, ch);
		send_to_char("That doesn't sound like a good idea to me.. Sorry.\n\r", ch);
                ansi(END, ch);
		return;
	}

	if (!(fl = fopen(IDEA_FILE, "a")))
	{
		perror ("do_idea");
                ansi(CLR_ERROR, ch);
		send_to_char("Could not open the idea-file.\n\r", ch);
                ansi(END, ch);
		return;
	}

	sprintf(str, "**%s: %s\n", GET_NAME(ch), argument);
	fputs(str, fl);
	fclose(fl);
	ansi(CLR_ACTION, ch);
	send_to_char("Ok. The Gods thank you.\n\r", ch);
	ansi(END, ch);
}


void do_typo(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
	{
                ansi(CLR_ERROR, ch);
		send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
                ansi(END, ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)
	{
                ansi(CLR_ERROR, ch);
		send_to_char("I beg your pardon?\n\r", ch);
                ansi(END, ch);
		return;
	}

	if (!(fl = fopen(TYPO_FILE, "a")))
	{
		perror ("do_typo");
                ansi(CLR_ERROR, ch);
		send_to_char("Could not open the typo-file.\n\r", ch);
                ansi(END, ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
		GET_NAME(ch), world[ch->in_room].number, argument);
	fputs(str, fl);
	fclose(fl);
	ansi(CLR_ACTION, ch);
	send_to_char("Ok. The Gods thank you.\n\r", ch);
	ansi(END, ch);
}


void do_bug(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
	{
                ansi(CLR_ERROR, ch);
		send_to_char("You are a monster! Bug off!\n\r", ch);
                ansi(END, ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)
	{
                ansi(CLR_ERROR, ch);
		send_to_char("Pardon?\n\r", ch);
                ansi(END, ch);
		return;
	}

	if (!(fl = fopen(BUG_FILE, "a")))
	{
		perror ("do_bug");
                ansi(CLR_ERROR, ch);
		send_to_char("Could not open the bug-file.\n\r", ch);
                ansi(END, ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
		GET_NAME(ch), world[ch->in_room].number, argument);
	fputs(str, fl);
	fclose(fl);
	ansi(CLR_ACTION, ch);
	send_to_char("Ok. The Gods thank you.\n\r", ch);
	ansi(END, ch);
}


void do_brief(struct char_data *ch, char *argument, int cmd)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->specials.act, PLR_BRIEF))
	{
                ansi(CLR_ACTION, ch);
		send_to_char("Brief mode off.\n\r", ch);
                ansi(END, ch);
		REMOVE_BIT(ch->specials.act, PLR_BRIEF);
	}
	else
	{
                ansi(CLR_ACTION, ch);
		send_to_char("Brief mode on.\n\r", ch);
                ansi(END, ch);
		SET_BIT(ch->specials.act, PLR_BRIEF);
	}
}


void do_compact(struct char_data *ch, char *argument, int cmd)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->specials.act, PLR_COMPACT))
	{
                ansi(CLR_ACTION, ch);
		send_to_char("You are now in the uncompacted mode.\n\r", ch);
                ansi(END, ch);
		REMOVE_BIT(ch->specials.act, PLR_COMPACT);
	}
	else
	{
                ansi(CLR_ACTION, ch);
		send_to_char("You are now in compact mode.\n\r", ch);
                ansi(END, ch);
		SET_BIT(ch->specials.act, PLR_COMPACT);
	}
}


void do_assist(struct char_data *ch, char *arg, int cmd)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->specials.act, PLR_ASSIST)){
                ansi(CLR_ACTION, ch);
		send_to_char("You will no longer auto-assist group members.\n\r", ch);
                ansi(END, ch);
		REMOVE_BIT(ch->specials.act, PLR_ASSIST);
	}else{
                ansi(CLR_ACTION, ch);
		send_to_char("You will now auto-assist group members that are attacked.\n\r", ch);
                ansi(END, ch);
		SET_BIT(ch->specials.act, PLR_ASSIST);
	}
}


void do_wimpy(struct char_data *ch, char *arg, int cmd)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->specials.act, PLR_WIMPY)){
                ansi(CLR_ACTION, ch);
		send_to_char("You will no longer auto-flee.\n\r", ch);
                ansi(END, ch);
		REMOVE_BIT(ch->specials.act, PLR_WIMPY);
	}else{
                ansi(CLR_ACTION, ch);
		send_to_char("You will now auto-flee when you are badly hurt (WIMP!).\n\r", ch);
                ansi(END, ch);
		SET_BIT(ch->specials.act, PLR_WIMPY);
	}
}

void do_display(struct char_data *ch, char *arg, int cmd)
{
	const char *toggle[] = {
	  "noprompt",
	  "room",
	  "hit",
	  "mana",
	  "move",
	  "exits",
	  "allprompt",
	  "\n"
	};
	char buf[256];
	int i;

	if (IS_NPC(ch)){
	  ansi(CLR_ERROR, ch);
	  send_to_char("It doesn't work for mobs sorry.\n\r", ch);
	  ansi(END, ch);
	  return;
	}
	arg = one_argument(arg, buf);
	if (*buf)
	  i = (old_search_block(buf, 0, strlen(buf), toggle, 0));
	else
	  i = -1;
	
	switch(i) {
	case 1: /*no prompt*/
	  REMOVE_BIT(ch->specials.act, PLR_SHOW_ROOM);
	  REMOVE_BIT(ch->specials.act, PLR_SHOW_HP);
	  REMOVE_BIT(ch->specials.act, PLR_SHOW_MANA);
	  REMOVE_BIT(ch->specials.act, PLR_SHOW_MOVE);
	  REMOVE_BIT(ch->specials.act, PLR_SHOW_EXITS);
	  break;

	case 2: /*room*/
	  if (IS_SET(ch->specials.act, PLR_SHOW_ROOM)){
	    ansi(CLR_ACTION, ch);
	    send_to_char("Display room off.\n\r", ch);
	    ansi(END, ch);
	    REMOVE_BIT(ch->specials.act, PLR_SHOW_ROOM);
	  }else{
	    ansi(CLR_ACTION, ch);
	    send_to_char("Display room on.\n\r", ch);
	    ansi(END, ch);
	    SET_BIT(ch->specials.act, PLR_SHOW_ROOM);
	  }
	  break;
	case 3: /*hit*/
	  if (IS_SET(ch->specials.act, PLR_SHOW_HP)){
	    ansi(CLR_ACTION, ch);
	    send_to_char("Display hit points off.\n\r", ch);
	    ansi(END, ch);
	    REMOVE_BIT(ch->specials.act, PLR_SHOW_HP);
	  }else{
	    ansi(CLR_ACTION, ch);
	    send_to_char("Display hit points on.\n\r", ch);
	    ansi(END, ch);
	    SET_BIT(ch->specials.act, PLR_SHOW_HP);
	  }
	  break;
	case 4: /*mana*/
	  if (IS_SET(ch->specials.act, PLR_SHOW_MANA)){
	    ansi(CLR_ACTION, ch);
	    send_to_char("Display mana off.\n\r", ch);
	    ansi(END, ch);
	    REMOVE_BIT(ch->specials.act, PLR_SHOW_MANA);
	  }else{
	    ansi(CLR_ACTION, ch);
	    send_to_char("Display mana on.\n\r", ch);
	    ansi(END, ch);
	    SET_BIT(ch->specials.act, PLR_SHOW_MANA);
	  }
	  break;
	case 5: /*move*/
	  if (IS_SET(ch->specials.act, PLR_SHOW_MOVE)){
	    ansi(CLR_ACTION, ch);
	    send_to_char("Display move off.\n\r", ch);
	    ansi(END, ch);
	    REMOVE_BIT(ch->specials.act, PLR_SHOW_MOVE);
	  }else{
	    ansi(CLR_ACTION, ch);
	    send_to_char("Display move on.\n\r", ch);
	    ansi(END, ch);
	    SET_BIT(ch->specials.act, PLR_SHOW_MOVE);
	  }
	  break;
	case 6: /*exits*/
	  if (IS_SET(ch->specials.act, PLR_SHOW_EXITS)){
	    ansi(CLR_ACTION, ch);
	    send_to_char("Display exits off.\n\r", ch);
	    ansi(END, ch);
	    REMOVE_BIT(ch->specials.act, PLR_SHOW_EXITS);
	  }else{
	    ansi(CLR_ACTION, ch);
	    send_to_char("Display exits on.\n\r", ch);
	    ansi(END, ch);
	    SET_BIT(ch->specials.act, PLR_SHOW_EXITS);
	  }
	  break;
	case 7: /*all prompt*/
	  SET_BIT(ch->specials.act, PLR_SHOW_HP);
	  SET_BIT(ch->specials.act, PLR_SHOW_MANA);
	  SET_BIT(ch->specials.act, PLR_SHOW_MOVE);
	  SET_BIT(ch->specials.act, PLR_SHOW_EXITS);
	  break;
	default:
	  sprintf(buf, "Format: Display <TOGGLE>\n\rValid toggles are: ");
	  for (i=0; *toggle[i] != '\n'; i++){
	    strcat(buf, toggle[i]);
	    strcat(buf, " ");
	  }
          strcat(buf, "\n\r\n\r");
	  ansi(CLR_ACTION, ch);
	  send_to_char(buf, ch);
          send_to_char("Prompt Displays:        Channels:\n\r",ch);
          sprintf(buf, "Hit Points:     ");
          strcat(buf, IS_SET(ch->specials.act, PLR_SHOW_HP) ? "Yes" : "No ");
	  send_to_char(buf, ch);
          sprintf(buf, "     Gossip:         ");
          strcat(buf, !IS_SET(ch->specials.act, PLR_NOGOSSIP) ? "Yes\n\r" : "No \n\r");
	  send_to_char(buf, ch);

          sprintf(buf, "Mana Points:    ");
          strcat(buf, IS_SET(ch->specials.act, PLR_SHOW_MANA) ? "Yes" : "No ");
	  send_to_char(buf, ch);
          sprintf(buf, "     Auction:        ");
          strcat(buf, !IS_SET(ch->specials.act, PLR_NOAUCTION) ? "Yes\n\r" : "No \n\r");
	  send_to_char(buf, ch);

          sprintf(buf, "Move Points:    ");
          strcat(buf, IS_SET(ch->specials.act, PLR_SHOW_MOVE) ? "Yes" : "No ");
	  send_to_char(buf, ch);
          sprintf(buf, "     Grats:          ");
          strcat(buf, !IS_SET(ch->specials.act, PLR_NOGRATS) ? "Yes\n\r" : "No \n\r");
	  send_to_char(buf, ch);

          sprintf(buf, "Visible Exits:  ");
          strcat(buf, IS_SET(ch->specials.act, PLR_SHOW_EXITS) ? "Yes" : "No ");
	  send_to_char(buf, ch);
          sprintf(buf, "     System:         ");
          strcat(buf, !IS_SET(ch->specials.act, PLR_NOSYS) ? "Yes\n\r" : "No \n\r");
	  send_to_char(buf, ch);

          sprintf(buf, "Room Numbers:   ");
          strcat(buf, IS_SET(ch->specials.act, PLR_SHOW_ROOM) ? "Yes" : "No ");
	  send_to_char(buf, ch);
          sprintf(buf, "     Immtalk:        ");
          strcat(buf, !IS_SET(ch->specials.act, PLR_NOIMM) && (GET_LEVEL(ch) >= IMO_LEV) ? "Yes\n\r\n\r" : "No \n\r\n\r");
	  send_to_char(buf, ch);
          
          send_to_char("Misc.:\n\r",ch);
          sprintf(buf, "Tells:          ");
          strcat(buf, !IS_SET(ch->specials.act, PLR_NOTELL) ? "Yes\n\r" : "No\n\r");
	  send_to_char(buf, ch);

          sprintf(buf, "Ansi:           ");
          strcat(buf, IS_SET(ch->specials.act, PLR_ANSI) ? "Yes\n\r" : "No\n\r");
	  send_to_char(buf, ch);
          
          sprintf(buf, "Compact:        ");
          strcat(buf, IS_SET(ch->specials.act, PLR_COMPACT) ? "Yes\n\r" : "No\n\r");
	  send_to_char(buf, ch);
          
          sprintf(buf, "Brief:          ");
          strcat(buf, IS_SET(ch->specials.act, PLR_BRIEF) ? "Yes\n\r" : "No\n\r");
	  send_to_char(buf, ch);
          
          sprintf(buf, "Nosummon:       ");
          strcat(buf, IS_SET(ch->specials.act, PLR_NOSUMMON) ? "Yes\n\r" : "No\n\r");
	  send_to_char(buf, ch);
          
          sprintf(buf, "Nohassle:       ");
          strcat(buf, IS_SET(ch->specials.act, PLR_NOHASSLE) && (GET_LEVEL(ch) >= IMO_LEV) ? "Yes\n\r" : "No \n\r");
	  send_to_char(buf, ch);
          
          sprintf(buf, "Wizinvis:       ");
          strcat(buf, IS_AFFECTED(ch,AFF_WIZINVIS) && (GET_LEVEL(ch) >= IMO_LEV) ? "Yes\n\r" : "No \n\r");
	  send_to_char(buf, ch);
          ansi(END, ch);
          
	  break;
	}
}



void do_auto(struct char_data *ch, char *arg, int cmd)
{
	const char *toggle[] = {
	  "none",
	  "loot",
	  "split",
	  "aggressive",
	  "flee",
	  "assist",
	  "all",
	  "\n"
	};
	char buf[256];
	int i;

	if (IS_NPC(ch)){
          ansi(CLR_ERROR, ch);
	  send_to_char("It doesn't work for mobs sorry\n\r", ch);
          ansi(END, ch);
	  return;
	}
	arg = one_argument(arg, buf);
	if (*buf)
	  i = (old_search_block(buf, 0, strlen(buf), toggle, 0));
	else
	  i = -1;
	
	switch(i) {
	case 1: /*none*/
	  REMOVE_BIT(ch->specials.act, PLR_AUTOLOOT);
	  REMOVE_BIT(ch->specials.act, PLR_AUTOSPLIT);
	  REMOVE_BIT(ch->specials.act, PLR_AUTOAGGR);
	  REMOVE_BIT(ch->specials.act, PLR_AUTOFLEE);
	  REMOVE_BIT(ch->specials.act, PLR_AUTOASSIST);
	  break;

	case 2: /*loot*/
	  if (IS_SET(ch->specials.act, PLR_AUTOLOOT)){
            ansi(CLR_ACTION, ch);
	    send_to_char("You will no longer auto-loot.\n\r", ch);
            ansi(END, ch);
	    REMOVE_BIT(ch->specials.act, PLR_AUTOLOOT);
	  }else{
            ansi(CLR_ACTION, ch);
	    send_to_char("You will now auto-loot your kills.\n\r", ch);
	    SET_BIT(ch->specials.act, PLR_AUTOLOOT);
	    ansi(END, ch);
	  }
	  break;
	case 3: /*split*/
	  if (IS_SET(ch->specials.act, PLR_AUTOSPLIT)){
            ansi(CLR_ACTION, ch);
	    send_to_char("You will no longer auto-split gold.\n\r", ch);
	    ansi(END, ch);
	    REMOVE_BIT(ch->specials.act, PLR_AUTOSPLIT);
	  }else{
            ansi(CLR_ACTION, ch);
	    send_to_char("You will now auto-split gold with your group.\n\r", ch);
	    ansi(END, ch);
	    SET_BIT(ch->specials.act, PLR_AUTOSPLIT);
	  }
	  break;
	case 4: /*aggr*/
	  if (IS_SET(ch->specials.act, PLR_AUTOAGGR)){
            ansi(CLR_ACTION, ch);
	    send_to_char("You will no longer auto-attack aggressive mobs.\n\r", ch);
	    ansi(END, ch);
	    REMOVE_BIT(ch->specials.act, PLR_AUTOAGGR);
	  }else{
            ansi(CLR_ACTION, ch);
	    send_to_char("You will now auto-attack aggressive mobs.\n\r", ch);
	    ansi(END, ch);
	    SET_BIT(ch->specials.act, PLR_AUTOAGGR);
	  }
	  break;
	case 5: /*flee*/
	  if (IS_SET(ch->specials.act, PLR_AUTOFLEE)){
            ansi(CLR_ACTION, ch);
	    send_to_char("You will no longer auto-flee.\n\r", ch);
	    ansi(END, ch);
	    REMOVE_BIT(ch->specials.act, PLR_AUTOFLEE);
	  }else{
            ansi(CLR_ACTION, ch);
	    send_to_char("You will now auto-flee when badly hurt (WIMP!).\n\r", ch);
	    ansi(END, ch);
	    SET_BIT(ch->specials.act, PLR_AUTOFLEE);
	  }
	  break;
	case 6: /*assist*/
	  if (IS_SET(ch->specials.act, PLR_AUTOASSIST)){
            ansi(CLR_ACTION, ch);
	    send_to_char("You will no longer auto-assist.\n\r", ch);
	    ansi(END, ch);
	    REMOVE_BIT(ch->specials.act, PLR_AUTOASSIST);
	  }else{
            ansi(CLR_ACTION, ch);
	    send_to_char("You will now auto-assist group members that start a fight.\n\r", ch);
	    ansi(END, ch);
	    SET_BIT(ch->specials.act, PLR_AUTOASSIST);
	  }
	  break;
	case 7: /*all*/
	  SET_BIT(ch->specials.act, PLR_AUTOLOOT);
	  SET_BIT(ch->specials.act, PLR_AUTOSPLIT);
	  SET_BIT(ch->specials.act, PLR_AUTOAGGR);
	  SET_BIT(ch->specials.act, PLR_AUTOFLEE);
	  SET_BIT(ch->specials.act, PLR_AUTOASSIST);
	  ansi(CLR_ACTION, ch);
	  send_to_char("All auto-actions are now on.\n\r", ch);
	  ansi(END, ch);
	  break;
	default:
	  sprintf(buf, "Format: Auto <TOGGLE>\n\rValid toggles are: ");
	  for (i=0; *toggle[i] != '\n'; i++){
	    strcat(buf, toggle[i]);
	    strcat(buf, " ");
	  }
	  ansi(CLR_ACTION, ch);
	  send_to_char(buf, ch);
          sprintf(buf, "\n\r\n\rAuto-loot:       ");
          strcat(buf, IS_SET(ch->specials.act, PLR_AUTOLOOT) ? "Yes\n\r" : "No\n\r");
	  send_to_char(buf, ch);

          sprintf(buf, "Auto-split:      ");
          strcat(buf, IS_SET(ch->specials.act, PLR_AUTOSPLIT) ? "Yes\n\r" : "No\n\r");
	  send_to_char(buf, ch);

          sprintf(buf, "Auto-aggressive: ");
          strcat(buf, IS_SET(ch->specials.act, PLR_AUTOAGGR) ? "Yes\n\r" : "No\n\r");
	  send_to_char(buf, ch);

          sprintf(buf, "Auto-flee:       ");
          strcat(buf, IS_SET(ch->specials.act, PLR_AUTOFLEE) ? "Yes\n\r" : "No\n\r");
	  send_to_char(buf, ch);

          sprintf(buf, "Auto-assist:     ");
          strcat(buf, IS_SET(ch->specials.act, PLR_AUTOASSIST) ? "Yes\n\r" : "No\n\r");
	  send_to_char(buf, ch);
	  ansi(END, ch);
	  break;
	}
}



void do_group(struct char_data *ch, char *argument, int cmd)
{
	char name[256];
	char buf[256];
	struct char_data *victim, *k;
	struct follow_type *f;
	bool found;

	one_argument(argument, name);

	if (!*name) {
		if (!IS_AFFECTED(ch, AFF_GROUP)) {
		        ansi(CLR_ERROR, ch);
			send_to_char("But you are a member of no group?!\n\r", ch);
			ansi(END, ch);
		} else {
		        ansi(CLR_ACTION, ch);
			send_to_char("Your group consists of:\n\r", ch);
			ansi(END, ch);
			if (ch->master)
				k = ch->master;
			else
				k = ch;

			if (IS_AFFECTED(k, AFF_GROUP)) {
				sprintf(buf, "[%2d] [%-15s] [Hit %4d/%4d Mana %4d/%4d Move %4d/%4d ] (Leader)\n\r",
				  GET_LEVEL(k),
				  GET_NAME(k),
				  GET_HIT(k),
				  GET_MAX_HIT(k),
				  GET_MANA(k),
				  GET_MAX_MANA(k),
				  GET_MOVE(k),
				  GET_MAX_MOVE(k)
				);
				ansi(CLR_GROUP, ch);
				send_to_char(buf, ch);
				ansi(END, ch);
			}

			for(f=k->followers; f; f=f->next)
				if (IS_AFFECTED(f->follower, AFF_GROUP)) {
					sprintf(buf, "[%2d] [%-15s] [Hit %4d/%4d Mana %4d/%4d Move %4d/%4d ]\n\r",
					  GET_LEVEL(f->follower),
					  GET_NAME(f->follower),
					  GET_HIT(f->follower),
					  GET_MAX_HIT(f->follower),
					  GET_MANA(f->follower),
					  GET_MAX_MANA(f->follower),
					  GET_MOVE(f->follower),
					  GET_MAX_MOVE(f->follower)
					);
					ansi(CLR_ACTION, ch);
					send_to_char(buf, ch);
					ansi(END, ch);
				}
		}

		return;
	}

	if (!(victim = get_char_room_vis(ch, name))) {
	        ansi(CLR_ERROR, ch);
		send_to_char("No one here by that name.\n\r", ch);
		ansi(END, ch);
	} else {

		if (ch->master) {
			ansi_act("You can not enroll group members without being head of a group.", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
			return;
		}

		found = FALSE;

		if (victim == ch)
			found = TRUE;
		else {
			for(f=ch->followers; f; f=f->next) {
				if (f->follower == victim) {
					found = TRUE;
					break;
				}
			}
		}
		
		if (found) {
			if (IS_AFFECTED(victim, AFF_GROUP)) {
				ansi_act("$n has been kicked out of the group!", FALSE, victim, 0, ch, TO_ROOM,CLR_ERROR);
				ansi_act("You are no longer a member of the group!", FALSE, victim, 0, 0, TO_CHAR,CLR_ERROR);
				REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
			} else {
				ansi_act("$n is now a group member.", FALSE, victim, 0, 0, TO_ROOM,CLR_ACTION);
				ansi_act("You are now a group member.", FALSE, victim, 0, 0, TO_CHAR,CLR_ACTION);
				SET_BIT(victim->specials.affected_by, AFF_GROUP);
			}
		} else {
			ansi_act("$N must follow you, to enter the group", FALSE, ch, 0, victim, TO_CHAR,CLR_ERROR);
		}
	}
}


void do_quaff(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp;
  int i;
	bool equipped;

	equipped = FALSE;

  one_argument(argument,buf);

	if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
		temp = ch->equipment[HOLD];
		equipped = TRUE;
	  if ((temp==0) || !isname(buf, temp->name)) {
			ansi_act("You do not have that item.",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
  	  return;
  	}
	}

  if (temp->obj_flags.type_flag!=ITEM_POTION)
  {
    ansi_act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
    return;
  }

  ansi_act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM,CLR_ACTION);
  ansi_act("You quaff $p which dissolves.",FALSE,ch,temp,0,TO_CHAR,CLR_ACTION);

  for (i=1; i<4; i++)
    if ( (temp->obj_flags.value[i] >= 0) 
	 && (temp->obj_flags.value[i] < MAX_SPL_LIST)
	 && (spell_info[temp->obj_flags.value[i]].spell_pointer))
      ((*spell_info[temp->obj_flags.value[i]].spell_pointer)
        ((signed char) MINV(NPC_LEV,MAXV(1,temp->obj_flags.value[0])), ch, "", SPELL_TYPE_POTION, ch, 0));

  if (equipped)
	unequip_char(ch, HOLD);

  extract_obj(temp);
}


void do_recite(struct char_data *ch, char *argument, int cmd)
{
	char buf[100];
	struct obj_data *scroll, *obj;
	struct char_data *victim;
	int i, bits;
	bool equipped;

	equipped = FALSE;
	obj = 0;
	victim = 0;

	argument = one_argument(argument,buf);

	if (!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
		scroll = ch->equipment[HOLD];
		equipped = TRUE;
	  if ((scroll==0) || !isname(buf, scroll->name)) {
			ansi_act("You do not have that item.",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
			return;
  	}
	}

  if (scroll->obj_flags.type_flag!=ITEM_SCROLL)
  {
    ansi_act("Recite is normally used for scrolls.",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
    return;
  }

	if (*argument) {
	  bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM |
        FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &victim, &obj);
		if (bits == 0) {
		        ansi(CLR_ERROR, ch);
			send_to_char("No such thing around to recite the scroll on.\n\r", ch);
			ansi(END, ch);
			return;
		}
	} else {
		victim = ch;
	}

	ansi_act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM,CLR_ACTION);
	ansi_act("You recite $p which dissolves.",FALSE,ch,scroll,0,TO_CHAR,CLR_ACTION);

  for (i=1; i<4; i++)
    if ( (scroll->obj_flags.value[i] >= 1)
	 && (scroll->obj_flags.value[i] < MAX_SPL_LIST)
	 && (spell_info[scroll->obj_flags.value[i]].spell_pointer))
      ((*spell_info[scroll->obj_flags.value[i]].spell_pointer)
        ((signed char) MAXV(1,MINV(NPC_LEV,scroll->obj_flags.value[0])), ch, "", SPELL_TYPE_SCROLL, victim, obj));

  if (equipped)
	unequip_char(ch, HOLD);

  extract_obj(scroll);
}


void do_use(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct char_data *tmp_char;
  struct obj_data *tmp_object, *stick;

  int bits;

  argument = one_argument(argument,buf);

  if (ch->equipment[HOLD] == 0 ||
      !isname(buf, ch->equipment[HOLD]->name)) {
    ansi_act("You do not hold that item in your hand.",
	     FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
    return;
  }

  stick = ch->equipment[HOLD];

  if (stick->obj_flags.type_flag == ITEM_STAFF) {
    ansi_act("$n taps $p three times on the ground.",
	     TRUE, ch, stick, 0,TO_ROOM,CLR_ACTION);
    ansi_act("You tap $p three times on the ground.",
	     FALSE,ch, stick, 0,TO_CHAR,CLR_ACTION);

    if (stick->obj_flags.value[2] > 0) {  /* Is there any charges left? */
      stick->obj_flags.value[2]--;
      if(spell_info[MINV(MAX_SPL_LIST-1,
			 MAXV(1,stick->obj_flags.value[3]))].spell_pointer)
	((*spell_info[stick->obj_flags.value[3]].spell_pointer)
	 ((signed char) stick->obj_flags.value[0], ch, "", 
	  SPELL_TYPE_STAFF, 0, 0));

    } else {
      ansi(CLR_ERROR, ch);
      send_to_char("The staff seems powerless.\n\r", ch);
      ansi(END, ch);
    }
  } else if (stick->obj_flags.type_flag == ITEM_WAND) {

    bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | 
			FIND_OBJ_ROOM | FIND_OBJ_EQUIP, 
			ch, &tmp_char, &tmp_object);
    if (bits) {
      if (bits == FIND_CHAR_ROOM) {
	ansi_act("$n points $p at $N.", 
		 TRUE, ch, stick, tmp_char, TO_ROOM,CLR_ACTION);
	ansi_act("You point $p at $N.",
		 FALSE,ch, stick, tmp_char, TO_CHAR,CLR_ACTION);
      } else {
	ansi_act("$n points $p at $P.", 
		 TRUE, ch, stick, tmp_object, TO_ROOM,CLR_ACTION);
	ansi_act("You point $p at $P.",
		 FALSE,ch, stick, tmp_object, TO_CHAR,CLR_ACTION);
      }

      if (stick->obj_flags.value[2] > 0) { /* Is there any charges left? */
	stick->obj_flags.value[2]--;
	if(spell_info[MINV(MAX_SPL_LIST-1,
			   MAXV(1,stick->obj_flags.value[3]))].spell_pointer)
	  ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
	   ((signed char) stick->obj_flags.value[0], 
	    ch, "", SPELL_TYPE_WAND, tmp_char, tmp_object));
      } else {
	ansi(CLR_ERROR, ch);
	send_to_char("The wand seems powerless.\n\r", ch);
	ansi(END, ch);
      }
    } else {
      ansi(CLR_ERROR, ch);
      send_to_char("What should the wand be pointed at?\n\r", ch);
      ansi(END, ch);
    }
  } else {
    ansi(CLR_ERROR, ch);
    send_to_char("Use is normally only for wands and staffs.\n\r", ch);
    ansi(END, ch);
  }
}

void do_deposit(struct char_data *ch, char *argument, int cmd){
  ansi(CLR_ERROR, ch);
  send_to_char("Try the bankers... \n\r",ch);
  ansi(END, ch);
}

void do_withdraw(struct char_data *ch, char *argument, int cmd){
  ansi(CLR_ERROR, ch);
  send_to_char("Try the bankers... \n\r",ch);
  ansi(END, ch);
}

void do_balance(struct char_data *ch, char *argument, int cmd){
  ansi(CLR_ERROR, ch);
  send_to_char("Try the bankers... \n\r",ch);
  ansi(END, ch);
}

void do_simple_split(struct char_data *ch, long total)
{
  char buf[MAX_INPUT_LENGTH];
  int no_members, share;
  struct char_data *lead;
  struct follow_type *f;

  /* will choke if the character isnt in a group */
  if (!IS_AFFECTED(ch, AFF_GROUP)) return;
  if (total > GET_GOLD(ch)){
    total = GET_GOLD(ch);
  }

  if (!(lead=ch->master))
    lead = ch; /* find leader */

  if (IS_AFFECTED(lead, AFF_GROUP) && (lead->in_room==ch->in_room)) 
    no_members = 1;
  else
    no_members = 0;
  for (f=lead->followers; f; f=f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) 
        && (f->follower->in_room == ch->in_room)) 
      no_members +=1;

  if (no_members <= 1){
    ansi_act("You split the money with yourself.", FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
    return;
  }
  share = total/no_members;
  total = share*no_members; /* round off */
  GET_GOLD(ch) -= total;
  GET_GOLD(ch) += total%no_members; /* give back remainder */

  sprintf(buf, "You split the money and keep your share of %ld coins.", share+total%no_members);
  ansi_act(buf, FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
  
  sprintf(buf, "$n splits the money and you receive your share of %d coins.", share);
  if (IS_AFFECTED(lead, AFF_GROUP) && (lead->in_room==ch->in_room)) {
    GET_GOLD(lead) += share;
    if (lead != ch) 
      ansi_act(buf, FALSE, ch, 0, lead, TO_VICT,CLR_ACTION);
  } 
  for (f=lead->followers; f; f=f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) 
        && (f->follower->in_room == ch->in_room)) {
      GET_GOLD(f->follower) += share;
      if (f->follower != ch) act(buf, FALSE, ch, 0, f->follower, TO_VICT);
    }
}

void do_split(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int total;

  if (!IS_AFFECTED(ch, AFF_GROUP)){ 
    ansi_act("You can't split up money when you're not part of a group.", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
    return;
  }

  arg = one_argument(arg, buf);
  if (is_number(buf))
    total = atoi(buf);
  else {
    ansi_act("Split yes, fine but how much?", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
    return;
  }
  if (total > GET_GOLD(ch)){
    ansi_act("You don't have that much money.", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
    return;
  }
  do_simple_split(ch, total);
}

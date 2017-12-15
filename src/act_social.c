/* ************************************************************************
*  file: act.social.c , Implementation of commands.       Part of DIKUMUD *
*  Usage : Social commands.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "ansi.h"
#include "act.h"

/* #define REENABLE_POSES */

struct social_messg
{
	int act_nr;
	int hide;
	int min_victim_position; /* Position of victim */

	/* No argument was supplied */
	char *char_no_arg;
	char *others_no_arg;

	/* An argument was there, and a victim was found */
	char *char_found;		/* if NULL, read no further, ignore args */
	char *others_found;
	char *vict_found;

	/* An argument was there, but no victim was found */
	char *not_found;

	/* The victim turned out to be the character */
	char *char_auto;
	char *others_auto;
} *soc_mess_list = 0;


static int list_top = -1;


char *fread_action(FILE *fl)
{
	char buf[MAX_STRING_LENGTH], *rslt;

	for (;;)
	{
		fgets(buf, MAX_STRING_LENGTH, fl);
		if (feof(fl))
		{
			log("Fread_action - unexpected EOF.");
			exit(0);
		}

		if (*buf == '#')
			return(0);
		{
			*(buf + strlen(buf) - 1) = '\0';
			CREATE(rslt, char, strlen(buf) + 1);
			strcpy(rslt, buf);
			return(rslt);
		}
	}
}
	

void boot_social_messages(void)
{
	FILE *fl;
	int tmp, hide, min_pos;

	if (!(fl = fopen(SOCMESS_FILE, "r")))
	{
		perror("boot_social_messages");
		exit(0);
	}

	for (;;)
	{
		fscanf(fl, " %d ", &tmp);
		if (tmp < 0)
			break;
		fscanf(fl, " %d ", &hide);
		fscanf(fl, " %d \n", &min_pos);

		/* alloc a new cell */
		if (!soc_mess_list)
		{
			CREATE(soc_mess_list, struct social_messg, 1);
			list_top = 0;
		}
		else
			if (!(soc_mess_list = (struct social_messg *)
				realloc(soc_mess_list, sizeof (struct social_messg) *
				(++list_top + 1))))
			{
				perror("boot_social_messages. realloc");
				exit(1);
			}

		/* read the stuff */
		soc_mess_list[list_top].act_nr = tmp;
		soc_mess_list[list_top].hide = hide;
		soc_mess_list[list_top].min_victim_position = min_pos;

		soc_mess_list[list_top].char_no_arg = fread_action(fl);
		soc_mess_list[list_top].others_no_arg = fread_action(fl);

		soc_mess_list[list_top].char_found = fread_action(fl);

		/* if no char_found, the rest is to be ignored */
		if (!soc_mess_list[list_top].char_found)
			continue;

		soc_mess_list[list_top].others_found = fread_action(fl);	
		soc_mess_list[list_top].vict_found = fread_action(fl);

		soc_mess_list[list_top].not_found = fread_action(fl);

		soc_mess_list[list_top].char_auto = fread_action(fl);

		soc_mess_list[list_top].others_auto = fread_action(fl);
	}
	fclose(fl);
}



int find_action(int cmd)
{
	int bot, top, mid;

	bot = 0;
	top = list_top;

	if (top < 0)
		return(-1);

	for(;;)
	{
		mid = (bot + top) / 2;

		if (soc_mess_list[mid].act_nr == cmd)
			return(mid);
		if (bot == top)
			return(-1);

		if (soc_mess_list[mid].act_nr > cmd)
			top = --mid;
		else
			bot = ++mid;
	}
}


void do_action(struct char_data *ch, char *argument, int cmd)
{
	int act_nr;
	char buf[MAX_INPUT_LENGTH];
	struct social_messg *action;
	struct char_data *vict;

	if ((act_nr = find_action(cmd)) < 0)
	{
	        ansi(CLR_ERROR, ch);
		send_to_char("That action is not supported.\n\r", ch);
		ansi(END, ch);
		return;
	}

	action = &soc_mess_list[act_nr];

	if (action->char_found)
		one_argument(argument, buf);
	else
		*buf = '\0';

	if (!*buf)
	{
                ansi(CLR_ACTION, ch);
		send_to_char(action->char_no_arg, ch);
		send_to_char("\n\r", ch);
		ansi(END, ch);
		ansi_act(action->others_no_arg, action->hide, ch, 0, 0, TO_ROOM,CLR_ACTION);
		return;
	}

	if (!(vict = get_char_room_vis(ch, buf)))
	{
                ansi(CLR_ACTION, ch);
		send_to_char(action->not_found, ch);
		send_to_char("\n\r", ch);
		ansi(END, ch);
	}
	else if (vict == ch)
	{
                ansi(CLR_ACTION, ch);
		send_to_char(action->char_auto, ch);
		send_to_char("\n\r", ch);
		ansi(END, ch);
		ansi_act(action->others_auto, action->hide, ch, 0, 0, TO_ROOM,CLR_ACTION);
	}
	else
	{
		if (GET_POS(vict) < action->min_victim_position) {
			ansi_act("$N is not in a proper position for that.",FALSE,ch,0,vict,TO_CHAR,CLR_ERROR);
		} else {
			ansi_act(action->char_found, 0, ch, 0, vict, TO_CHAR,CLR_ACTION);

			ansi_act(action->others_found, action->hide, ch, 0, vict, TO_NOTVICT,CLR_ACTION);

			ansi_act(action->vict_found, action->hide, ch, 0, vict, TO_VICT,CLR_ACTION);
		}
	}
}



void do_insult(struct char_data *ch, char *argument, int cmd)
{
	static char buf[100];
	static char arg[MAX_STRING_LENGTH];
	struct char_data *victim;

	one_argument(argument, arg);

	if(*arg) {
		if(!(victim = get_char_room_vis(ch, arg))) {
		        ansi(CLR_ERROR, ch);
			send_to_char("Can't hear you!\n\r", ch);
			ansi(END, ch);
		} else {
			if(victim != ch) { 
				sprintf(buf, "You insult %s.\n\r",GET_NAME(victim) );
				ansi(CLR_ACTION, ch);
				send_to_char(buf,ch);
				ansi(END, ch);

				switch(random()%3) {
					case 0 : {
						if (GET_SEX(ch) == SEX_MALE) {
							if (GET_SEX(victim) == SEX_MALE)
							  ansi_act("$n accuses you of fighting like a woman!", FALSE,ch, 0, victim, TO_VICT,CLR_ACTION);
				  		else
						  ansi_act("$n says that women can't fight.", FALSE, ch, 0, victim, TO_VICT,CLR_ACTION);
						} else { /* Ch == Woman */
							if (GET_SEX(victim) == SEX_MALE)
							  ansi_act("$n accuses you of having the smallest.... (brain?)",FALSE, ch, 0, victim, TO_VICT,CLR_ACTION);
				  		else
						  ansi_act("$n tells you that you'd loose a beautycontest against a troll.",FALSE, ch, 0, victim, TO_VICT,CLR_ACTION);
						}
					} break;
					case 1 : {
					  ansi_act("$n calls your mother a bitch!",FALSE, ch, 0, victim, TO_VICT,CLR_ACTION);
					} break;
					default : {
					  ansi_act("$n tells you to get lost!",FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
					} break;
				} /* end switch */

				ansi_act("$n insults $N.", TRUE, ch, 0, victim, TO_NOTVICT,CLR_ACTION);
			} else { /* ch == victim */
			  ansi(CLR_DAM, ch);
			  send_to_char("You feel insulted.\n\r", ch);
			  ansi(END, ch);
			}
		}
	} else {
	  ansi(CLR_ACTION, ch);
	  send_to_char("Sure you don't want to insult everybody.\n\r", ch);
	  ansi(END, ch);
	}
      }


#ifdef REENABLE_POSES 
struct pose_type
{
	int level;          /* minimum level for poser */
	char *poser_msg[4];  /* message to poser        */
	char *room_msg[4];   /* message to room         */
} pose_messages[MAX_MESSAGES];

void boot_pose_messages(void)
{
  FILE *fl;
  signed char counter;
  signed char class;

  if (!(fl = fopen(POSEMESS_FILE, "r")))
  {
    perror("boot_pose_messages");
    exit(0);
  }

  for (counter = 0;;counter++)
  {
    fscanf(fl, " %d ", &pose_messages[counter].level);
    if (pose_messages[counter].level < 0)
      break;
		for (class = 0;class < 4;class++)
		{
			pose_messages[counter].poser_msg[class] = fread_action(fl);
			pose_messages[counter].room_msg[class] = fread_action(fl);
		}
	}
  
	fclose(fl);
}

void do_pose(struct char_data *ch, char *argument, int cmd)
{
	signed char to_pose;
	signed char counter;

	ansi(CLR_ERROR, ch);
	send_to_char("Sorry Buggy command.\n\r", ch);
	ansi(END, ch);
	return;

	if ((GET_LEVEL(ch) < pose_messages[0].level) || IS_NPC(ch))
	{
                ansi(CLR_ERROR, ch);
		send_to_char("You can't do that.\n\r", ch);
		ansi(END, ch);
		return;
	}

	for (counter = 0; (pose_messages[counter].level < GET_LEVEL(ch)) && 
                    (pose_messages[counter].level > 0); counter++);
	counter--;
  
	to_pose = number(0, counter);
	
	ansi_act(pose_messages[to_pose].poser_msg[GET_CLASS(ch)-1], 0, ch, 0, 0, TO_CHAR,CLR_ACTION);
	ansi_act(pose_messages[to_pose].room_msg[GET_CLASS(ch)-1], 0, ch, 0, 0, TO_ROOM,CLR_ACTION);
}
#endif

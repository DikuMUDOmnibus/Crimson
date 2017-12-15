/**************************************************************
 * act.comm.c                                                  *
 *   Implementation of communications commands                  *
 ******************************************************************/

#include		<stdio.h>
#include		<strings.h>
#include		"structs.h"
#include		"constants.h"
#include		"utils.h"
#include		"comm.h"
#include		"interpreter.h"
#include		"handler.h"
#include		"db.h"
#include		"spells.h"

	/* Arbitrary define */
#define			MAX_NOTE_LENGTH			1000

	/* External Variables */
extern struct char_data *character_list;
extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;

void do_say(struct char_data *ch, char *argument, int cmd)
{
	int i;
	static char buf[MAX_STRING_LENGTH];

#include "muzzle.h"

	for (i = 0; *(argument + i) == ' '; i++);
       
	if (!*(argument + i))
		send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
	{
		sprintf(buf,"$n says '%s'", argument + i);
		act(buf,FALSE,ch,0,0,TO_ROOM);
		sprintf(buf,"You say, '%s'\n\r",argument + i);
		send_to_char(buf,ch);
	}
}

void do_shout(struct char_data *ch, char *argument, int cmd)
{
	static char buf1[MAX_STRING_LENGTH];
	struct descriptor_data *i;

#include "muzzle.h"

	if (IS_NPC(ch) || (GET_LEVEL(ch)<IMO_LEV && IS_SET(ch->specials.act,PLR_NOSHOUT)))
	{
		send_to_char("You can't shout!!\n\r", ch);
		return;
	}

	for (; *argument == ' '; argument++);

	if (!(*argument)) {
		send_to_char("Shout? Yes! Fine! Shout we must, but WHAT??\n\r", ch);
		return;
	}
	sprintf(buf1,"You shout, '%s'\n\r",argument);
	send_to_char(buf1,ch);
	sprintf(buf1, "$n shouts '%s'", argument);

    	for (i = descriptor_list; i; i = i->next)
      	if (i->character != ch && !i->connected)
		act(buf1, 0, ch, 0, i->character, TO_VICT);
	if(ch->desc) /* anyone know what this is for? */
		ch->desc->last_input[0]='\0';
}

void do_holler(struct char_data *ch, char *argument, int cmd)
{
	static char buf1[MAX_STRING_LENGTH];
	struct descriptor_data *i;
	int zone;

#include "muzzle.h"

	for (; *argument == ' '; argument++);
        zone = world[ch->in_room].zone;
	if (!(*argument))
		send_to_char("Holler? Yes! Fine! holler we must, but WHAT??\n\r", ch);
	sprintf(buf1,"You holler, '%s'\n\r",argument);
	send_to_char(buf1,ch);
	sprintf(buf1, "$n hollers '%s'", argument);

    	for (i = descriptor_list; i; i = i->next)
      	if (i->character != ch && !i->connected && world[i->character->in_room].zone == zone)
		act(buf1, 0, ch, 0, i->character, TO_VICT);

	if(ch->desc) /* anyone know what this is for? */
		ch->desc->last_input[0]='\0';
}


void do_tell(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict, *k;
	struct follow_type *f;
	char name[MAX_INPUT_LENGTH], message[MAX_STRING_LENGTH],
		buf[MAX_STRING_LENGTH];

#include "muzzle.h"

	half_chop(argument,name,message);

	if(!*name || !*message)
		send_to_char("Who do you wish to tell what??\n\r", ch);
	else if(!(strcmp(name,"group"))) {
		if(!IS_AFFECTED(ch,AFF_GROUP))
			send_to_char("You are not grouped!\n\r",ch);
		else {
			sprintf(buf,"%s tells the group '%s'\n\r",
		 (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);
			k = (ch->master) ? ch->master : ch;
			if(IS_AFFECTED(k,AFF_GROUP)  &&
					k!=ch && GET_POS(k)>POSITION_SLEEPING)
				send_to_char(buf,k);
			for(f = k->followers ; f ; f = f->next) {
				if(IS_AFFECTED(f->follower,AFF_GROUP) &&
				f->follower!=ch &&
				GET_POS(f->follower)>POSITION_SLEEPING)
					send_to_char(buf,f->follower);
			}
				sprintf(buf,"You tell the group, '%s'\n\r",message);
				send_to_char(buf,ch);
		}
	} else if (!(vict = get_char_vis(ch, name)) || IS_SET(vict->specials.act, PLR_NOTELL))
		send_to_char("No-one by that name here..\n\r", ch);
	else if (ch == vict)
		send_to_char("You try to tell yourself something.\n\r", ch);
	else if ((GET_POS(vict) == POSITION_SLEEPING) &&
		 (GET_LEVEL(ch) < IMO_LEV2)||
		 (!IS_NPC(vict) && !vict->desc))
	 {
		act("$E can't hear you.",FALSE,ch,0,vict,TO_CHAR);
	} else {
		sprintf(buf,"%s tells you '%s'\n\r",
		  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),message);
		send_to_char(buf, vict);
			sprintf(buf,"You tell %s, '%s'\n\r",fname(GET_NAME(vict)),message);
			send_to_char(buf,ch);
	}
}

void do_whisper(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict;
	char name[MAX_INPUT_LENGTH], message[MAX_STRING_LENGTH],
		buf[MAX_STRING_LENGTH];

#include "muzzle.h"

	half_chop(argument,name,message);

	if(!*name || !*message) {
		send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
		return;
	} else if (!(vict = get_char_room_vis(ch, name))) {
		send_to_char("No-one by that name here..\n\r", ch);
		return;
	}
	if (vict == ch) {
		act("$n whispers quietly to $mself.",FALSE,ch,0,0,TO_ROOM);
		send_to_char("You can't seem to get your mouth close enough to your ear...\n\r", ch);
		return;
	} else {
		sprintf(buf,"$n whispers to you, '%s'",message);
		act(buf, FALSE, ch, 0, vict, TO_VICT);
		send_to_char("Ok.\n\r", ch);
		act("$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);
		sprintf(buf,"You whisper to %s, '%s'\n\r",fname(GET_NAME(vict)),message);
		send_to_char(buf,ch);
	}
}

void do_ask(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *vict;
	char name[MAX_INPUT_LENGTH], message[MAX_STRING_LENGTH],
		buf[MAX_STRING_LENGTH];

#include "muzzle.h"

	half_chop(argument,name,message);

	if(!*name || !*message) {
		send_to_char("Who do you want to ask something.. and what??\n\r", ch);
		return;
	} else if (!(vict = get_char_room_vis(ch, name))) {
		send_to_char("No-one by that name here..\n\r", ch);
		return;
        } else if (vict == ch) {
		act("$n quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
		send_to_char("You think about it for a while...\n\r", ch);
		return;
	}
	sprintf(buf,"$n asks you '%s'",message);
	act(buf, FALSE, ch, 0, vict, TO_VICT);
	act("$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT);
	sprintf(buf,"You ask %s, '%s'\n\r",fname(GET_NAME(vict)),message);
	send_to_char(buf,ch);
}

void do_write(struct char_data *ch, char *argument, int cmd)
{
	struct obj_data *paper = 0, *pen = 0;
	char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH],
		buf[MAX_STRING_LENGTH];

	argument_interpreter(argument, papername, penname);

	if (!ch->desc)
		return;

	if (!*papername)  /* nothing was delivered */
	{   
		send_to_char(
			"Write? On what? With what? what are you trying to do??\n\r", ch);
		return;
	}
	if (*penname) /* there were two arguments */
	{
		if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
		if (!(paper = get_obj_in_list_vis(ch, papername, world[ch->in_room].contents))){ 
			sprintf(buf, "You have no %s.\n\r", papername);
			send_to_char(buf, ch);
			return;
		}
		if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))
		{
			sprintf(buf, "You have no %s.\n\r", papername);
			send_to_char(buf, ch);
			return;
		}
	}
	else  /* there was one arg.let's see what we can find */
	{			
		if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
		{
			sprintf(buf, "There is no %s in your inventory.\n\r", papername);
			send_to_char(buf, ch);
			return;
		}
		if (paper->obj_flags.type_flag == ITEM_PEN)  /* oops, a pen.. */
		{
			pen = paper;
			paper = 0;
		}
		else if (paper->obj_flags.type_flag != ITEM_NOTE)
		{
			send_to_char("That thing has nothing to do with writing.\n\r", ch);
			return;
		}

		/* one object was found. Now for the other one. */
		if (!ch->equipment[HOLD])
		{
			sprintf(buf, "You can't write with a %s alone.\n\r", papername);
			send_to_char(buf, ch);
			return;
		}
		if (!CAN_SEE_OBJ(ch, ch->equipment[HOLD]))
		{
			send_to_char("The stuff in your hand is invisible! Yeech!!\n\r", ch);
			return;
		}
		
		if (pen)
			paper = ch->equipment[HOLD];
		else
			pen = ch->equipment[HOLD];
	}
			
	/* ok.. now let's see what kind of stuff we've found */
	if (pen->obj_flags.type_flag != ITEM_PEN)
	{
		act("$p is no good for writing with.",FALSE,ch,pen,0,TO_CHAR);
	}
	else if (paper->obj_flags.type_flag != ITEM_NOTE)
	{
		act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
	}
	else if (paper->action_description)
		send_to_char("There's something written on it already.\n\r", ch);
	else
	{
		/* we can write - hooray! */
				
		send_to_char("Ok.. go ahead and write.. end the note with a @.\n\r",
			ch);
		act("$n begins to jot down a note.", TRUE, ch, 0,0,TO_ROOM);
		ch->desc->str = &paper->action_description;
		ch->desc->max_str = MAX_NOTE_LENGTH;
	}
}

void do_gossip(struct char_data *ch, char *argument, int cmd)
{
	static char buf1[MAX_STRING_LENGTH];
	struct descriptor_data *i;

#include "muzzle.h"

	for (; *argument == ' '; argument++);
	if (!(*argument)) {
		send_to_char("Gossip? Yes! Fine! Gossip we must, but WHAT??\n\r", ch);
		return;
	}
	REMOVE_BIT(ch->specials.act, PLR_NOGOSSIP);
	sprintf(buf1, "Gossip: [$n] %s", argument);

    	for (i = descriptor_list; i; i = i->next)
  	if (i->character != ch && !i->connected && !IS_SET(i->character->specials.act, PLR_NOGOSSIP))
		act(buf1, 0, ch, 0, i->character, TO_VICT);

	sprintf(buf1,"You gossip: %s\n\r",argument);
	send_to_char(buf1,ch);
}
void do_wizinvis(struct char_data *ch, char *arg, int cmd)
{
	if (IS_SET(ch->specials.affected_by, AFF_WIZINVIS)){
		REMOVE_BIT(ch->specials.affected_by, AFF_WIZINVIS);
		send_to_char("You will now be visible\n\r",ch);
	} else {
		SET_BIT(ch->specials.affected_by, AFF_WIZINVIS);
		send_to_char("You will no longer be visible.\n\r",ch);
	}
}
void do_nogossip(struct char_data *ch, char *arg, int cmd)
{
	if (IS_SET(ch->specials.act, PLR_NOGOSSIP)){
		REMOVE_BIT(ch->specials.act, PLR_NOGOSSIP);
		send_to_char("You will now receive gossip.\n\r",ch);
	} else {
		SET_BIT(ch->specials.act, PLR_NOGOSSIP);
		send_to_char("You will no longer receive gossip.\n\r",ch);
	}
}
void do_auction(struct char_data *ch, char *argument, int cmd)
{
	static char buf1[MAX_STRING_LENGTH];
	struct descriptor_data *i;

#include "muzzle.h"

	for (; *argument == ' '; argument++);
	if (!(*argument)) {
		send_to_char("Auction? Yes! Fine! Auction we must, but WHAT??\n\r", ch);
		return;
	}
	REMOVE_BIT(ch->specials.act, PLR_NOAUCTION);
	sprintf(buf1, "Auction: [$n] %s", argument);

    	for (i = descriptor_list; i; i = i->next)
  	if (i->character != ch && !i->connected && !IS_SET(i->character->specials.act, PLR_NOAUCTION))
		act(buf1, 0, ch, 0, i->character, TO_VICT);

	sprintf(buf1,"You auction: %s\n\r",argument);
	send_to_char(buf1,ch);
}
void do_noauction(struct char_data *ch, char *arg, int cmd)
{
	if (IS_SET(ch->specials.act, PLR_NOAUCTION)){
		REMOVE_BIT(ch->specials.act, PLR_NOAUCTION);
		send_to_char("You will now receive auctions.\n\r",ch);
	} else {
		SET_BIT(ch->specials.act, PLR_NOAUCTION);
		send_to_char("You will no longer receive auctions.\n\r",ch);
	}
}
void do_grats(struct char_data *ch, char *argument, int cmd)
{
	static char buf1[MAX_STRING_LENGTH];
	struct descriptor_data *i;

#include "muzzle.h"

	for (; *argument == ' '; argument++);
	if (!(*argument)) {
		send_to_char("Grats? Yes! Fine! Grats we must, but WHAT??\n\r", ch);
		return;
	}
	REMOVE_BIT(ch->specials.act, PLR_NOGRATS);
	sprintf(buf1, "Grats: [$n] %s", argument);

    	for (i = descriptor_list; i; i = i->next)
  	if (i->character != ch && !i->connected && !IS_SET(i->character->specials.act, PLR_NOGRATS))
		act(buf1, 0, ch, 0, i->character, TO_VICT);

	sprintf(buf1,"You grats: %s\n\r",argument);
	send_to_char(buf1,ch);
}
void do_nograts(struct char_data *ch, char *arg, int cmd)
{
	if (IS_SET(ch->specials.act, PLR_NOGRATS)){
		REMOVE_BIT(ch->specials.act, PLR_NOGRATS);
		send_to_char("You will now receive grats.\n\r",ch);
	} else {
		SET_BIT(ch->specials.act, PLR_NOGRATS);
		send_to_char("You will no longer receive grats.\n\r",ch);
	}
}
void do_notell(struct char_data *ch, char *arg, int cmd)
{
	if (IS_SET(ch->specials.act, PLR_NOTELL)){
		REMOVE_BIT(ch->specials.act, PLR_NOTELL);
		send_to_char("You will now receive tell's.\n\r",ch);
	} else {
		SET_BIT(ch->specials.act, PLR_NOTELL);
		send_to_char("You will no longer receive tell's.\n\r",ch);
	}
}
void do_nosummon(struct char_data *ch, char *arg, int cmd)
{
	if (IS_SET(ch->specials.act, PLR_NOSUMMON)){
		REMOVE_BIT(ch->specials.act, PLR_NOSUMMON);
		send_to_char("You can now be summoned.\n\r",ch);
	} else {
		SET_BIT(ch->specials.act, PLR_NOSUMMON);
		send_to_char("You can no longer be summoned.\n\r",ch);
	}
}
void do_nohassle(struct char_data *ch, char *arg, int cmd)
{
	if ((GET_LEVEL(ch) < IMO_LEV2) && (!isname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))){
		send_to_char("Sorry you're too lowly for that command\n\r", ch);
		return;
	}
	if (IS_SET(ch->specials.act, PLR_NOHASSLE)){
		REMOVE_BIT(ch->specials.act, PLR_NOHASSLE);
		send_to_char("You can now be hassled by aggr. mobs.\n\r",ch);
	} else {
		SET_BIT(ch->specials.act, PLR_NOHASSLE);
		send_to_char("You can no longer be hassled by aggr. mobs.\n\r",ch);
	}
}


void do_immtalk(struct char_data *ch, char *arg, int cmd)
{
        static char buf1[MAX_STRING_LENGTH];
        struct descriptor_data *i;
 
        for (; *arg == ' '; arg++);
 
        sprintf(buf1, "Immort: [$n] %s", arg);
 
        for (i = descriptor_list; i; i = i->next)
        if (i->character != ch && !i->connected && GET_LEVEL(i->character) >= IMO_LEV)
                act(buf1, 0, ch, 0, i->character, TO_VICT);
 
        sprintf(buf1,"You immtalk: %s\n\r",arg);
        send_to_char(buf1,ch);
}

void do_ansichat(struct char_data *ch, char *arg, int cmd)
{
	static char buf1[MAX_STRING_LENGTH];
	struct descriptor_data *i;

	for (; *arg == ' '; arg++);

	for (i = descriptor_list; i; i = i->next)
	if (i->character != ch && !i->connected)
	{
		if (!SET_BIT(i->character->specials.act, PLR_ANSI))
		{
			sprintf(buf1, "AnsiChat: [$n] %s\n\r",arg);
			act(buf1, 0, ch, 0, i->character, TO_VICT);
		}
		else
		{
			sprintf(buf1, "\033[0;1mAnsiChat\033[0m: [$n] %s\n\r",arg);
			act(buf1, 0, ch, 0, i->character, TO_VICT);
		}
	}
	sprintf(buf1, "You AnsiChat: %s\n\r", arg);
	send_to_char(buf1,ch);
}


/**************************************************************
 * act.comm.c                                                  *
 *   Implementation of communications commands                  *
 ******************************************************************/

#include <stdio.h>
#include <strings.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "constants.h"
#include "spells.h"
#include "ansi.h"

#include "act.h"

/* Arbitrary define */
#define MAX_NOTE_LENGTH	1000

/* External Variables */

void do_say(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char buf[MAX_INPUT_LENGTH+128];
  
  MUZZLECHECK();
  for (i = 0; *(argument + i) == ' '; i++);
       
  if (!*(argument + i)) {
    ansi_act("Yes, but WHAT do you want to say?",
	     FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
  } else {
    sprintf(buf,"$n says '%s'", argument + i);
    ansi_act(buf,FALSE,ch,0,0,TO_ROOM,CLR_COMM);
    sprintf(buf,"You say, '%s'",argument + i);
    ansi_act(buf,FALSE,ch,0,0,TO_CHAR,CLR_COMM);
  }
}

void do_shout(struct char_data *ch, char *argument, int cmd)
{
  char buf1[MAX_INPUT_LENGTH+128];
  struct descriptor_data *i;
  
  MUZZLECHECK();

  if (IS_NPC(ch) || (GET_LEVEL(ch) < IMO_LEV2 
		     && IS_SET(ch->specials.act,PLR_NOSHOUT)))
    {
      ansi_act("You can't shout!!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  for (; *argument == ' '; argument++);

  if (!(*argument)) {
    ansi_act("Shout? Yes! Fine! Shout we must, but WHAT?",
	     FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
    return;
  }
  sprintf(buf1,"You shout, '%s'",argument);
  ansi_act(buf1, 0, ch, 0, 0, TO_CHAR, CLR_COMM);
  sprintf(buf1, "$n shouts, '%s'", argument);

  for (i = descriptor_list; i; i = i->next)
    if (i->character != ch && !i->connected)
      ansi_act(buf1, 0, ch, 0, i->character, TO_VICT, CLR_COMM);
  if (ch->desc) /* anyone know what this is for? */
    ch->desc->last_input[0]='\0';
}

void do_holler(struct char_data *ch, char *argument, int cmd)
{
  char buf1[MAX_INPUT_LENGTH+128];
  struct descriptor_data *i;
  int zone;

  MUZZLECHECK();

  for (; *argument == ' '; argument++);
  zone = world[ch->in_room].zone;
  if (!(*argument)) {
    ansi_act("Holler? Yes! Fine! Holler we must, but WHAT??",
	     FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
  }
  sprintf(buf1,"You holler, '%s'",argument);
  ansi_act(buf1, 0, ch, 0, 0, TO_CHAR, CLR_COMM);
  sprintf(buf1, "$n hollers, '%s'", argument);

  for (i = descriptor_list; i; i = i->next)
    if (i->character != ch && !i->connected 
	&& world[i->character->in_room].zone == zone)
      ansi_act(buf1, 0, ch, 0, i->character, TO_VICT, CLR_COMM);

  if(ch->desc) /* anyone know what this is for? */
    ch->desc->last_input[0]='\0';
}


void do_tell(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict, *k;
  struct follow_type *f;
  char name[MAX_INPUT_LENGTH];
  char message[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH+128];

  MUZZLECHECK();

  half_chop(argument,name,message);

  if(!*name || !*message) {
    ansi(CLR_ERROR, ch);
    send_to_char("Who do you wish to tell what??\n\r", ch);
    ansi(END, ch);
  }
  else if(!(strcmp(name,"group"))) {
    if(!IS_AFFECTED(ch,AFF_GROUP)) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not grouped!\n\r",ch);
      ansi(END, ch);
    }
    else {
      sprintf(buf,"%s tells the group, '%s'",
	      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);
      k = (ch->master) ? ch->master : ch;
      if(IS_AFFECTED(k,AFF_GROUP)  &&
	 k!=ch && GET_POS(k)>POSITION_SLEEPING)
	ansi_act(buf,FALSE,k,0,0,TO_CHAR,CLR_GROUP);
      for(f = k->followers ; f ; f = f->next) {
	if(IS_AFFECTED(f->follower,AFF_GROUP) &&
	   f->follower!=ch &&
	   GET_POS(f->follower)>POSITION_SLEEPING)
	  ansi_act(buf,FALSE,f->follower,0,0,TO_CHAR,CLR_GROUP);
      }
      sprintf(buf,"You tell the group, '%s'",message);
      ansi_act(buf,FALSE,ch,0,0,TO_CHAR,CLR_GROUP);
    }
  } else if (!(vict = get_char_vis(ch, name)) || IS_SET(vict->specials.act, PLR_NOTELL)) {
    ansi(CLR_ERROR, ch);
    send_to_char("No-one by that name here..\n\r", ch);
    ansi(END, ch);
  }
  else if (ch == vict) {
    ansi(CLR_ERROR, ch);
    send_to_char("You try to tell yourself something.\n\r", ch);
    ansi(END, ch);
  }
  else if ((GET_POS(vict) == POSITION_SLEEPING) 
	   && (GET_LEVEL(ch) < IMO_LEV2))
    /* what did this used to do  - ||(!IS_NPC(vict) && !vict->desc) */
    {
      ansi_act("$E can't hear you.",FALSE,ch,0,vict,TO_CHAR,CLR_ERROR);
    } else {
      sprintf(buf,"%s tells you, '%s'",
	      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),message);
      ansi_act(buf,FALSE,vict,0,0,TO_CHAR,CLR_COMM);
      sprintf(buf,"You tell %s, '%s'",fname(GET_NAME(vict)),message);
      ansi_act(buf,FALSE,ch,0,0,TO_CHAR,CLR_COMM);
    }
}

void do_whisper(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH]; 
  char message[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH+128];
  
  MUZZLECHECK();
  
  half_chop(argument,name,message);
  
  if(!*name || !*message) {
    ansi(CLR_ERROR, ch);
    send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
    ansi(END, ch);
    return;
  } else if (!(vict = get_char_room_vis(ch, name))) {
    ansi(CLR_ERROR, ch);
    send_to_char("No-one by that name here..\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (vict == ch) {
    ansi_act("$n whispers quietly to $mself.",FALSE,ch,0,0,TO_ROOM,CLR_COMM);
    ansi_act("You can't seem to get your mouth close enough to your ear...",
	     FALSE,ch,0,0,TO_CHAR,CLR_COMM);
    return;
  } else {
    sprintf(buf,"$n whispers to you, '%s'",message);
    ansi_act(buf, FALSE, ch, 0, vict, TO_VICT,CLR_COMM);
    ansi_act("$n whispers something to $N.", 
	     FALSE, ch, 0, vict, TO_NOTVICT, CLR_COMM);
    sprintf(buf,"You whisper to %s, '%s'",fname(GET_NAME(vict)),message);
    ansi_act(buf,FALSE,ch,0,0,TO_CHAR,CLR_COMM);
  }
}

void do_ask(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH];
  char message[MAX_INPUT_LENGTH],
  buf[MAX_INPUT_LENGTH+128];
  
  MUZZLECHECK();

  half_chop(argument,name,message);

  if(!*name || !*message) {
    ansi(CLR_ERROR, ch);
    send_to_char("Who do you want to ask something.. and what??\n\r", ch);
    ansi(END, ch);
    return;
  } else if (!(vict = get_char_room_vis(ch, name))) {
    ansi(CLR_ERROR, ch);
    send_to_char("No-one by that name here..\n\r", ch);
    ansi(END, ch);
    return;
  } else if (vict == ch) {
    ansi_act("$n quietly asks $mself a question.",
	     FALSE,ch,0,0,TO_ROOM, CLR_ACTION);
    ansi(CLR_ACTION, ch);
    send_to_char("You think about it for a while...\n\r", ch);
    ansi(END, ch);
    return;
  }
  sprintf(buf,"$n asks you, '%s'",message);
  ansi_act(buf, FALSE, ch, 0, vict, TO_VICT,CLR_COMM);
  ansi_act("$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT,CLR_COMM);
  sprintf(buf,"You ask %s, '%s'",fname(GET_NAME(vict)),message);
  ansi_act(buf,FALSE,ch,0,0,TO_CHAR,CLR_COMM);
}

void do_write(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *paper = 0, *pen = 0;
  char papername[MAX_INPUT_LENGTH];
  char penname[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  
  argument_interpreter(argument, papername, penname);
  
  if (!ch->desc)
    return;
  
  if (!*papername)  /* nothing was delivered */
    {   
      ansi(CLR_ERROR, ch);
      send_to_char("Write? On what? With what?\n\r", ch);
      ansi(END, ch);
      return;
    }
  if (*penname) /* there were two arguments */
    {
      if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
	if (!(paper=get_obj_in_list_vis(ch,papername,world[ch->in_room].contents))) { 
	  sprintf(buf, "You have no %s.\n\r", papername);
	  ansi(CLR_ERROR, ch);
	  send_to_char(buf, ch);
	  ansi(END, ch);
	  return;
	}
      if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))
	{
	  sprintf(buf, "You have no %s.\n\r", papername);
	  ansi(CLR_ERROR, ch);
	  send_to_char(buf, ch);
	  ansi(END, ch);
	  return;
	}
    } else {  /* there was one arg.let's see what we can find */
      if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
	sprintf(buf, "There is no %s in your inventory.\n\r", papername);
	ansi(CLR_ERROR, ch);
	send_to_char(buf, ch);
	ansi(END, ch);
	return;
      }
      if (paper->obj_flags.type_flag == ITEM_PEN) {  /* oops, a pen.. */
	pen = paper;
	paper = 0;
      }
      else if (paper->obj_flags.type_flag != ITEM_NOTE) {
	ansi(CLR_ERROR, ch);
	send_to_char("That thing has nothing to do with writing.\n\r", ch);
	ansi(END, ch);
	return;
      }

      /* one object was found. Now for the other one. */
      if (!ch->equipment[HOLD]) {
	sprintf(buf, "You can't write with a %s alone.\n\r", papername);
	ansi(CLR_ERROR, ch);
	send_to_char(buf, ch);
	ansi(END, ch);
	return;
      }
      if (!CAN_SEE_OBJ(ch, ch->equipment[HOLD])) {
	ansi(CLR_ERROR, ch);
	send_to_char("The stuff in your hand is invisible! Yeech!!\n\r", ch);
	ansi(END, ch);
	return;
      }
      
      if (pen)
	paper = ch->equipment[HOLD];
      else
	pen = ch->equipment[HOLD];
    }
  
  /* ok.. now let's see what kind of stuff we've found */
  if (pen->obj_flags.type_flag != ITEM_PEN) {
    ansi_act("$p is no good for writing with.",
	     FALSE,ch,pen,0,TO_CHAR,CLR_ERROR);
  }
  else if (paper->obj_flags.type_flag != ITEM_NOTE) {
    ansi_act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR,CLR_ERROR);
  }
  else if (paper->action_description) {
    ansi(CLR_ERROR, ch);
    send_to_char("There's something written on it already.\n\r", ch);
    ansi(END, ch);
  } else { /* we can write - hooray! */
    ansi(CLR_ACTION, ch);
    send_to_char("Ok.. go ahead and write.. end the note with a @.\n\r", ch);
    ansi(END, ch);
    ansi_act("$n begins to jot down a note.",TRUE, ch, 0,0,TO_ROOM,CLR_ACTION);
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}

void do_gossip(struct char_data *ch, char *argument, int cmd) 
{
  char buf1[MAX_INPUT_LENGTH+128];
  struct descriptor_data *i;

  MUZZLECHECK();

  for (; *argument == ' '; argument++);
  if (!(*argument)) {
    ansi(CLR_ERROR, ch);
    send_to_char("Gossip? Yes! Fine! Gossip we must, but WHAT??\n\r", ch);
    ansi(END, ch);
    return;
  }
  REMOVE_BIT(ch->specials.act, PLR_NOGOSSIP);
  sprintf(buf1, "Gossip: [$n] %s", argument);

  for (i = descriptor_list; i; i = i->next)
    if (i->character != ch && !i->connected 
	&& !IS_SET(i->character->specials.act, PLR_NOGOSSIP))
      ansi_act(buf1, 0, ch, 0, i->character, TO_VICT, CLR_GOSSIP);
  
  sprintf(buf1,"You gossip: %s",argument);
  ansi_act(buf1, 0, ch, 0, 0, TO_CHAR, CLR_GOSSIP);
}

void do_wizinvis(struct char_data *ch, char *arg, int cmd)
{
  if (IS_SET(ch->specials.affected_by, AFF_WIZINVIS)) {
    REMOVE_BIT(ch->specials.affected_by, AFF_WIZINVIS);
    ansi(CLR_ACTION, ch);
    send_to_char("You will now be visible.\n\r",ch);
    ansi(END, ch);
  } else {
    SET_BIT(ch->specials.affected_by, AFF_WIZINVIS);
    ansi(CLR_ACTION, ch);
    send_to_char("You will no longer be visible.\n\r",ch);
    ansi(END, ch);
  }
}

void do_nogossip(struct char_data *ch, char *arg, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_NOGOSSIP)){
    REMOVE_BIT(ch->specials.act, PLR_NOGOSSIP);
    ansi(CLR_ACTION, ch);
    send_to_char("You will now receive gossip.\n\r",ch);
    ansi(END, ch);
  } else {
    SET_BIT(ch->specials.act, PLR_NOGOSSIP);
    ansi(CLR_ACTION, ch);
    send_to_char("You will no longer receive gossip.\n\r",ch);
    ansi(END, ch);
  }
}

void do_noclan(struct char_data *ch, char *arg, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_NOCLAN)) {
    REMOVE_BIT(ch->specials.act, PLR_NOCLAN);
    ansi(CLR_ACTION, ch);
    send_to_char("You will now receive clan communications.\n\r",ch);
    ansi(END, ch);
  } else {
    SET_BIT(ch->specials.act, PLR_NOCLAN);
    ansi(CLR_ACTION, ch);
    send_to_char("You will no longer receive clan communications.\n\r",ch);
    ansi(END, ch);
  }
}

void do_nosys(struct char_data *ch, char *arg, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_NOSYS)) {
    REMOVE_BIT(ch->specials.act, PLR_NOSYS);
    ansi(CLR_ACTION, ch);
    send_to_char("You will now receive system messages.\n\r",ch);
    ansi(END, ch);
  } else {
    SET_BIT(ch->specials.act, PLR_NOSYS);
    ansi(CLR_ACTION, ch);
    send_to_char("You will no longer receive system messages.\n\r",ch);
    ansi(END, ch);
  }
}

void do_noimm(struct char_data *ch, char *arg, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_NOIMM)) {
    REMOVE_BIT(ch->specials.act, PLR_NOIMM);
    ansi(CLR_ACTION, ch);
    send_to_char("You will now receive immortal messages.\n\r",ch);
    ansi(END, ch);
  } else {
    SET_BIT(ch->specials.act, PLR_NOIMM);
    ansi(CLR_ACTION, ch);
    send_to_char("You will no longer receive immortal messages.\n\r",ch);
    ansi(END, ch);
  }
}

void do_auction(struct char_data *ch, char *argument, int cmd)
{
  char buf1[MAX_INPUT_LENGTH+128];
  struct descriptor_data *i;
  
  MUZZLECHECK();
  
  for (; *argument == ' '; argument++);
  if (!(*argument)) {
    ansi(CLR_ERROR, ch);
    send_to_char("Auction? Yes! Fine! Auction we must, but WHAT??\n\r", ch);
    ansi(END, ch);
    return;
  }
  REMOVE_BIT(ch->specials.act, PLR_NOAUCTION);
  sprintf(buf1, "Auction: [$n] %s", argument);
  
  for (i = descriptor_list; i; i = i->next)
    if (i->character != ch && !i->connected 
	&& !IS_SET(i->character->specials.act, PLR_NOAUCTION))
      ansi_act(buf1, 0, ch, 0, i->character, TO_VICT, CLR_AUCTION);

  sprintf(buf1,"You auction: %s",argument);
  ansi_act(buf1, 0, ch, 0, 0, TO_CHAR, CLR_AUCTION);
}

void do_noauction(struct char_data *ch, char *arg, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_NOAUCTION)) {
    REMOVE_BIT(ch->specials.act, PLR_NOAUCTION);
    ansi(CLR_ACTION, ch);
    send_to_char("You will now receive auctions.\n\r",ch);
    ansi(END, ch);
  } else {
    SET_BIT(ch->specials.act, PLR_NOAUCTION);
    ansi(CLR_ACTION, ch);
    send_to_char("You will no longer receive auctions.\n\r",ch);
    ansi(END, ch);
  }
}

void do_grats(struct char_data *ch, char *argument, int cmd)
{
  char buf1[MAX_INPUT_LENGTH+128];
  struct descriptor_data *i;
  
  MUZZLECHECK();
  for (; *argument == ' '; argument++);
  if (!(*argument)) {
    ansi(CLR_ERROR, ch);
    send_to_char("Grats? Yes! Fine! Grats we must, but WHAT??\n\r", ch);
    ansi(END, ch);
    return;
  }
  REMOVE_BIT(ch->specials.act, PLR_NOGRATS);
  sprintf(buf1, "Grats: [$n] %s", argument);
  
  for (i = descriptor_list; i; i = i->next)
    if (i->character != ch && !i->connected 
	&& !IS_SET(i->character->specials.act, PLR_NOGRATS))
      ansi_act(buf1, 0, ch, 0, i->character, TO_VICT, CLR_GRATS);

  sprintf(buf1,"You grats: %s\n\r",argument);
  ansi_act(buf1, 0, ch, 0, 0, TO_CHAR, CLR_GRATS);
}

void do_nograts(struct char_data *ch, char *arg, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_NOGRATS)) {
    REMOVE_BIT(ch->specials.act, PLR_NOGRATS);
    ansi(CLR_ACTION, ch);
    send_to_char("You will now receive grats.\n\r",ch);
    ansi(END, ch);
  } else {
    SET_BIT(ch->specials.act, PLR_NOGRATS);
    ansi(CLR_ACTION, ch);
    send_to_char("You will no longer receive grats.\n\r",ch);
    ansi(END, ch);
  }
}

void do_notell(struct char_data *ch, char *arg, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_NOTELL)) {
    REMOVE_BIT(ch->specials.act, PLR_NOTELL);
    ansi(CLR_ACTION, ch);
    send_to_char("You will now receive tells.\n\r",ch);
    ansi(END, ch);
  } else {
    SET_BIT(ch->specials.act, PLR_NOTELL);
    ansi(CLR_ACTION, ch);
    send_to_char("You will no longer receive tells.\n\r",ch);
    ansi(END, ch);
  }
}

void do_nosummon(struct char_data *ch, char *arg, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_NOSUMMON)) {
    REMOVE_BIT(ch->specials.act, PLR_NOSUMMON);
    ansi(CLR_ACTION, ch);
    send_to_char("You can now be summoned.\n\r",ch);
    ansi(END, ch);
  } else {
    SET_BIT(ch->specials.act, PLR_NOSUMMON);
    ansi(CLR_ACTION, ch);
    send_to_char("You can no longer be summoned.\n\r",ch);
    ansi(END, ch);
  }
}

void do_nohassle(struct char_data *ch, char *arg, int cmd)
{
  if ((!isexactname(GET_NAME(ch),zone_table[world[ch->in_room].zone].lord))
      && (GET_LEVEL(ch) < IMO_LEV2)) {
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry you're too lowly for that command.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (IS_SET(ch->specials.act, PLR_NOHASSLE)) {
    REMOVE_BIT(ch->specials.act, PLR_NOHASSLE);
    ansi(CLR_ACTION, ch);
    send_to_char("You can now be hassled by aggressive mobs.\n\r",ch);
    ansi(END, ch);
  } else {
    SET_BIT(ch->specials.act, PLR_NOHASSLE);
    ansi(CLR_ACTION, ch);
    send_to_char("You can no longer be hassled by aggressive mobs.\n\r",ch);
    ansi(END, ch);
  }
}

void do_immtalk(struct char_data *ch, char *arg, int cmd)
{
  char buf1[MAX_INPUT_LENGTH+128];
  struct descriptor_data *i;
 
  for (; *arg == ' '; arg++);
 
  sprintf(buf1, "$n immtalks: %s", arg);
 
  for (i = descriptor_list; i; i = i->next)
    if (i->character != ch && !i->connected 
	&& GET_LEVEL(i->character) >= IMO_LEV 
	&& !IS_SET(i->character->specials.act, PLR_NOIMM))
      ansi_act(buf1, 0, ch, 0, i->character, TO_VICT, CLR_IMMTALK);
 
  sprintf(buf1,"You immtalk: %s",arg);
  ansi_act(buf1, 0, ch, 0, 0, TO_CHAR, CLR_IMMTALK);
}


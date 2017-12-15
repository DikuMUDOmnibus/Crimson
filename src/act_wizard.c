/* ************************************************************************
 *  file: actwiz.c , Implementation of commands.           Part of DIKUMUD *
 *  Usage : Wizard Commands.                                               *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "limits.h"
#include "interpreter.h"
#include "constants.h"
#include "spells.h"
#include "reception.h"
#include "ansi.h"

#include "act.h"

void do_emote(struct char_data *ch, char *arg, int cmd)
{
  int i;
  char buf[MAX_STRING_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  MUZZLECHECK();
  for (i = 0; *(arg + i) == ' '; i++);
  
  if (!*(arg + i)) {
    ansi(CLR_ERROR, ch);
    send_to_char("Yes.. But what?\n\r", ch);
    ansi(END, ch);
  }
  else
    {
      sprintf(buf,"$n %s", arg + i);
      ansi_act(buf,FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
      send_to_char("Ok.\n\r", ch);
    }
}


void do_echo(struct char_data *ch, char *arg, int cmd)
{
  int j;
  char buf[MAX_INPUT_LENGTH+20];  /* room for you echo: too */
  struct descriptor_data *i;
  
  if (IS_NPC(ch))
    return;
  
  for (j = 0; *(arg + j) == ' '; j++);
  
  if (!*(arg + j)) {
    ansi(CLR_ERROR, ch);
    send_to_char("That must be a mistake...\n\r", ch);
    ansi(END, ch);
  }
  else
    {
      /*Blake Globalized Echo*/
      sprintf(buf,"You echo:  %s\n\r", arg + j);
      ansi(CLR_ACTION, ch);
      send_to_char(buf,ch);
      ansi(END, ch);
      sprintf(buf, "%s", arg + j);
      
      for (i = descriptor_list; i; i = i->next)
	if (i->character != ch && !i->connected) 
	  ansi_act(buf, 0, ch, 0, i->character, TO_VICT,YELLOW);
    }
}


void do_trans(struct char_data *ch, char *arg, int cmd)
{
  struct descriptor_data *i;
  struct char_data *victim;
  char buf[100];
  sh_int target;
  
  if (IS_NPC(ch))
    return;
  
  one_argument(arg,buf);
  if (!*buf) {
    ansi(CLR_ERROR, ch);
    send_to_char("Who do you wish to transfer?\n\r",ch);
    ansi(END, ch);
  }
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis(ch,buf))) {
      ansi(CLR_ERROR, ch);
      send_to_char("No-one by that name around.\n\r",ch);
      ansi(END, ch);
    }
    else {
      ansi_act("$n disappears in a cloud of mushrooms.", FALSE, victim, 0, 0, TO_ROOM,CLR_ACTION);
      target = ch->in_room;
      char_from_room(victim);
      char_to_room(victim,target);
      ansi_act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0,TO_ROOM,CLR_ACTION);
      ansi_act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
      do_look(victim,"",15);
      ansi(CLR_ACTION, ch);
      send_to_char("Ok. Transfer complete.\n\r",ch);
      ansi(END, ch);
    }
  } else { /* Trans All */
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected) {
	victim = i->character;
	ansi_act("$n disappears in a cloud of mushrooms.", FALSE, victim, 0, 0, TO_ROOM,CLR_ACTION);
	target = ch->in_room;
	char_from_room(victim);
	char_to_room(victim,target);
	ansi_act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM,CLR_ACTION);
	ansi_act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
	do_look(victim,"",15);
      }
    ansi(CLR_ACTION, ch);
    send_to_char("Ok.\n\r",ch);
    ansi(END, ch);
  }
}

void do_at(struct char_data *ch, char *arg, int cmd)
{
  char command[MAX_INPUT_LENGTH], loc_str[MAX_INPUT_LENGTH];
  int loc_nr, location, original_loc;
  struct char_data *target_mob;
  struct obj_data *target_obj;
  extern int top_of_world;
  
  if (IS_NPC(ch))
    return;
  
  half_chop(arg, loc_str, command);
  if (!*loc_str)
    {
      ansi(CLR_ERROR, ch);
      send_to_char("You must supply a room number or a name.\n\r", ch);
      ansi(END, ch);
      return;
    }
  
  
  if (isdigit(*loc_str))
    {
      loc_nr = atoi(loc_str);
      for (location = 0; location <= top_of_world; location++)
	if (world[location].number == loc_nr)
	  break;
	else if (location == top_of_world)
	  {
	    ansi(CLR_ERROR, ch);
	    send_to_char("No room exists with that number.\n\r", ch);
	    ansi(END, ch);
	    return;
	  }
    }
  else if ( (target_mob = get_char_vis(ch, loc_str)) )
    location = target_mob->in_room;
  else if ( (target_obj = get_obj_vis(ch, loc_str)) )
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else
      {
	ansi(CLR_ERROR, ch);
	send_to_char("The object is not available.\n\r", ch);
	ansi(END, ch);
	return;
      }
  else
    {
      ansi(CLR_ERROR, ch);
      send_to_char("No such creature or object around.\n\r", ch);
      ansi(END, ch);
      return;
    }
  
  /* a location has been found. */
  
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);
  
  /* check if the guy's still there */
  for (target_mob = world[location].people; target_mob; target_mob =
       target_mob->next_in_room)
    if (ch == target_mob)
      {
	char_from_room(ch);
	char_to_room(ch, original_loc);
      }
}

void do_goto(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int loc_nr, location, i, j;
  struct char_data *target_mob, *pers;
  struct obj_data *target_obj;
  extern int top_of_world;
  int move_reqd, mana_reqd;
  
  if (IS_NPC(ch))
    return;
  
  move_reqd = GET_MAX_MOVE(ch) / 10;
  move_reqd = MAXV(move_reqd, 100);
  mana_reqd = GET_MAX_MANA(ch) / 10;
  mana_reqd = MAXV(mana_reqd, 100);

  if (GET_LEVEL(ch) <= IMO_LEV) {
    if ((GET_MOVE(ch) < move_reqd) ||(GET_MANA(ch) < mana_reqd)) {
      ansi(CLR_ERROR, ch);
      send_to_char("Whew! you're too tired for that kind of thing.\n\r",ch);
      ansi(END, ch);
      return;	
    } else 
      GET_MOVE(ch) -= move_reqd;
      GET_MANA(ch) -= mana_reqd;
  }
  
  one_argument(arg, buf);
  if (!*buf)
    {
      ansi(CLR_ERROR, ch);
      send_to_char("You must supply a room number or a name.\n\r", ch);
      ansi(END, ch);
      return;
    }
  
  
  if (isdigit(*buf))
    {
      loc_nr = atoi(buf);
      for (location = 0; location <= top_of_world; location++)
	if (world[location].number == loc_nr)
	  break;
	else if (location == top_of_world)
	  {
	    ansi(CLR_ERROR, ch);
	    send_to_char("No room exists with that number.\n\r", ch);
	    ansi(END, ch);
	    return;
	  }
    }
  else if ( (target_mob = get_char_vis(ch, buf)) )
    location = target_mob->in_room;
  else if ( (target_obj = get_obj_vis(ch, buf)) )
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else
      {
	ansi(CLR_ERROR, ch);
	send_to_char("The object is not available.\n\r", ch);
	ansi(END, ch);
	return;
      }
  else
    {
      ansi(CLR_ERROR, ch);
      send_to_char("No such creature or object around.\n\r", ch);
      ansi(END, ch);
      return;
    }
  
  /* a location has been found. */
  if (IS_SET(world[location].room_flags, NO_TELEPORT) && (GET_LEVEL(ch) < IMO_LEV2)){
    ansi(CLR_ERROR, ch);
    send_to_char("A strange power surge interferes!\n\r",ch);
    ansi(END, ch);
    return;
  }
  /* Morals Smorals */
  if (IS_SET(world[location].room_flags, PRIVATE) && (GET_LEVEL(ch) < IMO_LEV2))
    {
      for (i = 0, pers = world[location].people; pers; pers =
	   pers->next_in_room, i++);
      if (i > 1)
	{
	  ansi(CLR_ERROR, ch);
	  send_to_char(
		       "There's a private conversation going on in that room.\n\r", ch);
	  ansi(END, ch);
	  return;
	}
    }
    /* Can't enter NOENTER zone */
    if (IS_SET(zone_table[world[location].zone].flags, ZONE_NOENTER) && (GET_LEVEL(ch) < IMO_LEV2)) {
      ansi(CLR_ERROR, ch);
      send_to_char("A strange power surge interferes!\n\r",ch);
      ansi(END, ch);
      return;
    }

  /* avatar inaccuracy */
  if (GET_LEVEL(ch) == IMO_LEV)
    for (i=0; i<10; i++) {
      int dir, flag;
      
      dir = number(0,5);
      flag = 1;
      for (j = dir+1; flag; j++) {
	if (j>5) j=0;
	if (world[location].dir_option[j]
	    &&  world[location].dir_option[j]->to_room != NOWHERE
	    && !IS_SET(world[location].room_flags, DEATH)) {
	  location = world[location].dir_option[j]->to_room;
	  flag = 0;
	}
	if (j==dir) flag = 0;
		}
    }
  
  strcpy(buf,"$n");
  if (ch->player.immortal_exit)
    strcat(buf,ch->player.immortal_exit);
  else
    strcat(buf," disappears in a puff of smoke.");
  ansi_act(buf, TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
  char_from_room(ch);
  char_to_room(ch, location);
  strcpy(buf,"$n");
  if (ch->player.immortal_enter)
    strcat(buf,ch->player.immortal_enter);
  else
    strcat(buf," magically appears in the middle of the room.");
  ansi_act(buf,TRUE,ch,0,0,TO_ROOM,CLR_ACTION);
  do_look(ch, "",15);
}


void do_stat(struct char_data *ch, char *arg, int cmd)
{
  struct affected_type *aff;
  char arg1[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct room_data *rm=0;
  struct char_data *k=0;
  struct obj_data  *j=0;
  struct obj_data  *j2=0;
  struct extra_descr_data *desc;
  struct follow_type *fol;
  int i, virtual;
  int i2;
  bool found;
  
  if (IS_NPC(ch))
    return;
  
  arg = one_argument(arg, arg1);
  
  /* no arg */
  if (!*arg1) {
    ansi(CLR_ERROR, ch);
    send_to_char("Stats on who or what?\n\r",ch);
    ansi(END, ch);
    return;
  } else {
    /* stats on room */
    if (!str_cmp("room", arg1)) {
      rm = &world[ch->in_room];
      sprintf(buf, "Room name: %s, Of zone : %d. V-Number : %d, R-number : %d\n\r", rm->name, rm->zone, rm->number, ch->in_room);
      ansi(CLR_ACTION, ch);
      send_to_char(buf, ch);
      
      sprinttype(rm->sector_type,sector_types,buf2);
      sprintf(buf, "Sector type : %s", buf2);
      send_to_char(buf, ch);
      
      strcpy(buf,"Special procedure : ");
      strcat(buf,(rm->funct) ? "Exists\n\r" : "No\n\r");
      send_to_char(buf, ch);
      
      send_to_char("Room flags: ", ch);
      sprintbit((long) rm->room_flags,room_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf,ch);
      
      send_to_char("Description:\n\r", ch);
      send_to_char(rm->description, ch);
      
      strcpy(buf, "Extra description keywords(s): ");
      if(rm->ex_description) {
	strcat(buf, "\n\r");
	for (desc = rm->ex_description; desc; desc = desc->next) {
	  strcat(buf, desc->keyword);
	  strcat(buf, "\n\r");
	}
				strcat(buf, "\n\r");
	send_to_char(buf, ch);
      } else {
	strcat(buf, "None\n\r");
	send_to_char(buf, ch);
      }
      
      strcpy(buf, "------- Chars present -------\n\r");
      for (k = rm->people; k; k = k->next_in_room)
	{
	  strcat(buf, GET_NAME(k));
	  strcat(buf,(!IS_NPC(k) ? "(PC)\n\r" : (!IS_MOB(k) ? "(NPC)\n\r" : "(MOB)\n\r")));
	}
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
      
      strcpy(buf, "--------- Contents ---------\n\r");
      for (j = rm->contents; j; j = j->next_content)
	{
	  strcat(buf, j->name);
	  strcat(buf, "\n\r");
	}
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
      
      send_to_char("------- Exits defined -------\n\r", ch);
      for (i = 0; i <= 5; i++) {
	if (rm->dir_option[i]) {
	  sprintf(buf,"Direction %s . Keyword : %s\n\r",
		  dirs[i], rm->dir_option[i]->keyword);
	  send_to_char(buf, ch);
	  strcpy(buf, "Description:\n\r  ");
	  if(rm->dir_option[i]->general_description)
	    strcat(buf, rm->dir_option[i]->general_description);
	  else
	    strcat(buf,"UNDEFINED\n\r");
	  send_to_char(buf, ch);
	  sprintbit(rm->dir_option[i]->exit_info,exit_bits,buf2);
	  sprintf(buf, "Exit flag: %s \n\rKey no: %d\n\rTo room (R-Number): %d\n\r",
		  buf2, rm->dir_option[i]->key,
		  rm->dir_option[i]->to_room);
	  send_to_char(buf, ch);
	  ansi(END, ch);
	}
      }
      return;
    }
    
    /* stat on object */
    if ( (j = get_obj_vis(ch, arg1)) ) {
      virtual = (j->item_number >= 0) ? obj_index[j->item_number].virtual : 0;
      sprintf(buf, "Object name: [%s], R-number: [%d], V-number: [%d] Item type: ",
	      j->name, j->item_number, virtual);
      sprinttype(GET_ITEM_TYPE(j),item_types,buf2);
      strcat(buf,buf2); strcat(buf,"\n\r");
      send_to_char(buf, ch);
      sprintf(buf, "Short description: %s\n\rLong description:\n\r%s\n\r",
	      ((j->short_description) ? j->short_description : "None"),
	      ((j->description) ? j->description : "None") );
      send_to_char(buf, ch);
      if(j->ex_description){
	strcpy(buf, "Extra description keyword(s):\n\r----------\n\r");
	for (desc = j->ex_description; desc; desc = desc->next) {
	  strcat(buf, desc->keyword);
	  strcat(buf, "\n\r");
	}
	strcat(buf, "----------\n\r");
	send_to_char(buf, ch);
      } else {
	strcpy(buf,"Extra description keyword(s): None\n\r");
	send_to_char(buf, ch);
      }
      
      send_to_char("Can be worn on :", ch);
      sprintbit(j->obj_flags.wear_flags,wear_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf, ch);
      
      send_to_char("Set char bits  :", ch);
      sprintbit(j->obj_flags.bitvector,affected_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf, ch);
      
      send_to_char("Extra flags: ", ch);
      sprintbit(j->obj_flags.extra_flags,extra_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf,ch);
      
      if (GET_ITEM_TYPE(j) == ITEM_WEAPON) {
	send_to_char("Weapon flags:",ch);
	sprintbit(j->obj_flags.value[0],weapon_bits,buf);
	strcat(buf,"\n\r");
	send_to_char(buf,ch);
      }
      
      sprintf(buf,"Weight: %d, Value: %d, Cost/day: %d, Timer: %d\n\r",
	      j->obj_flags.weight,j->obj_flags.cost,
	      j->obj_flags.cost_per_day,  j->obj_flags.timer);
      send_to_char(buf, ch);
      
      strcpy(buf,"In room: ");
      if (j->in_room == NOWHERE)
	strcat(buf,"Nowhere");
      else {
	sprintf(buf2,"%d",world[j->in_room].number);
	strcat(buf,buf2);
      }
      strcat(buf," ,In object: ");
      strcat(buf, (!j->in_obj ? "None" : fname(j->in_obj->name)));
      strcat(buf," ,Carried by:");
      strcat(buf, (!j->carried_by) ? "Nobody" : GET_NAME(j->carried_by));
      strcat(buf,"\n\r");
      send_to_char(buf, ch);
      
      switch (j->obj_flags.type_flag) {
      case ITEM_LIGHT : 
	sprintf(buf, "Colour : [%d]\n\rType : [%d]\n\rHours : [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2]);
	break;
      case ITEM_SCROLL : 
	sprintf(buf, "Spells : %d, %d, %d, %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3] );
	break;
      case ITEM_WAND : 
	sprintf(buf, "Spell : %d\n\rMana : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	break;
      case ITEM_STAFF : 
	sprintf(buf, "Spell : %d\n\rMana : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	break;
      case ITEM_WEAPON :
	sprintf(buf, "Tohit : %d\n\rTodam : %dD%d\n\rType : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
      case ITEM_FIREWEAPON : 
	sprintf(buf, "Tohit : %d\n\rTodam : %dD%d\n\rType : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
	    			j->obj_flags.value[3]);
	break;
				case ITEM_MISSILE : 
				  sprintf(buf, "Tohit : %d\n\rTodam : %d\n\rType : %d",
					  j->obj_flags.value[0],
					  j->obj_flags.value[1],
					  j->obj_flags.value[3]);
	break;
      case ITEM_ARMOR :
	sprintf(buf, "AC-apply : [%d]",
		j->obj_flags.value[0]);
	break;
      case ITEM_POTION : 
	sprintf(buf, "Spells : %d, %d, %d, %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]); 
	break;
      case ITEM_TRAP :
	sprintf(buf, "Spell : %d\n\r- Hitpoints : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	break;
      case ITEM_CONTAINER :
	sprintf(buf, "Max-contains : %d\n\rLocktype : %d\n\rCorpse : %s",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[3]?"Yes":"No");
	break;
      case ITEM_DRINKCON :
	sprinttype(j->obj_flags.value[2],drinks,buf2);
	sprintf(buf, "Max-contains : %d\n\rContains : %d\n\rPoisoned : %d\n\rLiquid : %s",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[3],
		buf2);
	break;
      case ITEM_NOTE :
	sprintf(buf, "Tounge : %d",
		j->obj_flags.value[0]);
	break;
      case ITEM_KEY :
	sprintf(buf, "Keytype : %d",
		j->obj_flags.value[0]);
	break;
      case ITEM_FOOD :
	sprintf(buf, "Makes full : %d\n\rPoisoned : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[3]);
	break;
      default :
	sprintf(buf,"Values 0-3 : [%d] [%d] [%d] [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
      }
      send_to_char(buf, ch);
      
      strcpy(buf,"\n\rEquipment Status: ");
      if (!j->carried_by)
	strcat(buf,"NONE");
      else {
	found = FALSE;
	for (i=0;i < MAX_WEAR;i++) {
	  if (j->carried_by->equipment[i] == j) {
	    sprinttype(i,equipment_types,buf2);
	    strcat(buf,buf2);
	    found = TRUE;
	  }
	}
	if (!found)
	  strcat(buf,"Inventory");
      }
      send_to_char(buf, ch);
      
      strcpy(buf, "\n\rSpecial procedure : ");
      if (j->item_number >= 0)
	strcat(buf, (obj_index[j->item_number].func ? "exists\n\r" : "No\n\r"));
      else
	strcat(buf, "No\n\r");
      send_to_char(buf, ch);
      
      strcpy(buf, "Contains :\n\r");
      found = FALSE;
      for(j2=j->contains;j2;j2 = j2->next_content) {
	strcat(buf,fname(j2->name));
	strcat(buf,"\n\r");
	found = TRUE;
      }
      if (!found)
	strcpy(buf,"Contains : Nothing\n\r");
      send_to_char(buf, ch);
      
      send_to_char("Can affect char :\n\r", ch);
      for (i=0;i<MAX_OBJ_AFFECT;i++) {
	sprinttype(j->affected[i].location,apply_types,buf2);
	sprintf(buf,"    Affects : %s By %d\n\r", buf2,j->affected[i].modifier);
	send_to_char(buf, ch);
      }			
      return;
    }
    
    /* mobile in world */
    if ( (k = get_char_vis(ch, arg1)) ){
      
      switch(k->player.sex) {
      case SEX_NEUTRAL : 
	strcpy(buf,"NEUTER"); 
	break;
      case SEX_MALE :
	strcpy(buf,"MALE");
	break;
      case SEX_FEMALE :
	strcpy(buf,"FEMALE");
	break;
      default : 
	strcpy(buf,"ILLEGAL-SEX!!");
	break;
      }
      
      sprintf(buf2, " %s - Name : %s [R-Number%d], In room [%d]\n\r",
	      (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
	      GET_NAME(k), k->nr, world[k->in_room].number);
      strcat(buf, buf2);
      send_to_char(buf, ch);
      if (IS_MOB(k)) {
	sprintf(buf, "V-Number [%d]\n\r", mob_index[k->nr].virtual);
	send_to_char(buf, ch);
      }
      
      strcpy(buf,"Short description: ");
      strcat(buf, (k->player.short_descr ? k->player.short_descr : "None"));
      strcat(buf,"\n\r");
      send_to_char(buf,ch);
      
			strcpy(buf,"Title: ");
      strcat(buf, (k->player.title ? k->player.title : "None"));
      strcat(buf,"\n\r");
      send_to_char(buf,ch);
      
      send_to_char("Long description: ", ch);
      if (k->player.long_descr)
	send_to_char(k->player.long_descr, ch);
      else
	send_to_char("None", ch);
      send_to_char("\n\r", ch);
      
      if (IS_NPC(k)) {
	strcpy(buf,"Monster Class: ");
	sprinttype(k->player.class,npc_class_types,buf2);
      } else {
	strcpy(buf,"Class:[");
	sprinttype(k->player.class,pc_class_types,buf2);
			}
      strcat(buf, buf2);
      
      sprintf(buf2,"] Level[%d] Alignment[%d] ",k->player.level,
	      k->specials.alignment);
      strcat(buf, buf2);
      send_to_char(buf, ch);
      /*
	 sprintf(buf,"Birth : [%ld]secs, Logon[%ld]secs, Played[%ld]secs\n\r", 
	 k->player.time.birth,
	 k->player.time.logon,
	 k->player.time.played);
	 send_to_char(buf, ch);
	 */
      sprintf(buf,"Age:[%d]Y [%d]M [%d]D [%d]H\n\r",
	      age(k).year, age(k).month, age(k).day, age(k).hours);
      send_to_char(buf,ch);
      /*
	 sprintf(buf,"Height [%d]cm  Weight [%d]pounds \n\r", GET_HEIGHT(k), GET_WEIGHT(k));
	 send_to_char(buf,ch);
	 */
      sprintf(buf,"Hometown[%d], Speaks[%d/%d/%d], (STL[%d]/per[%d]/NSTL[%d])\n\r",
	      k->player.hometown,
	      k->player.talks[0],
	      k->player.talks[1],
	      k->player.talks[2],
	      k->specials.spells_to_learn,
	      int_app[GET_INT(k)].learn,
	      wis_app[GET_WIS(k)].bonus);
      send_to_char(buf, ch);
      
      sprintf(buf,"Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]\n\r",
	      GET_STR(k), GET_ADD(k),
	      GET_INT(k),
	      GET_WIS(k),
	      GET_DEX(k),
	      GET_CON(k) );
      send_to_char(buf,ch);
      
      sprintf(buf,"Mana p.:[%d/%d+%d]  Hit p.:[%d/%d+%d]  Move p.:[%d/%d+%d]\n\r",
	      GET_MANA(k),mana_limit(k),mana_gain(k),
	      GET_HIT(k),hit_limit(k),hit_gain(k),
	      GET_MOVE(k),move_limit(k),move_gain(k) );
      send_to_char(buf,ch);
      
      sprintf(buf,"AC:[%d/10], Coins: [%d], Exp: [%d], Hitroll: [%d], Damroll: [%d]\n\r",
	      GET_AC(k),
	      GET_GOLD(k),
	      GET_EXP(k),
	      k->points.hitroll,
	      k->points.damroll );
      send_to_char(buf,ch);
      
      sprinttype(GET_POS(k),position_types,buf2);
      sprintf(buf,"Pos.: %s, Fighting: %s,",buf2,
	      ((k->specials.fighting) ? GET_NAME(k->specials.fighting) : "Nobody") );
      if (k->desc) { 
	sprinttype(k->desc->connected,connected_types,buf2);
	strcat(buf," Connect: ");
	strcat(buf,buf2);
      } 
      
      strcat(buf," Def. pos.: ");
      sprinttype((k->specials.default_pos),position_types,buf2);
      strcat(buf, buf2);
      if (IS_NPC(k))
	{
	  strcat(buf,",NPC flags: ");
	  sprintf(buf2,"%ld\n\r ",k->specials.act);
	  strcat(buf,buf2);
	  sprintbit(k->specials.act,action_bits,buf2);
	}
      else
	{
	  strcat(buf,",PC flags:\n\r ");
	  sprintbit(k->specials.act,player_bits,buf2);
	}
      
      strcat(buf, buf2);
      
      sprintf(buf2,"\n\rTimer [%d] ", k->specials.timer);
      strcat(buf, buf2);
      send_to_char(buf, ch);
      
      if (IS_MOB(k)) {
	strcpy(buf, "\n\rMobile Special procedure : ");
	strcat(buf, (mob_index[k->nr].func ? "Exists\n\r" : "None\n\r"));
	send_to_char(buf, ch);
      }
      
      if (IS_NPC(k)) {
	sprintf(buf, "NPC Bare Hand Damage %dd%d.\n\r",
					k->specials.damnodice, k->specials.damsizedice);
	send_to_char(buf, ch);
      }
      
      sprintf(buf,"Carried weight: %d   Carried items: %d\n\r",
	      IS_CARRYING_W(k),
	      IS_CARRYING_N(k) );
      send_to_char(buf,ch);
      
      for(i=0,j=k->carrying;j;j=j->next_content,i++);
      sprintf(buf,"Items in inventory: %d, ",i);
      
      for(i=0,i2=0;i<MAX_WEAR;i++)
	if (k->equipment[i]) i2++;
      sprintf(buf2,"Items in equipment: %d\n\r", i2);
      strcat(buf,buf2);
      send_to_char(buf, ch);
      
      sprintf(buf,"Apply saving throws: [%d][%d][%d][%d][%d] ",
	      k->specials.apply_saving_throw[0],
	      k->specials.apply_saving_throw[1],
	      k->specials.apply_saving_throw[2],
	      k->specials.apply_saving_throw[3],
	      k->specials.apply_saving_throw[4]);
      send_to_char(buf, ch);
      
      sprintf(buf, "Thirst:%d, Hunger:%d, Drunk:%d\n\r",
	      k->specials.conditions[THIRST],
	      k->specials.conditions[FULL],
	      k->specials.conditions[DRUNK]);
      send_to_char(buf, ch);
      
      sprintf(buf,"Normal Maxes : Mana.:[%d]  Hit p.:[%d]  Move p.:[%d]\n\r",
	      k->points.max_mana,
	      k->points.max_hit,
	      k->points.max_move );
      send_to_char(buf,ch);
      sprintf(buf,"Bonuses : Mana.:[%d]  Hit p.:[%d]  Move p.:[%d]\n\r",
	      0,
	      con_app[GET_CON(k)].hitp,
	      0 );
      send_to_char(buf,ch);
      
      
      sprintf(buf, "Master:'%s' ",
	      ((k->master) ? GET_NAME(k->master) : "NOBODY"));
      send_to_char(buf, ch);
      send_to_char("Followers:", ch);
      for(fol=k->followers; fol; fol = fol->next)
	act("    $N", FALSE, ch, 0, fol->follower, TO_CHAR);
      
      /* Showing the bitvector */
      sprintbit(k->specials.affected_by,affected_bits,buf);
      send_to_char("Affected: ", ch);
      strcat(buf,"\n\r");
      send_to_char(buf, ch);
      
      /* Routine to show what spells a char is affected by */
      if (k->affected) {
	send_to_char("\n\rAffecting Spells:\n\r--------------\n\r", ch);
	for(aff = k->affected; aff; aff = aff->next) {
	  sprintf(buf, "Spell : '%s'\n\r",spells[aff->type-1]);
	  send_to_char(buf, ch);
	  sprintf(buf,"     Modifies %s by %d points\n\r",
		  apply_types[aff->location], aff->modifier);
	  send_to_char(buf, ch);
	  sprintf(buf,"     Expires in %3d hours, Bits set ",
		  aff->duration);
	  send_to_char(buf, ch);
	  sprintbit(aff->bitvector,affected_bits,buf);
	  strcat(buf,"\n\r");
	  send_to_char(buf, ch);
	}
      }
      return;
    } else {
      send_to_char("No mobile or object by that name in the world\n\r", ch);
    }
  }
}


void do_shutdow(struct char_data *ch, char *arg, int cmd)
{
  ansi(CLR_ERROR, ch);
  send_to_char("If you want to shut something down - say so!\n\r", ch);
  ansi(END, ch);
}


void do_terminate(struct char_data *ch, char *arg, int cmd)
{
  extern int terminate, game_reboot;
  char buf[100], arg1[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  one_argument(arg, arg1);
  
  if (!*arg)
    {
      sprintf(buf, "SYS: Terminated by %s.", GET_NAME(ch) );
      ansi(CLR_SYS, ch);
      send_to_all(buf);
      log(buf);
      ansi(END, ch); 
      terminate = 1;
    }
  else if (!str_cmp(arg, "reboot"))
    {
      ansi(CLR_SYS, ch);
      sprintf(buf, "SYS: Reboot by %s.", GET_NAME(ch));
      send_to_all(buf);
      log(buf);
      ansi(END, ch);
      terminate = game_reboot = 1;
    }
  else {
    ansi(CLR_ERROR, ch);
    send_to_char("Go shut down someone your own size.\n\r", ch);
    ansi(END, ch);
  }
}

void do_snoop(struct char_data *ch, char *arg, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  struct char_data *victim;
  
  if (!ch->desc)
    return;
  
  one_argument(arg, arg1);
  
  if(!*arg1)
    {
      ansi(CLR_ERROR, ch);
      send_to_char("Snoop who ?\n\r",ch);
      ansi(END, ch);
      return;
    }
  
  if(!(victim=get_char_vis(ch, arg1)))
    {
      ansi(CLR_ERROR, ch);
      send_to_char("No such person around.\n\r",ch);
      ansi(END, ch);
      return;
    }
  
  if(!victim->desc)
    {
      ansi(CLR_ERROR, ch);
      send_to_char("There's no link.. nothing to snoop.\n\r",ch);
      ansi(END, ch);
      return;
    }
  
  if(victim == ch)
    {
      ansi(CLR_ACTION, ch);
      send_to_char("Ok, you just snoop yourself.\n\r",ch);
      ansi(END, ch);
      if(ch->desc->snoop.snooping)
	{
	  ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
	  ch->desc->snoop.snooping = 0;
	}
      return;
    }
  
  if(victim->desc->snoop.snoop_by)	
    {
      ansi(CLR_ERROR, ch);
      send_to_char("Busy already. \n\r",ch);
      ansi(END, ch);
      return;
    }
  
  if(GET_LEVEL(victim)>=GET_LEVEL(ch))
    {
      ansi(CLR_ERROR, ch);
      send_to_char("You failed.\n\r",ch);
      ansi(END, ch);
      return;
    }
  
  ansi(CLR_ACTION, ch);
  send_to_char("Ok. You begin to snoop.\n\r",ch);
  ansi(END, ch);
  
  if(ch->desc->snoop.snooping)
    ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
  
  ch->desc->snoop.snooping = victim;
  victim->desc->snoop.snoop_by = ch;
  return;
}


void do_switch(struct char_data *ch, char *arg, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  struct char_data *victim;
  
  if (IS_NPC(ch))
    return;
  
  one_argument(arg, arg1);
  
  if (!*arg1)
    {
      ansi(CLR_ERROR, ch);
      send_to_char("Switch with who?\n\r", ch);
      ansi(END, ch);
    }
  else
    {
      if (!(victim = get_char(arg1))) {
	ansi(CLR_ERROR, ch);
	send_to_char("They aren't here.\n\r", ch);
	ansi(END, ch);
      }
      else
	{
	  if (ch == victim)
	    {
	      ansi(CLR_ERROR, ch);
	      send_to_char("He he he... We are jolly funny today, eh?\n\r", ch);
	      ansi(END, ch);
	      return;
	    }
	  
	  if (!ch->desc || ch->desc->snoop.snoop_by || ch->desc->snoop.snooping)
	    {
	      ansi(CLR_ERROR, ch);
	      send_to_char("Mixing snoop & switch is bad for your health.\n\r", ch);
	      ansi(END, ch);
	      return;
	    }
	  
	  if(victim->desc || (!IS_NPC(victim))) 
	    {
	      ansi(CLR_ERROR, ch);
	      send_to_char(
			   "You can't do that, the body is already in use!\n\r",ch);
	      ansi(END, ch);
	    }
	  else
	    {
	      ansi(CLR_ACTION, ch);
	      send_to_char("Ok. Enjoy your new body.\n\r", ch);
	      ansi(END, ch);
	      
	      ch->desc->character = victim;
	      ch->desc->original = ch;
	      
	      victim->desc = ch->desc;
	      ch->desc = 0;
	    }
	}
    }
}


void do_return(struct char_data *ch, char *arg, int cmd)
{
  if(!ch->desc)
    return;
  
  if(!ch->desc->original)
    { 
      ansi(CLR_ERROR, ch);
      send_to_char("Arglebargle, glop-glyf!?!\n\r", ch);
      ansi(END, ch);
      return;
    }
  else
    {
      ansi(CLR_ACTION, ch);
      send_to_char("You return to your originaly body.\n\r",ch);
      ansi(END, ch);
      
      ch->desc->character = ch->desc->original;
      ch->desc->original = 0;
      
      ch->desc->character->desc = ch->desc; 
      ch->desc = 0;
    }
}


void do_force(struct char_data *ch, char *arg, int cmd)
{
  struct descriptor_data *i;
  struct char_data *vict;
  char name[100], to_force[100],buf[100]; 
  
  if (IS_NPC(ch))
    return;
  
  half_chop(arg, name, to_force);
  
  if (!*name || !*to_force) {
    ansi(CLR_ERROR, ch);
    send_to_char("Who do you wish to force to do what?\n\r", ch);
    ansi(END, ch);
  }
  else if (str_cmp("all", name)) {
    if (!(vict = get_char_vis(ch, name))) {
      ansi(CLR_ERROR, ch);
      send_to_char("No-one by that name here..\n\r", ch);
      ansi(END, ch);
    }
    else
      {
	if ((GET_LEVEL(ch) < GET_LEVEL(vict)) && !IS_NPC(vict)) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You can't control your superior!!\n\r", ch);
	  ansi(END, ch);
	}
	else {
	  sprintf(buf, "$n has forced you to '%s'.", to_force);
	  ansi_act(buf, FALSE, ch, 0, vict, TO_VICT,CLR_ACTION);
	  ansi(CLR_ACTION, ch);
	  send_to_char("Ok.\n\r", ch);
	  ansi(END, ch);
	  command_interpreter(vict, to_force);
	}
      }
  } else { /* force all */
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected) {
	vict = i->character;
	if ((GET_LEVEL(ch) < GET_LEVEL(vict)) && !IS_NPC(vict)) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You can't control your superior!!\n\r", ch);
	  ansi(END, ch);
	}
	else {
	  sprintf(buf, "$n has forced you to '%s'.", to_force);
	  ansi_act(buf, FALSE, ch, 0, vict, TO_VICT,CLR_ACTION);
	  command_interpreter(vict, to_force);
	}
      }
    ansi(CLR_ACTION, ch);
    send_to_char("Ok.\n\r", ch);
    ansi(END, ch);
  }
  sprintf(buf,"SYS: [%s] force%s",ch->player.name,arg);
  log(buf);
  do_sys(buf, IMO_LEV2);
}

void do_vnum(struct char_data *ch, char *arg, int cmd)
{
  extern int top_of_mobt;
  extern int top_of_objt;
  extern struct index_data *obj_index;
  extern struct index_data *mob_index;
  char buf[256], buf1[256];
  int i;
  
  arg = one_argument(arg, buf);
  if (*buf) 
    arg = one_argument(arg, buf1);
  if (!*buf || !*buf1) {	
    send_to_char("Try vnum <obj || mob> <keyword>\n\r", ch);
    return;
  }
  if (is_abbrev(buf, "obj")) {
    /* its an obj */
    for (i=0; i<=top_of_objt; i++) {
      if (isname(buf1, obj_index[i].name)) {
	sprintf(buf, "[%5d] (%3d) %s\n\r", obj_index[i].virtual, obj_index[i].number, obj_index[i].name);
	send_to_char(buf, ch);
      }
    }
  } else if (is_abbrev(buf, "mob")) {
	  /* its a mob */
    for (i=0; i<=top_of_mobt; i++) {
      if (isname(buf1, mob_index[i].name)) {
	sprintf(buf, "[%5d] (%3d) %s\n\r", mob_index[i].virtual, mob_index[i].number, mob_index[i].name);
	send_to_char(buf, ch);
      }
    }
  } else
    send_to_char("Try vnum <obj || mob> <keyword>\n\r", ch);
  
}


void do_load(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *mob;
  struct obj_data *obj;
  char type[100], num[100];
  int number, r_num;
  char buf[MAX_STRING_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  argument_interpreter(arg, type, num);
  
  if (!*type)
    {
      send_to_char("Syntax:\n\r   load <'char/mob' | 'obj'> <number>.\n\r", ch);
      send_to_char("or load <news, credits, motd, help, info, wizlist, story, or greeting>\n\r", ch);
      return;
    }
  if (is_abbrev(type, "news")) {
    free(news);
    news = file2string(NEWS_FILE);
    send_to_char("News re-loaded\n\r", ch);
    return;
  } else if (is_abbrev(type, "credits")) {
    free(credits);
    news = file2string(CREDITS_FILE);
    send_to_char("Credits re-loaded\n\r", ch);
    return;
  } else if (is_abbrev(type, "motd")) {
    free(motd);
    motd = file2string(MOTD_FILE);
    send_to_char("MOTD re-loaded\n\r", ch);
    return;
  } else if (is_abbrev(type, "help")) {
    free(help);
    help = file2string(HELP_PAGE_FILE);
    send_to_char("Help re-loaded\n\r", ch);
    return;
  } else if (is_abbrev(type, "info")) {
    free(info);
    info = file2string(INFO_FILE);
    send_to_char("Info re-loaded\n\r", ch);
    return;
  } else if (is_abbrev(type, "wizlist")) {
    free(wizlist);
    wizlist = file2string(WIZLIST_FILE);
    send_to_char("Wizlist re-loaded\n\r", ch);
    return;
  } else if (is_abbrev(type, "story")) {
    free(story);
    story = file2string("story");
    send_to_char("Story re-loaded\n\r", ch);
    return;
  } else if (is_abbrev(type, "greeting")) {
    free(greeting);
    greeting = file2string("greeting");
    send_to_char("Greeting re-loaded\n\r", ch);
    return;
  } else if (!*type || !*num || !isdigit(*num)) {
    send_to_char("Syntax:\n\r   load <'char/mob' | 'obj'> <number>.\n\r", ch);
    send_to_char("or load <news, credits, motd, help, info, wizlist, story, or greeting>\n\r", ch);
    return;
  }
  
  if ((number = atoi(num)) < 0)
    {
      send_to_char("A NEGATIVE number??\n\r", ch);
      return;
    }
  if ((is_abbrev(type, "char"))
      ||(is_abbrev(type, "mob")))
    {
      if ((r_num = real_mobile(number)) < 0)
	{
	  send_to_char("There is no monster with that number.\n\r", ch);
	  return;
	}
      mob = read_mobile(r_num, REAL);
      char_to_room(mob, ch->in_room);
      
      act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
	  0, 0, TO_ROOM);
      act("$n has summoned $N from the ether!", FALSE, ch, 0, mob, TO_ROOM);
      act("You have made $N",FALSE,ch,0,mob,TO_CHAR);
    }
  else if (is_abbrev(type, "obj"))
    {
      if ((r_num = real_object(number)) < 0)
	{
	  send_to_char("There is no object with that number.\n\r", ch);
	  return;
	}
      obj = read_object(r_num, REAL);
      obj_to_room(obj, ch->in_room);
      act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
      act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
      act("You have created $p!",FALSE,ch,obj,0,TO_CHAR);
    }
  else {
		send_to_char("Syntax:\n\r   load <'char/mob' | 'obj'> <number>.\n\r", ch);
		send_to_char("or load <news, credits, motd, help, info, wizlist, story, or greeting>\n\r", ch);
		return;
	      }
  /* this will only log load obj/char commands here */
  sprintf(buf,"SYS: [%s] load%s",ch->player.name,arg);
  log(buf);
  do_sys(buf, IMO_LEV2);
}

/* clean a room of all mobiles and objects */
void do_purge (struct char_data *ch, char *arg, int cmd)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;
  int zone;
  char name[100], buf[100];
  
  /* error check bypassed by negative cmd (and no output too!)*/
  /* so that zreboot is faster mostly */
  if (cmd >= 0) {
    if (IS_NPC(ch))
      return;
    one_argument(arg, name);
    zone = world[ch->in_room].zone;
    if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
      send_to_char("You are not authorized to do that here.\n\r", ch);
      return;
		}
  } else
    *name = '\0';
  
  if (*name)  /* arg supplied. destroy single object or char */
    {
      if ( (vict = get_char_room_vis(ch, name)) )
	{
	  if (!IS_NPC(vict) && (GET_LEVEL(ch)<IMO_LEV)) {
	    send_to_char("Fuuuuuuuuu!\n\r", ch);
	    sprintf(buf,"IMM: %s purge %s",ch->player.name,name);
	    log(buf);
	    do_sys(buf, IMO_LEV);
	    return;
	  }
	  
	  act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);
	  act("You disintegrate $N.", FALSE, ch, 0, vict, TO_CHAR);
	  
	  if (IS_NPC(vict)) {
	    extract_char(vict);
	  } else {
	    if (vict->desc) 
	      {
		close_socket(vict->desc);
		vict->desc = 0;
		extract_char(vict);
	      }
	    else 
	      {
		extract_char(vict);
	      }
	  }
	}
      else if ( (obj = get_obj_in_list_vis(ch, name,world[ch->in_room].contents)) )
	{
	  act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
	  act("You destroy $p.", FALSE, ch, obj, 0, TO_CHAR);
	  extract_obj(obj);
	}
      else
	{
	  send_to_char("I don't know anyone or anything by that name.\n\r", ch);
	  return;
	}
      
  } else { /* no arg. clean out room */
    if (cmd >= 0) { /* neg command masks output(for zreboot mostly) */
      send_to_char("You purge the room of mobiles and objects.\n\r",ch);
      act("$n gestures... You are surrounded by scorching flames!", 
	  FALSE, ch, 0, 0, TO_ROOM);
      send_to_room("The world seems a little cleaner.\n\r", ch->in_room);
    }
    for (vict = world[ch->in_room].people; vict; vict = next_v)
      {
	next_v = vict->next_in_room;
	if (IS_NPC(vict))
	  extract_char(vict);
      }
    
    for (obj = world[ch->in_room].contents; obj; obj = next_o)
      {
	next_o = obj->next_content;
	extract_obj(obj);
      }
  }
}



/* Give pointers to the five abilities */
void roll_abilities(struct char_data *ch)
{
  int i, j, k, temp;
  byte table[5];
  byte rools[4];
  
  for(i=0; i<5; table[i++]=0)  ;
  
  for(i=0; i<5; i++) {
    
    for(j=0; j<4; j++) {
      if (GET_LEVEL(ch) >= 8) {
	rools[j] = number(3,6);
      } 
      if (GET_LEVEL(ch) <= 2) {
	rools[j] = 5;
      } else {
	rools[j] = number(2,6);
      }
    }
    temp = rools[0]+rools[1]+rools[2]+rools[3] -
      MINV(rools[0], MINV(rools[1], MINV(rools[2],rools[3])));
    
    for(k=0; k<5; k++)
      if (table[k] < temp)
	SWITCH(temp, table[k]);
  }
  
  ch->abilities.str_add = 0;
  if (GET_LEVEL(ch) == 8) { /* at 8th level reroll abilities are guaranteed to go up */
    switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER: {
      ch->abilities.intel = MAXV(ch->abilities.intel, table[0]);
      ch->abilities.wis   = MAXV(ch->abilities.wis  , table[1]);
      ch->abilities.dex   = MAXV(ch->abilities.dex  , table[2]);
      ch->abilities.str   = MAXV(ch->abilities.str  , table[3]);
      ch->abilities.con   = MAXV(ch->abilities.con  , table[4]);
    }	break;
    case CLASS_CLERIC: {
      ch->abilities.wis   = MAXV(ch->abilities.wis  , table[0]);
      ch->abilities.intel = MAXV(ch->abilities.intel, table[1]);
      ch->abilities.str   = MAXV(ch->abilities.str  , table[2]);
      ch->abilities.dex   = MAXV(ch->abilities.dex  , table[3]);
      ch->abilities.con   = MAXV(ch->abilities.con  , table[4]);
    }	break;
    case CLASS_THIEF: {
      ch->abilities.dex   = MAXV(ch->abilities.dex  , table[0]);
      ch->abilities.str   = MAXV(ch->abilities.str  , table[1]);
      ch->abilities.con   = MAXV(ch->abilities.con  , table[2]);
      ch->abilities.wis   = MAXV(ch->abilities.wis  , table[3]);
      ch->abilities.intel = MAXV(ch->abilities.intel, table[4]);
    }	break;
    case CLASS_WARRIOR: {
      ch->abilities.str   = MAXV(ch->abilities.str  , table[0]);
      ch->abilities.dex   = MAXV(ch->abilities.dex  , table[1]);
      ch->abilities.con   = MAXV(ch->abilities.con  , table[2]);
      ch->abilities.wis   = MAXV(ch->abilities.wis  , table[3]);
      ch->abilities.intel = MAXV(ch->abilities.intel, table[4]);
      ch->abilities.str_add = MAXV(ch->abilities.str_add, number(20,100));
    }	break;
    case CLASS_BARD: {
      ch->abilities.dex   = MAXV(ch->abilities.dex  , table[0]);
      ch->abilities.intel = MAXV(ch->abilities.intel, table[1]);
      ch->abilities.wis   = MAXV(ch->abilities.wis  , table[2]);
      ch->abilities.str   = MAXV(ch->abilities.str  , table[3]);
      ch->abilities.con   = MAXV(ch->abilities.con  , table[4]);
    } break;
    case CLASS_KAI: {
      ch->abilities.wis   = MAXV(ch->abilities.wis  , table[0]);
      ch->abilities.str   = MAXV(ch->abilities.str  , table[1]);
      ch->abilities.intel = MAXV(ch->abilities.intel, table[2]);
      ch->abilities.con   = MAXV(ch->abilities.con  , table[3]);
      ch->abilities.dex   = MAXV(ch->abilities.dex  , table[4]);
      ch->abilities.str_add = MAXV(ch->abilities.str_add, number(10,60));
    } break;
    case CLASS_DRAKKHAR: {
      ch->abilities.wis   = MAXV(ch->abilities.wis  , table[0]);
      ch->abilities.str   = MAXV(ch->abilities.str  , table[1]);
      ch->abilities.intel = MAXV(ch->abilities.intel, table[2]);
      ch->abilities.con   = MAXV(ch->abilities.con  , table[3]);
      ch->abilities.dex   = MAXV(ch->abilities.dex  , table[4]);
      ch->abilities.str_add = MAXV(ch->abilities.str_add, number(10,60));
    } break;
    case CLASS_EKNIGHT: {
      ch->abilities.intel = MAXV(ch->abilities.intel, table[0]);
      ch->abilities.dex   = MAXV(ch->abilities.dex  , table[1]);
      ch->abilities.wis   = MAXV(ch->abilities.wis  , table[2]);
      ch->abilities.str   = MAXV(ch->abilities.str  , table[3]);
      ch->abilities.con   = MAXV(ch->abilities.con  , table[4]);
      ch->abilities.str_add = MAXV(ch->abilities.str_add, number(0,60));
    } break;
    case CLASS_DKNIGHT: {
      ch->abilities.intel = MAXV(ch->abilities.intel, table[0]);
      ch->abilities.dex   = MAXV(ch->abilities.dex  , table[1]);
      ch->abilities.wis   = MAXV(ch->abilities.wis  , table[2]);
      ch->abilities.str   = MAXV(ch->abilities.str  , table[3]);
      ch->abilities.con   = MAXV(ch->abilities.con  , table[4]);
      ch->abilities.str_add = MAXV(ch->abilities.str_add, number(0,60));
    } break;
    case CLASS_DRAGONW: {
      ch->abilities.con   = MAXV(ch->abilities.con  , table[0]);
      ch->abilities.str   = MAXV(ch->abilities.str  , table[1]);
      ch->abilities.wis   = MAXV(ch->abilities.wis  , table[2]);
      ch->abilities.dex   = MAXV(ch->abilities.dex  , table[3]);
      ch->abilities.intel = MAXV(ch->abilities.intel, table[4]);
      ch->abilities.str_add = MAXV(ch->abilities.str_add, number(20,100));
    } break;
    }
  } else { /* normal reroll */
    switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER: {
      ch->abilities.intel = table[0];
      ch->abilities.wis   = table[1];
      ch->abilities.dex   = table[2];
      ch->abilities.str   = table[3];
      ch->abilities.con   = table[4];
    }	break;
    case CLASS_CLERIC: {
      ch->abilities.wis   = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.str   = table[2];
      ch->abilities.dex   = table[3];
      ch->abilities.con   = table[4];
    }	break;
    case CLASS_THIEF: {
      ch->abilities.dex   = table[0];
      ch->abilities.str   = table[1];
      ch->abilities.con   = table[2];
      ch->abilities.wis   = table[3];
      ch->abilities.intel = table[4];
    }	break;
    case CLASS_WARRIOR: {
      ch->abilities.str   = table[0];
      ch->abilities.dex   = table[1];
      ch->abilities.con   = table[2];
      ch->abilities.wis   = table[3];
      ch->abilities.intel = table[4];
      ch->abilities.str_add = MINV(100, number(20,120));
    }	break;
    case CLASS_BARD: {
      ch->abilities.dex   = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.wis   = table[2];
      ch->abilities.str   = table[3];
      ch->abilities.con   = table[4];
    } break;
    case CLASS_KAI: {
      ch->abilities.wis = table[0];
      ch->abilities.str = table[1];
      ch->abilities.intel = table[2];
      ch->abilities.con = table[3];
      ch->abilities.dex = table[4];
      ch->abilities.str_add = number(10,60);
    } break;
    case CLASS_DRAKKHAR: {
      ch->abilities.wis = table[0];
      ch->abilities.str = table[1];
      ch->abilities.intel = table[2];
      ch->abilities.con = table[3];
      ch->abilities.dex = table[4];
      ch->abilities.str_add = number(10,60);
    } break;
    case CLASS_EKNIGHT: {
      ch->abilities.intel = table[0];
      ch->abilities.dex = table[1];
      ch->abilities.wis = table[2];
      ch->abilities.str = table[3];
      ch->abilities.con = table[4];
      ch->abilities.str_add = number(0,60);
    } break;
    case CLASS_DKNIGHT: {
      ch->abilities.intel = table[0];
      ch->abilities.dex = table[1];
      ch->abilities.wis = table[2];
      ch->abilities.str = table[3];
      ch->abilities.con = table[4];
      ch->abilities.str_add = number(0,60);
    } break;
    case CLASS_DRAGONW: {
      ch->abilities.con = table[0];
      ch->abilities.str = table[1];
      ch->abilities.wis = table[2];
      ch->abilities.dex = table[3];
      ch->abilities.intel = table[4];
      ch->abilities.str_add = number(20,100);
    } break;
    }
  } /* unindented if 8th level business */
  ch->tmpabilities = ch->abilities;
}



void do_start(struct char_data *ch)
{
  int i;
  int starter_kit[]={3010,3010,3102,3102,3403,3404,-1};
  struct obj_data *obj;
  char buf[100];
  
  GET_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;
  set_title(ch);
  roll_abilities(ch);
  
  for (i=0; starter_kit[i] != -1; i++)
    if ( (obj = read_object(starter_kit[i], VIRTUAL)) ) {
      act("Gods have pity on you and give you $p.",FALSE,ch,obj,0,TO_CHAR);
      obj_to_char(obj, ch);
    }
  send_to_char("[0;37mA divine voice says to you, 'Welcome. You are now in [1;31mCrimsonMUD[0;37m.\n\r", ch);
  send_to_char("You should earn some experience points, and befriend some people.\n\r", ch);
  send_to_char("\n\rPlease type [1;33mINFO<return>[0;37m for instructions on how to play!\n\r", ch);
  send_to_char("You are in the Dressing Room now.  There is a [1;37mmap[0;37m here to guide you.'\n\r\n\r", ch);
  send_to_char("If you are new to the game, please go NORTH from here and visit the [1;31mAcademy[0;37m.\n\r\n\r", ch);
  send_to_char("You could email [1;33mM Say[0;37m or the [1;33mSYSOP[0;37m at the BBS if you have any problems/questions regarding the MUD.\n\r", ch);
  
  
  ch->points.max_hit  =  10;  /* These are BASE numbers   */
  ch->points.max_move  = 80;  /* These are BASE numbers   */
  ch->points.max_mana  = 10;  
  /* New player now receives some coins from start */
  ch->points.bank = 3000;
  ch->points.gold = 1000;
  
  switch (GET_CLASS(ch)) {
    
  case CLASS_MAGIC_USER : 
    ch->points.max_mana  = 100;  
    ch->specials.spells_to_learn = 2;
    break;
    
  case CLASS_CLERIC: 
    ch->points.max_mana  = 110;  
    ch->specials.spells_to_learn = 2;
    break;
    
  case CLASS_KAI : 
  case CLASS_EKNIGHT:
    ch->points.max_mana  = 90;  
    ch->points.max_hit += 2;
    GET_ALIGNMENT(ch) = 1000;
    ch->specials.spells_to_learn = 1;
    break;
    
  case CLASS_DRAKKHAR : 
  case CLASS_DKNIGHT:
    ch->points.max_mana  = 90;  
    ch->points.max_hit += 2;
    GET_ALIGNMENT(ch) = -1000;
    ch->specials.spells_to_learn = 1;
    break;
    
  case CLASS_THIEF : {
    ch->specials.spells_to_learn = 4;
  } break;
    
  case CLASS_BARD : {
    ch->points.max_mana  = 80;  
    ch->specials.spells_to_learn = 2;
  } break;
    
  case CLASS_WARRIOR: {
    ch->points.max_hit += 5;
  } break;
    
  case CLASS_DRAGONW: {
    ch->points.max_hit += 8;
  } break;
  }
  
  advance_level(ch);
  
  GET_HIT(ch) = hit_limit(ch);
  GET_MANA(ch) = mana_limit(ch);
  GET_MOVE(ch) = move_limit(ch);
  
  GET_COND(ch,THIRST) = 48;
  GET_COND(ch,FULL) = 48;
  GET_COND(ch,DRUNK) = 0;
  
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);
  SET_BIT(ch->specials.act, PLR_WIMPY);
  SET_BIT(ch->specials.act, PLR_ASSIST);
  SET_BIT(ch->specials.act, PLR_SHOW_EXITS);
  SET_BIT(ch->specials.act, PLR_SHOW_MOVE);
  SET_BIT(ch->specials.act, PLR_NOSYS);
  SET_BIT(ch->specials.act, PLR_ANSI);
  SET_BIT(ch->specials.act, PLR_AUTOLOOT);
  SET_BIT(ch->specials.act, PLR_AUTOSPLIT);
  if (GET_MANA(ch) > 50)
    SET_BIT(ch->specials.act, PLR_SHOW_MANA);
  SET_BIT(ch->specials.act, PLR_SHOW_HP);

  sprintf(buf, "[1;33mGossip: [The Mayor of Midgaard] Please welcome %s who has just become a proud citizen of Midgaard.\n\r[0;37m", GET_NAME(ch)); 
  send_to_all(buf); 
}

void do_advance(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *victim;
  char name[MAX_INPUT_LENGTH];
  char level[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char passwd[MAX_INPUT_LENGTH];
  int adv, newlevel;
  int i;
  
  half_chop(arg, name, buf);
  argument_interpreter(buf, level, passwd);
  
  if (*name)
    {
      if (!(victim = get_char_room_vis(ch, name)))
	{
	  send_to_char("That player is not here.\n\r", ch);
	  return;
	}
  } else {
    send_to_char("Advance whom?\n\r", ch);
    return;
  }
  
  if (IS_NPC(victim)) {
    send_to_char("NO! Not on NPC's.\n\r", ch);
    return;
  }
  
  if (GET_LEVEL(victim) == 0)
    adv = 1;
  else if (!*level) {
    send_to_char("You must supply a level number.\n\r", ch);
    return;
  } else {
    if (!isdigit(*level))
      {
	send_to_char("Second arg must be a positive integer.\n\r",ch);
	return;
      }
  }
  newlevel = atoi(level);
  
  if (newlevel<1)
    {
      send_to_char("1 is the lowest possible level.\n\r", ch);
      return;
    }
  
  
  if (newlevel > IMP_LEV)
    {
      send_to_char("Too high...try again.\n\r", ch);
      return;
    }
  
  if(newlevel > GET_LEVEL(ch))
    {
      send_to_char("No can do...\n\r",ch);
      return;
    }
  
  if (newlevel < GET_LEVEL(victim))
    {
      send_to_char("Warning: Lowering a player's level!\n\r",ch);
      
      GET_LEVEL(victim) = 1;
      GET_EXP(victim) = 1;
      set_title(victim);
      
      victim->points.max_hit  =  10;  /* These are BASE numbers   */
      victim->points.max_move  = 80;  /* These are BASE numbers   */
      victim->points.max_mana  = 10;  
      for(i=1;i<=MAX_SKILLS;i++) { /* Zero them out first */
	victim->skills[i].learned=0;
      }
      victim->specials.spells_to_learn = 0;
      
      switch (GET_CLASS(victim)) {
      case CLASS_THIEF : {
	victim->skills[SKILL_SNEAK].learned = 10;
	victim->skills[SKILL_HIDE].learned =  5;
	victim->skills[SKILL_STEAL].learned = 15;
	victim->skills[SKILL_BACKSTAB].learned = 10;
	victim->skills[SKILL_PICK_LOCK].learned = 10;
      } break;
      }
      
      GET_HIT(victim) = hit_limit(victim);
      GET_MANA(victim) = mana_limit(victim);
      GET_MOVE(victim) = move_limit(victim);
      GET_COND(victim,THIRST) = 24;
      GET_COND(victim,FULL) = 24;
      GET_COND(victim,DRUNK) = 0;
      
      advance_level(victim);
    }
  
  adv = newlevel - GET_LEVEL(victim);
  
  send_to_char("You feel generous.\n\r", ch);
  act("$n makes some strange gestures.  A strange feeling comes upon you,"
      "\n\rLike a giant hand, light comes down from above, grabbing your body,"
      "\n\rthat begins to pulse with colored lights from inside.  Your head"
      "\n\rhead seems to be filled with demons from another plane as your body"
      "\n\rdissolves to the elements of time and space itself.  Suddenly a"
      "\n\rsilent explosion of light snaps you back to reality. You feel"
      "\n\rslightly different.",FALSE,ch,0,victim,TO_VICT);
  
  if (GET_LEVEL(victim) == 0) {
    do_start(victim);
    GET_EXP(victim)=1;
  }
  
  gain_exp_regardless(victim, 
		      (titles[GET_CLASS(victim)-1]
		       [GET_LEVEL(victim)+adv].exp)-GET_EXP(victim));
  
  sprintf(buf,"SYS: [%s] advance%s",ch->player.name,arg);
  log(buf);
  do_sys(buf, IMO_LEV2);
}

void do_reroll(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *victim;
  char buf[100];
  struct descriptor_data *q;
  
  if (IS_NPC(ch))
    return;
  
  one_argument(arg,buf);
  if (!*buf)
    send_to_char("Who do you wish to reroll?\n\r",ch);
  else
    if(!(victim = get_char(buf)))
      send_to_char("No-one by that name in the world.\n\r",ch);
    else {
      send_to_char("Rerolled...\n\r", ch);
      roll_abilities(victim);
    }
  sprintf(buf,"IMM: %s reroll %s",ch->player.name,arg);
  log(buf);
  
  for (q = descriptor_list; q; q = q->next)
    if (!q->connected && GET_LEVEL(q->character) > IMO_LEV)
      act(buf, 0, ch, 0, q->character, TO_VICT);
  
  send_to_char(buf,ch);
}


void do_restore(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *victim;
  char buf[100];
  int i;
  
  void update_pos( struct char_data *victim );
  
  if (IS_NPC(ch))
    return;
  
  one_argument(arg,buf);
  if (!*buf)
    send_to_char("Who do you wich to restore?\n\r",ch);
  else
    if(!(victim = get_char(buf)))
      send_to_char("No-one by that name in the world.\n\r",ch);
    else {
      if (GET_LEVEL(victim) > IMO_LEV) {
	for (i = 0; i < MAX_SKILLS; i++) {
	  victim->skills[i].learned = 100;
	  victim->skills[i].recognise = TRUE;
	}
	
	if (GET_LEVEL(victim) >= IMO_LEV4) {
	  victim->points.max_hit = 99;
	  victim->points.max_mana = 99;
	  victim->points.max_move = 99;
	  victim->abilities.str_add = 100;
	  victim->abilities.intel = 25;
	  victim->abilities.wis = 25;
	  victim->abilities.dex = 25;
	  victim->abilities.str = 25;
	  victim->abilities.con = 25;
	  victim->tmpabilities = victim->abilities;
	}
      }
      GET_MANA(victim) = GET_MAX_MANA(victim);
      GET_HIT(victim) = GET_MAX_HIT(victim);
      GET_MOVE(victim) = GET_MAX_MOVE(victim);
      
      update_pos( victim );
      send_to_char("Done.\n\r", ch);
      act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
    }
}




void do_noshout(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  one_argument(arg, buf);
  
  if (!*buf)
    if (IS_SET(ch->specials.act, PLR_NOSHOUT))
      {
	send_to_char("You can now hear shouts again.\n\r", ch);
	REMOVE_BIT(ch->specials.act, PLR_NOSHOUT);
      }
    else
      {
	send_to_char("From now on, you won't hear shouts.\n\r", ch);
	SET_BIT(ch->specials.act, PLR_NOSHOUT);
      }
  else if (!generic_find(arg, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n\r", ch);
  else if (GET_LEVEL(vict) > GET_LEVEL(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else if (IS_SET(vict->specials.act, PLR_NOSHOUT))
    {
      send_to_char("You can shout again.\n\r", vict);
      send_to_char("NOSHOUT removed.\n\r", ch);
      REMOVE_BIT(vict->specials.act, PLR_NOSHOUT);
    }
  else
    {
      send_to_char("The gods take away your ability to shout!\n\r", vict);
      send_to_char("NOSHOUT set.\n\r", ch);
      SET_BIT(vict->specials.act, PLR_NOSHOUT);
    }
}

void do_muzzle(struct char_data *ch, char *arg, int cmd){
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  one_argument(arg, buf);
  
  if (!*buf)
    send_to_char("Try muzzle <char>.\n\r", ch);
  else if (!generic_find(arg, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n\r", ch);
  else if (GET_LEVEL(vict) > GET_LEVEL(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else if (IS_SET(vict->specials.act, PLR_MUZZLE))
    {
      send_to_char("You can speak again.\n\r", vict);
      send_to_char("MUZZLE removed.\n\r", ch);
      REMOVE_BIT(vict->specials.act, PLR_MUZZLE);
    }
  else
    {
      send_to_char("The gods take away your ability to speak!\n\r", vict);
      send_to_char("MUZZLE set.\n\r", ch);
      SET_BIT(vict->specials.act, PLR_MUZZLE);
    }
}

void do_poofin(struct char_data *ch, char *arg,int cmd)
{
  
  if (IS_NPC(ch)) return;
  if (!*arg)
    {
      send_to_char("Usage: poofin <emote_message>\n\r",ch);
      send_to_char("Example: poofin appears in the middle of the room in a puff of smoke.\n\r",ch);
      return;
    }
  if (strlen(arg)>75)
    {
      send_to_char("That poofin message is too long.\n\r",ch);
      return;
    }
  if(ch->player.immortal_enter) /* only free if it exists */
    str_free(ch->player.immortal_enter);
  ch->player.immortal_enter = str_alloc(arg);
  send_to_char("Poofin message changed:\n\r",ch);
  send_to_char(GET_NAME(ch),ch);
  send_to_char(ch->player.immortal_enter,ch);
  send_to_char("\n\r",ch);
}

void do_poofout(struct char_data *ch, char *arg,int cmd)
{
  
  if (IS_NPC(ch)) return;
  if (!*arg)
    {
      send_to_char("Usage: poofout <emote_message>\n\r",ch);
      send_to_char("Example: poofout disappears in a puff of smoke.\n\
r",ch);
      return;
    }
  if (strlen(arg)>75)
    {
      send_to_char("That poofout message is too long.\n\r",ch);
      return;
    }
  if(ch->player.immortal_exit)
    str_free(ch->player.immortal_exit);
  ch->player.immortal_exit = str_alloc(arg);
  send_to_char("Poofout message changed:\n\r",ch);
  send_to_char(GET_NAME(ch),ch);
  send_to_char(ch->player.immortal_exit,ch);
  send_to_char("\n\r",ch);
}

void do_crashsave(struct char_data *ch, char *arg, int cmd){
  struct char_data *i;
  struct obj_cost cost;
  
  cost.no_carried = 0;
  cost.total_cost = 0;
  cost.ok = TRUE;
  
  for (i = character_list; i; i = i->next)
    if ((i) && (!IS_NPC(i))) {
      save_obj(i, &cost);
      save_char(i, NOWHERE);
    }
  if (ch) {
      ansi(LRED,ch); 
      send_to_char("All characters saved (even linkless).\n\r",ch);
      ansi(END,ch);
      send_to_all("[1;31mYou are safely saved by the [1;33mMagic Code[1;31m.[0;37m\n\r");
      log("[1;31mCrash-save activated.[0;37m\n\r");
    }
}




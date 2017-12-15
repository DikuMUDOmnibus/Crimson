/* ************************************************************************
*  file: act.informative.c , Implementation of commands.  Part of DIKUMUD *
*  Usage : Informative commands.                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "limits.h"
#include "interpreter.h"
#include "spells.h"
#include "constants.h"
#include "modify.h"
#include "ansi.h"
#include "weather.h"

#include "act.h"

/* intern functions */
void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
		      bool show);

/* Procedures related to 'look' */
void argument_split_2(char *arg, char *first_arg, char *second_arg) 
{
  int look_at, found, begin;
  found = begin = 0;

  /* Find first non blank */
  for ( ;*(arg + begin ) == ' ' ; begin++);

  /* Find length of first word */
  for (look_at=0; *(arg+begin+look_at) > ' ' ; look_at++)
    /* Make all letters lower case, AND copy them to first_arg */
    *(first_arg + look_at) = LOWER(*(arg + begin + look_at));
  *(first_arg + look_at) = '\0';
  begin += look_at;

  /* Find first non blank */
  for ( ;*(arg + begin ) == ' ' ; begin++);

  /* Find length of second word */
  for ( look_at=0; *(arg+begin+look_at)> ' ' ; look_at++)
    /* Make all letters lower case, AND copy them to second_arg */
    *(second_arg + look_at) = LOWER(*(arg + begin + look_at));
  *(second_arg + look_at)='\0';
  begin += look_at;
}

struct obj_data *get_object_in_equip_vis(struct char_data *ch, char *arg, 
					 struct obj_data *equipment[], int *j)
{
  for ((*j) = 0; (*j) < MAX_WEAR ; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch,equipment[(*j)]))
	if (isname(arg, equipment[(*j)]->name))
	  return(equipment[(*j)]);
  
  return (0);
}

char *find_ex_description(char *word, struct extra_descr_data *list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word,i->keyword))
      return(i->description);

  return(0);
}

void show_obj_to_char(struct obj_data *object, struct char_data *ch, 
		      int mode, int num)
{
  char buffer[MAX_STRING_LENGTH] = "\0";
  bool found;

  if (IS_OBJ_STAT(object,ITEM_NOSEE))
    if ((GET_LEVEL(ch) < IMP_LEV) && 
	(!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord)))
      return; /* hide nosee items from lesser mortals */

  if ((mode == 0) && object->description) {
    strcat(buffer, object->description);
  } else if (object->short_description 
	     && ((mode == 1) || (mode == 2) || (mode==3) || (mode == 4))) 
    strcat(buffer,object->short_description);
  else if (mode == 5) {
    if (object->obj_flags.type_flag == ITEM_NOTE) {
      if (object->action_description) {
	strcat(buffer, "There is something written upon it:\n\r\n\r");
	strcat(buffer, object->action_description);
	page_string(ch->desc, buffer, 1);
      } else
	ansi_act("It's blank.", FALSE, ch,0,0,TO_CHAR,LGRAY);
      return;
    } else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
      strcat(buffer,"You see nothing special..");
    } else {/* ITEM_TYPE == ITEM_DRINKCON */
      strcat(buffer, "It looks like a drink container.");
    }
  }

  if (mode != 3) { 
    found = FALSE;
    if (IS_OBJ_STAT(object,ITEM_INVISIBLE)) {
      strcat(buffer,"[1;30m(invisible)[0;37m");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object,ITEM_EVIL) && IS_AFFECTED(ch,AFF_DETECT_EVIL)) {
      strcat(buffer,"[1;31m..It glows red![0;37m");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object,ITEM_MAGIC) && IS_AFFECTED(ch,AFF_DETECT_MAGIC)) {
      strcat(buffer,"[1;34m..It glows blue![0;37m");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object,ITEM_GLOW)) {
      strcat(buffer,"[1;37m...it glows brightly![0;37m");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object,ITEM_HUM)) {
      strcat(buffer,"...it emits a faint hum!");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object,ITEM_DARK)) {
      strcat(buffer,"[1;30m..clouded in darkness![0;37m");
      found = TRUE;
    }
  }

  if (num > 1)   /* if list mode collect multiples */
    sprintf(buffer+strlen(buffer), " [%d]", num); 
  strcat(buffer, "\n\r");
  ansi(CLR_OBJ, ch);
  page_string(ch->desc, buffer, 1);
  ansi(END, ch);

  /*
    if (((mode == 2) || (mode == 4)) && (GET_ITEM_TYPE(object) == 
    ITEM_CONTAINER)) {
    strcpy(buffer,"The ");
    strcat(buffer,fname(object->name));
    strcat(buffer," contains:\n\r");
    send_to_char(buffer, ch);
    if (mode == 2) list_obj_to_char(object->contains, ch, 1,TRUE);
    if (mode == 4) list_obj_to_char(object->contains, ch, 3,TRUE);
    }
    */
}

void list_obj_to_char(struct obj_data *list,struct char_data *ch, 
		      int mode, bool show) 
{
  struct obj_data *i;
  bool found;
  int num;

  found = FALSE;
  for ( i = list ; i ; i = i->next_content ) { 
    if (CAN_SEE_OBJ(ch,i)) {
      num = 1; /* one found so far, look for more of the same */
      if (GET_ITEM_TYPE(i) != ITEM_ZCMD) {
	while((i->next_content)
	      &&(i->next_content->item_number == i->item_number)
	      &&(GET_ITEM_TYPE(i->next_content) == GET_ITEM_TYPE(i))
	      &&(i->next_content->obj_flags.extra_flags == 
		 i->obj_flags.extra_flags)) {
	  num++;
	  i=i->next_content;
	}
      }
      show_obj_to_char(i, ch, mode, num);
      found = TRUE;
    }    
  }  
  if ((! found) && (show)) 
    send_to_char("Nothing\n\r", ch);
}

void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
  char buffer[MAX_STRING_LENGTH];
  int j, found, percent;
  struct obj_data *tmp_obj;

  if (mode == 0) {
    if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i)) {
      if (IS_AFFECTED(ch, AFF_SENSE_LIFE) && !IS_AFFECTED(i, AFF_WIZINVIS))
	ansi_act("You sense a hidden life form in the room.", 
		 FALSE, ch, 0, 0, TO_CHAR, CLR_MOB);
      return;
    }

    if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos)) {
      /* A player or a mobile without long descr, or not in default pos. */
      if (!IS_NPC(i)) { 
	strcpy(buffer,GET_NAME(i));
	strcat(buffer," ");
	strcat(buffer,GET_TITLE(i));
      } else {
	strcpy(buffer, i->player.short_descr);
	CAP(buffer);
      }

      switch(GET_POS(i)) {
      case POSITION_STUNNED  : 
	strcat(buffer," is lying here, stunned."); break;
      case POSITION_INCAP    : 
	strcat(buffer," is lying here, incapacitated."); break;
      case POSITION_MORTALLYW: 
	strcat(buffer," is lying here, mortally wounded."); break;
      case POSITION_DEAD     : 
	strcat(buffer," is lying here, dead."); break;
      case POSITION_STANDING : 
	strcat(buffer," is standing here."); break;
      case POSITION_SITTING  : 
	strcat(buffer," is sitting here.");  break;
      case POSITION_RESTING  : 
	strcat(buffer," is resting here.");  break;
      case POSITION_SLEEPING : 
	strcat(buffer," is sleeping here."); break;
      case POSITION_FIGHTING :
	if (i->specials.fighting) {
	  strcat(buffer," is here, fighting ");
	  if (i->specials.fighting == ch)
	    strcat(buffer," YOU!");
	  else {
	    if (i->in_room == i->specials.fighting->in_room)
	      if (IS_NPC(i->specials.fighting)) {
		strcat(buffer, i->specials.fighting->player.short_descr);
		strcat(buffer,".");
	      } else {
		strcat(buffer, GET_NAME(i->specials.fighting));
		strcat(buffer, ".");
	      } else
		strcat(buffer, "someone who has already left.");
	  }
	} else /* NIL fighting pointer */
	  strcat(buffer," is here struggling with thin air.");
	break;
	default : strcat(buffer," is floating here."); break;
      }
      ansi(CLR_MOB, ch);
      send_to_char(buffer, ch);
      ansi(END, ch);
      if (IS_AFFECTED(i,AFF_INVISIBLE)) {
	ansi(DGRAY, ch);
	send_to_char(" (invisible)", ch);
	ansi(END, ch);
      }
      if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
	ansi(LRED, ch);
	send_to_char(" (evil)", ch);
	ansi(END, ch);
      }
      send_to_char("\n\r", ch);
    } else { /* npc with long */
      strcpy(buffer, i->player.long_descr);
      buffer[strlen(buffer)-2] = '\0';
      ansi(CLR_MOB, ch);
      send_to_char(buffer, ch);      
      ansi(END, ch);
      if (IS_AFFECTED(i,AFF_INVISIBLE)) {
	ansi(DGRAY, ch);
	send_to_char(" (invisible)", ch);
	ansi(END, ch);
      }
      if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
	ansi(LRED, ch);
	send_to_char(" (evil)", ch);
	ansi(END, ch);
      }
      send_to_char("\n\r", ch);
    }

    if (IS_AFFECTED(i,AFF_SANCTUARY))
      act("[0;32m...$n [1;33mglows with a bright light![0;37m", 
	  FALSE, i, 0, ch, TO_VICT); 

  } else if (mode == 1) {
    if (i->player.description) {
      ansi(CLR_DESC, ch);
      send_to_char(i->player.description, ch);
      ansi(END, ch);
    } else {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }

    /* Show a character to another */

    if (GET_MAX_HIT(i) > 0)
      percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
    else
      percent = -1; /* How could MAX_HIT be < 1?? */

    if (IS_NPC(i))
      strcpy(buffer, i->player.short_descr);
    else
      strcpy(buffer, GET_NAME(i));

    if (percent >= 100)
      strcat(buffer, " is in an excellent condition.");
    else if (percent >= 90)
      strcat(buffer, " has a few scratches.");
    else if (percent >= 75)
      strcat(buffer, " has some small wounds and bruises.");
    else if (percent >= 50)
      strcat(buffer, " has quite a few wounds.");
    else if (percent >= 30)
      strcat(buffer, " has some big nasty wounds and scratches.");
    else if (percent >= 15)
      strcat(buffer, " looks pretty hurt.");
    else if (percent >= 0)
      strcat(buffer, " is in an awful condition.");
    else
      strcat(buffer, " is bleeding awfully from big wounds.");
    
    ansi_act(buffer, FALSE, ch, 0, 0, TO_CHAR, CLR_MOB);
    
    found = FALSE;
    for (j=0; j< MAX_WEAR; j++) {
      if (i->equipment[j]) {
	if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	  found = TRUE;
	}
      }
    }
    if (found) {
      ansi_act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT,CLR_ACTION);
      for (j=0; j< MAX_WEAR; j++) {
	if (i->equipment[j]) {
	  if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	    send_to_char((char*)where[j],ch);
	    show_obj_to_char(i->equipment[j],ch,1,-1);
	  }
	}
      }
    }
    if ((GET_LEVEL(ch)>IMO_LEV) 
	|| ((GET_CLASS(ch) == CLASS_THIEF) && (ch != i))) {
      found = FALSE;
      ansi(CLR_ACTION, ch);
      send_to_char("\n\rYou attempt to peek at the inventory:\n\r", ch);
      ansi(END, ch);
      for(tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
	if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0,20) < GET_LEVEL(ch))) {
	  show_obj_to_char(tmp_obj, ch, 1, -1);
	  found = TRUE;
	}
      }
      if (!found) {
	ansi(CLR_ACTION, ch);
	send_to_char("You can't see anything.\n\r", ch);
	ansi(END, ch);
      }
    }
  } else if (mode == 2) {
    /* Lists inventory */
    ansi_act("$n is carrying:", FALSE, i, 0, ch, TO_VICT,CLR_ACTION);
    list_obj_to_char(i->carrying,ch,1,TRUE);
  }
}

void list_char_to_char(struct char_data *list, struct char_data *ch, int mode)
{
  struct char_data *i;

  for (i = list; i ; i = i->next_in_room) {
    if ( (ch!=i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
		     (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE))) ) {
      ansi(CLR_ACTION, ch);
      show_char_to_char(i,ch,0); 
      ansi(END, ch);
    }
  } 
}

void do_look(struct char_data *ch, char *arg, int cmd)
{
  char buffer[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  int keyword_no;
  int j, bits, temp;
  bool found;
  struct obj_data *tmp_object, *found_object;
  struct char_data *tmp_char;
  char *tmp_desc;
  const char *keywords[]= { 
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "in",
    "at",
    "",  /* Look at '' case */
    "\n" };
  
  if (!ch->desc)
    return;

  if (GET_POS(ch) < POSITION_SLEEPING) {
    ansi(CLR_ERROR, ch);
    send_to_char("You can't see anything but stars!\n\r", ch);
    ansi(END, ch);
  }
  else if (GET_POS(ch) == POSITION_SLEEPING) {
    ansi(CLR_ERROR, ch);
    send_to_char("You can't see anything, you're sleeping!\n\r", ch);
    ansi(END, ch);
  }
  else if (IS_AFFECTED(ch, AFF_BLIND)) {
    ansi(CLR_ERROR, ch);
    send_to_char("You can't see a damn thing, you're blinded!\n\r", ch);
    ansi(END, ch);
  }
  else if (IS_DARK(ch->in_room) && !IS_AFFECTED(ch,AFF_DARKSIGHT) 
	   && GET_LEVEL(ch) < IMO_LEV) 
    send_to_char("It is pitch black...\n\r", ch);
  else {
    argument_split_2(arg,arg1,arg2);
    keyword_no = search_block(arg1, keywords, FALSE); /* Partiel Match */

    if ((keyword_no == -1) && *arg1) {
      keyword_no = 7;
      strcpy(arg2, arg1); /* Let arg2 become the target object (arg1) */
    }

    found = FALSE;
    tmp_object = 0;
    tmp_char  = 0;
    tmp_desc  = 0;

    switch(keyword_no) {
      /* look <dir> */
    case 0 :
    case 1 :
    case 2 : 
    case 3 : 
    case 4 :
    case 5 : {   
      if (EXIT(ch, keyword_no)) {
	if (EXIT(ch, keyword_no)->general_description) {
	  send_to_char(EXIT(ch, keyword_no)->general_description, ch);
	} else {
	  send_to_char("You see nothing special.\n\r", ch);
	}

	if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED) && 
	    (EXIT(ch, keyword_no)->keyword)) {
	  sprintf(buffer, "The %s is closed.\n\r",
		  fname(EXIT(ch, keyword_no)->keyword));
	  send_to_char(buffer, ch);
	}  else {
	  if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_ISDOOR) &&
	      EXIT(ch, keyword_no)->keyword) {
	    sprintf(buffer, "The %s is open.\n\r",
		    fname(EXIT(ch, keyword_no)->keyword));
	    send_to_char(buffer, ch);
	  }
	}
      } else {
	send_to_char("Nothing special there...\n\r", ch);
      }
      switch(keyword_no) {
      case 0:
	ansi_act("$n looks northwards.",TRUE,ch,0,0,TO_ROOM,CLR_ACTION);
	break;
      case 1:
	ansi_act("$n looks eastwards.",TRUE,ch,0,0,TO_ROOM,CLR_ACTION);
	break;
      case 2:
	ansi_act("$n looks southwards.",TRUE,ch,0,0,TO_ROOM,CLR_ACTION);
	break;
      case 3:
	ansi_act("$n looks westwards.",TRUE,ch,0,0,TO_ROOM,CLR_ACTION);
	break;
      case 4:
	ansi_act("$n looks upwards.",TRUE,ch,0,0,TO_ROOM,CLR_ACTION);
	break;
      case 5:
	ansi_act("$n looks downwards.",TRUE,ch,0,0,TO_ROOM,CLR_ACTION);
	break;
      }
    }
      break;

      /* look 'in'   */
    case 6: {
      if (*arg2) {
	/* Item carried */
	
	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
	
	if (bits) { /* Found something */
	  if (GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON)
	    {
	      if (tmp_object->obj_flags.value[1] <= 0) {
		act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
	      } else {
		temp=((tmp_object->obj_flags.value[1]*3)/
		      tmp_object->obj_flags.value[0]);
		sprintf(buffer,"It's %sfull of a %s liquid.\n\r",
                        fullness[temp],color_liquid[tmp_object->
						    obj_flags.value[2]]);
		send_to_char(buffer, ch);
	      }
	    } else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
	      if (!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)) {
		send_to_char(fname(tmp_object->name), ch);
		switch (bits) {
		case FIND_OBJ_INV :
		  send_to_char(" (carried) : \n\r", ch);
		  break;
		case FIND_OBJ_ROOM :
		  send_to_char(" (here) : \n\r", ch);
		  break;
		case FIND_OBJ_EQUIP :
		  send_to_char(" (used) : \n\r", ch);
		  break;
		}
		list_obj_to_char(tmp_object->contains, ch, 2, TRUE);
	      } else
		send_to_char("It is closed.\n\r", ch);
	    } else {
	      send_to_char("That is not a container.\n\r", ch);
	    }
	} else { /* wrong argument */
	  send_to_char("You do not see that item here.\n\r", ch);
	}
      } else { /* no argument */
	ansi(CLR_ERROR, ch);
	send_to_char("Look in what?!\n\r", ch);
	ansi(END, ch);
      }
    }
      break;

      /* look 'at'   */
    case 7 : {
      if (*arg2) {
	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, 
			    &tmp_char, &found_object);

	if (tmp_char) {
	  show_char_to_char(tmp_char, ch, 1);
	  if (ch != tmp_char) {
	    ansi_act("$n looks at you.", 
		     TRUE, ch, 0, tmp_char, TO_VICT,CLR_ACTION);
	    ansi_act("$n looks at $N.", 
		     TRUE, ch, 0, tmp_char, TO_NOTVICT,CLR_ACTION);
	  }
	  return;
	}

	/* Search for Extra Descriptions in room and items */

	/* Extra description in room?? */
	if (!found) {
	  tmp_desc = find_ex_description(arg2, 
					 world[ch->in_room].ex_description);
	  if (tmp_desc) {
	    ansi(CLR_DESC, ch);
	    page_string(ch->desc, tmp_desc, 0);
	    ansi(END, ch);
	    return; /* RETURN SINCE IT WAS A ROOM DESCRIPTION */
	    /* Old system was: found = TRUE; */
	  }
	}
	
	/* Search for extra descriptions in items */
	/* Equipment Used */
	if (!found) {
	  for (j = 0; j< MAX_WEAR && !found; j++) {
	    if (ch->equipment[j]) {
	      if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
		tmp_desc=find_ex_description(arg2, 
					     ch->equipment[j]->ex_description);
		if (tmp_desc) {
		  ansi(CLR_DESC, ch);
		  page_string(ch->desc, tmp_desc, 1);
		  found = TRUE;
		  ansi(END, ch);
		}
	      }
	    }
	  }
	}

	/* In inventory */
	if (!found) {
	  for(tmp_object = ch->carrying; 
	      tmp_object && !found; 
	      tmp_object = tmp_object->next_content) {
	    if CAN_SEE_OBJ(ch, tmp_object) {
	      tmp_desc = find_ex_description(arg2, 
					     tmp_object->ex_description);
	      if (tmp_desc) {
		ansi(CLR_DESC, ch);
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
		ansi(END, ch);
	      }
	    }
	  }
	}
	
	/* Object In room */
	if (!found) {
	  for(tmp_object = world[ch->in_room].contents; 
	      tmp_object && !found; 
	      tmp_object = tmp_object->next_content) {
	    if CAN_SEE_OBJ(ch, tmp_object) {
	      tmp_desc = find_ex_description(arg2, 
					     tmp_object->ex_description);
	      if (tmp_desc) {
		ansi(CLR_DESC, ch);
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
		ansi(END, ch);
	      }
	    }
	  }
	}
	/* wrong argument */
	if (bits) { /* If an object was found */
	  if (!found) 	  /* Show no-description */
	    show_obj_to_char(found_object, ch, 5, -1); 
	  else
	    show_obj_to_char(found_object, ch, 6, -1); /* Find hum, glow etc */
	} else if (!found) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You do not see that here.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	/* no argument */
	ansi(CLR_ERROR, ch);
	send_to_char("Look at what?\n\r", ch);
	ansi(END, ch);
      }
    }
      break;
      
      
      /* look ''     */ 
    case 8 : {
      ansi(CLR_TITLE, ch);
      send_to_char(world[ch->in_room].name, ch);
      ansi(END, ch);
      send_to_char("\n\r", ch);
      
      if (!IS_SET(ch->specials.act, PLR_BRIEF)) {
	ansi(CLR_DESC, ch);
	send_to_char(world[ch->in_room].description, ch);
	ansi(END, ch);
      } else {
	ansi(CLR_DESC, ch);
	do_exits(ch, "\0", 8);
	ansi(END, ch);
      }
      
      list_obj_to_char(world[ch->in_room].contents, ch, 0,FALSE);
      list_char_to_char(world[ch->in_room].people, ch, 0);
    }
      break;

      /* wrong arg   */
    case -1 : {
      ansi(CLR_ERROR, ch);
      send_to_char("Sorry, I didn't understand that!\n\r", ch);
      ansi(END, ch);
    }
      break;
    }
  }
}

/* end of look */

void do_read(struct char_data *ch, char *arg, int cmd)
{
   char buf[100];

   /* This is just for now - To be changed later.! */
   sprintf(buf,"at %s",arg);
   do_look(ch,buf,15);
}


void do_examine(struct char_data *ch, char *arg, int cmd)
{
   char name[100], buf[100];
   int bits;
   struct char_data *tmp_char;
   struct obj_data *tmp_object;

   sprintf(buf,"at %s",arg);
   do_look(ch,buf,15);

   one_argument(arg, name);

   if (!*name)
   {
      ansi(CLR_ERROR, ch);
      send_to_char("Examine what?\n\r", ch);
      ansi(END, ch);
      return;
   }

   bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
          FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

   if (tmp_object) {
      if ((GET_ITEM_TYPE(tmp_object)==ITEM_DRINKCON) ||
          (GET_ITEM_TYPE(tmp_object)==ITEM_CONTAINER)) {
	 ansi(CLR_ACTION, ch);
         send_to_char("When you look inside, you see:\n\r", ch);
	 ansi(END, ch);
         sprintf(buf,"in %s",arg);
         do_look(ch,buf,15);
      }
   }
}


void do_exits(struct char_data *ch, char *arg, int cmd)
{
   int door;
   char buf[MAX_STRING_LENGTH];
   char *exits[] =
   {  
      "North",
      "East ",
      "South",
      "West ",
      "Up   ",
      "Down "
   };

   *buf = '\0';

   for (door = 0; door <= 5; door++)
      if (EXIT(ch, door))
         if (EXIT(ch, door)->to_room != NOWHERE &&
             !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
            if (IS_DARK(EXIT(ch, door)->to_room))
               sprintf(buf + strlen(buf), "%s - Too dark to tell\n\r", exits[door]);
            else
               sprintf(buf + strlen(buf), "%s - %s\n\r", exits[door],
                  world[EXIT(ch, door)->to_room].name);

   send_to_char("Obvious exits:\n\r", ch);

   if (*buf) {
      ansi(CLR_TITLE, ch);
      send_to_char(buf, ch);
      ansi(END, ch);
   } else
      send_to_char("None.\n\r", ch);
}


void do_score(struct char_data *ch, char *arg, int cmd)
{
   struct time_info_data playing_time;
   static char buf[100];

   struct time_info_data real_time_passed(time_t t2, time_t t1);

   sprintf(buf,"You are: %s %s (level %d).\n\r",
      GET_NAME(ch),
      GET_TITLE(ch), GET_LEVEL(ch) );
   ansi(CLR_TITLE, ch);
   send_to_char(buf,ch);
   ansi(END, ch);

   if (IS_NPC(ch)) {
     ansi(CLR_ERROR, ch);
     send_to_char("And a Mob!\n\r", ch);
     ansi(END, ch);
   }
   sprintf(buf, "You are %d years old.", GET_AGE(ch));
   playing_time = real_time_passed((time(0)-ch->player.time.logon) +
      ch->player.time.played, 0);
   sprintf(buf,"You have been playing for %d days and %d hours. You are %d years old\n\r",
      playing_time.day,
      playing_time.hours,
      GET_AGE(ch));    
   send_to_char(buf, ch);     

   if ((age(ch).month == 0) && (age(ch).day == 0)) {
      ansi(YELLOW, ch);
      send_to_char("It's your birthday today.\n\r",ch);
      ansi(END, ch);
    }

   if (GET_COND(ch,DRUNK)>10) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are intoxicated.\n\r", ch);
      ansi(END, ch);
    }

   sprintf(buf, 
      "You have %d(%d) hit points, %d(%d) mana %d(%d) movement points,\n\r",
      GET_HIT(ch),GET_MAX_HIT(ch),
      GET_MANA(ch),GET_MAX_MANA(ch),
      GET_MOVE(ch),GET_MAX_MOVE(ch));
   send_to_char(buf,ch);

   sprintf(buf, "THAC0 %d, and AC %d.%d. You're carrying %d/%d pounds.\n\r",
      thaco[GET_CLASS(ch)-1][GET_LEVEL(ch)], GET_AC(ch)/10, MAXV(GET_AC(ch)%10*-1,GET_AC(ch)%10), IS_CARRYING_W(ch), CAN_CARRY_W(ch));
   send_to_char(buf,ch);
   
   if (GET_LEVEL(ch)>10)
        {
                sprintf(buf,"Str:[%d] Int:[%d] Wis:[%d] Dex:[%d] Con:[%d]",
                                GET_STR(ch), 
                                GET_INT(ch),
                                GET_WIS(ch),
                                GET_DEX(ch),
                                GET_CON(ch) );
                if (GET_ADD(ch)>0) {
                  send_to_char(buf,ch);
		  sprintf(buf," Fighter Dam. bonus:[+%d]", GET_ADD(ch)/25);
		}
                strcat(buf,"\n\r");
                send_to_char(buf,ch);
        }

   if (GET_LEVEL(ch) < IMO_LEV)
   	sprintf(buf,"You have scored %d/%d (%d to go) exp,\n\r and have %d gold coins. (and %d more in the bank)\n\r",
   	   GET_EXP(ch), 
   	   titles[GET_CLASS(ch)-1][GET_LEVEL(ch)+1].exp, 
   	   titles[GET_CLASS(ch)-1][GET_LEVEL(ch)+1].exp - GET_EXP(ch), 
   	   GET_GOLD(ch),
           ch->points.bank);
   else
   	sprintf(buf,"You have scored %d exp, and have %d gold coins.\n\r",
   	   GET_EXP(ch), 
   	   GET_GOLD(ch));
   send_to_char(buf,ch);
   
   if (GET_ALIGNMENT(ch) > 800) {
      ansi(LCYAN, ch);
      send_to_char("You're so good you've developed a halo.\n\r",ch);
    } else if (GET_ALIGNMENT(ch) > 600) {
      ansi(CYAN, ch);
      send_to_char("You're a virtual saint.\n\r",ch);
    } else if (GET_ALIGNMENT(ch) > 300) {
      ansi(GREEN, ch);
      send_to_char("You're a fairly good person.\n\r",ch);
    } else if (GET_ALIGNMENT(ch) > -300) {
      ansi(WHITE, ch);
      send_to_char("You're as neutral as they come.\n\r",ch);
    } else if (GET_ALIGNMENT(ch) > -600) {
      ansi(BROWN, ch);
      send_to_char("You're mean and spitefull.\n\r",ch);
    } else if (GET_ALIGNMENT(ch) > -800) {
      ansi(RED, ch);
      send_to_char("You're a malevolent fiend.\n\r",ch);
    } else {
      ansi(LRED, ch);
      send_to_char("You're so evil you make demons look like Kai Lords.\n\r",ch);
    }
   ansi(END, ch);
   
   switch(GET_POS(ch)) {
      case POSITION_DEAD : 
         send_to_char("You are DEAD!\n\r", ch); break;
      case POSITION_MORTALLYW :
         send_to_char("You are mortally wounded!, you should seek help!\n\r", ch); break;
      case POSITION_INCAP : 
         send_to_char("You are incapacitated, slowly fading away\n\r", ch); break;
      case POSITION_STUNNED : 
         send_to_char("You are stunned! You can't move\n\r", ch); break;
      case POSITION_SLEEPING : 
         send_to_char("You are sleeping.\n\r",ch); break;
      case POSITION_RESTING  : 
         send_to_char("You are resting.\n\r",ch); break;
      case POSITION_SITTING  : 
         send_to_char("You are sitting.\n\r",ch); break;
      case POSITION_FIGHTING :
         if (ch->specials.fighting)
            ansi_act("You are fighting $N.\n\r", FALSE, ch, 0,
                 ch->specials.fighting, TO_CHAR,CLR_FIGHT);
         else
            send_to_char("You are fighting thin air.\n\r", ch);
         break;
      case POSITION_STANDING : 
         send_to_char("You are standing.\n\r",ch); break;
      default :
         send_to_char("You are floating.\n\r",ch); break;
   }

   ansi(YELLOW, ch);
   send_to_char("You have the following bits set:\n\r",ch);
   ansi(END, ch);
   sprintbit(ch->specials.act, player_bits, buf);
   send_to_char(buf, ch);
   send_to_char("\n\r", ch);
}


void do_time(struct char_data *ch, char *arg, int cmd)
{
  char buf[100], *suf;
  int weekday, day;
  int hours, minutes;
  long elapsed;
  extern long up_time;
  extern struct time_info_data time_info;

  sprintf(buf, "It is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am") );
  
  weekday = ((35*time_info.month)+time_info.day+1) % 7;/* 35 days in a month */
  
  strcat(buf,weekdays[weekday]);
  strcat(buf,".\n\r");
  send_to_char(buf,ch);

  day = time_info.day + 1;   /* day in [1..35] */

  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";
  
  sprintf(buf, "The %d%s Day of the %s, Year %d.\n\r",
	  day,
	  suf,
	  month_name[time_info.month],
	  time_info.year);
  send_to_char(buf,ch);
  
  elapsed = (time(0) - up_time);
  hours = elapsed / 3600;
  minutes = elapsed % 3600 / 60;
  
  sprintf(buf, "\n\rUptime is: %d hours %d minutes.\n\r", hours, minutes);
  ansi(WHITE, ch);
  send_to_char(buf, ch);
  ansi(END, ch);
}


void do_weather(struct char_data *ch, char *arg, int cmd)
{
   char buf[128]; /* messages are short here, only small buf needed */
   static char *sky_look[4]= {
   "cloudless",
   "cloudy",
   "rainy",
   "lit by flashes of lightning"};

   if (OUTSIDE(ch)) {
      sprintf(buf, 
      "The sky is %s and %s.\n\r",
         sky_look[weather_info.sky],
         (weather_info.change >=0 ? "you feel a warm wind from the south" :
        "your foot tells you bad weather is due"));
      send_to_char(buf,ch);
   } else
      send_to_char("You have no feeling about the weather at all.\n\r", ch);
}


void do_help(struct char_data *ch, char *arg, int cmd)
{
   extern int top_of_helpt;
   extern struct help_index_element *help_index;
   extern FILE *help_fl;

   int chk, bot, top, mid, minlen;
   char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];

   if (!ch->desc)
      return;

   for(;isspace(*arg); arg++);

   if (*arg)
   {
      if (!help_index)
      {
	 ansi(CLR_ERROR, ch);
         send_to_char("No help available.\n\r", ch);
	 ansi(END, ch);
         return;
      }
      bot = 0;
      top = top_of_helpt;

      for (;;)
      {
         mid = (bot + top) / 2;
         minlen = strlen(arg);

         if (!(chk = strn_cmp(arg, help_index[mid].keyword, minlen)))
         {
            fseek(help_fl, help_index[mid].pos, 0);
            *buffer = '\0';
            for (;;)
            {
               fgets(buf, 80, help_fl);
               if (*buf == '#')
                  break;
               strcat(buffer, buf);
               strcat(buffer, "\r");
            }
            page_string(ch->desc, buffer, 1);
            return;
         }
         else if (bot >= top)
         {
	    ansi(CLR_ERROR, ch);
            send_to_char("There is no help on that word.\n\r", ch);
	    ansi(END, ch);
            return;
         }
         else if (chk > 0)
            bot = ++mid;
         else
            top = --mid;
      }
      return;
   }
   send_to_char(help, ch);
}


void do_wizhelp(struct char_data *ch, char *arg, int cmd){
   char buf[MAX_STRING_LENGTH];
   int no, i, lord = 1;
   extern char *command[];  /* The list of commands (interpreter.c)  */
                            /* First command is command[0]           */
   extern struct command_info cmd_info[];
                            /* cmd_info[1] ~~ commando[0]            */

   if (IS_NPC(ch))
      return;
   if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord)))
      lord = 0;

   ansi(CLR_TITLE, ch);
   send_to_char("The following privileged commands are available:\n\r", ch);
   ansi(END, ch);
   *buf = '\0';

   for (no = 1, i = 0; *command[i] != '\n'; i++)
      if (((ubyte)GET_LEVEL(ch) >= cmd_info[i+1].minimum_level) &&
         (cmd_info[i+1].minimum_level >= IMO_LEV))
      {
         sprintf(buf + strlen(buf), "%-10s", command[i]);
         if (!(no % 7))
            strcat(buf, "\n\r");
         no++;
      }
   strcat(buf, "\n\r");
   page_string(ch->desc, buf, 1);

   ansi(LCYAN, ch);
   send_to_char("\n\rThe following online editing comands are available:\n\r", ch);
   ansi(END, ch);
   *buf = '\0';
   for (no = 1, i = 0; *command[i] != '\n'; i++)
      if ((cmd_info[i+1].minimum_level<=0) &&
         (lord))
      {
         sprintf(buf + strlen(buf), "%-10s", command[i]);
         if (!(no % 7))
            strcat(buf, "\n\r");
         no++;
      }
   strcat(buf, "\n\r");
   page_string(ch->desc, buf, 1);
}


void do_commands(struct char_data *ch, char *arg, int cmd)
{
   char buf[MAX_STRING_LENGTH];
   int no, i;
   extern char *command[];  /* The list of commands (interpreter.c)  */
                            /* First command is command[0]           */
   extern struct command_info cmd_info[];
                            /* cmd_info[1] ~~ commando[0]            */

   if (IS_NPC(ch))
      return;

   ansi(CLR_TITLE, ch);
   send_to_char("The following commands are available:\n\r\n\r", ch);
   ansi(END, ch);
   *buf = '\0';
   for (no = 1, i = 0; *command[i] != '\n'; i++)
      if ( ((ubyte)GET_LEVEL(ch) >= cmd_info[i+1].minimum_level )  &&
	  (cmd_info[i+1].minimum_level > 0))
      {
         sprintf(buf + strlen(buf), "%-10s", command[i]);
         if (!(no % 7))
            strcat(buf, "\n\r");
         no++;
      }
   strcat(buf, "\n\r");
   page_string(ch->desc, buf, 1);
}


void do_who(struct char_data *ch, char *arg, int cmd)
{
   struct descriptor_data *d;
   struct char_data *c;
   char buf[256];

   ansi(LRED, ch);
   send_to_char("Players Currently Online\n\r", ch);
   ansi(RED, ch);
   send_to_char("------------------------\n\r", ch);
   ansi(END, ch);
   for (d = descriptor_list; d; d = d->next)
   {
     if ((d->connected==CON_PLYNG || d->connected==CON_LDEAD) 
	 && ((CAN_SEE(ch,d->character) && (GET_LEVEL(d->character) > IMO_LEV)) 
	     || (GET_LEVEL(d->character) < IMO_LEV2))) { 
       if(d->original) /* If switched */         
	 c=d->original;
       else
	 c=d->character;

         switch (GET_LEVEL(c)) {
         case IMP_LEV: {
            sprintf(buf, "[-Over-]");
	    ansi(LCYAN, ch);
	  }
            break;
         case IMO_LEV4: {
	    ansi(LGREEN, ch);
            sprintf(buf, "[-High-");
	  }	    
            break;
         case IMO_LEV3: {
	    ansi(YELLOW, ch);
            sprintf(buf, "[-Ante-");
	  }
            break;
         case IMO_LEV2: {
	    ansi(LBLUE, ch);
            sprintf(buf, "[-Meth-");
	  }
            break;
         case IMO_LEV: {
            ansi(PURPLE, ch);
            sprintf(buf, "[-Avat-");
	  }
            break;
         default: {
	    ansi(YELLOW, ch);
            if (GET_LEVEL(ch)>=30) {
               sprintf(buf, "[%d--", GET_LEVEL(c));
               strncpy(buf+3, pc_class_types[c->player.class],4);
            } else if (GET_LEVEL(ch)>=25) {
               if (GET_LEVEL(c)>9)
                  sprintf(buf, "[%d?-", GET_LEVEL(c)/10);
               else
                  sprintf(buf, "[?--");
               strncpy(buf+3, pc_class_types[c->player.class],4);
            } else {
               if (GET_LEVEL(ch)>=20) {
                  if (GET_LEVEL(c)>9)
                     sprintf(buf, "[%d?----", GET_LEVEL(c)/10);
                  else
                     sprintf(buf, "[?-----");
                  sprintf(buf, "[%d-----", GET_LEVEL(c));
               } else 
                     sprintf(buf, "[------");
               if (GET_LEVEL(c)<3) {
		  ansi(WHITE, ch);
                  sprintf(buf, "[Newbie");
		}
            }
	  }
            break;
         }
         sprintf(buf+7, "] %s %s", GET_NAME(c), c->player.title);
         send_to_char(buf, ch);
	 if (IS_SET(c->specials.act, PLR_PKILLER)) {
	   ansi(LRED, ch);
	   send_to_char(" <PKILLER>", ch);
	 } else if (IS_SET(c->specials.act, PLR_ENFORCER)) {
	   ansi(LBLUE, ch);
	   send_to_char(" <ENFORCER>", ch);
	 }
	 if (IS_AFFECTED(c, AFF_GROUP)) {
	   ansi(LGREEN, ch);
	   send_to_char(" <GROUPING>", ch); 
	 }
	 if (IS_SET(c->specials.affected_by, AFF_INVISIBLE)) {
	   ansi(DGRAY, ch);
	   send_to_char(" <INVIS>", ch);
	 }
	 if (IS_SET(c->specials.affected_by, AFF_WIZINVIS)) {
	   ansi(DGRAY, ch);
	   send_to_char(" <WIZINVIS>", ch);
	 }
	 if (d->connected == CON_LDEAD)  {
	   ansi(BROWN, ch);
	   send_to_char(" <LINKLESS>", ch);
	 }
       ansi(END, ch);
       send_to_char("\n\r", ch);
      }
   }
}

void do_spells(struct char_data *ch, char *arg, int cmd) {
   char buf[512];
   struct affected_type *aff;

/* Routine to show what spells a char is affected by */

   ansi(CLR_ACTION, ch);
   send_to_char("Affecting Spells:\n\r--------------\n\r", ch);
   if (ch->affected) {
      for(aff = ch->affected; aff; aff = aff->next) {
         sprintf(buf, "Spell : '%s' Time left:[%3d]\n\r",
            spells[aff->type-1],
            aff->duration);
	 ansi(CYAN, ch);
         send_to_char(buf, ch);
	 ansi(END, ch);
      }
   } else {
      ansi(CLR_ERROR, ch);
      send_to_char("NONE!\n\r", ch);
      ansi(END, ch);
    }
}

void do_users(struct char_data *ch, char *arg, int cmd)
{
   char buf[MAX_STRING_LENGTH], line[200];

   struct descriptor_data *d;

   strcpy(buf, "Connections:\n\r------------\n\r");
   
   for (d = descriptor_list; d; d = d->next)
   {
      if (d->character && d->character->player.name)
      {
      if(d->original)
         sprintf(line, "%-16s: ", d->original->player.name);
      else
         sprintf(line, "%-16s: ", d->character->player.name);
      }
      else
         strcpy(line, "UNDEFINED       : ");
      if (d->host)
         sprintf(line + strlen(line), "[%s]\n\r", d->host);
      else
         strcat(line, "[Hostname unknown]\n\r");

      strcat(buf, line);
   }
   ansi(CLR_ACTION, ch);
   send_to_char(buf, ch);
   ansi(END, ch);
}


void do_inventory(struct char_data *ch, char *arg, int cmd) {

   ansi(CLR_ACTION, ch);
   send_to_char("You are carrying:\n\r", ch);
   ansi(END, ch);
   list_obj_to_char(ch->carrying, ch, 1, TRUE);
}


void do_equipment(struct char_data *ch, char *arg, int cmd) {
int j;
bool found;

   ansi(CLR_ACTION, ch);
   send_to_char("You are using:\n\r", ch);
   ansi(END, ch);
   found = FALSE;
   for (j=0; j< MAX_WEAR; j++) {
      if (ch->equipment[j]) {
         if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
            send_to_char((char*)where[j],ch);
            show_obj_to_char(ch->equipment[j],ch,1,-1);
            found = TRUE;
         } else {
            send_to_char((char*)where[j],ch);
            send_to_char("Something.\n\r",ch);
            found = TRUE;
         }
      }
   }
   if(!found) {
      ansi(CLR_ERROR, ch);
      send_to_char(" Nothing.\n\r", ch);
      ansi(END, ch);
   }
}


void do_credits(struct char_data *ch, char *arg, int cmd) {

   page_string(ch->desc, credits, 0);
}


void do_news(struct char_data *ch, char *arg, int cmd) {

   page_string(ch->desc, news, 0);
}


void do_info(struct char_data *ch, char *arg, int cmd) {

   page_string(ch->desc, info, 0);
}


void do_wizlist(struct char_data *ch, char *arg, int cmd) {

   page_string(ch->desc, wizlist, 0);
}



void do_where(struct char_data *ch, char *arg, int cmd)
{
   char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], buf2[256];
   register struct char_data *i;
   register struct obj_data *k;
   struct descriptor_data *d;

   one_argument(arg, name);

   if (!*name) {
      if (GET_LEVEL(ch) < 42)
      {
         strcpy(buf, "Players:\n\r--------\n\r");
      
         for (d = descriptor_list; d; d = d->next) {
            if (d->character && (d->connected == CON_PLYNG) && (d->character->in_room != NOWHERE) && CAN_SEE(ch, d->character) && (world[ch->in_room].zone == world[d->character->in_room].zone) && !(d->original)) {
               sprintf(buf, "%-20s - %s\n\r",
                 d->character->player.name,
                 world[d->character->in_room].name);
               send_to_char(buf, ch);
            }
         }
         return;
      }
      else
      {
         strcpy(buf, "Players:\n\r--------\n\r");
      
         for (d = descriptor_list; d; d = d->next) {
            if (d->character && (d->connected == CON_PLYNG) && (d->character->in_room != NOWHERE)) {
               if (d->original)   /* If switched */
                  sprintf(buf, "%-20s - %s [%d] In body of %s\n\r",
                    d->original->player.name,
                    world[d->character->in_room].name,
                    world[d->character->in_room].number,
                    fname(d->character->player.name));
               else
                  sprintf(buf, "%-20s - %s [%d]\n\r",
                    d->character->player.name,
                    world[d->character->in_room].name,
                    world[d->character->in_room].number);
                   
               send_to_char(buf, ch);
            }
         }
         return;
      }
   }

   *buf = '\0';

   for (i = character_list; i; i = i->next)
      if (isname(name, i->player.name) && CAN_SEE(ch, i) )
      {
         if ((i->in_room != NOWHERE) && ((GET_LEVEL(ch)>IMO_LEV) ||
             (world[i->in_room].zone == world[ch->in_room].zone))) {

            if (IS_NPC(i))
               sprintf(buf, "%-30s- %s ", i->player.short_descr,
                  world[i->in_room].name);
            else
               sprintf(buf, "%-30s- %s ", i->player.name,
                  world[i->in_room].name);

            if (GET_LEVEL(ch) >= IMO_LEV2)
               sprintf(buf2,"[%d]\n\r", world[i->in_room].number);
            else
               strcpy(buf2, "\n\r");

            strcat(buf, buf2);
            send_to_char(buf, ch);

            if (GET_LEVEL(ch) < IMO_LEV)
               break;
         }
      }

   if (GET_LEVEL(ch) > IMO_LEV) {
      for (k = object_list; k; k = k->next)
         if (isname(name, k->name) && CAN_SEE_OBJ(ch, k) && 
            (k->in_room != NOWHERE)) {
               sprintf(buf, "%-30s- %s [%d]\n\r",
                  k->short_description,
                  world[k->in_room].name,
                  world[k->in_room].number);
                  send_to_char(buf, ch);
            }
   }

   if (!*buf)
      send_to_char("Couldn't find any such thing.\n\r", ch);
}

void do_levels(struct char_data *ch, char *arg, int cmd)
{
   int i;
   char buf[MAX_STRING_LENGTH];

   if (IS_NPC(ch))
   {
      send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
      return;
   }

   *buf = '\0';
   
   for (i = 1; i < IMO_LEV; i++)
   {
      sprintf(buf + strlen(buf), "%7d-%-7d : ",
         titles[GET_CLASS(ch) - 1][i].exp,
         titles[GET_CLASS(ch) - 1][i + 1].exp);
      switch(GET_SEX(ch))
      {
         case SEX_MALE:
            strcat(buf, titles[GET_CLASS(ch) - 1][i].title_m); break;
         case SEX_FEMALE:
            strcat(buf, titles[GET_CLASS(ch) - 1][i].title_f); break;
         default:
            send_to_char("Oh dear.\n\r", ch); break;
      }
      strcat(buf, "\n\r");
   }
   send_to_char(buf, ch);
}


void do_consider(struct char_data *ch, char *arg, int cmd)
{
   struct char_data *victim;
   char name[MAX_INPUT_LENGTH];
   int diff;

   one_argument(arg, name);

   if (!(victim = get_char_room_vis(ch, name))) {
      send_to_char("Consider killing who?\n\r", ch);
      return;
   }

   if (victim == ch) {
      send_to_char("Easy! Very easy indeed!\n\r", ch);
      return;
   }

   if (!IS_NPC(victim)) {
      send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
      return;
   }

   diff = (GET_LEVEL(victim)-GET_LEVEL(ch));

   if (diff <= -15)
      send_to_char("Now where did that chicken go?\n\r", ch);
   else if (diff <= -10)
      send_to_char("Say the word and its history...\n\r", ch);
   else if (diff <= -5)
      send_to_char("You could do it with a needle!\n\r", ch);
   else if (diff <= -2)
      send_to_char("Easy.\n\r", ch);
   else if (diff <= -1)
      send_to_char("Fairly easy.\n\r", ch);
   else if (diff == 0)
      send_to_char("The perfect match!\n\r", ch);
   else if (diff <= 1)
      send_to_char("You would need some luck!\n\r", ch);
   else if (diff <= 2)
      send_to_char("You would need a lot of luck!\n\r", ch);
   else if (diff <= 3)
      send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
   else if (diff <= 5)
      send_to_char("Do you feel lucky, punk?\n\r", ch);
   else if (diff <= 10)
      send_to_char("Bring some friends!?\n\r", ch);
   else if (diff <= 15)
      send_to_char("Bring *LOTS* of friends!?\n\r", ch);
   else if (diff <= 20)
      send_to_char("Are you mad!?\n\r", ch);
   else if (diff <= 100)
      send_to_char("You ARE mad!\n\r", ch);

}

void do_ansi(struct char_data *ch, char *arg, int cmd)
{
	char buf[MAX_INPUT_LENGTH];

	one_argument(arg, buf);

	if(IS_SET(ch->specials.act, PLR_ANSI))
	{
		send_to_char("[1;31mA[1;32mN[1;33mS[1;36mI[0;37m disabled.\n\r",ch);
		REMOVE_BIT(ch->specials.act, PLR_ANSI);
	}
	else
	{
		send_to_char("[1;31mA[1;32mN[1;33mS[1;36mI[0;37m graphics enabled.\n\r",ch);
		SET_BIT(ch->specials.act, PLR_ANSI);
	}
}

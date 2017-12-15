/* ************************************************************************
*  file: act.obj1.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands mainly moving around objects.                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


/* extern functions */
extern void do_simple_split(struct char_data *ch, long total);
struct obj_data *create_money( int amount );


/* procedures related to get */
void get(struct char_data *ch, struct obj_data *obj_object, 
	struct obj_data *sub_object) {
char buffer[MAX_STRING_LENGTH];

	if (sub_object) {
		obj_from_obj(obj_object);
		obj_to_char(obj_object, ch);
		if (sub_object->carried_by == ch) {
		  ansi_act("You get $p from $P.", 0, ch, obj_object, sub_object, TO_CHAR, CLR_ACTION);
		  ansi_act("$n gets $p from $s $P.", 1, ch, obj_object, sub_object, TO_ROOM, CLR_ACTION);
		} else {
		  ansi_act("You get $p from $P.", 0, ch, obj_object, sub_object, TO_CHAR, CLR_ACTION);
		  ansi_act("$n gets $p from $P.", 1, ch, obj_object, sub_object, TO_ROOM, CLR_ACTION);
		}
	} else {
		obj_from_room(obj_object);
		obj_to_char(obj_object, ch);
		ansi_act("You get $p.", 0, ch, obj_object, 0, TO_CHAR, CLR_ACTION);
		ansi_act("$n gets $p.", 1, ch, obj_object, 0, TO_ROOM, CLR_ACTION);
	}
	if((obj_object->obj_flags.type_flag == ITEM_MONEY) && 
		(obj_object->obj_flags.value[0]>=1))
	{
		obj_from_char(obj_object);
		sprintf(buffer,"There was %d coins.\n\r", obj_object->obj_flags.value[0]);
		send_to_char(buffer,ch);
		GET_GOLD(ch) += obj_object->obj_flags.value[0];

		/* check for autosplit */
		if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_GROUP) && IS_SET(ch->specials.act, PLR_AUTOSPLIT))
			do_simple_split(ch, obj_object->obj_flags.value[0]);
		extract_obj(obj_object);
	}
}


void do_get(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *sub_object;
  struct obj_data *obj_object;
  struct obj_data *next_obj;
  bool found = FALSE;
  bool fail  = FALSE;
  int type   = 3;

  argument_interpreter(argument, arg1, arg2);

  /* get type */
  if (!*arg1) {
    type = 0;
  }
  if (*arg1 && !*arg2) {
    if (!str_cmp(arg1,"all")) {
      type = 1;
    } else {
      type = 2;
    }
  }
  if (*arg1 && *arg2) {
    if (!str_cmp(arg1,"all")) {
      if (!str_cmp(arg2,"all")) {
	type = 3;
      } else {
	type = 4;
      }
    } else {
      if (!str_cmp(arg2,"all")) {
	type = 5;
      } else {
	type = 6;
      }
    }
  }

  switch (type) {
    /* get */
  case 0:{ 
    ansi(CLR_ERROR, ch);
    send_to_char("Get what?\n\r", ch); 
    ansi(END, ch);
  } break;
    /* get all */
  case 1: {
    if (ch->in_room == real_room(3419)) {
      /* Can't get all in donation room */
      ansi_act("Hey! Don't be greedy. Please get just what you want.",
	       FALSE, ch, 0, 0, TO_CHAR, CLR_ERROR);
      return;
    }
    sub_object = 0;
    found = FALSE;
    fail = FALSE;
    for(obj_object = world[ch->in_room].contents;
	obj_object;
	obj_object = next_obj) {
      next_obj = obj_object->next_content;
      if (CAN_SEE_OBJ(ch,obj_object)) {
	if ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)) {
	  if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <= 
	      CAN_CARRY_W(ch)) {
	    if (CAN_WEAR(obj_object,ITEM_TAKE)) {
	      get(ch,obj_object,sub_object);
	      found = TRUE;
	    } else {
	      ansi(CLR_ERROR, ch);
	      send_to_char("You can't take that\n\r", ch);
	      ansi(END, ch);
	      fail = TRUE;
	    }
	  } else {
	    sprintf(buffer,"%s : You can't carry that much weight.\n\r", fname(obj_object->name));
	    ansi(CLR_ERROR, ch);
	    send_to_char(buffer, ch);
	    ansi(END, ch);
	    fail = TRUE;
	  }
	} else {
	  sprintf(buffer,"%s : You can't carry that many items.\n\r", fname(obj_object->name));
	  ansi(CLR_ERROR, ch);
	  send_to_char(buffer, ch);
	  ansi(END, ch);
	  fail = TRUE;
	}
      }
    }
    if (found) {
      ansi(CLR_ACTION, ch);
      send_to_char("OK.\n\r", ch);
      ansi(END, ch);
    } else {
      if (!fail) {
	ansi(CLR_ERROR, ch);
	send_to_char("You see nothing here.\n\r",ch);
	ansi(END, ch);
      }
    }
  } break;
    /* get ??? */
  case 2:{
    sub_object = 0;
    found = FALSE;
    fail	= FALSE;
    obj_object = get_obj_in_list_vis(ch, arg1, 
				     world[ch->in_room].contents);
    if (obj_object) {
      if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < 
	    CAN_CARRY_W(ch)) {
	  if (CAN_WEAR(obj_object,ITEM_TAKE)) {
	    get(ch,obj_object,sub_object);
	    found = TRUE;
	  } else {
	    ansi(CLR_ERROR, ch);
	    send_to_char("You can't take that\n\r", ch);
	    ansi(END, ch);
	    fail = TRUE;
	  }
	} else {
	  sprintf(buffer,"%s : You can't carry that much weight.\n\r", fname(obj_object->name));
	  ansi(CLR_ERROR, ch);
	  send_to_char(buffer, ch);
	  ansi(END, ch);
	  fail = TRUE;
	}
      } else {
	sprintf(buffer,"%s : You can't carry that many items.\n\r", fname(obj_object->name));
	ansi(CLR_ERROR, ch);
	send_to_char(buffer, ch);
	ansi(END, ch);
	fail = TRUE;
      }
    } else {
      sprintf(buffer,"You do not see a %s here.\n\r", arg1);
      ansi(CLR_ERROR, ch);
      send_to_char(buffer, ch);
      ansi(END, ch);
      fail = TRUE;
    }
  } break;
    /* get all all */
  case 3:{ 
    ansi(CLR_ERROR, ch);
    send_to_char("You must be joking?!\n\r", ch);
    ansi(END, ch);
  } break;
    /* get all ??? */
  case 4:{
    found = FALSE;
    fail  = FALSE; 
    sub_object = get_obj_in_list_vis(ch, arg2, 
				     world[ch->in_room].contents);
    if (!sub_object){
      sub_object = get_obj_in_list_vis(ch, arg2, ch->carrying);
    }
    if (sub_object) {
      if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
	if (IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("Don't you think you have to open it first?\n\r", ch);
	  ansi(END, ch);
          return;
	} 
	for(obj_object = sub_object->contains;
	    obj_object;
	    obj_object = next_obj) {
	  next_obj = obj_object->next_content;
	  if (CAN_SEE_OBJ(ch,obj_object)) {
	    if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	      if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < 
		  CAN_CARRY_W(ch)) {
		if (CAN_WEAR(obj_object,ITEM_TAKE)) {
		  get(ch,obj_object,sub_object);
		  found = TRUE;
		} else {
		  ansi(CLR_ERROR, ch);
		  send_to_char("You can't take that\n\r", ch);
		  ansi(END, ch);
		  fail = TRUE;
		}
	      } else {
		sprintf(buffer,"%s : You can't carry that much weight.\n\r", fname(obj_object->name));
		ansi(CLR_ERROR, ch);
		send_to_char(buffer, ch);
		ansi(END, ch);
		fail = TRUE;
	      }
	    } else {
	      sprintf(buffer,"%s : You can't carry that many items.\n\r", fname(obj_object->name));
	      send_to_char(buffer, ch);
	      fail = TRUE;
	    }
	  }
	}
	if (!found && !fail) {
	  sprintf(buffer,"You do not see anything in the %s.\n\r", fname(sub_object->name));
	  ansi(CLR_ERROR, ch);
	  send_to_char(buffer, ch);
	  ansi(END, ch);
	  fail = TRUE;
	}
      } else {
	sprintf(buffer,"The %s is not a container.\n\r", fname(sub_object->name));
	ansi(CLR_ERROR, ch);
	send_to_char(buffer, ch);
	ansi(END, ch);
	fail = TRUE;
      }
    } else { 
      sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
      ansi(CLR_ERROR, ch);
      send_to_char(buffer, ch);
      ansi(END, ch);
      fail = TRUE;
    }
  } break;
  case 5:{ 
    ansi(CLR_ERROR, ch);
    send_to_char("You can't take a thing from more than one container.\n\r", ch);
    ansi(END, ch);
  } break;
  case 6:{
    found = FALSE;
    fail	= FALSE;
    sub_object = get_obj_in_list_vis(ch, arg2, 
				     world[ch->in_room].contents);
    if (!sub_object){
      sub_object = get_obj_in_list_vis(ch, arg2, ch->carrying);
    }
    if (sub_object) {
      if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
	if (IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("Don't you think you have to open it first?\n\r", ch);
	  ansi(END, ch);
          return;
	} 
	obj_object = get_obj_in_list_vis(ch, arg1, sub_object->contains);
	if (obj_object) {
	  if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	    if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < 
		CAN_CARRY_W(ch)) {
	      if (CAN_WEAR(obj_object,ITEM_TAKE)) {
		get(ch,obj_object,sub_object);
		found = TRUE;
	      } else {
		ansi(CLR_ERROR, ch);
		send_to_char("You can't take that\n\r",ch);
		ansi(END, ch);
		fail = TRUE;
	      }
	    } else {
	      sprintf(buffer,"%s : You can't carry that much weight.\n\r", fname(obj_object->name));
	      ansi(CLR_ERROR, ch);
	      send_to_char(buffer, ch);
	      ansi(END, ch);
	      fail = TRUE;
	    }
	  } else {
	    sprintf(buffer,"%s : You can't carry that many items.\n\r", fname(obj_object->name));
	    ansi(CLR_ERROR, ch);
	    send_to_char(buffer, ch);
	    ansi(END, ch);
	    fail = TRUE;
	  }
	} else {
	  sprintf(buffer,"The %s does not contain the %s.\n\r", fname(sub_object->name), arg1);
	  ansi(CLR_ERROR, ch);
	  send_to_char(buffer, ch);
	  ansi(END, ch);
	  fail = TRUE;
	}
      } else {
	sprintf(buffer,"The %s is not a container.\n\r", fname(sub_object->name));
	ansi(CLR_ERROR, ch);
	send_to_char(buffer, ch);
	ansi(END, ch);
	fail = TRUE;
      }
    } else {
      sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
      ansi(CLR_ERROR, ch);
      send_to_char(buffer, ch);
      ansi(END, ch);
      fail = TRUE;
    }
  } break;
  }
}


void do_drop(struct char_data *ch, char *argument, int cmd) 
{
  char arg[MAX_STRING_LENGTH];
  int amount;
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *tmp_object;
  struct obj_data *next_obj;
  bool test = FALSE;

  argument = one_argument(argument, arg);
  if (is_number(arg)) {
    amount = atoi(arg);
    argument = one_argument(argument,arg);
    if (str_cmp("coins",arg) && str_cmp("coin",arg)) {
      ansi(CLR_ERROR, ch);
      send_to_char("You need to specify how much though.\n\r",ch);
      ansi(END, ch);
      return;
    }
    if (amount < 0) {
      ansi(CLR_ERROR, ch);
      send_to_char("Sorry, you can't do that!\n\r",ch);
      ansi(END, ch);
      return;
    }
    if(GET_GOLD(ch) < amount) {
      ansi(CLR_ERROR, ch);
      send_to_char("You haven't got that many coins!\n\r",ch);
      ansi(END, ch);
      return;
    }
    ansi(CLR_ACTION, ch);
    send_to_char("OK.\n\r",ch);
    ansi(END, ch);
    if (amount == 0)
      return;
		
    ansi_act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
    tmp_object = create_money(amount);
    obj_to_room(tmp_object,ch->in_room);
    GET_GOLD(ch)-=amount;
    return;
  }

  if (*arg) {
    if (!str_cmp(arg,"all")) {
      for(tmp_object = ch->carrying;
	  tmp_object;
	  tmp_object = next_obj) {
	next_obj = tmp_object->next_content;
	if (! IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
	  if (CAN_SEE_OBJ(ch, tmp_object)) {
	    sprintf(buffer, "You drop %s.\n\r", OBJS(tmp_object,ch));
	    ansi(CLR_ACTION, ch);
	    send_to_char(buffer, ch);
	    ansi(END, ch);
	  } else {
	    ansi(CLR_ACTION, ch);
	    send_to_char("You drop something.\n\r", ch);
	    ansi(END, ch);
	  }
	  ansi_act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM,CLR_ACTION);
	  obj_from_char(tmp_object);
	  obj_to_room(tmp_object,ch->in_room);
	  test = TRUE;
	} else {
	  if (CAN_SEE_OBJ(ch, tmp_object)) {
	    sprintf(buffer, "You can't drop the %s, it must be CURSED!\n\r", fname(tmp_object->name));
	    ansi(CLR_ERROR, ch);
	    send_to_char(buffer, ch);
	    ansi(END, ch);
	    test = TRUE;
	  }
	}
      }
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You do not seem to have anything.\n\r", ch);
	ansi(END, ch);
      }
    } else {
      tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
      if (tmp_object) {
	if (! IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
	  sprintf(buffer, "You drop the %s.\n\r", fname(tmp_object->name));
	  ansi(CLR_ACTION, ch);
	  send_to_char(buffer, ch);
	  ansi(END, ch);
	  ansi_act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM,CLR_ACTION);
	  obj_from_char(tmp_object);
	  obj_to_room(tmp_object,ch->in_room);
	} else {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You can't drop it, it must be CURSED!\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	ansi(CLR_ERROR, ch);
	send_to_char("You do not have that item.\n\r", ch);
	ansi(END, ch);
      }
    }
  } else {
    ansi(CLR_ERROR, ch);
    send_to_char("Drop what?\n\r", ch);
    ansi(END, ch);
  }
}

void do_put(struct char_data *ch, char *argument, int cmd)
{
	char buffer[MAX_STRING_LENGTH];
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	struct obj_data *obj_object;
	struct obj_data *sub_object;
	struct char_data *tmp_char;
	int bits;

	argument_interpreter(argument, arg1, arg2);
	if (*arg1) {
	  if (*arg2) {
	    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
	    if (obj_object) {
	      bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM,
				  ch, &tmp_char, &sub_object);
	      if (sub_object) {
		if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
		  if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
		    if (obj_object == sub_object) {
		      ansi(CLR_ERROR, ch);
		      send_to_char("You attempt to fold it into itself, but fail.\n\r", ch);
		      ansi(END, ch);
		      return;
		    }
		    if (((sub_object->obj_flags.weight) + 
			 (obj_object->obj_flags.weight)) <
			(sub_object->obj_flags.value[0])) {
		      ansi(CLR_ACTION, ch);
		      send_to_char("Ok.\n\r", ch);
		      ansi(END, ch);
		      if (bits==FIND_OBJ_INV) {
			obj_from_char(obj_object);
			/* make up for above line */
			IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(obj_object);
			/* obj_from_char(sub_object); do not rearrange order */
			obj_to_obj(obj_object, sub_object);
			/* obj_to_char(sub_object,ch);do not rearrange order */
		      } else {
			obj_from_char(obj_object);
			/* Do we need obj_from_room???(sub_object,....); */
			obj_to_obj(obj_object, sub_object);
			/* Dow we need obj_to_room???(sub_object,ch);    */
		      }
		      
		      ansi_act("$n puts $p in $P.",TRUE, ch, obj_object, sub_object, TO_ROOM,CLR_ACTION);
		    } else {
		      ansi(CLR_ERROR, ch);
		      send_to_char("It won't fit.\n\r", ch);
		      ansi(END, ch);
		    }
		  } else {
		    ansi(CLR_ERROR, ch);
		    send_to_char("It seems to be closed.\n\r", ch);
		    ansi(END, ch);
		  }
		} else {
		  sprintf(buffer,"The %s is not a container.\n\r", fname(sub_object->name));
		  ansi(CLR_ERROR, ch);
		  send_to_char(buffer, ch);
		  ansi(END, ch);
		}
	      } else {
		sprintf(buffer, "You don't have the %s.\n\r", arg2);
		ansi(CLR_ERROR, ch);
		send_to_char(buffer, ch);
		ansi(END, ch);
	      }
	    } else {
	      sprintf(buffer, "You don't have the %s.\n\r", arg1);
	      ansi(CLR_ERROR, ch);
	      send_to_char(buffer, ch);
	      ansi(END, ch);
	    }
	  } else {
	    sprintf(buffer, "Put %s in what?\n\r", arg1);
	    ansi(CLR_ERROR, ch);
	    send_to_char(buffer, ch);
	    ansi(END, ch);
	  }
	} else {
	  ansi(CLR_ERROR, ch);
	  send_to_char("Put what in what?\n\r",ch);
	  ansi(END, ch);
	}
      }


void do_give(struct char_data *ch, char *argument, int cmd)
{
	char obj_name[MAX_INPUT_LENGTH], vict_name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int amount;
	struct char_data *vict;
	struct obj_data *obj;

	argument=one_argument(argument,obj_name);
	if(is_number(obj_name))
	{
		amount = atoi(obj_name);
		argument=one_argument(argument, arg);
		if (str_cmp("coins",arg) && str_cmp("coin",arg))
		{
		  ansi(CLR_ERROR, ch);
		  send_to_char("Sorry, you can't do that (yet)...\n\r",ch);
		  ansi(END, ch);
		  return;
		}
		if(amount<0)
		{
		  ansi(CLR_ERROR, ch);
		  send_to_char("Sorry, you can't do that!\n\r",ch);
		  ansi(END, ch);
		  return;
		}
		if((GET_GOLD(ch)<amount) && (IS_NPC(ch) || (GET_LEVEL(ch) < IMO_LEV3)))
		{
		  ansi(CLR_ERROR, ch);
		  send_to_char("You haven't got that many coins!\n\r",ch);
		  ansi(END, ch);
		  return;
		}
		argument=one_argument(argument, vict_name);
		if(!*vict_name)
		{
		  ansi(CLR_ERROR, ch);
		  send_to_char("To who?\n\r",ch);
		  ansi(END, ch);
		  return;
		}
		if (!(vict = get_char_room_vis(ch, vict_name)))
		{
			send_to_char("To who?\n\r",ch);
			return;
		}
		ansi(CLR_ACTION, ch);
		send_to_char("Ok.\n\r",ch);
		ansi(END, ch);
		sprintf(buf,"%s gives you %d gold coins.\n\r",PERS(ch,vict),amount);
		ansi(CLR_ACTION, ch);
		send_to_char(buf,vict);
		ansi(END, ch);
		ansi_act("$n gives some gold to $N.", 1, ch, 0, vict, TO_NOTVICT,CLR_ACTION);
		if (IS_NPC(ch) || (GET_LEVEL(ch) < IMO_LEV2))
			GET_GOLD(ch)-=amount;
		GET_GOLD(vict)+=amount;
		return;
	}

	argument=one_argument(argument, vict_name);

	if (!*obj_name || !*vict_name)
	{
	  ansi(CLR_ERROR, ch);
	  send_to_char("Give what to who?\n\r", ch);
	  ansi(END, ch);
	  return;
	}
	if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
	{
	  ansi(CLR_ERROR, ch);
	  send_to_char("You do not seem to have anything like that.\n\r", ch);
	  ansi(END, ch);
	  return;
	}
	if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP))
	{
	  ansi(CLR_ERROR, ch);
	  send_to_char("You can't let go of it! Yeech!!\n\r", ch);
	  ansi(END, ch);
	  return;
	}
	if (!(vict = get_char_room_vis(ch, vict_name)))
	{
	  ansi(CLR_ERROR, ch);
	  send_to_char("No one by that name around here.\n\r", ch);
	  ansi(END, ch);
	  return;
	}

	if ((1+IS_CARRYING_N(vict)) > CAN_CARRY_N(vict))
	{
	  ansi_act("$N seems to have $S hands full.", 0, ch, 0, vict, TO_CHAR,CLR_ERROR);
	  return;
	}
	if (obj->obj_flags.weight + IS_CARRYING_W(vict) > CAN_CARRY_W(vict))
	{
	  ansi_act("$E can't carry that much weight.", 0, ch, 0, vict, TO_CHAR,CLR_ERROR);
	  return;
	}
	obj_from_char(obj);
	obj_to_char(obj, vict);
	ansi_act("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT,CLR_ACTION);
	ansi_act("$n gives you $p.", 0, ch, obj, vict, TO_VICT,CLR_ACTION);
	ansi(CLR_ACTION, ch);
	send_to_char("Ok.\n\r", ch);
	ansi(END, ch);
}



void do_donate(struct char_data *ch, char *argument, int cmd)
{
  char obj_name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct obj_data *obj;

  argument=one_argument(argument, obj_name);

  if (!*obj_name) {
    ansi(CLR_ERROR, ch);
    send_to_char("What do you wish to donate?\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((IS_SET(zone_table[world[ch->in_room].zone].flags, ZONE_NOENTER) 
       || IS_SET(zone_table[world[ch->in_room].zone].flags, ZONE_TESTING)) 
      && (GET_LEVEL(ch) < IMP_LEV)) {
    ansi(CLR_ERROR, ch);
    send_to_char("A strange power surge interferes!\n\r",ch);
    ansi(END, ch);
    return;
  }

  if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You do not seem to have anything like that.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
    ansi(CLR_ERROR, ch);
    send_to_char("You can't let go of it! Yeech!!\n\r", ch);
    ansi(END, ch);
    return;
  }

  obj_from_char(obj);  /* Delete object from donor */
  ansi_act("$n donates $p to the Donate Room.", 0, ch, obj, 0, TO_ROOM,CLR_ACTION);
  ansi(CLR_ACTION, ch);
  send_to_char("Object sent to the Donation Room.  Newbies will thank you.\n\r", ch);
  ansi(END, ch);
  
  obj_to_room(obj, real_room(3419));  
  sprintf(buf,"The ceiling cracks open and something falls to the ground.\n\r");
  send_to_room(buf, real_room(3419));
}












/* ************************************************************************
*  file: act.obj2.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands mainly using objects.                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "limits.h"
#include "interpreter.h"
#include "spells.h"
#include "constants.h"
#include "ansi.h"
#include "act.h"


void weight_change_object(struct obj_data *obj, int weight)
{
	struct obj_data *tmp_obj;
	struct char_data *tmp_ch;

	if (obj->in_room != NOWHERE) {
		GET_OBJ_WEIGHT(obj) += weight;
	} else if ( (tmp_ch = obj->carried_by) ) {
		obj_from_char(obj);
		GET_OBJ_WEIGHT(obj) += weight;
		obj_to_char(obj, tmp_ch);
	} else if ( (tmp_obj = obj->in_obj) ) {
		obj_from_obj(obj);
		GET_OBJ_WEIGHT(obj) += weight;
		obj_to_obj(obj, tmp_obj);
	} else {
		log("Unknown attempt to subtract weight from an object.");
	}
}



void name_from_drinkcon(struct obj_data *obj)
{
	int i;
        char *new_name;

	for(i=0; (*((obj->name)+i)!=' ') && (*((obj->name)+i)!='\0'); i++)  ;

	if (*((obj->name)+i)==' ') {
		new_name=strdup((obj->name)+i+1);
		free(obj->name);
		obj->name=new_name;
	}
}



void name_to_drinkcon(struct obj_data *obj,int type)
{
	char *new_name;

	CREATE(new_name,char,strlen(obj->name)+strlen(drinknames[type])+2);
	sprintf(new_name,"%s %s",drinknames[type],obj->name);
	free(obj->name);
	obj->name=new_name;
}



void do_drink(struct char_data *ch, char *argument, int cmd)
{
	char buf[100];
	struct obj_data *temp;
	struct affected_type af;
	int amount;


	one_argument(argument,buf);

	if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying)))
	{
	  if(!(temp = get_obj_in_list_vis(ch,buf,world[ch->in_room].contents))) {
	    ansi_act("You can't find it!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
	    return;
	  }
	}

	if (temp->obj_flags.type_flag!=ITEM_DRINKCON)
	{
	  ansi_act("You can't drink from that!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
	  return;
	}

	if((GET_COND(ch,DRUNK)>10)&&(GET_COND(ch,THIRST)>0)) 
        /* The pig is drunk */
	{
	  ansi_act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
	  ansi_act("$n tried to drink but missed $s mouth!", TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
	  return;
	}

	if((GET_COND(ch,FULL)>20)&&(GET_COND(ch,THIRST)>0)) /* Stomach full */
	{
	  ansi_act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
	  return;
	}

  if (temp->obj_flags.type_flag==ITEM_DRINKCON){
		if(temp->obj_flags.value[1]>0)  /* Not empty */
		{	
		  sprintf(buf,"$n drinks %s from $p.",drinks[temp->obj_flags.value[2]]);
		  ansi_act(buf, TRUE, ch, temp, 0, TO_ROOM,CLR_ACTION);
		  sprintf(buf,"You drink the %s.\n\r",drinks[temp->obj_flags.value[2]]);
                  ansi(CLR_ACTION, ch);
		  send_to_char(buf,ch);
		  ansi(END, ch);

		  if (drink_aff[temp->obj_flags.value[2]][DRUNK] > 0 )
		    amount = (25-GET_COND(ch,THIRST))/drink_aff[temp->obj_flags.value[2]][DRUNK];
		  else
		    amount = number(3,10);
		  
		  amount = MINV(amount,temp->obj_flags.value[1]);

		  weight_change_object(temp, -amount);  /* Subtract amount */

		  gain_condition(ch,DRUNK,(int)((int)drink_aff
			[temp->obj_flags.value[2]][DRUNK]*amount)/4);

		  gain_condition(ch,FULL,(int)((int)drink_aff
		       [temp->obj_flags.value[2]][FULL]*amount)/4);

		  gain_condition(ch,THIRST,(int)((int)drink_aff
			 [temp->obj_flags.value[2]][THIRST]*amount)/4);

		  if(GET_COND(ch,DRUNK)>10)
		    ansi_act("You feel drunk.",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);

		  if(GET_COND(ch,THIRST)>20)
		    ansi_act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR,CLR_ACTION);

		  if(GET_COND(ch,FULL)>20)
		    ansi_act("You are full.",FALSE,ch,0,0,TO_CHAR,CLR_ACTION);

		  if(temp->obj_flags.value[3]) /* The shit was poisoned ! */
		    {
		      ansi_act("Ooops, it tasted rather strange ?!!?",
			       FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
		      ansi_act("$n chokes and utters some strange sounds.",
			       TRUE,ch,0,0,TO_ROOM,CLR_DAM);
		      af.type = SPELL_POISON;
		      af.duration = amount*3;
		      af.modifier = 0;
		      af.location = APPLY_NONE;
		      af.bitvector = AFF_POISON;
		      affect_join(ch,&af, FALSE, FALSE);
		    }

		  /* empty the container, and no longer poison. */
		  temp->obj_flags.value[1] -= amount;
		  if(!temp->obj_flags.value[1]) {  /* The last bit */
		    temp->obj_flags.value[2]=0;
		    temp->obj_flags.value[3]=0;
		    name_from_drinkcon(temp);
		  }
		  return;
		  
		}
	      }
	
	ansi_act("It's empty already.",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
	
	return;
      }



void do_eat(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp;
  struct affected_type af;
  
  one_argument(argument,buf);
  
  if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) 
    {
      ansi_act("You can't find it!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  if((temp->obj_flags.type_flag != ITEM_FOOD) && (GET_LEVEL(ch) < 42))
    {
      ansi_act("Your stomach refuses to eat that!?!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  if(GET_COND(ch,FULL)>20) /* Stomach full */
    {	
      ansi_act("You are too full to eat more!",FALSE,ch,0,0,TO_CHAR,CLR_ACTION);
      return;
    }

  ansi_act("$n eats $p.",TRUE,ch,temp,0,TO_ROOM,CLR_ACTION);
  ansi_act("You eat the $o.",FALSE,ch,temp,0,TO_CHAR,CLR_ACTION);

  gain_condition(ch,FULL,temp->obj_flags.value[0]);

  if(GET_COND(ch,FULL)>20)
    ansi_act("You are full.",FALSE,ch,0,0,TO_CHAR,CLR_ACTION);

  if(temp->obj_flags.value[3] && (GET_LEVEL(ch) < 21)) 
   /* The shit was poisoned ! */
    {
      ansi_act("Ooops, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      ansi_act("$n coughs and utters some strange sounds.",FALSE,ch,0,0,TO_ROOM,CLR_DAM);

      af.type = SPELL_POISON;
      af.duration = temp->obj_flags.value[0]*2;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_join(ch,&af, FALSE, FALSE);
    }

  extract_obj(temp);
}


void do_pour(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct obj_data *from_obj;
  struct obj_data *to_obj;
  int amount;
  
  argument_interpreter(argument, arg1, arg2);
  
  if(!*arg1) /* No arguments */
    {
      ansi_act("What do you want to pour from?",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }
  
  if(!(from_obj = get_obj_in_list_vis(ch,arg1,ch->carrying)))
    {
      if(!(from_obj = get_obj_in_list_vis(ch,arg1,world[ch->in_room].contents))) {
	ansi_act("You can't find it!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
	return;
      }	
    }

  if(from_obj->obj_flags.type_flag!=ITEM_DRINKCON)
    {
      ansi_act("You can't pour from that!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  if(from_obj->obj_flags.value[1]==0)
    {
      ansi_act("The $p is empty.",FALSE, ch,from_obj, 0,TO_CHAR,CLR_ERROR);
      return;
    }

  if(!*arg2)
    {
      ansi_act("Where do you want it? Out or in what?",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  if(!str_cmp(arg2,"out"))
    {
      ansi_act("$n empties $p.", TRUE, ch,from_obj,0,TO_ROOM,CLR_ACTION);
      ansi_act("You empty $p.", FALSE, ch,from_obj,0,TO_CHAR,CLR_ACTION);

      weight_change_object(from_obj, -from_obj->obj_flags.value[1]); /* Empty */

      from_obj->obj_flags.value[1]=0;
      from_obj->obj_flags.value[2]=0;
      from_obj->obj_flags.value[3]=0;
      name_from_drinkcon(from_obj);
      
      return;
    }

  if(!(to_obj = get_obj_in_list_vis(ch,arg2,ch->carrying)))
    {
      if(!(to_obj = get_obj_in_list_vis(ch,arg2,world[ch->in_room].contents))) {
	ansi_act("You can't find it!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
	return;
      }
    }

  if(to_obj->obj_flags.type_flag!=ITEM_DRINKCON)
    {
      ansi_act("You can't pour anything into that.",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  if((to_obj->obj_flags.value[1]!=0)&&
     (to_obj->obj_flags.value[2]!=from_obj->obj_flags.value[2]))
    {
      ansi_act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  if(!(to_obj->obj_flags.value[1]<to_obj->obj_flags.value[0]))
    {
      ansi_act("There is no room for more.",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  sprintf(buf,"You pour %s into %s.\n\r",
	  drinks[from_obj->obj_flags.value[2]],arg2);
  ansi(CLR_ACTION, ch);
  send_to_char(buf,ch);
  ansi(END, ch);
  ansi_act("$n fills from $p.", TRUE, ch, from_obj, 0, TO_ROOM,CLR_ACTION);

  /* New alias */
  if (to_obj->obj_flags.value[1]==0) 
    name_to_drinkcon(to_obj,from_obj->obj_flags.value[2]);

  /* First same type liq. */
  to_obj->obj_flags.value[2]=from_obj->obj_flags.value[2];

  /* Then how much to pour */
  from_obj->obj_flags.value[1] -= (amount=(to_obj->obj_flags.value[0]-to_obj->obj_flags.value[1]));

  to_obj->obj_flags.value[1]=to_obj->obj_flags.value[0];

  if(from_obj->obj_flags.value[1]<0)    /* There was to little */
    {
      to_obj->obj_flags.value[1]+=from_obj->obj_flags.value[1];
      amount += from_obj->obj_flags.value[1];
      from_obj->obj_flags.value[1]=0;
      from_obj->obj_flags.value[2]=0;
      from_obj->obj_flags.value[3]=0;
      name_from_drinkcon(from_obj);
    }

  /* Then the poison boogie */
  to_obj->obj_flags.value[3]=
    (to_obj->obj_flags.value[3]||from_obj->obj_flags.value[3]);

  /* And the weight boogie */

  weight_change_object(from_obj, -amount);
  weight_change_object(to_obj, amount);   /* Add weight */

  return;
}

void do_sip(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct obj_data *temp;

  one_argument(argument,arg);

  if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
      ansi_act("You can't find it!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  if(temp->obj_flags.type_flag!=ITEM_DRINKCON)
    {
      ansi_act("You can't sip from that!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  if(GET_COND(ch,DRUNK)>10) /* The pig is drunk ! */
    {
      ansi_act("You simply fail to reach your mouth!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      ansi_act("$n tries to sip, but fails!",TRUE,ch,0,0,TO_ROOM,CLR_ERROR);
      return;
    }

  if(!temp->obj_flags.value[1])  /* Empty */
    {
      ansi_act("But there is nothing in it?",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  ansi_act("$n sips from the $o.",TRUE,ch,temp,0,TO_ROOM,CLR_ACTION);
  sprintf(buf,"It tastes like %s.\n\r",drinks[temp->obj_flags.value[2]]);
  ansi(CLR_ACTION, ch);
  send_to_char(buf,ch);
  ansi(END, ch);

  gain_condition(ch,DRUNK,(int)(drink_aff[temp->obj_flags.value[2]][DRUNK]/4));

  gain_condition(ch,FULL,(int)(drink_aff[temp->obj_flags.value[2]][FULL]/4));

  gain_condition(ch,THIRST,(int)(drink_aff[temp->obj_flags.value[2]][THIRST]/4));

  weight_change_object(temp, -1);  /* Subtract one unit */

  if(GET_COND(ch,DRUNK)>10)
    ansi_act("You feel drunk.",FALSE,ch,0,0,TO_CHAR,CLR_DAM);

  if(GET_COND(ch,THIRST)>20)
    ansi_act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR,CLR_ACTION);

  if(GET_COND(ch,FULL)>20)
    ansi_act("You are full.",FALSE,ch,0,0,TO_CHAR,CLR_ACTION);

  if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON)) 
    /* The shit was poisoned ! */
    {
      ansi_act("But it also had a strange taste!",FALSE,ch,0,0,TO_CHAR,CLR_DAM);

      af.type = SPELL_POISON;
      af.duration = 3;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_to_char(ch,&af);
    }
  
  temp->obj_flags.value[1]--;
  
  if(!temp->obj_flags.value[1])  /* The last bit */
    {
      temp->obj_flags.value[2]=0;
      temp->obj_flags.value[3]=0;
      name_from_drinkcon(temp);
    }
  
  return;
}


void do_taste(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  char arg[MAX_STRING_LENGTH];
  struct obj_data *temp;

  one_argument(argument,arg);

  if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
      ansi_act("You can't find it!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  if(temp->obj_flags.type_flag==ITEM_DRINKCON)
    {
      do_sip(ch,argument,0);
      return;
    }

  if(!(temp->obj_flags.type_flag==ITEM_FOOD))
    {
      ansi_act("Taste that?!? Your stomach refuses!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      return;
    }

  ansi_act("$n tastes the $o.", FALSE, ch, temp, 0, TO_ROOM,CLR_ACTION);
  ansi_act("You taste the $o.", FALSE, ch, temp, 0, TO_CHAR,CLR_ACTION);

  gain_condition(ch,FULL,1);
  
  if(GET_COND(ch,FULL)>20)
    ansi_act("You are full.",FALSE,ch,0,0,TO_CHAR,CLR_ACTION);

  if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON)) 
    /* The shit was poisoned ! */
    {
      ansi_act("Ooops, it did not taste good at all!",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);

      af.type = SPELL_POISON;
      af.duration = 2;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_to_char(ch,&af);
    }

  temp->obj_flags.value[0]--;

  if(!temp->obj_flags.value[0])	/* Nothing left */
    {
      ansi_act("There is nothing left now.",FALSE,ch,0,0,TO_CHAR,CLR_ERROR);
      extract_obj(temp);
    }
  
  return;
}



/* functions related to wear */

void perform_wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
  switch(keyword) {
  case 0 :
    ansi_act("$n light $p and holds it.", FALSE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You light $p and hold it.", FALSE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    if(obj_object->obj_flags.value[2] == 0)
      ansi_act("Be warned that your $p is used up.", FALSE, ch, obj_object,0,TO_CHAR,CLR_ERROR);
    break;
  case 1 : 
    ansi_act("$n wears $p on $s finger.", TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wear $p on your finger.", TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 2 : 
    ansi_act("$n wears $p around $s neck.", TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wear $p around your neck.", TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 3 : 
    ansi_act("$n wears $p on $s body.", TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wear $p on your body.", TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 4 : 
    ansi_act("$n wears $p on $s head.",TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wear $p on your head.",TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 5 : 
    ansi_act("$n wears $p on $s legs.",TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wear $p on your legs.",TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 6 : 
    ansi_act("$n wears $p on $s feet.",TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wear $p on your feet.",TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 7 : 
    ansi_act("$n wears $p on $s hands.",TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wear $p on your hands.",TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 8 : 
    ansi_act("$n wears $p on $s arms.",TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wear $p on your arms.",TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 9 : 
    ansi_act("$n wears $p about $s body.",TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wear $p about your body.",TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 10 : 
    ansi_act("$n wears $p about $s waist.",TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wear $p about your waist.",TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 11 : 
    ansi_act("$n wears $p around $s wrist.",TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wear $p around your wrist.",TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 12 : 
    ansi_act("$n wields $p as a weapon.",TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You wield $p as a weapon.",TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 13 : 
    ansi_act("$n grabs $p.",TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You grab $p.",TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  case 14 : 
    ansi_act("$n starts using $p as a shield.", TRUE, ch, obj_object,0,TO_ROOM,CLR_ACTION);
    ansi_act("You start using $p as a shield.", TRUE, ch, obj_object,0,TO_CHAR,CLR_ACTION);
    break;
  }
}



void wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
  char buffer[MAX_STRING_LENGTH];
  bool test = FALSE;
  
  if (keyword > 900) { /* flag to mask output ie for wear all */
    keyword = keyword - 1000;
    test = TRUE;
  }
  switch(keyword) {
  case 0: {  /* LIGHT SOURCE */
    if (ch->equipment[WEAR_LIGHT]) {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You are already holding a light source.\n\r", ch);
	ansi(END, ch);
      }
    } else {
      perform_wear(ch,obj_object,keyword);
      obj_from_char(obj_object);
      equip_char(ch,obj_object, WEAR_LIGHT);
      if (obj_object->obj_flags.value[2])
	world[ch->in_room].light++;
    }
  } break;
    
  case 1: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) {
      if ((ch->equipment[WEAR_FINGER_L]) && (ch->equipment[WEAR_FINGER_R])) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You are already wearing something on your fingers.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	if (ch->equipment[WEAR_FINGER_L]) {
	  obj_from_char(obj_object);
	  perform_wear(ch,obj_object,keyword);
	  equip_char(ch, obj_object, WEAR_FINGER_R);
	} else {
	  obj_from_char(obj_object);
	  perform_wear(ch,obj_object,keyword);
	  equip_char(ch, obj_object, WEAR_FINGER_L);
	}
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't wear that on your finger.\n\r", ch);
	ansi(END, ch);
      }	
    }
  } break;
  case 2: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) {
      if ((ch->equipment[WEAR_NECK_1]) && (ch->equipment[WEAR_NECK_2])) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You can't wear any more around your neck.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	perform_wear(ch,obj_object,keyword);
	if (ch->equipment[WEAR_NECK_1]) {
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WEAR_NECK_2);
	} else {
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WEAR_NECK_1);
	}
      }
    } else {
      if (!test) {
	send_to_char("You can't wear that around your neck.\n\r", ch);
	ansi(END, ch);
      }
    }
  } break;
  case 3: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) {
      if (ch->equipment[WEAR_BODY]) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You already wear something on your body.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	perform_wear(ch,obj_object,keyword);
	obj_from_char(obj_object);
	equip_char(ch,  obj_object, WEAR_BODY);
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't wear that on your body.\n\r", ch);
	ansi(END, ch);
      }
    }
  } break;
  case 4: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) {
      if (ch->equipment[WEAR_HEAD]) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You already wear something on your head.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	perform_wear(ch,obj_object,keyword);
	obj_from_char(obj_object);
	equip_char(ch, obj_object, WEAR_HEAD);
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't wear that on your head.\n\r", ch);
	ansi(END, ch);
      }
    }
  } break;
  case 5: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) {
      if (ch->equipment[WEAR_LEGS]) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You already wear something on your legs.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	perform_wear(ch,obj_object,keyword);
	obj_from_char(obj_object);
	equip_char(ch, obj_object, WEAR_LEGS);
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't wear that on your legs.\n\r", ch);
	ansi(END, ch);
      }
    }
  } break;
  case 6: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) {
      if (ch->equipment[WEAR_FEET]) {
	if (!test) { 
	  ansi(CLR_ERROR, ch);
	  send_to_char("You already wear something on your feet.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	perform_wear(ch,obj_object,keyword);
	obj_from_char(obj_object);
	equip_char(ch, obj_object, WEAR_FEET);
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't wear that on your feet.\n\r", ch);
	ansi(END, ch);
}
    }
  } break;
  case 7: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) {
      if (ch->equipment[WEAR_HANDS]) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You already wear something on your hands.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	perform_wear(ch,obj_object,keyword);
	obj_from_char(obj_object);
	equip_char(ch, obj_object, WEAR_HANDS);
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't wear that on your hands.\n\r", ch);
	ansi(END, ch);
      }
    }
  } break;
  case 8: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) {
      if (ch->equipment[WEAR_ARMS]) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You already wear something on your arms.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	perform_wear(ch,obj_object,keyword);
	obj_from_char(obj_object);
	equip_char(ch, obj_object, WEAR_ARMS);
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't wear that on your arms.\n\r", ch);
	ansi(END, ch);
      }
    }
  } break;
  case 9: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) {
      if (ch->equipment[WEAR_ABOUT]) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You already wear something about your body.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	perform_wear(ch,obj_object,keyword);
	obj_from_char(obj_object);
	equip_char(ch, obj_object, WEAR_ABOUT);
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't wear that about your body.\n\r", ch);
	ansi(END, ch);
      }
    }
  } break;
  case 10: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) {
      if (ch->equipment[WEAR_WAISTE]) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You already wear something about your waist.\n\r",ch);
	  ansi(END, ch);
	}
      } else {
	perform_wear(ch,obj_object,keyword);
	obj_from_char(obj_object);
	equip_char(ch,  obj_object, WEAR_WAISTE);
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't wear that about your waist.\n\r", ch);
	ansi(END, ch);
      }
    }
  } break;
  case 11: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) {
      if ((ch->equipment[WEAR_WRIST_L]) && (ch->equipment[WEAR_WRIST_R])) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You already wear something around both your wrists.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	obj_from_char(obj_object);
	if (ch->equipment[WEAR_WRIST_L]) {
	  equip_char(ch,  obj_object, WEAR_WRIST_R);
	  perform_wear(ch,obj_object,keyword);
	} else {
	  equip_char(ch, obj_object, WEAR_WRIST_L);
	  perform_wear(ch,obj_object,keyword);
	}
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't wear that around your wrist.\n\r", ch);
	ansi(END, ch);
      }
    }
  } break;
  case 12:
    if (CAN_WEAR(obj_object,ITEM_WIELD)) {
      if (ch->equipment[WIELD]) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You are already wielding something.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	/* Cleric exception has been removed, and is temporarily placed */
	/* at the end of this file                                      */
	if (GET_OBJ_WEIGHT(obj_object) >
	    str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("It is too heavy for you to use.\n\r",ch);
	  ansi(END, ch);
	} else {
	  perform_wear(ch,obj_object,keyword);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WIELD);
	}
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't wield that.\n\r", ch);
	ansi(END, ch);
      }
    }
    break;
  case 13:
    if (CAN_WEAR(obj_object,ITEM_HOLD)) {
      if (ch->equipment[HOLD]) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You are already holding something.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	/* Cleric exception has been removed, and is temporarily placed */
	/* at the end of this file                                      */
	perform_wear(ch,obj_object,keyword);
	obj_from_char(obj_object);
	equip_char(ch, obj_object, HOLD);
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't hold this.\n\r", ch);
	ansi(END, ch);
      }
    }
    break;
  case 14: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) {
      if ((ch->equipment[WEAR_SHIELD])) {
	if (!test) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("You are already using a shield.\n\r", ch);
	  ansi(END, ch);
	}
      } else {
	perform_wear(ch,obj_object,keyword);
	obj_from_char(obj_object);
	equip_char(ch, obj_object, WEAR_SHIELD);
      }
    } else {
      if (!test) {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't use that as a shield.\n\r", ch);
	ansi(END, ch);
      }
    }
  } break;
  case -1: {
    sprintf(buffer,"Wear %s where?\n\r",fname(obj_object->name));
    ansi(CLR_ERROR, ch);
    send_to_char(buffer, ch);
    ansi(END, ch);
  } break;
  case -2: {
    if (!test) {
      sprintf(buffer,"You can't wear %s.\n\r",fname(obj_object->name));
      ansi(CLR_ERROR, ch);
      send_to_char(buffer, ch);
      ansi(END, ch);
    }
  } break;
  default: {
    log("Unknown type called in wear.");
  } break;
  }
}


void do_wear_all(struct char_data *ch) 
{
  struct obj_data *tmp_object;
  struct obj_data *next_obj;
  bool test = FALSE;
  int keyword;
 
  for(tmp_object = ch->carrying;
      tmp_object;
      tmp_object = next_obj) {
    next_obj = tmp_object->next_content;
    if (CAN_SEE_OBJ(ch, tmp_object)) {
 			
      keyword = -2 + 1000;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_SHIELD)) keyword = 1014;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_FINGER)) keyword = 1001;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_NECK)) keyword = 1002;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_WRIST)) keyword = 1011;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_WAISTE)) keyword = 1010;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_ARMS)) keyword = 1008;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_HANDS)) keyword = 1007;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_FEET)) keyword = 1006;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_LEGS)) keyword = 1005;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_ABOUT)) keyword = 1009;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_HEAD)) keyword = 1004;
      if (CAN_WEAR(tmp_object,ITEM_WEAR_BODY)) keyword = 1003;
      if (CAN_WEAR(tmp_object,ITEM_HOLD))      keyword = 1013;
      if (CAN_WEAR(tmp_object,ITEM_WIELD))     keyword = 1012;
      if (tmp_object->obj_flags.type_flag == ITEM_LIGHT) keyword = 1000;
      
      wear(ch, tmp_object, keyword);
      test = TRUE;
    }
  }
  if (!test) {
    ansi(CLR_ERROR, ch);
    send_to_char("You do not seem to have anything to wear.\n\r", ch);
    ansi(END, ch);
  }
}

void do_wear(struct char_data *ch, char *argument, int cmd) 
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buf[256];
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *obj_object;
  int keyword;
  const char *keywords[] = {
    "finger",
    "neck",
    "body",
    "head",
    "legs",
    "feet",
    "hands",
    "arms",
    "about",
    "waist",
    "wrist",
    "shield",
    "\n"
    };
  
  argument_interpreter(argument, arg1, arg2);
  
  if (*arg1) {
    if (!str_cmp(arg1,"all")) {
      do_wear_all(ch);
    } else {
      obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
      if (obj_object) {
	if (*arg2) {
	  keyword = search_block(arg2, keywords, FALSE); /* Partial Match */
	  if (keyword == -1) {
	    sprintf(buf, "%s is an unknown body location.\n\r", arg2);
	    ansi(CLR_ERROR, ch);
	    send_to_char(buf, ch);
	    ansi(END, ch);
	  } else {
	    wear(ch, obj_object, keyword+1);
	  }
	} else {
	  keyword = -2;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) keyword = 14;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) keyword = 1;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) keyword = 2;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) keyword = 11;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) keyword = 10;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) keyword = 8;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) keyword = 7;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) keyword = 6;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) keyword = 5;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) keyword = 9;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) keyword = 4;
	  if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) keyword = 3;
	  
	  wear(ch, obj_object, keyword);
	}
      } else {
	sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
	ansi(CLR_ERROR, ch);
	send_to_char(buffer,ch);
	ansi(END, ch);
      }
    }
  } else {
    ansi(CLR_ERROR, ch);
    send_to_char("Wear what?\n\r", ch);
    ansi(END, ch);
  }
}


void do_wield(struct char_data *ch, char *argument, int cmd) 
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *obj_object;
  int keyword = 12;
  
  argument_interpreter(argument, arg1, arg2);
  if (*arg1) {
    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
    if (obj_object) {
      wear(ch, obj_object, keyword);
    } else {
      sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
      ansi(CLR_ERROR, ch);
      send_to_char(buffer,ch);
      ansi(END, ch);
    }
  } else {
    ansi(CLR_ERROR, ch);
    send_to_char("Wield what?\n\r", ch);
    ansi(END, ch);
  }
}


void do_grab(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *obj_object;


  argument_interpreter(argument, arg1, arg2);

  if (*arg1) {
    obj_object = get_obj_in_list(arg1, ch->carrying);
    if (obj_object) {
      if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
	wear(ch, obj_object, WEAR_LIGHT);
      else
	wear(ch, obj_object, 13);
    } else {
      sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
      ansi(CLR_ERROR, ch);
      send_to_char(buffer,ch);
      ansi(END, ch);
    }
  } else {
    ansi(CLR_ERROR, ch);
    send_to_char("Hold what?\n\r", ch);
    ansi(END, ch);
  }
}


void do_remove(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  struct obj_data *obj_object;
  int  j;

  one_argument(argument, arg1);

  if (*arg1) {
    obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j);
    if (obj_object) {
      if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
	obj_to_char(unequip_char(ch, j), ch);
	if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
	  if (obj_object->obj_flags.value[2])
	    world[ch->in_room].light--;
	
	ansi_act("You stop using $p.",FALSE,ch,obj_object,0,TO_CHAR,CLR_ACTION);
	ansi_act("$n stops using $p.",TRUE,ch,obj_object,0,TO_ROOM,CLR_ACTION);
      } else {
	ansi(CLR_ERROR, ch);
	send_to_char("You can't carry that many items.\n\r", ch);
	ansi(END, ch);
      }
    } else {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not using it.\n\r", ch);
      ansi(END, ch);
    }
  } else {
    ansi(CLR_ERROR, ch);
    send_to_char("Remove what?\n\r", ch);
    ansi(END, ch);
  }
}

/* case 12: ...
   	if ( ( (ch->player.class == CLASS_CLERIC) && 
	     ( (GET_ITEM_TYPE(obj_object) == ITEM_WEAPON) && 
	     (sharp[obj_object->obj_flags.value[3]]) ) ||
	     ( (ch->player.class == CLASS_CLERIC) && 
	     ( (GET_ITEM_TYPE(obj_object) == ITEM_FIREWEAPON) &&
	     (sharp[obj_object->obj_flags.value[3]]) ) ) ) ) {
	     if (GET_ITEM_TYPE(obj_object) == ITEM_WEAPON) {
	     strcpy(buffer, "You can't wield that! its SHARP.\n\r");
	     strcat(buffer, "You are forbidden to use sharp weapons!\n\r");
	     send_to_char(buffer, ch);
	     }
	     if (GET_ITEM_TYPE(obj_object) == ITEM_FIREWEAPON) {
	     strcpy(buffer, 
	     "You can't wield that! it fires only SHARP things.\n\r");
	     strcat(buffer, "You are forbidden to use sharp weapons!\n\r");
	     send_to_char(buffer, ch);
	     }
	     } else {
*/

/* case 13: ...
		if ( (ch->player.class == CLASS_CLERIC) && 
		( (GET_ITEM_TYPE(obj_object) == ITEM_MISSILE) && 
		(sharp[obj_object->obj_flags.value[3]]) ) ) {
		strcpy(buffer,
		"This is a SHARP thing, to be used in a weapon.\n\r.");
		strcat(buffer, "You are forbidden to use sharp weapons!\n\r");
		send_to_char(buffer, ch);
		} else {
*/

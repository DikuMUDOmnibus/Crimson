/* ************************************************************************
*  file: limits.c , Limit and gain control module.        Part of DIKUMUD *
*  Usage: Procedures controling gain and limit.                           *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "spells.h"
#include "constants.h"
#include "fight.h"
#include "act.h"
#include "ansi.h"
#include "limits.h"

/* should be in config.sys */
#define CANT_IDLE_IN_FIGHT 1
#define IDLE_TIMEOUT 12  /* in ticks */
#define DROPLINK_TIMEOUT 48 /* in ticks */


/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

   if (age < 15)
      return(p0);                               /* < 15   */
   else if (age <= 29) 
      return (int) (p1+(((age-15)*(p2-p1))/15));  /* 15..29 */
   else if (age <= 44)
      return (int) (p2+(((age-30)*(p3-p2))/15));  /* 30..44 */
   else if (age <= 59)
      return (int) (p3+(((age-45)*(p4-p3))/15));  /* 45..59 */
   else if (age <= 79)
      return (int) (p4+(((age-60)*(p5-p4))/20));  /* 60..79 */
   else
      return(p6);                               /* >= 80 */
}


/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data *ch)
{
   int max;

   if (!IS_NPC(ch) && GET_LEVEL(ch) < IMO_LEV3) {
      max = (ch->points.max_mana);
      switch  (GET_CLASS(ch)) {
         case CLASS_MAGIC_USER:
         case CLASS_BARD:
	 case CLASS_EKNIGHT:
         case CLASS_DKNIGHT:
            max += (GET_LEVEL(ch)/2)*(con_app[GET_INT(ch)].hitp);
	    break;
         
	 case CLASS_CLERIC:
         case CLASS_KAI:
         case CLASS_DRAKKHAR:
            max += (GET_LEVEL(ch)/2)*(con_app[GET_WIS(ch)].hitp);
            break;
      }
   } else 
      max = 99;

   return(max);
}


int hit_limit(struct char_data *ch)
{
   int max;

   if (!IS_NPC(ch) && (GET_LEVEL(ch) < IMO_LEV3))
      max = (ch->points.max_hit) +
            (GET_LEVEL(ch)*con_app[GET_CON(ch)].hitp);
   else 
      max = (ch->points.max_hit);


/* Class/Level calculations */
/* Skill/Spell calculations */
   
  return (max);
}


int move_limit(struct char_data *ch)
{
   int max;

   max = ch->points.max_move;
   if(!IS_NPC(ch) && GET_LEVEL(ch) < IMO_LEV3)
   max += (GET_LEVEL(ch)/2)*(con_app[GET_DEX(ch)].hitp);

/* Class/Level calculations */
/* Skill/Spell calculations */

  return (max);
}




/* manapoint gain pr. game hour */
int mana_gain(struct char_data *ch)
{
   int gain;

   if(IS_NPC(ch)) {
      /* Neat and fast */
      gain = GET_LEVEL(ch);
   } else {
      gain = 8;
      gain += age(ch).year/20;

      /* Class calculations */

      /* Skill/Spell calculations */
      switch  (GET_CLASS(ch)) {
         case CLASS_MAGIC_USER:
            gain += gain;
            break;
         case CLASS_CLERIC:
            gain += gain;
            break;
         case CLASS_BARD:
            gain += gain >> 2;
            break;
         case CLASS_KAI:
            gain = MINV(12,(GET_ALIGNMENT(ch) - 200) / 50);
            break;
         case CLASS_DRAKKHAR:
            gain = MINV(12,(-1 * GET_ALIGNMENT(ch) - 200) / 50);
            break;
         case CLASS_EKNIGHT:
            gain = MINV(10,(GET_ALIGNMENT(ch) - 200) / 50);
            break;
         case CLASS_DKNIGHT:
            gain = MINV(10,(-1 * GET_ALIGNMENT(ch) - 200) / 50);
            break;
      }
      /* Position calculations    */
      switch (GET_POS(ch)) {
         case POSITION_SLEEPING:
            gain += gain;
            break;
         case POSITION_RESTING:
            gain+= (gain>>1);  /* Divide by 2 */
            break;
         case POSITION_SITTING:
            gain += (gain>>2); /* Divide by 4 */
            break;
      }
   }
   if (IS_AFFECTED(ch,AFF_REGENERATION))
      gain += (gain>>1);

   if (IS_AFFECTED(ch,AFF_POISON))
      gain >>= 2;

   if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
      gain >>= 2;
 
   if (world[ch->in_room].room_flags == TUNNEL) 
     gain += gain;     /* temporarily TUNNEL is for fast healing rooms */

  return (gain);
}


int hit_gain(struct char_data *ch)
/* Hitpoint gain pr. game hour */
{
   int gain;

   if(IS_NPC(ch)) {
      gain = GET_LEVEL(ch);
      /* Neat and fast */
   } else {
      if ((world[ch->in_room].sector_type ==SECT_UNDERWATER)
       && GET_LEVEL(ch) < IMO_LEV && !IS_AFFECTED(ch,AFF_BREATHWATER)) {
	ansi_act("You're starting to drown!",FALSE, ch, 0, 0, TO_CHAR,CLR_DAM);
	gain = -1*number(1,4);
	if (IS_AFFECTED(ch,AFF_POISON)) {
	  gain >>= 2;
	  DAMAGE(ch,ch,2,SPELL_POISON);
         }
      }

      /* gain = graf(age(ch).year, 2,5,10,18,6,4,2); */
      gain = 16;
      gain -= age(ch).year/10;

      /* Class/Level calculations */
      switch  (GET_CLASS(ch)) {
         case CLASS_MAGIC_USER:
            gain = gain >> 1;
         case CLASS_WARRIOR:
            gain += gain >> 1;
            break;
         case CLASS_THIEF:
            gain += gain >> 2;
            break;
         case CLASS_DRAGONW:
            gain += gain;
            break;
         case CLASS_KAI:
            gain = MINV(18,(GET_ALIGNMENT(ch) - 200) / 50);
            gain = MAXV(gain,-5);
            break;
         case CLASS_DRAKKHAR:
            gain = MINV(18,(-1 * GET_ALIGNMENT(ch) - 200) / 50);
            gain = MAXV(gain,-5);
            break;
         case CLASS_EKNIGHT:
            gain = MINV(18,(GET_ALIGNMENT(ch) - 200) / 50);
            gain = MAXV(gain,0);
            break;
         case CLASS_DKNIGHT:
            gain = MINV(18,(-1 * GET_ALIGNMENT(ch) - 200) / 50);
            gain = MAXV(gain,0);
            break;
      }

      /* Position calculations    */
      switch (GET_POS(ch)) {
         case POSITION_SLEEPING:
            gain += (gain>>1); /* Divide by 2 */
            break;
         case POSITION_RESTING:
            gain+= (gain>>2);  /* Divide by 4 */
            break;
         case POSITION_SITTING:
            gain += (gain>>3); /* Divide by 8 */
            break;
      }
   }
   if (IS_AFFECTED(ch,AFF_REGENERATION))
      gain += gain>>1;

   if (IS_AFFECTED(ch,AFF_POISON))
      {
      gain >>= 2;
      DAMAGE(ch,ch,2,SPELL_POISON);
      }
   if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
      gain >>= 2;

   if (world[ch->in_room].room_flags == TUNNEL) 
     gain += gain;    /* temporarily TUNNEL is for fast healing rooms */

   return (gain);
}



int move_gain(struct char_data *ch)
/* move gain pr. game hour */
{
   int gain;

   if(IS_NPC(ch)) {
      return(GET_LEVEL(ch));  
      /* Neat and fast */
   } else {
      /* gain = graf(age(ch).year, 12,18,22,21,14,10,6) */;
      gain = 21;
      gain -= age(ch).year/10;

      /* Class/Level calculations */
      switch  (GET_CLASS(ch)) {
         case CLASS_THIEF:
            gain += gain;
            break;
         case CLASS_BARD:
            gain += gain >> 1;
            break;
         case CLASS_DRAGONW:
            gain -= gain>>2;
            break;
      }

      /* Position calculations    */
      switch (GET_POS(ch)) {
         case POSITION_SLEEPING:
            gain += (gain>>1); /* Divide by 2 */
            break;
         case POSITION_RESTING:
            gain+= (gain>>2);  /* Divide by 4 */
            break;
         case POSITION_SITTING:
            gain += (gain>>3); /* Divide by 8 */
            break;
      }
   }
   if (IS_AFFECTED(ch,AFF_REGENERATION))
      gain += gain>>1;
   if (IS_AFFECTED(ch,AFF_POISON))
      gain >>= 2;
   if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
      gain >>= 2;

   if (world[ch->in_room].room_flags == TUNNEL) 
     gain += gain;       /* temporarily TUNNEL is for fast healing rooms */

   return (gain);
}

/* Gain maximum in various points */
void advance_level(struct char_data *ch)
{
   int add_hp, add_move,add_mana,i;

   add_hp = 0;
   add_move = 0;
   add_mana = 0;

   switch(GET_CLASS(ch)) {
      case CLASS_MAGIC_USER : 
         add_hp += number(4, 8);
         add_move += number(1,8);
         add_mana += number(1,5);
         break;
      case CLASS_CLERIC : 
         add_hp += number(5, 10);
         add_move += number(1,8);
         add_mana += number(2,6);
         break;
      case CLASS_THIEF : 
         add_hp += number(6,13);
         add_move += number(5,8);
         break;
      case CLASS_WARRIOR : 
         add_hp += number(9,15);
         add_move += number(1,8);
         break;
      case CLASS_BARD : 
         add_hp += number(5, 13);
         add_move += number(3,8);
         add_mana += number(0,3);
         break;
      case CLASS_KAI : 
         add_hp  += number(8, 14);
         add_move += number(1,8);
         add_mana += number(0,3);
         break;
      case CLASS_DRAKKHAR : 
         add_hp += number(8,14);
         add_move += number(1,8);
         add_mana += number(0,3);
         break;
      case CLASS_EKNIGHT : 
         add_hp += number(7,13);
         add_move += number(1,8);
         add_mana += number(0,3);
         break;
      case CLASS_DKNIGHT : 
         add_hp += number(7, 13);
         add_move += number(1,8);
         add_mana += number(0,3);
         break;
      case CLASS_DRAGONW : 
         add_hp += number(10, 17);
         add_move += number(1,6);
         break;
   }

   ch->points.max_hit += MAXV(1, add_hp);
   ch->points.max_mana += MAXV(1, add_mana);
   ch->points.max_move += MAXV(1, add_move);

   if (GET_CLASS(ch) == CLASS_MAGIC_USER || GET_CLASS(ch) == CLASS_CLERIC)
      ch->specials.spells_to_learn += MAXV(2,number(0, wis_app[GET_WIS(ch)].bonus)+1);
   else if (GET_CLASS(ch) == CLASS_WARRIOR || GET_CLASS(ch) == CLASS_THIEF || GET_CLASS(ch) == CLASS_DRAGONW)
      ch->specials.spells_to_learn += number(0, wis_app[GET_WIS(ch)].bonus)/2+1;
   else
      ch->specials.spells_to_learn += number(0, wis_app[GET_WIS(ch)].bonus+1)/2+2;

   if (GET_LEVEL(ch) >= IMO_LEV )
      for (i = 0; i < 3; i++)
         ch->specials.conditions[i] = -1;
   if (GET_LEVEL(ch) == 3 ||
   GET_LEVEL(ch) == 5 || GET_LEVEL(ch) == 8) roll_abilities(ch);

}  


void set_title(struct char_data *ch)
{
   if (GET_TITLE(ch))
      RECREATE(GET_TITLE(ch),char,strlen(READ_TITLE(ch))+1);
   else
      CREATE(GET_TITLE(ch),char,strlen(READ_TITLE(ch))+1);
   strcpy(GET_TITLE(ch), READ_TITLE(ch));
}



void gain_exp(struct char_data *ch, int gain)
{
   int i;
   char buf[128];
   bool is_altered = FALSE;

   if (IS_SET(zone_table[world[ch->in_room].zone].flags, ZONE_TESTING)) {
     ansi_act("Warning: This zone is till being tester.", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
   }
   if (IS_SET(zone_table[world[ch->in_room].zone].flags, ZONE_NOENTER)) {
     ansi_act("Warning: This zone is closed - why are you here?", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
   }
   if (IS_NPC(ch) || ((GET_LEVEL(ch) < IMO_LEV) && (GET_LEVEL(ch) > 0))) {
      if (gain > 0) {
         GET_EXP(ch) += gain;
         if ((!IS_NPC(ch))&&(GET_LEVEL(ch) < IMO_LEV) ) {
            gain = MAXV(gain, 
              (titles[GET_CLASS(ch)-1][GET_LEVEL(ch)+1].exp
              - titles[GET_CLASS(ch)-1][GET_LEVEL(ch)].exp)/4
            );
            gain = MAXV(gain, 200000);
            for (i = 0; titles[GET_CLASS(ch)-1][i].exp <= GET_EXP(ch) && GET_LEVEL(ch) < IMO_LEV; i++) {
               if (i > GET_LEVEL(ch)) {
		 ansi_act("Congratulations! You raise a level.", FALSE, ch, 0, 0, TO_CHAR,WHITE);
		 GET_LEVEL(ch) = i;
		 advance_level(ch);
		 sprintf(buf, "SYS: %s has raised a level!", GET_NAME(ch));
		 do_sys(buf, 1);
		 is_altered = TRUE;
		 sprintf(buf,"[1;33mGrats: [The Mayor of Midgaard] I am proud of you, %s!\n\r[0;37m", GET_NAME(ch));
		 send_to_all(buf);
               }
            }
         }
      }

      if (gain < 0) {
         gain = MAXV(-500000, gain);  /* Never lose more than 1/2 mil */
         GET_EXP(ch) += gain;
         if (GET_EXP(ch) < 0)
            GET_EXP(ch) = 0;
      }

      if (is_altered)
         set_title(ch);
   }
}


void gain_exp_regardless(struct char_data *ch, int gain)
{
   int i;
   bool is_altered = FALSE;

   if (!IS_NPC(ch)) {
      if (gain > 0) {
         GET_EXP(ch) += gain;
         for (i = 0; (i<MAX_LEV) && (titles[GET_CLASS(ch)-1][i].exp <= GET_EXP(ch)); i++) {
            if (i > GET_LEVEL(ch)) {
	      ansi_act("Congratulations! You raise a level.", FALSE, ch, 0, 0, TO_CHAR,WHITE);
	      GET_LEVEL(ch) = i;
	      advance_level(ch);
	      is_altered = TRUE;
            }
         }
      }
      if (gain < 0) 
         GET_EXP(ch) += gain;
      if (GET_EXP(ch) < 0)
         GET_EXP(ch) = 0;
   }
   if (is_altered)
      set_title(ch);
}

void gain_condition(struct char_data *ch,int condition,int value)
{
   bool intoxicated;

   if(GET_COND(ch, condition)==-1) /* No change */
      return;

   intoxicated=(GET_COND(ch, DRUNK) > 0);

   GET_COND(ch, condition)  += value;
   GET_COND(ch,condition) = MAXV(0,GET_COND(ch,condition));
   GET_COND(ch,condition) = MINV(24,GET_COND(ch,condition));

   if(GET_COND(ch,condition))
      return;
   switch(condition){
      case FULL :{
         ansi_act("You are hungry.", FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
         return;
      }
      case THIRST :{
         ansi_act("You are thirsty.", FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
         return;
      }
      case DRUNK :{
         if(intoxicated) {
         ansi_act("You are now sober.", FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
         return;
       }
      }
      default : break;
   }
}


void check_idling(struct char_data *ch)
{
   char buf[MAX_INPUT_LENGTH];

   if (++(ch->specials.timer) > IDLE_TIMEOUT)
      if (ch->specials.was_in_room == NOWHERE && ch->in_room != NOWHERE)
      {
         if (ch->specials.fighting)
         {
            if (CANT_IDLE_IN_FIGHT) {
              return; /* keep fighting till death */
            } else {
              stop_fighting(ch->specials.fighting);
              stop_fighting(ch);
            }
         }
         ch->specials.was_in_room = ch->in_room;
         ansi_act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
         act("You have been idle, and are pulled into a void.", FALSE, ch, 0, 0, TO_CHAR);
         char_from_room(ch);
         char_to_room(ch, 1);  /* Into room number 0 */
      }
      else if (ch->specials.timer > DROPLINK_TIMEOUT)
      {
         int save_room;
         struct obj_cost cost;
         int recep_offer(struct char_data *ch, struct char_data *receptionist, struct obj_cost *cost);
         int save_obj(struct char_data *ch, struct obj_cost *cost);
         int del_objs(struct char_data *ch); /* inventory & equip */
         int del_char_objs(struct char_data *ch); /* del rent file */

         if (ch->in_room != NOWHERE)
            char_from_room(ch);
         if (ch->specials.was_in_room != NOWHERE)
            char_to_room(ch, ch->specials.was_in_room);
         else
            char_to_room(ch, 1);

         /* if we got the money rent us out */
         if (recep_offer(ch, ch, &cost)) {
           sprintf(buf, "SYSTEM: %s has just been auto-rented.", GET_NAME(ch));
           log(buf); 
           obj_to_char(read_object(3431, VIRTUAL),ch);  /* give autorent 
                                                           notice */
           save_obj(ch, &cost);
           del_objs(ch);
           save_room = ch->in_room;
           extract_char(ch);
           ch->in_room = world[save_room].number;
           save_char(ch, ch->in_room);
         } else {
           sprintf(buf, "SYSTEM: %s hasn't got enough coins for auto-rent.", GET_NAME(ch));
           log(buf); 
           save_room = ch->in_room;
           extract_char(ch);
	   del_char_objs(ch);
           ch->in_room = world[save_room].number;
           save_char(ch, ch->in_room);
	 }

         /* close out the link the player */
         if (ch->desc)
            close_socket(ch->desc);
         ch->desc = 0;
      }
}





/* Update both PC's & NPC's and objects*/
void point_update( void )
{  
  void update_char_objects( struct char_data *ch ); /* handler.c */
  void extract_obj(struct obj_data *obj); /* handler.c */
  struct char_data *i, *next_dude;
  struct obj_data *j, *next_thing, *jj, *next_thing2;

  /* characters */
  for (i = character_list; i; i = next_dude) {
    next_dude = i->next;
    GET_HIT(i)  = MINV(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
    GET_MANA(i) = MINV(GET_MANA(i) + mana_gain(i), mana_limit(i));
    GET_MOVE(i) = MINV(GET_MOVE(i) + move_gain(i), move_limit(i));
    if (GET_POS(i) == POSITION_INCAP)
      DAMAGE(i, i, 1, TYPE_SUFFERING);
    else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW)) {
      DAMAGE(i, i, 2, TYPE_SUFFERING);
    } else if (GET_POS(i) == POSITION_STUNNED || GET_HIT(i) <= 0) 
      update_pos( i );
    if (!IS_NPC(i)) {
      update_char_objects(i);
      if (GET_LEVEL(i) <= IMO_LEV2)
	check_idling(i);
    }
    gain_condition(i,FULL,-1);
    gain_condition(i,DRUNK,-1);
    gain_condition(i,THIRST,-1);
  } /* for */
  
  /* objects */
  for(j = object_list; j ; j = next_thing){
    next_thing = j->next; /* Next in object list */
    
    /* If this is a corpse */
    if ((GET_ITEM_TYPE(j) == ITEM_CONTAINER) && (j->obj_flags.value[3])) {
      /* timer count down */
      if (j->obj_flags.timer > 0) j->obj_flags.timer--;

      if (!j->obj_flags.timer) {
	if (j->carried_by)
	  act("$p decay in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) && (world[j->in_room].people)){
	  act("A quivering hoard of maggots consume $p.",
	      TRUE, world[j->in_room].people, j, 0, TO_ROOM);
	  act("A quivering hoard of maggots consume $p.",
	      TRUE, world[j->in_room].people, j, 0, TO_CHAR);
	}

        for(jj = j->contains; jj; jj = next_thing2) {
	  next_thing2 = jj->next_content; /* Next in inventory */
	  obj_from_obj(jj);

	  if (j->in_obj)
	    obj_to_obj(jj,j->in_obj);
	  else if (j->carried_by)
	    obj_to_room(jj,j->carried_by->in_room);
	  else if (j->in_room != NOWHERE)
	    obj_to_room(jj,j->in_room);
	  else
	    assert(FALSE);
	}
	extract_obj(j);
      }
    }
  }
}

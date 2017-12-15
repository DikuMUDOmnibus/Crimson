/* ************************************************************************
*  file: magic.c , Implementation of spells.              Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <assert.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "limits.h"
#include "constants.h"
#include "fight.h"
#include "act.h"
#include "ansi.h"
#include "spells.h"

int magic_fails(struct char_data *ch, struct char_data *tar_ch){
   /* check for magic resistance/wards/immunity */
   if ((tar_ch) && (IS_AFFECTED(tar_ch, AFF_MAGIC_IMMUNE))) {
      if (ch) {
        ansi(CLR_ERROR, ch);
        send_to_char("The magic is nullified somehow.\n\r", ch);
	ansi(END, ch);
      }
      return TRUE;
   } else if ((tar_ch) && (IS_AFFECTED(tar_ch, AFF_MAGIC_RESIST))
              && (number(0,101)<GET_LEVEL(tar_ch)+25)) {
      if (ch) {
        ansi(CLR_ERROR, ch);
        send_to_char("The magic fizzles unspectacularly.\n\r", ch);
        ansi(END, ch);
      }
      return TRUE;
   }
   return FALSE;
}


/* Offensive Spells */
void spell_magic_missile(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  dam = dice(MINV(level,4),3)+MINV(level,4);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  DAMAGE(ch, victim, dam, SPELL_MAGIC_MISSILE);
}



void spell_chill_touch(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  int dam;
    
  assert(victim && ch);
  assert((level >= 1) && (level <= NPC_LEV)); 
  if (magic_fails(ch,victim)) return;

  dam = dice(MINV(level>>1,6),6)+MINV((level+1)>>1,12);

 if ( !saves_spell(victim, SAVING_SPELL) )
  {
    af.type      = SPELL_CHILL_TOUCH;
    af.duration  = 6;
    af.modifier  = -2;
    af.location  = APPLY_STR;
    af.bitvector = 0;
    affect_join(victim, &af, TRUE, FALSE);
  } else {
    dam >>= 1;  
  }  
  DAMAGE(ch, victim, dam, SPELL_CHILL_TOUCH);
}



void spell_burning_hands(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
 

  assert(victim && ch);
  assert((level >= 1) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  dam = dice(MINV(level>>1,10),6)+10;

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  DAMAGE(ch, victim, dam, SPELL_BURNING_HANDS);
}



void spell_shocking_grasp(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
 
  assert(victim && ch);
  assert((level >= 1) && (level <= NPC_LEV)); 
  if (magic_fails(ch,victim)) return;

  dam = dice(MINV(level>>1,10),8)+level;
  DAMAGE(ch, victim, dam, SPELL_SHOCKING_GRASP);
}



void spell_lightning_bolt(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
 
  assert(victim && ch);
  assert((level >= 1) && (level <= NPC_LEV)); 
  if (magic_fails(ch,victim)) return;

  dam = dice(MINV(level,20),6)+20;

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  DAMAGE(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}



void spell_colour_spray(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  int dam;
 
  assert(victim && ch);
  assert((level >= 1) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  dam = dice(MINV(level,20),8)+level;

  if ( saves_spell(victim, SAVING_SPELL) && !(af.bitvector = AFF_BLIND) )
  {
   dam >>= 1;
   ansi_act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM,CLR_DAM);
   ansi(CLR_ERROR, victim);
   send_to_char("You have been blinded!\n\r", victim);
   ansi(END, victim);
   af.type      = SPELL_BLINDNESS;
   af.location  = APPLY_HITROLL;
   af.modifier  = -4;  /* Make hitroll worse */
   af.duration  = 1;
   af.bitvector = AFF_BLIND;
   affect_to_char(victim, &af);
   af.location = APPLY_AC;
   af.modifier = +40; /* Make AC Worse! */
   affect_to_char(victim, &af);
  }

  DAMAGE(ch, victim, dam, SPELL_COLOUR_SPRAY);
}


/* Drain XP, MANA, HP - caster gains HP and MANA */
void spell_energy_drain(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam, xp, mana;

  void set_title(struct char_data *ch);
  void gain_exp(struct char_data *ch, int gain);

  assert(victim && ch);
  if (magic_fails(ch,victim)) return;

  if ( !saves_spell(victim, SAVING_SPELL) ) {
    GET_ALIGNMENT(ch) = MINV(-1000, GET_ALIGNMENT(ch)-200);
    if (GET_LEVEL(victim) <= 5) {
      DAMAGE(ch, victim, 150, SPELL_ENERGY_DRAIN); /* Kill the sucker */
    } else {
      xp = number(level>>1,level)*1000;
      gain_exp(victim, -xp);

      dam = dice(1,30);

      mana = GET_MANA(victim)>>1;
      GET_MOVE(victim) >>= 1;
      GET_MANA(victim) = mana;

      GET_MANA(ch) += mana>>1;
      GET_HIT(ch) += dam;

      ansi(CLR_DAM, victim);
      send_to_char("Your life energy is drained!\n\r", victim);
      ansi(END, ch);
      DAMAGE(ch, victim, dam, SPELL_ENERGY_DRAIN);
    }
  } else {
     DAMAGE(ch, victim, 0, SPELL_ENERGY_DRAIN); /* Miss */
  }
}



void spell_fireball(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
 
  assert(victim && ch);
  if (magic_fails(ch,victim)) return;

  dam = dice(MINV(level,25),10)+2*level;

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  DAMAGE(ch, victim, dam, SPELL_FIREBALL);
}


void spell_turn_undead(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
 
  assert(victim && ch);

  if (IS_NPC(victim) && IS_SET(victim->specials.act, CLASS_UNDEAD)) { 
    dam = dice(level,4)+level;
    DAMAGE(ch, victim, dam, SPELL_TURN_UNDEAD);
  } else
    DAMAGE(ch, victim, 0, SPELL_TURN_UNDEAD);    
}


void spell_earthquake(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmp_victim, *temp, *leader;

  assert(ch);
  assert((level >= 1) && (level <= NPC_LEV)); 

  dam = dice(3,8)+2*level;

  ansi_act("The earth trembles beneath your feet!", FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
  ansi_act("$n makes the earth tremble and shiver!", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);

  if (ch->master)
    leader = ch->master;
  else
    leader = ch;
  for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim) ) {
      if (tmp_victim->master != leader)
         DAMAGE(ch, tmp_victim, dam, SPELL_EARTHQUAKE);
    } else
      if (world[ch->in_room].zone == world[tmp_victim->in_room].zone) {
        ansi_act("The earth trembles and shiver!", FALSE, tmp_victim, 0, 0, TO_CHAR,CLR_ACTION);
      }
  }
}



void spell_dispel_evil(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

   assert(ch && victim);
   assert((level >= 1) && (level<=NPC_LEV));
   if (magic_fails(ch,victim)) return;

   if (IS_EVIL(ch)) {
      victim = ch;
      ansi(CLR_DAM, ch);
      send_to_char("You are PRIME EVIL yourself!!!\n\r", ch);
      ansi(END, ch);
    }
   else
     if (IS_GOOD(victim)) {
         ansi_act("God protects $N.", FALSE, ch, 0, victim, TO_CHAR,CLR_ERROR);
         return;
     }

  if ((GET_LEVEL(victim) < level) || (victim == ch))
    dam = 100;
  else {
    dam = dice(level,4);
    if ( saves_spell(victim, SAVING_SPELL) )
     dam >>= 1;
  }

  DAMAGE(ch, victim, dam, SPELL_DISPEL_EVIL);
}



void spell_call_lightning(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  extern struct weather_data weather_info;

  assert(victim && ch);
  assert((level >= 1) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  dam = dice(MINV(level,15), 8);

  if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
    if ( saves_spell(victim, SAVING_SPELL) )
      dam >>= 1;
    DAMAGE(ch, victim, dam, SPELL_CALL_LIGHTNING);
   } else {
     ansi(CLR_ERROR, ch);
     send_to_char("Try later when bad weather.\n\r", ch);
     ansi(END, ch);
   }
}

void spell_harm(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int dam;

   assert(victim && ch);
   assert((level >= 1) && (level <= NPC_LEV));
   if (magic_fails(ch,victim)) return;

   dam = GET_HIT(victim) - dice(1,4);
   dam = MINV(100+2*GET_LEVEL(ch),dam);

   if (dam < 0)
      dam = 0; /* Kill the suffering bastard */
   else {
     if ( saves_spell(victim, SAVING_SPELL) )
         dam = MINV(20, dam/2);
   }

   DAMAGE(ch, victim, dam, SPELL_HARM);
}

void spell_conflagration(sbyte level, struct char_data *ch, struct char_data *victim, struct obj_data *obj)
{
   int dam;
   assert(victim && ch);
   assert((level >= 1) && (level <= NPC_LEV));
   if (magic_fails(ch,victim)) return;
 
   dam = dice(level, 14)+level;
   if (dam < 0)
      dam = 0;  /* Toast 'em off... */
   else { /* Hey GP forgot the open brace */
      if (saves_spell(victim, SAVING_SPELL))
              dam = dam/2;
   }
   dam = MAXV(250,dam);
   DAMAGE(ch, victim, dam, SPELL_CONFLAGRATION);
}

/* spells2.c - Not directly offensive spells */

void spell_armor(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  if (!affected_by_spell(victim, SPELL_ARMOR)) {
     af.type      = SPELL_ARMOR;
     af.duration  = 24;
     af.modifier  = -1*(MINV(20,level*2));
     af.location  = APPLY_AC;
     af.bitvector = 0;

     affect_to_char(victim, &af);
     if (victim->desc) {
       ansi(CLR_ACTION, victim);
       send_to_char("You feel someone protecting you.\n\r", victim);
       ansi(END, victim);
     }
     ansi_act("$n glows white for a moment.",FALSE,victim,0,0,TO_ROOM,WHITE);
  } else {
    ansi(CLR_ERROR, ch);
    send_to_char("Nothing new happens.\n\r", ch);
    ansi(END, ch);
  }  
}


void spell_teleport_zone(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int to_room,i,x,y,num,no;

   assert(ch);
   i = world[ch->in_room].zone;
   x = zone_table[i].real_bottom;
   y = zone_table[i].real_top;
   num = number(0, MAXV(1,y-x));
   
   for (no = 0; no < num; no++) { /* what am I doing here */
      to_room = world[to_room].next;
      if (IS_SET(world[to_room].room_flags, PRIVATE))
         to_room = world[to_room].next;
      if (world[to_room].zone != i) {
         to_room = zone_table[i].real_top; 
         break;
      } 
   }
   if (IS_SET(world[to_room].room_flags, PRIVATE))
      to_room = ch->in_room;
   if (world[to_room].zone != i)
      to_room = ch->in_room;

   ansi_act("$n suddenly winks out of existence.", FALSE, ch,0,0,TO_ROOM,CLR_ACTION);
   char_from_room(ch);
   char_to_room(ch, to_room);
   ansi_act("$n suddenly winks into existence.", FALSE, ch,0,0,TO_ROOM,CLR_ACTION);

   do_look(ch, "", 0);

   if (!IS_NPC(ch) && IS_SET(world[to_room].room_flags, DEATH) && GET_LEVEL(ch) < IMO_LEV) {
         death_cry(ch);
         extract_char(ch);
   }
}


void spell_teleport(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  void do_look(struct char_data *ch, char *argument, int cmd);
   int to_room;
   extern int top_of_world;      /* ref to the top element of world */

   assert(ch);

   do {
      to_room = number(0, top_of_world);
   } while (IS_SET(world[to_room].room_flags, PRIVATE));

   ansi_act("$n suddenly winks out of existence.", FALSE, ch,0,0,TO_ROOM,CLR_ACTION);
   char_from_room(ch);
   char_to_room(ch, to_room);
   ansi_act("$n suddenly winks into existence.", FALSE, ch,0,0,TO_ROOM,CLR_ACTION);

   do_look(ch, "", 0);

   if (!IS_NPC(ch) && IS_SET(world[to_room].room_flags, DEATH) && GET_LEVEL(ch) < IMO_LEV) {
         death_cry(ch);
         extract_char(ch);
   }
}



void spell_bless(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && (victim || obj));
  assert((level >= 0) && (level <= NPC_LEV));

  if (obj) {
    if ( (5*GET_LEVEL(ch) > GET_OBJ_WEIGHT(obj)) &&
         (GET_POS(ch) != POSITION_FIGHTING) &&
         !IS_OBJ_STAT(obj, ITEM_EVIL)) {
        SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
        ansi_act("$p briefly glows.",FALSE,ch,obj,0,TO_CHAR,WHITE);
      }
  } else {
     if ((GET_POS(victim) != POSITION_FIGHTING) &&
         (!affected_by_spell(victim, SPELL_BLESS))) {

       if (magic_fails(ch,victim)) return;
       send_to_char("You feel righteous.\n\r", victim);
       af.type      = SPELL_BLESS;
       af.duration  = 6;
       af.modifier  = 1;
       af.location  = APPLY_HITROLL;
       af.bitvector = 0;
       affect_to_char(victim, &af);

       af.location = APPLY_SAVING_SPELL;
       af.modifier = -1;                 /* Make better */
       affect_to_char(victim, &af);
     } 
     else if (affected_by_spell(victim, SPELL_BLESS)) {
       ansi(CLR_ERROR, ch);
       send_to_char("Nothing new happens.\n\r", ch);
       ansi(END, ch);
     }
  }
}



void spell_blindness(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;


  if (saves_spell(victim, SAVING_SPELL) ||
      affected_by_spell(victim, SPELL_BLINDNESS)) {
      ansi_act("$N shrugs off your spell with an effort of will.",FALSE,ch,0,victim,TO_CHAR,CLR_ERROR);
      return;
  }

  ansi_act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM,CLR_ACTION);
  ansi(CLR_ERROR, victim);
  send_to_char("You have been blinded!\n\r", victim);
  ansi(END, victim);

  af.type      = SPELL_BLINDNESS;
  af.location  = APPLY_HITROLL;
  af.modifier  = -4;  /* Make hitroll worse */
  af.duration  = 3;
  af.bitvector = AFF_BLIND;
  affect_to_char(victim, &af);


  af.location = APPLY_AC;
  af.modifier = +40; /* Make AC Worse! */
  affect_to_char(victim, &af);
  ansi_act("You successfully blind $N.",FALSE,ch,0,victim,TO_CHAR,CLR_ACTION);
}



void spell_control_weather(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   /* Control Weather is not possible here!!! */
   /* Better/Worse can not be transferred     */
}



void spell_create_food(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *tmp_obj;

  assert(ch);
  assert((level >= 0) && (level <= NPC_LEV));

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = str_alloc("mushroom");
  tmp_obj->short_description = str_alloc("A Magic Mushroom");
  tmp_obj->description = str_alloc("A really delicious looking magic mushroom lies here.");

  tmp_obj->obj_flags.type_flag = ITEM_FOOD;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE;
  tmp_obj->obj_flags.value[0] = 5+level;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_room(tmp_obj,ch->in_room);

  tmp_obj->item_number = -1;

  ansi_act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM,CLR_ACTION);
  ansi_act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR,CLR_ACTION);
}

void spell_sustenance(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  assert(ch && victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;
  
  victim->specials.conditions[THIRST] = MAXV(victim->specials.conditions[THIRST],24);
  victim->specials.conditions[FULL] = MAXV(victim->specials.conditions[FULL],24);
  if (GET_LEVEL(victim)>=IMO_LEV) {
    victim->specials.conditions[THIRST] = -1;
    victim->specials.conditions[FULL] = -1;
  }
  ansi_act("You feel your hunger and thirst decrease.",TRUE,victim,0,0,TO_CHAR,CLR_ACTION);
  ansi_act("$n glows yellow for a moment.",FALSE,victim,0,0,TO_ROOM,YELLOW);
  ansi(CLR_ACTION, ch);
  send_to_char("You have just given a feast!\n\r", ch);
  ansi(END, ch);
}


void spell_create_water(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int water;

  extern struct weather_data weather_info;
  void name_to_drinkcon(struct obj_data *obj,int type);
  void name_from_drinkcon(struct obj_data *obj);

  assert(ch && obj);

   if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
      if ((obj->obj_flags.value[2] != LIQ_WATER)
           && (obj->obj_flags.value[1] != 0)) {

         name_from_drinkcon(obj);
         obj->obj_flags.value[2] = LIQ_SLIME;
         name_to_drinkcon(obj, LIQ_SLIME);

      } else {

         water = 2*level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

         /* Calculate water it can contain, or water created */
         water = MINV(obj->obj_flags.value[0]-obj->obj_flags.value[1], water);

         if (water > 0) {
            obj->obj_flags.value[2] = LIQ_WATER;
            obj->obj_flags.value[1] += water;

            weight_change_object(obj, water);

            name_from_drinkcon(obj);
            name_to_drinkcon(obj, LIQ_WATER);
            ansi_act("$p is filled.", FALSE, ch, obj, 0,TO_CHAR,CLR_ACTION);
	    ansi_act("$n magically fills $p with water.", TRUE, ch, obj , 0, TO_ROOM,CLR_ACTION);
	  }
      }
   }
}


void spell_cure_blind(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

   if (affected_by_spell(victim, SPELL_BLINDNESS)) {
     affect_from_char(victim, SPELL_BLINDNESS);

     ansi(CLR_ACTION, victim);
     send_to_char("Your vision returns!\n\r", victim);
     ansi(END, victim);
     ansi_act("$n's vision returns.",FALSE,victim,0,0,TO_ROOM,CLR_ACTION);
   } else {
     ansi(CLR_ERROR, ch);
     send_to_char("But your friend isn't blind!\n\r", ch);
     ansi(END, ch);
   }
}

void spell_cure_critic(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  healpoints = dice(3,5)+2*level;
  healpoints = MAXV(40, healpoints);

  if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  ansi(CLR_ACTION, victim);
  send_to_char("You feel better!\n\r", victim);
  ansi(END, victim);
  ansi_act("$n starts to look a little better.",FALSE,victim,0,0,TO_ROOM,CLR_ACTION);

  update_pos(victim);
}

void spell_donate_mana(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  if ( (15 + GET_MANA(victim)) > mana_limit(victim) )
    GET_MANA(victim) = mana_limit(victim);
  else
    GET_MANA(victim) += 15;

  ansi(CLR_ACTION, victim);
  send_to_char("You feel mana flow into you!\n\r", victim);
  ansi(END, victim);
  if (victim != ch)
    ansi_act("You feel some of your mana flow to $N.",FALSE,ch,0,victim,TO_CHAR,CLR_ACTION);
}

void spell_cause_critic(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  healpoints = dice(3,8)+3;
  DAMAGE(ch,victim,healpoints,SPELL_CAUSE_CRITIC);

  update_pos(victim);
}

void spell_cure_light(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  assert(ch && victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  healpoints = dice(1,8) + level;
  healpoints = MAXV(20, healpoints);

  if ( (healpoints+GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  update_pos( victim );

  ansi(CLR_ACTION, victim);
  send_to_char("You feel slightly better!\n\r", victim);
  ansi(END, victim);
  ansi_act("$n looks slightly better.",FALSE,victim,0,0,TO_ROOM,CLR_ACTION);
}

void spell_cause_light(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  assert(ch && victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  healpoints = dice(1,8)+1;
  DAMAGE(ch,victim,healpoints,SPELL_CAUSE_LIGHT);
   
  update_pos( victim );
}

void spell_curse(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim || obj);
  assert((level >= 0) && (level <= NPC_LEV));

  if (obj) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_EVIL);
    SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

    /* LOWER ATTACK DICE BY -1 */
    if(obj->obj_flags.type_flag == ITEM_WEAPON)
      obj->obj_flags.value[2]--;
    ansi_act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR,LRED);
  } else {
    if (magic_fails(ch,victim)) return;
    if (saves_spell(victim, SAVING_SPELL)) {
      ansi(CLR_ERROR, ch);
      send_to_char("You failed.\n\r", ch);
      ansi(END, ch);
      return;
    } else if (affected_by_spell(victim, SPELL_CURSE)) {
      ansi(CLR_ERROR, ch);
      send_to_char("Your enemy is already badly cursed!\n\r", ch);
      ansi(END, ch);
      return;
    } else {
      af.type      = SPELL_CURSE;
      af.duration  = 24*7;       /* 7 Days */
      af.modifier  = -1;
      af.location  = APPLY_HITROLL;
      af.bitvector = AFF_CURSE;
      affect_to_char(victim, &af);
      
      af.location = APPLY_SAVING_PARA;
      af.modifier = 1; /* Make worse */
      affect_to_char(victim, &af);
      
     ansi_act("$n is surrounded red aura!", FALSE, victim, 0, 0, TO_ROOM,LRED);
     ansi_act("You feel very UNCOMFORTABLE.",FALSE,victim,0,0,TO_CHAR,CLR_DAM);
    }
  }
}

void spell_detect_evil(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

   assert(victim);
   assert((level >= 0) && (level <= NPC_LEV));
   if (magic_fails(ch,victim)) return;

   if ( affected_by_spell(victim, SPELL_DETECT_EVIL) ) {
     ansi(CLR_ERROR, ch);
     send_to_char("Nothing new happens.\n\r", ch);
     ansi(END, ch);
     return;
   }

  af.type      = SPELL_DETECT_EVIL;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;

  affect_to_char(victim, &af);

  ansi(YELLOW, victim);
  send_to_char("You can now distinguish evil from good.\n\r", victim);
  ansi(END, victim);
}



void spell_detect_invisibility(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  if ( affected_by_spell(victim, SPELL_DETECT_INVISIBLE) ) {
    ansi(CLR_ERROR, ch);
    send_to_char("Nothing new happens.\n\r", ch);
    ansi(END, ch);
    return;
  }

  af.type      = SPELL_DETECT_INVISIBLE;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVISIBLE;

  affect_to_char(victim, &af);

  ansi(YELLOW, victim);
  send_to_char("Your eyes tingle.\n\r", victim);
  ansi(END, victim);
}



void spell_detect_magic(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  if ( affected_by_spell(victim, SPELL_DETECT_MAGIC) ) {
    ansi(CLR_ERROR, ch);
    send_to_char("Nothing new happens.\n\r", ch);
    ansi(END, ch);
    return;
  }

  af.type      = SPELL_DETECT_MAGIC;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;

  affect_to_char(victim, &af);
  ansi(YELLOW, victim); 
  send_to_char("Your eyes tingle with magical glow.\n\r", victim);
  ansi(END, victim);
}

void spell_breathwater(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  if ( affected_by_spell(victim, SPELL_BREATHWATER) ) {
    ansi(CLR_ERROR, ch);
    send_to_char("Nothing new happens.\n\r", ch);
    ansi(END, ch);
    return;
  }

  af.type      = SPELL_BREATHWATER;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_BREATHWATER;

  affect_to_char(victim, &af);
  ansi(CLR_ACTION, victim);
  send_to_char("You breath easier.\n\r", victim);
  ansi(END, victim);
  if (victim != ch) 
    ansi_act("$N starts to breath a little easier.",FALSE,ch,0,victim,TO_CHAR,CLR_ACTION);
}

void spell_darksight(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  if ( affected_by_spell(victim, SPELL_DARKSIGHT) ) {
      ansi(CLR_ERROR, ch);
      send_to_char("Nothing new happens.\n\r", ch); 
      ansi(END, ch);
      return;
  }

  af.type      = SPELL_DARKSIGHT;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DARKSIGHT;

  affect_to_char(victim, &af);
  ansi(CLR_ACTION, victim);
  send_to_char("You see easier in darkness.\n\r", victim);
  ansi(END, victim);
  ansi_act("$n's eyes begin to glow with an eerie light.",FALSE,victim,0,0,TO_ROOM,LGREEN);
}

void spell_regeneration(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  if ( affected_by_spell(victim, SPELL_REGENERATION) ) {
      ansi(CLR_ERROR, ch);
      send_to_char("Nothing new happens.\n\r", ch); 
      ansi(END, ch);
      return;
    }

  af.type      = SPELL_REGENERATION;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_REGENERATION;

  affect_to_char(victim, &af);
  ansi(CLR_ACTION, victim);
  send_to_char("You begin to regenerate.\n\r", victim);
  ansi(END, victim);
  ansi_act("$n glows white for a moment.",FALSE,victim,0,0,TO_ROOM,WHITE);
}

void spell_magic_resist(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  if ( affected_by_spell(victim, SPELL_MAGIC_RESIST) ) {
      ansi(CLR_ERROR, ch);
      send_to_char("Nothing new happens.\n\r", ch); 
      ansi(END, ch);
      return;
    }

  af.type      = SPELL_MAGIC_RESIST;
  af.duration  = level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_MAGIC_RESIST;

  affect_to_char(victim, &af);
  ansi(CLR_ACTION, victim);
  send_to_char("You begin to resist the effects of magic.\n\r", victim);
  ansi(END, victim);
  ansi_act("$n glows black for a moment.",FALSE,victim,0,0,TO_ROOM,DGRAY);
}


void spell_magic_immune(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;

  if ( affected_by_spell(victim, SPELL_MAGIC_IMMUNE) ) {
      ansi(CLR_ERROR, ch);
      send_to_char("Nothing new happens.\n\r", ch); 
      ansi(END, ch);
      return;
    }

  af.type      = SPELL_MAGIC_IMMUNE;
  af.duration  = level>>2;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_MAGIC_IMMUNE;

  affect_to_char(victim, &af);
  ansi(CLR_ACTION, victim);
  send_to_char("All magic near you now fails.\n\r", victim);
  ansi(END, victim);
  ansi_act("$n glows black for a moment.",FALSE,victim,0,0,TO_ROOM,DGRAY);
}

void spell_restoration(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  assert(victim);
  assert((level >= 0) && (level <= NPC_LEV));
  if (magic_fails(ch,victim)) return;
  if (IS_NPC(victim)) {
      ansi(CLR_ERROR, ch);
      send_to_char("You can restore players only.\n\r", ch); 
      ansi(END, ch);
      return;
    }

  GET_EXP(victim) = 
    MAXV(GET_EXP(victim),titles[GET_CLASS(victim)-1][GET_LEVEL(victim)].exp); 
  ansi(CLR_ACTION, victim);
  send_to_char("You have been magically restored.\n\r", victim);
  ansi(END, victim);
  ansi_act("$n is surrounded by white light for a moment.",FALSE,victim,0,0,TO_ROOM,WHITE);
}

void spell_detect_poison(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   assert(ch && (victim || obj));

  if (victim) {
    if (victim == ch)
      if (IS_AFFECTED(victim, AFF_POISON)) {
        ansi(CLR_DAM, ch);
        send_to_char("You can sense poison in your blood.\n\r", ch);
	ansi(END, ch);
      }
      else {
	ansi(CLR_ACTION, ch);
        send_to_char("You feel healthy.\n\r", ch);
	ansi(END, ch);
      }
    else
      if (IS_AFFECTED(victim, AFF_POISON)) {
        ansi_act("You sense that $E is poisoned.",FALSE,ch,0,victim,TO_CHAR,CLR_DAM);
      } else {
        ansi_act("You sense that $E is healthy.",FALSE,ch,0,victim,TO_CHAR,CLR_ACTION);
      }
  } else { /* It's an object */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      if (obj->obj_flags.value[3])
        ansi_act("Poisonous fumes are revealed.",FALSE, ch, 0, 0, TO_CHAR,CLR_DAM);
      else {
        ansi(CLR_ACTION, ch);
        send_to_char("It looks very delicious.\n\r", ch);
	ansi(END, ch);
      }
    }
  }
}


void spell_enchant_weapon(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int i;

   assert(ch && obj);
   assert(MAX_OBJ_AFFECT >= 2);

   if (GET_ITEM_TYPE(obj) != ITEM_WEAPON) {
     ansi(CLR_ERROR, ch);
     send_to_char("You can enchant weapons only.\n\r", ch); 
     ansi(END, ch);
     return;
   } else if (!IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
     ansi(CLR_ERROR, ch);
     send_to_char("You can enchant non-magical weapons only.\n\r", ch); 
     ansi(END, ch);
     return;
   } else {
     for (i=0; i < MAX_OBJ_AFFECT; i++)
       if (obj->affected[i].location != APPLY_NONE) {
	 ansi(CLR_ERROR, ch);
	 send_to_char("This weapon isn't suitable for enchantment.\n\r", ch); 
	 ansi(END, ch);
	 return;
       }
   }

   SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

   obj->affected[0].location = APPLY_HITROLL;
   obj->affected[0].modifier = 1 + (level / 10);
   
   obj->affected[1].location = APPLY_DAMROLL;
   obj->affected[1].modifier = 1 + (level / 15);
   
   if (IS_GOOD(ch)) {
     SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
     ansi_act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR,LBLUE);
   } else if (IS_EVIL(ch)) {
     SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
     ansi_act("$p glows red.",FALSE,ch,obj,0,TO_CHAR,LRED);
   } else 
     ansi_act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR,YELLOW);
 }



void spell_heal(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   assert(victim);
   if (magic_fails(ch,victim)) return;

   if (affected_by_spell(victim, SPELL_BLINDNESS)) 
     spell_cure_blind(level, ch, victim, obj);
   if (affected_by_spell(victim, SPELL_POISON)) 
     spell_remove_poison(level, ch, victim, obj);

   GET_HIT(victim) += 100;

   if (GET_HIT(victim) >= hit_limit(victim))
      GET_HIT(victim) = hit_limit(victim)-dice(1,4);

  update_pos( victim );

/*  ansi(CLR_ACTION, victim);
 */ send_to_char("A warm feeling fills your body.\n\r", victim);
/*  ansi(END, victim);
 */
    ansi_act("$n suddenly looks a lot better.",FALSE,victim,0,0,TO_ROOM,CLR_ACTION);
}


void spell_invisibility(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert((ch && obj) || victim);
  if (magic_fails(ch,victim)) return;

  if (obj) {
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
      ansi(CLR_ERROR, ch);
      send_to_char("It is already invisible.\n\r", ch); 
      ansi(END, ch);
      return;
    } else {
      ansi_act("$p turns invisible.",FALSE,ch,obj,0,TO_CHAR,CLR_ACTION);
      ansi_act("$p turns invisible.",TRUE,ch,obj,0,TO_ROOM,CLR_ACTION);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
    }
  } else {              /* Then it is a PC | NPC */
    if (magic_fails(ch,victim)) return;
    if (!(IS_SET(victim->specials.affected_by, AFF_INVISIBLE))) {

      ansi_act("$n slowly fades out of existence.", TRUE, victim,0,0,TO_ROOM,CLR_ACTION);
      ansi(CLR_ACTION, victim);
      send_to_char("You vanish.\n\r", victim);
      ansi(END, victim);

      af.type      = SPELL_INVISIBLE;
      af.duration  = 24;
      af.modifier  = -40;
      af.location  = APPLY_AC;
      af.bitvector = AFF_INVISIBLE;
      affect_to_char(victim, &af);
    } else 
      ansi_act("$N is already invisible.", TRUE,ch,0,victim,TO_CHAR,CLR_ERROR);
  }
}


void spell_improved_invis(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

   assert((ch) || victim);
   if (magic_fails(ch,victim)) return;
 
   /* Then it is a PC | NPC */
   if ((!affected_by_spell(victim, SPELL_INVISIBLE)) && (!affected_by_spell(victim, SPELL_IMPROVED_INVIS))) {

      ansi_act("$n slowly fades out of existence.", TRUE, victim,0,0,TO_ROOM,CLR_ACTION);
      ansi(CLR_ACTION, victim);
      send_to_char("You vanish.\n\r", victim);
      ansi(END, victim);

      af.type      = SPELL_IMPROVED_INVIS;
      af.duration  = level/3+3;
      af.modifier  = -40;
      af.location  = APPLY_AC;
      af.bitvector = AFF_INVISIBLE;
      affect_to_char(victim, &af);
   }
  else 
    ansi_act("$N is already invisible.", FALSE,ch,0,victim,TO_CHAR,CLR_ERROR); 
}



void spell_locate_object(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *i,*next;
  char name[256];
  char buf[MAX_STRING_LENGTH];
  int j;

  assert(ch);

  strcpy(name, fname(obj->name));
  j=level>>1;

  ansi(CLR_ACTION, ch);
  for (i=object_list; i && (j>0); i = next){
    next = i->next;
    if (isname(name, i->name)) {
      if(i->carried_by) {
        sprintf(buf,"%s carried by %s.\n\r",
          i->short_description,PERS(i->carried_by,ch));
        send_to_char(buf,ch);
      } else if (i->in_obj) {
        sprintf(buf,"%s in %s.\n\r",i->short_description,
          i->in_obj->short_description);
        send_to_char(buf,ch);
      } else {
        sprintf(buf,"%s in %s.\n\r",i->short_description,
          (i->in_room == NOWHERE ? "NOWHERE!?!(bug)" : world[i->in_room].name));
        send_to_char(buf,ch);
      }
      j--;
    }
  }

  ansi(CLR_ERROR, ch);
  if(j==0) 
    send_to_char("You are very confused.\n\r",ch);
  if(j==level>>1) 
    send_to_char("No such object.\n\r",ch);
  ansi(END, ch);
}


void spell_poison(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   struct affected_type af;

   assert(victim || obj);
 
   if (victim) {
    if(!saves_spell(victim, SAVING_PARA))
    {
      if (magic_fails(ch,victim)) return;
      af.type = SPELL_POISON;
      af.duration = level*2;
      af.modifier = -2;
      af.location = APPLY_STR;
      af.bitvector = AFF_POISON;

      affect_join(victim, &af, FALSE, FALSE);

      ansi(CLR_DAM, victim);
      send_to_char("You feel very sick.\n\r", victim);
      ansi(END, victim);
      ansi_act("You have poisoned $N!", FALSE, ch, 0, victim, TO_CHAR,CLR_ACTION);
    }

  } else { /* Object poison */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 1;
      ansi_act("You poison $o!", FALSE, ch, obj, victim, TO_CHAR,CLR_ACTION);
    } else
      ansi_act("You can't poison $o!", FALSE, ch, obj, victim, TO_CHAR,CLR_ERROR);
  }
}


void spell_protection_from_evil(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  if (magic_fails(ch,victim)) return;

  if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) ) {
    af.type      = SPELL_PROTECT_FROM_EVIL;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char(victim, &af);
    ansi(CLR_ACTION, victim);
    send_to_char("You have a righteous feeling!\n\r", victim);
    ansi(END, victim);
    ansi_act("$n briefly glows with a white light!", FALSE, victim, 0, 0, TO_ROOM,WHITE);
  }
}


void spell_remove_curse(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   assert(ch && (victim || obj));

   if (obj) {

      if ( IS_SET(obj->obj_flags.extra_flags, ITEM_EVIL) ||
           IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
         ansi_act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR,LBLUE);

         REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_EVIL);
         REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
      }
   } else {      /* Then it is a PC | NPC */
      if (magic_fails(ch,victim)) return;
      if (affected_by_spell(victim, SPELL_CURSE) ) {
	 ansi_act("$n briefly glows red, then blue.",FALSE,victim,0,0,TO_ROOM,PURPLE);
         ansi_act("You feel better.",FALSE,victim,0,0,TO_CHAR,CLR_ACTION);
         affect_from_char(victim, SPELL_CURSE);
      }
   }
}


void spell_remove_poison(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

   assert(ch && (victim || obj));

   if (victim) {
     if (magic_fails(ch,victim)) return;
     if(affected_by_spell(victim,SPELL_POISON)) {
       affect_from_char(victim,SPELL_POISON);
       ansi_act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR,CLR_ACTION);
       ansi_act("$n looks better.",FALSE,ch,0,victim,TO_ROOM,CLR_ACTION);
     }
   } else {
     if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
       obj->obj_flags.value[3] = 0;
       ansi_act("The $p steams briefly.",FALSE,ch,obj,0,TO_CHAR,CLR_ACTION);
     }
   }
}



void spell_sanctuary(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if (magic_fails(ch,victim)) return;
  if (!IS_AFFECTED(victim, AFF_SANCTUARY) ) {

    ansi_act("$n is surrounded by a bright aura.",TRUE,victim,0,0,TO_ROOM,CLR_SANC);
    ansi_act("You start glowing.",TRUE,victim,0,0,TO_CHAR,CLR_SANC);

    af.type      = SPELL_SANCTUARY;
    af.duration  = (level<=IMO_LEV) ? 2+level/10 : level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char(victim, &af);
  }
}



void spell_sleep(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  if (magic_fails(ch,victim)) return;

  if ( !saves_spell(victim, SAVING_SPELL) )
  {
    af.type      = SPELL_SLEEP;
    af.duration  = MAXV(24,4+(level>>1));
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    affect_join(victim, &af, FALSE, FALSE);

    if (GET_POS(victim)>POSITION_SLEEPING)
    {
      ansi_act("You feel very sleepy ..... zzzzzz",FALSE,victim,0,0,TO_CHAR,CLR_DAM);
      ansi_act("$n suddenly falls asleep.",TRUE,victim,0,0,TO_ROOM,CLR_DAM);
      GET_POS(victim)=POSITION_SLEEPING;
    }

    return;
  } else { /* made their save */
    if (!number(0,5))
      hit(ch,victim,TYPE_UNDEFINED);
      ansi_act("$N resists the effect of your magic!",TRUE,ch,0,victim,TO_CHAR,CLR_ERROR);
  }
}



void spell_strength(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  if (magic_fails(ch,victim)) return;

  ansi_act("You feel stronger.", FALSE, victim,0,0,TO_CHAR,CLR_ACTION);
  if (victim != ch)
    ansi_act("You magically strengthen $N.", FALSE, ch, 0, victim,TO_CHAR,CLR_ACTION);

  af.type      = SPELL_STRENGTH;
  af.duration  = level;
  af.modifier  = 1+(level>18);
  af.location  = APPLY_STR;
  af.bitvector = 0;

  affect_join(victim, &af, TRUE, FALSE);
}



void spell_ventriloquate(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   /* Not possible!! No argument! */
}



void spell_word_of_recall(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  extern int top_of_world;
  int loc_nr,location,was_in;
  bool found = FALSE;

  void do_look(struct char_data *ch, char *argument, int cmd);

  assert(victim);
  if (magic_fails(ch,victim)) return;

  if (IS_NPC(victim))
    return;

  if ((ch != victim) 
       && IS_SET(victim->specials.act, PLR_NOSUMMON)
       && saves_spell(victim, SAVING_SPELL) ) {
    ansi(CLR_ERROR, ch);
    send_to_char("You failed.\n\r",ch);
    ansi(END, ch);
    ansi_act("$n just attempted to summon you!",FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
    return;
  }

  /*  loc_nr = GET_HOME(ch); */

  loc_nr = 3001;
  for (location = 0; location <= top_of_world; location++)
    if (world[location].number == loc_nr) {
      found = TRUE;
      break;
    }

  if ((location == top_of_world) || !found)
  {
    ansi(CLR_ERROR, victim);
    send_to_char("You are completely lost.\n\r", victim);
    ansi(END, victim);
    return;
  }

   /* a location has been found. */
  was_in = ch->in_room;
  ansi_act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM,CLR_ACTION);
  char_from_room(victim);
  char_to_room(victim, location);
  ansi_act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM,CLR_ACTION);
  do_look(victim, "",15);
  if (IS_SET(zone_table[world[was_in].zone].flags, ZONE_TESTING))
    char_del_zone_objs(ch);

}


void spell_summon(sbyte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
  void do_look(struct char_data *ch, char *argument, int cmd);
  sh_int target;

  assert(ch && victim);
  if (magic_fails(ch,victim)) return;

  if (GET_LEVEL(victim) > MINV(41,level+10)) {
    ansi(CLR_ERROR, ch);
    send_to_char("They are too high in level for YOU to summon.\n\r",ch);
    ansi(END, ch);
    return;
  }

  if ((IS_SET(zone_table[world[victim->in_room].zone].flags, ZONE_NOENTER) 
       || IS_SET(zone_table[world[victim->in_room].zone].flags, ZONE_TESTING)
       || IS_SET(zone_table[world[ch->in_room].zone].flags, ZONE_NOENTER) 
       || IS_SET(zone_table[world[ch->in_room].zone].flags, ZONE_TESTING)) 
      && (GET_LEVEL(ch) < IMO_LEV2)) {
    ansi(CLR_ERROR, ch);
    send_to_char("A strange power surge interferes!\n\r",ch);
    ansi(END, ch);
    return;
  }

  if (IS_SET(victim->specials.act, PLR_NOSUMMON)
      && saves_spell(victim, SAVING_SPELL) ) {
    ansi(CLR_ERROR, ch);
    send_to_char("You failed.\n\r",ch);
    ansi(END, ch);
    ansi_act("$n just attempted to summon you!",
	     FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
    return;
   }

  if (IS_NPC(victim) && saves_spell(victim, SAVING_SPELL) ) {
    ansi(CLR_ERROR, ch);
    send_to_char("You failed.\n\r", ch);
    ansi(END, ch);
    return;
  }

  ansi_act("$n disappears suddenly.",TRUE,victim,0,0,TO_ROOM,CLR_ACTION);
  
  target = ch->in_room;
  char_from_room(victim);
  char_to_room(victim,target);
  
  ansi_act("$n arrives suddenly.",TRUE,victim,0,0,TO_ROOM,CLR_ACTION);
  ansi_act("$n has summoned you!",FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
  do_look(victim,"",15);
}


void spell_succor(sbyte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
  void do_look(struct char_data *ch, char *argument, int cmd);
  sh_int target;

  assert(ch && victim);
  if (magic_fails(ch,victim)) return;
 
  if (GET_LEVEL(victim) > MINV(40,level+10)) {
    ansi(CLR_ERROR, ch);
    send_to_char("They are too high in level for YOU to succor.\n\r",ch);
    ansi(END, ch);
    return;
  }

  if ((IS_SET(zone_table[world[victim->in_room].zone].flags, ZONE_NOENTER) 
       || IS_SET(zone_table[world[victim->in_room].zone].flags, ZONE_TESTING)
       || IS_SET(zone_table[world[ch->in_room].zone].flags, ZONE_NOENTER) 
       || IS_SET(zone_table[world[ch->in_room].zone].flags, ZONE_TESTING)) 
      && (GET_LEVEL(ch) < IMO_LEV2)) {
    ansi(CLR_ERROR, ch);
    send_to_char("A strange power surge interferes!\n\r",ch);
    ansi(END, ch);
    return;
  }

  if (IS_NPC(victim) || 
      (IS_SET(victim->specials.act, PLR_NOSUMMON)
       && saves_spell(victim, SAVING_SPELL)) ) {
    ansi(CLR_ERROR, ch);
    send_to_char("You failed.\n\r",ch);
    ansi(END, ch);
    ansi_act("$n just attempted to succor you!",
	     FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
    return;
  }
  
  if (IS_NPC(victim) && saves_spell(victim, SAVING_SPELL) ) {
    ansi(CLR_ERROR, ch);
    send_to_char("You failed.\n\r", ch);
    ansi(END, ch);
    return;
  }

  ansi_act("$n disappears suddenly.",TRUE,ch,0,0,TO_ROOM,CLR_ACTION);

  target = victim->in_room;
  char_from_room(ch);
  char_to_room(ch,target);
  
  ansi_act("$n arrives suddenly.",TRUE,ch,0,0,TO_ROOM,CLR_ACTION);
  ansi_act("$n has succored you!",FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
  do_look(ch,"",15);
}


void spell_charm_person(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  void add_follower(struct char_data *ch, struct char_data *leader);
  bool circle_follow(struct char_data *ch, struct char_data *victim);
  void stop_follower(struct char_data *ch);

  assert(ch && victim);
  if (magic_fails(ch,victim)) return;

  /* By testing for IS_AFFECTED we avoid ei. Mordenkainens sword to be */
  /* abel to be "recharmed" with duration                              */

   if (victim == ch) {
      ansi(CLR_ACTION, ch);
      send_to_char("You like yourself even better!\n\r", ch);
      ansi(END, ch);
      return;
   }
   if (((IS_NPC(victim))&&(IS_SET(victim->specials.act, ACT_NOCHARM)))
   ||(level < GET_LEVEL(victim))) {
     ansi_act("$N prefers to be $S own master.", 
	      FALSE, ch, 0, victim, TO_CHAR, CLR_ERROR);
     if (!number(0,5))
       hit(ch,victim,TYPE_UNDEFINED);
     return;
   }

   if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
     if (circle_follow(victim, ch)) {
       ansi(CLR_ERROR, ch);
       send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
       ansi(END, ch);
       return;
     }

     if (saves_spell(victim, SAVING_SPELL)) {
       ansi_act("$N prefers to be $S own master.", 
		FALSE, ch, 0, victim, TO_CHAR, CLR_ERROR);
       if (!number(0,5))
         hit(ch,victim,TYPE_UNDEFINED);
       return;
     }

     if (victim->master)
       stop_follower(victim);

     add_follower(victim, ch);

     af.type      = SPELL_CHARM_PERSON;

     if (GET_INT(victim))
       af.duration  = 24*18/GET_INT(victim);
     else
       af.duration  = 24*18;

     af.modifier  = 0;
     af.location  = 0;
     af.bitvector = AFF_CHARM;
     affect_to_char(victim, &af);

     ansi_act("Isn't $n just such a nice fellow?",FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
   }
}



void spell_sense_life(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
  if (magic_fails(ch,victim)) return;

  if (!affected_by_spell(victim, SPELL_SENSE_LIFE)) {
    ansi(CLR_ACTION, ch);
    send_to_char("Your feel your awareness improve.\n\r", ch);
    ansi(END, ch);

    af.type      = SPELL_SENSE_LIFE;
    af.duration  = 5*level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SENSE_LIFE;
    affect_to_char(victim, &af);
  }
}


void spell_identify(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  char buf[256], buf2[256];
  int i;
  bool found;

  struct time_info_data age(struct char_data *ch);

  /* Spell Names */
   extern const char *spells[];

  assert(ch && (obj || victim));

  if (obj) {
    ansi(CLR_ACTION, ch);
    send_to_char("You feel informed:\n\r", ch);

    sprintf(buf, "Object '%s', Item type: ", obj->name);
    sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
    strcat(buf,buf2); strcat(buf,"\n\r");
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit(obj->obj_flags.bitvector,affected_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf, ch);
    }

    send_to_char("Item is: ", ch);
    sprintbit(obj->obj_flags.extra_flags,extra_bits,buf);
    strcat(buf,"\n\r");
    send_to_char(buf,ch);

    sprintf(buf,"Weight: %d, Value: %d\n\r",
      obj->obj_flags.weight, obj->obj_flags.cost);
    send_to_char(buf, ch);

    switch (GET_ITEM_TYPE(obj)) {

      case ITEM_SCROLL : 
      case ITEM_POTION :
        sprintf(buf, "Level %d spells of:\n\r",   obj->obj_flags.value[0]);
        send_to_char(buf, ch);
        if (obj->obj_flags.value[1] >= 1) {
          sprinttype(obj->obj_flags.value[1]-1,spells,buf);
          strcat(buf,"\n\r");
          send_to_char(buf, ch);
        }
        if (obj->obj_flags.value[2] >= 1) {
          sprinttype(obj->obj_flags.value[2]-1,spells,buf);
          strcat(buf,"\n\r");
          send_to_char(buf, ch);
        }
        if (obj->obj_flags.value[3] >= 1) {
          sprinttype(obj->obj_flags.value[3]-1,spells,buf);
          strcat(buf,"\n\r");
          send_to_char(buf, ch);
        }
        break;

      case ITEM_WAND : 
      case ITEM_STAFF : 
        sprintf(buf, "Has %d charges, with %d charges left.\n\r",
          obj->obj_flags.value[1],
          obj->obj_flags.value[2]);
        send_to_char(buf, ch);
          
        sprintf(buf, "Level %d spell of:\n\r", obj->obj_flags.value[0]);
        send_to_char(buf, ch);
          
        if (obj->obj_flags.value[3] >= 1) {
          sprinttype(obj->obj_flags.value[3]-1,spells,buf);
          strcat(buf,"\n\r");
          send_to_char(buf, ch);
        }
        break;

      case ITEM_WEAPON :
        send_to_char("Weapons bits: ",ch);
        sprintbit(obj->obj_flags.value[0],weapon_bits,buf);
        strcat(buf,"\n\r");
        send_to_char(buf,ch);
        sprintf(buf, "Damage Dice is '%dD%d'\n\r",
          obj->obj_flags.value[1],
          obj->obj_flags.value[2]);
        send_to_char(buf, ch);
        break;

      case ITEM_ARMOR :
        sprintf(buf, "AC-apply is %d\n\r",
          obj->obj_flags.value[0]);
        send_to_char(buf, ch);
        break;

    }

    found = FALSE;

    for (i=0;i<MAX_OBJ_AFFECT;i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
         (obj->affected[i].modifier != 0)) {
         if (!found) {
            send_to_char("Can affect you as :\n\r", ch);
            found = TRUE;
         }
   
         sprinttype(obj->affected[i].location,apply_types,buf2);
         sprintf(buf,"    Affects : %s By %d\n\r", buf2,obj->affected[i].modifier);
         send_to_char(buf, ch);
      }
    }

  } else {       /* victim */

    if (!IS_NPC(victim)) {
      sprintf(buf,"%d Years,  %d Months,  %d Days,  %d Hours old.\n\r",
        age(victim).year, age(victim).month,
        age(victim).day, age(victim).hours);
      send_to_char(buf,ch);
/*
      sprintf(buf,"Height %dcm  Weight %dpounds \n\r",
        GET_HEIGHT(victim), GET_WEIGHT(victim));
      send_to_char(buf,ch);
*/
      sprintf(buf,"Str %d/%d,  Int %d,  Wis %d,  Dex %d,  Con %d\n\r",
        GET_STR(victim), GET_ADD(victim),
        GET_INT(victim),
        GET_WIS(victim),
        GET_DEX(victim),
        GET_CON(victim) );
      send_to_char(buf,ch);
    } else { /* is a mob */
      ansi(CLR_ERROR, ch);
      send_to_char("You learn nothing new.\n\r", ch);
    }
  }
     ansi(END, ch);
}

void spell_fire_breath(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int dam;
   struct obj_data *burn;

   assert(victim && ch);
   assert((level >= 1) && (level <= NPC_LEV)); 

   if (IS_NPC(ch)) 
     dam = dice(level,10) + 3*level;
   else
     dam = dice(level>>1,10)+2*level;

   if ( saves_spell(victim, SAVING_BREATH) )
      dam >>= 1;

   /* And now for the damage on inventory */
   if(number(0,50)>GET_LEVEL(ch)){
      if (!saves_spell(victim, SAVING_BREATH) ){
         for(burn=victim->carrying ; 
            burn && 
            (burn->obj_flags.type_flag!=ITEM_SCROLL) && 
            (burn->obj_flags.type_flag!=ITEM_WAND) &&
            (burn->obj_flags.type_flag!=ITEM_STAFF) &&
            (burn->obj_flags.type_flag!=ITEM_NOTE) &&
            (number(0,2)==0) ;
             burn=burn->next_content);
         if(burn){ /* if we found one thing to burn its gone */
	       ansi_act("$o burns!",0,victim,burn,0,TO_CHAR,CLR_DAM);
               extract_obj(burn);
         }
      }
   } 
   DAMAGE(ch, victim, dam, SPELL_FIRE_BREATH);
}


void spell_frost_breath(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int dam;
   struct obj_data *frozen;
   struct affected_type af;

   assert(victim && ch);
   assert((level >= 1) && (level <= NPC_LEV)); 

   if ( !saves_spell(victim, SAVING_BREATH) ) {
       af.type      = SPELL_CHILL_TOUCH;
       af.duration  = 1;
       af.modifier  = -1;
       af.location  = APPLY_STR;
       af.bitvector = 0;
       affect_join(victim, &af, TRUE, FALSE);
       send_to_char("The intense cold saps your strength.\n\r", victim);
   }
   if (IS_NPC(ch))  
     dam = dice(level,8) + 3*level;
   else 
     dam = dice(MINV(level>>1,6),6)+MINV((level+1)>>1,12);

   if ( saves_spell(victim, SAVING_BREATH) )
      dam >>= 1;
   DAMAGE(ch, victim, dam, SPELL_FROST_BREATH);

   /* And now for the damage on inventory */
   if(number(0,50)<GET_LEVEL(ch))
   {
      if (!saves_spell(victim, SAVING_BREATH) )
      {
         for(frozen=victim->carrying ; 
            frozen && (frozen->obj_flags.type_flag!=ITEM_DRINKCON) && 
            (frozen->obj_flags.type_flag!=ITEM_FOOD) &&
            (frozen->obj_flags.type_flag!=ITEM_POTION) && (number(0,2)==0) ;
             frozen=frozen->next_content); 
         if(frozen)
         {
            ansi_act("$o breaks.",0,victim,frozen,0,TO_CHAR,CLR_DAM);
            extract_obj(frozen);
         }
      }
   }
}


void spell_acid_breath(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int dam;
   int damaged;
   int apply_ac(struct char_data *ch, int eq_pos);
   
   assert(victim && ch);
   assert((level >= 1) && (level <= NPC_LEV)); 

   if (IS_NPC(ch)) 
     dam = dice(level,6) + 3*level;
   else
     dam = dice(MINV(level>>1,10),6)+10;

   if ( saves_spell(victim, SAVING_BREATH) )
      dam >>= 1;

   /* And now for the damage on equipment */
   if(number(0,50)<GET_LEVEL(ch))
   {
      if (!saves_spell(victim, SAVING_BREATH) )
      {
         for(damaged = 0; damaged<MAX_WEAR &&
            (victim->equipment[damaged]) &&
            (victim->equipment[damaged]->obj_flags.type_flag==ITEM_ARMOR) &&
            (victim->equipment[damaged]->obj_flags.value[0]>0) && 
                        (number(0,2)==0) ; damaged++)  
         {
            ansi_act("$o is damaged.",0,victim,victim->equipment[damaged],0,TO_CHAR,CLR_DAM);
            GET_AC(victim)-=apply_ac(victim,damaged);
            ch->equipment[damaged]->obj_flags.value[0]-=number(1,7);
            GET_AC(victim)+=apply_ac(victim,damaged);
            ch->equipment[damaged]->obj_flags.cost = 0;
         }
      }
   }
   DAMAGE(ch, victim, dam, SPELL_ACID_BREATH);
}


void spell_gas_breath(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   struct affected_type af;
   int dam;

   assert(victim && ch);
   assert((level >= 1) && (level <= NPC_LEV)); 

   if (IS_NPC(ch)) 
     dam = dice(level,4) + 3*level;
   else 
     dam = dice(MINV(level>>1,4),3)+MINV(level>>1,4);

   if ( saves_spell(victim, SAVING_BREATH) )
      dam >>= 1;

   if(!saves_spell(victim, SAVING_PARA)) {
      af.type = SPELL_POISON;
      af.duration = level*2;
      af.modifier = -2;
      af.location = APPLY_STR;
      af.bitvector = AFF_POISON;

      affect_join(victim, &af, FALSE, FALSE);

      ansi_act("$n starts choking on the toxic fumes!!", TRUE, victim, 0, 0,TO_ROOM,CLR_DAM);
      ansi(CLR_ERROR, victim);
      send_to_char("You feel very sick.\n\r", victim);
      ansi(END, victim);
   }
   if(!saves_spell(victim, SAVING_BREATH)&&(!affected_by_spell(victim, SPELL_BLINDNESS))) {
      af.type      = SPELL_BLINDNESS;
      af.location  = APPLY_HITROLL;
      af.modifier  = -4;  /* Make hitroll worse */
      af.duration  = 3;
      af.bitvector = AFF_BLIND;
      affect_to_char(victim, &af);

      af.location = APPLY_AC;
      af.modifier = +40; /* Make AC Worse! */
      affect_to_char(victim, &af);

      ansi_act("$n seems to have been blinded by the gas!", TRUE, victim, 0, 0, TO_ROOM,CLR_DAM);
      ansi(CLR_DAM, victim);
      send_to_char("You have been blinded!\n\r", victim);
      ansi(END, victim);
   }
   DAMAGE(ch, victim, dam, SPELL_GAS_BREATH);
}


void spell_lightning_breath(sbyte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   int dam;

   assert(victim && ch);
   assert((level >= 1) && (level <= NPC_LEV)); 

   if (IS_NPC(ch)) 
     dam = dice(level,5) + 3*level; 
   else 
     dam = dice(MINV(level>>1,20),6)+20;

   if ( saves_spell(victim, SAVING_BREATH) )
      dam >>= 1;
   DAMAGE(ch, victim, dam, SPELL_LIGHTNING_BREATH);
}


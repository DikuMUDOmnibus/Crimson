/* ************************************************************************
*  File: fight.c , Combat module.                         Part of DIKUMUD *
*  Usage: Combat system and messages.                                     *
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
#include "constants.h" /* constants related to #classes and level limits */
#include "spells.h"
#include "ansi.h"
#include "act.h"

#include "fight.h"

/* Structures */
struct char_data *combat_list = 0;      /* head of l-list of fighting chars   */
struct char_data *combat_next_dude = 0; /* Next dude global trick           */


/* External procedures */
void stop_follower(struct char_data *ch);

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit",   "hits"},             /* TYPE_HIT      */
  {"pound", "pounds"},           /* TYPE_BLUDGEON */
  {"pierce", "pierces"},         /* TYPE_PIERCE   */
  {"slash", "slashes"},          /* TYPE_SLASH    */
  {"whip", "whips"},             /* TYPE_WHIP     */
  {"claw", "claws"},             /* TYPE_CLAW     */
  {"bite", "bites"},             /* TYPE_BITE     */
  {"sting", "stings"},           /* TYPE_STING    */
  {"crush", "crushes"}           /* TYPE_CRUSH    */
};




/* The Fight related routines */

void appear(struct char_data *ch)
{
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  if (!(affected_by_spell(ch, SPELL_IMPROVED_INVIS))) {
    REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
    act("$n slowly fade into existence.", FALSE, ch,0,0,TO_ROOM);
  }
}



void load_messages(void)
{
   FILE *f1;
   int i,type;
   struct message_type *messages;
   char chk[100];

   if (!(f1 = fopen(MESS_FILE, "r"))){
      perror("read messages");
      exit(0);
   }

   for (i = 0; i < MAX_MESSAGES; i++)
   { 
      fight_messages[i].a_type = 0;
      fight_messages[i].number_of_attacks=0;
      fight_messages[i].msg = 0;
   }

   fscanf(f1, " %s \n", chk);

   while(*chk == 'M')
   {
      fscanf(f1," %d\n", &type);
      for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type!=type) &&
         (fight_messages[i].a_type); i++);
      if(i>=MAX_MESSAGES){
         log("Too many combat messages.");
         exit(0);
      }

      CREATE(messages,struct message_type,1);
      fight_messages[i].number_of_attacks++;
      fight_messages[i].a_type=type;
      messages->next=fight_messages[i].msg;
      fight_messages[i].msg=messages;

      messages->die_msg.attacker_msg      = fread_string(f1);
      messages->die_msg.victim_msg        = fread_string(f1);
      messages->die_msg.room_msg          = fread_string(f1);
      messages->miss_msg.attacker_msg     = fread_string(f1);
      messages->miss_msg.victim_msg       = fread_string(f1);
      messages->miss_msg.room_msg         = fread_string(f1);
      messages->hit_msg.attacker_msg      = fread_string(f1);
      messages->hit_msg.victim_msg        = fread_string(f1);
      messages->hit_msg.room_msg          = fread_string(f1);
      messages->god_msg.attacker_msg      = fread_string(f1);
      messages->god_msg.victim_msg        = fread_string(f1);
      messages->god_msg.room_msg          = fread_string(f1);
      fscanf(f1, " %s \n", chk);
   }

   fclose(f1);
}


void update_pos( struct char_data *victim )
{

   if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED)) return;
   else if (GET_HIT(victim) > 0 ) GET_POS(victim) = POSITION_STANDING;
   else if (GET_HIT(victim) <= -11) GET_POS(victim) = POSITION_DEAD;
   else if (GET_HIT(victim) <= -6) GET_POS(victim) = POSITION_MORTALLYW;
   else if (GET_HIT(victim) <= -3) GET_POS(victim) = POSITION_INCAP;
   else GET_POS(victim) = POSITION_STUNNED;

}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
   assert(!ch->specials.fighting);

   ch->next_fighting = combat_list;
   combat_list = ch;
   if(IS_AFFECTED(ch,AFF_SLEEP))
      affect_from_char(ch,SPELL_SLEEP);
   if (!IS_NPC(ch) && vict->specials.fighting != ch) { /* mark if not self-defense */
      SET_BIT(ch->specials.act, PLR_ATTACKER);
      if (GET_LEVEL(ch) < IMO_LEV3)
        REMOVE_BIT(ch->specials.act, PLR_NOHASSLE);
   }
   ch->specials.fighting = vict;
   GET_POS(ch) = POSITION_FIGHTING;
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
   struct char_data *tmp;

   if (!ch->specials.fighting) {
     log("stop_fighting: (fight.c) ch->specials.fighting == NUL");
     return;
   }
   
   if (!IS_NPC(ch)) {
      REMOVE_BIT(ch->specials.act, PLR_ATTACKER);
   }

   if (ch == combat_next_dude)
      combat_next_dude = ch->next_fighting;

   if (combat_list == ch)
      combat_list = ch->next_fighting;
   else
   {
      for (tmp = combat_list; tmp && (tmp->next_fighting != ch); 
         tmp = tmp->next_fighting);
      if (!tmp) {
         log("Char fighting not found Error (fight.c, stop_fighting)");
         abort();
      }
      tmp->next_fighting = ch->next_fighting;
   }

   ch->next_fighting = 0;
   ch->specials.fighting = 0;
   GET_POS(ch) = POSITION_STANDING;
   update_pos(ch);
}



#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 10

void make_corpse(struct char_data *ch)
{
   struct obj_data *corpse, *o;
   struct obj_data *money; 
   char buf[MAX_STRING_LENGTH];
   int i;

/* char *strdup(char *source); in strings.h */
   struct obj_data *create_money( int amount );

   CREATE(corpse, struct obj_data, 1);
   clear_object(corpse);
   
   corpse->item_number = NOWHERE;
   corpse->in_room = NOWHERE;
   corpse->name = str_alloc("corpse");

   sprintf(buf, "Corpse of %s lies here rotting.", 
     (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
   corpse->description = str_alloc(buf);

   sprintf(buf, "Corpse of %s",
     (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
   corpse->short_description = str_alloc(buf);

   corpse->contains = ch->carrying;
   if (GET_GOLD(ch) > 0 && (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)))
   {
      money = create_money(GET_GOLD(ch));
      GET_GOLD(ch)=0;
      obj_to_obj(money,corpse);
   }

   corpse->obj_flags.type_flag = ITEM_CONTAINER;
   corpse->obj_flags.wear_flags = ITEM_TAKE;
   corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
   corpse->obj_flags.value[3] = 1; /* corpse identifyer */
   corpse->obj_flags.weight = GET_WEIGHT(ch)+IS_CARRYING_W(ch);
   corpse->obj_flags.cost_per_day = 100000;
   if (IS_NPC(ch))
      corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
   else
      corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;

   for (i=0; i<MAX_WEAR; i++)
      if (ch->equipment[i])
         obj_to_obj(unequip_char(ch, i), corpse);

   ch->carrying = 0;
   IS_CARRYING_N(ch) = 0;
   IS_CARRYING_W(ch) = 0;

   corpse->next = object_list;
   object_list = corpse;

   for(o = corpse->contains; o; o->in_obj = corpse, o = o->next_content);
   object_list_new_owner(corpse, 0);

   obj_to_room(corpse, ch->in_room);
}

/* When ch kills victim */
void change_alignment(struct char_data *ch, struct char_data *victim)
{
   int align_vict, abs_align, abs_diff, diff, sign = 1;

   if (GET_ALIGNMENT(ch)<0) { /* make all align's positive */
      sign = -1;
      GET_ALIGNMENT(ch) *= -1;
      align_vict = GET_ALIGNMENT(victim)*-1;
   } else
      align_vict = GET_ALIGNMENT(victim);
   abs_align = GET_ALIGNMENT(ch);
   diff = GET_ALIGNMENT(ch) - align_vict;
   abs_diff = MAXV(diff, -1*diff);

   if (align_vict > GET_ALIGNMENT(ch)) { /* killed someone more good than us */
      if (abs_diff > 550) /* wow killed someone *ALOT* more good than us */
         GET_ALIGNMENT(ch) -= (number(abs_align >> 1, abs_align)+50);
      else
         GET_ALIGNMENT(ch) = number(abs_align >> 1, abs_align);
   } else if (abs_diff > 550) /* make us more good */
         GET_ALIGNMENT(ch) += number((abs_diff-500) >> 4, (abs_diff-500) >> 2) +5; 
      else
         GET_ALIGNMENT(ch) += (number(abs_align >> 1, abs_align)+50);


   if (GET_ALIGNMENT(ch)>1000)
      GET_ALIGNMENT(ch) = 1000;
   if (GET_ALIGNMENT(ch)<-1000)
      GET_ALIGNMENT(ch) = -1000; /* pretty unlikely */
   GET_ALIGNMENT(ch) *= sign; /* restore sign of alignment */
 }


void death_cry(struct char_data *ch)
{
   int door, was_in;
   void del_char_objs(struct char_data *ch);

/* This is redundant they must drop link, quit or rent out */
/* we should create file in case crash occurs between corpse retrieval^save */
   del_char_objs(ch);  

   ansi_act("Your blood freezes as you hear $ns death cry.", FALSE, ch,0,0,TO_ROOM,CLR_DAM);
   was_in = ch->in_room;

   for (door = 0; door <= 5; door++) {
      if (CAN_GO(ch, door))   {
         ch->in_room = world[was_in].dir_option[door]->to_room;
         ansi_act("Your blood freezes as you hear someone's death cry.",FALSE,ch,0,0,TO_ROOM,CLR_DAM);
         ch->in_room = was_in;
      }
   }
}



void raw_kill(struct char_data *ch)
{
   if (ch->specials.fighting)
      stop_fighting(ch);

   death_cry(ch);

   make_corpse(ch);
   extract_char(ch);
}


void die(struct char_data *ch)
{
  long lose;
  char buf[128];
  
  REMOVE_BIT(ch->specials.act, PLR_PKILLER);
  REMOVE_BIT(ch->specials.act, PLR_ENFORCER);
  switch (GET_CLASS(ch)){
  case CLASS_KAI:
  case CLASS_EKNIGHT:
    GET_ALIGNMENT(ch) = 1000;
    break;
    
  case CLASS_DRAKKHAR:
  case CLASS_DKNIGHT:
    GET_ALIGNMENT(ch) = -1000;
    break;
    
  default:
    GET_ALIGNMENT(ch) = 0;
    break;
  } /* switch */  
  if ((GET_LEVEL(ch) > 5) && (GET_LEVEL(ch) < IMO_LEV)) {
    lose = MINV(GET_EXP(ch)>>4,25000*GET_LEVEL(ch));
    GET_EXP(ch) -= lose;
    sprintf(buf, "You lose %ld experience points!\n\r" , lose);
    if (ch->desc) {
      ansi(CLR_DAM, ch);
      send_to_char(buf, ch);   
      ansi(END, ch);
    }
  }
  else if (GET_LEVEL(ch) == IMO_LEV) {
    ch->points.max_hit -= 5;
    ch->points.max_mana -= 5;
    ch->points.max_move -= 5;
    if (number(0,10) == 0) {
      switch(number(0,3)) {
      case 0:
	ch->abilities.str--;
	ch->tmpabilities.str--;
	ansi_act("You lost one STRENGTH point! Ouch!!!", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
	break;
      case 1:
	ch->abilities.intel--;
	ch->tmpabilities.intel--;
	ansi_act("You lost one INTELLIGENCE point! Ouch!!!", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
	break;
      case 2:
	ch->abilities.wis--;
	ch->tmpabilities.wis--;
	ansi_act("You lost one WISDOM point! Ouch!!!", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
	break;
      case 3:
	ch->abilities.dex--;
	ch->tmpabilities.dex--;
	ansi_act("You lost one DEXTERITY point! Ouch!!!", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
	break;
      default:
	ch->abilities.con--;
	ch->tmpabilities.con--;
	ansi_act("You lost one CONSTITUTION point! Ouch!!!", FALSE, ch, 0, 0, TO_CHAR,CLR_ERROR);
	break;
      }
    }
    GET_EXP(ch) = MAXV(0, GET_EXP(ch));
  }
  raw_kill(ch);
}


void group_gain(struct char_data *ch, struct char_data *victim)
{
   char buf[256];
   int no_members, share, your_share,no_levels, max_lev=0;
   struct char_data *k;
   struct follow_type *f;

   if (!(k=ch->master))
      k = ch;

   if (IS_AFFECTED(k, AFF_GROUP) &&
      (k->in_room == ch->in_room))
      {
      no_members = 1;
      no_levels = GET_LEVEL(k);
      max_lev = MAXV(max_lev,GET_LEVEL(k));
      }
   else
      {
      no_members = 0;
      no_levels = 0;
      }

   for (f=k->followers; f; f=f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
         (f->follower->in_room == ch->in_room))
         {
         no_members++;
         no_levels += GET_LEVEL(f->follower);
         max_lev = MAXV(max_lev,GET_LEVEL(f->follower));
         }

   if (no_members > 1) /* be generous to groups */
      share = GET_EXP(victim) * 2;
   else if (no_members == 1) /* same as individ xp */
      share = GET_EXP(victim);
   else
      share = 0;

   if (!IS_NPC(victim)) /* handle a PC victim */
      share = 500*GET_LEVEL(victim); /* never called for PC victim? */

   if (IS_AFFECTED(k, AFF_GROUP) &&
      (k->in_room == ch->in_room)) {
      your_share = share * GET_LEVEL(k) / no_levels;
      /* scale xp according to rel. levels */

      your_share += (your_share*MINV(8, (GET_LEVEL(victim) - max_lev)))/8; 

      your_share = MAXV(1, your_share);
      sprintf(buf, "You receive your share of %d experience points." , your_share);
      ansi_act(buf, FALSE, k, 0, 0, TO_CHAR,CLR_ACTION);
      gain_exp(k, your_share);
      change_alignment(k, victim);
   }

   for (f=k->followers; f; f=f->next) {
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
         (f->follower->in_room == ch->in_room)) {
         your_share = share * GET_LEVEL(f->follower) / no_levels;
         your_share += (your_share*MINV(8, (GET_LEVEL(victim) - max_lev)))/8;
         your_share = MAXV(1, your_share);
         sprintf(buf, "You receive your share of %d experience points." , your_share);
         ansi_act(buf, FALSE, f->follower,0,0,TO_CHAR,CLR_ACTION);
         gain_exp(f->follower, your_share);
         change_alignment(f->follower, victim);
      }
   }
}

char *replace_string(char *str, char *weapon)
{
   static char buf[256];
   char *cp;
 
   cp = buf;

   for (; *str; str++) {
      if (*str == '#') {
         switch(*(++str)) {
            case 'W' : 
               for (; *weapon; *(cp++) = *(weapon++));
               break;
            default :
              *(cp++) = '#';
               break;
         }
      } else {
         *(cp++) = *str;
      }

      *cp = 0;
   } /* For */

   return(buf);
}


void dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type)
{
   struct obj_data *wield;
   char *buf;

   static struct dam_weapon_type {
      char *to_room;
      char *to_char;
      char *to_victim;
   } dam_weapons[] = {

    {"$n misses $N with $s #W.",                           /*    0    */
     "You miss $N with your #W.",
     "$n misses you with $s #W." },

      {"$n tickles $N with $s #W.",                          /*  1.. 2  */
       "You tickle $N as you #W $M.",
       "$n tickles you as $e #W you." },

      {"$n barely #W $N.",                                   /*  3.. 4  */
       "You barely #W $N.",
       "$n barely #W you."},

   {"$n #W $N.",                                          /*  5.. 6  */
       "You #W $N.",
       "$n #W you."}, 

    {"$n #W $N hard.",                                     /*  7..10  */
     "You #W $N hard.",
        "$n #W you hard."},

    {"$n #W $N very hard.",                                /* 11..14  */
     "You #W $N very hard.",
     "$n #W you very hard."},

    {"$n #W $N extremely hard.",                          /* 15..20  */
     "You #W $N extremely hard.",
     "$n #W you extremely hard."},

    {"$n massacres $N with $s #W.",                     /*  21..30    */
     "You massacre $N with your #W.", 
     "$n massacres you with $s #W."},

    {"$n annihilates $N with $s deadly #W.",             /* 31..40 */
     "You annihilate $N with your deadly #W.",
     "$n annihilates you with $s deadly #W."},

    {"$n obliterates $N with $s deadly #W.",             /* 41..50 */
     "You obliterate $N with your deadly #W.",
     "$n obliterates you with $s deadly #W."},

    {"$n demolishes $N with $s deadly #W.",             /* 51..75 */
     "You demolish $N with your deadly #W.",
     "$n demolishes you with $s deadly #W."},

    {"$n rends $N asunder with $s deadly #W.",
     "You rend $N asunder with your deadly #W.",
     "$n rends you asunder with $s deadly #W."}
                  /* JUST GUESS HOW MUCH */
   };

   w_type -= TYPE_HIT;   /* Change to base of table with text */
   wield = ch->equipment[WIELD];

   if (dam == 0) {
      buf = replace_string(dam_weapons[0].to_room, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[0].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_FIGHT);
      buf = replace_string(dam_weapons[0].to_victim, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_FIGHT);
   } else if (dam <= 4) {
      buf = replace_string(dam_weapons[1].to_room, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[1].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string(dam_weapons[1].to_victim, attack_hit_text[w_type].plural);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 8) {
      buf = replace_string(dam_weapons[2].to_room, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[2].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string(dam_weapons[2].to_victim, attack_hit_text[w_type].plural);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 12) {
      buf = replace_string(dam_weapons[3].to_room, attack_hit_text[w_type].plural);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[3].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string(dam_weapons[3].to_victim, attack_hit_text[w_type].plural);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 20) {
      buf = replace_string(dam_weapons[4].to_room, attack_hit_text[w_type].plural);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[4].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string(dam_weapons[4].to_victim, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 30) {
      buf = replace_string(dam_weapons[5].to_room, attack_hit_text[w_type].plural);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[5].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string(dam_weapons[5].to_victim, attack_hit_text[w_type].plural);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 40) {
      buf = replace_string(dam_weapons[6].to_room, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[6].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string(dam_weapons[6].to_victim, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 60) {
      buf = replace_string(dam_weapons[7].to_room, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[7].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string(dam_weapons[7].to_victim, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 80) {
      buf = replace_string(dam_weapons[8].to_room, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[8].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string(dam_weapons[8].to_victim, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 100) {
      buf = replace_string(dam_weapons[9].to_room, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[9].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string(dam_weapons[9].to_victim, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 150) {
      buf = replace_string(dam_weapons[10].to_room, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[10].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string(dam_weapons[10].to_victim, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else {
      buf = replace_string(dam_weapons[11].to_room, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string(dam_weapons[11].to_char, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string(dam_weapons[11].to_victim, attack_hit_text[w_type].singular);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   }
}

char *replace_string2(char *str, char *weapon, char num_attacks)
{
   static char buf[256];
   char *cp;
 
   cp = buf;

   for (; *str; str++) {
      if (*str == '#') {
         switch(*(++str)) {
            case 'W' : 
               for (; *weapon; *(cp++) = *(weapon++));
               break;
            case 'H' : 
               *(cp++) = num_attacks;
               break;
            default :
               *(cp++) = '#';
               break;
         }
      } else {
         *(cp++) = *str;
      }

      *cp = 0;
   } /* For */

   return(buf);
}


void mult_dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type, int hits)
{
   struct obj_data *wield;
   char *buf;
        char *num = "012345";

   static struct dam_weapon_type {
      char *to_room;
      char *to_char;
      char *to_victim;
   } dam_weapons[] = {

    {"$n misses $N with all #H of $s #W's.",              /*    0    */
     "You miss $N with all #H of your #W's.",
     "$n misses you with all #H of $s #W's." },

      {"$n tickles $N with #H quick #W's.",                  /*  1.. 2  */
       "You tickle $N with #H quick #W's.",
       "$n tickles you with #H quick #W's." },

      {"$n barely #W $N.",                                   /*  3.. 4  */
       "You barely #W $N.",
       "$n barely #W you."},

   {"$n hits $N with #H quick #W's.",                     /*  5.. 6  */
       "You hit $N with #H quick #W's.",
       "$n hits you with #H quick #W's."}, 

   {"$n hits $N with #H hard #W's.",                      /*  7.. 10  */
       "You hit $N with #H hard #W's.",
       "$n hits you with #H hard #W's."}, 

   {"$n hits $N with #H very hard #W's.",                 /* 11.. 14  */
       "You hit $N with #H very hard #W's.",
       "$n hits you with #H very hard #W's."}, 


   {"$n hits $N with #H extremely hard #W's.",            /* 15.. 20  */
       "You hit $N with #H extremely hard #W's.",
       "$n hits you with #H extremely hard #W's."}, 


    {"$n massacres $N with a flurry of #H #W's.",         /*  21..30    */
     "You massacre $N with a flurry of #H #W's.", 
     "$n massacres you with a flurry of #H #W's."},

    {"$n annihilates $N with #H deadly #W's.",            /* 31..40 */
     "You annihilate $N with #H deadly #W's.",
     "$n annihilates you with #H deadly #W's."},

    {"$n obliterates $N with #H deadly #W's.",            /* 41..50 */
     "You obliterate $N with your deadly #W's.",
     "$n obliterates you with #H deadly #W's."},

    {"$n demolishes $N with $s #H deadly #W's.",          /* 51..75 */
     "You demolish $N with your #H deadly #W's.",
     "$n demolishes you with $s #H deadly #W's."},

    {"$n rends $N asunder with $s #H deadly #W's.",      /* Wow! */
     "You rend $N asunder with your #H deadly #W's.",
     "$n rends you asunder with $s #H deadly #W's."}
                   
   };

   w_type -= TYPE_HIT;   /* Change to base of table with text */
   wield = ch->equipment[WIELD];

   if (dam == 0) {
      buf = replace_string2(dam_weapons[0].to_room, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[0].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_FIGHT);
      buf = replace_string2(dam_weapons[0].to_victim, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_FIGHT);
   } else if (dam <= 4) {
      buf = replace_string2(dam_weapons[1].to_room, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[1].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string2(dam_weapons[1].to_victim, attack_hit_text[w_type].plural, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 8) {
      buf = replace_string2(dam_weapons[2].to_room, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[2].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string2(dam_weapons[2].to_victim, attack_hit_text[w_type].plural, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 12) {
      buf = replace_string2(dam_weapons[3].to_room, attack_hit_text[w_type].plural, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[3].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string2(dam_weapons[3].to_victim, attack_hit_text[w_type].plural, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 20) {
      buf = replace_string2(dam_weapons[4].to_room, attack_hit_text[w_type].plural, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[4].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string2(dam_weapons[4].to_victim, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 30) {
      buf = replace_string2(dam_weapons[5].to_room, attack_hit_text[w_type].plural, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[5].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string2(dam_weapons[5].to_victim, attack_hit_text[w_type].plural, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 40) {
      buf = replace_string2(dam_weapons[6].to_room, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[6].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string2(dam_weapons[6].to_victim, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 60) {
      buf = replace_string2(dam_weapons[7].to_room, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[7].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string2(dam_weapons[7].to_victim, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 80) {
      buf = replace_string2(dam_weapons[8].to_room, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[8].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string2(dam_weapons[8].to_victim, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 100) {
      buf = replace_string2(dam_weapons[9].to_room, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[9].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string2(dam_weapons[9].to_victim, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else if (dam <= 150) {
      buf = replace_string2(dam_weapons[10].to_room, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[10].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string2(dam_weapons[10].to_victim, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   } else {
      buf = replace_string2(dam_weapons[11].to_room, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_NOTVICT, CLR_FIGHT);
      buf = replace_string2(dam_weapons[11].to_char, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_CHAR, CLR_HIT);
      buf = replace_string2(dam_weapons[11].to_victim, attack_hit_text[w_type].singular, num[hits]);
      ansi_act(buf, FALSE, ch, wield, victim, TO_VICT, CLR_DAM);
   }
}


void damage(struct char_data *ch, struct char_data *victim,
            int dam, int attacktype, int hits)
{
   char buf[MAX_STRING_LENGTH];
   struct message_type *messages;
   int i,j,nr,max_hit,exp;

   int hit_limit(struct char_data *ch);

   if (GET_POS(victim) <= POSITION_DEAD) {
      log("Just tried to damage a dead person... ugh!");
      return;
   }
   if ((GET_LEVEL(victim)>IMO_LEV) && !IS_NPC(victim)) 
      dam=0;                 /* You can't damage an immortal! */
      
   if (victim != ch) {
      if (GET_POS(ch) > POSITION_STUNNED) {
         if (!(ch->specials.fighting)) {
            /* check for assist & group here */
            struct char_data *k;
            struct follow_type *f;

            if (!(k=ch->master))
               k = ch;
            if ( (k!=ch)
                 && (!(k->specials.fighting))
                 && (!IS_NPC(k))
                 && (k->in_room == ch->in_room)
                 && (IS_SET(k->specials.act, PLR_ASSIST))
                 && (IS_AFFECTED(k, AFF_GROUP))
               )
            {
               set_fighting(k, victim);
               GET_POS(k) = POSITION_FIGHTING;
            }
            for (f=k->followers; f; f=f->next)
               if ((( (IS_AFFECTED(f->follower, AFF_GROUP)) 
                      && (IS_NPC(f->follower) || IS_SET(f->follower->specials.act, PLR_ASSIST))
                      && (IS_AFFECTED(f->follower, AFF_GROUP))
                     ) || (IS_AFFECTED(f->follower, AFF_CHARM))
                    )
                    && (!(f->follower->specials.fighting))
                    && (f->follower->in_room == ch->in_room)
                    && (f->follower != ch)
                   )
               {
                  set_fighting(f->follower, victim);
                  GET_POS(f->follower) = POSITION_FIGHTING;
               }
            set_fighting(ch, victim);
         }

         if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
             !number(0,10) && IS_AFFECTED(victim, AFF_CHARM) &&
             (victim->master->in_room == ch->in_room)) {
            if (ch->specials.fighting)
               stop_fighting(ch);
            hit(ch, victim->master, TYPE_UNDEFINED);
            return;
         }
      }
      if (GET_POS(victim) > POSITION_STUNNED) {
         if (!(victim->specials.fighting)) { 
            /* check for assist & group here */
            struct char_data *k;
            struct follow_type *f;

            if (!(k=victim->master))
               k = victim;
            if ( (k!=victim)
                 && (!(k->specials.fighting))
                 && (!IS_NPC(k))
                 && (k->in_room == victim->in_room)
                 && (IS_SET(k->specials.act, PLR_ASSIST))
                 && (IS_AFFECTED(k, AFF_GROUP))
               )
            {
               set_fighting(k, ch);
               GET_POS(k) = POSITION_FIGHTING;
            }
            for (f=k->followers; f; f=f->next)
               if ((( (IS_AFFECTED(f->follower, AFF_GROUP)) 
                      && (IS_NPC(f->follower) || IS_SET(f->follower->specials.act, PLR_ASSIST))
                      && (IS_AFFECTED(f->follower, AFF_GROUP))
                     ) || (IS_AFFECTED(f->follower, AFF_CHARM))
                    )
                    && (!(f->follower->specials.fighting))
                    && (f->follower->in_room == victim->in_room)
                    && (f->follower != victim)
                   )
               {
                  set_fighting(f->follower, ch);
                  GET_POS(f->follower) = POSITION_FIGHTING;
               }
            set_fighting(victim, ch);
         }
         GET_POS(victim) = POSITION_FIGHTING;
      }
   }
   if (victim->master == ch) {
      stop_follower(victim);
      if (victim->specials.fighting == victim) {
         stop_fighting(victim);
         set_fighting(victim, ch);
      }
   }
         
   if (IS_AFFECTED(ch, AFF_INVISIBLE))
      appear(ch);
   if (IS_AFFECTED(victim, AFF_SANCTUARY))
      dam = MAXV(0, dam / 2);  

   dam=MAXV(dam,0);
   GET_HIT(victim)-=dam;
   if (ch != victim && !IS_NPC(ch))
      gain_exp(ch,GET_LEVEL(victim)*dam);

   if ((GET_HIT(victim)>0)
       && ((IS_NPC(victim)) || (GET_LEVEL(victim)<=IMO_LEV))
       && (attacktype >= TYPE_HIT) 
       && (attacktype <= TYPE_CRUSH)) {
      if (hits > 1)
         mult_dam_message(dam, ch, victim, attacktype, hits);
      else
         dam_message(dam, ch, victim, attacktype);
   } else {
      for(i = 0; i < MAX_MESSAGES; i++) {
         if (fight_messages[i].a_type == attacktype) {
            nr=dice(1,fight_messages[i].number_of_attacks);
            for(j=1,messages=fight_messages[i].msg;(j<nr)&&(messages);j++)
               messages=messages->next;
   
            if (!IS_NPC(victim) && (GET_LEVEL(victim) > IMO_LEV)) {
               ansi_act(messages->god_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR, CLR_FIGHT);
               ansi_act(messages->god_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT, CLR_FIGHT);
               ansi_act(messages->god_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT, CLR_FIGHT);
            } else if (dam != 0) {
               if (GET_POS(victim) == POSITION_DEAD) {
                  ansi_act(messages->die_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR, CLR_HIT);
                  ansi_act(messages->die_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT, CLR_DAM);
                  ansi_act(messages->die_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT, CLR_FIGHT);
               } else {
                  ansi_act(messages->hit_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR, CLR_HIT);
                  ansi_act(messages->hit_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT, CLR_DAM);
                  ansi_act(messages->hit_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT, CLR_FIGHT);
               }
            } else { /* Dam == 0 */
               ansi_act(messages->miss_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR, CLR_FIGHT);
               ansi_act(messages->miss_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT, CLR_FIGHT);
               ansi_act(messages->miss_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT, CLR_FIGHT);
            }
         }
      }
   }
   update_pos(victim);
   switch (GET_POS(victim)) {
      case POSITION_MORTALLYW:
         act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
         act("You are mortally wounded, and will die soon, if not aided.", FALSE, victim, 0, 0, TO_CHAR);
         break;
      case POSITION_INCAP:
         act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
         act("You are incapacitated an will slowly die, if not aided.", FALSE, victim, 0, 0, TO_CHAR);
         break;
      case POSITION_STUNNED:
         act("$n is stunned, but will probably regain conscience again.", TRUE, victim, 0, 0, TO_ROOM);
         act("You're stunned, but will probably regain conscience again.", FALSE, victim, 0, 0, TO_CHAR);
         break;
      case POSITION_DEAD: {
         ansi_act("$n is DEAD! R.I.P.", TRUE, victim, 0, 0, TO_ROOM,CLR_DAM);
	 ansi_act("You are DEAD!  Sorry...", FALSE, victim, 0, 0, TO_CHAR,CLR_DAM);
       }
         break;

      default:  /* >= POSITION SLEEPING */

         max_hit=hit_limit(victim);
         if (dam > (max_hit/5))
            ansi_act("That Really did HURT!",FALSE, victim, 0, 0, TO_CHAR,CLR_DAM);

         if (GET_HIT(victim) < (max_hit/5)) {
            ansi_act("You wish that your wounds would stop BLEEDING that much!",FALSE,victim,0,0,TO_CHAR, CLR_DAM);
            if (IS_NPC(victim)) {
               if (IS_SET(victim->specials.act, ACT_WIMPY))
                  do_flee(victim, "", 0);
            } else if (IS_SET(victim->specials.act, PLR_WIMPY))
               do_flee(victim, "", 0);
         }
         break;      
   }

   if (!IS_NPC(victim) && !(victim->desc)) {
      do_flee(victim, "", 0);
      if (!victim->specials.fighting) {
         ansi_act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM,CLR_ACTION);
         victim->specials.was_in_room = victim->in_room;
         char_from_room(victim);
         char_to_room(victim, 0);
      }
   }

   if (GET_POS(victim) == POSITION_DEAD) {
      if (IS_NPC(victim)) {
         if (IS_AFFECTED(ch, AFF_GROUP)) {
            group_gain(ch, victim);
         } else {
            /* Calculate level-difference bonus */
            exp = GET_EXP(victim);
            if (IS_NPC(ch))
               exp = 0;
            else
               exp += (exp*MINV(8, (GET_LEVEL(victim) - GET_LEVEL(ch))))/8;
            exp = MAXV(exp, 1);
            sprintf(buf, "You receive %d experience points." , exp);
            ansi_act(buf, FALSE, ch, 0, 0, TO_CHAR, CLR_ACTION);
            sprintf(buf, "You could learn more fighting in a group.");
            ansi_act(buf, FALSE, ch, 0, 0, TO_CHAR, WHITE);

            gain_exp(ch, exp);
            change_alignment(ch, victim);
         }
         if (IS_SET(victim->specials.act, ACT_SPEC)) {
            (*mob_index[victim->nr].func)(ch, -1, "");
         }
      }
      if (!IS_NPC(victim)) {
         sprintf(buf, "%s killed by %s at %s",
            GET_NAME(victim),
            (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
            world[victim->in_room].name);
         log(buf);
         sprintf(buf, "SYS: %s just kicked the bucket.", GET_NAME(victim));
         do_sys(buf, 1);
      }
      /* set player killer bits here */
      if (!IS_NPC(ch) && !IS_NPC(victim) && IS_SET(ch->specials.act, PLR_ATTACKER)) {
         if (IS_SET(victim->specials.act, PLR_PKILLER)
             && !IS_SET(ch->specials.act, PLR_PKILLER)) {
            SET_BIT(ch->specials.act, PLR_ENFORCER);
	    ansi_act("You are rewarded for killing a wanted murderer!", FALSE, ch, 0, 0, TO_CHAR,WHITE);
            GET_GOLD(ch) += 400*GET_LEVEL(victim)*GET_LEVEL(victim);
         } if (!IS_SET(victim->specials.act, PLR_PKILLER)
             && !IS_SET(ch->specials.act, PLR_PKILLER))
            SET_BIT(ch->specials.act, PLR_PKILLER);
      }
      die(victim);
      if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_AUTOLOOT))
        do_get(ch, " all corpse", CMD_GET);
   }
   if (GET_POS(victim) < POSITION_STUNNED)
      if (ch->specials.fighting == victim)
         stop_fighting(ch);
   if (!AWAKE(victim))
      if (victim->specials.fighting)
         stop_fighting(victim);
}

int spec_damage(struct char_data *ch, struct char_data *victim, int dam)
{
   struct obj_data *wielded = 0;

   if ( (ch->equipment[WIELD]) &&
   (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON) ) {
      wielded = ch->equipment[WIELD];
      if (IS_SET(wielded->obj_flags.value[0],WEAPON_POISONOUS)&& (number(0,1) == 0)) {
         cast_poison(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,victim,0);
      }
      if (IS_SET(wielded->obj_flags.value[0],WEAPON_BLIND)&& (number(0,2) == 0)) {
         cast_blindness(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,victim,0);
      }
      if (IS_SET(wielded->obj_flags.value[0],WEAPON_VAMPIRIC)&& (number(0,3) == 0)) {
         GET_HIT(ch) = MAXV(hit_limit(ch),GET_HIT(ch)+(dam >> 2));
         act("You feel stolen life energy rush through your body!",FALSE,ch,0,0,TO_CHAR);
      }
      if (IS_SET(wielded->obj_flags.value[0],WEAPON_DRAIN_MANA)&& (number(0,3) == 0)) {
         cast_energy_drain(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,victim,0);
      }
      if ( (IS_SET(wielded->obj_flags.value[0],WEAPON_GOOD) && IS_GOOD(victim)) ||
      (IS_SET(wielded->obj_flags.value[0],WEAPON_NEUTRAL) && IS_NEUTRAL(victim)) ||
      (IS_SET(wielded->obj_flags.value[0],WEAPON_EVIL) && IS_EVIL(victim)) )
         dam >>= 1;
      if (IS_SET(wielded->obj_flags.value[0],WEAPON_GOOD_SLAYER) && IS_GOOD(victim)) 
         dam <<= 1;
      if (IS_SET(wielded->obj_flags.value[0],WEAPON_EVIL_SLAYER) && IS_EVIL(victim)) 
         dam <<= 1;
      if (IS_SET(wielded->obj_flags.value[0],WEAPON_DRAGON_SLAYER)) {
         if (IS_NPC(victim)) {
            if (IS_SET(victim->specials.act, CLASS_DRAGON))
               dam <<= 1;
         } else if (GET_CLASS(ch) == CLASS_DRAGONW)
            dam <<= 1;
      }
      if (IS_NPC(victim)) {
         if (IS_SET(wielded->obj_flags.value[0],WEAPON_UNDEAD_SLAYER)
            && IS_SET(victim->specials.act, CLASS_UNDEAD)) dam <<=1;
         if (IS_SET(wielded->obj_flags.value[0],WEAPON_ANIMAL_SLAYER)
            && IS_SET(victim->specials.act, CLASS_ANIMAL)) dam <<=1;
         if (IS_SET(wielded->obj_flags.value[0],WEAPON_GIANT_SLAYER)
            && IS_SET(victim->specials.act, CLASS_GIANT)) dam <<=1;
         if (!IS_SET(wielded->obj_flags.value[0],WEAPON_SILVER)
            && IS_SET(victim->specials.act, CLASS_SHADOW)) dam = 0;
         if (!IS_SET(wielded->obj_flags.value[0],WEAPON_WOOD_STAKE)
            && IS_SET(victim->specials.act, CLASS_VAMPIRE)) dam = 0;
      }

   } else if (IS_NPC(victim) && 
      ( IS_SET(victim->specials.act, CLASS_VAMPIRE)
      || IS_SET(victim->specials.act, CLASS_SHADOW) )) dam = 0;

   if (IS_NPC(ch)) {
      if (IS_SET(ch->specials.act, ACT_POISONOUS) && number(0,2) == 0)
         cast_poison(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,victim,0);
      if (IS_SET(ch->specials.act, ACT_DRAIN_XP) && number(0,2) == 0)
         cast_energy_drain(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,victim,0);
      if (IS_SET(ch->specials.act, ACT_BREATH_GAS) && number(0,2) == 0)
         cast_gas_breath( GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, 0, 0 );
      if (IS_SET(ch->specials.act, ACT_BREATH_FROST) && number(0,2) == 0)
         cast_frost_breath( GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, victim, 0 );
      if (IS_SET(ch->specials.act, ACT_BREATH_ACID) && number(0,2) == 0)
         cast_acid_breath( GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, victim, 0 );
      if (IS_SET(ch->specials.act, ACT_BREATH_LIGHTNING) && number(0,5) == 0)
         cast_lightning_breath( GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, victim, 0 );
      if (IS_SET(ch->specials.act, ACT_BREATH_FIRE) && number(0,2) == 0)
         cast_fire_breath( GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, victim, 0 );
   }
   return(dam);
}

void hit(struct char_data *ch, struct char_data *victim, int type)
{

   struct obj_data *wielded = 0;
   struct obj_data *held = 0;
   int w_type;
   int victim_ac, calc_thaco;
   int dam, num_attacks, i;
   sbyte diceroll;

   if (ch->in_room != victim->in_room) {
      log("NOT SAME ROOM WHEN FIGHTING!");
      return;
   }
   calc_thaco = 0;
   if (ch->equipment[HOLD])
      held = ch->equipment[HOLD];
   if (ch->equipment[WIELD] &&
      (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)) {
      wielded = ch->equipment[WIELD];
      switch (wielded->obj_flags.value[3]) {
         case 0  :
         case 1  :
         case 2  : w_type = TYPE_WHIP;
             calc_thaco = ch->skills[SKILL_SLASH].learned /10; break;
         case 3  : w_type = TYPE_SLASH;
             calc_thaco = ch->skills[SKILL_SLASH].learned /10; break;
         case 4  :
         case 5  :
         case 6  : w_type = TYPE_CRUSH; 
             calc_thaco = ch->skills[SKILL_BLUDGEON].learned /10; break;
         case 7  : w_type = TYPE_BLUDGEON; 
             calc_thaco = ch->skills[SKILL_BLUDGEON].learned /10; break;
         case 8  :
         case 9  :
         case 10 :
         case 11 : w_type = TYPE_PIERCE; 
             calc_thaco = ch->skills[SKILL_PIERCE].learned /10; break;
         default : w_type = TYPE_HIT; break;
      }
   }  else {
      if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
         w_type = ch->specials.attack_type;
      else
         w_type = TYPE_HIT;
   }

   /* Calculate the raw armor including magic armor */
   /* The lower AC, the better                      */

   if (!IS_NPC(ch))
      calc_thaco  = thaco[GET_CLASS(ch)-1][GET_LEVEL(ch)] - calc_thaco;
   else
      /* THAC0 for monsters is set in the HitRoll */
      calc_thaco = 30 - calc_thaco;

   calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
   calc_thaco -= GET_HITROLL(ch);
   victim_ac  = (GET_AC(victim) - victim->skills[SKILL_DODGE].learned)/10;
   diceroll = number(1,30);
   if (AWAKE(victim))
      victim_ac += dex_app[GET_DEX(victim)].defensive;

   if ((diceroll < 30) && AWAKE(victim) && 
   ((diceroll==1) || ((calc_thaco-diceroll) > victim_ac))) { /* miss */
      if (type == SKILL_BACKSTAB)
         damage(ch, victim, 0, SKILL_BACKSTAB, 0);
      else
         damage(ch, victim, 0, w_type, 0);
   } else {
      diceroll = number(1,30);
      num_attacks = 1;
      dam = 0;
      if ((number(0,101) < ch->skills[SKILL_SECOND].learned) && 
      (!((diceroll < 30) && AWAKE(victim) && ((diceroll==1) || ((calc_thaco-diceroll) > victim_ac))))) { /* hit */
         num_attacks += 1;
         diceroll = number(1,30);
         if ((number(0,101) < ch->skills[SKILL_THIRD].learned) && 
         (!((diceroll < 30) && AWAKE(victim) && ((diceroll==1) || ((calc_thaco-diceroll) > victim_ac))))) { /* hit */
            num_attacks += 1;
            diceroll = number(1,30);
            if ((number(0,101) < ch->skills[SKILL_THIRD].learned) && 
            (!((diceroll < 30) && AWAKE(victim) && ((diceroll==1) || ((calc_thaco-diceroll) > victim_ac))))) { /* hit */
               num_attacks += 1;
            }
         }
      }  
      for (i=0;i < num_attacks; i++) {
         dam += str_app[STRENGTH_APPLY_INDEX(ch)].todam;
         dam += GET_ADD(ch) / 25;
         dam += GET_DAMROLL(ch);

	 if (IS_NPC(ch))
	   dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
	 else
	   dam += number(0,2);  /* Max. 2 dam with bare hands */

         if (wielded) 
	   dam +=dice(wielded->obj_flags.value[1],wielded->obj_flags.value[2]);

         if (GET_POS(victim) < POSITION_FIGHTING)
            dam *= 1+(POSITION_FIGHTING-GET_POS(victim))/3;
         /* Position  sitting  x 1.33 */
         /* Position  resting  x 1.66 */
         /* Position  sleeping x 2.00 */
         /* Position  stunned  x 2.33 */
         /* Position  incap    x 2.66 */
         /* Position  mortally x 3.00 */

         dam -= victim->skills[SKILL_SCALES].learned / 10; 
         dam = spec_damage(ch,victim,dam);  /* slaying etc */
         if (GET_POS(victim) == POSITION_DEAD) /* got 'em with specials  - a tough bug to spot btw */
            return;
         dam = MAXV(1, dam);  /* Not less than 1 damage */
      }
      if (type == SKILL_BACKSTAB) {
         dam *= backstab_mult[GET_LEVEL(ch)];
         damage(ch, victim, dam, SKILL_BACKSTAB, num_attacks);
      } else {
         if ((GET_POS(victim) = POSITION_FIGHTING) && (dam > GET_HIT(victim)) &&
         (GET_MOVE(victim) > 0) && (number(0,101) < victim->skills[SKILL_PARRY].learned)) { 
            GET_MOVE(victim) = MAXV(0,GET_MOVE(victim) - 50);
            act("You desperately parry the blow that would have been your death.",
               FALSE, victim, 0, 0, TO_CHAR);
            act("$n desperately parries the blow that would have been $s death.",
                FALSE, victim, 0, 0, TO_ROOM);
         } else
            damage(ch, victim, dam, w_type, num_attacks);
      }
   }
}



/* control the fights going on */
void perform_violence(void)
{
   struct char_data *ch;

   for (ch = combat_list; ch; ch=combat_next_dude)
   {
      combat_next_dude = ch->next_fighting;
      assert(ch->specials.fighting);

      if (AWAKE(ch) && (ch->in_room==ch->specials.fighting->in_room)) {
         hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
      } else { /* Not in same room */
         stop_fighting(ch);
      }
   }
}


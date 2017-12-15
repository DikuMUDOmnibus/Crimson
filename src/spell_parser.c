/* ************************************************************************
*  file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD *
*  Usage : Interpreter of spells                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <assert.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h" 
#include "spells.h"
#include "handler.h"
#include "constants.h"
#include "ansi.h"

#define MANA_MU 1
#define MANA_CL 1

#define SPELLO(nr, beat, pos, mlev, clev, mana, tar, blev, klev, rlev, dlev, elev, func) { \
   spell_info[nr].spell_pointer = (func);      \
   spell_info[nr].beats = (beat);              \
   spell_info[nr].minimum_position = (pos);    \
   spell_info[nr].min_usesmana = (mana);       \
   spell_info[nr].min_level_cleric = (clev);   \
   spell_info[nr].min_level_magic = (mlev);    \
   spell_info[nr].min_level_bard = (blev);     \
   spell_info[nr].min_level_kai = (klev);      \
   spell_info[nr].min_level_drakkhar = (rlev); \
   spell_info[nr].min_level_dknight = (dlev);  \
   spell_info[nr].min_level_eknight = (elev);  \
   spell_info[nr].targets = (tar);             \
}

#define SPELL_LEVEL(ch, sn)                              \
  ( (GET_CLASS(ch) == CLASS_CLERIC) ? spell_info[sn].min_level_cleric :    \
   ( (GET_CLASS(ch) == CLASS_MAGIC_USER) ? spell_info[sn].min_level_magic : \
   ( (GET_CLASS(ch) == CLASS_BARD) ? spell_info[sn].min_level_bard :    \
   ( (GET_CLASS(ch) == CLASS_KAI) ? spell_info[sn].min_level_kai : \
   ( (GET_CLASS(ch) == CLASS_DRAKKHAR) ? spell_info[sn].min_level_drakkhar : \
   ( (GET_CLASS(ch) == CLASS_DKNIGHT) ? spell_info[sn].min_level_dknight : \
   ( (GET_CLASS(ch) == CLASS_EKNIGHT) ? spell_info[sn].min_level_eknight :                                             IMO_LEV)))))))


#define USE_MANA(ch, sn)                            \
  MAXV(spell_info[sn].min_usesmana, 100/(2+GET_LEVEL(ch)-SPELL_LEVEL(ch,sn)))

/* Global data */
extern int pulse; /* from comm.c */


struct spell_info_type spell_info[MAX_SPL_LIST];

const char *spells[]=
{
   "armor",               /* 1 */
   "teleport",
   "bless",
   "blindness",
   "burning hands",
   "call lightning",
   "charm person",
   "chill touch",
   "improved invisibility", /*used to be clones spot */
   "colour spray",
   "control weather",     /* 11 */
   "create food",
   "create water",
   "cure blind",
   "cure critic",
   "cure light",
   "curse",
   "detect evil",
   "detect invisibility",
   "detect magic",
   "detect poison",       /* 21 */
   "dispel evil",
   "earthquake",
   "enchant weapon",
   "energy drain",
   "fireball",
   "harm",
   "heal",
   "invisibility",
   "lightning bolt",
   "locate object",      /* 31 */
   "magic missile",
   "poison",
   "protection from evil",
   "remove curse",
   "sanctuary",
   "shocking grasp",
   "sleep",
   "strength",
   "summon",
   "ventriloquate",      /* 41 */
   "word of recall",
   "remove poison",
   "sense life",         /* 44 */
   "SKILL_SNEAK",        /* 45 */
   "SKILL_HIDE",
   "SKILL_STEAL",
   "SKILL_BACKSTAB",
   "SKILL_PICK_LOCK",
   "SKILL_KICK",         /* 50 */
   "SKILL_BASH",
   "SKILL_RESCUE",
   "identify",           /* 53 */
   "cause critic",
   "cause light",        /* 55 */
   "SKILL_DAMAGE",
   "SKILL_SLASH",
   "SKILL_PIERCE",
   "SKILL_BLUDGEON",
   "SKILL_PARRY",       /* 60 */
   "SKILL_DODGE",
   "SKILL_SCALES",
   "fire breath",
   "gas breath",
   "frost breath",
   "acid breath",      /* 66 */
   "lightning breath",
   "darksight",
   "SKILL_SECOND",
   "SKILL_THIRD",    /* 70 */
   "SKILL_FOURTH",
   "undetectable invisibility",
   "restoration",
   "regeneration",
   "minor wards", /* 75 */
   "major wards",
   "mordenkainens sword",
   "spiritual hammer",
   "turn undead",
   "succor",      /* 80 */
   "donate mana",
   "mana link",
   "magic resist",
   "magic immune",
   "dispel magic",   /* 85 */
   "conflagration", /* 86 */
   "breathwater", /* 87 */
   "sustenance",  /* 88 */
   "\n"
};


const sbyte saving_throws[MAX_CLASS][5][MAX_LEV] = {
{ /* Mage */
  {26,24,24,24,24,24,23,23,23,23,23,21,21,21,21,21,20,20,20,20,20,
      16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10, 1, 8, 6, 4, 0},
  {23,21,21,21,21,21,19,19,19,19,19,19,17,17,17,17,17,15,15,15,15,
      13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 0},
  {25,25,23,23,23,23,23,21,21,21,21,21,19,19,19,19,19,17,17,17,17,
      15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 0},
  {27,27,25,25,25,25,25,23,23,23,23,23,21,21,21,21,21,19,19,19,19,
      17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 0},
  {24,24,22,22,22,22,22,20,20,20,20,20,18,18,18,18,18,16,16,16,16,
      14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 0}
}, { /* Cleric */
  {21,21,20,20,20,19,19,19,17,17,17,16,16,16,15,15,15,14,14,14,12,
      11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 0},
  {26,26,24,24,24,23,23,23,21,21,21,20,20,20,19,19,19,18,18,18,17,
      16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 0},
  {25,25,23,23,23,22,22,22,20,20,20,19,19,19,18,18,18,17,17,17,15,
      15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 0},
  {28,28,26,26,26,25,25,25,23,23,23,22,22,22,21,21,21,20,20,20,18,
      18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 0},
  {27,27,25,25,25,24,24,24,22,22,22,21,21,21,20,20,20,19,19,19,17,
      17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 0}
}, { /* Thief */
  {25,23,23,23,23,23,22,22,22,22,21,21,21,21,20,20,20,20,19,19,19,
      15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 7, 6, 0},
  {26,26,24,24,24,24,22,22,22,22,20,20,20,20,18,18,18,18,16,16,16,
      16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 0},
  {24,24,22,22,22,22,21,21,21,21,20,20,20,20,19,19,19,19,18,18,18,
      14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 0},
  {28,28,26,26,26,26,25,25,25,25,24,24,24,24,23,23,23,23,22,22,22,
      18,16,16,16,16,15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12,11, 9, 5, 0},
  {27,27,25,25,25,25,23,23,23,23,21,21,21,21,19,19,19,19,17,17,17,
      17,15,15,15,15,13,13,13,13,11,11,11,11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 0}
}, { /* Warrior */
  {26,26,24,24,23,23,21,21,20,20,18,18,17,17,15,15,14,14,13,13,13,
      16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 0},
  {28,28,26,26,25,25,23,23,22,22,20,20,19,19,17,17,16,16,15,15,15,
      18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 0},
  {27,27,25,25,24,24,22,22,21,21,19,19,18,18,16,16,15,15,14,14,14,
      17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 0},
  {30,30,27,27,26,26,23,23,22,22,19,19,18,18,15,15,14,14,14,14,14, 
      20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0},
  {29,29,27,27,26,26,24,24,23,23,21,21,20,20,18,18,17,17,16,16,16,
      19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 0}
}, { /* Bard */
  {25,23,23,23,23,23,22,22,22,22,21,21,21,21,20,20,20,20,19,19,19,
      15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 7, 6, 0},
  {26,26,24,24,24,24,22,22,22,22,20,20,20,20,18,18,18,18,16,16,16,
      16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 0},
  {24,24,22,22,22,22,21,21,21,21,20,20,20,20,19,19,19,19,18,18,18,
      14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 0},
  {28,28,26,26,26,26,25,25,25,25,24,24,24,24,23,23,23,23,22,22,22,
      18,16,16,16,16,15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12,11, 9, 5, 0},
  {27,27,25,25,25,25,23,23,23,23,21,21,21,21,19,19,19,19,17,17,17,
      17,15,15,15,15,13,13,13,13,11,11,11,11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 0}
}, { /* Kai */
  {21,21,20,20,20,19,19,19,17,17,17,16,16,16,15,15,15,14,14,14,12,
      11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 0},
  {26,26,24,24,24,23,23,23,21,21,21,20,20,20,19,19,19,18,18,18,17,
      16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 0},
  {25,25,23,23,23,22,22,22,20,20,20,19,19,19,18,18,18,17,17,17,15,
      15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 0},
  {28,28,26,26,26,25,25,25,23,23,23,22,22,22,21,21,21,20,20,20,18,
      18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 0},
  {27,27,25,25,25,24,24,24,22,22,22,21,21,21,20,20,20,19,19,19,17,
      17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 0}
}, { /* Drakkhar */
  {21,21,20,20,20,19,19,19,17,17,17,16,16,16,15,15,15,14,14,14,12,
      11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 0},
  {26,26,24,24,24,23,23,23,21,21,21,20,20,20,19,19,19,18,18,18,17,
      16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 0},
  {25,25,23,23,23,22,22,22,20,20,20,19,19,19,18,18,18,17,17,17,15,
      15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 0},
  {28,28,26,26,26,25,25,25,23,23,23,22,22,22,21,21,21,20,20,20,18,
      18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 0},
  {27,27,25,25,25,24,24,24,22,22,22,21,21,21,20,20,20,19,19,19,17,
      17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 0}
}, { /* Demon Knight */
  {26,24,24,24,24,24,23,23,23,23,23,21,21,21,21,21,20,20,20,20,20,
      16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10, 1, 8, 6, 4, 0},
  {23,21,21,21,21,21,19,19,19,19,19,19,17,17,17,17,17,15,15,15,15,
      13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 0},
  {25,25,23,23,23,23,23,21,21,21,21,21,19,19,19,19,19,17,17,17,17,
      15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 0},
  {27,27,25,25,25,25,25,23,23,23,23,23,21,21,21,21,21,19,19,19,19,
      17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 0},
  {24,24,22,22,22,22,22,20,20,20,20,20,18,18,18,18,18,16,16,16,16,
      14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 0}
}, { /* Elfin Knight */
  {26,24,24,24,24,24,23,23,23,23,23,21,21,21,21,21,20,20,20,20,20,
      16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10, 1, 8, 6, 4, 0},
  {23,21,21,21,21,21,19,19,19,19,19,19,17,17,17,17,17,15,15,15,15,
      13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 0},
  {25,25,23,23,23,23,23,21,21,21,21,21,19,19,19,19,19,17,17,17,17,
      15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 0},
  {27,27,25,25,25,25,25,23,23,23,23,23,21,21,21,21,21,19,19,19,19,
      17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 0},
  {24,24,22,22,22,22,22,20,20,20,20,20,18,18,18,18,18,16,16,16,16,
      14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 0}
}, { /* Dragon-Kin Warrior */
  {26,26,24,24,23,23,21,21,20,20,18,18,17,17,15,15,14,14,13,13,13,
      16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 0},
  {28,28,26,26,25,25,23,23,22,22,20,20,19,19,17,17,16,16,15,15,15,
      18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 0},
  {27,27,25,25,24,24,22,22,21,21,19,19,18,18,16,16,15,15,14,14,14,
      17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 0},
  {30,30,27,27,26,26,23,23,22,22,19,19,18,18,15,15,14,14,14,14,14, 
      20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0},
  {29,29,27,27,26,26,24,24,23,23,21,21,20,20,18,18,17,17,16,16,16,
      19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 0}
}
};



void affect_update( void )
{
   static struct affected_type *af, *next_af_dude, ds_aff;
   static struct char_data *i;

   ds_aff.type = SPELL_DARKSIGHT; /* setup for l8r */
   ds_aff.modifier = 0;
   ds_aff.location = APPLY_NONE;
   ds_aff.bitvector = AFF_DARKSIGHT;

   for (i = character_list; i; i = i->next) {
      for (af = i->affected; af; af = next_af_dude) {
         next_af_dude = af->next;
         if (af->duration >= 1)
            af->duration--;
         else if (af->duration == 0) {
            if ((af->type > 0) && (af->type <= 100)) /* It must be a spell */
               if (!af->next  
                  || (af->next->type != af->type) 
                  || (af->next->duration > 0) )
                  if (*spell_wear_off_msg[af->type]
                      && !(af->type == SPELL_DARKSIGHT && GET_CLASS(i) == CLASS_DRAGONW)) {
                     /* dragon-kin dont get darksight expired messages */
                     send_to_char((char*)spell_wear_off_msg[af->type], i);
                     send_to_char("\n\r", i);
                  }

            affect_remove(i, af);
         }
      }
      /* If Dragon-Kin check to see if Darksight is working here */
      /* ie if !Darksight then check number 0-100 against skill
        and cast spell on char for duration of level if it succeeds */
      if (GET_CLASS(i) == CLASS_DRAGONW && !IS_AFFECTED(i, AFF_DARKSIGHT)) {
         if (number(0,100) < i->skills[SPELL_DARKSIGHT].learned) {
            ds_aff.duration = GET_LEVEL(i);
            affect_to_char(i, &ds_aff);
         }
      }
      if (GET_LEVEL(i) == IMO_LEV && !IS_AFFECTED(i,AFF_DARKSIGHT)){
         ds_aff.duration = -1;
         affect_to_char(i, &ds_aff);
      }
   }
}

#ifdef REENABLE_CLONE_CODE
void clone_char(struct char_data *ch)
{
   extern struct index_data *mob_index;
   struct char_data *clone;
   struct affected_type *af;
   int i;

   CREATE(clone, struct char_data, 1);


   clear_char(clone);       /* Clear EVERYTHING! (ASSUMES CORRECT) */

   clone->player    = ch->player;
   clone->abilities = ch->abilities;

   for (i=0; i<5; i++)
      clone->specials.apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

   for (af=ch->affected; af; af = af->next)
      affect_to_char(clone, af);

   for (i=0; i<3; i++)
      GET_COND(clone,i) = GET_COND(ch, i);

   clone->points = ch->points;

   for (i=0; i<MAX_SKILLS; i++)
      clone->skills[i] = ch->skills[i];

   clone->specials = ch->specials;
   clone->specials.fighting = 0;

   GET_NAME(clone) = str_alloc(GET_NAME(ch));

   clone->player.short_descr =   str_alloc(ch->player.short_descr);

   clone->player.long_descr = str_alloc(ch->player.long_descr);

   clone->player.description = 0;
   /* REMEMBER EXTRA DESCRIPTIONS */

   GET_TITLE(clone) = str_alloc(GET_TITLE(ch));

   clone->nr = ch->nr;

   if (IS_NPC(clone))
      mob_index[clone->nr].number++;
   else { /* Make PC's into NPC's */
      clone->nr = -1;
      SET_BIT(clone->specials.act, ACT_ISNPC);
   }

   clone->desc = 0;
   clone->followers = 0;
   clone->master = 0;

   clone->next = character_list;
   character_list = clone;

   char_to_room(clone, ch->in_room);
}



void clone_obj(struct obj_data *obj)
{
   struct obj_data *clone;
   struct extra_descr_data *ed, temp;


   CREATE(clone, struct obj_data, 1);

   *clone = *obj;

   clone->name               = str_alloc(obj->name);
   clone->description        = str_alloc(obj->description);
   clone->short_description  = str_alloc(obj->short_description);
   clone->action_description = str_alloc(obj->action_description);
   clone->ex_description     = 0;

   /* REMEMBER EXTRA DESCRIPTIONS */
   clone->carried_by         = 0;
   clone->in_obj             = 0;
   clone->contains           = 0;
   clone->next_content       = 0;
   clone->next               = 0;

   /* VIRKER IKKE ENDNU */
}
#endif



void say_spell( struct char_data *ch, int si )
{
   char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
   char buf2[MAX_STRING_LENGTH];

   int j, offs;
   struct char_data *temp_char;


   struct syllable {
      char org[10];
      char new[10];
   };

   struct syllable syls[] = {
   { " ", " " },
   { "ar", "abra"   },
   { "au", "kada"    },
   { "bless", "fido" },
   { "blind", "nose" },
   { "bur", "mosa" },
   { "cu", "judi" },
   { "de", "oculo"},
   { "en", "unso" },
   { "light", "dies" },
   { "lo", "hi" },
   { "mor", "zak" },
   { "move", "sido" },
   { "ness", "lacri" },
   { "ning", "illa" },
   { "per", "duda" },
   { "ra", "gru"   },
   { "re", "candus" },
   { "son", "sabru" },
   { "tect", "infra" },
   { "tri", "cula" },
   { "ven", "nofo" },
   {"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
   {"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
   {"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
   {"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
   };



   strcpy(buf, "");
   strcpy(splwd, spells[si-1]);

   offs = 0;

   while(*(splwd+offs)) {
      for(j=0; *(syls[j].org); j++)
         if (strncmp(syls[j].org, splwd+offs, strlen(syls[j].org))==0) {
            strcat(buf, syls[j].new);
            if (strlen(syls[j].org))
               offs+=strlen(syls[j].org);
            else
               ++offs;
         }
   }


   sprintf(buf2,"$n utters the words, '%s'", buf);
   sprintf(buf, "$n utters the words, '%s'", spells[si-1]);

   for(temp_char = world[ch->in_room].people;
      temp_char;
      temp_char = temp_char->next_in_room)
      if(temp_char != ch) {
         if (!IS_NPC(ch) && GET_CLASS(ch) == GET_CLASS(temp_char))
	   act(buf, FALSE, ch, 0, temp_char, TO_VICT);
         else
	   act(buf2, FALSE, ch, 0, temp_char, TO_VICT);

      }

}



bool saves_spell(struct char_data *ch, sh_int save_type)
{
   int save;

   /* Negative apply_saving_throw makes saving throw better! */

   save = ch->specials.apply_saving_throw[save_type];

   if (!IS_NPC(ch)) {
      save += saving_throws[GET_CLASS(ch)-1][save_type][GET_LEVEL(ch)];
      if (GET_LEVEL(ch) > IMO_LEV)
         return(TRUE);
   }

   return(MAXV(1,save) < number(1,30));
}



char *skip_spaces(char *string)
{
   for(;*string && (*string)==' ';string++);

   return(string);
}



/* Assumes that *argument does start with first letter of chopped string */

void do_cast(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *tar_obj;
  struct char_data *tar_char;
  char name[MAX_STRING_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  int qend, spl, i;
  bool target_ok;

  if (IS_NPC(ch))
    return;

  if (GET_LEVEL(ch) < IMO_LEV) {
      if (GET_CLASS(ch) == CLASS_WARRIOR) {
	send_to_char("Think you had better stick to fighting...\n\r", ch);
	return;
      }  else if (GET_CLASS(ch) == CLASS_THIEF) {
	send_to_char("Think you should stick to robbing and killing...\n\r", ch);
	return;
      }  else if (GET_CLASS(ch) == CLASS_DRAGONW) {
	send_to_char("Think you should stick to breathing fire...\n\r", ch);
	return;
      }
    }

  argument = skip_spaces(argument);

  /* If there is no chars in argument */
  if (!(*argument)) {
    send_to_char("Cast which what where?\n\r", ch);
    return;
  }

  if (*argument != '\'' && *argument != '\"') {
    for (qend=1; *(argument+qend) && isalpha(*(argument+qend)); qend++)
      *(argument+qend) = LOWER(*(argument+qend));
    spl = old_search_block(argument, 0, qend-1,spells, 0);
    --qend;
  } else {
    /* Locate the last quote && lowercase the magic words (if any) */
    for (qend=1; *(argument+qend) && (*(argument+qend) != '\'')
	 && (*(argument+qend) != '\"'); qend++)
      *(argument+qend) = LOWER(*(argument+qend));
 
    if (*(argument+qend) != '\'') {
      send_to_char("Come again? try something like cast \'a spell\'\n\r",ch);
      return;
    }
    spl = old_search_block(argument, 1, qend-1,spells, 0);
  }

  if (!spl) {
    send_to_char("Your lips do not move, no magic appears.\n\r",ch);
    return;
  }

  if ((ch->specials.fighting) && (pulse%PULSE_VIOLENCE)) {
    /* if we are fighting delay spell till start of next round */
    sprintf(buf, "cast %s", argument); /* queue command until next wait */
    WAIT_STATE(ch, PULSE_VIOLENCE - pulse%PULSE_VIOLENCE);
    write_to_head_q(buf, &ch->desc->input);
    return; /* we'll be back! */
  }
  if ((spl > 0) && (spl <= MAX_SPL_LIST) && spell_info[spl].spell_pointer) {
    if (GET_POS(ch) < spell_info[spl].minimum_position) {
      switch(GET_POS(ch)) {
      case POSITION_SLEEPING :
	send_to_char("You dream about great magical powers.\n\r", ch);
	break;
      case POSITION_RESTING :
	send_to_char("You can't concentrate enough while resting.\n\r",ch);
	break;
      case POSITION_SITTING :
	send_to_char("You can't do this sitting!\n\r", ch);
	break;
      case POSITION_FIGHTING :
	send_to_char("Impossible! You can't concentrate enough!.\n\r", ch);
	break;
      default:
	send_to_char("It seems like you're in a pretty bad shape!\n\r",ch);
	break;
      } /* Switch */
    }  else {

      if (GET_LEVEL(ch) < IMO_LEV) {
	if ((GET_CLASS(ch) == CLASS_MAGIC_USER) &&
	    (spell_info[spl].min_level_magic > GET_LEVEL(ch))) {
	  send_to_char("Sorry, you can't do that.\n\r", ch);
	  return;
	}
	if ((GET_CLASS(ch) == CLASS_CLERIC) &&
	    (spell_info[spl].min_level_cleric > GET_LEVEL(ch))) {
	  send_to_char("Sorry, you can't do that.\n\r", ch);
	  return;
	}
	if ((GET_CLASS(ch) == CLASS_BARD) &&
	    (spell_info[spl].min_level_bard > GET_LEVEL(ch))) {
	  send_to_char("Sorry, you can't do that.\n\r", ch);
	  return;
	}
	if ((GET_CLASS(ch) == CLASS_KAI) &&
	    (spell_info[spl].min_level_kai > GET_LEVEL(ch))) {
	  send_to_char("Sorry, you can't do that.\n\r", ch);
	  return;
	}
	if ((GET_CLASS(ch) == CLASS_DRAKKHAR) &&
	    (spell_info[spl].min_level_drakkhar > GET_LEVEL(ch))) {
	  send_to_char("Sorry, you can't do that.\n\r", ch);
	  return;
	}
	if ((GET_CLASS(ch) == CLASS_DKNIGHT) &&
	    (spell_info[spl].min_level_dknight > GET_LEVEL(ch))) {
	  send_to_char("Sorry, you can't do that.\n\r", ch);
	  return;
	}
	if ((GET_CLASS(ch) == CLASS_EKNIGHT) &&
	    (spell_info[spl].min_level_eknight > GET_LEVEL(ch))) {
	  send_to_char("Sorry, you can't do that.\n\r", ch);
	  return;
	}
      }

      argument+=qend+1; /* Point to the last ' */
      for(;*argument == ' '; argument++);

      /* **************** Locate targets **************** */

      target_ok = FALSE;
      tar_char = 0;
      tar_obj = 0;
      
      if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {
	argument = one_argument(argument, name);
	if (*name) {
	  if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
	    if ((tar_char = get_char_room_vis(ch, name)))
	      target_ok = TRUE;

	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
	    if ((tar_char = get_char_vis(ch, name)))
	      target_ok = TRUE;

	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
	    if ((tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)))
	      target_ok = TRUE;

	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
	    if ((tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
	      target_ok = TRUE;

	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    if ((tar_obj = get_obj_vis(ch, name)))
	      target_ok = TRUE;

	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
	    for(i=0; i<MAX_WEAR && !target_ok; i++)
	      if (ch->equipment[i] 
		  && str_cmp(name, ch->equipment[i]->name) == 0) {
		tar_obj = ch->equipment[i];
		target_ok = TRUE;
	      }
	  }

	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
	    if (str_cmp(GET_NAME(ch), name) == 0) {
	      tar_char = ch;
	      target_ok = TRUE;
	    }
	} else { /* No argument was typed */
	  if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
	    if (ch->specials.fighting) {
	      tar_char = ch;
	      target_ok = TRUE;
	    }
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
	    if (ch->specials.fighting) {
	      /* WARNING, MAKE INTO POINTER */
	      tar_char = ch->specials.fighting;
	      target_ok = TRUE;
	    }
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
	    tar_char = ch;
	    target_ok = TRUE;
	  }
	}
      } else {
	target_ok = TRUE; /* No target, is a good target */
      }

      if (!target_ok) {
	if (*name) {
	  if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
	    send_to_char("Nobody here by that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
	    send_to_char("Nobody playing by that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
	    send_to_char("You are not carrying anything like that.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
	    send_to_char("Nothing here by that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    send_to_char("Nothing at all by that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
	    send_to_char("You are not wearing anything like that.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    send_to_char("Nothing at all by that name.\n\r", ch);
	} else { /* Nothing was given as argument */
	  if (spell_info[spl].targets < TAR_OBJ_INV)
	    send_to_char("Who should the spell be cast upon?\n\r", ch);
	  else
	    send_to_char("What should the spell be cast upon?\n\r", ch);
	}
	return;
      } else { /* TARGET IS OK */
	if ((tar_char==ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
	  send_to_char("You cannot cast this spell upon yourself.\n\r", ch);
	  return;
	} else if ((tar_char != ch) 
		   && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
	  send_to_char("You can only cast this spell upon yourself.\n\r", ch);
	  return;
	} else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
	  send_to_char("You are afraid that it could harm your master.\n\r", ch);
	  return;
	}
      }

      if (GET_LEVEL(ch) < IMO_LEV3) {
	if (GET_MANA(ch) < USE_MANA(ch, spl)) {
	  send_to_char("You can't summon enough energy to cast the spell.\n\r", ch);
	  return;
	}
      }

      if (spl != SPELL_VENTRILOQUATE)  /* :-) */
	say_spell(ch, spl);

      /* synchronise spells with fighting */
      WAIT_STATE(ch, spell_info[spl].beats);
      
      if ((spell_info[spl].spell_pointer == 0) && spl>0)
	send_to_char("Sorry, this magic has not yet been implemented :(\n\r", ch);
      else {
	if (GET_LEVEL(ch) >= IMO_LEV3) {
	  ((*spell_info[spl].spell_pointer) 
	   (GET_LEVEL(ch), ch, argument, SPELL_TYPE_SPELL, tar_char, tar_obj));
	  return;
	}
	if (number(1,101) > ch->skills[spl].learned) { 
	  /* 101% is failure */
	  ansi_act("You lost your concentration!",FALSE,ch,0,0,TO_CHAR,WHITE);
	  GET_MANA(ch) -= (USE_MANA(ch, spl)>>1);
	  return;
	}
	GET_MANA(ch) -= (USE_MANA(ch, spl));
	sprintf(buf, "You use %d mana to cast the spell.", USE_MANA(ch, spl));
	ansi_act(buf, FALSE, ch, 0, 0, TO_CHAR,WHITE);
	((*spell_info[spl].spell_pointer) (GET_LEVEL(ch), ch, argument, SPELL_TYPE_SPELL, tar_char, tar_obj));
      }
    }  /* if GET_POS < min_pos */
    return;
  }

  switch (number(1,5)){
  case 1: send_to_char("Bylle Grylle Grop Gryf???\n\r", ch); break;
  case 2: send_to_char("Olle Bolle Snop Snyf?\n\r",ch); break;
  case 3: send_to_char("Olle Grylle Bolle Bylle?!?\n\r",ch); break;
  case 4: send_to_char("Gryffe Olle Gnyffe Snop???\n\r",ch); break;
  default: send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r",ch); break;
  }
}


void assign_spell_pointers(void)
{
   int i;

   for(i=0; i<MAX_SPL_LIST; i++)
      spell_info[i].spell_pointer = 0;

   /* From spells1.c */
   /* Offensive spells */

   SPELLO(32,12,POSITION_FIGHTING, 1, IMO_LEV, 15, 
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,2,IMO_LEV,IMO_LEV,2,IMO_LEV,
	  cast_magic_missile);

   SPELLO( 8,12,POSITION_FIGHTING, 4, IMO_LEV, 15,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,10,IMO_LEV,IMO_LEV,6,IMO_LEV,
	  cast_chill_touch);

   SPELLO( 5,12,POSITION_FIGHTING, 7, IMO_LEV, 15,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,16,IMO_LEV,IMO_LEV,10,IMO_LEV,
	  cast_burning_hands);

   SPELLO(37,12,POSITION_FIGHTING, 10, IMO_LEV, 15,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,18,IMO_LEV,IMO_LEV,14,IMO_LEV,
	  cast_shocking_grasp);

   SPELLO(30,12,POSITION_FIGHTING, 13, IMO_LEV, 15,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,23,IMO_LEV,IMO_LEV,18,IMO_LEV,
	  cast_lightning_bolt);

   SPELLO(10,12,POSITION_FIGHTING, 16,IMO_LEV, 15,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,27,IMO_LEV,IMO_LEV,22,IMO_LEV,
	  cast_colour_spray);

   SPELLO(25,12,POSITION_FIGHTING, 15,IMO_LEV, 35,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,25,IMO_LEV,IMO_LEV,20,IMO_LEV,
	  cast_energy_drain);

   SPELLO(26,12,POSITION_FIGHTING, 19,IMO_LEV, 15,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,35,IMO_LEV,IMO_LEV,26,IMO_LEV,
	  cast_fireball);

   SPELLO(23,12,POSITION_FIGHTING, IMO_LEV, 7, 10,
	  TAR_IGNORE,IMO_LEV,IMO_LEV,IMO_LEV,8,IMO_LEV,cast_earthquake);

   SPELLO(22,12,POSITION_FIGHTING, IMO_LEV, 10, 15,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,IMO_LEV,15,IMO_LEV,IMO_LEV,IMO_LEV,
	  cast_dispel_evil);

   SPELLO( 6,12,POSITION_FIGHTING, IMO_LEV, 12, 15,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,IMO_LEV,IMO_LEV,13,IMO_LEV,IMO_LEV,
	  cast_call_lightning);

   SPELLO(27,12,POSITION_FIGHTING, IMO_LEV, 15, 25,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,IMO_LEV,IMO_LEV,19,IMO_LEV,IMO_LEV,
	  cast_harm);

   SPELLO(SPELL_CONFLAGRATION, 12,POSITION_FIGHTING, 25, IMO_LEV, 15,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT,IMP_LEV,IMP_LEV,30,IMP_LEV,IMP_LEV,
	  cast_conflagration);

   /* Spells2.c */

   SPELLO( 1,12,POSITION_STANDING, 5,  1, 5, 
	  TAR_CHAR_ROOM,6,5,5,IMO_LEV,6,cast_armor);

   SPELLO( 2,12,POSITION_FIGHTING, 8, IMO_LEV, 35,
	  TAR_SELF_ONLY,13,IMO_LEV,IMO_LEV,IMO_LEV,13,cast_teleport);

   SPELLO( 3,12,POSITION_STANDING,IMO_LEV,  5, 5,
	  TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM,IMO_LEV,8,IMO_LEV,
	  IMO_LEV,IMO_LEV,cast_bless);

   SPELLO( 4,12,POSITION_STANDING, 8,  6, 5,
	  TAR_CHAR_ROOM,15,IMO_LEV,7,8,IMO_LEV,cast_blindness);

   SPELLO(7,12,POSITION_STANDING, 14, IMO_LEV, 5,
	  TAR_CHAR_ROOM | TAR_SELF_NONO,22,IMO_LEV,IMO_LEV,IMO_LEV,19,
	  cast_charm_person);

   SPELLO( 9,12,POSITION_STANDING,11, IMO_LEV, 20,
	  TAR_CHAR_ROOM,17,IMO_LEV,IMO_LEV,IMO_LEV,15,cast_improved_invis);

   SPELLO(11,12,POSITION_STANDING,10, 13, 25,
	  TAR_IGNORE,IMO_LEV,IMO_LEV,15,IMO_LEV,IMO_LEV,cast_control_weather);

   SPELLO(12,12,POSITION_STANDING,IMO_LEV,  3, 5,
	  TAR_IGNORE,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,cast_create_food);

   SPELLO(13,12,POSITION_STANDING,IMO_LEV,  2, 5,
	  TAR_OBJ_INV | TAR_OBJ_EQUIP,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,
	  cast_create_water);

   SPELLO(14,12,POSITION_STANDING,IMO_LEV,  4, 5,
	  TAR_CHAR_ROOM,IMO_LEV,7,IMO_LEV,IMO_LEV,IMO_LEV,cast_cure_blind);

   SPELLO(15,12,POSITION_FIGHTING,IMO_LEV,  9, 15,
	  TAR_CHAR_ROOM,IMO_LEV,9,IMO_LEV,IMO_LEV,IMO_LEV,cast_cure_critic);

   SPELLO(16,12,POSITION_FIGHTING,IMO_LEV,  1, 10,
	  TAR_CHAR_ROOM,IMO_LEV,1,IMO_LEV,IMO_LEV,IMO_LEV,cast_cure_light);

   SPELLO(17,12,POSITION_STANDING,12, 14, 20,
	  TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP,20,
	  IMO_LEV,21,12,IMO_LEV,cast_curse);

   SPELLO(18,12,POSITION_STANDING,IMO_LEV,  4, 5,
	  TAR_CHAR_ROOM | TAR_SELF_ONLY,IMO_LEV,4,4,IMO_LEV,IMO_LEV,
	  cast_detect_evil);

   SPELLO(19,12,POSITION_STANDING, 2,  5, 5,
	  TAR_CHAR_ROOM | TAR_SELF_ONLY,4,6,6,IMO_LEV,4,
	  cast_detect_invisibility);

   SPELLO(20,12,POSITION_STANDING, 2,  3, 5,
	  TAR_CHAR_ROOM | TAR_SELF_ONLY,3,3,3,IMO_LEV,3,cast_detect_magic);

   SPELLO(21,12,POSITION_STANDING,IMO_LEV,  2, 5,
	  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP,IMO_LEV,2,2,IMO_LEV,
	  IMO_LEV,cast_detect_poison);

   SPELLO(24,12,POSITION_STANDING,12, IMO_LEV, 100,
	  TAR_OBJ_INV | TAR_OBJ_EQUIP,19,IMO_LEV,IMO_LEV,IMO_LEV,17,
	  cast_enchant_weapon);

   SPELLO(28,12,POSITION_FIGHTING,IMO_LEV, 14, 25,
	  TAR_CHAR_ROOM,IMO_LEV,19,IMO_LEV,IMO_LEV,IMO_LEV,cast_heal);

   SPELLO(29,12,POSITION_STANDING, 4, IMO_LEV, 5,
	  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP,7,IMO_LEV,
	  IMO_LEV,IMO_LEV,7,cast_invisibility);

   SPELLO(31,12,POSITION_STANDING, 6, IMO_LEV, 20,
	  TAR_OBJ_WORLD,9,IMO_LEV,17,IMO_LEV,9,cast_locate_object);

   SPELLO(33,12,POSITION_STANDING,IMO_LEV,  8, 10,
	  TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP,IMO_LEV,
	  IMO_LEV,10,IMO_LEV,IMO_LEV,cast_poison);

   SPELLO(34,12,POSITION_STANDING,IMO_LEV,  6, 5,
	  TAR_CHAR_ROOM | TAR_SELF_ONLY,IMO_LEV,12,IMO_LEV,IMO_LEV,IMO_LEV,
	  cast_protection_from_evil);

   SPELLO(35,12,POSITION_STANDING,IMO_LEV, 12, 5,
	  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM,IMO_LEV,
	  17,IMO_LEV,IMO_LEV,IMO_LEV,cast_remove_curse);

   SPELLO(36,12,POSITION_STANDING,IMO_LEV, 13, 75,
	  TAR_CHAR_ROOM,IMO_LEV,25,25,IMO_LEV,IMO_LEV,cast_sanctuary);

   SPELLO(38,12,POSITION_STANDING,14, IMO_LEV, 20,
	  TAR_CHAR_ROOM,IMO_LEV,24,IMO_LEV,16,IMO_LEV,cast_sleep);

   SPELLO(39,12,POSITION_STANDING, 7, IMO_LEV, 20,
	  TAR_CHAR_ROOM,12,IMO_LEV,IMO_LEV,IMO_LEV,16,cast_strength);

   SPELLO(40,12,POSITION_STANDING,IMO_LEV,  8, 50,
	  TAR_CHAR_WORLD,IMO_LEV,IMO_LEV,12,IMO_LEV,IMO_LEV,cast_summon);

   SPELLO(41,12,POSITION_STANDING, 1, IMO_LEV, 5,
	  TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO,1,IMO_LEV,IMO_LEV,
	  IMO_LEV,2,cast_ventriloquate);

   SPELLO(42,12,POSITION_STANDING,IMO_LEV, 11, 5,
	  TAR_CHAR_ROOM | TAR_SELF_ONLY,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,
	  IMO_LEV,cast_word_of_recall);

   SPELLO(43,12,POSITION_STANDING,IMO_LEV,  9, 5,
	  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM,IMO_LEV,10,IMO_LEV,
	  IMO_LEV,IMO_LEV,cast_remove_poison);

   SPELLO(44,12,POSITION_STANDING,IMO_LEV,  7, 5,
	  TAR_CHAR_ROOM | TAR_SELF_ONLY,IMO_LEV,13,IMO_LEV,IMO_LEV,IMO_LEV,
	  cast_sense_life);

   SPELLO(45,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(46,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(47,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(48,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(49,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(50,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(51,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(52,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
 
   SPELLO(53,1,POSITION_STANDING,25,25, 100, TAR_OBJ_INV | TAR_OBJ_ROOM,
	  29,IMO_LEV,IMO_LEV,IMO_LEV,30,cast_identify);
   SPELLO(54,1,POSITION_FIGHTING,IMO_LEV,9, 15, TAR_CHAR_ROOM,
	  IMO_LEV,IMO_LEV,9,IMO_LEV,IMO_LEV,cast_cause_critic);
   SPELLO(55,1,POSITION_FIGHTING,IMO_LEV,1, 10, TAR_CHAR_ROOM,
	  IMO_LEV,IMO_LEV,1,IMO_LEV,IMO_LEV,cast_cause_light);

   SPELLO(56,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(57,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(58,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(59,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(60,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(61,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(62,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(63,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(64,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(65,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(66,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(67,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);

   SPELLO(SPELL_DARKSIGHT,3,POSITION_FIGHTING, 30, 20, 15, TAR_CHAR_ROOM,
	  39,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,cast_darksight);

   SPELLO(69,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(70,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);
   SPELLO(71,0,POSITION_STANDING,MAX_LEV,MAX_LEV,200, TAR_IGNORE,MAX_LEV,
	  MAX_LEV,MAX_LEV,MAX_LEV,MAX_LEV, 0);

   SPELLO(SPELL_RESTORATION,3,POSITION_FIGHTING, IMO_LEV, 27, 25, 
	  TAR_CHAR_ROOM,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,
	  cast_restoration);
   SPELLO(SPELL_REGENERATION,3,POSITION_FIGHTING, IMO_LEV, 27, 25, 
	  TAR_CHAR_ROOM, IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,
	  cast_regeneration);
   SPELLO(SPELL_TURN_UNDEAD,3,POSITION_FIGHTING, IMO_LEV, 1, 10, 
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT, IMO_LEV, 5, 5,IMO_LEV,IMO_LEV,
	  cast_turn_undead);
   SPELLO(SPELL_SUCCOR,3,POSITION_FIGHTING, 27,IMO_LEV, 50, 
	  TAR_CHAR_WORLD, 37,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,cast_succor);
   SPELLO(SPELL_DONATE_MANA,3,POSITION_FIGHTING, IMO_LEV, 29, 15, 
	  TAR_CHAR_ROOM, IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,
	  cast_donate_mana);
   SPELLO(SPELL_MAGIC_RESIST,3,POSITION_FIGHTING, 29, 29, 50, TAR_CHAR_ROOM,
	  IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,cast_magic_resist);
   SPELLO(SPELL_MAGIC_IMMUNE,3,POSITION_FIGHTING, 29, 35, 100, TAR_CHAR_ROOM,
	  IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,cast_magic_immune);
   SPELLO(SPELL_BREATHWATER,3,POSITION_FIGHTING, 32,22, 15, TAR_CHAR_ROOM,
	  38,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,cast_breathwater);
   SPELLO(SPELL_SUSTENANCE,3,POSITION_FIGHTING, IMO_LEV, 33, 25, TAR_CHAR_ROOM,
	  IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,IMO_LEV,cast_sustenance);
}


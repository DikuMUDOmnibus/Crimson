/* ************************************************************************
*  file: spells1.c , handling of magic.                   Part of DIKUMUD *
*  Usage : Procedures handling all offensive magic.                       *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"



void cast_burning_hands( sbyte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
   switch (type) {
      case SPELL_TYPE_SPELL:
         spell_burning_hands(level, ch, victim, 0); 
         break;
    default : 
      log("Serious screw-up in burning hands!");
      break;
   }
}


void cast_call_lightning( sbyte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  extern struct weather_data weather_info;

   switch (type) {
      case SPELL_TYPE_SPELL:
         if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
            spell_call_lightning(level, ch, victim, 0);
         } else {
            send_to_char("You fail to call upon the lightning from the sky!\n\r", ch);
         }
         break;
      case SPELL_TYPE_POTION:
         if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
            spell_call_lightning(level, ch, ch, 0);
         }
         break;
      case SPELL_TYPE_SCROLL:
         if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
            if(victim) 
               spell_call_lightning(level, ch, victim, 0);
            else if(!tar_obj) spell_call_lightning(level, ch, ch, 0);
         }
         break;
      case SPELL_TYPE_STAFF:
         if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
            for (victim = world[ch->in_room].people ;
                 victim ; victim = victim->next_in_room )
               if(victim != ch)
                  spell_call_lightning(level, ch, victim, 0);
         }
         break;
      default : 
         log("Serious screw-up in call lightning!");
         break;
   }
}


void cast_chill_touch( sbyte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_chill_touch(level, ch, victim, 0);
         break;
      default : 
         log("Serious screw-up in chill touch!");
         break;
   }
}


void cast_shocking_grasp( sbyte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_shocking_grasp(level, ch, victim, 0);
         break;
      default : 
         log("Serious screw-up in shocking grasp!");
         break;
   }
}


void cast_colour_spray( sbyte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_colour_spray(level, ch, victim, 0);
         break; 
    case SPELL_TYPE_SCROLL:
         if(victim) 
            spell_colour_spray(level, ch, victim, 0);
         else if (!tar_obj)
            spell_colour_spray(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim) 
            spell_colour_spray(level, ch, victim, 0);
         break;
    default : 
         log("Serious screw-up in colour spray!");
         break;
   }
}


void cast_earthquake( sbyte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_WAND:
         spell_earthquake(level, ch, 0, 0);
         break;
    default : 
         log("Serious screw-up in earthquake!");
         break;
   }
}


void cast_energy_drain( sbyte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_energy_drain(level, ch, victim, 0);
         break;
    case SPELL_TYPE_POTION:
         spell_energy_drain(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
            spell_energy_drain(level, ch, victim, 0);
         else if(!tar_obj)
            spell_energy_drain(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
            spell_energy_drain(level, ch, victim, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (victim = world[ch->in_room].people ;
              victim ; victim = victim->next_in_room )
            if(victim != ch)
               spell_energy_drain(level, ch, victim, 0);
         break;
    default : 
         log("Serious screw-up in energy drain!");
         break;
   }
}


void cast_fireball( sbyte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
        spell_fireball(level, ch, victim, 0);
       break;
    case SPELL_TYPE_SCROLL:
         if(victim)
            spell_fireball(level, ch, victim, 0);
         else if(!tar_obj)
            spell_fireball(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
            spell_fireball(level, ch, victim, 0);
         break;
    default : 
         log("Serious screw-up in fireball!");
         break;

   }
}

void cast_turn_undead( sbyte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
        spell_turn_undead(level, ch, victim, 0);
       break;
    case SPELL_TYPE_SCROLL:
         if(victim)
            spell_turn_undead(level, ch, victim, 0);
         else if(!tar_obj)
            spell_turn_undead(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
            spell_turn_undead(level, ch, victim, 0);
         break;
    default : 
         log("Serious screw-up in turn_undead!");
         break;

   }
}


void cast_harm( sbyte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
   switch (type) {
    case SPELL_TYPE_SPELL:
         spell_harm(level, ch, victim, 0);
         break;
    case SPELL_TYPE_POTION:
         spell_harm(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (victim = world[ch->in_room].people ;
              victim ; victim = victim->next_in_room )
            if(victim != ch)
               spell_harm(level, ch, victim, 0);
         break;
    default : 
         log("Serious screw-up in harm!");
         break;

  }
}

void cast_conflagration(sbyte level, struct char_data *ch, char *arg, int type,   struct char_data *victim, struct obj_data *tar_obj)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_conflagration(level, ch, victim, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if(victim)
      spell_conflagration(level, ch, victim, 0);
    else if(!tar_obj)
      spell_conflagration(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if(victim)
      spell_conflagration(level, ch, victim, 0);
    break;
    default : 
      log("Serious screw-up in conflagration!");
    break;
    }
}


void cast_lightning_bolt( sbyte level, struct char_data *ch, char *arg, 
			 int type, struct char_data *victim, 
			 struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
         spell_lightning_bolt(level, ch, victim, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(victim)
            spell_lightning_bolt(level, ch, victim, 0);
         else if(!tar_obj)
            spell_lightning_bolt(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
            spell_lightning_bolt(level, ch, victim, 0);
         break;
    default : 
         log("Serious screw-up in lightning bolt!");
         break;

  }
}


void cast_magic_missile( sbyte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_magic_missile(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
         if(victim)
            spell_magic_missile(level, ch, victim, 0);
         else if(!tar_obj)
            spell_magic_missile(level, ch, ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(victim)
            spell_magic_missile(level, ch, victim, 0);
         break;
    default : 
         log("Serious screw-up in magic missile!");
         break;

  }
}


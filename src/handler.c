/* ************************************************************************
*  file: handler.c , Handler module.                      Part of DIKUMUD *
*  Usage: Various routines for moving about objects/players               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */
	
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "structs.h"
#include "utility.h"
#include "interpreter.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "fight.h"
#include "spells.h"
#include "constants.h"
#include "ansi.h"

/* External procedures */
void remove_follower(struct char_data *ch);
struct time_info_data age(struct char_data *ch);

/* not really sure where to put this but good a place as any I suppose */
void do_sys(char *arg, int min_level)
{
	struct descriptor_data *i;

    	for (i = descriptor_list; i; i = i->next)
  	if (!i->connected && !IS_SET(i->character->specials.act, PLR_NOSYS) && GET_LEVEL(i->character) >= min_level)
		ansi_act(arg, 0, i->character, 0, 0, TO_CHAR, CLR_SYS);
}
   
   
char *fname(char *namelist)
{
	static char holder[30];
	register char *point;
	
	if (!strncmp(namelist,"obj ",4) || 
	    !strncmp(namelist,"mob ",4))
		namelist += 4; /* skip leading "obj " or "mob " */
	for (point = holder; isalpha(*namelist); namelist++, point++)
		*point = *namelist;

	*point = '\0';

	return(holder);
}

int isname(char *str, char *namelist)
{
	register char *curname, *curstr;

	if (!str || !namelist) return 0;
	curname = namelist;
	for (;;){
		for (curstr = str;; curstr++, curname++){
			if (!*curstr)
				return(1);

			if (!*curname)
				return(0);

			if (!*curstr || *curname == ' ')
				break;

			if (LOWER(*curstr) != LOWER(*curname))
				break;
		}

		/* skip to next name */
		for (; isalpha(*curname); curname++);
		if (!*curname)
			return(0);
		curname++;			/* first char of new name */
	}
}
int isexactname(char *str, char *namelist)
{
	register char *curname, *curstr;

	if (!str || !namelist) return 0;
	curname = namelist;
	for (;;){
		for (curstr = str;; curstr++, curname++){
			if (!*curstr && !isalpha(*curname))
				return(1);

			if (!*curname)
				return(0);

			if (!*curstr || *curname == ' ')
				break;

			if (LOWER(*curstr) != LOWER(*curname))
				break;
		}

		/* skip to next name */
		for (; isalpha(*curname); curname++);
		if (!*curname)
			return(0);
		curname++;			/* first char of new name */
	}
}




void affect_modify(struct char_data *ch, signed char loc, signed char mod, long bitv, bool add)
{
	struct affected_type af;
	char buf[256];
	
	af.duration  = -1;
	af.modifier  = 0;  
	af.location  = APPLY_NONE; 
	af.bitvector = 0; 
        if (add) {
		if (bitv)
			SET_BIT(ch->specials.affected_by, bitv);
	} else {
		if (bitv)
			REMOVE_BIT(ch->specials.affected_by, bitv);
		mod = -mod;
        }

	switch(loc)
	{
		case APPLY_NONE:
			break;

		case APPLY_STR:
		case APPLY_GOD_STR:
			GET_STR(ch) +=mod;
			break;

		case APPLY_DEX:
		case APPLY_GOD_DEX:
			GET_DEX(ch) +=mod;
			break;

		case APPLY_INT:
		case APPLY_GOD_INT:
			GET_INT(ch) +=mod;
			break;

		case APPLY_WIS:
		case APPLY_GOD_WIS:
			GET_WIS(ch) +=mod;
			break;

		case APPLY_CON:
		case APPLY_GOD_CON:
			GET_CON(ch) +=mod;
			break;

		case APPLY_SEX:			
		case APPLY_CLASS:
		case APPLY_LEVEL:
			break;
		
		case APPLY_AGE: 
			ch->player.time.birth -= mod*SECS_PER_MUD_YEAR;
			if (age(ch).year < 1){
			  send_to_char("Oh Oh that item just de-aged you prior to birth... (you're now dead)\n\r", ch);
			  death_cry(ch);
			  extract_char(ch);
			} 
			break;

		case APPLY_CHAR_WEIGHT:
			GET_WEIGHT(ch) +=mod;
			break;

		case APPLY_CHAR_HEIGHT:
			GET_HEIGHT(ch) +=mod;
			break;

		case APPLY_MANA:
			ch->points.max_mana += mod; 
			break;

		case APPLY_HIT:
			ch->points.max_hit += mod; 
			break;

		case APPLY_MOVE:
			ch->points.max_move += mod;
			break;

		case APPLY_GOLD:
		case APPLY_EXP:
			break;

		case APPLY_AC:
			GET_AC(ch) += mod;
			break;

		case APPLY_HITROLL:
			GET_HITROLL(ch) +=mod;
			break;

		case APPLY_DAMROLL:
			GET_DAMROLL(ch) +=mod;
			break;

		case APPLY_SAVING_PARA:
			ch->specials.apply_saving_throw[0] += mod;
			break;

		case APPLY_SAVING_ROD:
			ch->specials.apply_saving_throw[1] += mod;
			break;

		case APPLY_SAVING_PETRI:
			ch->specials.apply_saving_throw[2] += mod;
			break;

		case APPLY_SAVING_BREATH:
			ch->specials.apply_saving_throw[3] += mod;
			break;

		case APPLY_SAVING_SPELL:
			ch->specials.apply_saving_throw[4] += mod;
			break;

		case APPLY_DETECT_INVIS:
			if (add) {
				if (!affected_by_spell(ch,SPELL_DETECT_INVISIBLE)) {
					af.type      = SPELL_DETECT_INVISIBLE;
					af.bitvector = AFF_DETECT_INVISIBLE;
	  				affect_to_char(ch, &af);
				}
			} else if(affected_by_spell(ch, SPELL_DETECT_INVISIBLE))
				affect_from_char(ch, SPELL_DETECT_INVISIBLE);
			break;
		
		case APPLY_DARKSIGHT:
			if (add) {
				if (!affected_by_spell(ch,SPELL_DARKSIGHT)) {
					af.type      = SPELL_DARKSIGHT;
					af.bitvector = AFF_DARKSIGHT;
	  				affect_to_char(ch, &af);
				}
			} else if(affected_by_spell(ch, SPELL_DARKSIGHT))
				affect_from_char(ch, SPELL_DARKSIGHT);
			break;

		case APPLY_BREATHWATER:
			if (add) {
				if (!affected_by_spell(ch,SPELL_BREATHWATER)) {
					af.type      = SPELL_BREATHWATER;
					af.bitvector = AFF_BREATHWATER;
	  				affect_to_char(ch, &af);
				}
			} else if(affected_by_spell(ch, SPELL_BREATHWATER))
				affect_from_char(ch, SPELL_BREATHWATER);
			break;

		case APPLY_DETECT_MAGIC:
			if (add) {
				if (!affected_by_spell(ch,SPELL_DETECT_MAGIC)) {
					af.type      = SPELL_DETECT_MAGIC;
					af.bitvector = AFF_DETECT_MAGIC;
	  				affect_to_char(ch, &af);
				}
			} else if(affected_by_spell(ch, SPELL_DETECT_MAGIC))
				affect_from_char(ch, SPELL_DETECT_MAGIC);
			break;

		case APPLY_SENSE_LIFE:
			if (add) {
				if (!affected_by_spell(ch,SPELL_SENSE_LIFE)) {
					af.type      = SPELL_SENSE_LIFE;
					af.bitvector = AFF_SENSE_LIFE;
	  				affect_to_char(ch, &af);
				}
			} else if(affected_by_spell(ch, SPELL_SENSE_LIFE))
				affect_from_char(ch, SPELL_SENSE_LIFE);
			break;

		case APPLY_DETECT_EVIL:
			if (add) {
				if (!affected_by_spell(ch,SPELL_DETECT_EVIL)) {
					af.type      = SPELL_DETECT_EVIL;
					af.bitvector = AFF_DETECT_EVIL;
	  				affect_to_char(ch, &af);
				}
			} else if(affected_by_spell(ch, SPELL_DETECT_EVIL))
				affect_from_char(ch, SPELL_DETECT_EVIL);
			break;

		case APPLY_INVIS:
			if (add) {
				if (!affected_by_spell(ch,SPELL_INVISIBLE)
				    && !affected_by_spell(ch,SPELL_IMPROVED_INVIS)
				) { 
					af.type      = SPELL_INVISIBLE;
					af.bitvector = AFF_INVISIBLE;
					af.location  = APPLY_AC;
					af.modifier  = -40;
	  				affect_to_char(ch, &af);
				}
			} else if (affected_by_spell(ch,SPELL_INVISIBLE)) {
				affect_from_char(ch, SPELL_INVISIBLE);
			}
			break;

		case APPLY_IMPROVED_INVIS:
			if (add) {
				if (!affected_by_spell(ch,SPELL_INVISIBLE)
				    && !affected_by_spell(ch,SPELL_IMPROVED_INVIS)
				) { 
					af.type      = SPELL_IMPROVED_INVIS;
					af.bitvector = AFF_INVISIBLE;
					af.location  = APPLY_AC;
					af.modifier  = -40;
	  				affect_to_char(ch, &af);
				}
			} else if (affected_by_spell(ch,SPELL_IMPROVED_INVIS)) {
				affect_from_char(ch, SPELL_IMPROVED_INVIS);
			}
			break;

		case APPLY_REGENERATION:
			if (add) {
				if (!affected_by_spell(ch,SPELL_REGENERATION)) {
					af.type      = SPELL_REGENERATION;
					af.bitvector = AFF_REGENERATION;
	  				affect_to_char(ch, &af);
				}
			} else if(affected_by_spell(ch, SPELL_REGENERATION))
				affect_from_char(ch, SPELL_REGENERATION);
			break;

		default:
			sprintf(buf,"unknown apply (handler.c) %d",loc);
			log(buf);
			break;
	} /* switch */
}



/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
/* this is also a really stupid way to do things, especially when you consider
   wear all and the fact that it makes spells on items impossible, hence its 
   mostly not used */
void affect_total(struct char_data *ch)
{
	struct affected_type *af;
	int i,j;

	for(i=0; i<MAX_WEAR; i++) {
		if (ch->equipment[i])
			for(j=0; j<MAX_OBJ_AFFECT; j++)
				affect_modify(ch, ch->equipment[i]->affected[j].location,
				              ch->equipment[i]->affected[j].modifier,
				              ch->equipment[i]->obj_flags.bitvector, FALSE);
	}
	for(af = ch->affected; af; af=af->next)
		affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

	ch->tmpabilities = ch->abilities;
	for(i=0; i<MAX_WEAR; i++) {
		if (ch->equipment[i])
			for(j=0; j<MAX_OBJ_AFFECT; j++)
				affect_modify(ch, ch->equipment[i]->affected[j].location,
				              ch->equipment[i]->affected[j].modifier,
				              ch->equipment[i]->obj_flags.bitvector, TRUE);
	}
	for(af = ch->affected; af; af=af->next)
		affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

	/* Make certain values are between 0..25, not < 0 and not > 25! */
	i = (IS_NPC(ch) ? 25 :18);
	i = MINV(i+GET_LEVEL(ch)/9,25);
	GET_DEX(ch) = MAXV(0,MINV(GET_DEX(ch), i));
	GET_INT(ch) = MAXV(0,MINV(GET_INT(ch), i));
	GET_WIS(ch) = MAXV(0,MINV(GET_WIS(ch), i));
	GET_CON(ch) = MAXV(0,MINV(GET_CON(ch), i));
	GET_STR(ch) = MAXV(0,MINV(GET_STR(ch), i));
}

/* This affects all of a particular location type to a char
 */
void one_affect_total(struct char_data *ch, sbyte location)
{
	struct affected_type *af;
	int i,j;

	/* since we are only going to change the the one ability dont bother
           to remove anything */ 
	/* depends on the ability in question being reset prior to this
	   being called */
	for(i=0; i<MAX_WEAR; i++) {
	  if (ch->equipment[i])
	    for(j=0; j<MAX_OBJ_AFFECT; j++)
	      if (ch->equipment[i]->affected[j].location == location)
	        affect_modify(ch, 
                  ch->equipment[i]->affected[j].location,
                  ch->equipment[i]->affected[j].modifier,
                  ch->equipment[i]->obj_flags.bitvector, TRUE);
	}
	for(af = ch->affected; af; af=af->next)
	  if (af->location == location)
	    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
}


/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
/* except it will only affect the ONE! ability is absolutely has to      */
/* called by affect_to_char, affect_from_char, equip_char & unequip_char */
/* make sure for equip_char & unequip char it gets inserted into the for */
/* loop after each call to affect_modify */
void smart_affect_total(struct char_data *ch, sbyte location)
{
	int i;

	i = (IS_NPC(ch) ? 25 :18);
	i = MINV(i+GET_LEVEL(ch)/9,25); /* calc ability maxes */

	if ((location == APPLY_DEX) || (location == APPLY_GOD_DEX)) {
	  GET_DEX(ch) = ch->abilities.dex; /* reset dex to original*/
	  one_affect_total(ch, APPLY_DEX);
	  GET_DEX(ch) = MAXV(0,MINV(GET_DEX(ch), i));
	  one_affect_total(ch, APPLY_GOD_DEX);
	  GET_DEX(ch) = MAXV(0, MINV(GET_DEX(ch), 30));
	
	} else if ((location == APPLY_INT) || (location == APPLY_GOD_INT)) {
	  GET_INT(ch) = ch->abilities.intel; /* reset int to original*/
	  one_affect_total(ch, APPLY_INT);
	  GET_INT(ch) = MAXV(0,MINV(GET_INT(ch), i));
	  one_affect_total(ch, APPLY_GOD_INT);
	  GET_INT(ch) = MAXV(0,MINV(GET_INT(ch), 30));

	} else if ((location == APPLY_WIS) || (location == APPLY_GOD_WIS)) {
	  GET_WIS(ch) = ch->abilities.wis; /* reset wis to original*/
	  one_affect_total(ch, APPLY_WIS);
	  GET_WIS(ch) = MAXV(0,MINV(GET_WIS(ch), i));
	  one_affect_total(ch, APPLY_GOD_WIS);
	  GET_WIS(ch) = MAXV(0,MINV(GET_WIS(ch), 30));

	} else if ((location == APPLY_CON) || (location == APPLY_GOD_CON)) {
	  GET_CON(ch) = ch->abilities.con; /* reset con to original*/
	  one_affect_total(ch, APPLY_CON);
	  GET_CON(ch) = MAXV(0,MINV(GET_CON(ch), i));
	  one_affect_total(ch, APPLY_GOD_CON);
	  GET_CON(ch) = MAXV(0,MINV(GET_CON(ch), 30));

	} else if ((location == APPLY_STR) || (location == APPLY_GOD_STR)) {
	  GET_STR(ch) = ch->abilities.str; /* reset str to original*/
	  one_affect_total(ch, APPLY_STR);
	  GET_STR(ch) = MAXV(0,MINV(GET_STR(ch), i));
	  one_affect_total(ch, APPLY_GOD_STR);
	  GET_STR(ch) = MAXV(0,MINV(GET_STR(ch), 30));
	} 
}




/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char( struct char_data *ch, struct affected_type *af )
{
	struct affected_type *affected_alloc;

	CREATE(affected_alloc, struct affected_type, 1);
	*affected_alloc = *af;
	affected_alloc->next = ch->affected;
	ch->affected = affected_alloc;

	affect_modify(ch, af->location, af->modifier,
	              af->bitvector, TRUE);
	smart_affect_total(ch, af->location);
}



/* Remove an affected_type structure from a char (called when duration
   reaches zero). Pointer *af must never be NIL! Frees mem and calls 
   affect_location_apply                                                */
void affect_remove( struct char_data *ch, struct affected_type *af )
{
	struct affected_type *hjp;

	assert(ch->affected);
	affect_modify(ch, af->location, af->modifier,
	              af->bitvector, FALSE);

	/* remove structure *af from linked list */
	if (ch->affected == af) {
		/* remove head of list */
		ch->affected = af->next;
	} else {
		for(hjp = ch->affected; (hjp->next) && (hjp->next != af); hjp = hjp->next);

		if (hjp->next != af) {
			log("FATAL : Could not locate affected_type in ch->affected. (handler.c, affect_remove)");
			exit(1);
		}
		hjp->next = af->next; /* skip the af element */
	}

	smart_affect_total(ch, af->location);
	free ( af );

}



/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char( struct char_data *ch, signed char skill)
{
	struct affected_type *hjp,*next;

	for(hjp = ch->affected; hjp; hjp = next) {
		next = hjp->next;  /* Have to do this here cuz hjp might be free'd */
		if (hjp->type == skill)
			affect_remove( ch, hjp ); 
	}
}



/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates 
   not affected                                                        */
bool affected_by_spell( struct char_data *ch, signed char skill )
{
	struct affected_type *hjp;

	for (hjp = ch->affected; hjp; hjp = hjp->next)
		if ( hjp->type == skill )
			return( TRUE );

	return( FALSE );
}



void affect_join( struct char_data *ch, struct affected_type *af,
                  bool avg_dur, bool avg_mod )
{
	struct affected_type *hjp, *next;
	bool found = FALSE;

	for (hjp = ch->affected; !found && hjp; hjp = next) {
                next = hjp->next;
		if ( hjp->type == af->type ) {
			
			af->duration += hjp->duration;
			if (avg_dur)
				af->duration /= 2;

			af->modifier += hjp->modifier;
			if (avg_mod)
				af->modifier /= 2;

			affect_remove(ch, hjp);
			affect_to_char(ch, af);
			found = TRUE;
		}
	}
	if (!found)
		affect_to_char(ch, af);
}

/* move a player out of a room */
void char_from_room(struct char_data *ch)
{
	struct char_data *i;

	if (ch->in_room == NOWHERE) {
		log("NOWHERE extracting char from room (handler.c, char_from_room)");
		exit(1);
	}

	if (ch->equipment[WEAR_LIGHT])
		if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
			if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) /* Light is ON */
				world[ch->in_room].light--;

	if (ch == world[ch->in_room].people)  /* head of list */
		 world[ch->in_room].people = ch->next_in_room;

	else    /* locate the previous element */
	{
		for (i = world[ch->in_room].people; 
			 i->next_in_room != ch; i = i->next_in_room);

	 	i->next_in_room = ch->next_in_room;
	}

	ch->in_room = NOWHERE;
	ch->next_in_room = 0;
}


/* place a character in a room */
void char_to_room(struct char_data *ch, int room)
{
	void raw_kill(struct char_data *ch);

	ch->next_in_room = world[room].people;
	world[room].people = ch;
	ch->in_room = room;

	if (ch->equipment[WEAR_LIGHT])
		if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
			if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) /* Light is ON */
				world[room].light++;
}


/* give an object to a char   */
void obj_to_char(struct obj_data *object, struct char_data *ch)
{
	object->next_content = ch->carrying;
	ch->carrying = object;
	object->carried_by = ch;
	object->in_room = NOWHERE;
	IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
	IS_CARRYING_N(ch)++;

	sort_obj_list(ch->carrying);
}


/* take an object from a char */
void obj_from_char(struct obj_data *object)
{
	struct obj_data *tmp;

	if (object->carried_by->carrying == object)   /* head of list */
		 object->carried_by->carrying = object->next_content;

	else
	{
		for (tmp = object->carried_by->carrying; 
			 tmp && (tmp->next_content != object); 
		      tmp = tmp->next_content); /* locate previous */

		tmp->next_content = object->next_content;
	}

	IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
	IS_CARRYING_N(object->carried_by)--;
	object->carried_by = 0;
	object->next_content = 0;
}



/* Return the effect of a piece of armor in position eq_pos */
int apply_ac(struct char_data *ch, int eq_pos)
{
  assert(ch->equipment[eq_pos]);

  if (!(GET_ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_ARMOR))
    return 0;

  switch (eq_pos) {

    case WEAR_BODY:
      return (3*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 30% */
    case WEAR_HEAD:
      return (2*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
    case WEAR_LEGS:
      return (2*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
/*  case WEAR_FEET:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    
    case WEAR_HANDS:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    
    case WEAR_ARMS:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    
*/  default:  /* used to be shield */
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
  }
  return 0;
}


void equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
  int j;

  if(!(pos>=0 && pos<MAX_WEAR)) {
    log("EQUIP: loc out of range");
    return;
  }
  if (ch->equipment[pos]) {
    log("EQUIP: Obj is equipped_by when equip.");
    return;
  }
  if (obj->carried_by) {
    log("EQUIP: Obj is carried_by when equip.");
    return;
  }
  if (obj->in_room!=NOWHERE) {
    log("EQUIP: Obj is in_room when equip.");
    return;
  }
  if (!IS_NPC(ch)) {
    if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
      if (ch->in_room != NOWHERE) {
	act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n is zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_ROOM);
	obj_to_room(obj, ch->in_room);
	return;
      } else {
	log("ch->in_room = NOWHERE when equipping char.");
      }
    }
    if ((IS_OBJ_STAT(obj, ITEM_DIFFICULT) && GET_LEVEL(ch) < 10) ||
	(IS_OBJ_STAT(obj, ITEM_EXPERT) && GET_LEVEL(ch) < 20) ||
	(IS_OBJ_STAT(obj, ITEM_HEROIC) && GET_LEVEL(ch) < 30) ||
	(IS_OBJ_STAT(obj, ITEM_AVATAR) && GET_LEVEL(ch) < 41)) {
      if (ch->in_room != NOWHERE) {
	ansi_act("You are too inexperienced to use $p.", FALSE, ch, obj, 0, TO_CHAR,CLR_ERROR);
	ansi_act("$n realizes that $e is too inexperienced to use $p.", FALSE, ch, obj, 0, TO_ROOM,CLR_ACTION);
	obj_to_room(obj, ch->in_room);
	return;
      } else {
	log("ch->in_room = NOWHERE when equipping char.");
      }
    }
    if ((GET_LEVEL(ch)<IMO_LEV2)&&(
				  ((IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && (GET_CLASS(ch)==CLASS_WARRIOR)) ||
				   (IS_OBJ_STAT(obj, ITEM_ANTI_DRAGON) && (GET_CLASS(ch)==CLASS_DRAGONW)) ||
				   (IS_OBJ_STAT(obj, ITEM_ANTI_MAGE) && (GET_CLASS(ch)==CLASS_MAGIC_USER)) ||
				   (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC) && (GET_CLASS(ch)==CLASS_CLERIC)) ||
				   (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF) && (GET_CLASS(ch)==CLASS_THIEF)) ||
				   (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF) && IS_OBJ_STAT(obj, ITEM_ANTI_MAGE) && (GET_CLASS(ch)==CLASS_BARD)) ||
				   (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && IS_OBJ_STAT(obj, ITEM_ANTI_MAGE) && (GET_CLASS(ch)==CLASS_EKNIGHT)) ||
				   (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && IS_OBJ_STAT(obj, ITEM_ANTI_MAGE) && (GET_CLASS(ch)==CLASS_DKNIGHT)) ||
				   (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC) && (GET_CLASS(ch)==CLASS_KAI)) ||
				   (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC) && (GET_CLASS(ch)==CLASS_DRAKKHAR)) ||
				   (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && (GET_CLASS(ch)==CLASS_DRAGONW))) )){
      if (ch->in_room != NOWHERE) {
	ansi_act("You are the wrong class to use that.", FALSE, ch, obj, 0, TO_CHAR,CLR_ERROR);
	ansi_act("$n realizes that $p isn't the sort of thing $e can use.", FALSE, ch, obj, 0, TO_ROOM,CLR_ERROR);
	obj_to_room(obj, ch->in_room);
	return;
      } else {
	log("ch->in_room = NOWHERE when equipping char.");
      }
    }
  }
  ch->equipment[pos] = obj;
  obj->carried_by = ch;
  
  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
    GET_AC(ch) -= apply_ac(ch, pos);
  
  for(j=0; j<MAX_OBJ_AFFECT; j++) {
    affect_modify(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, TRUE);
    smart_affect_total(ch, obj->affected[j].location);
  }
}



struct obj_data *unequip_char(struct char_data *ch, int pos)
{
	int j;
	struct obj_data *obj;

	assert(pos>=0 && pos<MAX_WEAR);
	assert(ch->equipment[pos]);

	obj = ch->equipment[pos];
	obj->carried_by = NULL;
	if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
		GET_AC(ch) += apply_ac(ch, pos);

	ch->equipment[pos] = 0;

	for(j=0; j<MAX_OBJ_AFFECT; j++) {
		affect_modify(ch, obj->affected[j].location,
		  obj->affected[j].modifier,
		  obj->obj_flags.bitvector, FALSE);
		smart_affect_total(ch, obj->affected[j].location);
	}

	return(obj);
}


int get_number(char **name) {

	int i;
	char *ppos;
        char number[MAX_INPUT_LENGTH] = "";
 
	if ( (ppos = (char *) index(*name, '.')) ){
		*ppos++ = '\0';
		strcpy(number,*name);
		strcpy(*name, ppos);

		for(i=0; *(number+i); i++)
			if (!isdigit(*(number+i)))
				return(0);

		return(atoi(number));
	}

	return(1);
}


/* Search a given list for an object, and return a pointer to that object */
struct obj_data *get_obj_in_list(char *name, struct obj_data *list)
{
	struct obj_data *i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
 
	for (i = list, j = 1; i && (j <= number); i = i->next_content)
		if (isname(tmp, i->name)) {
			if (j == number) 
				return(i);
			j++;
		}

	return(0);
}


struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
	struct obj_data *i;

	for (i = list; i; i = i->next_content)
		if (i->item_number == num) 
			return(i);
		
	return(0);
}

/* Search a given list for an object number, and return a ptr to that obj */
/* changed from stock code to search for virtual numbers not real */
struct obj_data *get_obj_in_list_virtual(int num, struct obj_data *list)
{
	struct obj_data *i;

	for (i = list; i; i = i->next_content)
		if (obj_index[i->item_number].virtual == num) 
			return(i);
		
	return(0);
}





/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj(char *name)
{
	struct obj_data *i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

	for (i = object_list, j = 1; i && (j <= number); i = i->next)
		if (isname(tmp, i->name)) {
			if (j == number)
				return(i);
			j++;
		}

	return(0);
}





/*search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
	struct obj_data *i;

	for (i = object_list; i; i = i->next)
		if (i->item_number == nr) 
			return(i);

	return(0);
}





/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, int room)
{
	struct char_data *i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
		return(0);

	for (i = world[room].people, j = 1; i && (j <= number); i = i->next_in_room)
		if (isname(tmp, GET_NAME(i))) {
			if (j == number)
        			return(i);
			j++;
		}

	return(0);
}






/* search all over the world for a char, and return a pointer if found */
struct char_data *get_char(char *name)
{
	struct char_data *i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

	for (i = character_list, j = 1; i && (j <= number); i = i->next)
		if (isname(tmp, GET_NAME(i))) {
			if (j == number)
				return(i);
			j++;
		}

	return(0);
}



/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
	struct char_data *i;

	for (i = character_list; i; i = i->next)
		if (i->nr == nr)
			return(i);

	return(0);
}



/* finds objects the same as the one at the front of the list and moves them
 to the front too */
void sort_obj_list(struct obj_data *object) {
	struct obj_data *list_obj, *middle, *old_next;

	if (!object || GET_ITEM_TYPE(object)==ITEM_ZCMD || !object->next_content) {
	  return;
	} else { /* search for chain of like objects to group with start */
	  list_obj=object->next_content; /* start at next entry in list */
	  while( (list_obj->next_content) 
               &&( (list_obj->next_content->item_number != object->item_number)
                 ||(GET_ITEM_TYPE(list_obj->next_content) != GET_ITEM_TYPE(object))
                 ||(list_obj->next_content->obj_flags.extra_flags != object->obj_flags.extra_flags) )
               )
            list_obj = list_obj->next_content;
          if (!list_obj->next_content) return; /* return if nothing found */

	  /* found something I guess move it after top of list */
	  middle = list_obj; /* this one is going to be in the middle */
	  old_next = object->next_content;
	  object->next_content = list_obj->next_content;
	  
	  while( (list_obj->next_content) 
               &&( list_obj->next_content->item_number == object->item_number )
               &&( GET_ITEM_TYPE(list_obj->next_content) == GET_ITEM_TYPE(object) )
               &&( list_obj->next_content->obj_flags.extra_flags == object->obj_flags.extra_flags )
               )
            list_obj = list_obj->next_content;
	  middle->next_content = list_obj->next_content;
	  list_obj->next_content = old_next;
	}
}

/* put an object in a room, zone commmands should be first  */
/* l8r will sort like items together somehow */
void obj_to_room(struct obj_data *object, int room)
{
	struct obj_data *room_obj;

	room_obj = world[room].contents;
	if ((!room_obj) || (room_obj->obj_flags.type_flag != ITEM_ZCMD)) {
	  object->next_content = world[room].contents;
	  world[room].contents = object;
	} else {
	  while( (room_obj->next_content)
                &&(room_obj->next_content->obj_flags.type_flag == ITEM_ZCMD) )
            room_obj = room_obj->next_content;
          object->next_content = room_obj->next_content;
          room_obj->next_content = object;
	}

	/* internal obj data update stuff */
	object->in_room = room;
	object->carried_by = 0;
	
	/* group like objects together */
	sort_obj_list(world[room].contents);
}

/* Take an object from a room */
void obj_from_room(struct obj_data *object)
{
	struct obj_data *i;

	/* remove object from room */

	if (object == world[object->in_room].contents)  /* head of list */
	   world[object->in_room].contents = object->next_content;

	else     /* locate previous element in list */
	{
		for (i = world[object->in_room].contents; i && 
		   (i->next_content != object); i = i->next_content);

		i->next_content = object->next_content;
 	}

	object->in_room = NOWHERE;
	object->next_content = 0;
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to)
{
	struct obj_data *tmp_obj;

	if (!obj || !obj_to) {
	  log("obj_to_obj: NULL object pointer");
	  return;
        }
	obj->next_content = obj_to->contains;
	obj_to->contains = obj;
	obj->in_obj = obj_to;

	for(tmp_obj = obj->in_obj; tmp_obj;
	  GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj), tmp_obj = tmp_obj->in_obj);

	sort_obj_list(obj_to->contains);
}


/* remove an object from an object */
void obj_from_obj(struct obj_data *obj)
{
	struct obj_data *tmp, *obj_from;

	if (obj->in_obj) {
		obj_from = obj->in_obj;
		if (obj == obj_from->contains)   /* head of list */
		   obj_from->contains = obj->next_content;
		else {
			for (tmp = obj_from->contains; 
				tmp && (tmp->next_content != obj);
				tmp = tmp->next_content); /* locate previous */

			if (!tmp) {
				perror("Fatal error in object structures.");
				abort();
			}

			tmp->next_content = obj->next_content;
		}


		/* Subtract weight from containers container */
		for(tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj)
			GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

		GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

		/* Subtract weight from char that carries the object */
		if (tmp->carried_by)
			IS_CARRYING_W(tmp->carried_by) -= GET_OBJ_WEIGHT(obj);

		obj->in_obj = 0;
		obj->next_content = 0;
	} else {
		perror("Trying to object from object when in no object.");
		abort();
	}
}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data *list, struct char_data *ch)
{
	if (list) {
		object_list_new_owner(list->contains, ch);
		object_list_new_owner(list->next_content, ch);
		list->carried_by = ch;
	}
}


/* Extract an object from the world */
void extract_obj(struct obj_data *obj)
{
	struct obj_data *temp1, *temp2;

	if(obj->in_room != NOWHERE)
		obj_from_room(obj);
	else if(obj->carried_by)
		obj_from_char(obj);
	else if(obj->in_obj)
	{
		temp1 = obj->in_obj;
		if(temp1->contains == obj)   /* head of list */
			temp1->contains = obj->next_content;
		else
		{
			for( temp2 = temp1->contains ;
				temp2 && (temp2->next_content != obj);
				temp2 = temp2->next_content );

			if(temp2) {
				temp2->next_content =
					obj->next_content; }
		}
	}

	for( ; obj->contains; extract_obj(obj->contains)); 
		/* leaves nothing ! */

	if (object_list == obj )       /* head of list */
		object_list = obj->next;
	else
	{
		for(temp1 = object_list; 
			temp1 && (temp1->next != obj);
			temp1 = temp1->next);
		
		if(temp1)
			temp1->next = obj->next;
	}

	if(obj->item_number>=0)
		(obj_index[obj->item_number].number)--;
	free_obj(obj);
}
		
void update_object( struct obj_data *obj, int use){

	if (obj->obj_flags.timer > 0)	obj->obj_flags.timer -= use;
	if (obj->contains) update_object(obj->contains, use);
	if (obj->next_content) update_object(obj->next_content, use);
}

void update_char_objects( struct char_data *ch )
{

	int i;

	if (ch->equipment[WEAR_LIGHT])
		if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
			if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] > 0)
				(ch->equipment[WEAR_LIGHT]->obj_flags.value[2])--;

	for(i = 0;i < MAX_WEAR;i++) 
		if (ch->equipment[i])
			update_object(ch->equipment[i],2);

	if (ch->carrying) update_object(ch->carrying,1);
}


/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(struct char_data *ch)
{
	struct obj_data *i;
	struct char_data *k, *next_char;
	struct descriptor_data *t_desc;
	int l, was_in;

	extern struct char_data *combat_list;

	void do_save(struct char_data *ch, char *argument, int cmd);
	void do_return(struct char_data *ch, char *argument, int cmd);

	void die_follower(struct char_data *ch);

	if(!IS_NPC(ch) && !ch->desc)
	{
		for(t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
			if(t_desc->original==ch)
				do_return(t_desc->character, "", 0);
	}

	if (ch->in_room == NOWHERE) {
		log("NOWHERE extracting char. (handler.c, extract_char)");
		exit(1);
	}

	if (ch->followers || ch->master)
		die_follower(ch);

   if(ch->desc) {
		/* Forget snooping */
		if (ch->desc->snoop.snooping)
			ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

		if (ch->desc->snoop.snoop_by)
			{
				send_to_char("Your victim is no longer among us.\n\r",
					ch->desc->snoop.snoop_by);
				ch->desc->snoop.snoop_by->desc->snoop.snooping = 0;
			}
		
		ch->desc->snoop.snooping = ch->desc->snoop.snoop_by = 0;
	}

	if (ch->carrying)
	{
		/* transfer ch's objects to room */
		
		if (world[ch->in_room].contents)  /* room nonempty */
		{
			/* locate tail of room-contents */
			for (i = world[ch->in_room].contents; i->next_content; 
			   i = i->next_content);

			/* append ch's stuff to room-contents */
			i->next_content = ch->carrying;
		}
		else
		   world[ch->in_room].contents = ch->carrying;

		/* connect the stuff to the room */
		for (i = ch->carrying; i; i = i->next_content)
		{
			i->carried_by = 0;
			i->in_room = ch->in_room;
		}
	}

	if (ch->specials.fighting)
		stop_fighting(ch);

	for (k = combat_list; k ; k = next_char)
	{
		next_char = k->next_fighting;
		if (k->specials.fighting == ch)
			stop_fighting(k);
	}

	/* Must remove from room before removing the equipment! */
	was_in = ch->in_room;
	char_from_room(ch);

	/* clear equipment_list */
	for (l = 0; l < MAX_WEAR; l++)
		if (ch->equipment[l])
			obj_to_room(unequip_char(ch,l), was_in);


	/* pull the char from the list */

	if (ch == character_list)  
	   character_list = ch->next;
	else
	{
		for(k = character_list; (k) && (k->next != ch); k = k->next);
		if(k)
			k->next = ch->next;
		else {
			log("Trying to remove ?? from character_list. (handler.c, extract_char)");
			abort();
		}
	}

	GET_AC(ch) = MAX_ARMOUR;

	if (ch->desc)
	{
		if (ch->desc->original)
			do_return(ch, "", 0);
		save_char(ch, NOWHERE);
          }

	if (IS_NPC(ch)) 
	{
		if (ch->nr > -1) /* if mobile */
			mob_index[ch->nr].number--;
		free_char(ch);
	}

	if (ch->desc) {
		ch->desc->connected = CON_SLCT;
		SEND_TO_Q(MENU, ch->desc);
	}
}



/* ***********************************************************************
   Here follows high-level versions of some earlier routines, ie functions
   which incorporate the actual player-data.
   *********************************************************************** */


struct char_data *get_char_room_vis(struct char_data *ch, char *name)
{
	struct char_data *i;
	int j, number;
  char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

	strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
    return(0);

	for (i = world[ch->in_room].people, j = 1; i && (j <= number); i = i->next_in_room)
		if (isname(tmp, GET_NAME(i)))
			if (CAN_SEE(ch, i))	{
				if (j == number) 
					return(i);
				j++;
			}

	return(0);
}





struct char_data *get_char_vis(struct char_data *ch, char *name)
{
	struct char_data *i;
	int j, number;
  char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

	/* check location */
	if ( (i = get_char_room_vis(ch, name)) )
		return(i);

  strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
		return(0);

	for (i = character_list, j = 1; i && (j <= number); i = i->next)
		if (isname(tmp, GET_NAME(i)))
			if (CAN_SEE(ch, i))	{
				if (j == number)
					return(i);
				j++;
			}

	return(0);
}






struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, 
				struct obj_data *list)
{
	struct obj_data *i;
	int j, number;
  char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

  strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
		return(0);

	for (i = list, j = 1; i && (j <= number); i = i->next_content)
		if (isname(tmp, i->name))
			if (CAN_SEE_OBJ(ch, i)) {
				if (j == number)
					return(i);
				j++;
			}
	return(0);
}





/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
	struct obj_data *i;
	int j, number;
  char tmpname[MAX_INPUT_LENGTH];
	char *tmp;

	/* scan items carried */
	if ( (i = get_obj_in_list_vis(ch, name, ch->carrying)) )
		return(i);

	/* scan room */
	if ( (i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)) )
		return(i);

  strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
		return(0);

	/* ok.. no luck yet. scan the entire obj list   */
	for (i = object_list, j = 1; i && (j <= number); i = i->next)
		if (isname(tmp, i->name))
			if (CAN_SEE_OBJ(ch, i)) {
				if (j == number)
					return(i);
				j++;
			}
	return(0);
}


struct obj_data *create_money( int amount )
{
	struct obj_data *obj;
	struct extra_descr_data *new_descr;
	char buf[80];

	if(amount<=0)
	{
		log("ERROR: Try to create negative money.");
		exit(1);
	}

	CREATE(obj, struct obj_data, 1);
	CREATE(new_descr, struct extra_descr_data, 1);
	clear_object(obj);

	if(amount==1)
	{
		obj->name = str_alloc("coin gold");
		obj->short_description = str_alloc("a gold coin");
		obj->description = str_alloc("One miserable gold coin.");

		new_descr->keyword = str_alloc("coin gold");
		new_descr->description = str_alloc("One miserable gold coin.");
	}
	else
	{
		obj->name = str_alloc("coins gold");
		obj->short_description = str_alloc("gold coins");
		obj->description = str_alloc("A pile of gold coins.");

		new_descr->keyword = str_alloc("coins gold");
		if(amount<10) {
			sprintf(buf,"There is %d coins.",amount);
			new_descr->description = str_alloc(buf);
		} 
		else if (amount<100) {
			sprintf(buf,"There is about %d coins",10*(amount/10));
			new_descr->description = str_alloc(buf);
		}
		else if (amount<1000) {
			sprintf(buf,"It looks like something round %d coins",100*(amount/100));
			new_descr->description = str_alloc(buf);
		}
		else if (amount<100000) {
			sprintf(buf,"You guess there is %d coins",1000*((amount/1000)+ number(0,(amount/1000))));
			new_descr->description = str_alloc(buf);
		}
		else 
			new_descr->description = str_alloc("There is A LOT of coins");			
	}

	new_descr->next = 0;
	obj->ex_description = new_descr;

	obj->obj_flags.type_flag = ITEM_MONEY;
	obj->obj_flags.wear_flags = ITEM_TAKE;
	obj->obj_flags.value[0] = amount;
	obj->obj_flags.cost = amount;
	obj->item_number = -1;

	obj->next = object_list;
	object_list = obj;

	return(obj);
}



/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(char *arg, int bitvector, struct char_data *ch,
                   struct char_data **tar_ch, struct obj_data **tar_obj)
{
	static const char *ignore[] = {
		"the",
		"in",
		"on",
		"at",
		"\n" };

	int i;
	char name[256];
	bool found;

	found = FALSE;


	/* Eliminate spaces and "ignore" words */
	while (*arg && !found) {

		for(; *arg == ' '; arg++)   ;

		for(i=0; (name[i] = *(arg+i)) && (name[i]!=' '); i++)   ;
		name[i] = 0;
		arg+=i;
		if (search_block(name, ignore, TRUE) > -1)
			found = TRUE;

	}

	if (!name[0])
		return(0);

	*tar_ch  = 0;
	*tar_obj = 0;

	if (IS_SET(bitvector, FIND_CHAR_ROOM)) {      /* Find person in room */
		if ( (*tar_ch = get_char_room_vis(ch, name)) ){
			return(FIND_CHAR_ROOM);
		}
	}

	if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
		if ( (*tar_ch = get_char_vis(ch, name)) ){
			return(FIND_CHAR_WORLD);
		}
	}

	if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
		for(found=FALSE, i=0; i<MAX_WEAR && !found; i++) /* used to be str_cmp */
			if (ch->equipment[i] && isname(name, ch->equipment[i]->name)) {
				*tar_obj = ch->equipment[i];
				found = TRUE;
			}
			if (found) {
				return(FIND_OBJ_EQUIP);
			}
	}

	if (IS_SET(bitvector, FIND_OBJ_INV)) {
		if ( (*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)) ){
			return(FIND_OBJ_INV);
		}
	}

	if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
		if ( (*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)) ){
			return(FIND_OBJ_ROOM);
		}
	}

	if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
		if ( (*tar_obj = get_obj_vis(ch, name)) ){
			return(FIND_OBJ_WORLD);
		}
	}

	return(0);
}

void extract_zone_objs(struct obj_data *obj) {
   if (obj) {
     extract_zone_objs(obj->contains);
     extract_zone_objs(obj->next_content);
     if (obj->item_number >= 0 && obj->item_number <= top_of_objt)
       if (IS_SET(zone_table[obj_index[obj->item_number].zone].flags, ZONE_TESTING)) {
         obj_from_char(obj);
         extract_obj(obj);
       }
   }
}

void char_del_zone_objs(struct char_data *ch) {
   int i;
   struct obj_data *obj;

   send_to_char("Removing objects still being tested.\n\r", ch);
   for(i=0; i<MAX_WEAR; i++)
      if ( (obj=ch->equipment[i]) ) {
        if (obj->item_number >= 0 && obj->item_number <= top_of_objt)
          if (IS_SET(zone_table[obj_index[obj->item_number].zone].flags, ZONE_TESTING)) 
            obj_to_char(unequip_char(ch, i), ch);
      }
   if (ch->carrying) 
     extract_zone_objs(ch->carrying);
   
}


/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
   struct char_data *k;

   for(k=victim; k; k=k->master) {
      if (k == ch)
         return(TRUE);
   }

   return(FALSE);
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
  struct follow_type *j, *k;

  /* assert(ch->master); */
  if (!ch->master) return;
  if (!ch->master->followers) return;

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master,TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    if (affected_by_spell(ch, SPELL_CHARM_PERSON))
      affect_from_char(ch, SPELL_CHARM_PERSON);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
  }

  if (ch->master->followers->follower == ch) { /* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    free(k);
  } else { /* locate follower who is not head of list */
    for(k = ch->master->followers; k->next->follower!=ch; k=k->next)  ;

    j = k->next;
    k->next = j->next;
    free(j);
  }
  
  ch->master = 0;
  REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
   struct follow_type *next, *k, *j, *new;
   struct char_data *leader;
   if (ch->master) {/* leader didnt die */
      stop_follower(ch);
      return;
   }

   /* next person steps up to bat */
   new = ch->followers;
   if (!(new)) return; /* no leader, no followers */

   if (new->next) { /* make new the leader */
      leader = new->follower;
      leader->master = 0; /* we're the leader ! */
      send_to_char("Gratz! you just became the new leader of the group!\n\r", leader);
      for (k = new->next; k; k=next) {
         next = k->next;
         send_to_char(GET_NAME(leader), k->follower);
         send_to_char(" is now the new leader of the group!\n\r", k->follower);
         k->follower->master = leader;
         CREATE(j, struct follow_type, 1);
         j->follower = k->follower;
         j->next = leader->followers;
         leader->followers = j;
         free(k);
      }
      free(new);
   } else { /* down to last person in group */
      stop_follower(new->follower);
   }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader)
{
   int num_mobs = 0;
   int num_chars = 0;
   struct follow_type *k;

   if(ch->master)return;
   for (k=leader->followers; k; k=k->next)
      if ((k->follower) && IS_NPC(k->follower))
         num_mobs++;
      else
         num_chars++;

   if (num_mobs + num_chars >= 10) {
      send_to_char("You can only be followed by a maximum of 10 people\n\r", leader);
      send_to_char("That person has too many followers allready\n\r", ch);
      return;
   }
   if ((IS_NPC(ch)) && (num_mobs > GET_LEVEL(leader)/10) ) {
      send_to_char("You allready being followed by the maximum number of mobs\n\r", leader);
      send_to_char("That person has too many mobs following allready\n\r", ch);
      return;
   }
   ch->master = leader;

   CREATE(k, struct follow_type, 1);
   k->follower = ch;
   k->next = leader->followers;
   leader->followers = k;

   act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
   act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
   act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}




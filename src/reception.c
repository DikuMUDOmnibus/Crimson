/**************************************************************************
*  file: reception.c, Special module for Inn's.           Part of DIKUMUD *
*  Usage: Procedures handling saving/loading of player objects            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "spells.h"
#include "constants.h"
#include "act.h"
#include "ansi.h"

#include "reception.h"

/**************************************************************************
* NEW STUFF HERE                                                          *
***************************************************************************/

int Rent_get_filename(char *orig_name, char *filename)
{
  char *ptr, name[30];

  if (!*orig_name)
    return 0;

  strcpy(name, orig_name);
  for (ptr = name; *ptr; ptr++)
    *ptr = tolower(*ptr);

  switch (tolower(*name)) {
  case 'a': case 'b': case 'c': case 'd': case 'e':
    sprintf(filename, "rentfiles/A-E/%s.objs", name); break;
  case 'f': case 'g': case 'h': case 'i': case 'j':
    sprintf(filename, "rentfiles/F-J/%s.objs", name); break;
  case 'k': case 'l': case 'm': case 'n': case 'o':
    sprintf(filename, "rentfiles/K-O/%s.objs", name); break;
  case 'p': case 'q': case 'r': case 's': case 't':
    sprintf(filename, "rentfiles/P-T/%s.objs", name); break;
  case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    sprintf(filename, "rentfiles/U-Z/%s.objs", name); break;
  default:
    sprintf(filename, "rentfiles/ZZZ/%s.objs", name); break;
  }

  return 1;
}

int Rent_delete_file(char *name)
{
  char filename[50];
  char buf1[256];
  FILE *fl;

  if (!Rent_get_filename(name, filename))
    return 0;
  if (!(fl = fopen(filename, "rb"))) {
    if (errno != ENOENT) { /* if it fails but NOT because of no file */
      sprintf(buf1, "SYSERR: deleting crash file %s (1)", filename);
      perror(buf1);
    }
    return 0;
  }
  fclose(fl);

  if (unlink(filename) < 0) {
    if (errno != ENOENT) { /* if it fails, NOT because of no file */
      sprintf(buf1, "SYSERR: deleting crash file %s (2)", filename);
      perror(buf1);
    }
  }

  return(1);
}

/**************************************************************************
* Routines used for the "Offer"                                           *
***************************************************************************/

/* redone for new system, will refuse to store items with rent < 0 */

void add_obj_cost(struct char_data *ch, struct char_data *re,
		  struct obj_data *obj, struct obj_cost *cost)
{
  char buf[MAX_STRING_LENGTH];

  /* Add cost for an item and it's contents, and next->contents */

  if (obj) {
    if ((obj->item_number > -1) && (cost->ok) 
	&& (obj->obj_flags.cost_per_day>0)) {
      cost->no_carried++;
      cost->total_cost += MAXV(0, obj->obj_flags.cost_per_day);
      sprintf(buf,"$n tells you '$p will cost %d per day.'",obj->obj_flags.cost_per_day);
      act(buf,FALSE,re,obj,ch,TO_VICT);
      add_obj_cost(ch, re, obj->contains, cost);
      add_obj_cost(ch, re, obj->next_content, cost);
    } else
      if (cost->ok) {
	if (ch->desc) {
		act("$n tells you 'I refuse to store $p.'",FALSE,re,obj,ch,TO_VICT);
		cost->ok = FALSE;
		sprintf(buf,"%s tried to rent out with %s.", GET_NAME(ch), fname(obj->name));
		log(buf);
	} else {
	        /* For auto rent, just strip the object */
	        extract_obj(obj);
		cost->ok = TRUE;
	        sprintf(buf,"%s was taken away from %s at auto rent.", fname(obj->name), GET_NAME(ch));
		log(buf);
	}	
      }
  }
}

/* much the same, okay for new system */

bool recep_offer(struct char_data *ch,  struct char_data *receptionist,
		 struct obj_cost *cost)
{
  int i;
  char buf[MAX_STRING_LENGTH];
  struct obj_data *obj;

  if (GET_LEVEL(ch) <= 5)
    cost->total_cost = 100; /* Minimum cost */
  else if (GET_LEVEL(ch) <= 9)
    cost->total_cost = 5000;
  else if (GET_LEVEL(ch) <= 19)
    cost->total_cost = 20000;
  else if (GET_LEVEL(ch) <= 29)
    cost->total_cost = 50000;
  else if (GET_LEVEL(ch) <= 40)
    cost->total_cost = 100000;
  else if (GET_LEVEL(ch) <= 41)
    cost->total_cost = 150000;
  else
    cost->total_cost = 1000;

  cost->no_carried = 0;
  cost->ok = TRUE; /* Use if any "-1" objects */
  sprintf(buf, "$n tells you, 'Your room has a base price of %d coins.'", cost->total_cost); 
  act(buf,FALSE,receptionist,0,ch,TO_VICT);

  add_obj_cost(ch, receptionist, ch->carrying, cost);

  for(i = 0; i<MAX_WEAR; i++) {
/*    Only carried items are charged */
/*    add_obj_cost(ch, receptionist, ch->equipment[i], cost); */
    obj = ch->equipment[i];
    if (obj) {
      if (obj->obj_flags.cost_per_day < 0) {
	if (ch->desc) {
	  act("$n tells you 'I refuse to store $p.'",FALSE,receptionist,ch->equipment[i],ch,TO_VICT);
	  sprintf(buf,"%s tried to rent out with %s.", GET_NAME(ch), 
fname(obj->name));
	  log(buf);
	  cost->ok = FALSE;
	} else {
	  /* for auto-rent, just strip the object */
	  extract_obj(obj);
	  sprintf(buf,"%s was taken from %s at auto-rent.", fname(obj->name),GET_NAME(ch));
	  log(buf);
	}
      } else
	cost->no_carried++;
    }
  }


  if (!cost->ok)
    return(FALSE);
   
  if (cost->no_carried == 0) {
    ansi_act("$n tells you, 'But you are not carrying anything?'",
	     FALSE,receptionist,0,ch,TO_VICT,CLR_COMM);
    return(FALSE);
  }
   
  if (GET_LEVEL(ch) <= 5) {
    cost->total_cost = MAXV(2, cost->total_cost - 1000);
    sprintf(buf, "\n$n tells you 'Tell you what kid, I'll only charge you %d coins.'", cost->total_cost);
    act(buf,FALSE,receptionist,0,ch,TO_VICT);
  }  else {
    sprintf(buf, "\n$n tells you, 'It will cost you %d coins.'", cost->total_cost);
    ansi_act(buf,FALSE,receptionist,0,ch,TO_VICT,CLR_COMM);
  }

  if (cost->total_cost > GET_GOLD(ch) + ch->points.bank) {
    if (GET_LEVEL(ch) < IMO_LEV3)
      ansi_act("$n tells you, 'Which I can see you can't afford'",
	       FALSE,receptionist,0,ch,TO_VICT,CLR_COMM);
    else {
      ansi_act("$n tells you, 'Well, since you're a God, I guess it's okay.'",
	       FALSE,receptionist,0,ch,TO_VICT,CLR_COMM);
      cost->total_cost = 0;
    }
  }

  if ( cost->total_cost > GET_GOLD(ch) )
    return(FALSE);
  else
    return(TRUE);
}

/**************************************************************************
* Routines used to load a characters equipment from disk                  *
***************************************************************************/

void obj_store_to_char(struct char_data *ch, struct obj_file_elem *st)
{
  struct obj_data *obj;
  int j;

  void obj_to_char(struct obj_data *object, struct char_data *ch);

  if (st->item_number > -1) {
    if (real_object(st->item_number) > -1) {
      obj = read_object(st->item_number, VIRTUAL);

      obj->obj_flags.value[0] = st->value[0];
      obj->obj_flags.value[1] = st->value[1];
      obj->obj_flags.value[2] = st->value[2];
      obj->obj_flags.value[3] = st->value[3];

      obj->obj_flags.extra_flags = st->extra_flags;
      obj->obj_flags.weight      = st->weight;
      obj->obj_flags.timer       = st->timer;
      obj->obj_flags.bitvector   = st->bitvector;

      for(j=0; j<MAX_OBJ_AFFECT; j++)
	obj->affected[j] = st->affected[j];

      obj_to_char(obj, ch);
    }
  }
}

void del_char_objs(struct char_data *ch)
{
  extern int terminate;

  if ((!IS_NPC(ch)) && (!terminate))
    Rent_delete_file(GET_NAME(ch)); 
}

void load_char_objs(struct char_data *ch)
{
  FILE *fl;
  char buf[256], fname[128];
  float timegold;
  struct obj_file_u    theHeader;
  struct obj_file_elem st; /* stored item var */
 
  /* r+b is for Binary Reading/Writing */
  if (!Rent_get_filename(GET_NAME(ch), fname))
    return; /* char has no name ?!? */
  if (!(fl = fopen(fname, "r+b"))) {
    if (errno != ENOENT) { /* if it fails, NOT because of no file */
      sprintf(buf, "SYSERR: READING OBJECT FILE %s (5)", fname);
      perror(buf);
      send_to_char("\n\r********************* NOTICE *********************\n\r"
		   "There was a problem loading your objects from disk.\n\r"
		   "Contact a God for assistance.\n\r", ch);
    }
    sprintf(buf, "%s entering game with no equipment.", GET_NAME(ch));
    log(buf);
    return;
  }
  sprintf(buf, "%s entering game. (recovered equip ok)", GET_NAME(ch));
  log(buf);

  fread(&theHeader, sizeof(struct obj_file_u), 1, fl);
  if (str_cmp(theHeader.owner, GET_NAME(ch))){
    sprintf(buf, "%s's rent file was saved for char %s?!?",
	    GET_NAME(ch), theHeader.owner);
    log(buf);
  }
  while (!feof(fl)) {
    fread(&st, sizeof(struct obj_file_elem), 1, fl);
    if (!feof(fl)) obj_store_to_char(ch, &st);
  }

  /* calculate the chars gold, max one days worth of rent */
  timegold = (((float) MINV( 3*SECS_PER_REAL_DAY,
			    (time(0) - theHeader.last_update)))
	      / SECS_PER_REAL_DAY * theHeader.total_cost);
  GET_GOLD(ch) = (int) (GET_GOLD(ch) - timegold);
  if (GET_GOLD(ch) < 0)
    GET_GOLD(ch) = 0;
  if (ch->points.bank < 0)
    ch->points.bank = 0;
 
  fclose(fl); /* close up the file now that we're done */

  /* Save char, to avoid strange data if crashing */
  save_char(ch, NOWHERE);
}

/**************************************************************************
* Routines used to save a characters equipment to disk                    *
***************************************************************************/

/* Puts object in store */
void put_obj_in_store(struct obj_data *obj, struct obj_file_elem *st)
{
  int j;
   
  st->item_number = obj_index[obj->item_number].virtual;
  st->value[0] = obj->obj_flags.value[0];
  st->value[1] = obj->obj_flags.value[1];
  st->value[2] = obj->obj_flags.value[2];
  st->value[3] = obj->obj_flags.value[3];

  st->extra_flags = obj->obj_flags.extra_flags;
  st->weight  = obj->obj_flags.weight;
  st->timer  = obj->obj_flags.timer;
  st->bitvector  = obj->obj_flags.bitvector;
  for(j=0; j<MAX_OBJ_AFFECT; j++)
    st->affected[j] = obj->affected[j];
}

/* Destroy inventory after transferring it to store */
void obj_to_store(FILE *fl, struct obj_data *obj, struct obj_file_elem *st,
		  struct char_data *ch)
{
  static char buf[256];
  struct obj_data *tmp_obj;

  if (obj) {
    obj_to_store(fl, obj->contains, st, ch);
    obj_to_store(fl, obj->next_content, st, ch);

    if ((obj->obj_flags.timer < 0) && (obj->obj_flags.timer != OBJ_NOTIMER)) {
      sprintf(buf, "You're told: 'The %s is just old junk, I'll throw it away for you.'\n\r",
	      fname(obj->name));
      send_to_char(buf, ch);
    } else {
      for (tmp_obj = obj->in_obj; tmp_obj; tmp_obj = tmp_obj->in_obj)
	GET_OBJ_WEIGHT(tmp_obj) -= GET_OBJ_WEIGHT(obj);
      /* thats for containers */
      put_obj_in_store(obj, st);

      /* WRITE ST TO FILE HERE */
      if (fwrite(st, sizeof(struct obj_file_elem), 1, fl) < 1) {
	sprintf(buf, "Error creating rent file for %s.", GET_NAME(ch));
	log(buf);
      }
    }
  }
}

void obj_restore_weight(struct obj_data *obj) 
{
  if (obj) {
    obj_restore_weight(obj->contains);
    obj_restore_weight(obj->next_content);
    if (obj->in_obj)
      GET_OBJ_WEIGHT(obj->in_obj) += GET_OBJ_WEIGHT(obj);
  }
}

void extract_all_objs(struct obj_data *obj) 
{
  if (obj) {
    extract_all_objs(obj->contains);
    extract_all_objs(obj->next_content);
    extract_obj(obj);
  }
}

void del_objs(struct char_data *ch) 
{
  int i;

  for(i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i]) {
      obj_to_char(unequip_char(ch, i), ch);
    }
  if (ch->carrying) {
    extract_all_objs(ch->carrying);
    ch->carrying = 0;
  }
}

/******************************************************************
* WRITE THE OBJ DATA FILE OUT, SAVE EVERYTHING NO QUESTIONS ASKED *
*******************************************************************/

/* write the vital data of a player to the player file */
void save_obj(struct char_data *ch, struct obj_cost *cost)
{
  struct obj_file_u    theHeader;
  struct obj_file_elem st; /* stored item var */
  FILE *fl;
  int i;
  static char buf[512];
 
  for(i=0; i<20; i++)
    theHeader.owner[i] = 0;
  strcpy(theHeader.owner, GET_NAME(ch));
  theHeader.gold_left = GET_GOLD(ch);
  theHeader.total_cost = cost->total_cost;
  theHeader.last_update = time(0);
  theHeader.rent_flags = 0;

  /* open the file */
  if (IS_NPC(ch)) /* cheap insurance cant happen but cant hurt */
    return;
  if (!Rent_get_filename(GET_NAME(ch), buf))
    return;
  del_char_objs(ch); /* turf old file if any */
  if (!(fl = fopen(buf, "wb")))
    return;

  /* Write the header out */
  if (fwrite(&theHeader, sizeof(struct obj_file_u), 1, fl) < 1) {
    sprintf(buf, "Error creating rent file for %s.", GET_NAME(ch));
    log(buf);
  }
 
  /* write out all the equipment */
  obj_to_store(fl, ch->carrying, &st, ch);
  obj_restore_weight(ch->carrying);
  
  for(i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i]) {
      obj_to_store(fl, ch->equipment[i], &st, ch);
    }
  /* now rewrite the header with updated information? */
  /* nah not necessary */

  fclose(fl); /* close up shop */
}

/**************************************************************************
* Routines used to update object file, upon boot time                     *
***************************************************************************/

/* time independant rent file now, temporarily disable */
void update_obj_file(void)
{
  /* will be used again ! - will prevent player from renting out forever
     FILE *fl, *char_file;
     struct obj_file_u st;
     struct char_file_u ch_st;
     struct char_data tmp_char;
     int pos, no_read, player_i;
     long days_passed, secs_lost;
     char buf[MAX_STRING_LENGTH];

     int find_name(char *name);
     extern struct player_index_element *player_table;
     */
}

/**************************************************************************
* Routine Receptionist                                                    *
***************************************************************************/

int receptionist(struct char_data *ch, int cmd, char *arg)
{
  char buf[240];
  struct obj_cost cost;
  struct char_data *recep = 0;
  struct char_data *temp_char;
  sh_int save_room;
  sh_int action_tabel[9] = {
    CMD_SMILE,
    CMD_DANCE,
    CMD_SIGH,
    CMD_BLUSH,
    CMD_BURP,
    CMD_COUGH,
    CMD_FART,
    CMD_WHISTLE,
    CMD_YAWN
    };
  int amount, tax;
  
  void do_action(struct char_data *ch, char *arg, int cmd);
  int number(int from, int to);
  
  if (!ch->desc)
    return(FALSE); /* You've forgot FALSE - NPC couldn't leave */
  
  for (temp_char = world[ch->in_room].people; (temp_char) && (!recep);
       temp_char = temp_char->next_in_room)
    if (IS_MOB(temp_char))
      if (mob_index[temp_char->nr].func == receptionist)
	recep = temp_char;

  if (!recep) {
    log("no receptionist in room.\n\r");
    exit(1);
  }

  if (IS_NPC(ch))
    return(FALSE);

  if ((cmd != 92) && (cmd != 93) && (cmd != 220)&& (cmd != 221)&& (cmd != 219))
    {
      if (!number(0, 30))
	do_action(recep, "", action_tabel[number(0,8)]);
      return(FALSE);
    }

  if (!AWAKE(recep)) {
    act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
    return(TRUE);
  }

  if (!CAN_SEE(recep, ch)) {
    act("$n says, 'I don't deal with people I can't see!'", 
	FALSE, recep, 0, 0, TO_ROOM);
    return(TRUE);
  }
  if (cmd == 92) { /* Rent  */
    if (recep_offer(ch, recep, &cost)) {
      act("$n stores your stuff in the safe, and helps you into your chamber.",
	  FALSE, recep, 0, ch, TO_VICT);
      act("$n helps $N into $S private chamber.",FALSE, recep,0,ch,TO_NOTVICT);
      save_obj(ch, &cost);
      del_objs(ch);
      save_room = ch->in_room;
      extract_char(ch);
      ch->in_room = world[save_room].number;
      save_char(ch, ch->in_room);
    }
  } else if (cmd == 93) {         /* Offer */
    recep_offer(ch, recep, &cost);
    act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
  } else if (cmd == 219) {   /* withdraw */
    arg=one_argument(arg, buf);
    if (is_number(buf)) {
      amount = atoi(buf);
      if (amount < 0) {
	send_to_char("Ha ha ha... nice try.\n\r",ch);
	return(TRUE);
      } else if (amount > ch->points.bank) {
	send_to_char("I'm afraid you don't have that much in your account.\n\r", ch);
      } else {
	act("$n hands you your money.", FALSE, recep, 0, ch, TO_VICT);
	act("$n gives $N some money.",FALSE, recep,0,ch,TO_NOTVICT);
	ch->points.gold += amount;
	ch->points.bank -= amount;
      }
    }
  } else if (cmd == 220) {   /* deposit */
    arg=one_argument(arg, buf);
    if (is_number(buf)) {
      amount = atoi(buf);
      tax = amount / 20;
      if (amount < 0) {
	send_to_char("Ha ha ha... nice try\n\r",ch);
	return(TRUE);
      } else if (amount > ch->points.gold) {
	send_to_char("I'm afraid you don't have that much.\n\r", ch);
      } else {
	sprintf(buf, "I'm afraid the tax on this transaction comes to %d.\n\r", tax);
	send_to_char(buf, ch);
	act("$n takes your money.", FALSE, recep, 0, ch, TO_VICT);
	act("$n takes some money from $N.",FALSE, recep,0,ch,TO_NOTVICT);
	ch->points.gold -= amount;
	amount -= tax;
	ch->points.bank += amount;
      }
    }
  } else if (cmd == 221) {   /* balance */
    sprintf(buf, "You have %d coins on account.\n\r", ch->points.bank);
    send_to_char(buf, ch);
  }
  
  return(TRUE);
}




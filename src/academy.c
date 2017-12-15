/*************************************************************************
 *                                                                       * 
 *  File: academy.c                                                      *
 *  Special procedure handling the Royal Academy (Mud School)            *
 *                                                                       * 
 *************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "constants.h"
#include "limits.h"
#include "act.h"
#include "spells.h"
#include "ansi.h"

static char name_list[MAX_STRING_LENGTH];  

/* SPECIAL MOBILE PROCEDURES */
 
int academy_trainer(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tmp_ch, *trainer;

  trainer = NULL;
  for (tmp_ch = world[ch->in_room].people;
       (tmp_ch) && (!trainer);
       tmp_ch = tmp_ch->next)
    if (IS_MOB(tmp_ch))
      if (mob_index[tmp_ch->nr].func == academy_trainer)
	trainer = tmp_ch;

  for (tmp_ch = world[ch->in_room].people;
       tmp_ch;
       tmp_ch = tmp_ch->next_in_room)
    if (!IS_NPC(tmp_ch)) {
      if (!affected_by_spell(tmp_ch, SPELL_BLESS)) {
	ansi_act("Pitto looks you up and down to make sure you are fit.", 
		 FALSE, tmp_ch, 0, 0, TO_CHAR, CLR_ACTION);
	cast_bless(40, trainer, "", SPELL_TYPE_SPELL, tmp_ch, 0);
      }
      if (!affected_by_spell(tmp_ch, SPELL_ARMOR)) {
	ansi_act("Pitto looks you up and down to make sure you are fit.", 
		 FALSE, tmp_ch, 0, 0, TO_CHAR, CLR_ACTION);
	cast_armor(40, trainer, "", SPELL_TYPE_SPELL, tmp_ch, 0);
      }
      if (GET_HIT(tmp_ch) < GET_MAX_HIT(tmp_ch)) {
	ansi_act("Pitto looks you up and down to make sure you are fit.", 
		 FALSE, tmp_ch, 0, 0, TO_CHAR, CLR_ACTION);
	cast_cure_light(40, trainer, "", SPELL_TYPE_SPELL, tmp_ch, 0);
      }
    }
  return(FALSE);
}

int final_exam(struct char_data *ch, int cmd, char *arg) 
{
  char buf[100];

  if (cmd != -1)
    return(FALSE); 

  if (strstr(name_list, GET_NAME(ch)) == NULL) {
    sprintf(buf, "%s passed the Royal Academy Final Exam.", GET_NAME(ch));
    do_gossip(ch, "I have graduated from the Royal Academy!!!!!!!!!", 232);
    log(buf);
    strcat(name_list, "\n\r                              ");
    strcat(name_list, GET_NAME(ch));
  }  
  return(FALSE);
}

/* SPECIAL ROOM PROCEDURES */

int academy_entrance(struct char_data *ch, int cmd, char *arg)
{
  if (cmd == 1) { /* Trying to enter the academy */
    if ((GET_LEVEL(ch) > 5) && (GET_LEVEL(ch) < IMO_LEV2)) {
      ansi_act("A divine force blocks your way north.", 
	       FALSE, ch, 0, 0, TO_CHAR, CLR_ERROR);
      ansi_act("A voice says, 'You are over qualified for the academy.'",
	       FALSE, ch, 0, 0, TO_CHAR, CLR_ERROR);
      return(TRUE);
    }
  }
  return(FALSE);
}

int hall_of_fame(struct char_data *ch, int cmd, char *arg)
{
  char buf[100];

  if (cmd == 15) {  
    one_argument(arg, buf);
    if (str_cmp(buf, "plaque"))
      return(FALSE);
    
    ansi_act("These students have graduated from the Royal Academy today:",
	     FALSE, ch, 0, 0, TO_CHAR, WHITE); 
    page_string(ch->desc, name_list, 1);
    return(TRUE);
  }
  return(FALSE);
}

/* SPECIAL OBJECT PROCEDURES */

int academy_key(struct char_data *ch, int cmd, char *arg)
{
  static int counter = 10;
  struct obj_data *obj, *KEY;

  KEY = 0;
  for (obj = ch->carrying; obj && (!KEY); obj = obj->next_content)
    if (obj->item_number >= 0) 
      if (obj_index[obj->item_number].func == academy_key) {
	KEY = obj;
	ansi_act("You found $p!", FALSE, ch, KEY, 0, TO_ROOM, CLR_ERROR);
      }

  if (!KEY)
    return (FALSE);

  if (!KEY->carried_by) 
    return (FALSE);

  if (counter > 0) {
    counter--;
    ansi_act("You'd better hurry, as you can't keep $p for too long.", 
	     FALSE, KEY->carried_by, KEY, 0, TO_CHAR, CLR_ERROR);
  }
  else {
    ansi_act("$p slips out of your hand and disappear.", 
	     FALSE, KEY->carried_by, KEY, 0, TO_CHAR, CLR_ERROR);
    extract_obj(KEY);
    counter = 10;
  }
  return (FALSE);
}



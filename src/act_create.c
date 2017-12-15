/* ************************************************************************
*  file: act_create.c, Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for the world builders   *
*         brought to you by the razor-keen intellect of Cryogen           *
*         originally written for use with the Crimson server              *
*         if ya find a bug email it and the fix to rhaksi@engr.uvic.ca    *
*                                                                         *
*  Guess I should rename the file sometime soon to act.create.c huh?      *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <memory.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "limits.h"
#include "interpreter.h"
#include "spells.h"
#include "constants.h"
#include "modify.h"
#include "ansi.h"
#include "act.h"

/* for zones */
#define ZCMD zone_table[zone].cmd
 
/* *******************************************************************
*  Special procedures for Creation!                                  *
******************************************************************** */

/*****************************************************************
 *                                                               *
 *                                                               *
 *                     Z O N    S T U F F                        *
 *                                                               *
 *                                                               *
 *****************************************************************/

void do_zlist(struct char_data *ch, char *arg, int cmd)
{
  int i, zone, start, end;
  extern int top_of_zone_table;
  char buf[16384], buf1[128], buf2[128];
  
  if ((!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))
      && (GET_LEVEL(ch) < IMO_LEV2)) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  half_chop(arg, buf1, buf2);
  
  zone = world[ch->in_room].zone;
  if (is_number(buf1))
    start = (atoi(buf1));
  else 
    start = 0;   
  if (is_number(buf2))
    end = (atoi(buf2));
  else end = top_of_zone_table;
  
  sprintf(buf, "\n\r"); /* init buf to something for strcat */
  for (i = start; (i>=0) && (i <= top_of_zone_table) && (i <= end); i++) {
    sprintf(buf1, "[%-20s] (%5d-%5d) Number of Rooms[%3d] Zonelord[%s]\n\r", 
	    zone_table[i].filename,
	    zone_table[i].bottom,
	    zone_table[i].top,
	    (!IS_SET(zone_table[i].dirty_wld, SCRAMBLED) ? 
	     (zone_table[i].real_top - zone_table[i].real_bottom +1) 
	     : (zone_table[i].top - zone_table[i].bottom + 1)),
	    zone_table[i].lord);
    strcat(buf, buf1);
  }
  strcat(buf, "\n\rEnd of zlist.\n\r");
  page_string(ch->desc, buf, 1); 
}

void do_zstat(struct char_data *ch, char *arg, int cmd)
{
  int i;
  char buf[512];
  
  i = world[ch->in_room].zone;
  if ((GET_LEVEL(ch) < IMO_LEV2) && (!isexactname(GET_NAME(ch), zone_table[i].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  sprintf(buf, "[%-20s] (%5d-%5d) Zonelord[%s]\n\r", 
	  zone_table[i].filename,
	  zone_table[i].bottom,
	  zone_table[i].top,
	  zone_table[i].lord);
  send_to_char(buf, ch);
  sprintf(buf, "Reset Mode : [%3d]\n\r", zone_table[i].reset_mode);
  send_to_char(buf, ch);
  sprintf(buf, "Lifespan   : [%3d]\n\r", zone_table[i].lifespan);
  send_to_char(buf, ch);
  sprintf(buf, "Age        : [%3d]\n\r", zone_table[i].age);
  send_to_char(buf, ch);
  send_to_char("Zone Flags : ", ch);
  sprintbit(zone_table[i].flags,zflag_bits,buf);
  strcat(buf,"\n\r");
  send_to_char(buf, ch);

  sprintf(buf, "ZCMDs:    [%3d]\n\r", zone_table[i].reset_num);
  send_to_char(buf, ch);

  sprintf(buf, "\n\rRooms:    [%3d]\n\r", 
	  (!IS_SET(zone_table[i].dirty_wld, SCRAMBLED) 
	   ? (zone_table[i].real_top - zone_table[i].real_bottom +1) 
	   : (zone_table[i].top - zone_table[i].bottom + 1)));
  send_to_char(buf, ch);

  sprintf(buf, "Mobiles:  [%3d]\n\r", 
	  (!IS_SET(zone_table[i].dirty_mob, SCRAMBLED) 
	   ? (zone_table[i].real_top - zone_table[i].real_bottom +1) 
	   : -1));
  send_to_char(buf, ch);

  sprintf(buf, "Objects:  [%3d]\n\r", 
	  (!IS_SET(zone_table[i].dirty_obj, SCRAMBLED) 
	   ? (zone_table[i].top_of_objt - zone_table[i].bot_of_objt +1) 
	   : -1));
  send_to_char(buf, ch);

  send_to_char("\n\r.zon flags: ", ch);
  sprintbit(zone_table[i].dirty_zon,dirty_bits,buf);
  strcat(buf,"\n\r");
  send_to_char(buf, ch);
  send_to_char(".mob flags: ", ch);
  sprintbit(zone_table[i].dirty_mob,dirty_bits,buf);
  strcat(buf,"\n\r");
  send_to_char(buf, ch);
  send_to_char(".obj flags: ", ch);
  sprintbit(zone_table[i].dirty_obj,dirty_bits,buf);
  strcat(buf,"\n\r");
  send_to_char(buf, ch);
}

void do_zflag(struct char_data *ch, char *arg, int cmd){
  char buf[256], buf1[128];
  int loc,i,zone;
  unsigned long flag;

  if (GET_LEVEL(ch) < IMO_LEV3) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to do that.\n\r", ch);
    ansi(END, ch);
    return;
  }
  zone = world[ch->in_room].zone;
  one_argument(arg, buf1);
  if (!*buf1) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try zflag <flag>.\n\r", ch);
    ansi(END, ch);
    sprintf(buf, "Available zone flags: \n\r");
    for (i = 0; *zflag_bits[i] != '\n'; i++) {
      strcat(buf, zflag_bits[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    ansi(CLR_ACTION, ch);
    send_to_char(buf,ch);
    ansi(END, ch);
    return;
  }
  loc = (old_search_block(buf1, 0, strlen(buf1), zflag_bits, -1));
  if (loc == -1) { /* the -1 in old_search_block lowercases the list */
    send_to_char("That flag doesn't exist.\n\r", ch);
    return;
  }
  loc--;  /* search block returns 1 as first position */

  flag = 1;
  for (i = 0; i < loc; i++)
    flag *= 2;

  if (IS_SET(zone_table[zone].flags, flag)) {
    sprintf(buf, "You have removed the '%s' flag.\n\r", zflag_bits[loc]);
    REMOVE_BIT(zone_table[zone].flags, flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  } else {
    sprintf(buf, "You have set the '%s' flag.\n\r", zflag_bits[loc]);
    SET_BIT(zone_table[zone].flags, flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  }
  SET_BIT(zone_table[zone].dirty_zon, UNSAVED);
}


void do_zreboot(struct char_data *ch, char *arg, int cmd){
  char   buf[100];
  int    zone,i;

  zone = world[ch->in_room].zone;
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (IS_SET(zone_table[zone].dirty_zon, SHOWN)) {
    ansi(CLR_ERROR, ch);
    send_to_char("ZCMDS are shown! Cancel or update changes first.\n\r", ch);
    ansi(END, ch);
    return;
  }
  for (i = MAXV(0,zone_table[zone].real_bottom); i<=top_of_world && i <= zone_table[zone].real_top; i = world[i].next)
    do_purge(ch, "\0", -1); /* -1 bypassed error/auth check */
  reset_zone(zone);
  sprintf(buf, "Done, %s *REBOOTED* (All rooms purged then reset)!\n\r", zone_table[zone].filename);
  send_to_char(buf, ch);
} /* proc */


void do_zreset(struct char_data *ch, char *arg, int cmd){
  char   buf[100];
  int    zone;

  zone = world[ch->in_room].zone;
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (IS_SET(zone_table[zone].dirty_zon, SHOWN)) {
    ansi(CLR_ERROR, ch);
    send_to_char("ZCMDS are shown! Cancel or update changes first.\n\r", ch);
    ansi(END, ch);
    return;
  }
  reset_zone(zone);
  sprintf(buf, "Done. %s reset!\n\r", zone_table[zone].filename);
  ansi(CLR_ACTION, ch);
  send_to_char(buf, ch);
  ansi(END, ch);
} /* proc */


void remove_zcmds(struct char_data *ch, int zone){
/* turf all the zcmds */
  struct obj_data *tmp_obj, *next_obj;
  int i;
  
  if (!IS_SET(zone_table[zone].dirty_zon, SHOWN)) {
    ansi(CLR_ERROR, ch);
    send_to_char("Nothing to do.\n\r", ch);
    ansi(END, ch);
    return;
  }
  /* scan the zone for zcmds */
  for (i = zone_table[zone].real_bottom; (i>=0) && (i <= top_of_world) && (i <= zone_table[zone].real_top); i=world[i].next) {
    for(tmp_obj = world[i].contents; tmp_obj; tmp_obj = next_obj) {
      next_obj = tmp_obj->next_content;
      if(tmp_obj->obj_flags.type_flag == ITEM_ZCMD)
        extract_obj(tmp_obj);
    }
  }
  ansi(CLR_ACTION, ch);
  send_to_char("Zone commands hidden.\n\r", ch);
  ansi(END, ch);
  REMOVE_BIT(zone_table[zone].dirty_zon, SHOWN);
}

void update_zcmds(struct char_data *ch, int zone){
/* turf old zcmd table and reload with visible one */
  struct obj_data *tmp_obj, *next_obj;
  int i,j=0;
  
  if (!IS_SET(zone_table[zone].dirty_zon, SHOWN)) {
    ansi(CLR_ERROR, ch);
    send_to_char("Not in ZEDIT SHOW mode.\n\r", ch);
    ansi(END, ch);
    return;
  }
  /* lose existing table */
  free(zone_table[zone].cmd);
  zone_table[zone].cmd = 0;

  /* scan the zone for zcmds */
  for (i = MAXV(0,zone_table[zone].real_bottom); (i <= top_of_world) && (i <= zone_table[zone].real_top); i=world[i].next) {
    for(tmp_obj = world[i].contents; tmp_obj; tmp_obj = next_obj) {
      next_obj = tmp_obj->next_content;
      if(tmp_obj->obj_flags.type_flag == ITEM_ZCMD){
        /* make space for a new command */
        allocate_zcmd(zone, j+1);
        ZCMD[j].command = (char) tmp_obj->obj_flags.value[0];
        ZCMD[j].arg1 = tmp_obj->obj_flags.value[1];
        ZCMD[j].arg2 = tmp_obj->obj_flags.value[2];
        /* if M or O use room instead of value[3] */
        if (ZCMD[j].command == 'M' || ZCMD[j].command == 'O')
          ZCMD[j].arg3 = i;
        else 
          ZCMD[j].arg3 = tmp_obj->obj_flags.value[3];
        ZCMD[j].if_flag = tmp_obj->obj_flags.weight;
        j++;
      }
      extract_obj(tmp_obj);
    }
  }

  /* add terminating entry */
  allocate_zcmd(zone, j+1);
  ZCMD[j].command = 'S';
  ZCMD[j].arg1 = 0;
  ZCMD[j].arg2 = 0;
  ZCMD[j].arg3 = 0;
  ZCMD[j].if_flag = 0;

  REMOVE_BIT(zone_table[zone].dirty_zon, SHOWN);
  SET_BIT(zone_table[zone].dirty_zon, UNSAVED);
  ansi(CLR_ACTION, ch);
  send_to_char("Zone updated.\n\r", ch);
  ansi(END, ch);
}


void show_zcmds(struct char_data *ch, int zone){
/* show all the zcmds so that zone lords (only) can see them */
  int i, room;
  struct obj_data *tmp_obj;
  char buf[256], name[256];

  if (IS_SET(zone_table[zone].dirty_zon, SHOWN)) {
    ansi(CLR_ERROR, ch);
    send_to_char("Already shown (to abort changes ZEDIT CANCEL).\n\r", ch);
    ansi(END, ch);
    return;
  }
  for (i=0; ZCMD[i].command != 'S'; i++){ /* check against illegal zone stuff first off */
    if ( (ZCMD[i].command != 'M')
       &&(ZCMD[i].command != 'O')
       &&(ZCMD[i].command != 'P')
       &&(ZCMD[i].command != 'G')
       &&(ZCMD[i].command != 'E')
       &&(ZCMD[i].command != 'D')
    ){ /* if its not any of those then do nothing */
    } else {
      CREATE(tmp_obj, struct obj_data, 1);
      clear_object(tmp_obj);
      sprintf(name, "obj zcmd %c ", ZCMD[i].command);
      tmp_obj->obj_flags.type_flag = ITEM_ZCMD;
      tmp_obj->obj_flags.wear_flags = 0; /* Not takeable */
      tmp_obj->obj_flags.extra_flags = ITEM_NOSEE;
      tmp_obj->obj_flags.value[0] = (char) ZCMD[i].command;
      tmp_obj->obj_flags.value[1] = ZCMD[i].arg1;
      tmp_obj->obj_flags.value[2] = ZCMD[i].arg2;
      tmp_obj->obj_flags.value[3] = ZCMD[i].arg3;
      tmp_obj->obj_flags.weight = ZCMD[i].if_flag;
      tmp_obj->obj_flags.cost = -1;
      tmp_obj->obj_flags.cost_per_day = -1;
      tmp_obj->next = object_list;
      object_list = tmp_obj;
      tmp_obj->item_number = -1;

      if (ZCMD[i].if_flag){ 
        sprintf(buf, "(IF) ");
        sprintf(name+strlen(name), "if ");
      } else
        sprintf(buf, "ZCMD ");
      switch(ZCMD[i].command) {
      case 'M':
	sprintf(buf+strlen(buf), "[Mob] Mob[%5d]", mob_index[ZCMD[i].arg1].virtual);
	sprintf(buf+strlen(buf), " (Max[%4d]) ", ZCMD[i].arg2);
	strcat(buf, fname(mob_index[ZCMD[i].arg1].name));
	strcat(name, fname(mob_index[ZCMD[i].arg1].name));
        room = ZCMD[i].arg3;
        break;
      
      case 'O':
	sprintf(buf+strlen(buf), "[Obj] Obj[%5d]", obj_index[ZCMD[i].arg1].virtual);
	sprintf(buf+strlen(buf), " (Max[%4d]) ", ZCMD[i].arg2);
	strcat(buf, fname(obj_index[ZCMD[i].arg1].name));
	strcat(name, fname(obj_index[ZCMD[i].arg1].name));
        room = ZCMD[i].arg3;
        break;
      
      case 'P':
	sprintf(buf+strlen(buf), "[Put] Obj[%5d]", obj_index[ZCMD[i].arg1].virtual);
	sprintf(buf+strlen(buf), " (Max[%4d])", ZCMD[i].arg2);
	sprintf(buf+strlen(buf), " in Obj:[%5d] ", obj_index[ZCMD[i].arg3].virtual);
	strcat(buf, fname(obj_index[ZCMD[i].arg1].name));
	strcat(name, fname(obj_index[ZCMD[i].arg1].name));
        break;
      
      case 'G':
	sprintf(buf+strlen(buf), "[Giv] Obj[%5d]", obj_index[ZCMD[i].arg1].virtual);
	sprintf(buf+strlen(buf), " (Max[%4d]) ", ZCMD[i].arg2);
	strcat(buf, fname(obj_index[ZCMD[i].arg1].name));
	strcat(name, fname(obj_index[ZCMD[i].arg1].name));
        break;
      
      case 'E':
	sprintf(buf+strlen(buf), "[Equ] Obj[%5d]", obj_index[ZCMD[i].arg1].virtual);
	sprintf(buf+strlen(buf), " (Max[%4d]) ", ZCMD[i].arg2);
	strcat(buf, fname(obj_index[ZCMD[i].arg1].name));
	strcat(name, fname(obj_index[ZCMD[i].arg1].name));
        if ((ZCMD[i].arg3 >= 0) && (ZCMD[i].arg3 < MAX_WEAR))
	  sprintf(buf+strlen(buf), " %s", loc[ZCMD[i].arg3]);
        else 
	  sprintf(buf+strlen(buf), " <ERROR!>");
        break;
      
      default: /* what the hell could this be? */
        sprintf(buf+strlen(buf), "%c %d %d %d (%d)", ZCMD[i].command, ZCMD[i].arg1, ZCMD[i].arg2, ZCMD[i].arg3, ZCMD[i].if_flag);
        break;
      }

      tmp_obj->description = str_alloc(buf);
      if (room == ch->in_room){
        send_to_char(buf,ch);
        send_to_char("\n\r",ch);
      }
      buf[21]=0;
      tmp_obj->short_description = str_alloc(buf);
      tmp_obj->name = str_alloc(name);

      obj_to_room(tmp_obj,room);
    }
  }

  /* mark it as shown and exit */
  ansi(CLR_ACTION, ch);
  send_to_char("Done. (zone commands shown)\n\r", ch);
  ansi(END, ch);
  SET_BIT(zone_table[zone].dirty_zon, SHOWN);
}

void do_zedit(struct char_data *ch, char *arg, int cmd){
  int    zone, room;
  char   buf[MAX_INPUT_LENGTH];

  room = ch->in_room;
  zone = world[ch->in_room].zone;
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  arg = one_argument(arg, buf);
  if (!*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try zedit <show || cancel || update>.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (is_abbrev(buf, "show"))
    show_zcmds(ch, zone);
  else if (is_abbrev(buf, "cancel"))
    remove_zcmds(ch, zone);
  else if (is_abbrev(buf, "update")){
    update_zcmds(ch, zone);
  } else {
    ansi(CLR_ACTION, ch);
    send_to_char("Try zedit <show || cancel || update> to set the edit mode.\n\r", ch);
    ansi(END, ch);
  }
} /* proc */


void do_zfirst(struct char_data *ch, char *arg, int cmd){
  int    zone, room;
  struct obj_data *obj;
  char   buf[MAX_INPUT_LENGTH];

  room = ch->in_room;
  zone = world[ch->in_room].zone;
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  arg = one_argument(arg, buf);
  if (!*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try zfirst <ZCMD> to put the ZCMD first.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if ( (obj = get_obj_in_list(buf, world[room].contents)) ){
    if (obj->obj_flags.type_flag == ITEM_ZCMD){
      obj_from_room(obj);
      /* now put at front of room, obj_to_room would put at end of ZCMDS */
      obj->in_room = room;
      obj->carried_by = NULL;
      obj->next_content = world[room].contents;
      world[room].contents = obj;
      sprintf(buf, "%s moved to front.\n\r", obj->description);
      ansi(CLR_ACTION, ch);
      send_to_char(buf, ch);
      ansi(END, ch);
    } else {
      ansi(CLR_ERROR, ch);
      send_to_char("That's no zone command.\n\r",ch);
      ansi(END, ch);
      return;
    }
  } else {
    ansi(CLR_ERROR, ch);
    send_to_char("What ZCMD was that again?\n\r", ch);
    ansi(END, ch);
  }
} /* proc */


void do_zmax(struct char_data *ch, char *arg, int cmd){
  int    zone, room, arg1;
  struct obj_data *obj;
  char   buf[MAX_INPUT_LENGTH];

  room = ch->in_room;
  zone = world[ch->in_room].zone;
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  arg = one_argument(arg, buf);
  if (!*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try zmax <ZCMD> <NEW_MAX>.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if ( (obj = get_obj_in_list(buf, world[room].contents)) ){
    if (obj->obj_flags.type_flag == ITEM_ZCMD){
      /* update max field and desc */
      if ((char) obj->obj_flags.value[0] != 'D') {
        arg = one_argument(arg, buf);
        arg1 = atoi(buf);
        if(arg1 <= 0) arg1 = 1; /* min of 1 */
        obj->obj_flags.value[1] = arg1;
        sprintf(buf, "%4d", arg1);
        if(strlen(obj->description) > 31)
          strncpy(obj->description + 27, buf, 4);
        sprintf(buf, "Now %s.\n\r", obj->description);
	ansi(CLR_ACTION, ch);
        send_to_char(buf, ch);
	ansi(END, ch);
      } else {
	ansi(CLR_ERROR, ch);
        send_to_char("Doors don't have max's?!?\n\r", ch);
	ansi(END, ch);
      }
    } else {
      ansi(CLR_ERROR, ch);
      send_to_char("That's no zone command.\n\r",ch);
      ansi(END, ch);
      return;
    }
  } else {
    ansi(CLR_ERROR, ch);
    send_to_char("What ZCMD was that again?\n\r", ch);
    ansi(END, ch);
  }
} /* proc */


void do_zcreate(struct char_data *ch, char *arg, int cmd){
  int    zone, i, room;
  struct obj_data *tmp_obj;
  char buf[256]; /* must be big enuff to store entire loc array && >= maxinputlen*/
  char name[128];
  char desc[128];
  char command;
  int arg1, arg2, arg3, if_flag;

  room = ch->in_room;
  zone = world[ch->in_room].zone;
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  arg = one_argument(arg, buf);
  if (!*arg) {
    send_to_char("Try ZCREATE <ARGS> ['IF' || '!IF']:\n\r(the if flag will default depending on the zcmd)\n\r", ch);
    send_to_char("  zcreate 'MOB' <MOB#> <MAX#>\n\r", ch);
    send_to_char("  zcreate 'OBJ' <OBJ#> <MAX#>\n\r", ch);
    send_to_char("  zcreate 'PUT' <OBJ#> <MAX#> <DEST_OBJ>\n\r", ch);
    send_to_char("  zcreate 'GIV' <OBJ#> <MAX#>\n\r", ch);
    send_to_char("! zcreate 'DOOR' <STATE>\n\r", ch);
    send_to_char("  zcreate 'EQU' <OBJ#> <MAX#> <location>\n\r", ch);
    send_to_char("\n\rlocations: ",ch);
    buf[0]='\0';
    for (i = 0; *loc[i] != '\n'; i++) {
      strcat(buf, loc[i]);
      strcat(buf, " ");
    }
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
    return;
  }
  if (is_abbrev(buf, "mobile")) {
    /* default IF flag setting */
    sprintf(desc, "ZCMD ");
    if_flag = 0;
    command = 'M';

    /* what mob are we */
    arg = one_argument(arg, buf);
    arg1 = real_mobile(atoi(buf));
    if (arg1 < 0) {
      ansi(CLR_ERROR, ch);
      send_to_char("What mob was that again?\n\r", ch);
      ansi(END, ch);
      return;
    }  
    if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[arg1].zone].lord))) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not authorized to use that mob.\n\r", ch);
      ansi(END, ch);
      return;
    }
    sprintf(desc+strlen(desc), "[Mob] Mob[%5d]", mob_index[arg1].virtual);
    sprintf(name, fname(mob_index[arg1].name));

    /* how many are there */
    arg = one_argument(arg, buf);
    arg2 = atoi(buf);
    if(arg2<1) arg2=1;/* must be at least one */
    sprintf(desc+strlen(desc), " (Max[%4d]) ", arg2);
    strcat(desc, fname(mob_index[arg1].name));
    strcat(name, " mobile");

    /* where are we */
    arg3 = room;

  } else if (is_abbrev(buf, "object")) {
    /* default IF flag setting */
    sprintf(desc, "ZCMD ");
    if_flag = 0;
    command = 'O';

    /* what obj are we */
    arg = one_argument(arg, buf);
    arg1 = real_object(atoi(buf));
    if (arg1 < 0) {
      ansi(CLR_ERROR, ch);
      send_to_char("What obj was that again?\n\r", ch);
      ansi(END, ch);
      return;
    }  
    if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[arg1].zone].lord))) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not authorized to use that mob.\n\r", ch);
      ansi(END, ch);
      return;
    }
    sprintf(desc+strlen(desc), "[Obj] Obj[%5d]", obj_index[arg1].virtual);
    sprintf(name, fname(obj_index[arg1].name));
    strcat(name, " object");

    /* how many are there */
    arg = one_argument(arg, buf);
    arg2 = atoi(buf);
    if(arg2<1) arg2=1;/* must be at least one */
    sprintf(desc+strlen(desc), " (Max[%4d]) ", arg2);
    strcat(desc, fname(obj_index[arg1].name));

    /* where are we */
    arg3 = room;
  } else if (is_abbrev(buf, "give")) {
    /* default IF flag setting */
    sprintf(desc, "(IF) ");
    if_flag = 1;
    command = 'G';

    /* what obj are we */
    arg = one_argument(arg, buf);
    arg1 = real_object(atoi(buf));
    if (arg1 < 0) {
      ansi(CLR_ERROR, ch);
      send_to_char("What obj was that again?\n\r", ch);
      ansi(END, ch);
      return;
    }  
    if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[arg1].zone].lord))) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not authorized to use that obj.\n\r", ch);
      ansi(END, ch);
      return;
    }
    sprintf(desc+strlen(desc), "[Giv] Obj[%5d]", obj_index[arg1].virtual);
    sprintf(name, fname(obj_index[arg1].name));
    strcat(name, " give");

    /* how many are there */
    arg = one_argument(arg, buf);
    arg2 = atoi(buf);
    if(arg2<1) arg2=1;/* must be at least one */
    sprintf(desc+strlen(desc), " (Max[%4d]) ", arg2);
    strcat(desc, fname(obj_index[arg1].name));

    /* where are we */
    arg3 = room; /* not actually used for this command */

  } else if (is_abbrev(buf, "put")) {
    /* default IF flag setting */
    sprintf(desc, "(IF) ");
    if_flag = 1;
    command = 'P';

    /* what obj are we */
    arg = one_argument(arg, buf);
    arg1 = real_object(atoi(buf));
    if (arg1 < 0) {
      ansi(CLR_ERROR, ch);
      send_to_char("What obj was that again?\n\r", ch);
      ansi(END, ch);
      return;
    }  
    if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[arg1].zone].lord))) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not authorized to use that obj.\n\r", ch);
      ansi(END, ch);
      return;
    }
    sprintf(desc+strlen(desc), "[Put] Obj[%5d]", obj_index[arg1].virtual);
    sprintf(name, fname(obj_index[arg1].name));
    strcat(name, " put");

    /* how many are there */
    arg = one_argument(arg, buf);
    arg2 = atoi(buf);
    if(arg2<1) arg2=1;/* must be at least one */
    sprintf(desc+strlen(desc), " (Max[%4d]) ", arg2);

    /* in which obj? */
    arg = one_argument(arg, buf);
    arg3 = real_object(atoi(buf));
    if (arg3 < 0) {
      ansi(CLR_ERROR, ch);
      send_to_char("Put it in what obj?\n\r", ch);
      ansi(END, ch);
      return;
    }  
    if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[arg3].zone].lord))) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not authorized to use that obj.\n\r", ch);
      ansi(END, ch);
      return;
    }
    sprintf(desc+strlen(desc), "in Obj[%5d] ", obj_index[arg3].virtual);
    strcat(desc, fname(obj_index[arg1].name));

  } else if (is_abbrev(buf, "equip")) {
    /* default IF flag setting */
    sprintf(desc, "(IF) ");
    if_flag = 1;
    command = 'E';

    /* what obj are we */
    arg = one_argument(arg, buf);
    arg1 = real_object(atoi(buf));
    if (arg1 < 0) {
      ansi(CLR_ERROR, ch);
      send_to_char("What obj was that again?\n\r", ch);
      ansi(END, ch);
      return;
    }  
    if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[arg1].zone].lord))) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not authorized to use that obj.\n\r", ch);
      ansi(END, ch);
      return;
    }
    sprintf(desc+strlen(desc), "[Equ] Obj[%5d]", obj_index[arg1].virtual);
    sprintf(name, fname(obj_index[arg1].name));
    strcat(name, " equip");

    /* how many are there */
    arg = one_argument(arg, buf);
    arg2 = atoi(buf);
    if(arg2<1) arg2=1;/* must be at least one */
    sprintf(desc+strlen(desc), " (Max[%4d]) ", arg2);
    strcat(desc, fname(obj_index[arg1].name));

    /* where do we equip it */
    arg = one_argument(arg, buf);
    if (*buf)
      arg3 = (old_search_block(buf, 0, strlen(buf), loc, 0));
    else
      arg3 = -1;
    arg3--;
    if ((arg3>=0)&&(arg3<MAX_WEAR))
      sprintf(desc+strlen(desc), " %s", loc[arg3]);
    else {
      ansi(CLR_ERROR, ch);
      send_to_char("Invalid location specified! Choose from:\n\r",ch);
      ansi(END, ch);
      buf[0]='\0';
      for (i = 0; *loc[i] != '\n'; i++) {
	strcat(buf, loc[i]);
	strcat(buf, " ");
      }
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
      return;
    }
  }

  /* check for if flag override */
  arg = one_argument(arg, buf);
  if (*arg) {
    if (is_abbrev(buf, "if")) {
      strncpy(desc, "(IF)", 4);
      if_flag = 1;
    } else if (is_abbrev(buf, "!if")) {
      strncpy(desc, "ZCMD", 4);
      if_flag = 0;
    }
  }
  if (if_flag) strcat(name, " if");

  /* create the object */
  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);
  tmp_obj->obj_flags.type_flag = ITEM_ZCMD;
  tmp_obj->obj_flags.wear_flags = 0; /* Not takeable */
  tmp_obj->obj_flags.extra_flags = ITEM_NOSEE;
  tmp_obj->obj_flags.value[0] = (char) command;
  tmp_obj->obj_flags.value[1] = arg1;
  tmp_obj->obj_flags.value[2] = arg2;
  tmp_obj->obj_flags.value[3] = arg3;
  tmp_obj->obj_flags.weight = if_flag;
  tmp_obj->obj_flags.cost = -1;
  tmp_obj->obj_flags.cost_per_day = -1;
  tmp_obj->next = object_list;
  object_list = tmp_obj;
  tmp_obj->item_number = -1;
  tmp_obj->description = str_alloc(desc);
  buf[21]=0;
  tmp_obj->short_description = str_alloc(desc);
  sprintf(buf, "obj zcmd %s", name);
  tmp_obj->name = str_alloc(buf);

  /* append object to list of zone objects in room */
  obj_to_room(tmp_obj,room);

  sprintf(buf, "%s created.\n\r(ZEDIT UPDATE then ZREBOOT for changes to take affect)\n\r", desc);
  ansi(CLR_ACTION, ch);
  send_to_char(buf , ch);
  ansi(END, ch);
}


void do_zsave(struct char_data *ch, char *arg, int cmd){
  extern void Save_zon(int zone);
  char buf[256];

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (IS_SET(zone_table[world[ch->in_room].zone].dirty_zon,UNSAVED)) 
    Save_zon(world[ch->in_room].zone);
  sprintf(buf, "Areas/%s.zon saved.\n\r", zone_table[world[ch->in_room].zone].filename);
  ansi(CLR_ACTION, ch);
  send_to_char(buf,ch);
  ansi(END, ch);
  return;
}


/*****************************************************************
 *                                                               *
 *                                                               *
 *                     W L D    S T U F F                        *
 *                                                               *
 *                                                               *
 *****************************************************************/
 
void do_rhelp(struct char_data *ch, char *arg, int cmd){
  const char *rcommands[] = {
    "rcopy",
    "rname",
    "rsect",
    "rflag",
    "rlink",
    "rdesc",
    "rlist",
    "rlflag",
    "\n"
  };
  char buf[2048];
  int i;
  
  arg = one_argument(arg, buf);
  if (*buf)  
    i = (old_search_block(buf, 0, strlen(buf), rcommands, 0)); 
  else
    i = -1;
  
  switch (i) {
  case 1: 
    ansi(CLR_ACTION, ch);
    send_to_char("Simply type rcopy and hit return. A copy of the room you are in\n\r", ch);
    send_to_char("will be created (exits are not copied) and you will be placed in the\n\r", ch);
    send_to_char("newly created room.\n\r", ch);
    ansi(END, ch);
    break;
  case 2: 
    ansi(CLR_ACTION, ch);
    send_to_char("Try rname <name>\n\r", ch);
    ansi(END, ch);
    break;
  case 3: 
    ansi(CLR_ACTION, ch);
    send_to_char("Try rsect <sector_type>\n\r", ch);
    sprintf(buf, "Sector types: \n\r");
    for (i = 0; *sector_types[i] != '\n'; i++) {
      strcat(buf, sector_types[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    ansi(END, ch);
    break;
  case 4: 
    ansi(CLR_ACTION, ch);
    send_to_char("Try rflag <flag>\n\r", ch);
    sprintf(buf, "Room flags: \n\r");
    for (i = 0; *room_bits[i] != '\n'; i++) {
      strcat(buf, room_bits[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    ansi(END, ch);
    break;
  case 5: 
    ansi(CLR_ACTION, ch);
    send_to_char("Try rlink <dir> <room#> - create or replace a link.\n\r",ch);
    send_to_char("Try rlink <dir> delete  - delete a link.\n\r", ch);
    ansi(END, ch);
    break;
  case 6: 
    ansi(CLR_ACTION, ch);
    send_to_char("Try rdesc new    - to add a new description.\n\r", ch);
    send_to_char("Try rdesc append - to append to the existing one.\n\r", ch);
    ansi(END, ch);
    break;
  case 7: 
    ansi(CLR_ACTION, ch);
    send_to_char("Try rlist [startnum] [endnum].\n\r", ch);
    send_to_char("i.e. rlist           - list all rooms in zone.\n\r", ch);
    send_to_char("i.e. rlist 5120      - list all rooms >= 5120.\n\r", ch);
    send_to_char("i.e. rlist 5120 5125 - list all rooms in zone >=5120 and <=5125.\n\r", ch);
    ansi(END, ch);
    break;
  case 8: 
    ansi(CLR_ACTION, ch);
    send_to_char("Try rlflag <dir> <flag> [<key>] [<keywords>].\n\r", ch);
    send_to_char("i.e. rlflag n ex_isdoor                 - set door bit.\n\r", ch);
    send_to_char("i.e. rlflag n ex_isdoor 5120            - set door bit.\n\r", ch);
    send_to_char("                                      - openable with key 5120.\n\r", ch);
    send_to_char("i.e. rlflag n ex_isdoor 5120 gate black - set door bit.\n\r", ch);
    send_to_char("                                      - openable with key 5120.\n\r", ch);
    send_to_char("                                      - keywords are gate and black.\n\r", ch);
    sprintf(buf, "Exit flags: \n\r");
    for (i = 0; *exit_bits[i] != '\n'; i++) {
      strcat(buf, exit_bits[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    ansi(END, ch);
    break;
  default:
    ansi(CLR_ACTION, ch);
    send_to_char("The following commands have been implemented:\n\r\n\r",ch);
    send_to_char("  rcopy - creates a copy of the room you are in.\n\r",ch);
    send_to_char("  rname - gives the room a title.\n\r",ch);
    send_to_char("  rsect - change the rooms sector type.\n\r",ch);
    send_to_char("  rflag - set/unset a room flag.\n\r",ch);
    send_to_char("  rlink - make an exit.\n\r",ch); 
    send_to_char("  rlflag- set door, locked flag etc.\n\r",ch); 
    send_to_char("  rdesc - change the rooms description.\n\r",ch);
    send_to_char("  rlist - list rooms in zone.\n\r",ch);
    send_to_char("\n\r(If the command crashes the mud, DONT USE IT AGAIN!)\n\r",ch);
    send_to_char("type rhelp <command> for more help.\n\r",ch);
    ansi(END, ch);
    break;
  }
}

void do_rstat(struct char_data *ch, char *arg, int cmd){
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int zone;
  struct extra_descr_data *desc;
  int loc_nr, location, i;
  extern int top_of_world;

  if (IS_NPC(ch))
    return;

  one_argument(arg, buf);
  if (!*buf) {
    location = ch->in_room;
  } else {	
    if (isdigit(*buf)) {
      loc_nr = atoi(buf);
      for (location = 0; location <= top_of_world; location++)
	if (world[location].number == loc_nr)
	  break;
	else if (location == top_of_world) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("No room exists with that number.\n\r", ch);
	  ansi(END, ch);
	  return;
	}
      } else {
	ansi(CLR_ERROR, ch);
        send_to_char("Not a valid room number.\n\r", ch);
	ansi(END, ch);
	return;
    }
  }
    
  /* a location has been found. */
  zone = world[location].zone;
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
   
/* output the results of the stat */
  sprinttype(world[location].sector_type, sector_types, buf2);
  sprintf(buf, "#:[%d] Name:[%s] Type:[%s] Spec:[%s]\n\r",
    world[location].number, world[location].name, buf2,
    (world[location].funct) ? "Y" : "N");
  send_to_char(buf, ch);
  send_to_char("Flags: ", ch);
  sprintbit((long) world[location].room_flags, room_bits, buf);
  strcat(buf, "\n\r");
  send_to_char(buf, ch);
  send_to_char("Description:\n\r", ch);
  send_to_char(world[location].description, ch);

  for(desc = world[location].ex_description; desc; desc = desc->next) {
    sprintf(buf, "Extra Desc. /w/ Keywords:[%s]\n\r", desc->keyword);
    send_to_char(buf, ch);
  }

  send_to_char("\n\rExits:\n\r", ch);
  for (i=0; i<=5; i++) {
    if (world[location].dir_option[i]) {
      sprintbit(world[location].dir_option[i]->exit_info, exit_bits, buf2);
      sprintf(buf, "[%s] to room:[%d], flags:[%s]\n\r",
	dirs[i], world[world[location].dir_option[i]->to_room].number, buf2);
      send_to_char(buf, ch);
      if (world[location].dir_option[i]->keyword) {
        sprintf(buf, "Key:[%d] Keywords:[%s]\n\r",
       	  world[location].dir_option[i]->key, world[location].dir_option[i]->keyword);
        send_to_char(buf, ch);
      }
      if (world[location].dir_option[i]->general_description) {
        send_to_char(world[location].dir_option[i]->general_description, ch);
      }
    }
  }
  return;
}


void do_rgoto(struct char_data *ch, char *argument, int cmd)
{
    char buf[512];
    int loc_nr, location;
    extern int top_of_world;
    int zone;

    if (IS_NPC(ch))
      return;
	
    one_argument(argument, buf);
    if (!*buf) {
      ansi(CLR_ERROR, ch);
      send_to_char("You must supply a room number.\n\r",ch);
      ansi(END, ch);
      return;
    }
	
    if (isdigit(*buf)) {
      loc_nr = atoi(buf);
      for (location = 0; location <= top_of_world; location++)
	if (world[location].number == loc_nr)
	  break;
	else if (location == top_of_world) {
	  ansi(CLR_ERROR, ch);
	  send_to_char("No room exists with that number.\n\r", ch);
	  ansi(END, ch);
	  return;
	}
      } else {
	ansi(CLR_ERROR, ch);
        send_to_char("Not a valid room number.\n\r", ch);
	ansi(END, ch);
	return;
    }
    
    /* a location has been found. */
    zone = world[location].zone;
    if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not authorized to create in that zone.\n\r", ch);
      ansi(END, ch);
      return;
    }
 
     
    strcpy(buf,"$n");
    if (ch->player.immortal_exit)
      strcat(buf,ch->player.immortal_exit);
    else
      strcat(buf," disappears in a puff of smoke.");
    ansi_act(buf, TRUE, ch, 0, 0, TO_ROOM,CLR_ACTION);
    char_from_room(ch);
    char_to_room(ch, location);
    strcpy(buf,"$n");
    if (ch->player.immortal_enter)
      strcat(buf,ch->player.immortal_enter);
    else
      strcat(buf," magically appears in the middle of the room.");
    ansi_act(buf,TRUE,ch,0,0,TO_ROOM,CLR_ACTION);
    do_look(ch, "",15);
}


void do_rcopy(struct char_data *ch, char *arg, int cmd){
  char   buf[100];
  int    zone, tmp;
  struct extra_descr_data *theExtra, *new_descr;

  zone = world[ch->in_room].zone;
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (world[zone_table[zone].real_top].number+1 > zone_table[zone].top){
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry but this zone is full... nothing more can be added.\n\r",ch);
    ansi(END, ch);
    return;
  }
  top_of_world++;
  allocate_room(top_of_world);
  SET_BIT(zone_table[zone].dirty_wld, UNSAVED);
  SET_BIT(zone_table[zone].dirty_wld, SCRAMBLED);
  world[top_of_world].number = world[zone_table[zone].real_top].number+1;
  world[top_of_world].next = top_of_world+1;
  world[zone_table[zone].real_top].next = top_of_world;
  zone_table[zone].real_top = top_of_world;
  world[top_of_world].zone = zone;

  if (world[ch->in_room].name)
    world[top_of_world].name = str_alloc(world[ch->in_room].name);
  else
    world[top_of_world].name = (char *) 0;
  if (world[ch->in_room].description)
    world[top_of_world].description = str_alloc(world[ch->in_room].description);
  else 
    world[top_of_world].description = (char *) 0;
  world[top_of_world].room_flags = world[ch->in_room].room_flags;
  world[top_of_world].sector_type = world[ch->in_room].sector_type;
  world[top_of_world].funct = 0;
  world[top_of_world].contents = 0;
  world[top_of_world].people = 0;
  world[top_of_world].light = 0; /* Zero light sources */

  /* don't copy directions */
  for (tmp = 0; tmp <= 5; tmp++)
    world[top_of_world].dir_option[tmp] = 0;

  /* but copy the extra descriptions (never know I guess) */
  world[top_of_world].ex_description = 0;
  for (theExtra = world[ch->in_room].ex_description;
       theExtra;
       theExtra = theExtra->next) {
    CREATE(new_descr, struct extra_descr_data, 1);
    if (theExtra->keyword)
      new_descr->keyword = str_alloc(theExtra->keyword);
    else
      new_descr->keyword = (char *) 0;
    if (theExtra->description)
      new_descr->description = str_alloc(theExtra->description);
    else
      new_descr->description = (char *) 0;
    new_descr->next = world[top_of_world].ex_description;
    world[top_of_world].ex_description = new_descr;
  } /* for loop */
  char_from_room(ch);
  char_to_room(ch, top_of_world);
  sprintf(buf, "Done, room #%d created, and you're now in it!", world[top_of_world].number);
  ansi(CLR_ACTION, ch);
  send_to_char(buf, ch);
  ansi(END, ch);
} /* proc */

/* link one room to another, destroy old link if necessary */
/* but how to handle doors & keys? */
/* guess I'll have redesc and rkey commands... hmm... dunno */
/* how bout rlink <dir> <room#> [keywords], then prompt for desc */
/* also have rlflag <flag> [<key>] */
 
void do_rlink(struct char_data *ch, char *arg, int cmd){
    char buf1[256], buf2[256], buf3[256];
    int dir, zone;
    int cha_rm, tar_rm;

    zone = world[ch->in_room].zone;
    cha_rm = ch->in_room;
    if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not authorized to create in that zone.\n\r", ch);
      ansi(END, ch);
      return;
    }
    arg=one_argument(arg, buf1);
    if (!*buf1) {
      ansi(CLR_ACTION, ch);
      send_to_char("Try rlink <dir> (<room#> || delete)\n\r", ch);
      ansi(END, ch);
      return;
    }
    half_chop(arg, buf2, buf3);

    dir = (old_search_block(buf1, 0, strlen(buf1), dirs, 0)); 
    if (dir == -1) { /* the 0 above allows for substr match */
     ansi(CLR_ERROR, ch);
     send_to_char("That type doesn't exist.\n\r", ch);
     ansi(END, ch);
     return;
    }
    dir--;  /* search block returns 1 as first position */

    if (dir == -1) {
      ansi(CLR_ERROR, ch);
      send_to_char("What direction is that?\n\r", ch);
      ansi(END, ch);
      return;
    }
    if (*buf2 && is_abbrev(buf2, "delete")) {
      if (world[ch->in_room].dir_option[dir]) {
        if (world[ch->in_room].dir_option[dir]->general_description)
          str_free((char *)world[ch->in_room].dir_option[dir]->general_description);
        if (world[ch->in_room].dir_option[dir]->keyword)
          str_free((char *)world[ch->in_room].dir_option[dir]->keyword);
        free((char *)world[ch->in_room].dir_option[dir]);
        world[ch->in_room].dir_option[dir] = 0;
        SET_BIT(zone_table[world[ch->in_room].zone].dirty_wld, UNSAVED);
      }
      return;
    }
    tar_rm = real_room(atoi(buf2));

    if ((tar_rm<0) || (tar_rm>top_of_world)) {
      ansi(CLR_ERROR, ch);
      send_to_char("There's no room of that number.\n\r", ch);
      ansi(END, ch);
      return;
    }
    if ((world[tar_rm].zone != world[cha_rm].zone) 
        && (GET_LEVEL(ch) != IMP_LEV)) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not authorized to create an inter-zone link.\n\r", ch);
      ansi(END, ch);
      return;
    }
    if (world[ch->in_room].dir_option[dir]) {
      if (world[ch->in_room].dir_option[dir]->general_description)
         str_free((char *)world[ch->in_room].dir_option[dir]->general_description);
      if (world[ch->in_room].dir_option[dir]->keyword)
         str_free((char *)world[ch->in_room].dir_option[dir]->keyword);
      free((char *)world[ch->in_room].dir_option[dir]);
      world[ch->in_room].dir_option[dir] = 0;
    }
    CREATE(world[ch->in_room].dir_option[dir], struct room_direction_data, 1);
    world[ch->in_room].dir_option[dir]->general_description = 0;
    world[ch->in_room].dir_option[dir]->keyword = 0;
    if (*buf3) { /* if there are keywords add them */
      world[ch->in_room].dir_option[dir]->keyword = str_alloc(buf3);
    }
    world[ch->in_room].dir_option[dir]->exit_info = 0;
    world[ch->in_room].dir_option[dir]->key = -1;
    world[ch->in_room].dir_option[dir]->to_room = tar_rm;

    SET_BIT(zone_table[world[ch->in_room].zone].dirty_wld, UNSAVED);
    ansi(CLR_ACTION, ch);
    send_to_char("Enter description terminate with @.\n\r", ch);
    ansi(END, ch);
    ch->desc->str = &(world[ch->in_room].dir_option[dir]->general_description);
    ch->desc->max_str = 2048;
}

/* link one room to another, destroy old link if necessary */
/* also have rlflag <dir> <flag> [<key>] [<keywords>]*/
 
void do_rlflag(struct char_data *ch, char *arg, int cmd){
    char buf[128], buf1[128], buf2[128], buf3[128], buf4[128];
    int dir;
    int loc,i;
    unsigned long flag;

    if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))) {
      ansi(CLR_ERROR, ch);
      send_to_char("You are not authorized to create in that zone.\n\r", ch);
      ansi(END, ch);
      return;
    }
    arg = one_argument(arg, buf1);
    if (!*buf1) {
      ansi(CLR_ACTION, ch);
      send_to_char("Try rlflag <dir> <flag> [<key>] [<keywords>]\n\r", ch);
      sprintf(buf, "exit flags: \n\r");
      for (i = 0; *exit_bits[i] != '\n'; i++) {
        strcat(buf, exit_bits[i]);
        strcat(buf, " ");
      }
      strcat(buf,"\n\r");
      send_to_char(buf,ch);
      ansi(END, ch);
      return;
    }
    arg = one_argument(arg, buf2);
    half_chop(arg, buf3, buf4);
    if (!*buf1 || !*buf2) {
      ansi(CLR_ACTION, ch);
      send_to_char("Try rlflag <dir> <flag> [<key>] [<keywords>].\n\r", ch);
      ansi(END, ch);
      return;
    }

    dir = (old_search_block(buf1, 0, strlen(buf1), dirs, 0)); 
    if (dir == -1) { /* the 0 above allows for substr match */
     ansi(CLR_ERROR, ch);
     send_to_char("That direction doesn't exist.\n\r", ch);
     ansi(END, ch);
     return;
    }
    dir--;  /* search block returns 1 as first position */

    loc = (old_search_block(buf2, 0, strlen(buf2), exit_bits, -1));
    if (loc == -1) { /* the -1 in old_search_block lowercases the list */
      ansi(CLR_ERROR, ch);
      send_to_char("That flag doesn't exist.\n\r", ch);
      ansi(END, ch);
      return;
    }
    loc--;  /* search block returns 1 as first position */

    flag = 1;
    for (i = 0; i < loc; i++)
      flag *= 2;
 
    if (world[ch->in_room].dir_option[dir]) {
      SET_BIT(zone_table[world[ch->in_room].zone].dirty_wld, UNSAVED);
      if (IS_SET(world[ch->in_room].dir_option[dir]->exit_info, flag)) {
        sprintf(buf, "You have removed the '%s' flag.\n\r", exit_bits[loc]);
        REMOVE_BIT(world[ch->in_room].dir_option[dir]->exit_info, flag);
	ansi(CLR_ACTION, ch);
        send_to_char(buf, ch);
	ansi(END, ch);
      } else {
        sprintf(buf, "You have set the '%s' flag.\n\r", exit_bits[loc]);
        SET_BIT(world[ch->in_room].dir_option[dir]->exit_info, flag);
	ansi(CLR_ACTION, ch);
        send_to_char(buf, ch);
	ansi(END, ch);
      }
      if (*buf3 && is_number(buf3)) { /* set key if specified */
        world[ch->in_room].dir_option[dir]->key = atoi(buf3);
      }
      if (*buf4) { /* set keywords if entered */
        if (world[ch->in_room].dir_option[dir]->keyword)
           str_free((char *)world[ch->in_room].dir_option[dir]->keyword);
        world[ch->in_room].dir_option[dir]->keyword = str_alloc(buf4);
      }
    } else {
      ansi(CLR_ERROR, ch);
      send_to_char("No exit exists for that direction.\n\r", ch);
      ansi(END, ch);
    }
}


void do_rsect(struct char_data *ch, char *arg, int cmd){
  char buf[256], buf1[128];
  int loc,i;

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  arg = one_argument(arg, buf1);
  if (!*buf1) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try rsect <sector_type>.\n\r", ch);
    sprintf(buf, "Sector types: \n\r");
    for (i = 0; *sector_types[i] != '\n'; i++) {
      strcat(buf, sector_types[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    ansi(END, ch);
    return;
  }
  loc = (old_search_block(buf1, 0, strlen(buf1), sector_types, -1));
  if (loc == -1) {
    ansi(CLR_ERROR, ch);
    send_to_char("That type doesn't exist.\n\r", ch);
    ansi(END, ch);
    return;
  }
  loc--;  /* search block returns 1 as first position */

  world[ch->in_room].sector_type = loc;
  SET_BIT(zone_table[world[ch->in_room].zone].dirty_wld, UNSAVED);
  sprintf(buf, "sector type is now '%s'.\n\r", sector_types[loc]);
  ansi(CLR_ACTION, ch);
  send_to_char(buf, ch);
  ansi(END, ch);
}


void do_rlist(struct char_data *ch, char *arg, int cmd){
  int i, zone, start, end;
  char buf[16384], buf1[128], buf2[128];
  
  if ((GET_LEVEL(ch) < IMO_LEV2) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  half_chop(arg, buf1, buf2);
  
  zone = world[ch->in_room].zone;
  if (is_number(buf1))
    start = real_room(atoi(buf1));
  else 
    start = zone_table[zone].real_bottom;   
  if (is_number(buf2))
    end = real_room(atoi(buf2));
  else end = zone_table[zone].real_top;
  if (end - start >= 200) { 
    end = start + 199;
    ansi(CLR_ERROR, ch);
    send_to_char("Limiting search to 200 rooms (try rlist <start num> <end num> for rest).\n\r",ch);
    ansi(END, ch);
  } 
  sprintf(buf, "\n\r"); /* init buf to something for strcat */
  for (i = start; (i>=0) && (i <= top_of_world) && (i <= end); i=world[i].next) {
    sprintf(buf1, "%d - %s\n\r", world[i].number, world[i].name);
    strcat(buf, buf1);
  }
  ansi(CLR_ACTION, ch);
  strcat(buf, "\n\rEnd of rlist.\n\r");
  ansi(END, ch);
  page_string(ch->desc, buf, 1); 
}


void do_rflag(struct char_data *ch, char *arg, int cmd){
  char buf[256], buf1[128];
  int loc,i;
  unsigned long flag;

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  one_argument(arg, buf1);
  if (!*buf1) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try rflag <flag>\n\r", ch);
    sprintf(buf, "Room flags: \n\r");
    for (i = 0; *room_bits[i] != '\n'; i++) {
      strcat(buf, room_bits[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    ansi(END, ch);
    return;
  }
  loc = (old_search_block(buf1, 0, strlen(buf1), room_bits, -1));
  if (loc == -1) { /* the -1 in old_search_block lowercases the list */
    ansi(CLR_ERROR, ch);
    send_to_char("That flag doesn't exist.\n\r", ch);
    ansi(END, ch);
    return;
  }
  loc--;  /* search block returns 1 as first position */

  flag = 1;
  for (i = 0; i < loc; i++)
    flag *= 2;

  if (IS_SET(world[ch->in_room].room_flags, flag)) {
    sprintf(buf, "You have removed the '%s' flag.\n\r", room_bits[loc]);
    REMOVE_BIT(world[ch->in_room].room_flags, flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  } else {
    sprintf(buf, "You have set the '%s' flag.\n\r", room_bits[loc]);
    SET_BIT(world[ch->in_room].room_flags, flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  }
  SET_BIT(zone_table[world[ch->in_room].zone].dirty_wld, UNSAVED);
}


/* Give a room a new description */
void do_rdesc(struct char_data *ch, char *arg, int cmd){
  char arg1[256];

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  arg = one_argument(arg, arg1);
  if (!*arg1) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try rdesc <new || append>.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (world[ch->in_room].description) {
    send_to_char("The old description was: \n\r", ch);
    send_to_char(world[ch->in_room].description, ch);
    if (!is_abbrev(arg1, "append")) {
      str_free((char *)world[ch->in_room].description);
      world[ch->in_room].description = (char *)0;
    }
  }
  ansi(CLR_ACTION, ch);
  send_to_char("\n\rEnter a new description.  Terminate with a '@'.\n\r", ch);
  ansi(END, ch);
  ch->desc->str = &(world[ch->in_room].description);
  ch->desc->max_str = 2048;
  SET_BIT(zone_table[world[ch->in_room].zone].dirty_wld, UNSAVED);
}

/* Give a room an extra description */
void do_redesc(struct char_data *ch, char *arg, int cmd){
  char arg1[256];
  struct extra_descr_data *theExtra, *prev;

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  arg = one_argument(arg, arg1);
  for (;*arg==' ';arg++); /* strip out preceeding spaces if any */
  if (!*arg1 || !*arg) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try redesc <new || append || delete> <keyword list>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  /* find the damn keyword if it exists */
  for (theExtra = world[ch->in_room].ex_description;
       theExtra;
       theExtra = theExtra->next) {
    if (theExtra->keyword)
      if (isname(arg, theExtra->keyword))
        break;
  } /* for loop */

  if (is_abbrev(arg1, "delete")) { /* icky delete */
    if (!theExtra) {
      ansi(CLR_ERROR, ch);
      send_to_char("That Extra description does not exist.\n\r",ch);
      ansi(END, ch);
      return;
    }
    if (world[ch->in_room].ex_description == theExtra) {
      world[ch->in_room].ex_description = theExtra->next;
    } else {
      for(prev=world[ch->in_room].ex_description;
          prev->next != theExtra;
          prev=prev->next);
      prev->next = theExtra->next; /* take this one out of chain */
    }
    ansi(CLR_ACTION, ch);
    send_to_char("Extra description deleted.\n\r",ch);
    ansi(END, ch);
    str_free(theExtra->description);
    str_free(theExtra->keyword);
    free(theExtra);
    SET_BIT(zone_table[world[ch->in_room].zone].dirty_wld, UNSAVED);
    return;
  }

  if (!theExtra) {
    CREATE(theExtra, struct extra_descr_data, 1);
    theExtra->next = world[ch->in_room].ex_description;
    world[ch->in_room].ex_description = theExtra;
    theExtra->keyword = str_alloc(arg);
    theExtra->description = (char *) 0;
  }

  if (theExtra->description) {
    send_to_char("The old description was: \n\r", ch);
    send_to_char(theExtra->description, ch);
    if (!is_abbrev(arg1, "append")) {
      str_free(theExtra->description);
      theExtra->description = (char *)0;
    }
  }
  ansi(CLR_ACTION, ch);
  send_to_char("\n\rEnter a new description.  Terminate with a '@'.\n\r", ch);
  ansi(END, ch);
  ch->desc->str = &theExtra->description;
  ch->desc->max_str = 2048;
  SET_BIT(zone_table[world[ch->in_room].zone].dirty_wld, UNSAVED);
}

void do_rname(struct char_data *ch, char *arg, int cmd){

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (!arg || !*arg) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try rname [<desc>].\n\r", ch);
    ansi(END, ch);
    return;
  }
  for (;*arg==' ';arg++); /* strip out preceeding spaces if any */
  
  send_to_char("The old name was: \n\r", ch);
  send_to_char(world[ch->in_room].name, ch);

  str_free (world[ch->in_room].name);
  world[ch->in_room].name = str_alloc(arg);
  ansi(CLR_ACTION, ch);
  send_to_char("\n\rThe new name is: \n\r", ch);
  send_to_char(world[ch->in_room].name, ch);
  ansi(END, ch);
  SET_BIT(zone_table[world[ch->in_room].zone].dirty_wld, UNSAVED);
}

void do_rsave(struct char_data *ch, char *arg, int cmd){
  extern void Save_wld(int zone);
  char buf[256];

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (IS_SET(zone_table[world[ch->in_room].zone].dirty_wld,UNSAVED)) {
    Save_wld(world[ch->in_room].zone);
    sprintf(buf, "areas/%s.wld saved.\n\r", zone_table[world[ch->in_room].zone].filename);
  } else 
    sprintf(buf, "areas/%s.wld unchanged since last save.\n\r", zone_table[world[ch->in_room].zone].filename);
  ansi(CLR_ACTION, ch);
  send_to_char(buf,ch);
  ansi(END, ch);
  return;
}

/*****************************************************************
 *                                                               *
 *                                                               *
 *                     O B J    S T U F F                        *
 *                                                               *
 *                                                               *
 *****************************************************************/


void do_ocreate(struct char_data *ch, char *arg, int cmd){
  char   buf[100];
  int    zone, i;
  struct obj_data *obj;

  zone = world[ch->in_room].zone;
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (zone_table[zone].bot_of_objt == -1) /* check for empty */
    zone_table[zone].bot_of_objt = top_of_objt +1;
  /* and what if its full */
  else if (obj_index[zone_table[zone].top_of_objt].virtual+1 > zone_table[zone].top){
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry but this zone is full... nothing more can be added.\n\r",ch);
    ansi(END, ch);
    return;
  }
/* fix the top/bottom of the index for the new entry */
  top_of_objt++;
  allocate_obj(top_of_objt);
  SET_BIT(zone_table[zone].dirty_obj, UNSAVED);
  SET_BIT(zone_table[zone].dirty_obj, SCRAMBLED);
  if (zone_table[zone].top_of_objt >= 0) {
    obj_index[top_of_objt].virtual = obj_index[zone_table[zone].top_of_objt].virtual+1;
    obj_index[zone_table[zone].top_of_objt].next = top_of_objt;
  } else {
    obj_index[top_of_objt].virtual = zone_table[zone].bottom;
  }
  obj_index[top_of_objt].next = top_of_objt+1;
  obj_index[top_of_objt].zone = zone;
  obj_index[top_of_objt].number = 0;
  obj_index[top_of_objt].func = 0;
  obj_index[top_of_objt].next = top_of_objt+1;
  zone_table[zone].top_of_objt = top_of_objt;

/* give it some initial values */
  CREATE(obj, struct obj_data, 1);
  clear_object(obj);

  obj_index[top_of_objt].name = str_alloc("obj torch");
  obj->name = str_alloc("obj torch");
  obj->short_description = str_alloc("a torch"); 
  obj->description = str_alloc("A torch is lying here.");
  obj->action_description = 0;

  obj->obj_flags.type_flag = 1;
  obj->obj_flags.extra_flags = 0;
  obj->obj_flags.wear_flags = 1;
  obj->obj_flags.value[0] = 0;
  obj->obj_flags.value[1] = 0;
  obj->obj_flags.value[2] = 10;
  obj->obj_flags.value[3] = 0;
  obj->obj_flags.weight = 1;
  obj->obj_flags.cost = 5;
  obj->obj_flags.cost_per_day = 5;
  obj->ex_description = 0;

  for(i=0;i<MAX_OBJ_AFFECT;i++) {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }
  
  obj->next_content = 0;
  obj->carried_by = 0;
  obj->in_obj = 0;
  obj->contains = 0;
  obj->item_number = top_of_objt;
  obj->in_room = NOWHERE;

  obj_index[top_of_objt].prototype = (char *) obj;
/* let 'em know what happened */
  sprintf(buf, "Done. Obj #%d created!", obj_index[top_of_objt].virtual);
  ansi(CLR_ACTION, ch);
  send_to_char(buf, ch);
  ansi(END, ch);
} /* proc */



void do_olist(struct char_data *ch, char *arg, int cmd){
  int i, zone, start, end, no=1;
  char buf[2*MAX_STRING_LENGTH], buf1[256], buf2[256];
  
  zone = world[ch->in_room].zone;
  half_chop(arg, buf1, buf2);
  
  if (is_number(buf1)) {
    start = atoi(buf1);
    zone = zone_of(start);
  } else 
    start = zone_table[zone].bottom;   

  if (is_number(buf2))
    end = atoi(buf2);
  else 
    end = zone_table[zone].top;
  if (end < start)
    end = start+15;
  if (end > zone_table[zone].top)
    end = zone_table[zone].top;
  if (end - start >= 100) { 
    end = start + 99;
    ansi(CLR_ERROR, ch);
    send_to_char("Limiting search to 200 rooms (try olist <start num> <end num> for rest).\n\r",ch);
    ansi(END, ch);
  } 

  if ((GET_LEVEL(ch) < IMO_LEV2) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  sprintf(buf, "\n\r"); /* init buf to something for strcat */
  for (i = zone_table[zone].bot_of_objt; (i>=0) && (i <= top_of_objt) && (obj_index[i].virtual <= end); i=obj_index[i].next) {
    if (obj_index[i].virtual >= start) {
      sprintf(buf1, "[%5d] [%-28s] ", obj_index[i].virtual, obj_index[i].name);
      if (!(no%2))
        strcat(buf1, "\n\r");
      no++;
      strcat(buf, buf1);
    }
  }
  sprintf(buf1, "Done. objs[%d to %d]\n\r", start, end);
  strcat(buf, buf1);
  page_string(ch->desc, buf, 1); 
}

void do_oflag(struct char_data *ch, char *arg, int cmd){
  char buf[256], buf1[256], buf2[256];
  int loc,i;
  unsigned long flag;
  struct obj_data *obj;

  half_chop(arg, buf1, buf2);
  if (!is_number(buf1) || !*buf2) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try oflag <obj> <flag>.\n\r", ch);
    sprintf(buf, "Obj flags: \n\r");
    for (i = 0; *extra_bits[i] != '\n'; i++) {
      strcat(buf, extra_bits[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    ansi(END, ch);
    return;
  }
  loc = (old_search_block(buf2, 0, strlen(buf2), extra_bits, -1));
  if (loc == -1) { /* the -1 in old_search_block lowercases the list */
    ansi(CLR_ERROR, ch);
    send_to_char("That flag doesn't exist.\n\r", ch);
    ansi(END, ch);
    return;
  }
  loc--;  /* search block returns 1 as first position */

  if (!(obj = read_object(atoi(buf1), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }

  flag = 1;
  for (i = 0; i < loc; i++)
    flag *= 2;

  if (IS_SET(obj->obj_flags.extra_flags, flag)) {
    sprintf(buf, "You have removed the '%s' flag.\n\r", extra_bits[loc]);
    REMOVE_BIT(obj->obj_flags.extra_flags, flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  } else {
    sprintf(buf, "You have set the '%s' flag.\n\r", extra_bits[loc]);
    SET_BIT(obj->obj_flags.extra_flags, flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  }
  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}

void do_owflag(struct char_data *ch, char *arg, int cmd){
  char buf[256], buf1[256], buf2[256];
  int loc,i;
  unsigned long flag;
  struct obj_data *obj;

  half_chop(arg, buf1, buf2);
  if (!is_number(buf1) || !*buf2) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try owflag <obj> <flag>.\n\r", ch);
    sprintf(buf, "Weapon flags: \n\r");
    for (i = 0; *weapon_bits[i] != '\n'; i++) {
      strcat(buf, weapon_bits[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    ansi(END, ch);
    return;
  }
  loc = (old_search_block(buf2, 0, strlen(buf2), weapon_bits, -1));
  if (loc == -1) { /* the -1 in old_search_block lowercases the list */
    ansi(CLR_ERROR, ch);
    send_to_char("That flag doesn't exist.\n\r", ch);
    ansi(END, ch);
    return;
  }
  loc--;  /* search block returns 1 as first position */

  if (!(obj = read_object(atoi(buf1), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }

  flag = 1;
  for (i = 0; i < loc; i++)
    flag *= 2;

  if (IS_SET(obj->obj_flags.value[0], flag)) {
    sprintf(buf, "You have removed the '%s' flag.\n\r", weapon_bits[loc]);
    REMOVE_BIT(obj->obj_flags.value[0], flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  } else {
    sprintf(buf, "You have set the '%s' flag.\n\r", weapon_bits[loc]);
    SET_BIT(obj->obj_flags.value[0], flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  }
  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}


void do_owear(struct char_data *ch, char *arg, int cmd){
  char buf[256], buf1[256], buf2[256];
  int loc,i;
  unsigned long flag;
  struct obj_data *obj;

  half_chop(arg, buf1, buf2);
  if (!is_number(buf1) || !*buf2) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try owflag <obj> <flag>.\n\r", ch);
    sprintf(buf, "Wear flags: \n\r");
    for (i = 0; *wear_bits[i] != '\n'; i++) {
      strcat(buf, wear_bits[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    ansi(END, ch);
    return;
  }
  loc = (old_search_block(buf2, 0, strlen(buf2), wear_bits, -1));
  if (loc == -1) { /* the -1 in old_search_block lowercases the list */
    ansi(CLR_ERROR, ch);
    send_to_char("That flag doesn't exist.\n\r", ch);
    ansi(END, ch);
    return;
  }
  loc--;  /* search block returns 1 as first position */

  if (!(obj = read_object(atoi(buf1), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }

  flag = 1;
  for (i = 0; i < loc; i++)
    flag *= 2;

  if (IS_SET(obj->obj_flags.wear_flags, flag)) {
    sprintf(buf, "You have removed the '%s' flag.\n\r", wear_bits[loc]);
    REMOVE_BIT(obj->obj_flags.wear_flags, flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  } else {
    sprintf(buf, "You have set the '%s' flag.\n\r", wear_bits[loc]);
    SET_BIT(obj->obj_flags.wear_flags, flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  }
  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}


void do_otype(struct char_data *ch, char *arg, int cmd){
  char buf[256], buf1[256], buf2[256];
  int loc,i;
  struct obj_data *obj;

  half_chop(arg, buf1, buf2);
  if (!is_number(buf1) || !*buf2) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try otype <obj> <type>\n\r", ch);
    sprintf(buf, "Item types: \n\r");
    for (i = 1; *item_types[i] != '\n'; i++) {
      strcat(buf, item_types[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    ansi(END, ch);
    return;
  }
  loc = (old_search_block(buf2, 0, strlen(buf2), item_types, -1));
  if (loc == -1) { /* the -1 in old_search_block lowercases the list */
    ansi(CLR_ERROR, ch);
    send_to_char("That flag doesn't exist.\n\r", ch);
    ansi(END, ch);
    return;
  }
  loc--;  /* search block returns 1 as first position */
  if (loc == 0) {
    ansi(CLR_ERROR, ch);
    send_to_char("You can't set an item type to UNDEFINED...sheesh!\n\r",ch);
    ansi(END, ch);
    return;
  }
  if (loc == ITEM_ZCMD) {
    ansi(CLR_ERROR, ch);
    send_to_char("Those are reserved for online zone editing...sheesh!\n\r",ch);
    ansi(END, ch);
    return;
  }
  if (!(obj = read_object(atoi(buf1), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  GET_ITEM_TYPE(obj) = loc;

  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}

void do_oaffect(struct char_data *ch, char *arg, int cmd){
  char buf[1024], buf1[256], buf2[256];
  char buf3[256], buf4[256];
  int loc,i, one;
  struct obj_data *obj;

  arg = one_argument(arg, buf1);
  if (*buf1) arg = one_argument(arg, buf2);
  if (*buf2) half_chop(arg, buf3, buf4);
  if (!isdigit(*buf1) || !isdigit(*buf2) || !*buf3 || !*buf4) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try oaffect <obj> <aff#> <type> <modifier>.\n\r", ch);
    sprintf(buf, "Apply types: \n\r");
    for (i = 0; *apply_types[i] != '\n'; i++) {
      strcat(buf, apply_types[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    ansi(END, ch);
    return;
  }
  loc = (old_search_block(buf3, 0, strlen(buf3), apply_types, -1));
  if (loc == -1) { /* the -1 in old_search_block lowercases the list */
    ansi(CLR_ERROR, ch);
    send_to_char("That flag doesn't exist.\n\r", ch);
    ansi(END, ch);
    return;
  }
  loc--;  /* search block returns 1 as first position */

  if (!(obj = read_object(atoi(buf1), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  i = atoi(buf2);
  if (i < 0 || i>=MAX_OBJ_AFFECT) {
    ansi(CLR_ACTION, ch);
    send_to_char("Affect numbers range from 0 to 3.\n\r", ch);
    ansi(END, ch);
    return;
  }

  obj->affected[i].location = loc;
  sscanf(buf4, " %d", &one);
  obj->affected[i].modifier= one;
  
  sprinttype(obj->affected[i].location,apply_types,buf2);
  sprintf(buf, "OK. It now affects: %s By %d\n\r", buf2, obj->affected[i].modifier);
  ansi(CLR_ACTION, ch);
  send_to_char(buf, ch);
  ansi(END, ch);

  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}


void do_ostat(struct char_data *ch, char *arg, int cmd){
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int i;
  struct obj_data *obj;
  struct extra_descr_data *desc;

  arg = one_argument(arg, buf);
  if (!is_number(buf)) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try ostat <obj>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(obj = read_object(atoi(buf), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }

/* output the results of the stat */
  sprintf(buf, "Obj#:[%d]  Keywords:[%s] Type:[", 
    obj_index[obj->item_number].virtual, obj->name);
  sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
  strcat(buf,buf2); strcat(buf,"]\n\r");
  send_to_char(buf, ch);

  sprintf(buf, "Short description:[%s] name:[%s]\n\rLong description:\n\r%s\n\r", 
               ((obj->short_description) ? obj->short_description : "None"),
               fname(obj_index[obj->item_number].name), 
               ((obj->description) ? obj->description : "None") );
  send_to_char(buf, ch);

  send_to_char("Worn : ", ch);
  sprintbit(obj->obj_flags.wear_flags,wear_bits,buf);
  strcat(buf,"\n\r");
  send_to_char(buf, ch);

  send_to_char("Flag(s) : ", ch);
  sprintbit(obj->obj_flags.extra_flags,extra_bits,buf);
  strcat(buf,"\n\r");
  send_to_char(buf, ch);

  if (GET_ITEM_TYPE(obj) == ITEM_WEAPON) {
    send_to_char("Weapon :", ch);
    sprintbit(obj->obj_flags.value[0],weapon_bits,buf);
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
  }

  sprintf(buf, "Weight:[%d] Cost:[%d] Rent:[%d]\n\r",
  	       obj->obj_flags.weight,
               obj->obj_flags.cost,
               obj->obj_flags.cost_per_day);
  send_to_char(buf,ch);

  switch (obj->obj_flags.type_flag) {
  case ITEM_LIGHT : 
    sprintf(buf, "Colour:[%d] Type:[%d] Hours:[%d] Undef:[]\n\r", 
      obj->obj_flags.value[0],
      obj->obj_flags.value[1],
      obj->obj_flags.value[2]);
    break;
  case ITEM_SCROLL : 
  case ITEM_POTION : 
    sprintf(buf, "Level: [%d] Spells: [%d], [%d], [%d]\n\r",
    obj->obj_flags.value[0],
    obj->obj_flags.value[1],
    obj->obj_flags.value[2],
    obj->obj_flags.value[3] );
    break;
  case ITEM_WAND : 
  case ITEM_STAFF : 
    sprintf(buf, "Level[%d], Max-mana[%d], Left:[%d], Spell:[%d]\n\r",
    obj->obj_flags.value[0],
    obj->obj_flags.value[1],
    obj->obj_flags.value[2],
    obj->obj_flags.value[3] );
    break;
  case ITEM_WEAPON :
    sprintf(buf, "Wflag:[] Dam:[%d]D[%d] Type:[%d]\n\r",
    obj->obj_flags.value[1],
    obj->obj_flags.value[2],
    obj->obj_flags.value[3]);
    break;
  case ITEM_ARMOR :
    sprintf(buf, "AC-apply:[%d] Undef:[] Undef:[] Undef:[]\n\r",
    obj->obj_flags.value[0]);
    break;
  case ITEM_TRAP :
    sprintf(buf, "Spell:[%d] Hitpoints:[%d] Undef:[] Undef:[]\n\r",
    obj->obj_flags.value[0],
    obj->obj_flags.value[1]);
    break;
  case ITEM_CONTAINER :
    sprintf(buf, "Max-contains:[%d] Locktype:[%d] Undef:[] Corpse:[%s]\n\r",
    obj->obj_flags.value[0],
    obj->obj_flags.value[1],
    obj->obj_flags.value[3]?"Yes":"No");
  break;
  case ITEM_DRINKCON :
    sprinttype(obj->obj_flags.value[2],drinks,buf2);
    sprintf(buf, "Max:[%d] Contains:[%d] Liquid:[%s] Poisoned:[%d]\n\r",
    obj->obj_flags.value[0],
    obj->obj_flags.value[1],
    buf2,
    obj->obj_flags.value[3]);
  break;
  case ITEM_NOTE :
    sprintf(buf, "Tongue:[%d] Undef:[] Undef:[] Undef:[]\n\r",
    obj->obj_flags.value[0]);
    break;
  case ITEM_KEY :
    sprintf(buf, "Keytype:[%d] Undef:[] Undef:[] Undef:[]\n\r",
    obj->obj_flags.value[0]);
    break;
  case ITEM_FOOD :
    sprintf(buf, "Makes-full:[%d] Poisoned:[%d] Undef:[] Undef:[]\n\r",
    obj->obj_flags.value[0],
    obj->obj_flags.value[3]);
    break;
  default :
    sprintf(buf,"Values 0-3 : [%d] [%d] [%d] [%d]\n\r",
    obj->obj_flags.value[0],
    obj->obj_flags.value[1],
    obj->obj_flags.value[2],
    obj->obj_flags.value[3]);
    break;
  }
  send_to_char(buf, ch);

  send_to_char("Can affect char :\n\r", ch);
  for (i=0;i<MAX_OBJ_AFFECT;i++) {
    sprinttype(obj->affected[i].location,apply_types,buf2);
    sprintf(buf,"    Affects : %s By %d\n\r", buf2,obj->affected[i].modifier);
    send_to_char(buf, ch);
  }			

  if(obj->ex_description){
    strcpy(buf, "Extra description keyword(s):\n\r----------\n\r");
    for (desc = obj->ex_description; desc; desc = desc->next) {
      strcat(buf, desc->keyword);
      strcat(buf, "\n\r");
    }
    send_to_char(buf, ch);
  } else {
    strcpy(buf,"Extra description keyword(s): None\n\r");
    send_to_char(buf, ch);
  }
  return;
}

void do_ovalues(struct char_data *ch, char *arg, int cmd){
  char buf[MAX_STRING_LENGTH];
  struct obj_data *obj;
  
  arg = one_argument(arg, buf);
  if (!*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try ovalues <obj> <value> <value> <value> <value>.\n\r", ch);
    send_to_char("i.e. ovalue 6405 - 2 - 2 would change value[1] and value[3] to 2.\n\r", ch);
    send_to_char("Type HELP OVALUES for additional info.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(obj = read_object(atoi(buf), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  arg = one_argument(arg, buf);
  if (isdigit(*buf))
    obj->obj_flags.value[0] = atoi(buf);
  arg = one_argument(arg, buf);
  if (isdigit(*buf))
    obj->obj_flags.value[1] = atoi(buf);
  arg = one_argument(arg, buf);
  if (isdigit(*buf))
    obj->obj_flags.value[2] = atoi(buf);
  arg = one_argument(arg, buf);
  if (isdigit(*buf))
    obj->obj_flags.value[3] = atoi(buf);
  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}


void do_ocost(struct char_data *ch, char *arg, int cmd){
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct obj_data *obj;
  
  arg = one_argument(arg, buf2);
  arg = one_argument(arg, buf);
  if (!*buf2 || !is_number(buf)) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try ocost <value>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(obj = read_object(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (is_number(buf))
    obj->obj_flags.cost = atoi(buf);
  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}

void do_orent(struct char_data *ch, char *arg, int cmd){
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct obj_data *obj;
  int  one;
  
  arg = one_argument(arg, buf2);
  arg = one_argument(arg, buf);
  if (!*buf2 || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try orent <obj> <value>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(obj = read_object(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (sscanf(buf, " %d", &one))
    obj->obj_flags.cost_per_day = one;
  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}

void do_okeywords(struct char_data *ch, char *arg, int cmd){
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct obj_data *obj;
  
  half_chop(arg, buf2, buf);
  if (!*buf2 || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try okeywords <obj> keywords.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(obj = read_object(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  
  if (obj_index[obj->item_number].name) {
    str_free(obj_index[obj->item_number].name);
    str_free(obj->name);
   } if (strncmp(buf, "obj ", 4)) {
    sprintf(buf2, "obj %s", buf);
    obj_index[obj->item_number].name = str_alloc(buf2);
    obj->name = str_alloc(buf2);
  } else {
    obj_index[obj->item_number].name = str_alloc(buf);
    obj->name = str_alloc(buf);
  }

  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}



void do_osdesc(struct char_data *ch, char *arg, int cmd){
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct obj_data *obj;
  
  half_chop(arg, buf2, buf);
  if (!is_number(buf2) || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try osdesc <obj> osdesc.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(obj = read_object(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  
  obj->short_description = str_free(obj->short_description);
  obj->short_description = str_alloc(buf);

  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}

void do_oldesc(struct char_data *ch, char *arg, int cmd){
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct obj_data *obj;
  
  half_chop(arg, buf2, buf);
  if (!is_number(buf2) || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try oldesc <obj> oldesc.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(obj = read_object(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  obj->description = str_free(obj->description);
  obj->description = str_alloc(buf);

  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}


void do_oweight(struct char_data *ch, char *arg, int cmd){
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct obj_data *obj;
  
  arg = one_argument(arg, buf2);
  arg = one_argument(arg, buf);
  if (!*buf2 || !is_number(buf)) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try oweight <obj> <value>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(obj = read_object(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (is_number(buf))
    obj->obj_flags.weight = atoi(buf);
  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}


void do_osave(struct char_data *ch, char *arg, int cmd){
  extern void Save_obj(int zone);
  char buf[256];

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (IS_SET(zone_table[world[ch->in_room].zone].dirty_obj,UNSAVED)) {
    Save_obj(world[ch->in_room].zone);
    sprintf(buf, "areas/%s.obj saved.\n\r", zone_table[world[ch->in_room].zone].filename);
  } else
    sprintf(buf, "areas/%s.obj unchanged since last save.\n\r", zone_table[world[ch->in_room].zone].filename);
  ansi(CLR_ACTION, ch);
  send_to_char(buf,ch);
  ansi(END, ch);
  return;
}

/* Give an object an extra description */
void do_oedesc(struct char_data *ch, char *arg, int cmd){
  char arg1[256], arg_obj[MAX_INPUT_LENGTH];
  struct obj_data *obj;
  struct extra_descr_data *theExtra, *prev;

  arg = one_argument(arg, arg_obj);
  arg = one_argument(arg, arg1);
  for (;*arg==' ';arg++); /* strip out preceeding spaces if any */
  if (!*arg1 || !*arg) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try oedesc <obj> <new || append || delete> <keyword list>.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (!(obj = read_object(atoi(arg_obj), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that object.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[obj_index[obj->item_number].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }

  /* find the damn keyword if it exists */
  for (theExtra = obj->ex_description;
       theExtra;
       theExtra = theExtra->next) {
    if (theExtra->keyword)
      if (isname(arg, theExtra->keyword))
        break;
  } /* for loop */

  if (is_abbrev(arg1, "delete")) { /* icky delete */
    if (!theExtra) {
      ansi(CLR_ERROR, ch);
      send_to_char("That Extra description does not exist.\n\r",ch);
      ansi(END, ch);
      return;
    }
    if (obj->ex_description == theExtra) {
      obj->ex_description = theExtra->next;
    } else {
      for(prev=obj->ex_description;
          prev->next != theExtra;
          prev=prev->next);
      prev->next = theExtra->next; /* take this one out of chain */
    }
    ansi(CLR_ACTION, ch);
    send_to_char("Extra description deleted.\n\r",ch);
    ansi(END, ch);
    theExtra->description=str_free(theExtra->description);
    theExtra->keyword=str_free(theExtra->keyword);
    free(theExtra);
    SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
    return;
  }

  if (!theExtra) {
    CREATE(theExtra, struct extra_descr_data, 1);
    theExtra->next = obj->ex_description;
    obj->ex_description = theExtra;
    theExtra->keyword = str_alloc(arg);
    theExtra->description = (char *) 0;
  }

  if (theExtra->description) {
    send_to_char("The old description was: \n\r", ch);
    send_to_char(theExtra->description, ch);
    if (!is_abbrev(arg1, "append")) {
      str_free(theExtra->description);
      theExtra->description = (char *)0;
    }
  }
  ansi(CLR_ACTION, ch);
  send_to_char("\n\rEnter a new description.  Terminate with a '@'.\n\r", ch);
  ansi(END, ch);
  ch->desc->str = &theExtra->description;
  ch->desc->max_str = 4096;
  SET_BIT(zone_table[obj_index[obj->item_number].zone].dirty_obj, UNSAVED);
}


/*****************************************************************
 *                                                               *
 *                                                               *
 *                     M O B    S T U F F                        *
 *                                                               *
 *                                                               *
 *****************************************************************/
void do_mcreate(struct char_data *ch, char *arg, int cmd)
{
  char   buf[512];
  int    zone;
  struct char_data *mob;

  zone = world[ch->in_room].zone;
  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (zone_table[zone].bot_of_mobt == -1) /* check for empty */
    zone_table[zone].bot_of_mobt = top_of_mobt +1;
  /* and what if its full */
  else if (mob_index[zone_table[zone].top_of_mobt].virtual+1 > zone_table[zone].top) {
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry but this zone is full... nothing more can be added.\n\r",ch);
    ansi(END, ch);
    return;
  }
  /* fix the top/bottom of the index for the new entry */
  top_of_mobt++;
  allocate_mob(top_of_mobt);
  SET_BIT(zone_table[zone].dirty_mob, UNSAVED);
  SET_BIT(zone_table[zone].dirty_mob, SCRAMBLED);
  if (zone_table[zone].top_of_mobt >= 0) {
    mob_index[top_of_mobt].virtual = mob_index[zone_table[zone].top_of_mobt].virtual+1;
    mob_index[zone_table[zone].top_of_mobt].next = top_of_mobt;
  } else {
    mob_index[top_of_mobt].virtual = zone_table[zone].bottom;
  }
  mob_index[top_of_mobt].next = top_of_mobt+1;
  mob_index[top_of_mobt].zone = zone;
  mob_index[top_of_mobt].number = 0;
  mob_index[top_of_mobt].func = 0;
  mob_index[top_of_mobt].next = top_of_mobt+1;
  zone_table[zone].top_of_mobt = top_of_mobt;
  
  /* give it some initial values */
  CREATE(mob, struct char_data, 1);
  clear_char(mob);

  mob_index[top_of_mobt].name = str_alloc("mob blank");
  mob->player.name = str_alloc("mob blank");
  mob->player.short_descr= str_alloc("a blank mob"); 
  mob->player.long_descr= str_alloc("A blank mob stands here, looking formless.\n\r");
  mob->player.description=str_alloc("It is pretty formless, awaiting a creators molding.\n\r");
  mob->player.title = 0;

  mob->specials.act = ACT_ISNPC;
  mob->specials.affected_by = 0;
  mob->specials.alignment = 0;
  GET_LEVEL(mob) = 1;
  mob->points.hitroll = 0;
  mob->points.armor = 240;
  mob->points.max_hit = 1;
  mob->points.hit = 1;
  mob->player.weight = 1;
  mob->points.damroll = 1;
  mob->specials.damnodice = 1;
  mob->specials.damsizedice = 1;
  mob->points.mana = 10;
  mob->points.max_mana = 100;
  mob->points.move = 50;
  mob->points.max_move = 50;
  mob->points.gold = 1;
  mob->specials.position = POSITION_STANDING;
  mob->specials.default_pos = POSITION_STANDING;
  mob->player.sex = 0;
  mob->player.class = 0;
  mob->player.time.birth = time(0);
  mob->player.time.played = 0;
  mob->player.time.logon = mob->player.time.birth;
  mob->player.height = 198;
  mob->next = 0;
  mob->nr = top_of_mobt;
  mob->desc = 0;
  
  mob->in_room = NOWHERE;

  mob_index[top_of_mobt].prototype = (char *) mob;
  /* let 'em know what happened */
  sprintf(buf, "Done. Mob #%d created!", mob_index[top_of_mobt].virtual);
  ansi(CLR_ACTION, ch);
  send_to_char(buf, ch);
  ansi(END, ch);
} /* proc */


void do_mkeywords(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  
  half_chop(arg, buf2, buf);
  if (!*buf2 || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mkeywords <mob> keywords.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { 
    /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  
  if (mob_index[mob->nr].name) {
    str_free (mob_index[mob->nr].name);
    str_free (mob->player.name);
   } if (strncmp(buf, "mob ", 4)) {
    sprintf(buf2, "mob %s", buf);
    mob_index[mob->nr].name = str_alloc(buf2);
    mob->player.name = str_alloc(buf2);
  } else {
    mob_index[mob->nr].name = str_alloc(buf);
    mob->player.name = str_alloc(buf);
  }
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob,UNSAVED);
}



void do_mlist(struct char_data *ch, char *arg, int cmd)
{
  int i, zone, start, end, no=1;
  char buf[2*MAX_STRING_LENGTH], buf1[128], buf2[128];
  
  zone = world[ch->in_room].zone;
  half_chop(arg, buf1, buf2);
  
  if (is_number(buf1)) {
    start = atoi(buf1);
    zone = zone_of(start);
  } else 
    start = zone_table[zone].bottom;   

  if (is_number(buf2))
    end = atoi(buf2);
  else 
    end = zone_table[zone].top;
  if (end < start)
    end = start+15;
  if (end > zone_table[zone].top)
    end = zone_table[zone].top;
  if (end - start >= 100) { 
    end = start + 99;
    ansi(CLR_ERROR, ch);
    send_to_char("Limiting search to 200 rooms (try mlist <start num> <end num> for rest).\n\r",ch);
    ansi(END, ch);
  } 

  if ((GET_LEVEL(ch) < IMO_LEV2) && (!isexactname(GET_NAME(ch), zone_table[zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  sprintf(buf, "\n\r"); /* init buf to something for strcat */
  for (i = zone_table[zone].bot_of_mobt; (i>=0) && (i <= top_of_mobt) && (mob_index[i].virtual <= end); i=mob_index[i].next) {
    if (mob_index[i].virtual >= start) {
      sprintf(buf1, "[%d] [%-28s] ", mob_index[i].virtual, mob_index[i].name);
      if (!(no%2))
        strcat(buf, "\n\r");
      no++;
      strcat(buf, buf1);
    }
  }
  sprintf(buf1, "Done. Mobs[%d to %d]\n\r", start, end);
  strcat(buf, buf1);
  page_string(ch->desc, buf, 1); 
}


void do_mflag(struct char_data *ch, char *arg, int cmd){
  char buf[512], buf1[512], buf2[512];
  int loc,i;
  unsigned long flag;
  struct char_data *mob;

  half_chop(arg, buf1, buf2);
  if (!is_number(buf1) || !*buf2) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mflag <mob> <flag>.\n\r", ch);
    ansi(END, ch);
    sprintf(buf, "Mob flags: \n\r");
    for (i = 0; *action_bits[i] != '\n'; i++) {
      strcat(buf, action_bits[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    ansi(CLR_ACTION, ch);
    send_to_char(buf,ch);
    ansi(END, ch);
    return;
  }
  loc = (old_search_block(buf2, 0, strlen(buf2), action_bits, -1));
  if (loc == -1) { /* the -1 in old_search_block lowercases the list */
    ansi(CLR_ERROR, ch);
    send_to_char("That flag doesn't exist.\n\r", ch);
    ansi(END, ch);
    return;
  }
  loc--;  /* search block returns 1 as first position */

  if (!(mob = read_mobile(atoi(buf1), PROTOTYPE))) {
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }

  flag = 1;
  for (i = 0; i < loc; i++)
    flag *= 2;

  if (IS_SET(mob->specials.act, flag)) {
    ansi(CLR_ACTION, ch);
    sprintf(buf, "You have removed the '%s' flag.\n\r", action_bits[loc]);
    ansi(END, ch);
    REMOVE_BIT(mob->specials.act, flag);
    send_to_char(buf, ch);
  } else {
    sprintf(buf, "You have set the '%s' flag.\n\r", action_bits[loc]);
    SET_BIT(mob->specials.act, flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  }
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}

void do_msex(struct char_data *ch, char *arg, int cmd)
{
  char buf[512], buf1[512], buf2[512];
  int loc,i;
  struct char_data *mob;
  const char *sex[] = {
    "it",
    "male",
    "female",
    "\n"
  };

  half_chop(arg, buf1, buf2);
  if (!is_number(buf1) || !*buf2) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mflag <mob> <flag>.\n\r", ch);
    ansi(END, ch);
    sprintf(buf, "Mob flags: \n\r");
    for (i = 0; *sex[i] != '\n'; i++) {
      strcat(buf, sex[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    ansi(CLR_ACTION, ch);
    send_to_char(buf,ch);
    ansi(END, ch);
    return;
  }
  loc = (old_search_block(buf2, 0, strlen(buf2), sex, 0));
  if (loc == -1) { /* the -1 in old_search_block lowercases the list */
    ansi(CLR_ERROR, ch);
    send_to_char("That sex doesn't exist.\n\r", ch);
    ansi(END, ch);
    return;
  }
  loc--;  /* search block returns 1 as first position */

  if (!(mob = read_mobile(atoi(buf1), PROTOTYPE))) {
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }

  mob->player.sex = loc;
  sprintf(buf, "You have made the %s '%s'.\n\r", fname(mob_index[mob->nr].name), sex[loc]);
  ansi(CLR_ACTION, ch);
  send_to_char(buf, ch);
  ansi(END, ch);
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}

void do_maffect(struct char_data *ch, char *arg, int cmd)
{
  char buf[1024], buf1[256], buf2[256];
  int loc,i;
  unsigned long flag;
  struct char_data *mob;

  half_chop(arg, buf1, buf2);
  if (!is_number(buf1) || !*buf2) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try maffect <mob> <flag>.\n\r", ch);
    ansi(END, ch);
    sprintf(buf, "Mob flags: \n\r");
    for (i = 0; *affected_bits[i] != '\n'; i++) {
      strcat(buf, affected_bits[i]);
      strcat(buf, " ");
    }
    strcat(buf,"\n\r");
    ansi(CLR_ACTION, ch);
    send_to_char(buf,ch);
    ansi(END, ch);
    return;
  }
  loc = (old_search_block(buf2, 0, strlen(buf2), affected_bits, -1));
  if (loc == -1) { /* the -1 in old_search_block lowercases the list */
    ansi(CLR_ERROR, ch);
    send_to_char("That flag doesn't exist.\n\r", ch);
    ansi(END, ch);
    return;
  }
  loc--;  /* search block returns 1 as first position */

  if (!(mob = read_mobile(atoi(buf1), PROTOTYPE))) {
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }

  flag = 1;
  for (i = 0; i < loc; i++)
    flag *= 2;

  if (IS_SET(mob->specials.affected_by, flag)) {
    sprintf(buf, "You have removed the '%s' flag.\n\r", affected_bits[loc]);
    REMOVE_BIT(mob->specials.affected_by, flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  } else {
    sprintf(buf, "You have set the '%s' flag.\n\r", affected_bits[loc]);
    SET_BIT(mob->specials.affected_by, flag);
    ansi(CLR_ACTION, ch);
    send_to_char(buf, ch);
    ansi(END, ch);
  }
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}


void do_msdesc(struct char_data *ch, char *arg, int cmd){
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  
  half_chop(arg, buf2, buf);
  if (!is_number(buf2) || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try msdesc <mob> msdesc.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  
  if (mob->player.short_descr)
    str_free (mob->player.short_descr);
  mob->player.short_descr = str_alloc(buf);

  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}

void do_mldesc(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  
  half_chop(arg, buf2, buf);
  if (!is_number(buf2) || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mldesc <mob> mldesc.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  
  if (mob->player.long_descr)
    str_free (mob->player.long_descr);
  strcat(buf, "\n\r");
  mob->player.long_descr = str_alloc(buf);

  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}

void do_mdesc(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  
  half_chop(arg, buf2, buf);
  if (!is_number(buf2) || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mdesc <mob> <new || append>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  
  if (mob->player.description) {
    ansi(CLR_ACTION, ch);
    send_to_char("The old description was: \n\r", ch);
    ansi(END, ch);
    send_to_char(mob->player.description, ch);
    if (!is_abbrev(buf, "append")) {
      str_free((char *)mob->player.description);
      mob->player.description = (char *)0;
    }
  } else {
    mob->player.description = (char *)0;
  }
  ansi(CLR_ACTION, ch);
  send_to_char("\n\rEnter a new description.  Terminate with a '@'.\n\r", ch);
  ansi(END, ch);
  ch->desc->str = &(mob->player.description);
  ch->desc->max_str = 2048;
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}

void do_mac(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  int one;

  arg = one_argument(arg, buf2);
  arg = one_argument(arg, buf);
  if (!*buf2 || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mac <mob> <value>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  sscanf(buf, " %d", &one);
  mob->points.armor=one;
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}

void do_malign(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  int value;

  arg = one_argument(arg, buf2);
  arg = one_argument(arg, buf);
  if (!*buf2 || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try malign <mob> <value>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  sscanf(buf," %d", &value);
  GET_ALIGNMENT(mob) = MINV(1000, value);
  GET_ALIGNMENT(mob) = MAXV(-1000, value);
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}


void do_mthaco(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  int one; 

  arg = one_argument(arg, buf2);
  arg = one_argument(arg, buf);
  if (!*buf2 || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mthaco <mob> <value>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (sscanf(buf, " %d", &one))
    mob->points.hitroll = 30-one;
  else
    mob->points.hitroll = 30;
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}


void do_mhitpoints(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  char throwaway;
  int one, two, three;
  
  arg = one_argument(arg, buf2);
  arg = one_argument(arg, buf);
  if (!*buf2 || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mhitpoints <mob> <value>d<value>+<value>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  sscanf(buf, "%d %c %d + %d", &one, &throwaway, &two, &three);
  mob->points.max_hit = MAXV(1, one);
  mob->player.weight = MAXV(1, two);
  mob->points.hit = MAXV(0, three);
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}


void do_mdamage(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  char throwaway;
  int  one, two, three;
  
  arg = one_argument(arg, buf2);
  arg = one_argument(arg, buf);
  if (!*buf2 || !*buf) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mdamage <mob> <value>d<value>+<value>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  sscanf(buf, "%d %c %d + %d", &one, &throwaway, &two, &three);
  mob->specials.damnodice = MAXV(1, one);
  mob->specials.damsizedice = MAXV(1, two);
  mob->points.damroll = MAXV(0, three);
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}

void do_mlevel(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  
  arg = one_argument(arg, buf2);
  arg = one_argument(arg, buf);
  if (!*buf2 || !is_number(buf)) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mlevel <mob> <value>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (is_number(buf))
    GET_LEVEL(mob) = MAXV(0,MINV(NPC_LEV,atoi(buf)));
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}

void do_mexp(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  
  arg = one_argument(arg, buf2);
  arg = one_argument(arg, buf);
  if (!*buf2 || !is_number(buf)) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mexp <mob> <value>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (is_number(buf))
    GET_EXP(mob) = MAXV(1,atoi(buf));
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}

void do_mgold(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct char_data *mob;
  
  arg = one_argument(arg, buf2);
  arg = one_argument(arg, buf);
  if (!*buf2 || !is_number(buf)) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mgold <mob> <value>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf2), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (is_number(buf))
    mob->points.gold = MAXV(0,atoi(buf));
  SET_BIT(zone_table[mob_index[mob->nr].zone].dirty_mob, UNSAVED);
}


void do_mstat(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  struct char_data *mob;
 
  arg = one_argument(arg, buf);
  if (!is_number(buf)) {
    ansi(CLR_ACTION, ch);
    send_to_char("Try mstat <mob>.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if (!(mob = read_mobile(atoi(buf), PROTOTYPE))) { /* if allready loaded will do nothing */
    ansi(CLR_ERROR, ch);
    send_to_char("Sorry don't know that mob.\n\r", ch);
    ansi(END, ch);
    return;
  }

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[mob_index[mob->nr].zone].lord))) {
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    return;
  }

/* output the results of the stat */
  sprintf(buf, "Mob#:[%d]  Name:[%s] Sex:[", mob_index[mob->nr].virtual, mob->player.name);
  switch(mob->player.sex) {
  case SEX_NEUTRAL:
	strcat(buf, "None]\n\r"); break;
  case SEX_MALE:
	strcat(buf, "Male]\n\r"); break;
  case SEX_FEMALE:
	strcat(buf, "Female]\n\r"); break;
  default:
	strcpy(buf, "???]\n\r"); break;
  }
  send_to_char(buf, ch);

  sprintf(buf, "sdesc:[%s] name:[%s]\n\rldesc:[%s] Description:\n\r%s\n\r",
               ((mob->player.short_descr) ? mob->player.short_descr : "None"),
               fname(mob_index[mob->nr].name), 
               ((mob->player.long_descr) ? mob->player.long_descr : "None"),
               ((mob->player.description) ? mob->player.description : "None") );
  send_to_char(buf, ch);

  send_to_char("Flag(s) : ", ch);
  sprintbit(mob->specials.act,action_bits,buf);
  strcat(buf,"\n\r");
  send_to_char(buf, ch);

  send_to_char("Affect(s) : ", ch);
  sprintbit(mob->specials.affected_by,affected_bits,buf);
  strcat(buf,"\n\r");
  send_to_char(buf, ch);

  sprintf(buf, "Align:[%d] Level:[%d] Gold:[%d] Experience:[%d]\n\r",
    mob->specials.alignment, GET_LEVEL(mob), mob->points.gold, GET_EXP(mob));
  send_to_char(buf, ch);

  sprintf(buf, "AC:[%d] HP:[%dD%d+%d] THAC0:[%d] Damage:[%dD%d+%d]\n\r",
    mob->points.armor, mob->points.max_hit, mob->player.weight, mob->points.hit,
    30 - mob->points.hitroll, mob->specials.damnodice, mob->specials.damsizedice, 
    mob->points.damroll);
  send_to_char(buf, ch);

  return;
}


void do_msave(struct char_data *ch, char *arg, int cmd)
{
  extern void Save_obj(int zone);
  char buf[256];

  if ((GET_LEVEL(ch) < IMP_LEV) && (!isexactname(GET_NAME(ch), zone_table[world[ch->in_room].zone].lord))) {
    ansi(CLR_ERROR, ch);
    send_to_char("You are not authorized to create in that zone.\n\r", ch);
    ansi(END, ch);
    return;
  }
  if (IS_SET(zone_table[world[ch->in_room].zone].dirty_mob,UNSAVED)) {
    Save_mob(world[ch->in_room].zone);
    sprintf(buf, "areas/%s.mob saved.\n\r", zone_table[world[ch->in_room].zone].filename);
  } else
    sprintf(buf, "areas/%s.mob unchanged since last save.\n\r", zone_table[world[ch->in_room].zone].filename);
  send_to_char(buf,ch);
  return;
}



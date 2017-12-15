/*************************************************************************
 *                                                                       * 
 *  File: tuscany.c                                                      *
 *  Special procedure handling Tuscany quest.                            *
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
#include "ansi.h"

#define ARTIFACT_ACT "As the ancient artifact is dropped to the floor, a magical portal instantly\n\rappears and you feel that you are drawn into it by some unknown force.  You\n\rstart to feel uneasy as you realize that you must have disturbed the\n\ralmighty gods.\n\r" 

#define PORTAL_ACT "You finally summoned enough courage to enter the portal.  You feel a sudden\n\rchill goes through your body and that really makes you shiver.........\n\r"   

#define DESTINATION 15847
#define ARTIFACT    15687
#define PORTAL      15688

int chamber_of_gods(struct char_data *ch, int cmd, char *arg)
{
  /*
    Chamber of Gods               : room 15746
    Entrance to the Halls of Gods : room 15847

    The Ancient Artifact : object 15687
    The Magic Portal     : object 15688
    */

  struct obj_data *obj;
  struct char_data *leader;
  struct follow_type *fol;
  char buf[100];

  if (cmd == CMD_DROP) {
    do_drop(ch, arg, cmd); 
    if ((obj=get_obj_in_list_virtual(ARTIFACT, world[ch->in_room].contents))) {
      if (GET_LEVEL(ch) < IMO_LEV) {
	ansi_act("Sorry, you are too inexperienced to meet the Gods now!",
		 FALSE, fol->follower, 0, 0, TO_CHAR, CLR_ERROR);
	return(FALSE);
      }
      extract_obj(obj);
      ansi_act(ARTIFACT_ACT, FALSE, ch, 0, 0, TO_ROOM, WHITE);      
      obj = read_object(PORTAL, VIRTUAL);
      obj_to_room(obj, ch->in_room);
      return(TRUE);
    }
  }

  if ((cmd == 7) 
      && (obj=get_obj_in_list_virtual(PORTAL, world[ch->in_room].contents))) { 
    /* ENTER command */
      if (GET_LEVEL(ch) < IMO_LEV) {
	ansi_act("Sorry, you are too inexperienced to meet the Gods now!",
		 FALSE, fol->follower, 0, 0, TO_CHAR, CLR_ERROR);
	return(FALSE);
      }
      extract_obj(obj);
      ansi_act(PORTAL_ACT, FALSE, fol->follower, 0, 0, TO_CHAR, WHITE);
      char_from_room(ch);
      char_to_room(ch, real_room(DESTINATION));
      do_gossip(ch, "YES! I have solved the Tuscany quest!", 232);
      do_look(ch, "\0", 15);
      
      sprintf(buf,"QUEST: %s visited the Halls of Gods.", GET_NAME(ch));
      log(buf);
      
      if (!(leader = ch->master))
	leader = ch;

      for(fol = leader->followers; fol; fol = fol->next)
        if (!IS_NPC(fol->follower) && IS_AFFECTED(fol->follower, AFF_GROUP)) {
	  if (GET_LEVEL(fol->follower) < IMO_LEV) 
	    ansi_act("Sorry, you are too inexperienced to meet the Gods now!",
		     FALSE, fol->follower, 0, 0, TO_CHAR, CLR_ERROR);
	  else {
	    ansi_act(PORTAL_ACT, FALSE, fol->follower, 0, 0, TO_CHAR, WHITE);
	    char_from_room(fol->follower);
	    char_to_room(fol->follower, real_room(DESTINATION));
	    do_look(fol->follower, "\0", 15);
	  }
	}
      return(TRUE);
    }
  return(FALSE);
}





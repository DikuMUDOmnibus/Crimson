/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "limits.h"
#include "db.h"
#include "constants.h"
#include "spells.h"
#include "fight.h"
#include "act.h"
#include "ansi.h"

/* Data declarations */

struct social_type {
  char *cmd;
  int next_line;
};


/* ********************************************************************
*  Special procedures for rooms                                       *
******************************************************************** */

char *how_good(int percent)
{
   static char buf[256];

   if (percent == 0)
      strcpy(buf, " (not learned)");
   else if (percent <= 10)
      strcpy(buf, " (awful)");
   else if (percent <= 20)
      strcpy(buf, " (pathetic)");
   else if (percent <= 30)
      strcpy(buf, " (bad)");
   else if (percent <= 40)
      strcpy(buf, " (poor)");
   else if (percent <= 50)
      strcpy(buf, " (below average)");
   else if (percent <= 60)
      strcpy(buf, " (average)");
   else if (percent <= 70)
      strcpy(buf, " (above average)");
   else if (percent <= 80)
      strcpy(buf, " (good)");
   else if (percent <= 90)
      strcpy(buf, " (very good)");
   else
      strcpy(buf, " (Superb)");
   return (buf);
}

int guild(struct char_data *ch, int cmd, char *arg) {

#define PR_MAGE_OS      0
#define PR_THIEF_OS     5
#define PR_CLER_OS      17
#define PR_WAR_OS       23
#define PR_BARD_OS      34
#define PR_PAL_OS       43
#define PR_APAL_OS      53
#define PR_EKNI_OS      62
#define PR_DKNI_OS      71
#define PR_DRAG_OS      79

   char buf[MAX_STRING_LENGTH], buf2[256];
   int temp_offset, skill_max, skill_div, number, i, percent;

   char **temp_skills;
   static const char *m_skills[] = {
      "extra damage",   /* 0 */
      "slashing weapons",
      "piercing weapons",
      "bludgeoning weapons",
      "second attack",
      "\n"
   };

   static const char *t_skills[] = {
      "sneak",   /* No. 5 */
      "hide",
      "steal",
      "backstab",
      "picklock",
      "extra damage",
      "slashing weapons",
      "piercing weapons",
      "bludgeoning weapons",
      "dodge",
      "second attack",
      "third attack",
      "\n"
   };

   static const char *c_skills[] = {
      "extra damage",   /* 17 */
      "slashing weapons",
      "piercing weapons",
      "bludgeoning weapons",
      "second attack",
      "third attack",
      "\n"
   };

   static const char *w_skills[] = {
      "kick",  /* No. 23 */
      "bash",
      "rescue",
      "extra damage",
      "slashing weapons",
      "piercing weapons",
      "bludgeoning weapons",
      "parry death blow",
      "second attack",
      "third attack",
      "fourth attack",
      "\n"
   };

   static const char *b_skills[] = {
      "sneak",   /* 34*/
      "picklock",
      "extra damage",
      "slashing weapons",
      "piercing weapons",
      "bludgeoning weapons",
      "dodge",
      "second attack",
      "third attack",
      "\n"
   };

   static const char *p_skills[] = {
      "kick",  /* No. 43 */
      "bash",
      "rescue",
      "extra damage",
      "slashing weapons",
      "piercing weapons",
      "bludgeoning weapons",
      "second attack",
      "third attack",
      "fourth attack",
      "\n"
   };

   static const char *ap_skills[] = {
      "kick",  /* No. 53 */
      "bash",
      "extra damage",
      "slashing weapons",
      "piercing weapons",
      "bludgeoning weapons",
      "second attack",
      "third attack",
      "fourth attack",
      "\n"
   };

   static const char *ek_skills[] = {
      "kick",  /* No. 62 */
      "bash",
      "rescue",
      "extra damage",
      "slashing weapons",
      "piercing weapons",
      "bludgeoning weapons",
      "second attack",
      "third attack",
      "\n"
   };

   static const char *dk_skills[] = {
      "kick",  /* No. 71 */
      "bash",
      "extra damage",
      "slashing weapons",
      "piercing weapons",
      "bludgeoning weapons",
      "second attack",
      "third attack",
      "\n"
   };

   static const char *d_skills[] = {
      "kick",  /* No. 79 */
      "bash",
      "extra damage",
      "slashing weapons",
      "piercing weapons",
      "bludgeoning weapons",
      "tougher scales",
      "fire breath",
      "gas breath",
      "frost breath",
      "acid breath",
      "lightning breath",
      "darksight",
      "second attack",
      "third attack",
      "fourth attack",  /*94*/
      "\n"
   };

const sbyte skill_num[95][10] = {
      /* skill,lev1,lev2,lev3,max1,max2,max3,gain_div1,gain_div2,gain_div3 */
    /* Mage */
      {56,12,25,35,20,40,50,6,10,13},  /* damage  - 0 */
      {57,7,20,35,20,40,50,8,10,13},  /* slash */
      {58,5,15,35,20,40,50,7,10,13},  /* pierce */
      {59,7,20,35,20,40,50,8,10,13},  /* bludgeon */
      {69,15,25,35,20,50,75,5,6,10},  /* 2nd att */
    /* Thief */
      {45,1,10,20,50,75,90,2,3,5},   /* sneak- 5 */
      {46,3,15,25,50,75,90,2,3,5},  /* hide */
      {47,5,15,30,45,60,75,3,4,8},  /* steal */
      {48,1,10,30,45,60,75,3,4,8},  /* backstab */
      {49,3,10,25,50,75,90,2,3,5},  /* picklock */
      {56,10,20,30,20,40,50,6,10,13},  /* damage - 10*/
      {57,3,12,30,20,40,60,6,10,13},  /* slash */
      {58,1,10,25,30,40,60,5,10,13},  /* pierce */
      {59,5,15,35,20,40,50,7,10,13},  /* bludgeon */
      {61,7,15,25,30,50,80,5,6,10},  /* dodge */
      {69,15,25,35,25,50,70,7,8,10},  /* 2nd att - 15*/
      {70,25,30,35,20,25,30,10,11,13},  /* 3rd att */
    /* Cleric */
      {56,10,20,30,20,40,50,6,10,13},  /* damage */
      {57,5,15,35,20,40,50,7,10,13},  /* slash */
      {58,3,12,30,20,40,60,6,10,13},  /* pierce */
      {59,1,10,25,30,40,60,5,10,13},  /* bludgeon - 20*/
      {69,15,25,35,25,50,80,7,8,10},  /* 2nd att */
      {70,25,30,35,20,30,35,10,11,13},  /* 3rd att */
    /* Warrior */
      {50,7,10,30,50,80,90,2,3,4},   /* kick */
      {51,7,10,30,50,80,90,2,3,4},  /* bash */
      {52,3,5,30,50,80,90,2,3,4},  /* rescue - 25*/
      {56,5,15,25,30,50,70,5,10,13},  /* damage */
      {57,1,10,20,30,50,100,5,10,12},  /* slash */
      {58,1,10,20,30,50,100,5,10,12},  /* pierce */
      {59,1,10,20,30,50,100,5,10,12},  /* bludgeon */
      {60,15,25,35,20,25,30,10,12,13},  /* parry - 30 */
      {69,10,20,30,30,60,90,5,6,10},  /* 2nd att */
      {70,20,25,30,30,40,50,6,8,10},  /* 3rd att */
      {71,30,35,38,20,30,40,10,11,13},  /* 4th att */
   /* Bard */
      {45,3,15,25,50,75,90,3,4,5},   /* sneak */
      {49,7,20,30,50,75,90,3,4,5},  /* picklock - 35*/
      {56,10,20,30,20,40,50,6,10,13},  /* damage */
      {57,3,12,30,20,40,60,6,10,13},  /* slash */
      {58,3,12,30,30,40,60,5,10,13},  /* pierce */
      {59,3,12,30,20,40,60,7,10,13},  /* bludgeon */
      {61,12,20,30,30,50,80,5,6,10},  /* dodge - 40*/
      {69,15,25,35,25,50,80,7,8,10},  /* 2nd att */
      {70,25,30,38,20,30,35,10,11,13},  /* 3rd att */
   /* Kai */
      {50,10,15,35,50,80,90,2,3,4},   /* kick */
      {51,10,15,35,50,80,90,2,3,4},  /* bash */
      {52,5,10,35,40,60,70,2,3,4},  /* rescue - 45*/
      {56,5,15,30,30,50,70,5,10,13},  /* damage */
      {57,1,12,25,30,50,80,5,10,12},  /* slash */
      {58,1,12,25,30,50,80,5,10,12},  /* pierce */
      {59,1,12,25,30,50,80,5,10,12},  /* bludgeon */
      {69,12,22,30,30,60,80,5,6,10},  /* 2nd att - 50*/
      {70,22,30,35,20,30,40,6,8,10},  /* 3rd att */
      {71,32,35,38,10,20,25,11,12,14},  /* 4th att */
    /* Drakkhar */
      {50,10,15,35,50,80,90,2,3,4},   /* kick */
      {51,10,15,35,50,80,90,2,3,4},  /* bash */
      {56,5,15,30,30,50,70,5,10,13},  /* damage - 55*/
      {57,1,12,25,30,50,80,5,10,12},  /* slash */
      {58,1,12,25,30,50,80,5,10,12},  /* pierce */
      {59,1,12,25,30,50,80,5,10,12},  /* bludgeon */
      {69,12,22,30,30,60,80,5,6,10},  /* 2nd att */
      {70,22,30,35,20,30,40,6,8,10},  /* 3rd att - 60*/
      {71,32,35,38,10,20,25,11,12,14},  /* 4th att */
    /* Elfin Knight */
      {50,10,15,35,40,60,70,2,3,4},   /* kick */
      {51,10,15,35,40,60,70,2,3,4},  /* bash */
      {52,5,10,35,40,60,70,2,3,4},  /* rescue */
      {56,7,20,35,20,40,50,5,10,13},  /* damage - 65*/
      {57,3,12,30,20,40,80,5,10,12},  /* slash */
      {58,3,12,30,20,40,80,5,10,12},  /* pierce */
      {59,3,12,30,20,40,80,5,10,12},  /* bludgeon */
      {69,12,25,30,30,60,80,5,6,10},  /* 2nd att */
      {70,25,30,35,20,35,50,6,8,13},  /* 3rd att - 70*/
    /* Demon Knight */
      {50,10,15,35,40,60,70,2,3,4},   /* kick */
      {51,10,15,35,40,60,70,2,3,4},  /* bash */
      {56,7,20,35,20,40,50,5,10,13},  /* damage */
      {57,3,12,30,20,40,80,5,10,12},  /* slash */
      {58,3,12,30,20,40,80,5,10,12},  /* pierce - 75*/
      {59,3,12,30,20,40,80,5,10,12},  /* bludgeon */
      {69,12,25,30,30,60,80,5,6,10},  /* 2nd att */
      {70,25,30,35,20,35,50,6,8,13},  /* 3rd att */
    /* Dragon-Kin Warrior */
      {50,7,15,30,40,60,80,3,4,5},   /* kick */
      {51,7,10,25,60,85,95,2,3,4},  /* bash - 80*/
      {56,5,15,25,40,70,100,5,8,12},  /* damage */
      {57,1,15,30,10,40,80,7,11,13},  /* slash */
      {58,1,12,25,20,50,90,5,10,12},  /* pierce */
      {59,1,10,20,30,60,120,5,8,11},  /* bludgeon */
      {62,3,12,25,30,70,120,6,10,13},  /* scales - 85*/
      {63,25,30,35,30,60,95,3,4,5},  /* fire */
      {64,1,5,10,10,50,95,3,4,5},  /* gas */
      {65,10,15,20,30,60,90,3,4,5},  /* frost */
      {66,15,20,25,30,60,90,3,4,5},  /* acid */
      {67,20,25,30,30,60,90,3,4,5},  /* lightning - 90*/
      {68,3,5,10,30,75,100,5,2,1},  /* darksight */
      {69,10,20,30,30,60,90,5,6,10},  /* 2nd att */
      {70,20,25,30,25,35,45,6,8,10},  /* 3rd att */
      {71,30,35,38,10,20,30,10,12,13}  /* 4th att - 94*/
   };

   if ((cmd != 164) && (cmd != 170)) return(FALSE);
   if (IS_NPC(ch)) {
      ansi(CLR_ERROR, ch);
      send_to_char("Mobs don't practice much of anything...\n\r", ch);
      ansi(END, ch);
      return(FALSE);
   }
   buf[0] = 0; /* going to be strcat'ing to this */
   sprintf(buf2, "                                      "); /* 38 spcs */

   for(; (arg && *arg==' '); arg++);
   number = -1;
   switch (GET_CLASS(ch)) {
      case CLASS_MAGIC_USER :{
         temp_skills = (char**)m_skills;
         temp_offset = PR_MAGE_OS;
         if (!arg || !*arg) {
            send_to_char("You can practise any of these spells:\n\r", ch);
            number = 0;
            for(i=0; *spells[i] != '\n'; i++)
               if (spell_info[i+1].spell_pointer &&
		 (spell_info[i+1].min_level_magic <= GET_LEVEL(ch)))
                  if (arg || (!arg && ch->skills[i+1].learned)){
                     strcat(buf, spells[i]);
                     strcat(buf, how_good(ch->skills[i+1].learned));
                     if (number == 1) {
                        strcat(buf, "\n\r");
                        send_to_char(buf, ch);
			buf[0]=0;
                        number=0;
                     } else {
                        number++;
                        strcat(buf, buf2 + strlen(buf)); /* pad out to 38 spaces */
                     }
                  }
            if (number==1) send_to_char("\n\r", ch);
            number = -1;
         } else
            number = old_search_block(arg,0,strlen(arg),spells,FALSE);
            if(number != -1) {
               if (GET_LEVEL(ch) < spell_info[number].min_level_magic) {
                  send_to_char("You do not know of this spell...\n\r", ch);
                  return(TRUE);
               }
               if (ch->specials.spells_to_learn <= 0) {
		  ansi(CLR_ERROR, ch);
                  send_to_char("You do not seem to be able to practice now.\n\r", ch);
		  ansi(END, ch);
                  return(TRUE);
               }
               if (ch->skills[number].learned >= 95) {
		  ansi(CLR_ERROR, ch);
                  send_to_char("You are already learned in this area.\n\r", ch);
		  ansi(END, ch);
                  return(TRUE);
               }
               send_to_char("You Practice for a while...\n\r", ch);
               ch->specials.spells_to_learn--;

               percent = ch->skills[number].learned+MAXV(25,int_app[GET_INT(ch)].learn);
               ch->skills[number].learned = MINV(95, percent);

               if (ch->skills[number].learned >= 95) {
                  send_to_char("You are now learned in this area.\n\r", ch);
               }
               return(TRUE);
            }
      } break;

      case CLASS_THIEF: {
         temp_skills = (char**)t_skills;
         temp_offset = PR_THIEF_OS;
         if (GET_LEVEL(ch) >= IMO_LEV) {
         if (!arg || !*arg) {
            send_to_char("You can practise any of these spells:\n\r", ch);
            for(i=0; *spells[i] != '\n'; i++)
               if ( (spell_info[i+1].spell_pointer) 
		&&(
                  (spell_info[i+1].min_level_cleric <= GET_LEVEL(ch)/2)
                  ||(spell_info[i+1].min_level_magic <= GET_LEVEL(ch)/2)
                ))
		if (arg || (!arg && ch->skills[i+1].learned)){
                  send_to_char((char*)spells[i], ch);
                  send_to_char(how_good(ch->skills[i+1].learned), ch);
                  send_to_char("\n\r", ch);
               }
         } else
            number = old_search_block(arg,0,strlen(arg),spells,FALSE);
         if(number != -1) {
            if (GET_LEVEL(ch) < spell_info[number].min_level_cleric) {
               send_to_char("You do not know of this spell...\n\r", ch);
               return(TRUE);
            }
            if (ch->specials.spells_to_learn <= 0) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not seem to be able to practice now.\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->skills[number].learned >= 95) {
               send_to_char("You are already learned in this area.\n\r", ch);
               return(TRUE);
            }
            send_to_char("You Practice for a while...\n\r", ch);
            ch->specials.spells_to_learn--;  

            percent = ch->skills[number].learned+MAXV(25,int_app[GET_INT(ch)].learn);
            ch->skills[number].learned = MINV(95, percent);  

            if (ch->skills[number].learned >= 95) {
               send_to_char("You are now learned in this area.\n\r", ch);
            }
            return(TRUE);
         }
      }
      } break;


      case CLASS_CLERIC: {
         temp_skills = (char**)c_skills;
         temp_offset = PR_CLER_OS;
         if (!arg || !*arg) {
            send_to_char("You can practise any of these spells:\n\r", ch);
	    number = 0;
            for(i=0; *spells[i] != '\n'; i++)
               if (spell_info[i+1].spell_pointer &&
		   (spell_info[i+1].min_level_cleric <= GET_LEVEL(ch)))
                  if (arg || (!arg && ch->skills[i+1].learned)){
                     strcat(buf, spells[i]);
                     strcat(buf, how_good(ch->skills[i+1].learned));
                     if (number == 1) {
                        strcat(buf, "\n\r");
                        send_to_char(buf, ch);
			buf[0]=0;
                        number=0;
                     } else {
                        number++;
                        strcat(buf, buf2 + strlen(buf)); /* pad out to 38 spaces */
                     }
                  }
            if (number==1) send_to_char("\n\r", ch);
            number = -1;
         } else
            number = old_search_block(arg,0,strlen(arg),spells,FALSE);
         if(number != -1) {
            if (GET_LEVEL(ch) < spell_info[number].min_level_cleric) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not know of this spell...\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->specials.spells_to_learn <= 0) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not seem to be able to practice now.\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->skills[number].learned >= 95) {
               send_to_char("You are already learned in this area.\n\r", ch);
               return(TRUE);
            }
            send_to_char("You Practice for a while...\n\r", ch);
            ch->specials.spells_to_learn--;  

            percent = ch->skills[number].learned+MAXV(25,int_app[GET_INT(ch)].learn);
            ch->skills[number].learned = MINV(95, percent);  

            if (ch->skills[number].learned >= 95) {
               send_to_char("You are now learned in this area.\n\r", ch);
            }
            return(TRUE);
         }
      } break;

      case CLASS_WARRIOR: {
         temp_offset = PR_WAR_OS;
         temp_skills = (char**)w_skills;
         if (GET_LEVEL(ch) >= IMO_LEV) {
         if (!arg || !*arg) {
            send_to_char("You can practise any of these spells:\n\r", ch);
            for(i=0; *spells[i] != '\n'; i++)
               if ( (spell_info[i+1].spell_pointer) 
		&&(
                  (spell_info[i+1].min_level_cleric <= GET_LEVEL(ch)/2)
                  ||(spell_info[i+1].min_level_magic <= GET_LEVEL(ch)/2)
	       ))
					if (arg || (!arg && ch->skills[i+1].learned)){
						send_to_char((char*)spells[i], ch);
                  send_to_char(how_good(ch->skills[i+1].learned), ch);
                  send_to_char("\n\r", ch);
               }
         } else
            number = old_search_block(arg,0,strlen(arg),spells,FALSE);
         if(number != -1) {
            if (GET_LEVEL(ch) < spell_info[number].min_level_cleric) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not know of this spell...\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->specials.spells_to_learn <= 0) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not seem to be able to practice now.\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->skills[number].learned >= 95) {
               send_to_char("You are already learned in this area.\n\r", ch);
               return(TRUE);
            }
            send_to_char("You Practice for a while...\n\r", ch);
            ch->specials.spells_to_learn--;  

            percent = ch->skills[number].learned+MAXV(25,int_app[GET_INT(ch)].learn);
            ch->skills[number].learned = MINV(95, percent);  

            if (ch->skills[number].learned >= 95) {
               send_to_char("You are now learned in this area.\n\r", ch);
            }
            return(TRUE);
         }
      }
      } break;
      case CLASS_BARD: {
         temp_offset = PR_BARD_OS;
         temp_skills = (char**)b_skills;
         if (!arg || !*arg) {
            send_to_char("You can practise any of these spells:\n\r", ch);
            for(i=0; *spells[i] != '\n'; i++)
               if (spell_info[i+1].spell_pointer &&
						(spell_info[i+1].min_level_bard <= GET_LEVEL(ch)))
					if (arg || (!arg && ch->skills[i+1].learned)){
                  send_to_char((char*)spells[i], ch);
                  send_to_char(how_good(ch->skills[i+1].learned), ch);
                  send_to_char("\n\r", ch);
               }
         } else
            number = old_search_block(arg,0,strlen(arg),spells,FALSE);
         if(number != -1) {
            if (GET_LEVEL(ch) < spell_info[number].min_level_bard) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not know of this spell...\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->specials.spells_to_learn <= 0) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not seem to be able to practice now.\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->skills[number].learned >= 95) {
               send_to_char("You are already learned in this area.\n\r", ch);
               return(TRUE);
            }
            send_to_char("You Practice for a while...\n\r", ch);
            ch->specials.spells_to_learn--;  

            percent = ch->skills[number].learned+MAXV(25,int_app[GET_INT(ch)].learn);
            ch->skills[number].learned = MINV(95, percent);  

            if (ch->skills[number].learned >= 95) {
               send_to_char("You are now learned in this area.\n\r", ch);
            }
            return(TRUE);
         }
      } break;
      case CLASS_KAI: {
         temp_offset = PR_PAL_OS;
         temp_skills = (char**)p_skills;
         if (!arg || !*arg) {
            send_to_char("You can practise any of these spells:\n\r", ch);
            for(i=0; *spells[i] != '\n'; i++)
               if (spell_info[i+1].spell_pointer &&
						(spell_info[i+1].min_level_kai <= GET_LEVEL(ch)))
					if (arg || (!arg && ch->skills[i+1].learned)){
                  send_to_char((char*)spells[i], ch);
                  send_to_char(how_good(ch->skills[i+1].learned), ch);
                  send_to_char("\n\r", ch);
               }
         } else
            number = old_search_block(arg,0,strlen(arg),spells,FALSE);
         if(number != -1) {
            if (GET_LEVEL(ch) < spell_info[number].min_level_kai) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not know of this spell...\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->specials.spells_to_learn <= 0) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not seem to be able to practice now.\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->skills[number].learned >= 95) {
               send_to_char("You are already learned in this area.\n\r", ch);
               return(TRUE);
            }
            send_to_char("You Practice for a while...\n\r", ch);
            ch->specials.spells_to_learn--;  

            percent = ch->skills[number].learned+MAXV(25,int_app[GET_INT(ch)].learn);
            ch->skills[number].learned = MINV(95, percent);  

            if (ch->skills[number].learned >= 95) {
               send_to_char("You are now learned in this area.\n\r", ch);
            }
            return(TRUE);
         }
      } break;
      case CLASS_DRAKKHAR: {
         temp_offset = PR_APAL_OS;
         temp_skills = (char**)ap_skills;
         if (!arg || !*arg) {
            send_to_char("You can practise any of these spells:\n\r", ch);
            for(i=0; *spells[i] != '\n'; i++)
               if (spell_info[i+1].spell_pointer &&
						(spell_info[i+1].min_level_drakkhar <= GET_LEVEL(ch)))
					if (arg || (!arg && ch->skills[i+1].learned)){
                  send_to_char((char*)spells[i], ch);
                  send_to_char(how_good(ch->skills[i+1].learned), ch);
                  send_to_char("\n\r", ch);
               }
         } else
            number = old_search_block(arg,0,strlen(arg),spells,FALSE);
         if(number != -1) {
            if (GET_LEVEL(ch) < spell_info[number].min_level_drakkhar) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not know of this spell...\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->specials.spells_to_learn <= 0) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not seem to be able to practice now.\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->skills[number].learned >= 95) {
               send_to_char("You are already learned in this area.\n\r", ch);
               return(TRUE);
            }
            send_to_char("You Practice for a while...\n\r", ch);
            ch->specials.spells_to_learn--;  

            percent = ch->skills[number].learned+MAXV(25,int_app[GET_INT(ch)].learn);
            ch->skills[number].learned = MINV(95, percent);  

            if (ch->skills[number].learned >= 95) {
               send_to_char("You are now learned in this area.\n\r", ch);
            }
            return(TRUE);
         }
      } break;
      case CLASS_EKNIGHT: {
         temp_offset = PR_EKNI_OS;
         temp_skills = (char**)ek_skills;
         if (!arg || !*arg) {
            send_to_char("You can practise any of these spells:\n\r", ch);
            for(i=0; *spells[i] != '\n'; i++)
               if (spell_info[i+1].spell_pointer &&
						(spell_info[i+1].min_level_eknight <= GET_LEVEL(ch)))
					if (arg || (!arg && ch->skills[i+1].learned)){
                  send_to_char((char*)spells[i], ch);
                  send_to_char(how_good(ch->skills[i+1].learned), ch);
                  send_to_char("\n\r", ch);
               }
         } else
            number = old_search_block(arg,0,strlen(arg),spells,FALSE);
         if(number != -1) {
            if (GET_LEVEL(ch) < spell_info[number].min_level_eknight) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not know of this spell...\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->specials.spells_to_learn <= 0) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not seem to be able to practice now.\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->skills[number].learned >= 95) {
               send_to_char("You are already learned in this area.\n\r", ch);
               return(TRUE);
            }
            send_to_char("You Practice for a while...\n\r", ch);
            ch->specials.spells_to_learn--;  

            percent = ch->skills[number].learned+MAXV(25,int_app[GET_INT(ch)].learn);
            ch->skills[number].learned = MINV(95, percent);  

            if (ch->skills[number].learned >= 95) {
               send_to_char("You are now learned in this area.\n\r", ch);
            }
            return(TRUE);
         }
      } break;
      case CLASS_DKNIGHT: {
         temp_offset = PR_DKNI_OS;
         temp_skills = (char**)dk_skills;
         if (!arg || !*arg) {
            send_to_char("You can practise any of these spells:\n\r", ch);
            for(i=0; *spells[i] != '\n'; i++)
               if (spell_info[i+1].spell_pointer &&
						(spell_info[i+1].min_level_dknight <= GET_LEVEL(ch)))
					if (arg || (!arg && ch->skills[i+1].learned)){
                  send_to_char((char*)spells[i], ch);
                  send_to_char(how_good(ch->skills[i+1].learned), ch);
                  send_to_char("\n\r", ch);
               }
         } else
            number = old_search_block(arg,0,strlen(arg),spells,FALSE);
         if(number != -1) {
            if (GET_LEVEL(ch) < spell_info[number].min_level_dknight) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not know of this spell...\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->specials.spells_to_learn <= 0) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not seem to be able to practice now.\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->skills[number].learned >= 95) {
               send_to_char("You are already learned in this area.\n\r", ch);
               return(TRUE);
            }
            send_to_char("You Practice for a while...\n\r", ch);
            ch->specials.spells_to_learn--;  

            percent = ch->skills[number].learned+MAXV(25,int_app[GET_INT(ch)].learn);
            ch->skills[number].learned = MINV(95, percent);  

            if (ch->skills[number].learned >= 95) {
               send_to_char("You are now learned in this area.\n\r", ch);
            }
            return(TRUE);
         }
      } break;
      case CLASS_DRAGONW: {
         temp_offset = PR_DRAG_OS;
         temp_skills = (char**)d_skills;
         if (GET_LEVEL(ch) >= IMO_LEV) {
         if (!arg || !*arg) {
            send_to_char("You can practise any of these spells:\n\r", ch);
            for(i=0; *spells[i] != '\n'; i++)
               if ( (spell_info[i+1].spell_pointer) 
		&&(
                  (spell_info[i+1].min_level_cleric <= GET_LEVEL(ch)/2)
                  ||(spell_info[i+1].min_level_magic <= GET_LEVEL(ch)/2)
	       ))
					if (arg || (!arg && ch->skills[i+1].learned)){
                  send_to_char((char*)spells[i], ch);
                  send_to_char(how_good(ch->skills[i+1].learned), ch);
                  send_to_char("\n\r", ch);
               }
         } else
            number = old_search_block(arg,0,strlen(arg),spells,FALSE);
         if(number != -1) {
            if (GET_LEVEL(ch) < spell_info[number].min_level_cleric) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not know of this spell...\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->specials.spells_to_learn <= 0) {
	       ansi(CLR_ERROR, ch);
               send_to_char("You do not seem to be able to practice now.\n\r", ch);
	       ansi(END, ch);
               return(TRUE);
            }
            if (ch->skills[number].learned >= 95) {
               send_to_char("You are already learned in this area.\n\r", ch);
               return(TRUE);
            }
            send_to_char("You Practice for a while...\n\r", ch);
            ch->specials.spells_to_learn--;  

            percent = ch->skills[number].learned+MAXV(25,int_app[GET_INT(ch)].learn);
            ch->skills[number].learned = MINV(95, percent);  

            if (ch->skills[number].learned >= 95) {
               send_to_char("You are now learned in this area.\n\r", ch);
            }
            return(TRUE);
         }
      }
      } break;
   }     /* switch */

   if (!arg || !*arg) {
      send_to_char("\n\rYou can practise any of these skills:\n\r", ch);
      for(i=0; **(temp_skills+i) != '\n';i++) {
			  if (GET_LEVEL(ch) >= skill_num[temp_offset+i][1])
			  if ((!arg && ch->skills[skill_num[temp_offset+i][0]].learned)
				 ||(arg)){
				skill_max = skill_num[temp_offset+i][4];
				if (GET_LEVEL(ch) >= skill_num[temp_offset+i][2])
					skill_max = skill_num[temp_offset+i][5];
				if (GET_LEVEL(ch) >= skill_num[temp_offset+i][3])
					skill_max = skill_num[temp_offset+i][6];
				send_to_char(temp_skills[i], ch);
				sprintf(buf,"(%d) / %d \n\r",
					ch->skills[skill_num[temp_offset+i][0]].learned,skill_max);
				send_to_char(buf, ch);
			  }
		}
		send_to_char("\n\r", ch);
		sprintf(buf,"You have got %d practice sessions left.\n\r", ch->specials.spells_to_learn);
		send_to_char(buf, ch);
		return(TRUE);
	}
	number = search_block(arg, (const char **)temp_skills, FALSE);
	if(number == -1) {
	        ansi(CLR_ERROR, ch);
		send_to_char("You do not have ability to practise this skill!\n\r", ch);
		ansi(END, ch);
		return(TRUE);
	}
	if (ch->specials.spells_to_learn <= 0) {
	        ansi(CLR_ERROR, ch);
		send_to_char("You do not seem to be able to practice now.\n\r", ch);
		ansi(END, ch);
		return(TRUE);
	}
	skill_max = 0;
	if (GET_LEVEL(ch) >= skill_num[temp_offset+number][1]) {
		skill_max = skill_num[temp_offset+number][4];
		skill_div = skill_num[temp_offset+number][7];
	}
	if (GET_LEVEL(ch) >= skill_num[temp_offset+number][2]) {
		skill_max = skill_num[temp_offset+number][5];
		skill_div = skill_num[temp_offset+number][8];
	}
	if (GET_LEVEL(ch) >= skill_num[temp_offset+number][3]) {
		skill_max = skill_num[temp_offset+number][6];
		skill_div = skill_num[temp_offset+number][9];
	}
	if (ch->skills[skill_num[temp_offset+number][0]].learned >= skill_max) {
		send_to_char("You are already learned in this area.\n\r", ch);
		return(TRUE);
	}
	send_to_char("You Practice for a while...\n\r", ch);
	ch->specials.spells_to_learn--;

   percent = ch->skills[skill_num[temp_offset+number][0]].learned +
             MAXV(2, ((int_app[GET_INT(ch)].learn) / skill_div) );
   ch->skills[skill_num[temp_offset+number][0]].learned = MINV(skill_max, percent);

   if (ch->skills[skill_num[temp_offset+number][0]].learned >= skill_max) {
      send_to_char("You are now learned in this area.\n\r", ch);
   }
   return(TRUE);
}   /* guild */



int dump(struct char_data *ch, int cmd, char *arg) 
{
   struct obj_data *k;
   char buf[100];
   struct char_data *tmp_char;
   int value=0;

   void do_drop(struct char_data *ch, char *argument, int cmd);
   char *fname(char *namelist);

   for(k = world[ch->in_room].contents; k ; k = world[ch->in_room].contents)
   {
      sprintf(buf, "The %s vanish in a puff of smoke.\n\r" ,fname(k->name));
      for(tmp_char = world[ch->in_room].people; tmp_char;
         tmp_char = tmp_char->next_in_room)
         if (CAN_SEE_OBJ(tmp_char, k)) {
	    ansi(CLR_ACTION, ch);
            send_to_char(buf,tmp_char);
	    ansi(END, ch);
	  }
      extract_obj(k);
   }

   if(cmd!=60) return(FALSE);

   do_drop(ch, arg, cmd);
   value = 0;

   for(k = world[ch->in_room].contents; k ; k = world[ch->in_room].contents)
   {
      sprintf(buf, "The %s vanish in a puff of smoke.\n\r",fname(k->name));
      for(tmp_char = world[ch->in_room].people; tmp_char;
         tmp_char = tmp_char->next_in_room)
         if (CAN_SEE_OBJ(tmp_char, k)) {
	    ansi(CLR_ACTION, ch);
            send_to_char(buf,tmp_char);
	    ansi(END, ch);
	  }
         value += MAXV(1, MINV(50, k->obj_flags.cost/10));

      extract_obj(k);
   }

   if (value) 
   {
     ansi_act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
     ansi_act("$n has been awarded for being a good citizen.", TRUE, ch, 0,0, TO_ROOM,CLR_ACTION);

      if (GET_LEVEL(ch) < 3)
         gain_exp(ch, value);
      else
         GET_GOLD(ch) += value;
   }
   return(TRUE);
}

int mayor(struct char_data *ch, int cmd, char *arg)
{
  static char go_path[] =
    "GW3f3333333ab00300000111e014dc5303i123j1221112h0332gR.";

  static char return_path[] =
    "HT0333322221001001011S.";

/*   const struct social_type open_path[] = {
    {"G",0}
  };
  static void *thingy = 0;
  static int cur_line = 0;
  for (i=0; i < 1; i++)  {
    if (*(open_path[cur_line].cmd) == '!') {
      i++;
      exec_social(ch, (open_path[cur_line].cmd)+1,
        open_path[cur_line].next_line, &cur_line, &thingy);
  } else {
      exec_social(ch, open_path[cur_line].cmd,
        open_path[cur_line].next_line, &cur_line, &thingy);
  }  */

  static char *path;
  static int index;
  static bool move = FALSE;
  struct char_data *tmp_ch, *MAYOR;
  char *tmp_arg, argm[100], buf[100];

  void do_move(struct char_data *ch, char *argument, int cmd);
  void do_open(struct char_data *ch, char *argument, int cmd);
  void do_lock(struct char_data *ch, char *argument, int cmd);
  void do_unlock(struct char_data *ch, char *argument, int cmd);
  void do_close(struct char_data *ch, char *argument, int cmd);
  void do_gossip(struct char_data *ch, char *argument, int cmd); 

  MAYOR = 0;
  for (tmp_ch = world[ch->in_room].people; (!MAYOR) && (tmp_ch) ; 
                tmp_ch = tmp_ch->next_in_room)
    if (IS_MOB(tmp_ch))
      if (mob_index[tmp_ch->nr].func == mayor)
	MAYOR = tmp_ch;

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = go_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = return_path;
      index = 0;
    }
  }

  if ((cmd == 25) || (cmd==70) || (cmd==159) || (cmd==157) || (cmd==217))   
    /* Kill/Hit/Kick/Bash */
    {
      tmp_arg = one_argument(arg, argm);
      if (cmd==217 && *argm) { 
	/* Now check whether target is mayor or gas breath */
	if (!str_cmp("gas",argm)) {
	  one_argument(tmp_arg, argm);
	  if (!*argm) {
	    sprintf(buf,"%s Hey! Don't try that in front of me!",GET_NAME(ch));
	    ansi(CLR_ERROR, ch);
	    do_whisper(MAYOR,buf,19);
	    ansi(END, ch);
	    return(TRUE);
	  }
	}
	one_argument(tmp_arg, argm);
      }
      if (MAYOR == get_char_room(argm,ch->in_room)) { 
	sprintf(buf,"%s Don't ever try that again!",GET_NAME(ch));
	ansi(CLR_ERROR, ch);
	do_whisper(MAYOR,buf,19);
	ansi(END, ch);
	return(TRUE);
      } 
    } 
  
  if (cmd || !move || (GET_POS(ch) < POSITION_SLEEPING) ||
      (GET_POS(ch) == POSITION_FIGHTING))
    return FALSE;

  switch (path[index]) {
    case '0' :
    case '1' :
    case '2' :
    case '3' :
    case '4' :
    case '5' :  
      do_move(ch,"",path[index]-'0'+1);
      break;

   case 'G' :
   /* Relocate mayor's location if not in office */
   if (ch->in_room != real_room(3138)) {
     ansi_act("$n says, 'I have wasted enough time here.'",FALSE,ch,0,0,TO_ROOM,CLR_ACTION); 
     ansi_act("$n leaves in a hurry.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);      
     char_from_room(ch);
     char_to_room(ch, real_room(3138));
   }
   break;

   case 'H' :
   /* Relocate mayor's location if not in Grubby Inn */
   if (ch->in_room != real_room(3048)) {
     ansi_act("$n says, 'Oh God! Look at the time!'",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);      
     ansi_act("$n leaves in a hurry.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);      
     char_from_room(ch);
     char_to_room(ch, real_room(3048));
   }
   break;
      
   case 'W' :
   GET_POS(ch) = POSITION_STANDING;
   ansi_act("$n wakes up and gets ready for a new day.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   do_gossip(ch,"Good morning mudders!", 232);   
   break;

   case 'S' :
   do_gossip(ch,"Good night mudders!", 232);       
   ansi_act("$n goes to sleep.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   GET_POS(ch) = POSITION_SLEEPING;
   break;

   case 'R' :
   GET_POS(ch) = POSITION_SITTING;
   ansi_act("$n sits down on a barstool.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   break;

   case 'T' :
   GET_POS(ch) = POSITION_STANDING;
   ansi_act("$n gets up waves goodbye to you.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   break;

   case 'a' :
   ansi_act("$n growls, 'Aargh..., the light hurts my eyes!'",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   ansi_act("$n grumbles about the weather.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   break;

   case 'b' :
   ansi_act("$n gets a pair of dark shades from his bag.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   ansi_act("$n wears a pair of dark shades over his eyes.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   break;

   case 'c' :
   ansi_act("$n pats Sly on his back.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   break;

   case 'd' :
   ansi_act("$n smiles at the receptionist.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   break;

   case 'e' :
   ansi_act("$n drinks from the fountain.", FALSE, ch, 0,0,TO_ROOM,CLR_ACTION);
   break;

   case 'f' :
   ansi_act("$n chuckles as he looks at the sleepy secretary.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);;
   break;

   case 'g' :
   ansi_act("$n says, I'll have a beer!",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   ansi_act("$n buys a beer.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   break;

   case 'h' :
   ansi_act("$n greets you, 'How do you do?'",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   break;

   case 'i' :
   ansi_act("$n studies the board.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   break;

   case 'j' :
   ansi_act("$n spins the small globe.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   break;

   case 'O' :
   do_unlock(ch, "gate", 0);
   ansi_act("$n says, I do this for a living, can you believe it?",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   do_open(ch, "gate", 0);
   break;

   case 'C' :
   do_close(ch, "gate", 0);
   ansi_act("$n says, I do this for a living, can you believe it?",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
   do_lock(ch, "gate", 0);
   break;

   case '.' :
   move = FALSE;
   break;

  }

  index++;
  return FALSE;
}


int biff(struct char_data *ch, int cmd, char *arg)
{
  static char ready_path[] = "HG.";
  static char go_path[] = "HSa1be2cHRd.";
  static char sleep_path[] = "HL.";
  static char wake_path[] = "HW.";

  static char *path;
  static int index;
  static bool move = FALSE;

  struct obj_data *obj;
  struct char_data *tmp_ch, *BIFF;
  struct follow_type *fol;
  int found = FALSE;
  char *tmp_arg, buf[100], argm[100];

  void do_move(struct char_data *ch, char *argument, int cmd);
  void do_gossip(struct char_data *ch, char *argument, int cmd); 
  void do_whisper(struct char_data *ch, char *argument, int cmd); 

  BIFF = 0;
  for (tmp_ch = world[ch->in_room].people; (!BIFF) && (tmp_ch) ; 
                tmp_ch = tmp_ch->next_in_room)
    if (IS_MOB(tmp_ch))
      if (mob_index[tmp_ch->nr].func == biff)
	BIFF = tmp_ch;

if (!move) {
    if (time_info.hours == 9) {
      move = TRUE;
      path = ready_path;
      index = 0;
    } else if (time_info.hours == 10) {
      move = TRUE;
      path = go_path;
      index = 0;
    } else if (time_info.hours == 22) {
      move = TRUE;
      path = sleep_path;
      index = 0;
    } else if (time_info.hours == 8) {
      move = TRUE;
      path = wake_path;
      index = 0;
    }
  }

  if ((cmd == 25) || (cmd==70) || (cmd==159) || (cmd==157) || (cmd==217))   
    /* Kill/Hit/Kick/Bash */
    {
      tmp_arg = one_argument(arg, argm);
      if (cmd==217 && *argm) { 
	/* Now check whether target is Biff or gas breath */
	if (!str_cmp("gas",argm)) {
	  one_argument(tmp_arg, argm);
	  if (!*argm) {
	    sprintf(buf,"%s Hey! Don't try that in front of me!",GET_NAME(ch));
	    ansi(CLR_ERROR, ch);
	    do_whisper(BIFF,buf,19);
	    ansi(END, ch);
	    return(TRUE);
	  }
	}
	one_argument(tmp_arg, argm);
      }
      if (BIFF == get_char_room(argm,ch->in_room)) { 
	sprintf(buf,"%s Don't ever try that again!",GET_NAME(ch));
	ansi(CLR_ERROR, ch);
	do_whisper(BIFF,buf,19);
	ansi(END, ch);
	return(TRUE);
      } 
    } 

  if (cmd || !move || (GET_POS(ch) < POSITION_SLEEPING) ||
      (GET_POS(ch) == POSITION_FIGHTING))
    return FALSE;

  if ((cmd == 25) || (cmd==70) || (cmd==159) || (cmd==157))   
    /* Kill/Hit/Kick/Bash */
    {
      one_argument(arg, argm);
      if (BIFF == get_char_room(argm,ch->in_room)) {
	sprintf(buf,"%s Don't ever try that again!",GET_NAME(ch));
	ansi(CLR_ERROR, ch);
	do_whisper(BIFF,buf,19);
	ansi(END, ch);
	return(TRUE);
      }
    } 

  if (move) { 
    switch (path[index]) {
    case '0' :
    case '1' :
    case '2' :
    case '3' :
      do_move(ch,"",path[index]-'0'+1);
      break;
      
    case 'H' :
      /* Relocate Biff's location if not in Grunting Boar */
      if (ch->in_room != real_room(3007)) {
	do_gossip(ch, "Oh boy!  I am lost again!", 232);
	ansi_act("$n summons a portal.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);      
	char_from_room(ch);
	char_to_room(ch, real_room(3007));
      }
      break;
      
    case 'L' :
      if (GET_POS(ch) != POSITION_SLEEPING) {
	ansi_act("$n says, 'I am tired.'",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
	ansi_act("$n goes to sleep.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
	GET_POS(ch) = POSITION_SLEEPING;
      }
      break;
      
    case 'W' :
      if (GET_POS(ch) != POSITION_SITTING) {
	GET_POS(ch) = POSITION_SITTING;
	ansi_act("$n wakes up and sits up.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
	ansi_act("$n stretches his arms.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
      }
      break;
      
    case 'G' :
      do_gossip(ch,"All who want to kill dragon follow me!", 232);       
      break;
      
    case 'S' :
    GET_POS(ch) = POSITION_STANDING;
      ansi_act("$n stands up.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
      /* need to group followers */
      for (tmp_ch = world[ch->in_room].people; 
           tmp_ch;
           tmp_ch = tmp_ch->next_in_room)
        if (!IS_NPC(tmp_ch) && (tmp_ch->master == ch)) {
	  do_group(ch, GET_NAME(tmp_ch), 0);
	}
    break;
      
    case 'a' :
      ansi_act("$n says, 'Ready? Let's go.'",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
      ansi_act("$n opens the hidden portal and steps through it.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);   
      char_from_room(ch);
      char_to_room(ch, real_room(6136));
      for(fol = ch->followers; fol; fol = fol->next)
	if (!IS_NPC(fol->follower) && IS_AFFECTED(fol->follower, AFF_GROUP)) {
	  char_from_room(fol->follower);
	  char_to_room(fol->follower, real_room(6136));
	  ansi_act("You feel excited while Biff leads you into the portal.",FALSE,fol->follower,0,0,TO_CHAR,CLR_ACTION);
	  do_look(fol->follower, "\0", 15);
	  if (!affected_by_spell(fol->follower, SPELL_INVISIBLE) 
	      && !affected_by_spell(fol->follower, SPELL_IMPROVED_INVIS)) {
	    ansi_act("Biff makes sure that you are invisible.",
		     FALSE,fol->follower,0,0,TO_CHAR,CLR_ACTION);
	    cast_improved_invis(40,BIFF,"", SPELL_TYPE_SPELL, fol->follower,0);
	  }
	}
      break;
      
    case 'b' :
      do_gossip(ch, "Die, green dragon!", 232);       
      GET_LEVEL(ch) = 10;   /* Lower the level of Biff to level 10 */
                            /* So that group members will have some exp */
      do_move(ch,"",CMD_NORTH);
      for(obj = world[ch->in_room].contents; obj ; 
	  obj = world[ch->in_room].contents) {
	extract_obj(obj);
      }
      ansi_act("$n cleans up the garbage on the floor.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
      /* auto-attack Green Dragon if present */
      for (tmp_ch = world[ch->in_room].people; 
	   tmp_ch && !found; 
	   tmp_ch = tmp_ch->next_in_room)
	if (tmp_ch->nr == real_mobile(6112)) {
	  found = TRUE;
	  hit(ch, tmp_ch, TYPE_UNDEFINED);
	}
      break;

    case 'c' :
      do_gossip(ch, "That feels good!", 232);
      ansi_act("$n says, 'We'll do it again tomorrow.'",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
      ansi_act("$n says, 'Let's go back.'",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
      ansi_act("$n opens a portal and leads the group through it.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
      for(fol = ch->followers; fol; fol = fol->next)
	if (!IS_NPC(fol->follower) && IS_AFFECTED(fol->follower, AFF_GROUP)) {
	  char_from_room(fol->follower);
	  char_to_room(fol->follower, real_room(3007));
	  do_look(fol->follower, "\0", 15);
	}
      char_from_room(ch);
      char_to_room(ch, real_room(3007));
      break;
      
    case 'd' :
      ansi_act("$n says, 'I miss those days of slaying white dragons.'",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
      ansi_act("$n sighs.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
      break;
      
    case 'e' :
      GET_LEVEL(ch) = 50;  /* Restore the level of Biff */
      break;
      
    case 'R' :
      GET_POS(ch) = POSITION_RESTING;
      ansi_act("$n rests and starts to tell stories.",FALSE,ch,0,0,TO_ROOM,CLR_ACTION);
      break;
      
    case '.' :
      move = FALSE;
      break;
    }
    index++;
    return FALSE;
  }
}


/*********************************************************************
 *  General special procedures for mobiles                           *
 *********************************************************************/

/* SOCIAL GENERAL PROCEDURES

If first letter of the command is '!' this will mean that the following
command will be executed immediately.

"G",n      : Sets next line to n
"g",n      : Sets next line relative to n, fx. line+=n
"m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
"w",n      : Wake up and set standing (if possible)
"c<txt>",n : Look for a person named <txt> in the room
"o<txt>",n : Look for an object named <txt> in the room
"r<int>",n : Test if the npc in room number <int>?
"s",n      : Go to sleep, return false if can't go sleep
"e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
             contents of the **thing
"E<txt>",n : Send <txt> to person pointed to by thing
"B<txt>",n : Send <txt> to room, except to thing
"?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
             Will as usual advance one line upon sucess, and change
             relative n lines upon failure.
"O<txt>",n : Open <txt> if in sight.
"C<txt>",n : Close <txt> if in sight.
"L<txt>",n : Lock <txt> if in sight.
"U<txt>",n : Unlock <txt> if in sight.    */

/* Execute a social command.                                        */
void exec_social(struct char_data *npc, char *cmd, int next_line,
                 int *cur_line, void **thing)
{
  bool ok;

  void do_move(struct char_data *ch, char *argument, int cmd);
  void do_open(struct char_data *ch, char *argument, int cmd);
  void do_lock(struct char_data *ch, char *argument, int cmd);
  void do_unlock(struct char_data *ch, char *argument, int cmd);
  void do_close(struct char_data *ch, char *argument, int cmd);

  if (GET_POS(npc) == POSITION_FIGHTING)
    return;

  ok = TRUE;

  switch (*cmd) {

    case 'G' :
      *cur_line = next_line;
      return;

    case 'g' :
      *cur_line += next_line;
      return;

    case 'e' :
      act(cmd+1, FALSE, npc, *thing, *thing, TO_ROOM);
      break;

    case 'E' :
      act(cmd+1, FALSE, npc, 0, *thing, TO_VICT);
      break;

    case 'B' :
      act(cmd+1, FALSE, npc, 0, *thing, TO_NOTVICT);
      break;

    case 'm' :
      do_move(npc, "", *(cmd+1)-'0'+1);
      break;

    case 'w' :
      if (GET_POS(npc) != POSITION_SLEEPING)
        ok = FALSE;
      else
        GET_POS(npc) = POSITION_STANDING;
      break;

    case 's' :
      if (GET_POS(npc) <= POSITION_SLEEPING)
        ok = FALSE;
      else
        GET_POS(npc) = POSITION_SLEEPING;
      break;

    case 'c' :  /* Find char in room */
      *thing = get_char_room_vis(npc, cmd+1);
      ok = (*thing != 0);
      break;

    case 'o' : /* Find object in room */
      *thing = get_obj_in_list_vis(npc, cmd+1, world[npc->in_room].contents);
      ok = (*thing != 0);
      break;

    case 'r' : /* Test if in a certain room */
      ok = (npc->in_room == atoi(cmd+1));
      break;

    case 'O' : /* Open something */
      do_open(npc, cmd+1, 0);
      break;

    case 'C' : /* Close something */
      do_close(npc, cmd+1, 0);
      break;

    case 'L' : /* Lock something  */
      do_lock(npc, cmd+1, 0);
      break;

    case 'U' : /* UnLock something  */
      do_unlock(npc, cmd+1, 0);
      break;

    case '?' : /* Test a random number */
      if (atoi(cmd+1) <= number(1,100))
        ok = FALSE;
      break;

    default:
      break;
  }  /* End Switch */

  if (ok)
    (*cur_line)++;
  else
    (*cur_line) += next_line;
}



void npc_steal(struct char_data *ch,struct char_data *victim)
{
   int gold;

   if(IS_NPC(victim)) return;
   if(GET_LEVEL(victim)>20) return;

   if (AWAKE(victim) && (number(0,GET_LEVEL(ch)) == 0)) {
      ansi_act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT,CLR_ACTION);
      ansi_act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT,CLR_ACTION);
   } else {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
      if (gold > 0) {
         GET_GOLD(ch) += gold;
         GET_GOLD(victim) -= gold;
      }
   }
}


int snake(struct char_data *ch, int cmd, char *arg)
{
   if(cmd) return FALSE;

   if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
   
   if(ch->specials.fighting && 
      (ch->specials.fighting->in_room == ch->in_room) &&
      (number(0,32-GET_LEVEL(ch))==0))
      {
	 ansi_act("$n bites $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT,CLR_DAM);
         ansi_act("$n bites you!", 1, ch, 0, ch->specials.fighting, TO_VICT,CLR_DAM);
         cast_poison( GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,
             ch->specials.fighting, 0);
         return TRUE;
      }
   return FALSE;
}

int thief(struct char_data *ch, int cmd, char *arg)
{
   struct char_data *cons;

   if(cmd) return FALSE;

   if(GET_POS(ch)!=POSITION_STANDING)return FALSE;

   for(cons = world[ch->in_room].people; cons; cons = cons->next_in_room )
      if((!IS_NPC(cons)) && (GET_LEVEL(cons)<IMO_LEV) && (number(1,5)==1))
         npc_steal(ch,cons); 

   return TRUE;
}

int magic_user(struct char_data *ch, int cmd, char *arg)
{
   struct char_data *vict;

   if(cmd) return FALSE;
   if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
   if(!ch->specials.fighting) return FALSE;

   /* Find a dude to to evil things upon ! */
   for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room )
      if (vict->specials.fighting==ch )
         break;
   if (!vict)
      return FALSE;

   if( (vict!=ch->specials.fighting) && (GET_LEVEL(ch)>13) && (number(0,9)==0) )
   {
      ansi_act("$n utters the words, 'dilan oso.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
      cast_sleep(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
   }
   if( (GET_LEVEL(ch)>12) && (number(0,6)==0) )
   {
      ansi_act("$n utters the words, 'gharia miwi.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
      cast_curse(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
   }
   if( (GET_LEVEL(ch)>7) && (number(0,5)==0) )
   {
      ansi_act("$n utters the words, 'koholian dia.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
      cast_blindness(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
   }
   if( (GET_LEVEL(ch)>12) && (number(0,8)==0) && IS_EVIL(ch))
   {
      ansi_act("$n utters the words, 'ib er dranker.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
      cast_energy_drain(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
   }
   switch (GET_LEVEL(ch)) {
      case 1: case 2: case 3: case 4: case 5: 
         ansi_act("$n utters the words, 'hahili duvini.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
         cast_magic_missile(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
         break;
      case 6: case 7: case 8: case 9: case 10: 
         ansi_act("$n utters the words, 'grynt oef.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
         cast_burning_hands(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
         break;
      case 11: case 12: case 13: case 14: case 15: 
         ansi_act("$n utters the words, 'sjulk divi.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
         cast_lightning_bolt(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
         break;
      case 16: case 17: case 18: case 19: case 20: 
         ansi_act("$n utters the words, 'nasson hof.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
         cast_colour_spray(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
         break;
      case 21: case 22: case 23: case 24: case 25: 
         ansi_act("$n utters the words, 'tuborg.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
         cast_fireball(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
         break;
      default:	 
	 ansi_act("$n utters the words, 'hlbp fuqei.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
	 cast_conflagration(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	 break;
       }
   return TRUE;
}

int cleric(struct char_data *ch, int cmd, char *arg)
{
   struct char_data *vict;
   extern struct weather_data weather_info;

   if(cmd) return FALSE;
   if(GET_HIT(ch) < hit_limit(ch)  && GET_LEVEL(ch) < 9 && (number(0,2)==0) )
   {
      ansi_act("$n utters the words, 'ifrim curae.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
      cast_cure_light(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return TRUE;
   }
   if(GET_HIT(ch) < hit_limit(ch)  && GET_LEVEL(ch) > 9 
   && GET_LEVEL(ch) <= 15 && (number(0,2)==0) )
   {
      ansi_act("$n utters the words, 'ifrim cuzak.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
      cast_cure_critic(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return TRUE;
   }
   if(GET_HIT(ch) < hit_limit(ch)  && GET_LEVEL(ch) > 15 && (number(0,2)==0) )
   {
      ansi_act("$n utters the words, 'ifrim maxim.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
      cast_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return TRUE;
   }


   if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
   if(!ch->specials.fighting) return FALSE;

   /* Find a dude to to evil things upon ! */
   for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room )
      if (vict->specials.fighting==ch )
         break;
   if (!vict)
      return FALSE;

   if( (GET_LEVEL(ch)>13) && (number(0,5)==0) )
   {
      ansi_act("$n utters the words, 'gharia miwi.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
      cast_curse(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
   }
   if( (GET_LEVEL(ch)>5) && (number(0,3)==0) )
   {
      ansi_act("$n utters the words, 'nox conea.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
      cast_blindness(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
   }
   if( (GET_LEVEL(ch)>9) && (number(0,3)==0) )
   {
      ansi_act("$n utters the words, 'nox ven.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
      cast_poison(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
   }
   if( (GET_LEVEL(ch)>11) && (number(0,1)==0) &&
   (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) )
   {
      ansi_act("$n utters the words, 'hasta la vista.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
      cast_call_lightning(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
   }
   switch (GET_LEVEL(ch)) {
      case 1: case 2: case 3: case 4: case 5: case 6:
         ansi_act("$n utters the words, 'grynt curae.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
         cast_cause_light(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
         break;
      case 7: case 8: case 9:
         ansi_act("$n utters the words, 'duvini oef.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
         cast_earthquake(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
         break;
      case 10: case 11: case 12: case 13: case 14:
         ansi_act("$n utters the words, 'grynt cuzak.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
         cast_cause_critic(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
         break;
      default:
         ansi_act("$n utters the words, 'grynt maxim.'", 1, ch, 0, 0, TO_ROOM,CLR_ACTION);
         cast_harm(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
         break;
   }
   return TRUE;
}


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

int high_priest(struct char_data *ch, int cmd, char *arg) {
   const char *options[] = { /* text */
        "Strength    ",
        "Intelligence",
        "Wisdom      ",
        "Dexterity   ",
        "Constitution",
        "Practices   (2)",
        "Hit points  (5)",
        "Move points (5)",
        "Mana points (5)",
        "Cure Light Wounds   ",
        "Cure Critical Wounds",
        "Heal     ",
        "Heal Mana",
        "Heal Move",
        "\n"
   };
   const int cost[][3] = { /* costperlevel, base cost, min cost */
       {50000, 0, 0},
       {50000, 0, 0},
       {50000, 0, 0},
       {50000, 0, 0},
       {50000, 0, 0},
       {20000, 0, 0},
       {25000, 0, 0},
       {23000, 0, 0},
       {27000, 0, 0},
       {   25, 0, 75},
       {   40, 0, 200},
       {  200, 0,3000},
       {  200, 0,3000},
       {  200, 0,3000}
   };
   char buf[256]; /* not input length dependant */
   int number,i, c;
   struct char_data *temp_char;
   static struct char_data *priest = 0;

   /* should be pray (56 is buy) but want to put by altar */ 
   /* where pray summons the keys (at least for now) */
   if (!ch) return(FALSE);
   if (cmd != 56) return(FALSE);
   if (IS_NPC(ch)) {
      ansi(CLR_ERROR, ch);
      send_to_char("Mobs don't buy much of anything...\n\r", ch);
      ansi(END, ch);
      return(TRUE);
   }
   for (temp_char = world[ch->in_room].people;
        (temp_char) && (!priest);
        temp_char = temp_char->next)
     if (IS_MOB(temp_char))
       if (mob_index[temp_char->nr].func == high_priest)
	 priest = temp_char;

   number = -1;
   if(arg)for(;*arg==' ';arg++);
   if (!arg || !*arg) {
      send_to_char("You can buy any of the following:\n\r", ch);
      for(i=0; *options[i] != '\n'; i++) {
            send_to_char((char*)options[i], ch);
	    sprintf(buf, " for %d coins.\n\r", 
	      MAXV(cost[i][0]*GET_LEVEL(ch)+cost[i][1], cost[i][2]));
            send_to_char(buf, ch);
         }
   } else {/* -1 for upper case insensitive match */
      number = old_search_block(arg,0,strlen(arg),options,-1);
      c =  MAXV(cost[number-1][0]*GET_LEVEL(ch)+cost[number-1][1], cost[number-1][2]);
      ansi_act("$n whispers something into $N's ear.",FALSE,priest,0,ch,TO_NOTVICT,CLR_ACTION);
      if (GET_GOLD(ch) < c) {
	ansi(CLR_ERROR, ch);
         send_to_char("Come back when you can afford it.\n\r", ch);
	ansi(END, ch);
         return(TRUE);
      } else {
		  if (number <= 9 && GET_LEVEL(ch)<11) {
		         ansi(CLR_ERROR, ch);
			 send_to_char("Come back when you're a little bit wiser kid...\n\r",ch);
			 ansi(END, ch);
			 return(TRUE);
		  }
		  switch (number) {
		  case 1:
			 if (ch->abilities.str >=18+GET_LEVEL(ch)/9) {
			        ansi(CLR_ERROR, ch);
				send_to_char("That's as high as it can go... for now.\n\r", ch);
				ansi(END, ch);
			      }
			 else {
			        ansi(CLR_ACTION, ch);
				send_to_char("Done.\n\r", ch);
				ansi(END, ch);
				ch->abilities.str++;
				ch->tmpabilities.str++;
				GET_GOLD(ch) -= c;
			 }
			 break;
		  case 2:
			 if (ch->abilities.intel >=18+GET_LEVEL(ch)/9) {
			        ansi(CLR_ERROR, ch);
				send_to_char("That's as high as it can go... for now.\n\r", ch);
				ansi(END, ch);
			      }
			 else {
			   ansi(CLR_ACTION, ch);
			   send_to_char("Done.\n\r", ch);
			   ansi(END, ch);
			   ch->abilities.intel++;
			   ch->tmpabilities.intel++;
			   GET_GOLD(ch) -= c;
          }
          break;
        case 3:
          if (ch->abilities.wis >=18+GET_LEVEL(ch)/9) {
	    ansi(CLR_ERROR, ch);
            send_to_char("That's as high as it can go... for now.\n\r", ch);
	    ansi(END, ch);
	  }
          else {
	    ansi(CLR_ACTION,  ch);
            send_to_char("Done.\n\r", ch);
	    ansi(END, ch);
            ch->abilities.wis++;
            ch->tmpabilities.wis++;
            GET_GOLD(ch) -= c;
          }
          break;
        case 4:
          if (ch->abilities.dex >=18+GET_LEVEL(ch)/9) {
	    ansi(CLR_ERROR, ch);
            send_to_char("That's as high as it can go... for now.\n\r", ch);
	    ansi(END, ch);
	  }
          else {
	    ansi(CLR_ACTION, ch);
            send_to_char("Done.\n\r", ch);
	    ansi(END, ch);
            ch->abilities.dex++;
            ch->tmpabilities.dex++;
            GET_GOLD(ch) -= c;
          }
          break;
        case 5:
          if (ch->abilities.con >= 18+GET_LEVEL(ch)/9) {
	    ansi(CLR_ERROR, ch);
            send_to_char("That's as high as it can go... for now.\n\r", ch);
	    ansi(END, ch);
	  }
          else {
	    ansi(CLR_ACTION, ch);
            send_to_char("Done.\n\r", ch);
	    ansi(END, ch);
            ch->abilities.con++;
            ch->tmpabilities.con++;
            GET_GOLD(ch) -= c;
          }
          break;
        case 6:
          ch->specials.spells_to_learn+=2;  
 	  if (ch->specials.spells_to_learn < 0)
	    ch->specials.spells_to_learn = 127;
          GET_GOLD(ch) -= c;
          ansi(CLR_ACTION, ch);
          send_to_char("Done.\n\r", ch);
          ansi(END, ch);
          break;
        case 7:
          ch->points.max_hit += 5;
          GET_GOLD(ch) -= c;
          ansi(CLR_ACTION, ch);
          send_to_char("Done.\n\r", ch);
          ansi(END, ch);
          break;
        case 8:
          ch->points.max_move += 5;
          GET_GOLD(ch) -= c;
          ansi(CLR_ACTION, ch);
          send_to_char("Done.\n\r", ch);
          ansi(END, ch);
          break;
        case 9:
          ch->points.max_mana += 5;
          GET_GOLD(ch) -= c;
          send_to_char("Done.\n\r", ch);
          break;
        case 10: /* cure light */
          cast_cure_light(40, priest, "", SPELL_TYPE_SPELL, ch, 0);
          GET_GOLD(ch) -= c;
          break;
        case 11: 
          cast_cure_critic(40, priest, "", SPELL_TYPE_SPELL, ch, 0);
          GET_GOLD(ch) -= c;
          break;
        case 12:
          cast_heal(40, priest, "", SPELL_TYPE_SPELL, ch, 0);
          GET_GOLD(ch) -= c;
          break;
        case 13:
	  GET_MANA(ch) = MAXV(mana_limit(ch), GET_MANA(ch)+100);
          GET_GOLD(ch) -= c;
          ansi(CLR_ACTION, ch);
          send_to_char("Done.\n\r", ch);
          ansi(END, ch);			 
          break;
        case 14:
	  GET_MOVE(ch) = MAXV(move_limit(ch), GET_MOVE(ch)+100);
          GET_GOLD(ch) -= c;
          ansi(CLR_ACTION, ch);
          send_to_char("Done.\n\r", ch);
	  ansi(END, ch);
          break;
        default:
          ansi(CLR_ERROR, ch);
          send_to_char("Sorry kid, I'm not quite sure what you mean.\n\r", ch);
          ansi(END, ch);
          break;
        } /* switch */
     } /* else gold */
   }
   return(TRUE);
} /* end of funct. */


int guild_guard(struct char_data *ch, int cmd, char *arg)
{
   char buf[256], buf2[256];

   if (cmd>6 || cmd<1)
      return FALSE;

   strcpy(buf,  "The guard humiliates you, and block your way.\n\r");
   strcpy(buf2, "The guard humiliates $n, and blocks $s way.");

   if ((ch->in_room == real_room(3017)) && (cmd == CMD_WEST)) {
      if (GET_CLASS(ch) != CLASS_MAGIC_USER
      && GET_CLASS(ch) != CLASS_BARD
      && GET_CLASS(ch) != CLASS_EKNIGHT
      && GET_CLASS(ch) != CLASS_DKNIGHT) {
         ansi_act(buf2, FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
         ansi(CLR_ERROR, ch);
         send_to_char(buf, ch);
	 ansi(END, ch);
         return TRUE;
      }
   } else if ((ch->in_room == real_room(3004)) && (cmd == CMD_WEST)) {
      if (GET_CLASS(ch) != CLASS_CLERIC
      && GET_CLASS(ch) != CLASS_KAI
      && GET_CLASS(ch) != CLASS_DRAKKHAR) {
         ansi_act(buf2, FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
	 ansi(CLR_ERROR, ch);
         send_to_char(buf, ch);
	 ansi(END, ch);
         return TRUE;
      }
   } else if ((ch->in_room == real_room(3027)) && (cmd == CMD_EAST)) {
      if (GET_CLASS(ch) != CLASS_THIEF
      && GET_CLASS(ch) != CLASS_BARD) {
         ansi_act(buf2, FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
	 ansi(CLR_ERROR, ch);
         send_to_char(buf, ch);
	 ansi(END, ch);
         return TRUE;
      }
   } else if ((ch->in_room == real_room(3021)) && (cmd == CMD_EAST)) {
      if (GET_CLASS(ch) != CLASS_WARRIOR
      && GET_CLASS(ch) != CLASS_DRAGONW) {
         ansi_act(buf2, FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
	 ansi(CLR_ERROR, ch);
         send_to_char(buf, ch);
	 ansi(END, ch);
         return TRUE;
      }
   }
   return FALSE;
}



int puff(struct char_data *ch, int cmd, char *arg)
{
   void do_say(struct char_data *ch, char *argument, int cmd);

   if (cmd)
      return(0);

   switch (number(0, 60))
   {
      case 0:
         do_say(ch, "My god! It's full of stars!", 0);
         return(1);
      case 1:
         do_say(ch, "Have you seen Cryogen lately?  Everyone is looking for him.", 0);
         return(1);
      case 2:
         do_say(ch, "I'm a very female dragon.", 0);
         return(1);
      case 3:
         do_say(ch, "Do you know that there are more than 20 zones in the world?", 0);
         return(1);
      case 4:
         do_say(ch, "Hey buddy, I am level 26. What about you?", 0);
         return(1);
      default:
         return(0);
   }
}
               
int fido(struct char_data *ch, int cmd, char *arg)
{
   struct obj_data *i, *temp, *next_obj;

   if (cmd || !AWAKE(ch))
      return(FALSE);

   for (i = world[ch->in_room].contents; i; i = i->next_content) {
      if (GET_ITEM_TYPE(i)==ITEM_CONTAINER && i->obj_flags.value[3]) {
         ansi_act("$n savagely devour a corpse.", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
         for(temp = i->contains; temp; temp=next_obj)
         {
            next_obj = temp->next_content;
            obj_from_obj(temp);
            obj_to_room(temp,ch->in_room);
         }
         extract_obj(i);
         return(TRUE);
      }
   }
   return(FALSE);
}


int janitor(struct char_data *ch, int cmd, char *arg)
{
   struct obj_data *i;

   if (cmd || !AWAKE(ch))
      return(FALSE);

   for (i = world[ch->in_room].contents; i; i = i->next_content) {
      if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) && 
      ((i->obj_flags.type_flag == ITEM_DRINKCON) ||
        (i->obj_flags.cost <= 10))) {
         ansi_act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);

         obj_from_room(i);
         obj_to_char(i, ch);
         return(TRUE);
      }
   }
   return(FALSE);
}


int slime(struct char_data *ch, int cmd, char *arg)
{
   struct obj_data *i;

   if (cmd || !AWAKE(ch))
      return(FALSE);

   if (!number(0,20))
     ansi_act("$n gurgles noisily.", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
   for (i = world[ch->in_room].contents; i; i = i->next_content) {
		if ((IS_SET(i->obj_flags.wear_flags, ITEM_TAKE)) 
      		&& (!((i->obj_flags.type_flag == ITEM_CONTAINER) && 
		    (i->obj_flags.value[3]==1)) )){
			ansi_act("$n dissolves some stuff left lying around.", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
			extract_obj(i);
			return(TRUE);
		}
	}
	return(FALSE);
}

int dungeon_master(struct char_data *ch, int cmd, char *arg)
{
   if (cmd || !AWAKE(ch))
      return(FALSE);

   if (!ch->specials.fighting && !number(0,5)) {
        cast_teleport(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,0,0);
	return(TRUE);
      } else if (IS_EVIL(ch) && !number(0,3)) 
	ansi_act("$n shouts at you, 'Get lost!!!'", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
      else if (!IS_EVIL(ch) && !number(0,3))
	ansi_act("$n says to you, 'Have a nice day.'", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
   return(FALSE);
}

int gremlin(struct char_data *ch, int cmd, char *arg)
{
   struct obj_data *i;

   if (cmd || !AWAKE(ch))
      return(FALSE);

   if (!ch->specials.fighting && !number(0,10))
         cast_teleport(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,
             0, 0);
   if (!number(0,30))
     ansi_act("$n shrieks with insane laughter.", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
   for (i = world[ch->in_room].contents; i; i = i->next_content) {
		if (!number(0,10) && (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE)) 
      		&& (!((i->obj_flags.type_flag == ITEM_CONTAINER) && 
		    (i->obj_flags.value[3]==1)) )){
			extract_obj(i);
			return(TRUE);
		}
	}
	return(FALSE);
}


int cityguard(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch) || (GET_POS(ch) == POSITION_FIGHTING))
    return (FALSE);

  max_evil = 1000;
  evil = 0;

  for (tch=world[ch->in_room].people; tch; tch = tch->next_in_room) 
    if (tch->specials.fighting) {
      if (!IS_NPC(tch) && IS_SET(tch->specials.act, PLR_ATTACKER)) {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      } 
      else if (!IS_NPC(tch) && !number(0,3) && IS_SET(tch->specials.act, PLR_PKILLER)) {
	evil = tch;
	max_evil = -1001; /* kinda kludgey */
      } /* for */
      
      if (evil) {
	if (max_evil < -1000) {
	  ansi_act("$n screams, 'Hey!! You're wanted for MURDER!'", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
	  hit(ch, evil, TYPE_UNDEFINED);
	  return(TRUE);
	} else {
	  ansi_act("$n screams, 'Fool! Fighting is punishable by DEATH!!'", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
	  hit(ch, evil, TYPE_UNDEFINED);
	  return(TRUE);
	}
      }
    } else if ((GET_LEVEL(tch) > IMO_LEV) && !IS_NPC(tch) && !number(0,5)) {
      ansi_act("$n salutes you.", TRUE, ch, 0, tch, TO_VICT,CLR_ACTION);
      ansi_act("$n salutes $N.", TRUE, ch, 0, tch, TO_NOTVICT,CLR_ACTION);
      return(TRUE);
    }
   return(FALSE);
}


int bountyhunter(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tch, *evil;

  if (cmd || !AWAKE(ch) || (GET_POS(ch) == POSITION_FIGHTING))
    return (FALSE);

  evil = 0;

  for (tch=world[ch->in_room].people; tch; tch = tch->next_in_room) 
    if (tch->specials.fighting) {
      if (!IS_NPC(tch) && !number(0,3) && 
	  (IS_SET(tch->specials.act, PLR_PKILLER) || 
	   IS_SET(tch->specials.act, PLR_ENFORCER)))
	evil = tch;

  if (evil) {
    if (IS_SET(tch->specials.act, PLR_PKILLER)) {
      ansi_act("$n screams, 'Hey!! You're wanted for MURDER!'", FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
      hit(ch, evil, TYPE_UNDEFINED);
      return(TRUE);
    } else { 
      ansi_act("$n congratulates you.", FALSE, ch, 0, evil, TO_VICT,CLR_ACTION);
      ansi_act("$n congratulates $N.", FALSE, ch, 0, evil, TO_NOTVICT,CLR_ACTION);
      return(TRUE);
    } 
  }
  } else if ((GET_LEVEL(tch) > IMO_LEV) && !IS_NPC(tch) && !number(0,4)) { 
      ansi_act("$n bows before you.", TRUE, ch, 0, tch, TO_VICT,CLR_ACTION);
      ansi_act("$n bows before $N.", TRUE, ch, 0, tch, TO_NOTVICT,CLR_ACTION);
      return(TRUE);
    }
  return(FALSE);
}


int pet_shops(struct char_data *ch, int cmd, char *arg)
{
   char buf[MAX_STRING_LENGTH], pet_name[256];
   int pet_room;
   struct char_data *pet;

   pet_room = ch->in_room+1;

   if (cmd==59) { /* List */
      ansi(CLR_ACTION, ch);
      send_to_char("Available pets are:\n\r", ch);
      ansi(END, ch);
      for(pet = world[pet_room].people; pet; pet = pet->next_in_room) {
         sprintf(buf, "%8d - %s\n\r", 3*GET_EXP(pet), pet->player.short_descr);
         send_to_char(buf, ch);
      }
      return(TRUE);
   } else if (cmd==56) { /* Buy */

      arg = one_argument(arg, buf);
      arg = one_argument(arg, pet_name);
      /* Pet_Name is for later use when I feel like it */

      if (!(pet = get_char_room(buf, pet_room))) {
	 ansi(CLR_ERROR, ch);
         send_to_char("There is no such pet!\n\r", ch);
	 ansi(END, ch);
         return(TRUE);
      }

      if (GET_GOLD(ch) < (GET_EXP(pet)*3)) {
	 ansi(CLR_ERROR, ch);
         send_to_char("You don't have enough gold!\n\r", ch);
	 ansi(END, ch);
         return(TRUE);
      }

      GET_GOLD(ch) -= GET_EXP(pet)*3;

      pet = read_mobile(pet->nr, REAL);
      GET_EXP(pet) = 0;
      SET_BIT(pet->specials.affected_by, AFF_CHARM);

      if (*pet_name) {
         sprintf(buf,"%s %s", pet->player.name, pet_name);
         str_free(pet->player.name);
         pet->player.name = str_alloc(buf);     

         sprintf(buf,"%sA small sign on a chain around the neck says, 'My Name is %s.'\n\r",
           pet->player.description, pet_name);
         str_free(pet->player.description);
         pet->player.description = str_alloc(buf);
      }

      char_to_room(pet, ch->in_room);
      add_follower(pet, ch);

      /* Be certain that pet's can't get/carry/use/weild/wear items */
      IS_CARRYING_W(pet) = 1000;
      IS_CARRYING_N(pet) = 100;

      ansi(CLR_ACTION, ch);
      send_to_char("May you enjoy your pet.\n\r", ch);
      ansi(END, ch);
      ansi_act("$n bought $N as a pet.",FALSE,ch,0,pet,TO_ROOM,CLR_ACTION);

      return(TRUE);
   }

   /* All commands except list and buy */
   return(FALSE);
}


/* Idea of the LockSmith is functionally similar to the Pet Shop */
/* The problem here is that each key must somehow be associated  */
/* with a certain player. My idea is that the players name will  */
/* appear as the another Extra description keyword, prefixed     */
/* by the words 'item_for_' and followed by the player name.     */
/* The (keys) must all be stored in a room which is (virtually)  */
/* adjacent to the room of the lock smith.                       */

int pray_for_items(struct char_data *ch, int cmd, char *arg)
{
  char buf[256];
  int key_room, gold;
   bool found;
   struct obj_data *tmp_obj, *obj;

   if (cmd != 176) /* You must pray to get the stuff */
		return FALSE;

   key_room = 1+ch->in_room;

  strcpy(buf, "item_for_");
  strcat(buf, GET_NAME(ch));

  gold = 0;
  found = FALSE;

  for (tmp_obj = world[key_room].contents; tmp_obj; tmp_obj = tmp_obj->next_content)
      /* for(ext = tmp_obj->ex_description; ext; ext = ext->next) */
      /* if (str_cmp(buf, ext->keyword) == 0) */ {
        if (gold == 0) {
           gold = 1;
            ansi_act("$n kneels and at the altar and chants a prayer to Odin.",
                FALSE, ch, 0, 0, TO_ROOM,CLR_ACTION);
            ansi_act("You notice a faint light in Odin's eye.",
                FALSE, ch, 0, 0, TO_CHAR,CLR_ACTION);
        }
        obj = read_object(tmp_obj->item_number, REAL);
        obj_to_room(obj, ch->in_room);
        ansi_act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_ROOM,CLR_ACTION);
        ansi_act("$p slowly fades into existence.",FALSE,ch,obj,0,TO_CHAR,CLR_ACTION);
        gold += obj->obj_flags.cost;
        found = TRUE;
      }


  if (found) {
    GET_GOLD(ch) -= gold;
    GET_GOLD(ch) = MAXV(0, GET_GOLD(ch));
    return TRUE;
   }

  return FALSE;
}


/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */

#define CHAL_ACT \
"You are torn out of reality!\n\r\
You roll and tumble through endless voids for what seems like eternity...\n\r\
\n\r\
After a time, a new reality comes into focus... you are elsewhere.\n\r"


int chalice(struct char_data *ch, int cmd, char *arg)
{
   /* 222 is the normal chalice, 223 is chalice-on-altar */

   struct obj_data *chalice;
   char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
   static int chl = -1, achl = -1;

   if (chl < 1)
   {
      chl = real_object(222);
      achl = real_object(223);
   }

   switch(cmd)
   {
      case 10:    /* get */
         if (!(chalice = get_obj_in_list_num(chl,
            world[ch->in_room].contents))
            && CAN_SEE_OBJ(ch, chalice))
            if (!(chalice = get_obj_in_list_num(achl,
               world[ch->in_room].contents)) && CAN_SEE_OBJ(ch, chalice))
               return(0);
   
         /* we found a chalice.. now try to get us */       
         do_get(ch, arg, cmd);
         /* if got the altar one, switch her */
         if (chalice == get_obj_in_list_num(achl, ch->carrying))
         {
            extract_obj(chalice);
            chalice = read_object(chl, VIRTUAL);
            obj_to_char(chalice, ch);
         }
         return(1);
      break;
      case 67: /* put */
         if (!(chalice = get_obj_in_list_num(chl, ch->carrying)))
            return(0);

         argument_interpreter(arg, buf1, buf2);
         if (!str_cmp(buf1, "chalice") && !str_cmp(buf2, "altar"))
         {
            extract_obj(chalice);
            chalice = read_object(achl, VIRTUAL);
            obj_to_room(chalice, ch->in_room);
            send_to_char("Ok.\n\r", ch);
         }
         return(1);
      break;
      case 176: /* pray */
         if (!(chalice = get_obj_in_list_num(achl,
            world[ch->in_room].contents)))
            return(0);

         do_action(ch, arg, cmd);  /* pray */
         send_to_char(CHAL_ACT, ch);
         extract_obj(chalice);
         act("$n is torn out of existence!", TRUE, ch, 0, 0, TO_ROOM);
         char_from_room(ch);
         char_to_room(ch, real_room(2500));   /* before the fiery gates */
         do_look(ch, "", 15);
         return(1);
      break;
      default:
         return(0);
      break;
   }
}

int kings_hall(struct char_data *ch, int cmd, char *arg)
{
   if (cmd != 176)
      return(0);

   do_action(ch, arg, 176);

   send_to_char("You feel as if some mighty force has been offended.\n\r", ch);
   send_to_char(CHAL_ACT, ch);
   act("$n is struck by an intense beam of light and vanishes.",
      TRUE, ch, 0, 0, TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, real_room(1420));  /* behind the altar */
   do_look(ch, "", 15);
   return(1);
}

















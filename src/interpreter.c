/* ************************************************************************
*  file: Interpreter.c , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/telnet.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "limits.h"
#include "constants.h"
#include "act.h"
#include "ban.h"
#include "ansi.h"
#include "interpreter.h"

#define LOG_NEW_CONNECTION 0 /* dont particularly wish to see this in log */

#define COMMANDO(number,min_pos,pointer,min_level) {      \
   cmd_info[(number)].command_pointer = (pointer);         \
   cmd_info[(number)].minimum_position = (min_pos);        \
   cmd_info[(number)].minimum_level = (min_level); }

#define NOT !
#define AND &&
#define OR ||

#define STATE(d) ((d)->connected)

char echo_off[]={IAC,WILL,TELOPT_ECHO, '\0'};
char echo_on[]={IAC,WONT,TELOPT_ECHO, '\n','\r','\0'};

struct command_info cmd_info[MAX_CMD_LIST];

/* external fcntls */

void init_char(struct char_data *ch);
int create_entry(char *name);

const char *command[]=
{ "north",       /* 1 */
  "east",
  "south",
  "west",
  "up",
  "down",
  "enter",
  "exits",
  "kiss",
  "get",
  "drink",       /* 11 */
  "eat",
  "wear",
  "wield",
  "look",
  "score",
  "say",
  "shout",
  "tell",
  "inventory",
  "qui",        /* 21 */
  "bounce",
  "smile",    /* 23 */
  "dance",   /* 24 */
  "kill",
  "cackle",
  "laugh",
  "giggle",
  "shake",
  "puke",
  "growl",        /* 31 */    
  "scream",
  "insult",
  "comfort",
  "nod",
  "sigh",  /* 36 */
  "sulk",
  "help",
  "who",
  "emote",
  "echo",        /* 41 */
  "stand",
  "sit",
  "rest",
  "sleep",
  "wake",
  "force",
  "transfer",
  "hug",
  "snuggle",
  "cuddle",      /* 51 */
  "nuzzle",
  "cry",
  "news",
  "equipment",
  "buy",
  "sell",
  "value",
  "list",
  "drop",
  "goto",          /* 61 */
  "weather",
  "read",
  "pour",
  "grab",
  "remove",
  "put",
  "shutdow",
  "save",
  "hit",
  "string",      /* 71 */
  "give",
  "quit",
  "stat",
  "setskill",
  "time",
  "load",
  "purge",
  "shutdown",
  "idea",
  "typo",        /* 81 */
  "bug",
  "whisper",
  "cast",
  "at",
  "ask",
  "order",
  "sip",
  "taste",
  "snoop",
  "follow",      /* 91 */
  "rent",
  "offer",
  "poke",
  "advance",
  "accuse",
  "grin",
  "bow",
  "open",
  "close",
  "lock",        /* 101 */
  "unlock",
  "leave",
  "applaud",
  "blush",   /* 105 */
  "burp",
  "chuckle",
  "clap",
  "cough", /* 109 */
  "curtsey",
  "fart",        /* 111 */
  "flip",
  "fondle",
  "frown",
  "gasp",
  "glare",
  "groan",
  "grope",
  "hiccup",
  "lick",
  "love",        /* 121 */
  "moan",
  "nibble",
  "pout",
  "purr",
  "ruffle",
  "shiver",
  "shrug",
  "sing",
  "slap",
  "smirk",       /* 131 */
  "snap",
  "sneeze",
  "snicker",
  "sniff",
  "snore",
  "spit",
  "squeeze",
  "stare",
  "strut",
  "thank",       /* 141 */
  "twiddle", /* 142 */
  "wave",
  "whistle", /* 144 */
  "wiggle",
  "wink",
  "yawn",   /* 147 */
  "snowball",
  "write",
  "hold",
  "flee",        /* 151 */
  "sneak",
  "hide",
  "backstab",
  "pick",
  "steal",
  "bash",
  "rescue",
  "kick",
  "french",
  "comb",        /* 161 */
  "massage",
  "tickle",
  "practice",
  "pat",
  "examine",
  "take",
  "info",
  "'",
  "practise",
  "curse",       /* 171 */
  "use",
  "where",
  "levels",
  "reroll",
  "pray",
  ",",
  "beg",
  "bleed",
  "cringe",
  "daydream",    /* 181 */
  "fume",
  "grovel",
  "hop",
  "nudge",
  "peer",
  "point",
  "ponder",
  "punch",
  "snarl",
  "spank",       /* 191 */
  "steam",
  "tackle",
  "taunt",
  "think",
  "whine",
  "worship",
  "yodel",
  "brief",
  "wizlist",
  "consider",    /* 201 */
  "group",
  "restore",
  "return",
  "switch",      /* 205 */
  "quaff",
  "recite",
  "users",
  "speedwalk", /* 209 */
  "noshout",   /* 210 */
  "wizhelp",  
  "credits",
  "compact",
  "title",
  "junk",   /* 215 */
  "spells", /*216 (duh) */
  "breath", /* 217 */
  "visible", /* 218 */
  "withdraw", /* 219 */
  "deposit", /* 220 */
  "balance", /* 221 */
  "rhelp",    /* 222 */
  "rcopy",    /* 223 */
  "rlink",    /* 224 */
  "rsect",    /* 225 */
  "rflag",    /* 226 */
  "rdesc",    /* 227 */
  "rsave",    /* 228 */
  "rname",    /* 229 */
  "rlflag",    /* 230 */
  "rlist",    /* 231 */
  "gossip",   /* 232 */
  "immtalk",  /* 233 */
  "auction",  /* 234 */
  "poofin",   /* 235 */
  "poofout",  /* 236 */
  "ansi",     /* 237 */
  "noimm",  /* 238 */
  "olist",    /* 239 */
  "mlist",    /* 240 */
  "oflag",    /* 241 */
  "nogossip",  /* 242 */
  "noauction", /* 243 */
  "nosummon",  /* 244 */
  "holler",    /* 245 */
  "ostat",    /* 246 */
  "commands",    /* 247 */
  "mflag",    /* 248 */
  "osave",    /* 249 */
  "msave",    /* 250 */
  "mstat",    /* 251 */
  "maffect",    /* 252 */
  "rgoto",    /* 253 */
  "rstat",    /* 254 */
  "grats",    /* 255 */
  "nograts",    /* 256 */
  "notell",    /* 257 */
  "assist",    /* 258 */
  "wimpy",    /* 259 */
  "vnum",    /* 260 */
  "display",    /* 261 */
  "owflag",    /* 262 */
  "ovalues",    /* 263 */
  "zlist",    /* 264 */
  "ocost",    /* 265 */
  "orent",    /* 266 */
  "oweight",    /* 267 */
  "okeywords",    /* 268 */
  "otype",    /* 269 */
  "osdesc",    /* 270 */
  "oldesc",    /* 271 */
  "owear",    /* 272 */
  "oaffect",    /* 273 */
  "ocreate",    /* 274 */
  "msdesc",    /* 275 */
  "mldesc",    /* 276 */
  "mdesc",    /* 277 */
  "mac",    /* 278 */
  "malign",    /* 279 */
  "mthaco",    /* 280 */
  "mhitpts",    /* 281 */
  "mdamage",    /* 282 */
  "mlevel",    /* 283 */
  "mexp",    /* 284 */
  "mgold",    /* 285 */
  "mcreate",    /* 286 */
  "zreset",    /* 287 */
  "mkeywords",    /* 288 */
  "nohassle",    /* 289 */
  "wizinvis",    /* 290 */
  "crashsave",    /* 291 */
  "muzzle",    /* 292 */
  "zedit",    /* 293 */
  "ban",    /* 294 */
  "unban",    /* 295 */
  "nosys",    /* 296 */
  "noclan",    /* 297 */
  "split",    /* 298 */
  "auto",    /* 299 */
  "zcreate",    /* 300 */
  "zsave",    /* 301 */
  "msex",    /* 302 */
  "zfirst",    /* 303 */
  "zreboot",    /* 304 */
  "zmax",    /* 305 */
  "zstat",    /* 306 */
  "zflag",    /* 307 */
  "redesc",    /* 308 */
  "oedesc",    /* 309 */
  "donate",    /* 310 */
  "\n" /* current MAX_CMD_LIST is 312-=1 = 311 */
};


const char *fill[]=
{ "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

int search_block(char *arg, const char *list[], bool exact)
{
   register int i,l;

   /* Make into lower case, and get length of string */
   for(l=0; *(arg+l); l++)
      *(arg+l)=LOWER(*(arg+l));

   if (exact) {
      for(i=0; **(list+i) != '\n'; i++)
         if (!strcmp(arg, *(list+i)))
            return(i);
   } else {
      if (!l)
         l=1; /* Avoid "" to match the first available string */
      for(i=0; **(list+i) != '\n'; i++)
         if (!strncmp(arg, *(list+i), l))
            return(i);
   }

   return(-1);
}


int old_search_block(char *argument,int begin,int length,const char *list[],int mode)
{
   int guess, found, search;
        
   /* If the word contain 0 letters, then a match is already found */
   found = (length < 1);

   guess = 0;

   /* Search for a match */

   if(mode>0)
   while ( NOT found AND *(list[guess]) != '\n' )
   {
      found=(length==strlen(list[guess]));
      for(search=0;( search < length AND found );search++)
         found=(*(argument+begin+search)== *(list[guess]+search));
      guess++;
   } else if(mode == 0){
      while ( NOT found AND *(list[guess]) != '\n' ) {
         found=1;
         for(search=0;( search < length AND found );search++)
            found=(*(argument+begin+search)== *(list[guess]+search));
         guess++;
      }
   } else {
      while ( NOT found AND *(list[guess]) != '\n' ) {
         found=1;
         for(search=0;( search < length AND found );search++)
            found=(LOWER(*(argument+begin+search))== LOWER(*(list[guess]+search)));
         guess++;
      }
   }
   return ( found ? guess : -1 ); 
}

void command_interpreter(struct char_data *ch, char *argument) 
{
   int look_at, cmd, begin;
   extern int no_specials;

   REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

        /* Find first non blank */
   for (begin = 0 ; (*(argument + begin ) == ' ' ) ; begin++ );
   
   /* Find length of first word */
   for (look_at = 0; *(argument + begin + look_at ) > ' ' ; look_at++)

            /* Make all letters lower case AND find length */
      *(argument + begin + look_at) = 
      LOWER(*(argument + begin + look_at));

   
   cmd = old_search_block(argument,begin,look_at,command,0);
 
   
   if (!cmd)
      return;

   if ( cmd>0 && (unsigned int)GET_LEVEL(ch)<cmd_info[cmd].minimum_level )
   {
      ansi(CLR_ERROR, ch);
      send_to_char("Arglebargle, glop-glyf!?!\n\r", ch);
      ansi(END, ch);
      return;
   }

   if ( cmd>0 && (cmd_info[cmd].command_pointer != 0))
   {
      if( (unsigned int)GET_POS(ch) < cmd_info[cmd].minimum_position )
         switch(GET_POS(ch))
         {
            case POSITION_DEAD: {
               ansi(CLR_ERROR, ch);
               send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
               ansi(END, ch);
	     }
            break;
            case POSITION_INCAP:
            case POSITION_MORTALLYW: {
               ansi(CLR_ERROR, ch);
               send_to_char(
                  "You are in a pretty bad shape, unable to do anything!\n\r",
                  ch);
              ansi(END, ch); 
	     }
            break;

            case POSITION_STUNNED: {
               ansi(CLR_ERROR, ch);
               send_to_char(
               "All you can do right now, is think about the stars!\n\r", ch);
              ansi(END, ch); 
	     }
            break;
            case POSITION_SLEEPING: {
               ansi(CLR_ERROR, ch);
               send_to_char("In your dreams, or what?\n\r", ch);
               ansi(END, ch); 
	     }
            break;
            case POSITION_RESTING: {
               ansi(CLR_ERROR, ch);
               send_to_char("Nah... You feel too relaxed to do that..\n\r",
                  ch);
               ansi(END, ch); 
	     }
            break;
            case POSITION_SITTING: {
               ansi(CLR_ERROR, ch);
               send_to_char("Maybe you should get on your feet first?\n\r",ch);
               ansi(END, ch); 
	     } 
           break; 
           case POSITION_FIGHTING: {
               ansi(CLR_ERROR, ch);
               send_to_char("No way! You are fighting for your life!\n\r", ch);
               ansi(END, ch); 
	     }
            break;
         }
      else
      {
      /* according to Daerin's post this next line is the fix for a bug...
	 because do move calls the special proc too... */
	 if (cmd>=6)
            if (!no_specials && special(ch, cmd, argument + begin + look_at))
               return;  

         ((*cmd_info[cmd].command_pointer)
         (ch, argument + begin + look_at, cmd));
      }
      return;
   }
   if ( cmd>0 && (cmd_info[cmd].command_pointer == 0)) {
      ansi(CLR_ERROR, ch);
      send_to_char("Sorry, but that command has yet to be implemented...\n\r", ch);
      ansi(END, ch); 
    } else { 
      ansi(CLR_ERROR, ch);
      send_to_char("Arglebargle, glop-glyf!?!\n\r", ch);
      ansi(END, ch);
    }
}

void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
        int look_at, found, begin;

        found = begin = 0;

        do
        {
                /* Find first non blank */
                for ( ;*(argument + begin ) == ' ' ; begin++);

                /* Find length of first word */
                for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

                        /* Make all letters lower case,
                           AND copy them to first_arg */
                        *(first_arg + look_at) =
                        LOWER(*(argument + begin + look_at));

                *(first_arg + look_at)='\0';
                begin += look_at;

        }
        while( fill_word(first_arg));

        do
        {
                /* Find first non blank */
                for ( ;*(argument + begin ) == ' ' ; begin++);

                /* Find length of first word */
                for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

                        /* Make all letters lower case,
                           AND copy them to second_arg */
                        *(second_arg + look_at) =
                        LOWER(*(argument + begin + look_at));

                *(second_arg + look_at)='\0';
                begin += look_at;

        }
        while( fill_word(second_arg));
}

int is_number(char *str)
{
   int look_at;

   if(*str=='\0')
      return(0);

   for(look_at=0;*(str+look_at) != '\0';look_at++)
      if((*(str+look_at)<'0')||(*(str+look_at)>'9'))
         return(0);
   return(1);
}

/*  Quinn substituted a new one-arg for the old one.. I thought returning a 
    char pointer would be neat, and avoiding the func-calls would save a
    little time... If anyone feels pissed, I'm sorry.. Anyhow, the code is
    snatched from the old one, so it outta work..

void one_argument(char *argument,char *first_arg )
{
   static char dummy[MAX_STRING_LENGTH];

   argument_interpreter(argument,first_arg,dummy);
}

*/


/* find the first sub-argument of a string, return pointer to first char in
   primary argument, following the sub-arg                     */
char *one_argument(char *argument, char *first_arg )
{
   int found, begin, look_at;

        found = begin = 0;

        do
        {
                /* Find first non blank */
                for ( ;isspace(*(argument + begin)); begin++);

                /* Find length of first word */
                for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

                        /* Make all letters lower case,
                           AND copy them to first_arg */
                        *(first_arg + look_at) =
                        LOWER(*(argument + begin + look_at));

                *(first_arg + look_at)='\0';
      begin += look_at;
   }
        while (fill_word(first_arg));

   return(argument+begin);
}
   

int fill_word(char *argument)
{
   return ( search_block(argument,fill,TRUE) >= 0);
}


/* determine if a given string is an abbreviation of another */
int is_abbrev(char *arg1, char *arg2)
{
   if (!*arg1)
      return(0);

   for (; *arg1; arg1++, arg2++)
      if (LOWER(*arg1) != LOWER(*arg2))
         return(0);

   return(1);
}


/* return first 'word' plus trailing substring of input string */
void half_chop(char *string, char *arg1, char *arg2)
{
   for (; isspace(*string); string++);

   for (; !isspace(*arg1 = *string) && *string; string++, arg1++);

   *arg1 = '\0';

   for (; isspace(*string); string++);

   for (; (*arg2 = *string); string++, arg2++);
}


int special(struct char_data *ch, int cmd, char *arg)
{
   register struct obj_data *i;
   register struct char_data *k;
   int j;

   /* special in room? */
   if (world[ch->in_room].funct)
      if ((*world[ch->in_room].funct)(ch, cmd, arg))
         return(1);

   /* special in equipment list? */
   for (j = 0; j <= (MAX_WEAR - 1); j++)
      if (ch->equipment[j] && ch->equipment[j]->item_number>=0)
         if (obj_index[ch->equipment[j]->item_number].func)
            if ((*obj_index[ch->equipment[j]->item_number].func)
               (ch, cmd, arg))
               return(1);

   /* special in inventory? */
   for (i = ch->carrying; i; i = i->next_content)
      if (i->item_number>=0)
         if (obj_index[i->item_number].func)
            if ((*obj_index[i->item_number].func)(ch, cmd, arg))
              return(1);

   /* special in mobile present? */
   for (k = world[ch->in_room].people; k; k = k->next_in_room)
      if ( IS_MOB(k) )
         if (mob_index[k->nr].func)
            if ((*mob_index[k->nr].func)(ch, cmd, arg))
               return(1);

   /* special in object present? */
   for (i = world[ch->in_room].contents; i; i = i->next_content)
      if (i->item_number>=0)
         if (obj_index[i->item_number].func)
            if ((*obj_index[i->item_number].func)(ch, cmd, arg))
               return(1);

   return(0);
}

void assign_command_pointers ( void )
{
   int position;

   for (position = 0 ; position < MAX_CMD_LIST; position++)
      cmd_info[position].command_pointer = 0;

/* format like so */
/* COMMANDO(273,         POSITION_STANDING, do_oaffect, 0); */

   COMMANDO(CMD_NORTH,   POSITION_STANDING, do_move,    1);
   COMMANDO(CMD_EAST,    POSITION_STANDING, do_move,    1);
   COMMANDO(CMD_SOUTH,   POSITION_STANDING, do_move,    1);
   COMMANDO(CMD_WEST,    POSITION_STANDING, do_move,    1);
   COMMANDO(CMD_UP,      POSITION_STANDING, do_move,    1);
   COMMANDO(CMD_DOWN,    POSITION_STANDING, do_move,    1);
   COMMANDO(6,POSITION_STANDING,do_move,1);
   COMMANDO(7,POSITION_STANDING,do_enter,1);
   COMMANDO(8,POSITION_RESTING,do_exits,1);
   COMMANDO(9,POSITION_RESTING,do_action,1);
   COMMANDO(CMD_GET,     POSITION_RESTING,  do_get,     1);
   COMMANDO(11,POSITION_RESTING,do_drink,1);
   COMMANDO(12,POSITION_RESTING,do_eat,1);
   COMMANDO(13,POSITION_RESTING,do_wear,1);
   COMMANDO(14,POSITION_RESTING,do_wield,1);
   COMMANDO(15,POSITION_RESTING,do_look,1);
   COMMANDO(16,POSITION_DEAD,do_score,1);
   COMMANDO(17,POSITION_RESTING,do_say,1);
   COMMANDO(18,POSITION_RESTING,do_shout,1);
   COMMANDO(19,POSITION_DEAD,do_tell,1);
   COMMANDO(20,POSITION_DEAD,do_inventory,1);
   COMMANDO(21,POSITION_DEAD,do_qui,1);
   COMMANDO(22,POSITION_STANDING,do_action,1);
   COMMANDO(CMD_SMILE,POSITION_RESTING,do_action,1);
   COMMANDO(CMD_DANCE,POSITION_STANDING,do_action,1);
   COMMANDO(25,POSITION_FIGHTING,do_kill,1);
   COMMANDO(26,POSITION_RESTING,do_action,1);
   COMMANDO(27,POSITION_RESTING,do_action,1);
   COMMANDO(28,POSITION_RESTING,do_action,1);
   COMMANDO(29,POSITION_RESTING,do_action,1);
   COMMANDO(30,POSITION_RESTING,do_action,1);
   COMMANDO(31,POSITION_RESTING,do_action,1);
   COMMANDO(32,POSITION_RESTING,do_action,1);
   COMMANDO(33,POSITION_RESTING,do_insult,1);
   COMMANDO(34,POSITION_RESTING,do_action,1);
   COMMANDO(35,POSITION_RESTING,do_action,1);
   COMMANDO(CMD_SIGH,POSITION_RESTING,do_action,1);
   COMMANDO(37,POSITION_RESTING,do_action,1);
   COMMANDO(38,POSITION_DEAD,do_help,1);
   COMMANDO(39,POSITION_DEAD,do_who,1);
   COMMANDO(40,POSITION_SLEEPING,do_emote,1);
   COMMANDO(41,POSITION_SLEEPING,do_echo,IMO_LEV2); 
   COMMANDO(42,POSITION_RESTING,do_stand,1);
   COMMANDO(43,POSITION_RESTING,do_sit,1);
   COMMANDO(44,POSITION_RESTING,do_rest,1);
   COMMANDO(45,POSITION_SLEEPING,do_sleep,1);
   COMMANDO(46,POSITION_SLEEPING,do_wake,1);
   COMMANDO(47,POSITION_SLEEPING,do_force,IMO_LEV3);
   COMMANDO(48,POSITION_SLEEPING,do_trans,IMO_LEV2);
   COMMANDO(49,POSITION_RESTING,do_action,1);
   COMMANDO(50,POSITION_RESTING,do_action,1);
   COMMANDO(51,POSITION_RESTING,do_action,1);
   COMMANDO(52,POSITION_RESTING,do_action,1);
   COMMANDO(53,POSITION_RESTING,do_action,1);
   COMMANDO(54,POSITION_SLEEPING,do_news,1);
   COMMANDO(55,POSITION_SLEEPING,do_equipment,1);
   COMMANDO(56,POSITION_STANDING,do_not_here,1);
   COMMANDO(57,POSITION_STANDING,do_not_here,1);
   COMMANDO(58,POSITION_STANDING,do_not_here,1);
   COMMANDO(59,POSITION_STANDING,do_not_here,1);
   COMMANDO(CMD_DROP,POSITION_RESTING,do_drop,1);
   COMMANDO(61,POSITION_SLEEPING,do_goto,IMO_LEV);
   COMMANDO(62,POSITION_RESTING,do_weather,1);
   COMMANDO(63,POSITION_RESTING,do_read,1);
   COMMANDO(64,POSITION_STANDING,do_pour,1);
   COMMANDO(65,POSITION_RESTING,do_grab,1);
   COMMANDO(66,POSITION_RESTING,do_remove,1);
   COMMANDO(CMD_PUT,POSITION_RESTING,do_put,1);
   COMMANDO(68,POSITION_DEAD,do_shutdow,IMP_LEV);
   COMMANDO(69,POSITION_SLEEPING,do_save,1);
   COMMANDO(70,POSITION_FIGHTING,do_hit,1);
   COMMANDO(71,POSITION_SLEEPING,do_string,IMO_LEV4);
   COMMANDO(72,POSITION_RESTING,do_give,1);
   COMMANDO(73,POSITION_DEAD,do_quit,1);
   COMMANDO(74,POSITION_DEAD,do_stat,IMO_LEV3);
   COMMANDO(75,POSITION_SLEEPING,do_setskill,IMO_LEV3);
   COMMANDO(76,POSITION_DEAD,do_time,1);
   COMMANDO(77,POSITION_DEAD,do_load,IMO_LEV3);
   COMMANDO(78,POSITION_DEAD,do_purge,0);
   COMMANDO(79,POSITION_DEAD,do_terminate,IMP_LEV);
   COMMANDO(80,POSITION_DEAD,do_idea,1);
   COMMANDO(81,POSITION_DEAD,do_typo,1);
   COMMANDO(82,POSITION_DEAD,do_bug,1);
   COMMANDO(83,POSITION_RESTING,do_whisper,1);
   COMMANDO(84,POSITION_SITTING,do_cast,1);
   COMMANDO(85,POSITION_DEAD,do_at,IMO_LEV2);
   COMMANDO(86,POSITION_RESTING,do_ask,1);
   COMMANDO(87,POSITION_RESTING,do_order,1);
   COMMANDO(88,POSITION_RESTING,do_sip,1);
   COMMANDO(89,POSITION_RESTING,do_taste,1);
   COMMANDO(90,POSITION_DEAD,do_snoop,IMO_LEV3);
   COMMANDO(91,POSITION_RESTING,do_follow,1);
   COMMANDO(92,POSITION_STANDING,do_not_here,1);
   COMMANDO(93,POSITION_STANDING,do_not_here,1);
   COMMANDO(94,POSITION_RESTING,do_action,1);
   COMMANDO(95,POSITION_DEAD,do_advance,IMO_LEV4);
   COMMANDO(96,POSITION_SITTING,do_action,1);
   COMMANDO(97,POSITION_RESTING,do_action,1);
   COMMANDO(98,POSITION_STANDING,do_action,1);
   COMMANDO(99,POSITION_SITTING,do_open,1);
   COMMANDO(100,POSITION_SITTING,do_close,1);
   COMMANDO(101,POSITION_SITTING,do_lock,1);
   COMMANDO(102,POSITION_SITTING,do_unlock,1);
   COMMANDO(103,POSITION_STANDING,do_leave,1);
   COMMANDO(104,POSITION_RESTING,do_action,1);
   COMMANDO(CMD_BLUSH,POSITION_RESTING,do_action,1);
   COMMANDO(CMD_BURP,POSITION_RESTING,do_action,1);
   COMMANDO(107,POSITION_RESTING,do_action,1);
   COMMANDO(108,POSITION_RESTING,do_action,1);
   COMMANDO(CMD_COUGH,POSITION_RESTING,do_action,1);
   COMMANDO(110,POSITION_STANDING,do_action,1);
   COMMANDO(CMD_FART,POSITION_RESTING,do_action,1);
   COMMANDO(112,POSITION_STANDING,do_action,1);
   COMMANDO(113,POSITION_RESTING,do_action,1);
   COMMANDO(114,POSITION_RESTING,do_action,1);
   COMMANDO(115,POSITION_RESTING,do_action,1);
   COMMANDO(116,POSITION_RESTING,do_action,1);
   COMMANDO(117,POSITION_RESTING,do_action,1);
   COMMANDO(118,POSITION_RESTING,do_action,1);
   COMMANDO(119,POSITION_RESTING,do_action,1);
   COMMANDO(120,POSITION_RESTING,do_action,1);
   COMMANDO(121,POSITION_RESTING,do_action,1);
   COMMANDO(122,POSITION_RESTING,do_action,1);
   COMMANDO(123,POSITION_RESTING,do_action,1);
   COMMANDO(124,POSITION_RESTING,do_action,1);
   COMMANDO(125,POSITION_RESTING,do_action,1);
   COMMANDO(126,POSITION_STANDING,do_action,1);
   COMMANDO(127,POSITION_RESTING,do_action,1);
   COMMANDO(128,POSITION_RESTING,do_action,1);
   COMMANDO(129,POSITION_RESTING,do_action,1);
   COMMANDO(130,POSITION_RESTING,do_action,1);
   COMMANDO(131,POSITION_RESTING,do_action,1);
   COMMANDO(132,POSITION_RESTING,do_action,1);
   COMMANDO(133,POSITION_RESTING,do_action,1);
   COMMANDO(134,POSITION_RESTING,do_action,1);
   COMMANDO(135,POSITION_RESTING,do_action,1);
   COMMANDO(136,POSITION_SLEEPING,do_action,1);
   COMMANDO(137,POSITION_STANDING,do_action,1);
   COMMANDO(138,POSITION_RESTING,do_action,1);
   COMMANDO(139,POSITION_RESTING,do_action,1);
   COMMANDO(140,POSITION_STANDING,do_action,1);
   COMMANDO(141,POSITION_RESTING,do_action,1);
   COMMANDO(142,POSITION_RESTING,do_action,1);
   COMMANDO(143,POSITION_RESTING,do_action,1);
   COMMANDO(CMD_WHISTLE,POSITION_RESTING,do_action,1);
   COMMANDO(145,POSITION_STANDING,do_action,1);
   COMMANDO(146,POSITION_RESTING,do_action,1);
   COMMANDO(CMD_YAWN,POSITION_RESTING,do_action,1);
   COMMANDO(148,POSITION_STANDING,do_action,IMO_LEV);
   COMMANDO(149,POSITION_STANDING,do_write,1);
   COMMANDO(150,POSITION_RESTING,do_grab,1);
   COMMANDO(151,POSITION_FIGHTING,do_flee,1);   
   COMMANDO(152,POSITION_STANDING,do_sneak,1);  
   COMMANDO(153,POSITION_RESTING,do_hide,1); 
   COMMANDO(154,POSITION_STANDING,do_backstab,1);  
   COMMANDO(155,POSITION_STANDING,do_pick,1);   
   COMMANDO(156,POSITION_STANDING,do_steal,1);  
   COMMANDO(157,POSITION_FIGHTING,do_bash,1);   
   COMMANDO(158,POSITION_FIGHTING,do_rescue,1);
   COMMANDO(159,POSITION_FIGHTING,do_kick,1);
   COMMANDO(160,POSITION_RESTING,do_action,1);
   COMMANDO(161,POSITION_RESTING,do_action,1);
   COMMANDO(162,POSITION_RESTING,do_action,1);
   COMMANDO(163,POSITION_RESTING,do_action,1);
   COMMANDO(164,POSITION_RESTING,do_practice,1);
   COMMANDO(165,POSITION_RESTING,do_action,1);
   COMMANDO(166,POSITION_SITTING,do_examine,1);
   COMMANDO(CMD_TAKE,POSITION_RESTING,do_get,1); /* take */
   COMMANDO(168,POSITION_SLEEPING,do_info,1);
   COMMANDO(169,POSITION_RESTING,do_say,1);
   COMMANDO(170,POSITION_RESTING,do_practice,1);
   COMMANDO(171,POSITION_RESTING,do_action,1);
   COMMANDO(172,POSITION_SITTING,do_use,1);
   COMMANDO(173,POSITION_DEAD,do_where,1);
   COMMANDO(174,POSITION_DEAD,do_levels,1);
   COMMANDO(175,POSITION_DEAD,do_reroll,IMO_LEV3);
   COMMANDO(CMD_PRAY,POSITION_SITTING,do_action,1);
   COMMANDO(177,POSITION_SLEEPING,do_emote,1);
   COMMANDO(178,POSITION_RESTING,do_action,1);
   COMMANDO(179,POSITION_RESTING,do_action,1);
   COMMANDO(180,POSITION_RESTING,do_action,1);
   COMMANDO(181,POSITION_SLEEPING,do_action,1);
   COMMANDO(182,POSITION_RESTING,do_action,1);
   COMMANDO(183,POSITION_RESTING,do_action,1);
   COMMANDO(184,POSITION_RESTING,do_action,1);
   COMMANDO(185,POSITION_RESTING,do_action,1);
   COMMANDO(186,POSITION_RESTING,do_action,1);
   COMMANDO(187,POSITION_RESTING,do_action,1);
   COMMANDO(188,POSITION_RESTING,do_action,1);
   COMMANDO(189,POSITION_RESTING,do_action,1);
   COMMANDO(190,POSITION_RESTING,do_action,1);
   COMMANDO(191,POSITION_RESTING,do_action,1);
   COMMANDO(192,POSITION_RESTING,do_action,1);
   COMMANDO(193,POSITION_RESTING,do_action,1);
   COMMANDO(194,POSITION_RESTING,do_action,1);
   COMMANDO(195,POSITION_RESTING,do_action,1);
   COMMANDO(196,POSITION_RESTING,do_action,1);
   COMMANDO(197,POSITION_RESTING,do_action,1);
   COMMANDO(198,POSITION_RESTING,do_action,1);
   COMMANDO(199,POSITION_DEAD,do_brief,1);
   COMMANDO(200,POSITION_DEAD,do_wizlist,1);
   COMMANDO(201,POSITION_RESTING,do_consider,1);
   COMMANDO(202,POSITION_RESTING,do_group,1);
   COMMANDO(203,POSITION_DEAD,do_restore,IMO_LEV3);
   COMMANDO(204,POSITION_DEAD,do_return,1);
   COMMANDO(205,POSITION_DEAD,do_switch,IMO_LEV2);
   COMMANDO(206,POSITION_RESTING,do_quaff,1);
   COMMANDO(207,POSITION_RESTING,do_recite,1);
   COMMANDO(208,POSITION_DEAD,do_users,IMO_LEV2);
   COMMANDO(CMD_SPEEDWALK,POSITION_STANDING,do_speedwalk,1);
   COMMANDO(210,POSITION_SLEEPING,do_noshout,IMO_LEV2);
   COMMANDO(211,POSITION_SLEEPING,do_wizhelp,1);
   COMMANDO(212,POSITION_DEAD,do_credits,1);
   COMMANDO(213,POSITION_DEAD,do_compact,1);
   COMMANDO(214,POSITION_DEAD,do_title,1);
   COMMANDO(215,POSITION_RESTING,do_junk,1);
   COMMANDO(216,POSITION_SLEEPING,do_spells,6);
   COMMANDO(217,POSITION_FIGHTING,do_breath,1);
   COMMANDO(218,POSITION_RESTING,do_visible,1);
   COMMANDO(219,POSITION_RESTING,do_withdraw,1);
   COMMANDO(220,POSITION_RESTING,do_deposit,1);
   COMMANDO(221,POSITION_RESTING,do_balance,1);
   COMMANDO(222,POSITION_RESTING,do_rhelp,0);
   COMMANDO(223,POSITION_RESTING,do_rcopy,0);
   COMMANDO(224,POSITION_RESTING,do_rlink,0);
   COMMANDO(225,POSITION_RESTING,do_rsect,0);
   COMMANDO(226,POSITION_RESTING,do_rflag,0);
   COMMANDO(227,POSITION_RESTING,do_rdesc,0);
   COMMANDO(228,POSITION_RESTING,do_rsave,0);
   COMMANDO(229,POSITION_RESTING,do_rname,0);
   COMMANDO(230,POSITION_RESTING,do_rlflag,0);
   COMMANDO(231,POSITION_RESTING,do_rlist,0);
   COMMANDO(232,POSITION_RESTING,do_gossip,1);
   COMMANDO(233,POSITION_DEAD,do_immtalk,IMO_LEV);
   COMMANDO(234,POSITION_RESTING,do_auction,1);
   COMMANDO(235,POSITION_SLEEPING,do_poofin,IMO_LEV);
   COMMANDO(236,POSITION_SLEEPING,do_poofout,IMO_LEV);
   COMMANDO(237,POSITION_SLEEPING,do_ansi,1);
   COMMANDO(238,POSITION_RESTING,do_noimm,1);
   COMMANDO(239,POSITION_RESTING,do_olist,0);
   COMMANDO(240,POSITION_RESTING,do_mlist,0);
   COMMANDO(241,POSITION_RESTING,do_oflag,0);
   COMMANDO(242,POSITION_RESTING,do_nogossip,1);
   COMMANDO(243,POSITION_RESTING,do_noauction,1);
   COMMANDO(244,POSITION_RESTING,do_nosummon,1);
   COMMANDO(245,POSITION_RESTING,do_holler,1);
   COMMANDO(246,POSITION_RESTING,do_ostat,0);
   COMMANDO(247,POSITION_RESTING,do_commands,1);
   COMMANDO(248,POSITION_RESTING,do_mflag,0);
   COMMANDO(249,POSITION_RESTING,do_osave,0);
   COMMANDO(250,POSITION_RESTING,do_msave,0);
   COMMANDO(251,POSITION_RESTING,do_mstat,0);
   COMMANDO(252,POSITION_RESTING,do_maffect,0);
   COMMANDO(253,POSITION_STANDING,do_rgoto,0);
   COMMANDO(254,POSITION_STANDING,do_rstat,0);
   COMMANDO(255,POSITION_STANDING,do_grats,1);
   COMMANDO(256,POSITION_STANDING,do_nograts,1);
   COMMANDO(257,POSITION_STANDING,do_notell,1);
   COMMANDO(258,POSITION_STANDING,do_assist,1);
   COMMANDO(259,POSITION_STANDING,do_wimpy,1);
   COMMANDO(260,POSITION_STANDING,do_vnum,IMO_LEV2);
   COMMANDO(261,POSITION_STANDING,do_display,1);
   COMMANDO(262,POSITION_STANDING,do_owflag,0);
   COMMANDO(263,POSITION_STANDING,do_ovalues,0);
   COMMANDO(264,POSITION_STANDING,do_zlist,0);
   COMMANDO(265,POSITION_STANDING,do_ocost,0);
   COMMANDO(266,POSITION_STANDING,do_orent,0);
   COMMANDO(267,POSITION_STANDING,do_oweight,0);
   COMMANDO(268,POSITION_STANDING,do_okeywords,0);
   COMMANDO(269,POSITION_STANDING,do_otype,0);
   COMMANDO(270,POSITION_STANDING,do_osdesc,0);
   COMMANDO(271,POSITION_STANDING,do_oldesc,0);
   COMMANDO(272,POSITION_STANDING,do_owear,0);
   COMMANDO(273,         POSITION_STANDING, do_oaffect, 0);
   COMMANDO(274,         POSITION_STANDING, do_ocreate, 0);
   COMMANDO(275,         POSITION_STANDING, do_msdesc,  0);
   COMMANDO(276,         POSITION_STANDING, do_mldesc,  0);
   COMMANDO(277,         POSITION_STANDING, do_mdesc,   0);
   COMMANDO(278,         POSITION_STANDING, do_mac,     0);
   COMMANDO(279,         POSITION_STANDING, do_malign,  0);
   COMMANDO(280,         POSITION_STANDING, do_mthaco,  0);
   COMMANDO(281,         POSITION_STANDING, do_mhitpoints,0);
   COMMANDO(282,         POSITION_STANDING, do_mdamage, 0);
   COMMANDO(283,         POSITION_STANDING, do_mlevel,  0);
   COMMANDO(284,         POSITION_STANDING, do_mexp,    0);
   COMMANDO(285,         POSITION_STANDING, do_mgold,   0);
   COMMANDO(286,         POSITION_STANDING, do_mcreate, 0);
   COMMANDO(CMD_ZRESET,  POSITION_STANDING, do_zreset,  0);
   COMMANDO(288,         POSITION_STANDING, do_mkeywords,0);
   COMMANDO(289,         POSITION_STANDING, do_nohassle,0);
   COMMANDO(290,         POSITION_STANDING, do_wizinvis,IMO_LEV2);
   COMMANDO(291,         POSITION_STANDING, do_crashsave,IMO_LEV4);
   COMMANDO(292,         POSITION_STANDING, do_muzzle,  IMO_LEV4);
   COMMANDO(CMD_ZEDIT,   POSITION_STANDING, do_zedit,   0);
   COMMANDO(294,         POSITION_STANDING, do_ban,     IMO_LEV4);
   COMMANDO(295,         POSITION_STANDING, do_unban,   IMO_LEV4);
   COMMANDO(296,         POSITION_STANDING, do_nosys,   1);
   COMMANDO(297,         POSITION_STANDING, do_noclan,  1);
   COMMANDO(298,         POSITION_STANDING, do_split,   1);
   COMMANDO(CMD_AUTO,    POSITION_STANDING, do_auto,    1);
   COMMANDO(CMD_ZCREATE, POSITION_STANDING, do_zcreate, 0);
   COMMANDO(CMD_ZSAVE,   POSITION_STANDING, do_zsave,   0);
   COMMANDO(CMD_MSEX,    POSITION_STANDING, do_msex,    0);
   COMMANDO(CMD_ZFIRST,  POSITION_STANDING, do_zfirst,  0);
   COMMANDO(CMD_ZREBOOT, POSITION_STANDING, do_zreboot, 0);
   COMMANDO(CMD_ZMAX,    POSITION_STANDING, do_zmax,    0);
   COMMANDO(CMD_ZSTAT,   POSITION_STANDING, do_zstat,   0);
   COMMANDO(CMD_ZFLAG,   POSITION_STANDING, do_zflag,   0);
   COMMANDO(CMD_REDESC,  POSITION_STANDING, do_redesc,  0);
   COMMANDO(CMD_OEDESC,  POSITION_STANDING, do_oedesc,  0);
   COMMANDO(CMD_DONATE,  POSITION_STANDING, do_donate,  0);
}

/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
   int i;

   for (i = 0; i <= top_of_p_table; i++)
   {
      if (!str_cmp((player_table + i)->name, name))
         return(i);
   }

   return(-1);
}


int _parse_name(unsigned char *arg, char *name)
{
   int i;

   /* skip whitespaces */
   for (; isspace(*arg); arg++);
   
   for (i = 0; (*name = *arg); arg++, i++, name++) 
      if ((*arg >127) || !isalpha(*arg) || i > 15)
         return(1); 

   if (!i)
      return(1);

   return(0);
}
         

/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
   char buf[100];
   char tmp_name[20];
   int player_i;
   struct char_file_u tmp_store;
   struct char_data *tmp_ch;
   struct descriptor_data *k;
   extern struct descriptor_data *descriptor_list;

   void do_look(struct char_data *ch, char *argument, int cmd);
   void load_char_objs(struct char_data *ch);
   int load_char(char *name, struct char_file_u *char_element);

   switch (STATE(d))
   {
      case CON_NME:     /* wait for input of name  */
         if (!d->character)
         {
            CREATE(d->character, struct char_data, 1);
            clear_char(d->character);
            d->character->desc = d;
         }

         if (arg) for (; isspace(*arg); arg++);
         if (!arg || !*arg)
            close_socket(d);
         else {

            if(_parse_name((unsigned char *)arg, tmp_name))
            {
               SEND_TO_Q("Illegal name, please try another.", d);
               SEND_TO_Q("Name: ", d);
               return;
	     }


            /* Check if already playing */
            for(k=descriptor_list; k; k = k->next) {
               if ((k->character != d->character) && k->character) {
                  if (k->original) {
                     if (GET_NAME(k->original) &&
                      (str_cmp(GET_NAME(k->original), tmp_name) == 0))
                     {
                        SEND_TO_Q("Already playing, cannot connect.\n\r", d);
                        SEND_TO_Q("Name: ", d);
                        return;
                     }
                  } else { /* No switch has been made */
                     if (GET_NAME(k->character) &&
                      (str_cmp(GET_NAME(k->character), tmp_name) == 0))
                     {
                        SEND_TO_Q("Already playing, cannot connect.\n\r", d);
                        SEND_TO_Q("Name: ", d);
                        return;
                     }
                  }
               }
            }


            if ((player_i = load_char(tmp_name, &tmp_store)) > -1)
            {
               store_to_char(&tmp_store, d->character);

               strcpy(d->pwd, tmp_store.pwd);
               d->pos = player_table[player_i].nr;

               SEND_TO_Q("Password: ", d);
	       SEND_TO_Q(echo_off,d);

            STATE(d) = CON_PWDNRM;
            }
            else
            {
               /* player unknown gotta make a new */
               CAP(tmp_name);
               GET_NAME(d->character) = str_alloc(tmp_name);
               sprintf(buf, "Did I get that right, %s (Y/N)? ",
                tmp_name);

               SEND_TO_Q(buf, d);

               STATE(d) = CON_NMECNF;
            }
         }
      break;

      case CON_NMECNF:  /* wait for conf. of new name */
         /* skip whitespaces */
         for (; isspace(*arg); arg++);
         
         if (*arg == 'y' || *arg == 'Y')
         {
            SEND_TO_Q("A new player is born!\n\r", d);
	    if (isbanned(d->host) == BAN_NEW) {
	       SEND_TO_Q("Sorry your site has been BANNED from making new characters.\n\r", d);
	       close_socket(d);
	       return;
	    }

            sprintf(buf, 
               "Give me a password for %s: ",
               GET_NAME(d->character));
            
	    SEND_TO_Q(echo_off,d);
            SEND_TO_Q(buf, d);

            STATE(d) = CON_PWDGET;
         }
         else
         {
            if (*arg == 'n' || *arg == 'N') {
               SEND_TO_Q("Ok, what IS it, then? ", d);
               if (GET_NAME(d->character)) 
		 GET_NAME(d->character) = str_free(GET_NAME(d->character));
               STATE(d) = CON_NME;
            } else { /* Please do Y or N */
               SEND_TO_Q("Please type Yes or No? ", d);
            }
         }
      break;

      case CON_PWDNRM:  /* get pwd for known player   */
         /* skip whitespaces */
         for (; isspace(*arg); arg++);
         if (!*arg)
            close_socket(d);
         else
         {
            if (strncmp((char *)crypt(arg, d->pwd), d->pwd, 10))
            {
               SEND_TO_Q("Wrong password.\n\r", d);
               SEND_TO_Q("Password: ", d);
               return;
            }
	    SEND_TO_Q(echo_on,d);

            for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
               if (!str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch)) &&
                  !tmp_ch->desc && !IS_NPC(tmp_ch))
               {
                  SEND_TO_Q("Reconnecting.\n\r", d);
                  free_char(d->character);
                  tmp_ch->desc = d;
                  d->character = tmp_ch;
                  tmp_ch->specials.timer = 0;
                  STATE(d) = CON_PLYNG;
                  ansi_act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM, CLR_ACTION);
                  sprintf(buf, "%s[%s] has reconnected.", GET_NAME(
                     d->character), d->host);
                  log(buf);
                  return;
               }
               

            sprintf(buf, "SYS: %s has entered[1;31m Crimson[0;33m.[m", GET_NAME(d->character));
            if (GET_LEVEL(d->character)<=3) strcat(buf, "[1;33m(NEWBIE)[m");
            do_sys(buf, 1);
            if (LOG_NEW_CONNECTION) {
              sprintf(buf+strlen(buf),"[%s]", d->host);
              log(buf);
            }

            SEND_TO_Q(motd, d);
            SEND_TO_Q("\n\r*** PRESS RETURN: ", d);

            STATE(d) = CON_RMOTD;
         }
      break;

      case CON_PWDGET:  /* get pwd for new player  */
         /* skip whitespaces */
         for (; isspace(*arg); arg++);

         if (!*arg || strlen(arg) > 10)
         {
            SEND_TO_Q("Illegal password.\n\r", d);
            SEND_TO_Q("Password: ", d);
            return;
         }

         strncpy(d->pwd, (char *) crypt(arg, d->character->player.name), 10);
         *(d->pwd + 10) = '\0';
         
         SEND_TO_Q("Please retype password: ", d);

         STATE(d) = CON_PWDCNF;
      break;

      case CON_PWDCNF:  /* get confirmation of new pwd   */
         /* skip whitespaces */
         for (; isspace(*arg); arg++);

         if (strncmp((char *)crypt(arg, d->pwd), d->pwd, 10))
         {
            SEND_TO_Q("Passwords don't match.\n\r", d);
            SEND_TO_Q("Retype password: ", d);
            STATE(d) = CON_PWDGET;
            return;
         }
	 SEND_TO_Q(echo_on,d);

         SEND_TO_Q("What is your sex (M/F) ? ", d);
         STATE(d) = CON_QSEX;
      break;

      case CON_QSEX:    /* query sex of new user   */
         /* skip whitespaces */
         for (; isspace(*arg); arg++);
         switch (*arg)
         {
            case 'm':
            case 'M':
               /* sex MALE */
               d->character->player.sex = SEX_MALE;
            break;

            case 'f':
            case 'F':
               /* sex FEMALE */
               d->character->player.sex = SEX_FEMALE;
            break;

            default:
               SEND_TO_Q("That's not a sex..\n\r", d);
               SEND_TO_Q("What IS your sex? :", d);
               return;
            break;
         }

         SEND_TO_Q("\n\rSelect a class:\n\r1) Magic-user\n\r2) Cleric\n\r3) Warrior\n\r4) Thief", d);
         SEND_TO_Q("\n\r5) Bard\n\r6) Kai (good)\n\r7) Drakkhar (evil)", d);
         SEND_TO_Q("\n\r8) Elfin Knight (good)\n\r9) Demon Knight (evil)\n\r0) Dragon-Kin Warrior", d);
         SEND_TO_Q("\n\r\n\r(Aligned classes must maintain their alignment or else) Class :", d);
         STATE(d) = CON_QCLASS;
      break;

      case CON_QCLASS : {
	/* skip whitespaces */
	for (; isspace(*arg); arg++);
	switch (*arg)
	  {
	  case '1':
	  case 'm':
	  case 'M': {
	    GET_CLASS(d->character) = CLASS_MAGIC_USER;
	    init_char(d->character);
	    /* create an entry in the file */
	    d->pos = create_entry(GET_NAME(d->character));
	    save_char(d->character, NOWHERE);
	    SEND_TO_Q(motd, d);
	    SEND_TO_Q("\n\r*** PRESS RETURN: ", d);
	    STATE(d) = CON_RMOTD;
	  } break;
	  case '2':
	  case 'c':
	  case 'C': {
	    GET_CLASS(d->character) = CLASS_CLERIC;
	    init_char(d->character);
	    /* create an entry in the file */
	    d->pos = create_entry(GET_NAME(d->character));
	    save_char(d->character, NOWHERE);
	    SEND_TO_Q(motd, d);
	    SEND_TO_Q("\n\r*** PRESS RETURN: ", d);
	    STATE(d) = CON_RMOTD;
	  } break;
	  case '3':
	  case 'w':
	  case 'W': {
	    GET_CLASS(d->character) = CLASS_WARRIOR;
	    init_char(d->character);
	    /* create an entry in the file */
	    d->pos = create_entry(GET_NAME(d->character));
	    save_char(d->character, NOWHERE);
	    SEND_TO_Q(motd, d);
	    SEND_TO_Q("\n\r*** PRESS RETURN: ", d);
	    STATE(d) = CON_RMOTD;
	  } break;
	  case '4':
	  case 't':
	  case 'T': {
	    GET_CLASS(d->character) = CLASS_THIEF;
	    init_char(d->character);
	    /* create an entry in the file */
	    d->pos = create_entry(GET_NAME(d->character));
	    save_char(d->character, NOWHERE);
	    SEND_TO_Q(motd, d);
	    SEND_TO_Q("\n\r*** PRESS RETURN: ", d);
	    STATE(d) = CON_RMOTD;
	  } break;
	  case '5':
	  case 'b':
	  case 'B': {
	    GET_CLASS(d->character) = CLASS_BARD;
	    init_char(d->character);
	    /* create an entry in the file */
	    d->pos = create_entry(GET_NAME(d->character));
	    save_char(d->character, NOWHERE);
	    SEND_TO_Q(motd, d);
	    SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
	    STATE(d) = CON_RMOTD;
	  } break;
	  case '6':
	  case 'p':
	  case 'P': {
	    GET_CLASS(d->character) = CLASS_KAI;
	    init_char(d->character);
	    /* create an entry in the file */
	    d->pos = create_entry(GET_NAME(d->character));
	    save_char(d->character, NOWHERE);
	    SEND_TO_Q(motd, d);
	    SEND_TO_Q("\n\r*** PRESS RETURN: ", d);
	    STATE(d) = CON_RMOTD;
	  } break;
	  case '7':
	  case 'a':
	  case 'A': {
	    GET_CLASS(d->character) = CLASS_DRAKKHAR;
	    init_char(d->character);
	    /* create an entry in the file */
	    d->pos = create_entry(GET_NAME(d->character));
	    save_char(d->character, NOWHERE);
	    SEND_TO_Q(motd, d);
	    SEND_TO_Q("\n\r*** PRESS RETURN: ", d);
	    STATE(d) = CON_RMOTD;
	  } break;
	  case '8':
	  case 'e':
	  case 'E': {
	    GET_CLASS(d->character) = CLASS_EKNIGHT;
	    init_char(d->character);
	    /* create an entry in the file */
	    d->pos = create_entry(GET_NAME(d->character));
	    save_char(d->character, NOWHERE);
	    SEND_TO_Q(motd, d);
	    SEND_TO_Q("\n\r*** PRESS RETURN: ", d);
	    STATE(d) = CON_RMOTD;
	  } break;
	  case '9':
	  case 'd':
	  case 'D': {
	    GET_CLASS(d->character) = CLASS_DKNIGHT;
	    init_char(d->character);
	    /* create an entry in the file */
	    d->pos = create_entry(GET_NAME(d->character));
	    save_char(d->character, NOWHERE);
	    SEND_TO_Q(motd, d);
	    SEND_TO_Q("\n\r*** PRESS RETURN: ", d);
	    STATE(d) = CON_RMOTD;
	  } break;
	  case '0':
	  case 'k':
	  case 'K': {
	    GET_CLASS(d->character) = CLASS_DRAGONW;
	    init_char(d->character);
	    /* create an entry in the file */
	    d->pos = create_entry(GET_NAME(d->character));
	    save_char(d->character, NOWHERE);
	    SEND_TO_Q(motd, d);
	    SEND_TO_Q("\n\r*** PRESS RETURN: ", d);
	    STATE(d) = CON_RMOTD;
	  } break;
            default : {
	      SEND_TO_Q("\n\rThat's not a class.\n\rClass:", d);
	      STATE(d) = CON_QCLASS;
            } break;
            
	  } /* End Switch */
	if (STATE(d) != CON_QCLASS) {
	  sprintf(buf, "%s has entered [1;31mCrimson[1;33m (NEW CHARACTER)[0;37m.", GET_NAME(d->character));
	  do_sys(buf,1);
	  sprintf(buf+strlen(buf), "[%s]", d->host);
	  log(buf);
	}
      } break;

      case CON_RMOTD:      /* read CR after printing motd   */
         SEND_TO_Q(MENU, d);
         STATE(d) = CON_SLCT;
      break;

      case CON_SLCT:    /* get selection from main menu  */
	 SEND_TO_Q(echo_on,d);
         /* skip whitespaces */
         for (; isspace(*arg); arg++);
         switch (*arg)
         {
            case '0':
	       write_to_descriptor(d->descriptor, ciao);
	       close_socket(d);
            break;

            case '1':
               reset_char(d->character);
               load_char_objs(d->character);
               send_to_char(WELC_MESSG, d->character);
               d->character->next = character_list;
               character_list = d->character;
               if (d->character->in_room == NOWHERE)
                  char_to_room(d->character, real_room(3000));
               else {
                  if (real_room(d->character->in_room) > -1)
                     char_to_room(d->character, real_room(d->character->in_room));
                  else
               char_to_room(d->character, real_room(3000));
               }

               ansi_act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM,CLR_ACTION);
               STATE(d) = CON_PLYNG;
               if (!GET_LEVEL(d->character)) {
                  do_start(d->character);
		}
               do_look(d->character, "",15);
               d->prompt_mode = 1;
            break;
 
            case '2':
               SEND_TO_Q("Enter a text you'd like others to see when they look at you.\n\r", d);
               SEND_TO_Q("Terminate with an '@' on an otherwise blank line.\n\r", d);
               if (d->character->player.description)
               {
                  SEND_TO_Q("Old description :\n\r", d);
                  SEND_TO_Q(d->character->player.description, d);
                  d->character->player.description = str_free(d->character->player.description);
               }
               d->str = 
                  &d->character->player.description;
               d->max_str = MAX_DESC;
               STATE(d) = CON_EXDSCR;
            break;

            case '3':
               SEND_TO_Q(story, d);
               SEND_TO_Q("\n\r*** PRESS RETURN: ", d);
               STATE(d) = CON_RMOTD;
            break;
            case '4':
               SEND_TO_Q("Enter a new password: ", d);
	       SEND_TO_Q(echo_off,d);
               STATE(d) = CON_PWDNEW;
            break;
            default:
               SEND_TO_Q("Wrong option.\n\r", d);
               SEND_TO_Q(MENU, d);
            break;
         }
      break;
      case CON_PWDNEW:
         /* skip whitespaces */
         for (; isspace(*arg); arg++);

         if (!*arg || strlen(arg) > 10)
         {
            SEND_TO_Q("Illegal password.\n\r", d);
            SEND_TO_Q("Password: ", d);
            return;
         }

         strncpy(d->pwd, (char *)crypt(arg, d->character->player.name), 10);
         *(d->pwd + 10) = '\0';

         SEND_TO_Q("Please retype password: ", d);

         STATE(d) = CON_PWDNCNF;
      break;
      case CON_PWDNCNF:
         /* skip whitespaces */
         for (; isspace(*arg); arg++);

         if (strncmp((char *)crypt(arg, d->pwd), d->pwd, 10))
         {
            SEND_TO_Q("Passwords don't match.\n\r", d);
            SEND_TO_Q("Retype password: ", d);
            STATE(d) = CON_PWDNEW;
            return;
         }
	 SEND_TO_Q(echo_on,d);
         SEND_TO_Q(
            "\n\rDone. You must enter the game to make the change final\n\r",
               d);
         SEND_TO_Q(MENU, d);
         STATE(d) = CON_SLCT;
      break;
      default:
         log("Nanny: illegal state of con'ness");
         abort();
      break;
   }
}


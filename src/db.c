  /**************************************************************************
  *  file: db.c , Database module.                          Part of DIKUMUD *
  *  Usage: Loading/Saving chars, booting world, resetting etc.             *
  *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
  ***************************************************************************/
  
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <ctype.h>
  #include <time.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  
  #include "structs.h"
  #include "utility.h"
  #include "db.h"
  #include "comm.h"
  #include "handler.h"
  #include "limits.h"
  #include "constants.h"
  #include "ban.h"
  #include "weather.h"
  #include "reception.h"
  #include "modify.h"
  #include "spec_assign.h"
  #include "spells.h"
  #include "interpreter.h"
  #include "fight.h"
  #include "act.h"

  /* note for some inexplicable reason allocate_mob dont work
     nor allocate_obj or some combination of them and allocate_world
     because unless the INIT values are higher than the total number in
     the game it *MIGHT* give a segmentation fault when the zone boots!
     (and *DONT* ask me why I just code here) 'Cryogen
   */
/* soon to be moved to config.sys */
/* see also in comm.c for a few others */
  #define INIT_CARVED_WORLD 4200 /* put in zones.tbl file l8r*/
  #define INIT_CARVED_MOB   1200 /* have mud remember what it was last */
  #define INIT_CARVED_OBJ   1400 /* get zcmds for each zone too */
  #define DIE_ON_ERROR         0 /* change to zero to disable this */
  #define BACKUP_ZONES         1  
  #define SHOW_ZONE_BOOT       0  

  /**************************************************************************
  *  declarations of most of the 'global' variables                         *
  ************************************************************************ */
  long up_time; /* this ought to be interesting */
  
  struct room_data *world;              /* dyn alloc'ed array of rooms     */
  int top_of_world = 0;                 /* ref to the top element of world */
  int carved_world = 0;                 /* number of blocks currently chunked */

  struct zone_data *zone_table;         /* table of reset data             */
  int top_of_zone_table = 0;

  struct char_data *character_list = 0; /* global l-list of chars          */
  struct obj_data  *object_list = 0;    /* the global linked list of obj's */  
  struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */
  struct player_index_element *player_table = 0; /* index to player file   */
  int top_of_p_table = 0;               /* ref to top of table             */
  int top_of_p_file = 0;
  
  char *credits;      /* the Credits List                */
  char *news;         /* the news                        */
  char *motd;         /* the messages of today           */
  char *help;         /* the main help page              */
  char *info;         /* the info text                   */
  char *wizlist;      /* the wizlist                     */
  char *story;        /* the story off main menu         */
  char *greeting;     /* well duh-uh!         */
  char *ciao;         /* good bye screen */   

  FILE *mob_f;                          /* file containing mob prototypes  */
  FILE *obj_f;                          /* obj prototypes                  */
  struct index_data *mob_index;         /* index table for mobile file     */
  struct index_data *obj_index;         /* index table for object file     */

  int mob_f_zone = -1;                   /* zone of the open file */
  int obj_f_zone = -1;                   /* needed for new file system */

  int carved_mob = 0;                 /* number of blocks currently chunked */
  int carved_obj = 0;                 /* number of blocks currently chunked */
  
  int top_of_mobt = 0;                  /* top of mobile index table       */
  int top_of_objt = 0;                  /* top of object index table       */

  FILE *help_fl;                        /* file for help texts (HELP <kwd>)*/
  int top_of_helpt;                     /* top of help index table         */
  struct help_index_element *help_index = 0;
  
  /* local procedures */
  void boot_zones(void);
  void boot_mob(void);
  void boot_obj(void);
  void setup_dir(FILE *fl, int room, int dir);
  void boot_world(void);
  struct index_data *generate_indices(FILE *fl, int *top);
  void build_player_index(void);
  int is_empty(int zone_nr);
  void renum_world(void);
  void renum_zone_table(void);
  void reset_time(void);
  
  
  /*************************************************************************
  *  routines for booting the system                                       *
  *********************************************************************** */
  
  
  /* body of the booting system */
  void boot_db(void)
  {
     int i;
     extern int no_specials;
  
     fprintf(stderr," Resetting game time to:");
     reset_time();
 
     /* Using new file2string old was:   */ 
     /* file_to_string(INFO_FILE, info); */

     fprintf(stderr,"Reading: news,");
     news = file2string(NEWS_FILE);
     fprintf(stderr," credits,");
     credits = file2string(CREDITS_FILE);
     fprintf(stderr," motd,");
     motd = file2string(MOTD_FILE);
     fprintf(stderr," help,");
     help = file2string(HELP_PAGE_FILE);
     fprintf(stderr," info,");
     info = file2string(INFO_FILE); 
     fprintf(stderr," wizlist,");
     wizlist = file2string(WIZLIST_FILE);
     fprintf(stderr," ciao,");
     ciao = file2string(CIAO_FILE);
     fprintf(stderr," story,");       
     story = file2string("story");
     fprintf(stderr," greeting,");
     greeting = file2string("greeting");
     fprintf(stderr," helptbl.\n");
     if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
       log("   Could not open help file.");
     else 
       help_index = build_help_index(help_fl, &top_of_helpt);
  
     fprintf(stderr,"Loading: zone table,");
     boot_zones();
     fprintf(stderr," rooms,");
     boot_world();
     fprintf(stderr," player_index,");
     build_player_index();
     fprintf(stderr," mob_index,");
     boot_mob();
     fprintf(stderr," obj_index,");
     boot_obj();
     fprintf(stderr," banlist.\n");
     load_banned();
     fprintf(stderr,"Renumbering: rooms,");
     renum_world();
     fprintf(stderr," zone table.\n");
     renum_zone_table();
  
     fprintf(stderr,"Loading messages: fight,");
     load_messages();
     fprintf(stderr," social\n");
     boot_social_messages();

/* * disabled and I have no intention of ever re-enabling *
     fprintf(stderr," pose.\n");
     boot_pose_messages();
*/ 
     fprintf(stderr,"Assigning function pointers:");
     if (!no_specials)
     {
       fprintf(stderr," Mobiles,");
       assign_mobiles();
       fprintf(stderr," Objects,");
       assign_objects();
       fprintf(stderr," Rooms,");
       assign_rooms();
     }
     fprintf(stderr," Commands,");     
     assign_command_pointers();
     fprintf(stderr," Spells.\n");
     assign_spell_pointers();
  
     log("Updating characters with saved items:");
     update_obj_file();
  
     log("Performing boot-time reset of all zones:");
     for (i = 0; i <= top_of_zone_table; i++)
     {
       if (SHOW_ZONE_BOOT) {
         zone_table[i].bottom = (i ? (zone_table[i - 1].top + 1) : 0); 
         fprintf(stderr, "\n%s (%d-%d) #[%d]",
           zone_table[i].filename,
           zone_table[i].bottom,
           zone_table[i].top,
           zone_table[i].real_top - zone_table[i].real_bottom +1);
         fprintf(stderr, " r(%d-%d)",
           zone_table[i].real_bottom,
           zone_table[i].real_top);
         fprintf(stderr, " m(%d-%d)",
           zone_table[i].bot_of_mobt,
           zone_table[i].top_of_mobt);
         fprintf(stderr, " o(%d-%d)",
           zone_table[i].bot_of_objt,
           zone_table[i].top_of_objt);
       }
       reset_zone(i);
     }
     reset_q.head = reset_q.tail = 0;
     fprintf(stderr, "Boot_DB done - Totals: Zones=%d Rooms=%d Mobs=%d Obj's=%d\n\n", 
       top_of_zone_table+1, top_of_world+1, top_of_mobt+1, top_of_objt+1);
  }
  
  
  /* reset the time in the game from file */
  void reset_time(void)
  {
     char buf[MAX_STRING_LENGTH];
     long beginning_of_time = 650336715;
     struct time_info_data mud_time_passed(time_t t2, time_t t1);
  
     time_info = mud_time_passed(time(0), beginning_of_time);
     up_time = time(0);
     switch(time_info.hours){
       case 0:  case 1:  case 2:  case 3:  case 4:{
         weather_info.sunlight = SUN_DARK;
         break;
       }
       case 5 :{
         weather_info.sunlight = SUN_RISE;
         break;
       }
       case 6:  case 7:  case 8:  case 9:  case 10: case 11:
       case 12: case 13: case 14: case 15: case 16: case 17:
       case 18: case 19:
       case 20:{
         weather_info.sunlight = SUN_LIGHT;
         break;
       }
       case 21:{
         weather_info.sunlight = SUN_SET;
         break;
       }
       case 22: case 23: default:{
         weather_info.sunlight = SUN_DARK;
         break;
       }
     }
  
     sprintf(buf," %dH %dD %dM %dY.\n",
         time_info.hours, time_info.day,
         time_info.month, time_info.year);
     fprintf(stderr, "%s", buf);
  
     weather_info.pressure = 960;
     if ((time_info.month>=7)&&(time_info.month<=12))
       weather_info.pressure += dice(1,50);
     else
       weather_info.pressure += dice(1,80);
  
     weather_info.change = 0;
  
     if (weather_info.pressure<=980)
       weather_info.sky = SKY_LIGHTNING;
     else if (weather_info.pressure<=1000)
       weather_info.sky = SKY_RAINING;
     else if (weather_info.pressure<=1020)
       weather_info.sky = SKY_CLOUDY;
     else
       weather_info.sky = SKY_CLOUDLESS;
  }
  
  /* for now I'm going to hard code the
   * first malloc to the actual number of rooms.. I'll have it save
   * the info in the zones.tbl file sooner or later tho.. 
   */
  void allocate_room(int new_top) {
    if (new_top < carved_world)
      return;
    if (new_top > 0) {
      carved_world += 250;
    if (!(world = (struct room_data *) realloc(world, (carved_world + 2) * 
        sizeof(struct room_data)))) {
        perror("realloc room");
        exit(1);
      }
    } else {
      carved_world = INIT_CARVED_WORLD;
      CREATE(world, struct room_data, carved_world + 2);
    }
  }
  void allocate_zcmd(int zone, int new_top) {
  /* this one is different from the rest */
  /* cmd pointer must be inited to NULL elsewhere */
    if (new_top % 25 != 1) /* do 25 at a time */
      return;

    if(new_top==1){ /* some machines dont like realloc null */
       if (!(zone_table[zone].cmd = (struct reset_com *) malloc(25*sizeof(struct reset_com)) )){ 
          perror("realloc zcmd");
          exit(1);
       }
    }
    else if (!(zone_table[zone].cmd = (struct reset_com *) realloc(zone_table[zone].cmd, ((new_top/25+1)*25) * 
        sizeof(struct reset_com)))) {
        perror("realloc zcmd");
        exit(1);
    }
  }
  void allocate_mob(int new_top) { /*for allocating the index */
    if (new_top < carved_mob)
      return;
    if (new_top > 0) {
      carved_mob += 25;
      if (!(mob_index = (struct index_data *) 
         realloc(mob_index, (carved_mob + 2) * 
         sizeof(struct index_data)))) {
        perror("realloc mob");
        exit(1);
      }
      fprintf(stderr, "\nalloc_mob%d (better increase INIT_CARVED_MOB!", carved_mob);
    } else {
      carved_mob = INIT_CARVED_MOB;
      CREATE(mob_index, struct index_data, carved_mob + 2);
    }
  }
  void allocate_obj(int new_top) {
    if (new_top < carved_obj)
      return;
    if (new_top > 0) {
      carved_obj += 25;
      if (!(obj_index = (struct index_data *) 
         realloc(obj_index, (carved_obj + 2) * 
         sizeof(struct index_data)))) {
        perror("realloc obj");
        exit(1);
      }
      fprintf(stderr, "\nalloc_obj%d (better increase INIT_CARVED_OBJ!", carved_obj);
    } else {
      carved_obj = INIT_CARVED_OBJ;
      CREATE(obj_index, struct index_data, carved_obj + 2);
    }
  }
  

  /* generate index table for the player file */
  void build_player_index(void)
  {
     int nr = -1, i;
     struct char_file_u dummy;
     FILE *fl;
  
     if (!(fl = fopen(PLAYER_FILE, "rb+")))
     {
      perror("build player index");
      exit(0);
     }
  
     for (; !feof(fl);)
     {
       fread(&dummy, sizeof(struct char_file_u), 1, fl);
       if (!feof(fl)){ 
           /* Create new entry in the list */
           if (nr == -1) {
             CREATE(player_table, struct player_index_element, 25);
             nr = 0;
           } else {
             nr++;
             if (nr%25==0) {
               if (!(player_table = (struct player_index_element *)
                 realloc(player_table, ((nr+25) *
                 sizeof(struct player_index_element) ))))
               {
                 perror("generate index");
                 exit(0);
               }
             }
           }
           player_table[nr].nr = nr;
           CREATE(player_table[nr].name, char,
           strlen(dummy.name) + 1);
           for (i = 0; ( *(player_table[nr].name + i) = 
             LOWER(*(dummy.name + i)) ); i++);
       }
     }
     fclose(fl);
     top_of_p_table = nr;
     top_of_p_file = top_of_p_table;
  }
 
#define NEW_ZONEOF
#ifndef NEW_ZONEOF    
  /* Finds which zone a virtual number is in */
  /* Somebody should recode this as a binary search someday
     (probably me) */
  int zone_of(int virtual) {
     int zone;
  
     for(zone = 0; 
       ((zone<=top_of_zone_table)&&(virtual>zone_table[zone].top));
        zone++);
     if ((zone > top_of_zone_table)||(virtual < zone_table[zone].bottom)){
        fprintf(stderr, "zone_of: zone containing virtual#%d does not exist\n", virtual);
        return(-1);
     } else
        return(zone);   
  }
#endif
#ifdef NEW_ZONEOF    
int zone_of(int virtual) {
  int top, mid, bot;
 
  bot = 0;
  top = top_of_zone_table;
 
  /* if number is way out choke right off the bat */ 
  if ( (virtual < zone_table[bot].bottom)
     ||(virtual > zone_table[top].top)) {
    fprintf(stderr, "zone_of: zone containing virtual#%d does not exist\n", virtual);
    return(-1);
  }

  while (top > bot) {
    mid = (bot+top) /2;
    if (virtual < zone_table[mid].bottom) 
      top = mid-1;
    else if (virtual > zone_table[mid].top) 
      bot = mid+1;
    else { /* this must be it */
      bot = mid;
      top = mid;
    }
  }

  /* if number fell in a crack between zones detect it here */
  if ( (virtual < zone_table[bot].bottom)
     ||(virtual > zone_table[top].top)) {
    fprintf(stderr, "zone_of: zone containing virtual#%d does not exist\n", virtual);
    return(-1);
  }
  return (top);
}
#endif

  /* generate index table for object or monster file */
    struct index_data *generate_indices(FILE *fl, int *top) {
    int i = 0;
    struct index_data *index;
    char buf[82];
  
    rewind(fl);
    for (;;) {
      if (fgets(buf, 81, fl)) {
        if (*buf == '#') { /* allocate new cell */
          if (!i) {                            
            CREATE(index, struct index_data, 1);
          } else
            if (!(index = (struct index_data*) realloc(index, 
                  (i + 1) * sizeof(struct index_data)))) {
              perror("load indices");
              exit(0);
            }
          sscanf(buf, "#%d", &index[i].virtual);
          index[i].pos = ftell(fl);
          index[i].name = fread_string(fl);
          index[i].number = 0;
          index[i].func = 0;
          index[i].next = i+1;
          i++;
        } else 
          if (*buf == '$')    /* EOF */
             break;
      } else {
        perror("generate indices");
        exit(0);
      }
    }
    *top = i - 2; /* why -2 ? will lose last mob/obj?!?*/
    return(index);
  }
  /* generate index table for object or monster file */
  void boot_mob() {
    int  zone, i=-1, last=-1, error = 0;
    char buf[82], msg[128], *tmpchar;
    
    for (zone = 0; zone<=top_of_zone_table; zone++) {
      sprintf(buf, "areas/%s.mob", zone_table[zone].filename);
      if (!(mob_f = fopen(buf, "r"))){
        fprintf(stderr, "boot_mob: could not open %s.mob\n",
          zone_table[zone].filename);
        perror("fopen:");
        exit(0);
      }
      rewind(mob_f);
      for (;;) {
        if (fgets(buf, 81, mob_f)) {
          if (*buf == '#') { /* allocate new cell */
            i++;
            allocate_mob(i);
            if (!sscanf(buf, "#%d", &mob_index[i].virtual)) {
              sprintf(msg, "Arg no virtual after #%d in %s.mob", 
                mob_index[i-1].virtual, zone_table[zone].filename);
              log(msg);
              exit(1);
            }
	    if (mob_index[i].virtual > zone_table[zone].top
	     || mob_index[i].virtual < zone_table[zone].bottom) {
	      fprintf(stderr, "%s.mob: mob#%d outside the range (%d - %d)\n\r",
		zone_table[zone].filename,
		mob_index[i].virtual,
		zone_table[zone].bottom,
		zone_table[zone].top);
		error = 1;
	    }
            if (mob_index[i].virtual <= last) {
	      fprintf(stderr, "%s.mob: mob#%d <= to previous mob#\n\r",
		zone_table[zone].filename,
		mob_index[i].virtual);
		error = 1;
	    }
            last = MAXV(last, mob_index[i].virtual);
            if (zone_table[zone].bot_of_mobt == -1)
              zone_table[zone].bot_of_mobt = i;
            if (i > zone_table[zone].top_of_mobt)
              zone_table[zone].top_of_mobt = i;
            mob_index[i].pos = ftell(mob_f);
            mob_index[i].name = fread_string(mob_f);
	    if (strncmp(mob_index[i].name, "mob ", 4)) { /* make sure obj is a keyword */
		tmpchar = (char *) malloc(strlen(mob_index[i].name)+5);
		strcpy(tmpchar, "mob ");
		strcpy(tmpchar+4, mob_index[i].name);
		free(mob_index[i].name);
		mob_index[i].name = tmpchar;
            }
            mob_index[i].zone = zone;
            mob_index[i].number = 0;
            mob_index[i].func = 0;
            mob_index[i].next = i+1;
          } else if (*buf == '$')    /* EOF */
            break;
        } else {
          perror("boot_mob: what the hell was that? unexpected EOF");
          if (i>=0)
	    sprintf(buf, "%s.mob is corrupt after mob#%d!?!", zone_table[zone].filename, mob_index[i].virtual);
          else 
	    sprintf(buf, "%s.mob is corrupt!?!", zone_table[zone].filename);
          log(buf);
          exit(1);
        }
      }
      top_of_mobt = i;
      fclose(mob_f);
    } /* for */
    if (error && DIE_ON_ERROR) {
      log("One or more fatal errors occurred; please fix and reboot the server");
      exit(1);
    }
  }
 
  /* generate index table for object or monster file */
  void boot_obj() {
    int  zone, i=-1, last=-1, error=0;
    char buf[82], msg[128], *tmpchar;
  
    
    for (zone = 0; zone<=top_of_zone_table; zone++) {
      sprintf(buf, "areas/%s.obj", zone_table[zone].filename);
      if (!(obj_f = fopen(buf, "r"))){
        fprintf(stderr, "boot_obj: could not open %s.obj\n",
          zone_table[zone].filename);
        perror("fopen:");
        exit(0);
      }
      rewind(obj_f);
      for (;;) {
        if (fgets(buf, 81, obj_f)) {
          if (*buf == '#') { /* allocate new cell */
            i++;
            allocate_obj(i);
            if (!sscanf(buf, "#%d", &obj_index[i].virtual)){
              sprintf(msg, "Arg no virtual after #%d in %s.obj", 
                obj_index[i-1].virtual, zone_table[zone].filename);
              log(msg);
              exit(1);
            }
	    if (obj_index[i].virtual > zone_table[zone].top
	     || obj_index[i].virtual < zone_table[zone].bottom) {
	      fprintf(stderr, "%s.obj: obj#%d outside the range (%d - %d)\n\r",
		zone_table[zone].filename,
		obj_index[i].virtual,
		zone_table[zone].bottom,
		zone_table[zone].top);
		error = 1;
	    }
            if (obj_index[i].virtual <= last) {
	      fprintf(stderr, "%s.obj: obj#%d <= to previous obj#\n\r",
		zone_table[zone].filename,
		obj_index[i].virtual);
		error = 1;
	    }
            last = MAXV(last, obj_index[i].virtual);
            if (zone_table[zone].bot_of_objt == -1)
              zone_table[zone].bot_of_objt = i;
            if (i > zone_table[zone].top_of_objt)
              zone_table[zone].top_of_objt = i;
            obj_index[i].pos  = ftell(obj_f);
            obj_index[i].name = fread_string(obj_f);
	    if (strncmp(obj_index[i].name, "obj ", 4)) { /* make sure obj is a keyword */
		tmpchar = (char *) malloc(strlen(obj_index[i].name)+5);
		strcpy(tmpchar, "obj ");
		strcpy(tmpchar+4, obj_index[i].name);
		free(obj_index[i].name);
		obj_index[i].name = tmpchar;
            }
            obj_index[i].zone = zone;
            obj_index[i].number = 0;
            obj_index[i].func = 0;
            obj_index[i].next = i+1;
          } else if (*buf == '$')    /* EOF */
            break;
        } else {
          perror("boot_obj: what the hell was that? unexpected EOF");
          if (i>=0)
	    sprintf(buf, "%s.obj is corrupt after obj#%d!?!", zone_table[zone].filename, obj_index[i].virtual);
          else 
	    sprintf(buf, "%s.obj is corrupt!?!", zone_table[zone].filename);
          log(buf);
          exit(0);
        }
      }
      top_of_objt = i; 
      fclose(obj_f);
    } /* for loop */
    if (error && DIE_ON_ERROR) {
      log("One or more fatal errors occurred; please fix and reboot the server");
      exit(1);
    }
  } /* boot_obj() */

  /* load the rooms */
  void boot_world(void)
  {
    FILE *fl;
    int room_nr = -1, zone, virtual_nr, tmp;
    char *temp, chk[100];
    struct extra_descr_data *new_descr;

    for (zone = 0; zone<=top_of_zone_table; zone++) {
      sprintf(chk, "areas/%s.wld", zone_table[zone].filename);
      if (!(fl = fopen(chk, "r"))){
        fprintf(stderr, "boot_world: could not open %s.wld\n",
          zone_table[zone].filename);
        perror("fopen:");
        exit(0);
      }
      while(1) {
        tmp=fscanf(fl, " #%d\n", &virtual_nr);
        temp = fread_string(fl);
        if (*temp == '$'){
          free(temp);
          break;
        } else if (!tmp)
          fprintf(stderr,"boot_world: no virtual # found (%s.wld is corrupt)",
            zone_table[zone].filename);
        room_nr++;
        allocate_room(room_nr);
        world[room_nr].number = virtual_nr;
        world[room_nr].name = temp;
        world[room_nr].description = fread_string(fl);
        world[room_nr].next = room_nr + 1; /* start list sorted */
        if (top_of_zone_table >= 0){
          fscanf(fl, " %*d ");
          /* OBS: Assumes ordering of input rooms */
          if((world[room_nr].number > zone_table[zone].top) ||
             (world[room_nr].number < zone_table[zone].bottom)){
            fprintf(stderr, "Boot_world: Room %d is outside zone %s (skipping rest).\n",
            virtual_nr, zone_table[zone].filename);
            exit(0); /* until zone reboots are bulletproof */
            room_nr -= 1;
            break; 
          }
          world[room_nr].zone = zone;
          if (zone_table[zone].real_bottom == -1)
            zone_table[zone].real_bottom = room_nr;
          if (room_nr > zone_table[zone].real_top)
            zone_table[zone].real_top = room_nr;
        }
        if (!fscanf(fl, " %d ", &tmp))
          fprintf(stderr,"boot_world: no room flags found (%s.wld is corrupt)",
            zone_table[zone].filename);
        world[room_nr].room_flags = tmp;
        if (!fscanf(fl, " %d ", &tmp))
          fprintf(stderr,"boot_world: no sector type found (%s.wld is corrupt)",
            zone_table[zone].filename);
        world[room_nr].sector_type = tmp;
        world[room_nr].funct = 0;
        world[room_nr].contents = 0;
        world[room_nr].people = 0;
        world[room_nr].light = 0; /* Zero light sources */
        for (tmp = 0; tmp <= 5; tmp++)
          world[room_nr].dir_option[tmp] = 0;
        world[room_nr].ex_description = 0;
        for (;;){
          fscanf(fl, " %s \n", chk);
          if (*chk == 'D')  /* direction field */
            setup_dir(fl, room_nr, atoi(chk + 1));
          else if (*chk == 'E'){  /* extra description field */
            CREATE(new_descr, struct extra_descr_data, 1);
            new_descr->keyword = fread_string(fl);
            new_descr->description = fread_string(fl);
            new_descr->next = world[room_nr].ex_description;
            world[room_nr].ex_description = new_descr;
          }
          else if (*chk == 'S')    /* end of current room */
            break;
        }
      }
      fclose(fl);
    } /* for loop to get all zone files*/
    top_of_world = room_nr; 
  } 

  
  /* read direction data */
  void setup_dir(FILE *fl, int room, int dir)
  {
     int tmp;
  
     CREATE(world[room].dir_option[dir], 
       struct room_direction_data, 1);
  
     world[room].dir_option[dir]->general_description =
       fread_string(fl);
     world[room].dir_option[dir]->keyword = fread_string(fl);
  
     fscanf(fl, " %d ", &tmp);
/* I just use the flags added up....
     if (tmp == 1)
       world[room].dir_option[dir]->exit_info = EX_ISDOOR;
     else if (tmp == 2)
       world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
     else
       world[room].dir_option[dir]->exit_info = 0;
 */
     world[room].dir_option[dir]->exit_info = tmp;
   
     fscanf(fl, " %d ", &tmp);
     world[room].dir_option[dir]->key = tmp;
  
     fscanf(fl, " %d ", &tmp);
     world[room].dir_option[dir]->to_room = tmp;
  }
  
  
  
  
  void renum_world(void)
  {
     register int room, door;
  
     for (room = 0; room <= top_of_world; room++)
       for (door = 0; door <= 5; door++)
         if (world[room].dir_option[door])
           if (world[room].dir_option[door]->to_room != NOWHERE)
             world[room].dir_option[door]->to_room =
               real_room(world[room].dir_option[door]->to_room);
  }
  
  
  void renum_zone_table(void)
  {
     int zone, comm;
  
     for (zone = 0; zone <= top_of_zone_table; zone++)
      for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++)
           switch(zone_table[zone].cmd[comm].command)
           {
            case 'M':
             zone_table[zone].cmd[comm].arg1 =
                  real_mobile(zone_table[zone].cmd[comm].arg1);
             zone_table[zone].cmd[comm].arg3 = 
                  real_room(zone_table[zone].cmd[comm].arg3);
            break;
            case 'O':
             zone_table[zone].cmd[comm].arg1 = 
                  real_object(zone_table[zone].cmd[comm].arg1);
             if (zone_table[zone].cmd[comm].arg3 != NOWHERE)
                  zone_table[zone].cmd[comm].arg3 =
                    real_room(zone_table[zone].cmd[comm].arg3);
            break;
            case 'G':
             zone_table[zone].cmd[comm].arg1 =
                  real_object(zone_table[zone].cmd[comm].arg1);
            break;
            case 'E':
             zone_table[zone].cmd[comm].arg1 =
                  real_object(zone_table[zone].cmd[comm].arg1);
            break;
            case 'P':
             zone_table[zone].cmd[comm].arg1 =
                  real_object(zone_table[zone].cmd[comm].arg1);
             zone_table[zone].cmd[comm].arg3 =
                  real_object(zone_table[zone].cmd[comm].arg3);
            break;                        
            case 'D':
             zone_table[zone].cmd[comm].arg1 =
                  real_room(zone_table[zone].cmd[comm].arg1);
            break;
           }
  }
  
  /* load the zone table and command tables */
  void boot_zones(void)
  {
     FILE *fl,*tbl;
     int zon = 0, cmd_no = 0, expand, tmp;
     char *check, buf[256], temp1[256];
  
     if (!(tbl = fopen("zones.tbl", "r"))){
       perror("boot_zones(zones.tbl)");
       exit(0);
     }
     while(1) { /* Read in the Zone Table */
       /* fgets(buf,255,tbl); */
       for(fscanf(tbl, " %s", temp1);
           *temp1 == '*' || *temp1 == '#';
           fscanf(tbl, " %s", temp1));
       if (*temp1 == '$')
           break;         /* end of file */
  
       /* alloc a new zone */
       if (!zon){
         CREATE(zone_table, struct zone_data, 1);
       } else
         if (!(zone_table = (struct zone_data *) 
           realloc(zone_table,(zon + 1) * sizeof(struct zone_data))))
         {
           perror("boot_zones realloc");
           exit(0);
         }
       fscanf(tbl," %d %d ",  
         &zone_table[zon].bottom,
         &zone_table[zon].top );
       zone_table[zon].filename=str_alloc(temp1);
       zone_table[zon].lord    =fread_string(tbl);
       strcpy(zone_table[zon].filename, temp1);
      
       zone_table[zon].dirty_wld = 0; /* start all zones "clean"  */
       zone_table[zon].dirty_mob = 0; 
       zone_table[zon].dirty_obj = 0;
       zone_table[zon].dirty_zon = 0;
       zone_table[zon].real_bottom = -1; /* nothing read in yet */
       zone_table[zon].real_top = -1;
       zone_table[zon].top_of_mobt = -1;
       zone_table[zon].bot_of_mobt = -1;
       zone_table[zon].top_of_objt = -1;
       zone_table[zon].bot_of_objt = -1;
       zone_table[zon].cmd = 0;
       zon++;
     }
     fclose(tbl);
     top_of_zone_table = --zon;

     for (zon = 0; zon <= top_of_zone_table; zon++) {
       sprintf(buf,"areas/%s.zon",zone_table[zon].filename);
       if (!(fl = fopen(buf, "r"))){
         fprintf(stderr, "ARG! zone %s.zon not found\n", zone_table[zon].filename);
         perror("boot_zones");
         exit(0);
       }
       
       fscanf(fl, " #%d\n", &zone_table[zon].flags);
/* DEBUG */
if (zone_table[zon].lord)
  zone_table[zon].flags = ZONE_TESTING | ZONE_NOENTER;
else
  zone_table[zon].flags = 0;
       check = fread_string(fl);
       zone_table[zon].name = check;
       fscanf(fl, " %d ", &zone_table[zon].lifespan); /* throw out old top */
       fscanf(fl, " %d ", &zone_table[zon].lifespan);
       fscanf(fl, " %d ", &zone_table[zon].reset_mode);
      
       /* read the command table */
       cmd_no = 0;
       for (expand = 1;;) {
         if (expand)
         allocate_zcmd(zon, cmd_no+1);
         expand = 1;
         fscanf(fl, " "); /* skip blanks */
         fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);
           
         if (zone_table[zon].cmd[cmd_no].command == 'S')
            break;
         if (zone_table[zon].cmd[cmd_no].command == '*')
         {
           expand = 0; /* note online edits will strip out comments */
           fgets(buf, 80, fl); /* skip command */
           continue;
         }
         fscanf(fl, " %d %d %d", 
           &tmp,
           &zone_table[zon].cmd[cmd_no].arg1,
           &zone_table[zon].cmd[cmd_no].arg2);
         zone_table[zon].cmd[cmd_no].if_flag = tmp;

         if (zone_table[zon].cmd[cmd_no].command == 'M' ||
           zone_table[zon].cmd[cmd_no].command == 'O' ||
           zone_table[zon].cmd[cmd_no].command == 'E' ||
           zone_table[zon].cmd[cmd_no].command == 'P' ||
           zone_table[zon].cmd[cmd_no].command == 'D')
           fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);
         else
           zone_table[zon].cmd[cmd_no].arg3 = -1;

         fgets(buf, 80, fl); /* read comment (if any) */
         if (expand) cmd_no++;
      }
      zone_table[zon].reset_num = cmd_no;
      fclose(fl);
    } /* read all the zone files loop */
  }
  
  
  /*************************************************************************
  *  procedures for resetting, both play-time and boot-time       *
  *********************************************************************** */
  
   /* read a mobile from MOB_FILE */
  struct char_data *read_mobile(int nr, int type)
  {
     int i;
     long tmp, tmp2, tmp3;
     struct char_data *mob, *proto_mob;
     char buf[100];
     char letter;

     i = nr;
     if ((type == VIRTUAL) || (type == PROTOTYPE)) { /* add to db.h */
       nr = real_mobile(nr);
       if ((nr < 0)||(nr > top_of_mobt)){
          sprintf(buf, "Mobile(R) %d does not exist in database.", i);
          log(buf);
          return(0);
       }
     }
     if ( (proto_mob = (struct char_data *) mob_index[nr].prototype) ){
       if (type == PROTOTYPE) {
         return(proto_mob); /* if prototype allready loaded just return */
       }
       CREATE(mob, struct char_data, 1);
       clear_char(mob);
    
       /***** String data *** */
       mob->player.name=str_alloc(proto_mob->player.name);
       mob->player.short_descr=str_alloc(proto_mob->player.short_descr);
       mob->player.long_descr=str_alloc(proto_mob->player.long_descr);
       mob->player.description=str_alloc(proto_mob->player.description);
       mob->player.title = 0;

       /* *** Numeric data *** */
       mob->specials.act = proto_mob->specials.act;
       mob->specials.affected_by = proto_mob->specials.affected_by;
       mob->specials.alignment = proto_mob->specials.alignment;
       mob->abilities.str   = 11;
       mob->abilities.intel = 11; 
       mob->abilities.wis   = 11;
       mob->abilities.dex   = 11;
       mob->abilities.con   = 11;
       GET_LEVEL(mob) = GET_LEVEL(proto_mob);
       mob->points.hitroll = proto_mob->points.hitroll;
       mob->points.armor = proto_mob->points.armor;
       mob->points.max_hit = dice(proto_mob->points.max_hit,
                                  proto_mob->player.weight)
                                  +proto_mob->points.hit;
       mob->points.hit = mob->points.max_hit;
       mob->points.damroll = proto_mob->points.damroll;
       mob->specials.damnodice = proto_mob->specials.damnodice;
       mob->specials.damsizedice = proto_mob->specials.damsizedice;
       mob->points.mana = proto_mob->points.mana;
       mob->points.max_mana = proto_mob->points.max_mana;
       mob->points.move = proto_mob->points.move;
       mob->points.max_move = proto_mob->points.max_move;
       mob->points.gold = proto_mob->points.gold;
       GET_EXP(mob) = GET_EXP(proto_mob);
       mob->specials.position = proto_mob->specials.position;
       mob->specials.default_pos = proto_mob->specials.default_pos;
       mob->player.sex = proto_mob->player.sex;

       /* init play values */
       mob->player.class = 0;
       mob->player.time.birth = time(0);
       mob->player.time.played  = 0;
       mob->player.time.logon  = time(0);
       mob->player.weight = 200;
       mob->player.height = 198;
       for (i = 0; i < 3; i++)
           GET_COND(mob, i) = -1;
       for (i = 0; i < 5; i++)
           mob->specials.apply_saving_throw[i] = MAXV(30-GET_LEVEL(mob), 2);
       mob->tmpabilities = mob->abilities;
       for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
         mob->equipment[i] = 0;
       mob->nr = nr;
       mob->desc = 0;
  
       /* insert in list */
       mob->next = character_list;
       character_list = mob;
       mob_index[nr].number++;
       return(mob); /* loaded mob data from prototype */
     }
     if (mob_f_zone != mob_index[nr].zone) {
       if (mob_f_zone != -1)
         fclose(mob_f);
       sprintf(buf,"areas/%s.mob",zone_table[mob_index[nr].zone].filename);
       if (!(mob_f = fopen(buf, "r"))){
         fprintf(stderr, "ARG! file %s.mob not found\n", zone_table[mob_index[nr].zone].filename);
         perror("read_mob");
         exit(0);
       }
     } /* load mob data from file */
     mob_f_zone = mob_index[nr].zone;
     fseek(mob_f, mob_index[nr].pos, 0);
     CREATE(mob, struct char_data, 1);
     clear_char(mob);
  
     /***** String data *** */
     mob->player.name = fread_string(mob_f);
     mob->player.short_descr = fread_string(mob_f);
     mob->player.long_descr = fread_string(mob_f);
     mob->player.description = fread_string(mob_f);
     mob->player.title = 0;

     /* *** Numeric data *** */
     fscanf(mob_f, "%ld ", &tmp);
     mob->specials.act = tmp;
     SET_BIT(mob->specials.act, ACT_ISNPC);
     fscanf(mob_f, " %ld ", &tmp);
     mob->specials.affected_by = tmp;
     fscanf(mob_f, " %ld ", &tmp);
     mob->specials.alignment = tmp;
     fscanf(mob_f, " %c \n", &letter);
  
     if (letter == 'S') {
       /* The new easy monsters */
       if (type != PROTOTYPE) {
         mob->abilities.str   = 11;
         mob->abilities.intel = 11; 
         mob->abilities.wis   = 11;
         mob->abilities.dex   = 11;
         mob->abilities.con   = 11;
       }
  
       fscanf(mob_f, " %ld ", &tmp);
       GET_LEVEL(mob) = tmp;
       fscanf(mob_f, " %ld ", &tmp);
       mob->points.hitroll = 30-tmp;
       fscanf(mob_f, " %ld ", &tmp);
       mob->points.armor = 10*tmp;
  
       fscanf(mob_f, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
       if (type == PROTOTYPE) {  
         mob->points.max_hit = MAXV(1,tmp);
         mob->points.hit = MAXV(0,tmp3);
         mob->player.weight = MAXV(1,tmp2);
       } else {
         mob->points.max_hit = dice(MAXV(1,tmp), MAXV(1,tmp2))+MAXV(0,tmp3);
         mob->points.hit = mob->points.max_hit;
         mob->player.weight = 200;
       }  
       fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
       mob->points.damroll = MAXV(0,tmp3);
       mob->specials.damnodice = MAXV(1,tmp);
       mob->specials.damsizedice = MAXV(1,tmp2);
  
       mob->points.mana = 10;
       mob->points.max_mana = 100;
       mob->points.move = 50;
       mob->points.max_move = 50;
  
       fscanf(mob_f, " %ld ", &tmp);
       mob->points.gold = tmp;
       fscanf(mob_f, " %ld \n", &tmp);
       GET_EXP(mob) = tmp;
       fscanf(mob_f, " %ld ", &tmp);
       mob->specials.position = MINV(POSITION_STANDING, MAXV(POSITION_SLEEPING, tmp));
       fscanf(mob_f, " %ld ", &tmp);
       mob->specials.default_pos = MINV(POSITION_STANDING, MAXV(POSITION_SLEEPING, tmp));
       fscanf(mob_f, " %ld \n", &tmp);
       mob->player.sex = tmp;
       mob->player.class = 0;
       mob->player.time.birth = time(0);
       mob->player.time.played  = 0;
       mob->player.time.logon  = time(0);
       mob->player.height = 198;
       if (type != PROTOTYPE) {
         for (i = 0; i < 3; i++)
           GET_COND(mob, i) = -1;
         for (i = 0; i < 5; i++)
           mob->specials.apply_saving_throw[i] = MAXV(30-GET_LEVEL(mob), 2);
       }
     } else { /* encountered an old style mob */ 
       fprintf(stderr, "(old style mob) Mobile(V) %d does not exist in database.\n", i);
       fprintf(stderr, "read_mob: old style mob not supported\n");
       /* not going to support old mobs */
       exit(1);
     }
     mob->desc = 0;
     mob->nr = nr;
     if (type == PROTOTYPE) {
       mob_index[nr].prototype = (char *) mob;
       mob->next = 0;
     } else {
       mob->tmpabilities = mob->abilities;
       for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
         mob->equipment[i] = 0;
       /* insert in list */
       mob->next = character_list;
       character_list = mob;
       mob_index[nr].number++;
     }
     return(mob); /* loaded mob_data from file */
  }
 
  /* read an object from OBJ_FILE */
  struct obj_data *read_object(int nr, int type)
  {
     struct obj_data *obj, *proto_obj;
     int tmp, i;
     char chk[50], buf[100];
     struct extra_descr_data *new_descr;
     struct extra_descr_data *old_descr;
  
     i = nr;
     if ((type == VIRTUAL)||(type == PROTOTYPE))
       if ((nr = real_object(nr)) < 0) {
         sprintf(buf, "Object(V) %d does not exist in database.", i);
         log(buf);
         return(0);
       }
     if ( ((nr < 0) || (nr > top_of_objt)) && (type == REAL)) {
       sprintf(buf, "Object(R) %d does not exist in database.", i);
       log(buf);
       return(0);
     }
     if ( (proto_obj = (struct obj_data *) obj_index[nr].prototype) ){
       if (type == PROTOTYPE) {
         return(proto_obj); /* if prototype allready loaded just return */
       }
       CREATE(obj, struct obj_data, 1);
       clear_object(obj);
  
       /* *** string data *** */
       obj->name=str_alloc(proto_obj->name);
       obj->short_description = str_alloc(proto_obj->short_description);
       obj->description=str_alloc(proto_obj->description);
       obj->action_description=str_alloc(proto_obj->action_description);

       /* *** numeric data *** */
       obj->obj_flags.type_flag    =proto_obj->obj_flags.type_flag;
       obj->obj_flags.extra_flags  =proto_obj->obj_flags.extra_flags;
       obj->obj_flags.wear_flags   =proto_obj->obj_flags.wear_flags;
       obj->obj_flags.value[0]     =proto_obj->obj_flags.value[0];
       obj->obj_flags.value[1]     =proto_obj->obj_flags.value[1];
       obj->obj_flags.value[2]     =proto_obj->obj_flags.value[2];
       obj->obj_flags.value[3]     =proto_obj->obj_flags.value[3];
       obj->obj_flags.weight       =proto_obj->obj_flags.weight;
       obj->obj_flags.cost         =proto_obj->obj_flags.cost;
       obj->obj_flags.cost_per_day =proto_obj->obj_flags.cost_per_day;

       /* *** extra descriptions *** */
       obj->ex_description = 0;
       old_descr = proto_obj->ex_description;
       while (old_descr){
         CREATE(new_descr, struct extra_descr_data, 1);
         new_descr->keyword=      str_alloc(old_descr->keyword);
         new_descr->description=  str_alloc(old_descr->description);
         new_descr->next =        obj->ex_description;
         obj->ex_description = new_descr;
         old_descr = old_descr->next;
       }

       for( i = 0 ; (i < MAX_OBJ_AFFECT); i++){
         obj->affected[i].location = proto_obj->affected[i].location;
         obj->affected[i].modifier = proto_obj->affected[i].modifier;
       }
  
       obj->in_room = NOWHERE;
       obj->next_content = 0;
       obj->carried_by = 0;
       obj->in_obj = 0;
       obj->contains = 0;
       obj->item_number = nr;   
       obj->next = object_list;
       object_list = obj;
       obj_index[nr].number++;
       return (obj);  
     }
     if (obj_f_zone != obj_index[nr].zone) {
       if (obj_f_zone != -1)
         fclose(obj_f);
       sprintf(buf,"areas/%s.obj",zone_table[obj_index[nr].zone].filename);
       if (!(obj_f = fopen(buf, "r"))){
         fprintf(stderr, "ARG! file %s.obj not found\n", zone_table[obj_index[nr].zone].filename);
         perror("read_obj");
         exit(0);
       }
     }
     obj_f_zone = obj_index[nr].zone;
     fseek(obj_f, obj_index[nr].pos, 0);
     CREATE(obj, struct obj_data, 1);
     clear_object(obj);
  
     /* *** string data *** */
     obj->name = fread_string(obj_f);
     obj->short_description = fread_string(obj_f);
     obj->description = fread_string(obj_f);
     obj->action_description = fread_string(obj_f);

/* the following checks for an extra /n/r at the end of the obj description */
/* fix for screwed up object descriptions */
/* can be taken out in the future */
     {
       char *the_desc;
       int  the_size;

       the_desc = obj->description;
       if (the_desc) {
         the_size = strlen(the_desc);
         if (the_size > 2) 
           if ((the_desc[the_size-2] == '\n') && (the_desc[the_size-1] == '\r'))
             the_desc[the_size-2] = 0; 
       }
     } 
/* fix for screwed up object descriptions */
/* can be taken out in the future */
 
     /* *** numeric data *** */
     fscanf(obj_f, " %d ", &tmp);
     obj->obj_flags.type_flag = tmp;
     fscanf(obj_f, " %d ", &tmp);
     obj->obj_flags.extra_flags = tmp;
     fscanf(obj_f, " %d ", &tmp);
     obj->obj_flags.wear_flags = tmp;
     fscanf(obj_f, " %d ", &tmp);
     obj->obj_flags.value[0] = tmp;
     fscanf(obj_f, " %d ", &tmp);
     obj->obj_flags.value[1] = tmp;
     fscanf(obj_f, " %d ", &tmp);
     obj->obj_flags.value[2] = tmp;
     fscanf(obj_f, " %d ", &tmp);
     obj->obj_flags.value[3] = tmp;
     fscanf(obj_f, " %d ", &tmp);
     obj->obj_flags.weight = tmp;
     fscanf(obj_f, " %d \n", &tmp);
     obj->obj_flags.cost = tmp;
     fscanf(obj_f, " %d \n", &tmp);
     obj->obj_flags.cost_per_day = tmp;
  
     /* *** extra descriptions *** */
     obj->ex_description = 0;
     while (fscanf(obj_f, " %s \n", chk), *chk == 'E'){
       CREATE(new_descr, struct extra_descr_data, 1);
       new_descr->keyword = fread_string(obj_f);
       new_descr->description = fread_string(obj_f);
       new_descr->next = obj->ex_description;
       obj->ex_description = new_descr;
     }
  
     for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++){
       fscanf(obj_f, " %d ", &tmp);
       obj->affected[i].location = tmp;
       fscanf(obj_f, " %d \n", &tmp);
       obj->affected[i].modifier = tmp;
       fscanf(obj_f, " %s \n", chk);
     }
  
     for (;(i < MAX_OBJ_AFFECT);i++){
       obj->affected[i].location = APPLY_NONE;
       obj->affected[i].modifier = 0;
     }
     obj->next_content = 0;
     obj->carried_by = 0;
     obj->in_obj = 0;
     obj->contains = 0;
     obj->item_number = nr;   
     obj->in_room = NOWHERE;
     if (type == PROTOTYPE) {
       obj_index[nr].prototype = (char *) obj;
     } else {
       obj->next = object_list;
       object_list = obj;
       obj_index[nr].number++;
     }
     return (obj);
  }
  
  
  #define ZO_DEAD  999
  /* update zone ages, queue for reset if necessary, and dequeue when possible */
  void zone_update(void)
  {
     int i;
     struct reset_q_element *update_u, *temp;
  
     /* enqueue zones */
     for (i = 0; i <= top_of_zone_table; i++){
      if (zone_table[i].age < zone_table[i].lifespan &&
           zone_table[i].reset_mode)
           (zone_table[i].age)++;
      else
           if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode)
           {
           /* enqueue zone */
  
           CREATE(update_u, struct reset_q_element, 1);
   
           update_u->zone_to_reset = i;
           update_u->next = 0;
  
           if (!reset_q.head)
            reset_q.head = reset_q.tail = update_u;
           else
           {
            reset_q.tail->next = update_u;
            reset_q.tail = update_u;
           }
  
           zone_table[i].age = ZO_DEAD;
           }
     }
  
     /* dequeue zones (if possible) and reset */
  
     for (update_u = reset_q.head; update_u; update_u = update_u->next) 
      if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
           is_empty(update_u->zone_to_reset))
      {
      reset_zone(update_u->zone_to_reset);
  
      /* dequeue */
  
      if (update_u == reset_q.head)
           reset_q.head = reset_q.head->next;
      else
      {
           for (temp = reset_q.head; temp->next != update_u;
            temp = temp->next);
  
           if (!update_u->next)
            reset_q.tail = temp;
  
           temp->next = update_u->next;
  
  
      }
  
      free(update_u);
      break;
      } 
  }
  
  
  
  /* execute the reset command table of a given zone */
  #define ZCMD zone_table[zone].cmd[cmd_no]
  void reset_zone(int zone)
  {
    int cmd_no, last_cmd = 1, help = 0;
    char buf[256];
    struct char_data *mob = 0;
    struct obj_data *obj, *obj_to;
  
    if (IS_SET(zone_table[zone].dirty_zon, SHOWN))
      return;
    for (cmd_no = 0;;cmd_no++) {
      if (ZCMD.command == 'S')
           break;
  
      if (last_cmd || !ZCMD.if_flag)
        switch(ZCMD.command) {
          case 'm': /* disabled command */
          case 'o': /* disabled command */
          case 'p': /* disabled command */
          case 'g': /* disabled command */
          case 'e': /* disabled command */
          case 'd': /* disabled command */
            break;

          case 'M': /* read a mobile */
          if (!((ZCMD.arg1>=0) && (ZCMD.arg1<=top_of_mobt))) {
            fprintf(stderr,"  reset_zone(M): unknown mob# [%s]\n",
              zone_table[zone].filename);
            ZCMD.command = 'm'; /* lower case command to disable it */
            break;
          }
          if (!((ZCMD.arg3>=0) && (ZCMD.arg3<=top_of_world))) {
            fprintf(stderr,"  reset_zone(M): unknown room# [%s]\n",
              zone_table[zone].filename);
            ZCMD.command = 'm'; /* lower case command to disable it */
            break;
          }
          if (mob_index[ZCMD.arg1].number < ZCMD.arg2){
            mob = read_mobile(ZCMD.arg1, REAL);
            char_to_room(mob, ZCMD.arg3);
            last_cmd = 1;
          } else
             last_cmd = 0;
          if (help) fprintf(stderr,"  reset_zone(M): mob#%d [%s]\n",
            mob_index[ZCMD.arg1].virtual, zone_table[zone].filename);
          break;
  
          case 'H': /* help whats going on */
	    if (!help) fprintf(stderr,"\n");
            fprintf(stderr,"Help message %d [%s]\n",
              ZCMD.arg1, zone_table[zone].filename);
            help = 1;
          break;
  
          case 'O': /* read an object */
            if (!((ZCMD.arg1>=0) && (ZCMD.arg1<=top_of_objt))) {
              fprintf(stderr,"  reset_zone(O): unknown obj# [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'o'; /* lower case command to disable it */
              break;
            }
            if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
              if (ZCMD.arg3 >= 0) {
                if (!get_obj_in_list_num(ZCMD.arg1,world[ZCMD.arg3].contents)) {
                  obj = read_object(ZCMD.arg1, REAL);
                  obj_to_room(obj, ZCMD.arg3);
                  last_cmd = 1;
                  /* error check */
		  if (!(obj->carried_by) 
		    && !(obj->in_obj) 
		    && obj->in_room == NOWHERE)
		    extract_obj(obj);
                } else
                  last_cmd = 0;
              } else {
		/* why would we want to do this?
                obj = read_object(ZCMD.arg1, REAL);
                obj->in_room = NOWHERE;
		*/
                fprintf(stderr,"  reset_zone(O %d): unknown room# [%s]\n",
                  obj_index[ZCMD.arg1].number, zone_table[zone].filename);
                ZCMD.command = 'o'; /* lower case command to disable it */
                last_cmd = 1;
              } 
            else
              last_cmd = 0;
            if (help) fprintf(stderr,"  reset_zone(O): [%s]\n",
              zone_table[zone].filename);
          break;
  
          case 'P': /* object to object */
            if (!((ZCMD.arg1>=0) && (ZCMD.arg1<=top_of_objt))) {
              fprintf(stderr,"  reset_zone(P): unknown obj# [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'p'; /* lower case command to disable it */
              break;
            }
            if (!((ZCMD.arg3>=0) && (ZCMD.arg3<=top_of_objt))) {
              fprintf(stderr,"  reset_zone(P): unknown obj# [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'p'; /* lower case command to disable it */
              break;
            }
            if (obj_index[ZCMD.arg1].number < ZCMD.arg2){
              obj_to = get_obj_num(ZCMD.arg3);

              if (!obj_to) {
                fprintf(stderr,"  reset_zone(P): unknown target obj[%s]\n",
                  zone_table[zone].filename);
                ZCMD.command = 'p'; /* lower case command to disable it */
                break;
              }

              obj = read_object(ZCMD.arg1, REAL);
              obj_to_obj(obj, obj_to);
              last_cmd = 1;
              /* error check */
	      if (!(obj->carried_by) 
	        && !(obj->in_obj) 
	        && obj->in_room == NOWHERE)
	        extract_obj(obj);
            } else
              last_cmd = 0;
            if (help) fprintf(stderr,"  reset_zone(P): [%s]\n",
              zone_table[zone].filename);
          break;
  
          case 'G': /* obj_to_char */
            if (!((ZCMD.arg1>=0) && (ZCMD.arg1<=top_of_objt))) {
              fprintf(stderr,"  reset_zone(G): unknown obj# [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'g'; /* lower case command to disable it */
              break;
            }
            if (!(mob)) { 
              fprintf(stderr,"  reset_zone(G): no mob def'd# [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'g'; /* lower case command to disable it */
              break;
            } 
            if (obj_index[ZCMD.arg1].number < ZCMD.arg2){         
              obj = read_object(ZCMD.arg1, REAL);
              obj_to_char(obj, mob);
              last_cmd = 1;
              /* error check */
	      if (!(obj->carried_by) 
	        && !(obj->in_obj) 
	        && obj->in_room == NOWHERE)
	        extract_obj(obj);
            } else
              last_cmd = 0;
            if (help) fprintf(stderr,"  reset_zone(G): [%s]\n",
              zone_table[zone].filename);
          break;
  
          case 'E': /* object to equipment list */
            if (!((ZCMD.arg1>=0) && (ZCMD.arg1<=top_of_objt))) {
              fprintf(stderr,"  reset_zone(E): unknown obj# [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'e'; /* lower case command to disable it */
              break;
            }
            if (!(mob)) {
              fprintf(stderr,"  reset_zone(E): no mob def'd# [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'e'; /* lower case command to disable it */
              break;
            }
            if (ZCMD.arg3 <0 || ZCMD.arg3 > MAX_WEAR) {
              fprintf(stderr,"  reset_zone(E): illegal location [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'e'; /* lower case command to disable it */
              break;
            }
            if (!(mob)) {
              fprintf(stderr,"  reset_zone(E): mob allready equipped [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'e'; /* lower case command to disable it */
              break;
            }
            if (mob->equipment[ZCMD.arg3]) {
              fprintf(stderr,"  reset_zone(E): location allready equipped [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'e'; /* lower case command to disable it */
              break;
            }
            if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {         
              obj = read_object(ZCMD.arg1, REAL);
              equip_char(mob, obj, ZCMD.arg3);
              last_cmd = 1;

              /* error check  - will this screw this up? */
	      if (!(obj->carried_by) 
	        && !(obj->in_obj) 
	        && obj->in_room == NOWHERE)
	        extract_obj(obj);
            } else
              last_cmd = 0;
            if (help) fprintf(stderr,"  reset_zone(E): [%s]\n",
              zone_table[zone].filename);
           break;
  
           case 'D': /* set state of door */
            if (!((ZCMD.arg1>=0) && (ZCMD.arg1<=top_of_world))) {
              fprintf(stderr,"  reset_zone(D): unknown room# [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'd'; /* lower case command to disable it */
              break;
            }
            if (!((ZCMD.arg2>=0) && (ZCMD.arg2<=5))) {
              fprintf(stderr,"  reset_zone(D): unknown dir# [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'd'; /* lower case command to disable it */
              break;
            }
            if (!(world[ZCMD.arg1].dir_option[ZCMD.arg2])) {
              fprintf(stderr,"  reset_zone(D): no dir exists [%s]\n",
                zone_table[zone].filename);
              ZCMD.command = 'd'; /* lower case command to disable it */
              break;
            }
            switch (ZCMD.arg3) {
              case 0:
                REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                  EX_LOCKED);
                SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                  EX_ISDOOR);
                REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                  EX_CLOSED);
              break;
              case 1:
                SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                  EX_ISDOOR);
                SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                  EX_CLOSED);
                REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                  EX_LOCKED);
              break;
              case 2:
                SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                  EX_LOCKED);
                SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                  EX_ISDOOR);
                SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
                  EX_CLOSED);
              break;
              default:
                fprintf(stderr,"  reset_zone(D): unknown door state [%s]\n",
                  zone_table[zone].filename);
                ZCMD.command = 'd'; /* lower case command to disable it */
              break;
            } /* switch arg3 */
            last_cmd = 1;
            if (help) fprintf(stderr,"  reset_zone(D): [%s]\n",
              zone_table[zone].filename);
          break; /* case E */
  
          default:
            sprintf(buf, "Undefd cmd in reset table; zone %s cmd %d.\n\r",
             zone_table[zone].filename, cmd_no);
            log(buf);
            exit(0);
          break;
      } else
        last_cmd = 0;
    }
    zone_table[zone].age = 0;
  }
  #undef ZCMD
  
  
  /* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
  int is_empty(int zone_nr)
  {
     struct descriptor_data *i;
  
     for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
           if (world[i->character->in_room].zone == zone_nr)
            return(0);
  
     return(1);
  }
  
  
  
  
  
  /*************************************************************************
  *  stuff related to the save/load player system                                      *
  *********************************************************************** */
  
  /* Load a char, TRUE if loaded, FALSE if not */
  int load_char(char *name, struct char_file_u *char_element)
  {
     FILE *fl;
     int player_i;
  
     int find_name(char *name);
  
     if ((player_i = find_name(name)) >= 0) {
  
      if (!(fl = fopen(PLAYER_FILE, "r"))) {
           perror("Opening player file for reading. (db.c, load_char)");
           exit(0);
      }
  
      fseek(fl, (long) (player_table[player_i].nr *
      sizeof(struct char_file_u)), 0);
  
      fread(char_element, sizeof(struct char_file_u), 1, fl);
      fclose(fl);
      return(player_i);
     } else
  
      return(-1);
  }
  
  
  
  
  /* copy data from the file structure to a char struct */  
  void store_to_char(struct char_file_u *st, struct char_data *ch)
  {
     int i;
  
     GET_SEX(ch) = st->sex;
     GET_CLASS(ch) = st->class;
     GET_LEVEL(ch) = st->level;
  
     ch->player.short_descr = 0;
     ch->player.long_descr = 0;
  
     if (*st->title)
     {
      CREATE(ch->player.title, char, strlen(st->title) + 1);
      strcpy(ch->player.title, st->title);
     }
     else
      GET_TITLE(ch) = 0;
  
     if (*st->description)
     {
      CREATE(ch->player.description, char, 
           strlen(st->description) + 1);
      strcpy(ch->player.description, st->description);
     }
     else
      ch->player.description = 0;

     if (*st->immortal_enter) {
       CREATE(ch->player.immortal_enter,char, strlen(st->immortal_enter)+1);
       strcpy(ch->player.immortal_enter,st->immortal_enter);
     } else
       ch->player.immortal_enter=0;
     if (*st->immortal_exit){ 
       CREATE(ch->player.immortal_exit,char, strlen(st->immortal_exit)+1);
       strcpy(ch->player.immortal_exit,st->immortal_exit);
     } else
       ch->player.immortal_exit=0;

     ch->player.hometown = st->hometown;
  
     ch->player.time.birth = st->birth;
     ch->player.time.played = st->played;
     ch->player.time.logon  = time(0);
  
     for (i = 0; i <= MAX_TOUNGE - 1; i++)
      ch->player.talks[i] = st->talks[i];
  
     ch->player.weight = st->weight;
     ch->player.height = st->height;
  
     ch->abilities = st->abilities;
     ch->tmpabilities = st->abilities;
     ch->points = st->points;
  
     for (i = 0; i <= MAX_SKILLS - 1; i++)
      ch->skills[i] = st->skills[i];
  
     ch->specials.spells_to_learn = st->spells_to_learn;
     ch->specials.alignment    = st->alignment;
  
     ch->specials.act          = st->act;
     ch->specials.carry_weight = 0;
     ch->specials.carry_items  = 0;
     ch->points.armor          = MAX_ARMOUR;
     ch->points.hitroll        = 0;
     ch->points.damroll        = 0;
  
     CREATE(GET_NAME(ch), char, strlen(st->name) +1);
     strcpy(GET_NAME(ch), st->name);
  
     /* Not used as far as I can see (Michael) */
     for(i = 0; i <= 4; i++)
       ch->specials.apply_saving_throw[i] = st->apply_saving_throw[i];
  
     for(i = 0; i <= 2; i++)
       GET_COND(ch, i) = st->conditions[i];
  
     /* Add all spell effects */
     for(i=0; i < MAX_AFFECT; i++) {
      if (st->affected[i].type)
           affect_to_char(ch, &st->affected[i]);
     }
     ch->in_room = st->load_room;
     affect_total(ch);
  } /* store_to_char */
  
     
  
     
  /* copy vital data from a players char-structure to the file structure */
  void char_to_store(struct char_data *ch, struct char_file_u *st)
  {
     int i;
     struct affected_type *af;
     struct obj_data *char_eq[MAX_WEAR];
  
     /* Unaffect everything a character can be affected by */
  
     for(i=0; i<MAX_WEAR; i++) {
      if (ch->equipment[i])
           char_eq[i] = unequip_char(ch, i);
      else
           char_eq[i] = 0;
     }
  
     for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
      if (af) {
           st->affected[i] = *af;
           st->affected[i].next = 0;
           /* subtract effect of the spell or the effect will be doubled */
           affect_modify( ch, st->affected[i].location,
                  st->affected[i].modifier,
                  st->affected[i].bitvector, FALSE);                         
           af = af->next;
      } else {
           st->affected[i].type = 0;  /* Zero signifies not used */
           st->affected[i].duration = 0;
           st->affected[i].modifier = 0;
           st->affected[i].location = 0;
           st->affected[i].bitvector = 0;
           st->affected[i].next = 0;
      }
     }
  
     if ((i >= MAX_AFFECT) && af && af->next)
      log("WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");
  
  
  
     ch->tmpabilities = ch->abilities;
  
     st->birth      = ch->player.time.birth;
     st->played     = ch->player.time.played;
     st->played    += (long) (time(0) - ch->player.time.logon);
     st->last_logon = time(0);
  
     ch->player.time.played = st->played;
     ch->player.time.logon = time(0);
  
     st->hometown = ch->player.hometown;
     st->weight   = GET_WEIGHT(ch);
     st->height   = GET_HEIGHT(ch);
     st->sex      = GET_SEX(ch);
     st->class    = GET_CLASS(ch);
     st->level    = GET_LEVEL(ch);
     st->abilities = ch->abilities;
     st->points    = ch->points;
     st->alignment       = ch->specials.alignment;
     st->spells_to_learn = ch->specials.spells_to_learn;
     st->act             = ch->specials.act;
  
     st->points.armor   = MAX_ARMOUR;
     st->points.hitroll =  0;
     st->points.damroll =  0;
  
     if (GET_TITLE(ch))
      strcpy(st->title, GET_TITLE(ch));
     else
      *st->title = '\0';
  
     if (ch->player.description)
      strcpy(st->description, ch->player.description);
     else
      *st->description = '\0';
    
    if (ch->player.immortal_enter)
      strcpy(st->immortal_enter, ch->player.immortal_enter);
     else
      *st->immortal_enter = '\0';
     
     if (ch->player.immortal_exit)
      strcpy(st->immortal_exit, ch->player.immortal_exit);
     else
      *st->immortal_exit = '\0';
  
     for (i = 0; i <= MAX_TOUNGE - 1; i++)
      st->talks[i] = ch->player.talks[i];
  
     for (i = 0; i <= MAX_SKILLS - 1; i++)
      st->skills[i] = ch->skills[i];
  
     strcpy(st->name, GET_NAME(ch) );
  
     for(i = 0; i <= 4; i++)
       st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];
  
     for(i = 0; i <= 2; i++)
       st->conditions[i] = GET_COND(ch, i);
  
     for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
      if (af) {
           /* Add effect of the spell or it will be lost */
           /* When saving without quitting               */
           affect_modify( ch, st->affected[i].location,
                  st->affected[i].modifier,
                  st->affected[i].bitvector, TRUE);
           af = af->next;
      }
     }
  
     for(i=0; i<MAX_WEAR; i++) {
      if (char_eq[i])
           equip_char(ch, char_eq[i], i);
     }
  
     /* affect_total(ch); */
  } /* Char to store */
  
  
  
  
  /* create a new entry in the in-memory index table for the player file */
  int create_entry(char *name)
  {
     int i;
  
     if (top_of_p_table < 0)
     {
      CREATE(player_table, struct player_index_element, 1);
      top_of_p_table = 0;
     }
     else
      if ( (!(player_table = (struct player_index_element *) 
        realloc(player_table, sizeof(struct player_index_element) * 
        (++top_of_p_table + 1)))) )
      {
           perror("create entry");
           exit(1);
      }
     CREATE(player_table[top_of_p_table].name, char , strlen(name) + 1);
  
     /* copy lowercase equivalent of name to table field */
     for (i = 0; ( *(player_table[top_of_p_table].name + i) = 
           LOWER(*(name + i)) ); i++);
  
     player_table[top_of_p_table].nr = top_of_p_table;
     return (top_of_p_table);
   }
      
  
  
  /* write the vital data of a player to the player file */
  void save_char(struct char_data *ch, sh_int load_room)
  {
     struct char_file_u st;
     FILE *fl;
     char mode[4];
     int expand;
  
     if (IS_NPC(ch) || !ch->desc)
      return;
  
     if ( (expand = (ch->desc->pos > top_of_p_file)) )
     {
      strcpy(mode, "a");
      top_of_p_file++;
     }
     else
      strcpy(mode, "r+");
  
     char_to_store(ch, &st);
     st.load_room = load_room;
  
     strcpy(st.pwd, ch->desc->pwd);
  
     if (!(fl = fopen(PLAYER_FILE, mode)))
     {
      perror("save char");
      exit(1);
     }
  
     if (!expand)
      fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);
  
     fwrite(&st, sizeof(struct char_file_u), 1, fl);
  
     fclose(fl);
  }
  
  
  
  
  /* for possible later use with qsort */
  int compare(struct player_index_element *arg1, struct player_index_element 
     *arg2)
  {
     return (str_cmp(arg1->name, arg2->name));
  }
  
  
  
  
  /************************************************************************
  *  procs of a (more or less) general utility nature            *
  ********************************************************************** */
  
  
  /* read and allocate space for a '~'-terminated string from a given file */
  char *fread_string(FILE *fl)
  {
     #define READ_MAX_LENGTH 2048
  
     char buf[MAX_STRING_LENGTH], tmp[READ_MAX_LENGTH];
     char *rslt;
     register char *point;
     int flag;
  
     bzero(buf, MAX_STRING_LENGTH);
  
     do
     {
      if (!fgets(tmp, READ_MAX_LENGTH, fl))
      {
           log("Oh oh string too long: hope this helps");
           log(buf);
           log(tmp);
           perror("fread_str: (db.c)");
           exit(0);
      }
  
      if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
      {
           log("fread_string: string too large (db.c)");
           log(tmp);
           exit(0);
      }
      else
           strcat(buf, tmp);
  
      for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
           point--);      
      if ( (flag = (*point == '~')) )
           if (*(buf + strlen(buf) - 3) == '\n')
           {
            *(buf + strlen(buf) - 2) = '\r';
            *(buf + strlen(buf) - 1) = '\0';
           }
           else
            *(buf + strlen(buf) -2) = '\0';
      else
      {
           *(buf + strlen(buf) + 1) = '\0';
           *(buf + strlen(buf)) = '\r';
      }
     }
     while (!flag);
  
     /* do the allocate boogie  */
  
     if (strlen(buf) > 0)
     {
      CREATE(rslt, char, strlen(buf) + 1);
      strcpy(rslt, buf);
     }
     else
      rslt = 0;
     return(rslt);
  }
  
  
  
  
  
  /* release memory allocated for a char struct */
  void free_char(struct char_data *ch)
  {
     struct affected_type *af;
  
     free(GET_NAME(ch));
  
     if (ch->player.title)
      free(ch->player.title);
     if (ch->player.short_descr)
      free(ch->player.short_descr);
     if (ch->player.long_descr)
      free(ch->player.long_descr);
     if(ch->player.description)
      free(ch->player.description);
     if(ch->player.immortal_enter)
      free(ch->player.immortal_enter);
     if(ch->player.immortal_exit)
      free(ch->player.immortal_exit);
  
     for (af = ch->affected; af; af = af->next) 
      affect_remove(ch, af);
  
     free(ch);
  }
  
  
  
  
  
  
  
  /* release memory allocated for an obj struct */
  void free_obj(struct obj_data *obj)
  {
     struct extra_descr_data *this, *next_one;
  
     free(obj->name);
     if(obj->description)
      free(obj->description);
     if(obj->short_description)
      free(obj->short_description);
     if(obj->action_description)
      free(obj->action_description);
  
     for( this = obj->ex_description ;
      (this != 0);this = next_one )
     {
      next_one = this->next;
      if(this->keyword)
           free(this->keyword);
      if(this->description)
           free(this->description);
      free(this);
     }
  
     free(obj);
  }
  
  /* my version of file to string much better */
  char *file2string(char *name) {
     struct stat theStat;
     char *theString, *finalString;
     FILE *theFile;
     long i, offset = 0, cr = 0, lf = 0;
     
     if (  (stat(name, &theStat))
        || (!(theString = (char *) malloc((size_t) theStat.st_size +1)))
        || (!(theFile = fopen(name, "rb"))) ){
        if (!(theString = (char *) malloc(1))) {
           log("FATAL ERROR IN file2string(db.c) NOT EVEN 1!?! byte free");
           exit(1);
        }
        else if (!(theFile = fopen(name, "rb")))  
	  fprintf(stderr, "file2string (db.c) error in fopen %s\n", name);
        else {
	  fprintf(stderr, "file2string (db.c) unable to read in %s\n", name);
	  *theString = 0; /* must return a valid string */
	}
     } else {
       fread(theString, (size_t) theStat.st_size, 1, theFile);
       theString[(size_t) theStat.st_size] = 0; /* terminate string */
       fclose(theFile);
     }
     for (i=0;i<theStat.st_size;i++){
       if (theString[i] == '\r') lf++; 
       if (theString[i] == '\n') cr++; 
     } 
     if (cr >= lf)  
       return theString;
     if (cr>0) {
       log("file2string: some crlf's but not all!?!?");
       return theString;
     }
    
     /* not as many cr's as lf's */
     /* assuming here that either lines are crlf, or lf, lines ending
        in just cr will really screw this up */
     if(!(finalString = (char *) malloc((size_t)theStat.st_size+1+lf-cr))){
       log("FATAL ERROR IN file2string(db.c) cant alloc 2nd buffer");
       exit(1);
     }
     for (i=0;i<=theStat.st_size;i++) { /* go right to terminating 0 */
       if (theString[i] == '\r') {
         finalString[i+offset] = '\n';
         offset++;
       }
       finalString[i+offset] = theString[i]; 
     }
     free(theString);
     return finalString;
  }
  
  
  
  /* read contents of a text file, and place in buf */
  int file_to_string(char *name, char *buf)
  {
     FILE *fl;
     char tmp[100];
  
     *buf = '\0';
  
     if (!(fl = fopen(name, "r")))
     {
      perror("file-to-string");
      *buf = '\0';
      return(-1);
     }
  
     do
     {
      fgets(tmp, 99, fl);
  
      if (!feof(fl))
      {
           if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH)
           {
            log("fl->strng: string too big (db.c, file_to_string)");
            *buf = '\0';
            return(-1);
           }
  
           strcat(buf, tmp);
           *(buf + strlen(buf) + 1) = '\0';
           *(buf + strlen(buf)) = '\r';
      }
     }
     while (!feof(fl));
  
     fclose(fl);
  
     return(0);
  }
  
  
  
  
  /* clear some of the the working variables of a char */
  void reset_char(struct char_data *ch)
  {
     int i;
  
     for (i = 0; i < MAX_WEAR; i++) /* Initialisering */
      ch->equipment[i] = 0;
  
     ch->followers = 0;
     ch->master = 0;
  /* ch->in_room = NOWHERE; Used for start in room */
     ch->carrying = 0;
     ch->next = 0;
     ch->next_fighting = 0;
     ch->next_in_room = 0;
     ch->specials.fighting = 0;
     ch->specials.position = POSITION_STANDING;
     ch->specials.default_pos = POSITION_STANDING;
     ch->specials.carry_weight = 0;
     ch->specials.carry_items = 0;
  
     if (GET_HIT(ch) <= 0)
      GET_HIT(ch) = 1;
     if (GET_MOVE(ch) <= 0)
      GET_MOVE(ch) = 1;
     if (GET_MANA(ch) <= 0)
      GET_MANA(ch) = 1;
  }
  
  
  
  /* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
  void clear_char(struct char_data *ch)
  {
     memset(ch, '\0', sizeof(struct char_data));
  
     ch->in_room = NOWHERE;
     ch->specials.was_in_room = NOWHERE;
     ch->specials.position = POSITION_STANDING;
     ch->specials.default_pos = POSITION_STANDING;
     GET_AC(ch) = MAX_ARMOUR; /* Basic Armor */
  }
  
  
  void clear_object(struct obj_data *obj)
  {
     memset(obj, '\0', sizeof(struct obj_data));
  
     obj->item_number = -1;
     obj->in_room     = NOWHERE;
  }
  
  
  
  
  /* initialize a new character only if class is set */
  void init_char(struct char_data *ch)
  {
     int i;
  
     /* *** if this is our first player --- he be level 45 *** */
  
     if (top_of_p_table < 0)
     {
       GET_EXP(ch) = 500000000;
       GET_LEVEL(ch) = IMP_LEV;
     }
  
     set_title(ch);
  
     ch->player.short_descr = 0;
     ch->player.long_descr = 0;
     ch->player.description = 0;
     ch->player.immortal_enter = 0;
     ch->player.immortal_exit = 0;
  
     ch->player.hometown = number(1,4);
  
     ch->player.time.birth = time(0);
     ch->player.time.played = 0;
     ch->player.time.logon = time(0);
  
     for (i = 0; i < MAX_TOUNGE; i++)
       ch->player.talks[i] = 0;
  
     GET_STR(ch) = 9;
     GET_INT(ch) = 9;
     GET_WIS(ch) = 9;
     GET_DEX(ch) = 9;
     GET_CON(ch) = 9;
  
     /* make favors for sex */
     if (ch->player.sex == SEX_MALE) {   
       GET_STR(ch) = 11; /* Ryan is sexist! */
       ch->player.weight = number(120,180);
       ch->player.height = number(160,200);
     } else {
       GET_DEX(ch) = 11;
       ch->player.weight = number(100,160);
       ch->player.height = number(150,180);
     }
  /* if (GET_LEVEL(ch) == IMP_LEV)
      {
      
      }
  */
     ch->points.mana = GET_MAX_MANA(ch);
     ch->points.hit = GET_MAX_HIT(ch);
     ch->points.move = GET_MAX_MOVE(ch);
     ch->points.armor = MAX_ARMOUR;
  
     for (i = 0; i <= MAX_SKILLS - 1; i++)
     {
       if (GET_LEVEL(ch) <IMP_LEV) {
           ch->skills[i].learned = 0;
           ch->skills[i].recognise = FALSE;
       }    else {
           ch->skills[i].learned = 100;
           ch->skills[i].recognise = FALSE;
       }
     }
  
     ch->specials.affected_by = 0;
     ch->specials.spells_to_learn = 0;
  
     for (i = 0; i < 5; i++)
       ch->specials.apply_saving_throw[i] = 0;
  
     for (i = 0; i < 3; i++)
       GET_COND(ch, i) = (GET_LEVEL(ch) == IMP_LEV ? -1 : IMP_LEV);
  }
  
  
  /* returns the real number of the room with given virtual number */
  int real_room(int virtual)
  {
     int bot, top, mid, zone;

     if ((zone = zone_of(virtual)) == -1) {
        fprintf(stderr, "Room %d does not exist (illegal zone)\n", virtual);
        return(-1);
     }
     bot = zone_table[zone].real_bottom;
     top = zone_table[zone].real_top; /* should set to real_top of zone */
     
     if ((top < 0) || (bot < 0)){
        fprintf(stderr, "Room %d does not exist (empty zone)\n", virtual);
        return(-1);
     }
     if (top>top_of_world) {
        fprintf(stderr, "real_room: force limit search to top_of_world\n");
        top = top_of_world;
     }
     if (IS_SET(zone_table[zone].dirty_wld, SCRAMBLED)) {
        /* oh oh order of zone is scrambled - no binary */
        for (mid = bot; 
             mid <= top_of_world && mid <= zone_table[zone].real_top; 
             mid = world[mid].next) {
           if (world[mid].number == virtual)
              return(mid);
           else if(world[mid].number > virtual) 
              break;
        }
        fprintf(stderr, "Real_Room(d): %d does not exist in database\n", virtual);
        return(-1);
     } else {
         /* perform binary search on world-table */
        for (;;){
           mid = (bot + top) / 2;
           if ((world + mid)->number == virtual)
              return(mid);
           if (bot >= top){
              fprintf(stderr, "Real_Room(c): %d does not exist in database\n", virtual);
              return(-1);
           }
           if ((world + mid)->number > virtual)
              top = mid - 1;
           else
              bot = mid + 1;
        }
     }
  }
  
  
   /* returns the real number of the object with given virtual number */
  int real_mobile(int virtual)
  {
     int bot, top, mid, zone;

     if ((zone = zone_of(virtual)) == -1) {
        fprintf(stderr, "Real_Mob: Mob %d does not exist (illegal zone)\n", virtual);
        return(-1);
     }
     bot = zone_table[zone].bot_of_mobt;
     top = zone_table[zone].top_of_mobt; /* should set to real_top of zone */
     
     if ((top < 0) || (bot < 0)){
        fprintf(stderr, "mob %d does not exist (empty zone)\n", virtual);
        return(-1);
     }
     if (top>top_of_mobt) {
        fprintf(stderr, "real_mob: force limit search to top_of_mobt\n");
        top = top_of_mobt;
     }
     if (IS_SET(zone_table[zone].dirty_mob,SCRAMBLED)) {
        /* oh oh order of zone is scrambled - no binary */
        for (mid = bot; mid <= top_of_mobt && mid <= top; mid = mob_index[mid].next) {
           if (mob_index[mid].virtual == virtual)
              return(mid);
           else if(mob_index[mid].virtual > virtual) 
              break;
        }
        fprintf(stderr, "Real_Mob(d): %d does not exist in database\n", virtual);
        return(-1);
     } else {
         /* perform binary search on mob-table */
        for (;;){
           mid = (bot + top) / 2;
           if ((mob_index + mid)->virtual == virtual) {
              return(mid);
           }
           if (bot >= top){
              fprintf(stderr, "Real_Mob(c): %d does not exist in database\n", virtual);
              return(-1);
           }
           if ((mob_index + mid)->virtual > virtual)
              top = mid - 1;
           else
              bot = mid + 1;
        }
     }
  }
   
  
  /* returns the real number of the object with given virtual number */
  int real_object(int virtual)
  {
    int bot, top, mid, zone;

    if ((zone = zone_of(virtual)) == -1)
      return(-1);
    bot = zone_table[zone].bot_of_objt;
    top = zone_table[zone].top_of_objt; /* should set to real_top of zone */
     
    if ((top < 0) || (bot < 0))
      return(-1);
    if (IS_SET(zone_table[zone].dirty_obj,SCRAMBLED)) {
      /* oh oh order of zone is scrambled - no binary */
      for (mid = bot; mid <= top_of_objt && mid <= top; mid = obj_index[mid].next) {
        if (obj_index[mid].virtual == virtual)
          return(mid);
        else if(obj_index[mid].virtual > virtual) 
          break;
      }
      fprintf(stderr, "Real_obj(dirty): %d does not exist in database\n", virtual);
      return(-1);
    } else {
      /* perform binary search on obj-table */
      for (;;){
        mid = (bot + top) / 2;
        if ((obj_index + mid)->virtual == virtual)
          return(mid);
        if (bot >= top) {
          fprintf(stderr, "Real_obj(clean): %d does not exist in database\n", virtual);
          return(-1);
        }
        if ((obj_index + mid)->virtual > virtual)
          top = mid - 1;
        else
          bot = mid + 1;
      }
    }
  }

/* Save routines go here! */
void Write_String(FILE *theFile, char *theString) {
   int i,j;

   if (theString) {
      j = strlen(theString);
      for(i=0; i<j; i++) {
         if (theString[i] != 13)
           fputc(theString[i],theFile);
      }
   }
   fprintf(theFile,"~\n");
}

int Virtual_Room(int real){
   if ((real < 0) || (real > top_of_world))
      return -1;
   else
      return world[real].number;
}
int Virtual_Dir(int real){
/* this was a dumb idea, in order to make all the direction
   flags backwards compatible to original diku... however
   since the flags got screwed up somewhere during transition
   why worry about it, this routine isnt even called anymore btw
   and if you intend to resurrect it, it should read:
   if IS_SET(EX_ISDOOR) && !IS_SET(EX_PICK... etc etc */
   
     if (real == EX_ISDOOR)
       return 1;
     else if (real == EX_ISDOOR | EX_PICKPROOF)
       return 2;
     else
       return 0;
}
int Virtual_Obj(int real){
   if ((real < 0) || (real > top_of_objt))
      return -1;
   else
      return obj_index[real].virtual;
}
int Virtual_Mob(int real){
   if ((real < 0) || (real > top_of_mobt))
      return -1;
   else
      return mob_index[real].virtual;
}



void Save_zon(int zone) {
   char sys[100];
   FILE *theFile;
   int  cmd_no;
   
   sprintf(sys, "areas/%s.zon", zone_table[zone].filename);
   if ( (theFile = fopen(sys, "r+")) )
   {
      fclose(theFile);
      sprintf(sys, "mv areas/%s.zon areas.bak/%s.zon", 
      zone_table[zone].filename, zone_table[zone].filename);
      system(sys);
   } else {
      fclose(theFile);
   }
   sprintf(sys, "areas/%s.zon", zone_table[zone].filename);
   if (!(theFile = fopen(sys, "w+"))){
      perror("fopen");
      log("Save_zon: could not open file for writing.");
      exit(0);
   }

   /* and now the fun begins - write out the zones zon file */
   rewind(theFile); /* just ensure we're at the start */
   /* next field isnt used anymore reserved for zone flags */
   fprintf(theFile, "#%d\n",zone_table[zone].flags); 
   Write_String(theFile, zone_table[zone].name); /* reserved for zone help */
   fprintf(theFile, "%d ",  zone_table[zone].top);
   fprintf(theFile, "%d ",  zone_table[zone].lifespan);
   fprintf(theFile, "%d\n*\n", zone_table[zone].reset_mode);

   cmd_no = 0;
   while(1) {
      if (zone_table[zone].cmd[cmd_no].command == 'S')
        break;
      fprintf(theFile, "%c %d", 
        zone_table[zone].cmd[cmd_no].command,
        zone_table[zone].cmd[cmd_no].if_flag);

      switch(zone_table[zone].cmd[cmd_no].command){
      case 'M':
         fprintf(theFile, " %d %d %d", 
           Virtual_Mob(zone_table[zone].cmd[cmd_no].arg1),
           zone_table[zone].cmd[cmd_no].arg2,
           Virtual_Room(zone_table[zone].cmd[cmd_no].arg3));
         break;
      case 'O':
         fprintf(theFile, " %d %d %d", 
           Virtual_Obj(zone_table[zone].cmd[cmd_no].arg1),
           zone_table[zone].cmd[cmd_no].arg2,
           Virtual_Room(zone_table[zone].cmd[cmd_no].arg3));
         break;
      case 'G':
         fprintf(theFile, " %d %d", 
           Virtual_Obj(zone_table[zone].cmd[cmd_no].arg1),
           zone_table[zone].cmd[cmd_no].arg2);
         break;
      case 'E':
         fprintf(theFile, " %d %d %d", 
           Virtual_Obj(zone_table[zone].cmd[cmd_no].arg1),
           zone_table[zone].cmd[cmd_no].arg2,
           zone_table[zone].cmd[cmd_no].arg3);
         break;
      case 'P':
         fprintf(theFile, " %d %d %d", 
           Virtual_Obj(zone_table[zone].cmd[cmd_no].arg1),
           zone_table[zone].cmd[cmd_no].arg2,
           Virtual_Obj(zone_table[zone].cmd[cmd_no].arg3));
         break;
      case 'D':
         fprintf(theFile, " %d %d %d", 
           Virtual_Room(zone_table[zone].cmd[cmd_no].arg1),
           zone_table[zone].cmd[cmd_no].arg2,
           zone_table[zone].cmd[cmd_no].arg3);
         break;
      default:
         fprintf(theFile, " %d %d", 
           zone_table[zone].cmd[cmd_no].arg1,
           zone_table[zone].cmd[cmd_no].arg2);
         break;
      }
      
      fprintf(theFile,"\n");
      cmd_no++;
   }

   /* okey dokey all done close up shop */
   fprintf(theFile, "*\nS\n*\n$~\n#999999\n"); /* trailer */
   fclose(theFile);
   REMOVE_BIT(zone_table[zone].dirty_zon, UNSAVED);  /* no longer unsaved */
   sprintf(sys,"Zone %s.zon saved (%d zcmds)",
      zone_table[zone].filename, cmd_no);
   zone_table[zone].reset_num = cmd_no;
   log(sys);
}

void Save_wld(int zone) {
   char sys[100];
   FILE *theFile;
   int  i, dir, count=0;
   struct extra_descr_data *extra;
   
   sprintf(sys, "areas/%s.wld", zone_table[zone].filename);
   if ( (theFile = fopen(sys, "r+")) )
   {
      fclose(theFile);
      sprintf(sys, "mv areas/%s.wld areas.bak/%s.wld", 
      zone_table[zone].filename, zone_table[zone].filename);
      system(sys);
   } else {
      fclose(theFile);
   }
   sprintf(sys, "areas/%s.wld", zone_table[zone].filename);
   if (!(theFile = fopen(sys, "w+"))){
      perror("fopen");
      log("Save_wld: could not open file for writing.");
      exit(0);
   }

   /* and now the fun begins - write out the zones wld file */
   rewind(theFile); /* just ensure we're at the start */
   if ((zone_table[zone].real_bottom < 0) ||
       (zone_table[zone].real_top < 0)) 
      log("Save_wld: no rooms in this zone"); 
   else
   for(i = zone_table[zone].real_bottom;
      world[i].number <= zone_table[zone].top 
      && world[i].number >= zone_table[zone].bottom;
      i = world[i].next ) 
   {
      if (i > top_of_world) {
         break;
      }
      if (i < 0) {
         log("Save_wld: tried to advance below beginning of world");
         break;
      }
      fprintf(theFile, "#%d\n", world[i].number);
      Write_String(theFile, world[i].name);
      Write_String(theFile, world[i].description);
      fprintf(theFile, "%d ",   world[i].zone);
      fprintf(theFile, "%d ",   world[i].room_flags);
      fprintf(theFile, "%d\n",   world[i].sector_type);
      
      for (dir = 0; dir <= 5; dir++){
         if (world[i].dir_option[dir] != 0){
             fprintf(theFile, "D%d\n", dir);
             Write_String(theFile, world[i].dir_option[dir]->general_description);
             Write_String(theFile, world[i].dir_option[dir]->keyword);
             fprintf(theFile, "%d %d %d\n",
                world[i].dir_option[dir]->exit_info, /* used to be Virtual_Dir() */
                world[i].dir_option[dir]->key,
                Virtual_Room(world[i].dir_option[dir]->to_room));
         }
      } 
      for(extra = world[i].ex_description;;
           extra = extra->next) {
           if (!(extra)) break;
           fprintf(theFile, "E\n");
           Write_String(theFile, extra->keyword);
           Write_String(theFile, extra->description);
      }
      fprintf(theFile, "S\n");
      count++;
   }
   /* okey dokey all done close up shop */
   fprintf(theFile, "$~\n#999999\n"); /* trailer */
   fclose(theFile);
   REMOVE_BIT(zone_table[zone].dirty_wld, UNSAVED); /* zone saved but order is scrambled */
   sprintf(sys,"Zone %s.wld saved (%d rooms)", 
      zone_table[zone].filename, count);
   log(sys);
}


void Write_Mob(FILE *theNewFile, struct char_data *mob, int index) {

  fprintf(theNewFile, "#%d\n", mob_index[index].virtual);
  mob_index[index].pos = ftell(theNewFile); 

  /***** String data *** */
  Write_String(theNewFile, mob_index[index].name);
  Write_String(theNewFile, mob->player.short_descr);
  Write_String(theNewFile, mob->player.long_descr);
  Write_String(theNewFile, mob->player.description);

  /* *** Numeric data *** */
  fprintf(theNewFile, "%ld ", mob->specials.act);
  fprintf(theNewFile, "%ld ", mob->specials.affected_by);
  fprintf(theNewFile, "%d S\n", mob->specials.alignment);

  /* all mobs are simple mobs */
  fprintf(theNewFile, "%d ", GET_LEVEL(mob));
  fprintf(theNewFile, "%d ", -1*(mob->points.hitroll-30));
  fprintf(theNewFile, "%d ", mob->points.armor/10);
  fprintf(theNewFile, "%dd%d+%d ", 
    mob->points.max_hit,
    mob->player.weight,
    mob->points.hit);
  fprintf(theNewFile, "%dd%d+%d\n",
    mob->specials.damnodice,
    mob->specials.damsizedice,
    mob->points.damroll);
              
  fprintf(theNewFile, "%d ", mob->points.gold);
  fprintf(theNewFile, "%d\n", GET_EXP(mob));
  fprintf(theNewFile, "%d ", mob->specials.position);
  fprintf(theNewFile, "%d ", mob->specials.default_pos);
  fprintf(theNewFile, "%d\n", mob->player.sex);
}

void Save_mob(int zone) {
   char sys[128];
   FILE *theNewFile;
   int  index;
   struct char_data *mob; 
    
   if (zone_table[zone].bot_of_mobt < 0)
     return;

   for(index = zone_table[zone].bot_of_mobt;
       index <= zone_table[zone].top_of_mobt;
       index =mob_index[index].next) {
     mob = read_mobile(mob_index[index].virtual, PROTOTYPE); /* read 'em all */
   }
   fclose(mob_f);
   mob_f_zone = -1;
   sprintf(sys, "areas/%s.mob", zone_table[zone].filename);
   if ( (theNewFile = fopen(sys, "r+")) ){
      fclose(theNewFile);
      sprintf(sys, "mv areas/%s.mob areas.bak/%s.mob", 
      zone_table[zone].filename, zone_table[zone].filename);
      system(sys);
   } else {
      fclose(theNewFile);
   }
   sprintf(sys, "areas/%s.mob", zone_table[zone].filename);
   if (!(theNewFile = fopen(sys, "w+"))){
      log("Save_mob: could not open file for writing.");
      perror("fopen");
      exit(1);
   }
   /* and now the fun begins - write out the zones mob file */
   for(index = zone_table[zone].bot_of_mobt;
       index <= zone_table[zone].top_of_mobt;
       index=mob_index[index].next) {
     Write_Mob(theNewFile, mob = (struct char_data *) mob_index[index].prototype, index);
     /* now free up the space taken by the prototype */
     free(mob->player.name);
     free(mob->player.name);
     free(mob->player.short_descr);
     free(mob->player.long_descr);
     free(mob->player.description);

/* Confirm mobs have no extra descriptions....
     next_descr = 0;
     for (new_descr = mob->ex_description; new_descr; new_descr = next_descr){
       free(new_descr->keyword);
       free(new_descr->description);
       next_descr = new_descr->next;
       free(new_descr);
     }
 */  
     free(mob);
     mob_index[index].prototype = 0;
   }
   /* okey dokey all done close up shop */
   fprintf(theNewFile, "$~\n#999999\n"); /* trailer */
   fclose(theNewFile);

   REMOVE_BIT(zone_table[zone].dirty_mob, UNSAVED); /* mobs saved */
   sprintf(sys,"Zone %s.mob saved", zone_table[zone].filename);
   log(sys);
}


         
void Write_Obj(FILE *theNewFile, struct obj_data *obj, int index) {
   int i;
      
   struct extra_descr_data *new_descr;
  
  fprintf(theNewFile, "#%d\n", obj_index[index].virtual);
  obj_index[index].pos = ftell(theNewFile); 

  /* *** string data *** */
  Write_String(theNewFile, obj_index[index].name);
  Write_String(theNewFile, obj->short_description);
  Write_String(theNewFile, obj->description);
  Write_String(theNewFile, obj->action_description);
  
  /* *** numeric data *** */
  fprintf(theNewFile, "%d ", obj->obj_flags.type_flag);
  fprintf(theNewFile, "%d ", obj->obj_flags.extra_flags);
  fprintf(theNewFile, "%d\n", obj->obj_flags.wear_flags);
  fprintf(theNewFile, "%d ", obj->obj_flags.value[0]);
  fprintf(theNewFile, "%d ", obj->obj_flags.value[1]);
  fprintf(theNewFile, "%d ", obj->obj_flags.value[2]);
  fprintf(theNewFile, "%d\n", obj->obj_flags.value[3]);
  fprintf(theNewFile, "%d ", obj->obj_flags.weight);
  fprintf(theNewFile, "%d ", obj->obj_flags.cost);
  fprintf(theNewFile, "%d\n", obj->obj_flags.cost_per_day);
  
     /* *** extra descriptions *** */
  for (new_descr = obj->ex_description; new_descr; new_descr = new_descr->next){
    fprintf(theNewFile, "E\n");
    Write_String(theNewFile, new_descr->keyword);
    Write_String(theNewFile, new_descr->description);
  }
  
  for( i = 0; (i < MAX_OBJ_AFFECT); i++){
    if (obj->affected[i].location != APPLY_NONE) 
      fprintf(theNewFile, "A\n%d %d\n", 
        obj->affected[i].location, 
        obj->affected[i].modifier);
  }
}


void Save_obj(int zone) {
   char sys[128];
   FILE *theNewFile;
   int  index;
   struct obj_data *obj;
   struct extra_descr_data *new_descr, *next_descr;
    
   if (zone_table[zone].bot_of_objt < 0)
     return;

   for(index = zone_table[zone].bot_of_objt;
       index<= zone_table[zone].top_of_objt;
       index=obj_index[index].next) {
     obj = read_object(obj_index[index].virtual, PROTOTYPE); /* read 'em all */
   }
   fclose(obj_f);
   obj_f_zone = -1;
   sprintf(sys, "areas/%s.obj", zone_table[zone].filename);
   if ( (theNewFile = fopen(sys, "r+")) ){
      fclose(theNewFile);
      sprintf(sys, "mv areas/%s.obj areas.bak/%s.obj", 
      zone_table[zone].filename, zone_table[zone].filename);
      system(sys);
   } else {
      fclose(theNewFile);
   }
   sprintf(sys, "areas/%s.obj", zone_table[zone].filename);
   if (!(theNewFile = fopen(sys, "w+"))){
      log("Save_obj: could not open file for writing.");
      perror("fopen");
      exit(1);
   }
   /* and now the fun begins - write out the zones obj file */
   for(index = zone_table[zone].bot_of_objt;
       index<= zone_table[zone].top_of_objt;
       index=obj_index[index].next) {
     Write_Obj(theNewFile, obj = (struct obj_data *) obj_index[index].prototype, index);
     /* now free up the space taken by the prototype */
     free(obj->name);
     free(obj->short_description);
     free(obj->description);
     free(obj->action_description);
     next_descr = 0;
     for (new_descr = obj->ex_description; new_descr; new_descr = next_descr){
       free(new_descr->keyword);
       free(new_descr->description);
       next_descr = new_descr->next;
       free(new_descr);
     }  
     free(obj);
     obj_index[index].prototype = 0;
   }
   /* okey dokey all done close up shop */
   fprintf(theNewFile, "$~\n#999999\n"); /* trailer */
   fclose(theNewFile);

   REMOVE_BIT(zone_table[zone].dirty_obj, UNSAVED); /* objs saved */
   sprintf(sys,"Zone %s.obj saved", zone_table[zone].filename);
   log(sys);
}

void Save_Area(int zone) {
   if ((zone >= 0) && (zone <= top_of_zone_table)) {
      if (IS_SET(zone_table[zone].dirty_zon, UNSAVED))
         Save_zon(zone);
      if (IS_SET(zone_table[zone].dirty_wld, UNSAVED))
         Save_wld(zone);
      if (IS_SET(zone_table[zone].dirty_obj, UNSAVED))
         Save_obj(zone);
      if (IS_SET(zone_table[zone].dirty_mob, UNSAVED))
         Save_mob(zone);
   }
}

void Global_Save(void) {
   int i;
   
   for(i=0;i<=top_of_zone_table;i++)
      Save_Area(i);
}


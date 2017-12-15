/*************************************************************************
*  file: db.h , Database module.                          Part ofDIKUMUD *
*  Usage: Loading/Saving chars booting world.                            *
************************************************************************ */
/* various variables */
extern char *credits;      /* the Credits List                */
extern char *news;         /* the news                        */
extern char *motd;         /* the messages of today           */
extern char *help;         /* the main help page              */
extern char *info;         /* the info text                   */
extern char *wizlist;      /* the wizlist                     */
extern char *story;        /* the story off main menu         */
extern char *greeting;     /* well duh-uh!         */
extern char *ciao;         /* good bye screen */

extern struct zone_data *zone_table;
extern struct room_data *world;

extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct message_list fight_messages[MAX_MESSAGES];

extern int top_of_zone_table;
extern int top_of_world;
extern int top_of_mobt;
extern int top_of_objt;

extern struct help_index_element *help_index;
extern struct time_info_data time_info;
extern struct weather_data weather_info;

extern struct player_index_element *player_table;
extern int top_of_p_table;

/* data files used by the game system */

#define DFLT_DIR          "lib"           /* default data directory     */

#define WORLD_FILE        "tinyworld.wld" /* room definitions          */
#define MOB_FILE          "tinyworld.mob" /* monster prototypes        */
#define OBJ_FILE          "tinyworld.obj" /* object prototypes         */
#define SHOP_FILE         "tinyworld.shp" /* shopkeepers               */
#define ZONE_FILE         "tinyworld.zon" /* zone defs & command tables */
#define CREDITS_FILE      "credits"       /* for the 'credits' command  */
#define NEWS_FILE         "news"          /* for the 'news' command     */
#define MOTD_FILE         "motd"          /* messages of today        */
#define CIAO_FILE         "ciao"          /* good bye screen          */
#define PLAYER_FILE       "players"       /* the player database      */
#define TIME_FILE         "time"          /* game calendar information  */
#define IDEA_FILE         "ideas"         /* for the'idea'-command     */
#define TYPO_FILE         "typos"         /*         'typo'           */
#define BUG_FILE          "bugs"          /*         'bug'            */
#define MESS_FILE         "messages"      /* damage message           */
#define SOCMESS_FILE      "actions"       /* messgs for socialacts     */
#define HELP_KWRD_FILE    "help_table"    /* for HELP <keywrd>        */
#define HELP_PAGE_FILE    "help"          /* for HELP <CR>            */
#define INFO_FILE         "info"          /* for INFO                 */
#define WIZLIST_FILE      "wizlist"       /* for WIZLIST              */
#define POSEMESS_FILE     "poses"         /* for 'pose'-command       */

/* online creation save routines */
extern void Save_zon(int zone);
extern void Save_wld(int zone);
extern void Save_mob(int zone);
extern void Save_obj(int zone);
extern void Save_Area(int zone);
extern void Global_Save(void);

/* public procedures in db.c */
extern char *file2string(char *name);
extern void boot_db(void);
extern void save_char(struct char_data *ch, sh_int load_room);
extern void store_to_char(struct char_file_u *st, struct char_data *ch);
extern int  create_entry(char *name);
extern void zone_update(void);
extern void init_char(struct char_data *ch);
extern void clear_char(struct char_data *ch);
extern void clear_object(struct obj_data *obj);
extern void reset_char(struct char_data *ch);
extern void free_char(struct char_data *ch);
extern void free_obj(struct obj_data *obj);
extern int  real_room(int virtual);
extern char *fread_string(FILE *fl);
extern int  real_object(int virtual);
extern int  real_mobile(int virtual);
extern void reset_zone(int zone);
extern void allocate_room(int new_top);
extern void allocate_obj(int new_top);
extern void allocate_mob(int new_top);
extern void allocate_zcmd(int zone, int new_top);
extern int  zone_of(int virtual);

/* Dirty flags */
#define UNSAVED   1<<0
#define SCRAMBLED 1<<1 /* for mob/obj */
#define SHOWN     1<<2 /* for zone file */
 
#define REAL 0
#define VIRTUAL 1
#define PROTOTYPE 2

struct obj_data *read_object(int nr, int type);
struct char_data *read_mobile(int nr, int type);

#define MENU         \
"\n\r[0;34m           :::::: [1;31mCrimsonMUD [1;37mMain Menu [0;34m::::::[1;37m\n\r\n\r\
        0) Exit from [1;31mCrimsonMUD[1;37m.\n\r\
        1) Enter the game.\n\r\
        2) Enter description.\n\r\
        3) Useful Background Information (Read it).\n\r\
        4) Change password (Do it frequently).\n\r\n\r\
[1;33m           Make your choice: [1;37m"


#define WELC_MESSG \
"\n\rWelcome to the land of [1;31mCrimsonMUD[1;37m. May your visit here be...Interesting.\
\n\r\n\r"

/* structure for the reset commands */
struct reset_com
{
     char command;   /* current command                      */ 
     bool if_flag;   /* if TRUE: exe only if preceding exe'd */
     int arg1;       /*                                      */
     int arg2;       /* Arguments to the command             */
     int arg3;       /*                                      */

     /* 
     *  Commands:              *
     *  'M': Read a mobile     *
     *  'O': Read an object    *
     *  'G': Give obj to mob   *
     *  'P': Put obj in obj    *
     *  'G': Obj to char       *
     *  'E': Obj to char equip *
     *  'D': Set state of door *
     */
};

/* element in monster and object index-tables   */
struct index_data
{
     int virtual;    /* virtual number of this mob/obj          */
     long pos;       /* file position of this field             */
     char *name;       /* file position of this field             */
     int number;     /* number of existing units of this mob/obj*/
     char *prototype; /* prototype for online creation */
     int (*func)();  /* special procedure for this mob/obj      */
     int next;        /* number of next entry in table if scrambled */
     int zone;        /* needed for online obj/mob creation - zone obj belongs too */
};



/* zone definition structure. for the 'zone-table'   */
struct zone_data
{
     char *filename;      /* whats the file name for this zone*/
     char *name;          /* old name of this zone - maybe help soon*/
     char *lord;          /* lord/author of the zone */
     int lifespan;        /* how long between resets (minutes)*/
     int age;             /* current age of this zone(minutes) */
     int flags;           /* zone flags, notably ZONE_CLOSED */
     
     int dirty_wld;     /* technically I guess I should use one byte & flags */
     int dirty_zon;     /* but there's never going to be that many zones */
     int dirty_obj;
     int dirty_mob;
     
     int top;             /* upper virtual limit for rooms in thiszone */
     int bottom;          /* the bottom of the rooms(low #/virtual) */
     int real_top;        /* for intelligent search */
     int real_bottom;
     int bot_of_mobt;     /* real range for mob's */
     int top_of_mobt;
     int bot_of_objt;     /* real range for obj's */
     int top_of_objt;

     int reset_mode;         /* conditions for reset (see below) */
     struct reset_com *cmd;  /* command table for reset          */
     int reset_num;         /* conditions for reset (see below) */
     /*
     *  Reset mode:                                   *
     *  0: Don't reset, and don't update age.         *
     *  1: Reset if no PC's are located in zone.      *
     *  2: Just reset.                                *
     *  -1: Zone offline for online creation purposes *
     *  -2: reserved (maybe for zone cmds online)     *
     */
};




/* for queueing zones for update   */
struct reset_q_element
{
     int zone_to_reset;            /* ref to zone_data */
     struct reset_q_element *next; 
};



/* structure for the update queue     */
struct reset_q_type
{
     struct reset_q_element *head;
     struct reset_q_element *tail;
} reset_q;



struct player_index_element
{
     char *name;
     int nr;
};


struct help_index_element
{
     char *keyword;
     long pos;
};

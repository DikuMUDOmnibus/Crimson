/* ************************************************************************
*  file: structs.h , Structures        .                  Part of DIKUMUD *
*  Usage: Declarations of central data structures                         *
************************************************************************* */

#include <sys/types.h>

typedef signed char sbyte;
typedef unsigned char ubyte;
typedef signed short int sh_int;
typedef unsigned short int ush_int;
typedef char bool;
typedef char byte;

#define PULSE_ZONE     240
#define PULSE_MOBILE    40
#define PULSE_VIOLENCE  12
#define WAIT_SEC       4
#define WAIT_ROUND     4
#define AUTOSAVE_DELAY 1500   /* in seconds */

#define MAX_STRING_LENGTH   4096
#define MAX_NAME_LENGTH       15 /* moved from comm.c */
#define MAX_INPUT_LENGTH     256 
#define MAX_MESSAGES          60
#define MAX_ITEMS            153
#define MAX_DESC            1024 

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR  75
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

/* The following defs are for obj_data  */

/* For 'type_flag' */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION    10 
#define ITEM_WORN      11
#define ITEM_OTHER     12
#define ITEM_TRASH     13
#define ITEM_TRAP      14
#define ITEM_CONTAINER 15
#define ITEM_NOTE      16
#define ITEM_DRINKCON  17
#define ITEM_KEY       18
#define ITEM_FOOD      19
#define ITEM_MONEY     20
#define ITEM_PEN       21
#define ITEM_BOAT      22
#define ITEM_ZCMD      23

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE          1<<0
#define ITEM_WEAR_FINGER   1<<1
#define ITEM_WEAR_NECK     1<<2
#define ITEM_WEAR_BODY     1<<3
#define ITEM_WEAR_HEAD     1<<4
#define ITEM_WEAR_LEGS     1<<5
#define ITEM_WEAR_FEET     1<<6
#define ITEM_WEAR_HANDS    1<<7
#define ITEM_WEAR_ARMS     1<<8
#define ITEM_WEAR_SHIELD   1<<9
#define ITEM_WEAR_ABOUT    1<<10
#define ITEM_WEAR_WAISTE   1<<11
#define ITEM_WEAR_WRIST    1<<12
#define ITEM_WIELD         1<<13
#define ITEM_HOLD          1<<14
#define ITEM_THROW         1<<15
#define ITEM_UNUSED_YET    1<<16
#define ITEM_USABLE        1<<17

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW         1<<0
#define ITEM_HUM          1<<1
#define ITEM_DARK         1<<2
#define ITEM_LOCK         1<<3
#define ITEM_EVIL         1<<4
#define ITEM_INVISIBLE    1<<5
#define ITEM_MAGIC        1<<6
#define ITEM_NODROP       1<<7
#define ITEM_BLESS        1<<8
#define ITEM_ANTI_GOOD    1<<9  /* not usable by good people    */
#define ITEM_ANTI_EVIL    1<<10 /* not usable by evil people    */
#define ITEM_ANTI_NEUTRAL 1<<11 /* not usable by neutral people */
#define ITEM_ANTI_MAGE    1<<12
#define ITEM_ANTI_THIEF   1<<13
#define ITEM_ANTI_CLERIC  1<<14
#define ITEM_ANTI_WARRIOR 1<<15
#define ITEM_ANTI_DRAGON  1<<16
#define ITEM_GOOD         1<<17
#define ITEM_NORENT       1<<18
#define ITEM_DIFFICULT    1<<19
#define ITEM_EXPERT       1<<20
#define ITEM_HEROIC       1<<21
#define ITEM_AVATAR       1<<22
#define ITEM_NOSEE        1<<23 /* cant EVER see it */

/* Some different kinds of weapons */

#define WEAPON_POISONOUS    1
#define WEAPON_BLIND             2
#define WEAPON_PARALYZE          4
#define WEAPON_SILVER            8
#define WEAPON_WOOD_STAKE       16
#define WEAPON_VAMPIRIC         32
#define WEAPON_DRAIN_MANA       64
#define WEAPON_GOOD       128
#define WEAPON_NEUTRAL         256
#define WEAPON_EVIL       512
#define WEAPON_GOOD_SLAYER    1024
#define WEAPON_EVIL_SLAYER    2048
#define WEAPON_UNDEAD_SLAYER  4096
#define WEAPON_DRAGON_SLAYER  8192
#define WEAPON_ANIMAL_SLAYER   16384
#define WEAPON_GIANT_SLAYER    32768

/* Some different kind of liquids */

#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_COKE       15

/* for containers  - value[1] */

#define CONT_CLOSEABLE      1
#define CONT_PICKPROOF      2
#define CONT_CLOSED         4
#define CONT_LOCKED         8

struct extra_descr_data
{
     char *keyword;                 /* Keyword in look/examine          */
     char *description;             /* What to see                      */
     struct extra_descr_data *next; /* Next in list                     */
};

#define MAX_OBJ_AFFECT 4         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OBJ_NOTIMER    -7000000

struct obj_flag_data
{
     int value[4];       /* Values of the item (see list)    */
     sbyte type_flag;     /* Type of item                     */
     int wear_flags;     /* Where you can wear it            */
     int extra_flags;    /* If it hums,glows etc             */
     int weight;         /* Weigt what else                  */
     int cost;           /* Value when sold (gp.)            */
     int cost_per_day;   /* Cost to keep pr. real day        */
     int timer;          /* Timer for object                 */
     long bitvector;     /* To set chars bits                */
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type {
     sbyte location;      /* Which ability to change (APPLY_XXX) */
     sbyte modifier;     /* How much it changes by              */
};

/* ======================== Structure for object ========================= */
struct obj_data
{
     sh_int item_number;            /* Where in data-base               */
     sh_int in_room;                /* In what room -1 when conta/carr  */ 
     struct obj_flag_data obj_flags;/* Object information               */
     struct obj_affected_type
         affected[MAX_OBJ_AFFECT];  /* Which abilities in PC to change  */

     char *name;                    /* Title of object :get etc.        */
     char *description ;            /* When in room                     */
     char *short_description;       /* when worn/carry/in cont.         */
     char *action_description;      /* What to write when used          */
     struct extra_descr_data *ex_description; /* extra descriptions     */
     struct char_data *carried_by;  /* Carried by :NULL in room/conta   */

     struct obj_data *in_obj;       /* In what object NULL when none    */
     struct obj_data *contains;     /* Contains objects                 */

     struct obj_data *next_content; /* For 'contains' lists             */
     struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= */

/* The following defs are for room_data  */

#define NOWHERE    -1    /* nil reference for room-database    */

/* Bitvector For 'room_flags' */

#define DARK           1
#define DEATH          2
#define NO_MOB         4
#define INDOORS        8
#define LAWFULL       16
#define NEUTRAL       32
#define CHAOTIC       64
#define NO_MAGIC     128
#define TUNNEL       256
#define PRIVATE      512
#define DRAIN_MAGIC 1024
#define HARMFULL1   2048
#define HARMFULL2   4096
#define NO_TELEPORT  1<<13

/* zone flags */
#define ZONE_TESTING 1<<0
#define ZONE_NOENTER 1<<1

/* For 'dir_option' */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR        1
#define EX_PICKPROOF     2
#define EX_LOCKED        4
#define EX_CLOSED        8
#define EX_HIDDEN       16

/* For 'Sector types' */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_UNDERWATER      8

struct room_direction_data
{
     char *general_description;       /* When look DIR.                  */ 
     char *keyword;                   /* for open/close                  */     
     sh_int exit_info;                /* Exit info                       */
     sh_int key;                            /* Key's number (-1 for no key)    */
     sh_int to_room;                  /* Where direction leeds (NOWHERE) */
};

/* ========================= Structure for room ========================== */
struct room_data
{
     sh_int number;               /* Rooms number                       */
     sh_int zone;                 /* Room zone (for resetting)          */
     int sector_type;             /* sector type (move/hide)            */
     char *name;                  /* Rooms name 'You are ...'           */
     char *description;           /* Shown when entered                 */
     struct extra_descr_data *ex_description; /* for examine/look       */
     struct room_direction_data *dir_option[6]; /* Directions           */
     sh_int room_flags;           /* DEATH,DARK ... etc                 */ 
     sbyte light;                  /* Number of lightsources in room     */
     int (*funct)();              /* special procedure                  */
     ush_int next;            /* used when zone is dirty */
         
     struct obj_data *contents;   /* List of items in room              */
     struct char_data *people;    /* List of NPC / PC in room           */
};
/* ======================================================================== */

/* The following defs and structures are related to char_data   */

/* For 'equipment' */

#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAISTE    13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WIELD          16
#define HOLD           17


/* For 'char_payer_data' */

#define MAX_TOUNGE  3     /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_SKILLS  100    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */ /* 53 */
#define MAX_WEAR    18
#define MAX_AFFECT  25    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

/* Predifined  conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2

/* Bitvector for 'affected_by' */
#define AFF_BLIND             1
#define AFF_INVISIBLE         2
#define AFF_DETECT_EVIL       4
#define AFF_DETECT_INVISIBLE  8
#define AFF_DETECT_MAGIC      16
#define AFF_SENSE_LIFE        32
#define AFF_HOLD              64
#define AFF_SANCTUARY         128
#define AFF_GROUP             256
#define AFF_CURSE             1024
#define AFF_FLAMING           2048
#define AFF_POISON            4096
#define AFF_PROTECT_EVIL      8192
#define AFF_PARALYSIS         16384
#define AFF_MORDEN_SWORD      32768
#define AFF_WIZINVIS          65536
#define AFF_SLEEP             131072
#define AFF_DODGE             262144
#define AFF_SNEAK             524288
#define AFF_HIDE              1048576
#define AFF_FEAR              2097152
#define AFF_CHARM             4194304
#define AFF_FOLLOW            8388608
#define AFF_BREATHWATER      16777216
#define AFF_DARKSIGHT        33554432
#define AFF_REGENERATION     67108864
#define AFF_MINOR_WARDS     134217728
#define AFF_MAJOR_WARDS     268435456
#define AFF_MAGIC_RESIST    536870912
#define AFF_MAGIC_IMMUNE   1073741824
/* modifiers to char's abilities */

#define APPLY_NONE              0
#define APPLY_STR               1
#define APPLY_DEX               2
#define APPLY_INT               3
#define APPLY_WIS               4
#define APPLY_CON               5
#define APPLY_SEX               6
#define APPLY_CLASS             7
#define APPLY_LEVEL             8
#define APPLY_AGE               9
#define APPLY_CHAR_WEIGHT      10
#define APPLY_CHAR_HEIGHT      11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_GOLD             15
#define APPLY_EXP              16
#define APPLY_AC               17
#define APPLY_ARMOR            17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PARA      20
#define APPLY_SAVING_ROD       21
#define APPLY_SAVING_PETRI     22
#define APPLY_SAVING_BREATH    23
#define APPLY_SAVING_SPELL     24
#define APPLY_EXTRA_ATTACKS    25
#define APPLY_DETECT_INVIS    26
#define APPLY_DARKSIGHT       27
#define APPLY_MAGIC_RESIST    28
#define APPLY_MAGIC_IMMUNE    29
#define APPLY_BREATHWATER     30
#define APPLY_DETECT_MAGIC    31
#define APPLY_SENSE_LIFE      32
#define APPLY_DETECT_EVIL     33
#define APPLY_SNEAK      34
#define APPLY_INVIS      35
#define APPLY_IMPROVED_INVIS  36
#define APPLY_REGENERATION    37
#define APPLY_MINOR_WARDS     38
#define APPLY_MAJOR_WARDS     39
#define APPLY_UNDETECT_INVIS    40
#define APPLY_GOD_STR           41
#define APPLY_GOD_DEX           42
#define APPLY_GOD_INT           43
#define APPLY_GOD_WIS           44
#define APPLY_GOD_CON           45




/* 'class' for PC's */
#define CLASS_MAGIC_USER  1
#define CLASS_CLERIC      2
#define CLASS_THIEF       3
#define CLASS_WARRIOR     4
#define CLASS_BARD        5
#define CLASS_KAI         6
#define CLASS_DRAKKHAR    7
#define CLASS_DKNIGHT     8
#define CLASS_EKNIGHT     9
#define CLASS_DRAGONW    10
#define CLASS_SAMURAI    11

/* sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */
#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_INCAP      2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8


/* for mobile actions: specials.act */
#define ACT_SPEC         1     /* special routine to be called if exist   */
#define ACT_SENTINEL     2     /* this mobile not to be moved             */
#define ACT_SCAVENGER    4     /* pick up stuff lying around              */
#define ACT_ISNPC        8     /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF  16     /* Set if a thief should NOT be killed     */
#define ACT_AGGRESSIVE  32     /* Set if automatic attack on NPC's        */
#define ACT_STAY_ZONE   64     /* MOB Must stay inside its own zone       */
#define ACT_WIMPY      128     /* MOB Will flee when injured, and if      */
                               /* aggressive only attack sleeping players */
#define ACT_MAGE       256
#define ACT_CLERIC     512
#define ACT_THIEF     1024
#define ACT_BREATH_FIRE 2048
#define ACT_BREATH_GAS  4096
#define ACT_BREATH_ACID 8192
#define ACT_BREATH_LIGHTNING 16384
#define ACT_BREATH_FROST     32768
#define ACT_POISONOUS        65536
#define ACT_PARALYZE         131072
#define ACT_NOCHARM         262144
#define ACT_DRAIN_XP        524288
#define CLASS_SHADOW       1048576
#define CLASS_VAMPIRE       2097152
#define CLASS_DRAGON       4194304
#define CLASS_GIANT        8388608
#define CLASS_ANIMAL       16777216
#define CLASS_UNDEAD      33554432


/* For players : specials.act */
#define PLR_BRIEF       1<<0
#define PLR_NOSHOUT     1<<1
#define PLR_COMPACT     1<<2
#define PLR_DONTSET     1<<3   /* Dont EVER set */
#define PLR_ANSI        1<<4
#define PLR_NOSUMMON    1<<5
#define PLR_NOGOSSIP    1<<6
#define PLR_NOAUCTION   1<<7
#define PLR_NOGRATS     1<<8
#define PLR_PKILLER     1<<9
#define PLR_ENFORCER    1<<10
#define PLR_NOTELL      1<<11
#define PLR_ASSIST      1<<12
#define PLR_AUTOASSIST  1<<12 /* same as assist */
#define PLR_AUTOSPLIT   1<<13
#define PLR_ATTACKER    1<<14
#define PLR_WIMPY       1<<15
#define PLR_AUTOFLEE    1<<15 /* same as wimpy */
#define PLR_SHOW_ROOM   1<<16
#define PLR_SHOW_HP     1<<17
#define PLR_SHOW_MANA   1<<18
#define PLR_SHOW_MOVE   1<<19
#define PLR_SHOW_EXITS  1<<20
#define PLR_NOHASSLE    1<<21
#define PLR_MUZZLE      1<<22
#define PLR_NOSYS       1<<23
#define PLR_NOIMM       1<<24
#define PLR_NOCLAN      1<<25
#define PLR_AUTOLOOT    1<<26
#define PLR_AUTOAGGR    1<<27
#define PLR_AUTOJUNK    1<<28
#define PLR_AUTOEAT     1<<29



/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data
{
     sbyte hours, day, month;
     sh_int year;
};

/* These data contain information about a players time data */
struct time_data
{
  signed long birth;    /* This represents the characters age                */
  time_t logon;    /* Time of the last logon (used to calculate played) */
  int played;      /* This is the total accumulated time played in secs */
};

struct char_player_data
{
     char *name;             /* PC / NPC s name (kill ...  )         */
     char *short_descr;  /* for 'actions'                        */
     char *long_descr;   /* for 'look'.. Only here for testing   */
     char *description;  /* Extra descriptions                   */
     char *title;        /* PC / NPC s title                     */
     sbyte sex;           /* PC / NPC s sex                       */
     sbyte class;         /* PC s class or NPC alignment          */
     sbyte level;         /* PC / NPC s level                     */
     int hometown;       /* PC s Hometown (zone)                 */
     bool talks[MAX_TOUNGE]; /* PC s Tounges 0 for NPC           */
     struct time_data time; /* PC s AGE in days                 */
     ubyte weight;       /* PC / NPC s weight                    */
     ubyte height;       /* PC / NPC s height                    */
     char *immortal_enter;      /* Poofin message                       */
     char *immortal_exit;       /* Poofout message                      */
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data
{
     sbyte str; 
     sbyte str_add;      /* 000 - 100 if strength 18             */
     sbyte intel;
     sbyte wis; 
     sbyte dex; 
     sbyte con; 
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data
{
     int mana;         
     int max_mana;     /* Not useable may be erased upon player file renewal */
     int hit;   
     int max_hit;      /* Max hit for NPC                         */
     int move;  
     int max_move;     /* Max move for NPC                        */

     sh_int armor;        /* Internal -100..100, external -10..10 AC */
     int gold;            /* Money carried                           */
     int bank;            /* NEW NEW NEW for gold in bank */                    /* TAKE OUT MAYBE */
     int exp;             /* The experience of the player            */

     sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
     sbyte damroll;       /* Any bonus or penalty to the damage roll */
};


struct char_special_data
{
     struct char_data *fighting; /* Opponent                             */
     struct char_data *hunting;  /* Hunting person..                     */

     long affected_by;        /* Bitvector for spells/skills affected by */ 

     sbyte position;           /* Standing or ...                         */
     sbyte default_pos;        /* Default position for NPC                */
     long act; /* NEW NEW NEW NEW MORE actions POSSIBLE NEW NEW NEW NEW  */
     /* ubyte act; */              /* flags for NPC behavior             */

     sbyte spells_to_learn;    /* How many can you learn yet this level   */

     int carry_weight;        /* Carried weight                          */
     sbyte carry_items;        /* Number of items carried                 */
     int timer;               /* Timer for update                        */
     sh_int was_in_room;      /* storage of location for linkdead people */
     sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)             */
     sbyte conditions[3];      /* Drunk full etc.                        */

     sbyte damnodice;           /* The number of damage dice's            */
     sbyte damsizedice;         /* The size of the damage dice's          */
     sbyte last_direction;      /* The last direction the monster went    */
     int attack_type;          /* The Attack Type Bitvector for NPC's    */
     int alignment;            /* +-1000 for alignments                  */

     long extra1;          /* extra flags that can be set */               /* TAKE OUT MAYBE */
     long extra2;             /* intended for quests */
     long extra3;
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data
{
     sbyte learned;           /* % chance for success 0 = not learned   */
     bool recognise;         /* If you can recognise the scroll etc.   */
};



/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type
{
     sbyte type;           /* The type of spell that caused this      */
     sh_int duration;      /* For how long its effects will last      */
     sbyte modifier;       /* This is added to apropriate ability     */
     sbyte location;        /* Tells which ability to change(APPLY_XXX)*/
     long bitvector;       /* Tells which bits to set (AFF_XXX)       */

     struct affected_type *next;
};

struct follow_type
{
     struct char_data *follower;
     struct follow_type *next;
};

/* ================== Structure for player/non-player ===================== */
struct char_data
{
     sh_int nr;                            /* monster nr (pos in file)      */
     sh_int in_room;                       /* Location                      */

     struct char_player_data player;       /* Normal data                   */
     struct char_ability_data abilities;   /* Abilities                     */
     struct char_ability_data tmpabilities;/* The abilities we will use     */
     struct char_point_data points;        /* Points                        */
     struct char_special_data specials;    /* Special plaing constants      */
     struct char_skill_data skills[MAX_SKILLS]; /* Skills                   */

     struct affected_type *affected;       /* affected by what spells       */
     struct obj_data *equipment[MAX_WEAR]; /* Equipment array               */

     struct obj_data *carrying;            /* Head of list                  */
     struct descriptor_data *desc;         /* NULL for mobiles              */

     struct char_data *next_in_room;     /* For room->people - list         */
     struct char_data *next;             /* For either monster or ppl-list  */
     struct char_data *next_fighting;    /* For fighting list               */

     struct follow_type *followers;        /* List of chars followers       */
     struct char_data *master;             /* Who is char following?        */
};


/* ======================================================================== */

/* How much light is in the land ? */

#define SUN_DARK    0
#define SUN_RISE    1
#define SUN_LIGHT   2
#define SUN_SET          3

/* And how is the sky ? */

#define SKY_CLOUDLESS    0
#define SKY_CLOUDY  1
#define SKY_RAINING 2
#define SKY_LIGHTNING    3

struct weather_data
{
     int pressure;  /* How is the pressure ( Mb ) */
     int change;    /* How fast and what way does it change. */
     int sky;  /* How is the sky. */
     int sunlight;  /* And how much sun. */
};


/* ***********************************************************************
*  file element for player file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */


struct char_file_u
{
     sbyte sex;
     sbyte class;
     sbyte level;
     time_t birth;  /* Time of birth of character     */
     int played;    /* Number of secs played in total */

     ubyte weight;
     ubyte height;

     char title[80];
     sh_int hometown;
     char description[MAX_DESC];
     bool talks[MAX_TOUNGE];

     char immortal_enter[80];  /* Poofin message                     */
     char immortal_exit[80];   /* Poofout message                    */

     sh_int load_room;            /* Which room to place char in           */

     struct char_ability_data abilities;

     struct char_point_data points;

     struct char_skill_data skills[MAX_SKILLS];

     struct affected_type affected[MAX_AFFECT];

     /* specials */

     sbyte spells_to_learn;  
     int alignment;     

     time_t last_logon;  /* Time (in secs) of last logon */
     long act;          /* ACT Flags                    */

     /* char data */
     char name[20];
     char pwd[11];
     sh_int apply_saving_throw[5];
     int conditions[3];
};



/* ***********************************************************************
*  file element for object file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */

/* this isnt used anymore I dont think */
#define MAX_OBJ_SAVE 25 /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */

struct obj_cost {
   int total_cost;
   int no_carried;
   bool ok;
};

struct obj_file_elem 
{
     sh_int item_number;

     int value[4];
     int extra_flags;
     int weight;
     int timer;
     long bitvector;
     struct obj_affected_type affected[MAX_OBJ_AFFECT];
};

struct obj_file_u
{
     char owner[20];    /* Name of player                     */
     int gold_left;     /* Number of goldcoins left at owner  */
     int total_cost;    /* The cost for all items, per day    */
     long last_update;  /* Time in seconds, when last updated */
     long rent_flags;   /* rent flags ie cryo-rent etc */
/*
     struct obj_file_elem objects[MAX_OBJ_SAVE];
*/
};

/* ***********************************************************
*  The following structures are related to descriptor_data   *
*********************************************************** */



struct txt_block
{
     char *text;
     struct txt_block *next;
};

struct txt_q
{
     struct txt_block *head;
     struct txt_block *tail;
};



/* modes of connectedness */

#define CON_PLYNG   0
#define CON_NME     1
#define CON_NMECNF  2
#define CON_PWDNRM  3
#define CON_PWDGET  4
#define CON_PWDCNF  5
#define CON_QSEX    6
#define CON_RMOTD   7
#define CON_SLCT    8
#define CON_EXDSCR  9
#define CON_QCLASS  10
#define CON_LDEAD   11
#define CON_PWDNEW  12
#define CON_PWDNCNF 13

struct snoop_data
{
     struct char_data *snooping;   
          /* Who is this char snooping */
     struct char_data *snoop_by;
          /* And who is snooping on this char */
};

struct descriptor_data
{
     int descriptor;                 /* file descriptor for socket */
     char host[50];                /* hostname                   */
     char pwd[12];                 /* password                   */
     int pos;                      /* position in player-file    */
     int connected;                /* mode of 'connectedness'    */
     int wait;                     /* wait for how many loops    */
     char *showstr_head;                /* for paging through texts   */
     char *showstr_point;                    /*       -                    */
     char **str;                   /* for the modify-str system  */
     int max_str;                  /* -                          */
     int prompt_mode;              /* control of prompt-printing */
     int prompt_cr;                /* control of prompt-printing */
     char buf[MAX_STRING_LENGTH];  /* buffer for raw input       */
     char last_input[MAX_INPUT_LENGTH];/* the last input         */
     struct txt_q output;          /* q of strings to send       */
     struct txt_q input;           /* q of unprocessed input     */
     struct char_data *character;  /* linked to char             */
     struct char_data *original;   /* original char              */
     struct snoop_data snoop;      /* to snoop people.          */
     struct descriptor_data *next; /* link to next descriptor    */
};

struct msg_type 
{
     char *attacker_msg;  /* message to attacker */
     char *victim_msg;    /* message to victim   */
     char *room_msg;      /* message to room     */
};

struct message_type
{
     struct msg_type die_msg;      /* messages when death            */
     struct msg_type miss_msg;     /* messages when miss             */
     struct msg_type hit_msg;      /* messages when hit              */
     struct msg_type sanctuary_msg;/* messages when hit on sanctuary */
     struct msg_type god_msg;      /* messages when hit on god       */
     struct message_type *next;/* to next messages of this kind.*/
};

struct message_list
{
     int a_type;               /* Attack type                               */
     int number_of_attacks;     /* How many attack messages to chose from. */
     struct message_type *msg; /* List of messages.                    */
};

struct dex_skill_type
{
     sh_int p_pocket;
     sh_int p_locks;
     sh_int traps;
     sh_int sneak;
     sh_int hide;
};

struct dex_app_type
{
     sh_int reaction;
     sh_int miss_att;
     sh_int defensive;
};

struct str_app_type
{
     sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
     sh_int todam;    /* Damage Bonus/Penalty                */
     sh_int carry_w;  /* Maximum weight that can be carrried */
     sh_int wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type
{
     sbyte bonus;       /* how many bonus skills a player can */
                       /* practice pr. level                 */
};

struct int_app_type
{
     sbyte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type
{
     sh_int hitp;
     sh_int shock;
};

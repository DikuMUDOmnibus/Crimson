/**************************************************************************/
/* the codes below are used for highlighting text, and is set for the     */
/* codes for VT-100 terminal emulation. If you are using a different      */
/* teminal type, replace the codes below with the correct codes           */
/**************************************************************************/
#define BLACK	"[30m" /* darker shades */
#define RED	"[0;31m" /* RED */
#define GREEN	"[0;32m" /* GREEN */
#define BROWN	"[0;33m" /* BROWN */
#define BLUE	"[0;34m" /* BLUE */
#define PURPLE	"[0;35m" /* PURPLE */
#define CYAN	"[0;36m" /* CYAN */
#define LGRAY	"[0;37m" /* LGRAY */
#define DGRAY	"[1;30m" /* DGRAY */
#define LRED	"[1;31m" /* LRED */
#define LGREEN	"[1;32m" /* LGREEN */
#define YELLOW	"[1;33m" /* YELLOW */
#define LBLUE	"[1;34m" /* LBLUE */
#define LPURPLE	"[1;35m" /* LPURPLE */
#define LCYAN	"[1;36m" /* LCYAN */
#define WHITE	"[1;37m" /* WHITE */
#define END	"[m" /* END */   

/* fighting messages */
#define CLR_DAM     LRED   	/* ouch I'm hit messages */
#define CLR_FIGHT   BROWN    	/* other peoples fight messages */
#define CLR_HIT     RED    	/* you hit the target messages */

/* rooms, mobs, objects */
#define CLR_TITLE   CYAN  	/* color of room titles */
#define CLR_DESC    LBLUE   	/* color of extra/room descriptions */
#define CLR_OBJ     GREEN 	/* color of objects */
#define CLR_MOB     GREEN 	/* color of mobs */
#define CLR_EVIL    LRED        /* color of evil mobs/players */
#define CLR_MAGIC   LBLUE       /* ...it glows blue desc */
#define CLR_GLOW    WHITE       /* ...it glows with a bright light desc */
#define CLR_SANC    YELLOW      /* ...glows with a bright light desc */
#define CLR_INVIS   "[1;30m[5m" /* color of invisible mobs/players/obj */
 
/* the communications channels */
#define CLR_COMM    LPURPLE 	/* say, tell, shout */
#define CLR_GOSSIP  YELLOW    	/* gossip  channel */
#define CLR_IMMTALK YELLOW    	/* immtalk channel */
#define CLR_AUCTION YELLOW    	/* auction channel */
#define CLR_GRATS   YELLOW    	/* grats   channel */
#define CLR_SYS     BROWN    	/* system  channel */
#define CLR_GROUP   LGREEN      /* group tell */

/* other miscellaneous colors */
#define CLR_ACTION  LCYAN	/* color of character actions */
#define CLR_ERROR   LRED        /* color of error messages or warnings */

#define REVERSE	"[7m"  /* test  - so you can more ansi.h to see it */
#define BLINK 	"[5m"  /* test */
#define BOLD 	"[1m"  /* BOLD */
#define FAINT 	"[2m"  /* FAINT */
#define ITALIC 	"[3m"  /* ITALIC */

#define MORE_END	"[m" /* END */   

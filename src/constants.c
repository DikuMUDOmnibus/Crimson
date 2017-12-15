/* ************************************************************************
*  file: constants.c                                      Part of DIKUMUD *
*  Usage: For constants used by the game.                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include "structs.h"
#include "limits.h"
#include "constants.h"

const char *ban_types[] = {
  "no",
  "new",
  "select",
  "all",
  "ERROR"
};

const char *spell_wear_off_msg[] = {
  "RESERVED DB.C",
  "You feel less protected.",    /* 1 */
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness dissolve.",
  "!Burning Hands!",
  "!Call Lightning!",
  "You feel more self-confident.",
  "You no longer feel drained.",     /* For Chill Touch used in energy drain */
  "You feel yourself exposed.",
  "!Color Spray!",   /* 10 */
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",
  "!Cure Light!",
  "You feel yourself again.",
  "You sense the red in your vision disappear.",
  "The detect invisible wears off.",
  "The detect magic wears off.",       /* 20 */
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",
  "!Fireball!",
  "!Harm!",
  "!Heal",
  "You feel yourself exposed.",
  "!Lightning Bolt!",      /* 30 */
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel more vulnerable to evil.",
  "!Remove Curse!",
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You feel weaker.",
  "!Summon!",              /* 40 */
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your surroundings.",
  "",  /* NO MESSAGE FOR SNEAK*/
  "!Hide!",
  "!Steal!",
  "!Backstab!",
  "!Pick Lock!",
  "!Kick!",    /* 50 */
  "!Bash!",
  "!Rescue!",
  "!IDENTIFY!"
  "!Cause Critic!",
  "!Cause Light!",
  "!Damage skill!",
  "!slash!",
  "!pierce!",
  "!bludgeon!",
  "!parry!",      /* 60 */
  "Whew you're too tired to dodge any more...",
  "!SCALES!"
  "!breath fire!",
  "!breath gas!",
  "!breath frost!",
  "!breath acid!",
  "!breath lightning!",
  "Things suddenly seem a lot darker now.",
  "!Second!",
  "!Third!",     /* 70 */
  "!Fourth!",
  "You feel yourself fade into existence.", 
  "!restoration!",
  "!offset1!",
  "!offset2!",  
  "You feel your recuperative powers reduced to normal.",
  "!minor wards!",
  "!major wards!",
  "Your magical sword dissipates into nothingness.",
  "Your phantasmal hammer winks out of existence.", 
  "!Turn undead!",
  "!Succor!",     /* 80 */
  "!Donate mana!",
  "You feel your magical link suddenly break...",
  "You feel more susceptible to magic...",
  "You feel more susceptible to magic...",
  "!dispel magic!", 
  "!conflagration!",
  "You begin to breath normally...", 
  "!sustenance!",    
};


const int rev_dir[] = 
{
   2,
   3,
   0,
   1, 
   5,
   4
}; 

const int movement_loss[]=
{
   1, /* Inside     */
   2,  /* City       */
   2,  /* Field      */
   3,  /* Forest     */
   4,  /* Hills      */
   6,  /* Mountains  */
  4,  /* Swimming   */
  1,  /* Unswimable */
 6 /* underwater */
};

const char *dirs[] = 
{
   "north",
   "east",
   "south",
   "west",
   "up",
   "down",
   "\n"
};

const char *weekdays[7] = { 
   "the Day of the Moon",
   "the Day of the Bull",
   "the Day of the Deception",
   "the Day of Thunder",
   "the Day of Freedom",
   "the day of the Great Gods",
   "the Day of the Sun" };

const char *month_name[17] = {
   "Month of Winter",           /* 0 */
   "Month of the Winter Wolf",
   "Month of the Frost Giant",
   "Month of the Old Forces",
   "Month of the Grand Struggle",
   "Month of the Spring",
   "Month of Nature",
   "Month of Futility",
   "Month of the Dragon",
   "Month of the Sun",
   "Month of the Heat",
   "Month of the Battle",
   "Month of the Dark Shades",
   "Month of the Shadows",
   "Month of the Long Shadows",
   "Month of the Ancient Darkness",
   "Month of the Great Evil"
};

const int sharp[] = {
   0,
   0,
   0,
   1,    /* Slashing */
   0,
   0,
   0,
   0,    /* Bludgeon */
   0,
   0,
   0,
   0 };  /* Pierce   */

const char *where[] = {
   "<used as light>    ",
   "<worn on finger>   ",
   "<worn on finger>   ",
   "<worn around neck> ",
   "<worn around neck> ",
   "<worn on body>     ",
   "<worn on head>     ",
   "<worn on legs>     ",
   "<worn on feet>     ",
   "<worn on hands>    ",
   "<worn on arms>     ",
   "<worn as shield>   ",
   "<worn about body>  ",
   "<worn about waist> ",
   "<worn on wrist>    ",
   "<worn on wrist>    ",
   "<wielded>          ",
   "<held>             " 
}; 


const char *loc[] = { /* mirrors wear */
   "light",
   "lfinger",
   "rfinger",
   "neck1",
   "neck2",
   "body",
   "head",
   "legs",
   "feet",
   "hands",
   "arms",
   "shield",
   "aboutbody",
   "waist",
   "lwrist",
   "rwrist",
   "wield",
   "held",
   "\n"
};

   
const char *drinks[]=
{
   "water",
   "beer",
   "wine",
   "ale",
   "dark ale",
   "whisky",
   "lemonade",
   "firebreather",
   "local speciality",
   "slime mold juice",
   "milk",
   "tea",
   "coffee",
   "blood",
   "salt water",
   "coca cola"
};

const char *drinknames[]=
{
   "water",
   "beer",
   "wine",
   "ale",
   "ale",
   "whisky",
   "lemonade",
   "firebreather",
   "local",
   "juice",
   "milk",
   "tea",
   "coffee",
   "blood",
   "salt",
   "cola"
};

const int drink_aff[][3] = {
   { 0,1,10 },  /* Water    */
   { 3,2,5 },   /* beer     */
   { 5,2,5 },   /* wine     */
   { 2,2,5 },   /* ale      */
   { 1,2,5 },   /* ale      */
   { 6,1,4 },   /* Whiskey  */
   { 0,1,8 },   /* lemonade */
   { 10,0,0 },  /* firebr   */
   { 3,3,3 },   /* local    */
   { 0,4,-8 },  /* juice    */
   { 0,3,6 },
   { 0,1,6 },
   { 0,1,6 },
   { 0,2,-1 },
   { 0,1,-2 },
   { 0,1,5 }
};

const char *color_liquid[]=
{
   "clear",
   "brown",
   "clear",
   "brown",
   "dark",
   "golden",
   "red",
   "green",
   "clear",
   "light green",
   "white",
   "brown",
   "black",
   "red",
   "clear",
   "black"
};

const char *fullness[] =
{
   "less than half ",
   "about half ",
   "more than half ",
   ""
};

/* The xp for each level is recalculated.
   In the new system, a nth level player will need the xp of killing 
   (5*n) mobs of same level to level, excluding xp gotten through 
   fighting.
   And xp for mob is 1500*level.
   Example: A level 25 player will need the xp of killing 75 mobs of
   level 25, i.e. 75*25*1500 = 2,812500 points.
   So the base xp for a nth level player is 1500*5*n(n+1)(2n+1)/6. 
   The old constants.c is still copied to constants.cOLD.
*/

const struct title_type titles[MAX_CLASS][MAX_LEV] = {
{ {"the Man","the Woman",0},           /* Mage */
  {"the Gomer of Magic","the Gomer of Magic",1},
  {"the Neophyte Mage","the Neophyte Magess",7500},
  {"the Beginner Mage","the Beginner Magess",37500},
  {"the Apprentice Magician","the Apprentice Magician",105000},
  {"the Scribe of Magic","the Scribess of Magic",225000},
  {"the Novice Magician","the Novice Magician",412500},
  {"the Practicer of Cult Arts","the Practicer of Cult Arts",682500},
  {"the Brewer","the Brewer",1050000},
  {"the Controller","the Controller",1530000},
  {"the Abjurer","the Abjuress",2137500},
  {"the Invoker","the Invoker",2887500},
  {"the Enchanter","the Enchantress",3795000},
  {"the Conjurer","the Conjuress",4875000},
  {"the Magician","the Magician",6142500},
  {"the Master Magician","the Master Magician",7612500},
  {"the Master Magician","the Master Magician",9300000},
  {"the Magus","the Craftess",11220000},
  {"the Warlock of People","the Witch of People",13387500},
  {"the Warlock of Things","the Witch of Things",15817500},
  {"the Warlock of Places","the Witch of Places",18525000},
  {"the Apprentice Wizard","the Apprentice Wizard",21525000},
  {"the Wizard of Ice","the Wizard of Ice",24832500},
  {"the Wizard of Dust","the Wizard of Dust",28462500},
  {"the Wizard of Mud","the Wizard of Mud",32430000},
  {"the Wizard of Plasma","the Wizard of Plasma",36750000},
  {"the Wizard of Steam","the Wizard of Steam",41437500},
  {"the Wizard of Lava","the Wizard of Lava",46507500},
  {"the Master Wizard","the Master Wizard",51975000},
  {"the High Wizard","the High Wizard",57855000},
  {"the Apprentice Sorceror","the Apprentice Sorceress",64162500},
  {"the Sorceror of Air","the Sorceress of Air",70912500},
  {"the Sorceror of Water","the Sorceress of Water",78120000},
  {"the Sorceror of Earth","the Sorceress of Earth",85800000},
  {"the Sorceror of Fire","the Sorceress of Fire",93967500},
  {"the Master Sorceror","the Master Sorceress",102637500},
  {"the Great Sorceror","the Great Sorceress",111825000},
  {"the Grand Sorceror","the Grand Sorceress",121545000},
  {"the High Sorceror","the High Sorceress",131812500},
  {"the Sorceror Lord","the Lady Sorceress",142642500},
  {"The Crimson Sorceror","the Crimson Sorceress",154050000},
  {"the Avatar","the Avatar ",180000000},
  {"the Immortal","the Immortal",200000000},
  {"the God","the Goddess",300000000},
  {"the Greater God","the Greater Goddess",400000000},
  {"the OverLord of CrimsonWorld","the Lady Ruler of CrimsonWorld",500000000},
  {"the Implementor","the Implementress",1000000000} },

{ {"the Man","the Woman",0},           /* Cleric */
  {"the Believer","the Believer",1},
  {"the Attendant","the Attendant",7500},
  {"the Acolyte","the Acolyte",37500},
  {"the Novice","the Novice",105000},
  {"the Missionary","the Missionary",225000},
  {"the Adept","the Adept",412500},
  {"the Deacon","the Deaconess",682500},
  {"the Vicar","the Vicaress",1050000},
  {"the Priest","the Priestess",1530000},
  {"the Minister","the Lady Minister",2137500},
  {"the Canon","the Canon",2887500},
  {"the Levite","the Levitess",3795000},
  {"the Curate","the Curess",4875000},
  {"the Monk","the Nunne",6142500},
  {"the Healer","the Healess",7612500},
  {"the Chaplain","the Chaplain",9300000},
  {"the Expositor","the Expositress",11220000},
  {"the Bishop","the Bishop",13387500},
  {"the Arch Bishop","the Arch Lady of the Church",15817500},
  {"the Patriarch","the Matriarch",18525000},
  {"the Prelate","the Prelatess",21525000},
  {"the Prelate of Iron","the Prelatess of Iron",24832500},
  {"the Prelate of Steel","the Prelatess of Steel",28462500},
  {"the Cardinal","the Cardinal",32430000},
  {"the Arch Cardinal","the Arch Cardinal",36750000},
  {"the High Cardinal","the High Cardinal",41437500},
  {"the Sanctified","the Sanctified",46507500},
  {"the Most Sanctified","the Most Sanctified",51975000},
  {"the Doctor of the Church","the Doctoress of the Church",57855000},
  {"the High Doctor of the Church","the High Doctoress of the Church",64162500},
  {"the Arch Doctor of the Church","the Arch Doctoress of the Church",70912500},
  {"the Doge","the Dogess",78120000},
  {"the High Doge","the High Dogess",85800000},
  {"the Arch Doge","the Arch Dogess",93967500},
  {"the Lord Doge","the Lord Dogess",102637500},
  {"the Papal","the Papal",111825000},
  {"the Holy","the Holy",121545000},
  {"the Most Holy","the Most Holy",131812500},
  {"the Sainted","the Sainted",142642500},
  {"the Divinely Ordered","the Divinely Ordered",154050000},
  {"the Avatar","the Avatar",180000000},
  {"the Immortal","the Immortal",200000000},
  {"the God","the Goddess",300000000},
  {"the Greater God","the Greater Goddess",400000000},
  {"the OverLord of CrimsonWorld","the Lady Ruler of CrimsonWorld",500000000},
  {"the Implementor","the Implementor",1000000000} },

{ {"the Man","the Woman",0},              /* THIEF */
  {"the Pilferer","the Pilferess",1},
  {"the Footpad","the Footpad",7500},
  {"the Filcher","the Filcheress",37500},
  {"the Pick-Pocket","the Pick-Pocket",105000},
  {"the Sneak","the Sneak",225000},
  {"the Pincher","the Pincheress",412500},
  {"the Cut-Purse","the Cut-Purse",682500},
  {"the Snatcher","the Snatcheress",1050000},
  {"the Sharper","the Sharpress",1530000},
  {"the Rogue","the Rogue",2137500},
  {"the Robber","the Robber",2887500},
  {"the Magsman","the Magswoman",3795000},
  {"the Highwayman","the Highwaywoman",4875000},
  {"the Burglar","the Burglaress",6142500},
  {"the Thief","the Thief",7612500},
  {"the Knifer","the Knifer",9300000},
  {"the Quick-Blade","the Quick-Blade",11220000},
  {"the Killer","the Murderess",13387500},
  {"the Brigand","the Brigand",15817500},
  {"the Cut-Throat","the Cut-Throat",18525000},
  {"the Seventh-Order Shadowwalker","the Seventh-Order Shadowwalker",21525000},
  {"the Sixth-Order Shadowwalker","the Sixth-Order Shadowwalker",24832500},
  {"the Fifth-Order Shadowwalker","the Fifth-Order Shadowwalker",28462500},
  {"the Fourth-Order Shadowwalker","the Fourth-Order Shadowwalker",32430000},
  {"the Third-Order Shadowwalker","the Third-Order Shadowwalker",36750000},
  {"the Second-Order Shadowwalker","the Second-Order Shadowwalker",41437500},
  {"the First-Order Shadowwalker","the First-Order Shadowwalker",46507500},
  {"the Fifth-Order Wraithkin","the Fifth-Order Wraithkin",51975000},
  {"the Fourth-Order Wraithkin","the Fourth-Order Wraithkin",57855000},
  {"the Third-Order Wraithkin","the Third-Order Wraithkin",64162500},
  {"the Second-Order Wraithkin","the Second-Order Wraithkin",70912500},
  {"the First-Order Wraithkin","the First-Order Wraithkin",78120000},
  {"the Third Shadowform","the Third Shadowform",85800000},
  {"the Second Shadowform","the Second Shadowform",93967500},
  {"the Shadowform","the Shadowform",102637500},
  {"the Concealed","the Concealed",111825000},
  {"the Kharass","the Kharassess",121545000},
  {"the Arcan","the Arcaness",131812500},
  {"the Lord Thief","the Lady Thief",142642500},
  {"the Silent Horror","the Silent Horror",154050000},
  {"the Avatar","the Avatar",180000000},
  {"the Immortal","the Immortal",200000000},
  {"the God","the Goddess",300000000},
  {"the Greater God","the Greater Goddess",400000000},
  {"the OverLord of CrimsonWorld","the Lady Ruler of CrimsonWorld",500000000},
  {"the Implementor","the Implementor",1000000000} },

{ {"the Man","the Woman",0},              /* Warrior */
  {"the Swordpupil","the Swordpupil",1},
  {"the Recruit","the Recruit",7500},
  {"the Sentry","the Sentress",37500},
  {"the Fighter","the Fighter",105000},
  {"the Soldier","the Soldier",225000},
  {"the Warrior","the Warrior",412500},
  {"the Veteran","the Veteran",682500},
  {"the Swordsman","the Swordswoman",1050000},
  {"the Fencer","the Fenceress",1530000},
  {"the Combatant","the Combatess",2137500},
  {"the Hero","the Heroine",2887500},
  {"the Myrmidon","the Myrmidon",3795000},
  {"the Swashbuckler","the Swashbuckleress",4875000},
  {"the Mercenary","the Mercenaress",6142500},
  {"the Swordmaster","the Swordmistress",7612500},
  {"the Lieutenant","the Lieutenant",9300000},
  {"the Champion","the Lady Champion",11220000},
  {"the Dragoon","the Lady Dragoon",13387500},
  {"the Cavalier","the Cavalier",15817500},
  {"the Knight","the Lady Knight",18525000},
  {"the Knight","the Lady Knight",21525000},
  {"the Knight of the Silver Sword","the Lady Knight of the Silver Sword",24832500},
  {"the Knight of the Gold Sword","the Lady Knight of the Gold Sword",28462500},
  {"the Knight of the Platinum Sword","the Lady Knight of the Platinum Sword",32430000},
  {"the Knight of the Silver Crown","the Lady Knight of the Silver Crown",36750000},
  {"the Knight of the Gold Crown","the Lady Knight of the Gold Crown",41437500},
  {"the Knight of the Platinum Crown","the Lady Knight of the Platinum Crown",46507500},
  {"the Knight of the Serpent","the Lady Knight of the Serpent",51975000},
  {"the Knight of the Hawk","the Lady Knight of the Hawk",57855000},
  {"the Knight of the Falcon","the Lady Knight of the Falcon",64162500},
  {"the Knight of the Crescent","the Lady Knight of the Crescent",70912500},
  {"the Knight of the Star","the Lady Knight of the Star",78120000},
  {"the Knight of the Moon","the Lady Knight of the Moon",85800000},
  {"the Knight of the Sun","the Lady Knight of the Sun",93967500},
  {"the Knight of the Eagle","the Lady Knight of the Eagle",102637500},
  {"the Knight of the Talon","the Lady Knight of the Talon",111825000},
  {"the Knight of the Dawn","the Lady Knight of the Dawn",121545000},
  {"the Knight of the Rose","the Lady Knight of the Rose",131812500},
  {"the Knight of the Dragon","the Lady Knight of the Dragon",142642500},
  {"the Lord","the Lord",154050000},
  {"the Avatar","the Avatar",180000000},
  {"the Immortal","the Immortal",200000000},
  {"the God","the Goddess",300000000},
  {"the Greater God","the Greater Goddess",400000000},
  {"the OverLord of CrimsonWorld","the Lady Ruler of CrimsonWorld",500000000},
  {"the Implementor","the Implementor",1000000000} },

{ {"the Man","the Woman",0},           /* Bard */
  {"the Novice Bard","the Novice Bard",1},
  {"the Apprentice Bard","the Apprentice Bard",7500},
  {"the Bard","the Bard",37500},
  {"the Novice Storyteller","the Novice Storytelleress",105000},
  {"the Storyteller","the Storytelleress",225000},
  {"the High Storyteller","the High Storytelleress",412500},
  {"the Novice Taleweaver","the Novice Taleweaver",682500},
  {"the Taleweaver","the Taleweaver",1050000},
  {"the High Taleweaver","the High Taleweaver",1530000},
  {"the Novice Thoughtspinner","the Novice Thoughtspinneress",2137500},
  {"the Thoughtspinner","the Thoughtspinneress",2887500},
  {"the High Thoughtspinner","the High Thoughtspinneress",3795000},
  {"the Journeyman","the Journeywoman",4875000},
  {"the Initiated Journeyman","the Initiated Journeywoman",6142500},
  {"the Wanderer","the Wanderess",7612500},
  {"the Wordweaver","the Wordweaveress",9300000},
  {"the High Wordweaver","the High Wordweaveress",11220000},
  {"the Novice Minstrel","the Novice Lady Minstrel",13387500},
  {"the Minstrel","the Lady Minstrel",15817500},
  {"the Arch Minstrel","the Arch Lady Minstrel",18525000},
  {"the Novice Troubadour","the Novice Troubador",21525000},
  {"the Troubador","the Troubador",24832500},
  {"the Troubador of Tears","the Troubador of Tears",28462500},
  {"the Troubador of Roses","the Troubador of Roses",32430000},
  {"the Troubador of Dew","the Troubador of Dew",36750000},
  {"the Troubador of Night","the Troubador of Night",41437500},
  {"the Troubador of Tempests","the Troubador of Tempests",46507500},
  {"the Troubador of Dreams","the Troubador of Dreams",51975000},
  {"the Troubador of Sighs","the Troubador of Sighs",57855000},
  {"the Troubador of Stars","the Troubador of Stars",64162500},
  {"the Troubador of Tales","the Troubador of Tales",70912500},
  {"the Troubador of Thoughts","the Troubador of Thoughts",78120000},
  {"the Troubador of Sunrise","the Troubador of Sunrise",85800000},
  {"the Troubador of Sight","the Troubador of Sight",93967500},
  {"the Troubador of Peace","the Troubador of Peace",102637500},
  {"the Troubador of Earth","the Troubador of Earth",111825000},
  {"the Troubador of Sea","the Troubador of Sea",121545000},
  {"the Troubador of Flame","the Troubador of Flame",131812500},
  {"the Troubador of Sky","the Troubador of Sky",142642500},
  {"the Lord Bard","the Lord Bard",154050000},
  {"the Avatar","the Avatar",180000000},
  {"the Immortal","the Immortal",200000000},
  {"the God","the Goddess",300000000},
  {"the Greater God","the Greater Goddess",400000000},
  {"the OverLord of CrimsonWorld","the Lady Ruler of CrimsonWorld",500000000},
  {"the Implementor","the Implementor",1000000000} },

{ {"the Man","the Woman",0},           /* Kai */
  {"the Third Initiate","the Third Initiatess",1},
  {"the Second Initiate","the Second Initiatess",7500},
  {"the Initiate","the Initiate",37500},
  {"the Third Adept","the Third Adeptess",105000},
  {"the Second Adept","the Second Adeptess",225000},
  {"the Adept","the Adept",412500},
  {"the Veteran","the Veteran",682500},
  {"the Swordsman","the Swordswoman",1050000},
  {"the Fencer","the Fenceress",1530000},
  {"the Combatant","the Combatess",2137500},
  {"the Hero","the Heroine",2887500},
  {"the Myrmidon","the Myrmidon",3795000},
  {"the Swashbuckler","the Swashbuckleress",4875000},
  {"the Mercenary","the Mercenaress",6142500},
  {"the Swordmaster","the Swordmistress",7612500},
  {"the Lieutenant","the Lieutenant",9300000},
  {"the Champion","the Lady Champion",11220000},
  {"the Dragoon","the Lady Dragoon",13387500},
  {"the Cavalier","the Cavalier",15817500},
  {"the Knight","the Lady Knight",18525000},
  {"the Warmarn","the Warmarn",21525000},
  {"the Warmarn of Wood","the Warmarn of Wood",24832500},
  {"the Warmarn of Fire","the Warmarn of Fire",28462500},
  {"the Warmarn of Steel","the Warmarn of Steel",32430000},
  {"the Warmarn of Water","the Warmarn of Water",36750000},
  {"The 26th level Hero","The 26th Level Heroine",41437500},
  {"The 27th level Hero","The 27th Level Heroine",46507500},
  {"The 28th level Hero","The 28th Level Heroine",51975000},
  {"The 29th level Hero","The 29th Level Heroine",57855000},
  {"The 30th level Hero","The 30th Level Heroine",64162500},
  {"The 31st level Hero","The 31st Level Heroine",70912500},
  {"The 32nd level Hero","The 32nd Level Heroine",78120000},
  {"The 33rd level Hero","The 33rd Level Heroine",85800000},
  {"The 34th level Hero","The 34th Level Heroine",93967500},
  {"The 35th level Hero","The 35th Level Heroine",102637500},
  {"The 36th level Hero","The 36th Level Heroine",111825000},
  {"The 37th level Hero","The 37th Level Heroine",121545000},
  {"The 38th level Hero","The 38th Level Heroine",131812500},
  {"The 39th level Hero","The 39th Level Heroine",142642500},
  {"The 40th level Hero","The 40th Level Heroine",154050000},
  {"the Avatar","the Avatar",180000000},
  {"the Immortal","the Immortal",200000000},
  {"the God","the Goddess",300000000},
  {"the Greater God","the Greater Goddess",400000000},
  {"the OverLord of CrimsonWorld","the Lady Ruler of CrimsonWorld",500000000},
  {"the Implementor","the Implementor",1000000000} },

{ {"the Man","the Woman",0},
  {"the Anti-Paladin","the Anti-Paladin",1},      /* Drakkhar Warriors */
  {"the Anti-Paladin","the Anti-Paladin",7500},
  {"the Anti-Paladin","the Anti-Paladin",37500},
  {"the Fighter","the Fighter",105000},
  {"the Soldier","the Soldier",225000},
  {"the Warrior","the Warrior",412500},
  {"the Veteran","the Veteran",682500},
  {"the Swordsman","the Swordswoman",1050000},
  {"the Fencer","the Fenceress",1530000},
  {"the Combatant","the Combatess",2137500},
  {"the Hero","the Heroine",2887500},
  {"the Myrmidon","the Myrmidon",3795000},
  {"the Swashbuckler","the Swashbuckleress",4875000},
  {"the Mercenary","the Mercenaress",6142500},
  {"the Swordmaster","the Swordmistress",7612500},
  {"the Lieutenant","the Lieutenant",9300000},
  {"the Champion","the Lady Champion",11220000},
  {"the Dragoon","the Lady Dragoon",13387500},
  {"the Cavalier","the Cavalier",15817500},
  {"the Knight","the Lady Knight",18525000},
  {"The 21st level Hero","The 21st Level Heroine",21525000},
  {"The 22nd level Hero","The 22nd Level Heroine",24832500},
  {"The 23rd level Hero","The 23rd Level Heroine",28462500},
  {"The 24th level Hero","The 24th Level Heroine",32430000},
  {"The 25th level Hero","The 25th Level Heroine",36750000},
  {"The 26th level Hero","The 26th Level Heroine",41437500},
  {"The 27th level Hero","The 27th Level Heroine",46507500},
  {"The 28th level Hero","The 28th Level Heroine",51975000},
  {"The 29th level Hero","The 29th Level Heroine",57855000},
  {"The 30th level Hero","The 30th Level Heroine",64162500},
  {"The 31st level Hero","The 31st Level Heroine",70912500},
  {"The 32nd level Hero","The 32nd Level Heroine",78120000},
  {"The 33rd level Hero","The 33rd Level Heroine",85800000},
  {"The 34th level Hero","The 34th Level Heroine",93967500},
  {"The 35th level Hero","The 35th Level Heroine",102637500},
  {"The 36th level Hero","The 36th Level Heroine",111825000},
  {"The 37th level Hero","The 37th Level Heroine",121545000},
  {"The 38th level Hero","The 38th Level Heroine",131812500},
  {"The 39th level Hero","The 39th Level Heroine",142642500},
  {"The 40th level Hero","The 40th Level Heroine",154050000},
  {"the Avatar","the Avatar",180000000},
  {"the Immortal","the Immortal",200000000},
  {"the God","the Goddess",300000000},
  {"the Greater God","the Greater Goddess",400000000},
  {"the OverLord of CrimsonWorld","the Lady Ruler of CrimsonWorld",500000000},
  {"the Implementor","the Implementor",1000000000} },

{ {"the Man","the Woman",0},        /* Demon Knight */
  {"the Demon Knight","the Demon Knight",1},
  {"the Demon Knight","the Demon Knight",7500},
  {"the Demon Knight","the Demon Knight",37500},
  {"the Fighter","the Fighter",105000},
  {"the Soldier","the Soldier",225000},
  {"the Warrior","the Warrior",412500},
  {"the Veteran","the Veteran",682500},
  {"the Swordsman","the Swordswoman",1050000},
  {"the Fencer","the Fenceress",1530000},
  {"the Combatant","the Combatess",2137500},
  {"the Hero","the Heroine",2887500},
  {"the Myrmidon","the Myrmidon",3795000},
  {"the Swashbuckler","the Swashbuckleress",4875000},
  {"the Mercenary","the Mercenaress",6142500},
  {"the Swordmaster","the Swordmistress",7612500},
  {"the Lieutenant","the Lieutenant",9300000},
  {"the Champion","the Lady Champion",11220000},
  {"the Dragoon","the Lady Dragoon",13387500},
  {"the Cavalier","the Cavalier",15817500},
  {"the Knight","the Lady Knight",18525000},
  {"The 21st level Hero","The 21st Level Heroine",21525000},
  {"The 22nd level Hero","The 22nd Level Heroine",24832500},
  {"The 23rd level Hero","The 23rd Level Heroine",28462500},
  {"The 24th level Hero","The 24th Level Heroine",32430000},
  {"The 25th level Hero","The 25th Level Heroine",36750000},
  {"The 26th level Hero","The 26th Level Heroine",41437500},
  {"The 27th level Hero","The 27th Level Heroine",46507500},
  {"The 28th level Hero","The 28th Level Heroine",51975000},
  {"The 29th level Hero","The 29th Level Heroine",57855000},
  {"The 30th level Hero","The 30th Level Heroine",64162500},
  {"The 31st level Hero","The 31st Level Heroine",70912500},
  {"The 32nd level Hero","The 32nd Level Heroine",78120000},
  {"The 33rd level Hero","The 33rd Level Heroine",85800000},
  {"The 34th level Hero","The 34th Level Heroine",93967500},
  {"The 35th level Hero","The 35th Level Heroine",102637500},
  {"The 36th level Hero","The 36th Level Heroine",111825000},
  {"The 37th level Hero","The 37th Level Heroine",121545000},
  {"The 38th level Hero","The 38th Level Heroine",131812500},
  {"The 39th level Hero","The 39th Level Heroine",142642500},
  {"The 40th level Hero","The 40th Level Heroine",154050000},
  {"the Avatar","the Avatar",180000000},
  {"the Immortal","the Immortal",200000000},
  {"the God","the Goddess",300000000},
  {"the Greater God","the Greater Goddess",400000000},
  {"the OverLord of CrimsonWorld","the Lady Ruler of CrimsonWorld",500000000},
  {"the Implementor","the Implementor",1000000000} },

{ {"the Man","the Woman",0},           /* Elfin Knight */
  {"the Elfin Knight","the Elfin Knight",1},
  {"the Elfin Knight","the Elfin Knight",7500},
  {"the Elfin Knight","the Elfin Knight",37500},
  {"the Fighter","the Fighter",105000},
  {"the Soldier","the Soldier",225000},
  {"the Warrior","the Warrior",412500},
  {"the Veteran","the Veteran",682500},
  {"the Swordsman","the Swordswoman",1050000},
  {"the Fencer","the Fenceress",1530000},
  {"the Combatant","the Combatess",2137500},
  {"the Hero","the Heroine",2887500},
  {"the Myrmidon","the Myrmidon",3795000},
  {"the Swashbuckler","the Swashbuckleress",4875000},
  {"the Mercenary","the Mercenaress",6142500},
  {"the Swordmaster","the Swordmistress",7612500},
  {"the Lieutenant","the Lieutenant",9300000},
  {"the Champion","the Lady Champion",11220000},
  {"the Dragoon","the Lady Dragoon",13387500},
  {"the Cavalier","the Cavalier",15817500},
  {"the Knight","the Lady Knight",18525000},
  {"The 21st level Hero","The 21st Level Heroine",21525000},
  {"The 22nd level Hero","The 22nd Level Heroine",24832500},
  {"The 23rd level Hero","The 23rd Level Heroine",28462500},
  {"The 24th level Hero","The 24th Level Heroine",32430000},
  {"The 25th level Hero","The 25th Level Heroine",36750000},
  {"The 26th level Hero","The 26th Level Heroine",41437500},
  {"The 27th level Hero","The 27th Level Heroine",46507500},
  {"The 28th level Hero","The 28th Level Heroine",51975000},
  {"The 29th level Hero","The 29th Level Heroine",57855000},
  {"The 30th level Hero","The 30th Level Heroine",64162500},
  {"The 31st level Hero","The 31st Level Heroine",70912500},
  {"The 32nd level Hero","The 32nd Level Heroine",78120000},
  {"The 33rd level Hero","The 33rd Level Heroine",85800000},
  {"The 34th level Hero","The 34th Level Heroine",93967500},
  {"The 35th level Hero","The 35th Level Heroine",102637500},
  {"The 36th level Hero","The 36th Level Heroine",111825000},
  {"The 37th level Hero","The 37th Level Heroine",121545000},
  {"The 38th level Hero","The 38th Level Heroine",131812500},
  {"The 39th level Hero","The 39th Level Heroine",142642500},
  {"The 40th level Hero","The 40th Level Heroine",154050000},
  {"the Avatar","the Avatar",180000000},
  {"the Immortal","the Immortal",200000000},
  {"the God","the Goddess",300000000},
  {"the Greater God","the Greater Goddess",400000000},
  {"the OverLord of CrimsonWorld","the Lady Ruler of CrimsonWorld",500000000},
  {"the Implementor","the Implementor",1000000000} },

{ {"the Man","the Woman",0},           /* Dragon-Kin */
  {"the Dragon-Kin Warrior","the Dragon-Kin Warrior",1},
  {"the Dragon-Kin Warrior","the Dragon-Kin Warrior",7500},
  {"the Dragon-Kin Warrior","the Dragon-Kin Warrior",37500},
  {"the Fighter","the Fighter",105000},
  {"the Soldier","the Soldier",225000},
  {"the Warrior","the Warrior",412500},
  {"the Veteran","the Veteran",682500},
  {"the Swordsman","the Swordswoman",1050000},
  {"the Fencer","the Fenceress",1530000},
  {"the Combatant","the Combatess",2137500},
  {"the Hero","the Heroine",2887500},
  {"the Myrmidon","the Myrmidon",3795000},
  {"the Swashbuckler","the Swashbuckleress",4875000},
  {"the Mercenary","the Mercenaress",6142500},
  {"the Swordmaster","the Swordmistress",7612500},
  {"the Lieutenant","the Lieutenant",9300000},
  {"the Champion","the Lady Champion",11220000},
  {"the Dragoon","the Lady Dragoon",13387500},
  {"the Cavalier","the Cavalier",15817500},
  {"the Knight","the Lady Knight",18525000},
  {"The 21st level Hero","The 21st Level Heroine",21525000},
  {"The 22nd level Hero","The 22nd Level Heroine",24832500},
  {"The 23rd level Hero","The 23rd Level Heroine",28462500},
  {"The 24th level Hero","The 24th Level Heroine",32430000},
  {"The 25th level Hero","The 25th Level Heroine",36750000},
  {"The 26th level Hero","The 26th Level Heroine",41437500},
  {"The 27th level Hero","The 27th Level Heroine",46507500},
  {"The 28th level Hero","The 28th Level Heroine",51975000},
  {"The 29th level Hero","The 29th Level Heroine",57855000},
  {"The 30th level Hero","The 30th Level Heroine",64162500},
  {"The 31st level Hero","The 31st Level Heroine",70912500},
  {"The 32nd level Hero","The 32nd Level Heroine",78120000},
  {"The 33rd level Hero","The 33rd Level Heroine",85800000},
  {"The 34th level Hero","The 34th Level Heroine",93967500},
  {"The 35th level Hero","The 35th Level Heroine",102637500},
  {"The 36th level Hero","The 36th Level Heroine",111825000},
  {"The 37th level Hero","The 37th Level Heroine",121545000},
  {"The 38th level Hero","The 38th Level Heroine",131812500},
  {"The 39th level Hero","The 39th Level Heroine",142642500},
  {"The 40th level Hero","The 40th Level Heroine",154050000},
  {"the Avatar","the Avatar",180000000},
  {"the Immortal","the Immortal",200000000},
  {"the God","the Goddess",300000000},
  {"the Greater God","the Greater Goddess",400000000},
  {"the OverLord of CrimsonWorld","the Lady Ruler of CrimsonWorld",500000000},
  {"the Implementor","the Implementor",1000000000} }

};

const char *item_types[] = {
   "UNDEFINED",
   "LIGHT",
   "SCROLL",
   "WAND",
   "STAFF",
   "WEAPON",
   "!FIRE WEAPON",
   "!MISSILE",
   "TREASURE",
   "ARMOR",
   "POTION",
   "WORN",
   "OTHER",
   "TRASH",
   "TRAP",
   "CONTAINER",
   "NOTE",
   "DRINKCON",
   "KEY",
   "FOOD",
   "MONEY",
   "PEN",
   "BOAT",
   "!ZCMD",
   "\n"
};

const char *wear_bits[] = {
   "TAKE",
   "FINGER",
   "NECK",
   "BODY",
   "HEAD",
   "LEGS",
   "FEET",
   "HANDS",
   "ARMS",
   "SHIELD",
   "ABOUT",
   "WAIST",
   "WRIST",
   "WIELD",
   "HOLD",
   "THROW",
   "LIGHT-SOURCE",
   "USABLE",
   "\n"
};

const char *extra_bits[] = {
   "GLOW",
   "HUM",
   "DARK",
   "LOCK",
   "EVIL",
   "INVISIBLE",
   "MAGIC",
   "NODROP",
   "BLESS",
   "ANTI-GOOD",
   "ANTI-EVIL",
   "ANTI-NEUTRAL",
   "ANTI-MAGE",
   "ANTI-THIEF",
   "ANTI-CLERIC",
   "ANTI-WARRIOR",
   "ANTI-DRAGON",
   "GOOD",
   "NORENT",
   "DIFFICULT",
   "EXPERT",
   "HEROIC",
   "AVATAR",
   "NOSEE",
   "\n"
};


const char *weapon_bits[] = {
   "POISONOUS",
   "BLINDING",
   "!PARALYZING",
   "SILVER",
   "WOOD-STAKE",
   "VAMPIRIC",
   "MANA-DRAINING",
   "GOOD",
   "NEUTRAL",
   "EVIL",
   "GOOD-SLAYER",
   "EVIL-SLAYER",
   "UNDEAD-SLAYER",
   "DRAGON-SLAYER",
   "ANIMAL-SLAYER",
   "GIANT-SLAYER",
   "\n"
};


const char *room_bits[] = {
   "DARK",
   "DEATH",
   "NO_MOB",
   "INDOORS",
   "ANTI-LAWFULL",
   "ANTI-NEUTRAL",
   "ANTI-CHAOTIC",
   "!NO_MAGIC",
   "!TUNNEL",
   "PRIVATE",
   "DRAINS_MAGIC",
   "HARMFULL1",
   "HARMFULL2",
   "NO_TELEPORT",
   "\n"
};

const char *zflag_bits[] = {
   "TESTING",
   "NOENTER",
   "\n"
};

const char *dirty_bits[] = {
   "UNSAVED",
   "SCRAMBLED",
   "SHOWN",
   "\n"
};

const char *exit_bits[] = {
   "IS-DOOR",
   "PICKPROOF",
   "LOCKED",
   "CLOSED",
   "HIDDEN",
   "\n"
};

const char *sector_types[] = {
   "Inside",
   "City",
   "Field",
   "Forest",
   "Hills",
   "Mountains",
   "WaterSwim",
   "WaterNoSwim",
   "Underwater",
   "\n"
};

const char *equipment_types[] = {
   "Special",
   "Worn on right finger",
   "Worn on left finger",
   "First worn around Neck",
   "Second worn around Neck",
   "Worn on body",
   "Worn on head",
   "Worn on legs",
   "Worn on feet",
   "Worn on hands",
   "Worn on arms",
   "Worn as shield",
   "Worn about body",
   "Worn around waist",
   "Worn around right wrist",
   "Worn around left wrist",
   "Wielded",
   "Held",
   "\n"
};
   
const char *affected_bits[] = 
{  "BLIND",
   "INVISIBLE",
   "DETECT-EVIL",
   "DETECT-INVISIBLE",
   "DETECT-MAGIC",
   "SENSE-LIFE",
   "HOLD",
   "SANCTUARY",
   "GROUP",
   "UNUSED",
   "CURSE",
   "FLAMING-HANDS",
   "POISON",
   "PROTECT-EVIL",
   "!PARALYSIS",
   "!MORDENS-SWORD",
   "WIZINVIS",
   "SLEEP",
   "!DODGE",
   "SNEAK",
   "HIDE",
   "FEAR",
   "CHARM",
   "FOLLOW",
   "BREATHWATER",  /*not used , I think... */
   "DARKSIGHT",
   "REGENERATION",
   "!MINOR-WARDS",
   "!MAJOR-WARDS",
   "MAGIC-RESIST",
   "MAGIC-IMMUNE",
   "\n"
};

const char *apply_types[] = {
   "NONE",
   "STR",
   "DEX",
   "INT",
   "WIS",
   "CON",
   "!SEX",
   "!CLASS",
   "!LEVEL",
   "AGE",
   "!CHAR_WEIGHT",
   "!CHAR_HEIGHT",
   "MANA",
   "HIT",
   "MOVE",
   "!GOLD",
   "!EXP",
   "AC",
   "HITROLL",
   "DAMROLL",
   "SAVING_PARA",
   "SAVING_ROD",
   "SAVING_PETRI",
   "SAVING_BREATH",
   "SAVING_SPELL",
   "!EXTRA_ATTACKS",
   "DETECT-INVIS",
   "DARKSIGHT",
   "MAGIC-RESIST",
   "MAGIC-IMMUNE",
   "BREATHWATER",
   "DETECT-MAGIC",
   "SENSE-LIFE",
   "DETECT-EVIL",
   "!SNEAK",
   "INVISIBILITY",
   "IMPROVED-INVIS",
   "REGENERATION",
   "!MINOR-WARDS",
   "!MAJOR-WARDS",
   "UNDETECTABLE-INVIS",
   "GOD_STR",
   "GOD_DEX",
   "GOD_INT",
   "GOD_WIS",
   "GOD_CON",
   "\n"
};

const char *pc_class_types[] = {
   "UNDEFINED",
   "Magic User",
   "Cleric",
   "Thief",
   "Warrior",
   "Bard",
   "Kai ", /* must be minimum 4 chars */
   "Drakkhar",
   "Demon-Knight",
   "Elfin-Knight",
   "Dragon-Kin Warrior",
   "\n"
};

const char *npc_class_types[] = { /* Not used anywhere yet */
   "Normal",
   "Undead",
   "Humanoid",
   "Animal",
   "Dragon",
   "Giant",
   "\n"
};

const char *action_bits[] = {
   "SPEC",
   "SENTINEL",
   "SCAVENGER",
   "ISNPC",
   "NICE-THIEF",
   "AGGRESSIVE",
   "STAY-ZONE",
   "WIMPY",
   "MAGE",
   "PRIEST",
   "THIEF",
   "BREATHES-FIRE",
   "BREATHES-GAS",
   "BREATHES-ACID",
   "BREATHES-LIGHTNING",
   "BREATHES-FROST",
   "POISONOUS",
   "!PARALYZING",
   "NOCHARM",
   "DRAINS-XP",
   "CLASS-SHADOW",
   "CLASS-VAMPIRE",
   "CLASS-DRAGON",
   "CLASS-GIANT",
   "CLASS-ANIMAL",
   "CLASS-UNDEAD",
   "\n"
};


const char *player_bits[] = {
   "BRIEF",
   "NOSHOUT",
   "COMPACT",
   "DONTSET",
   "ANSI",
   "NOSUMMON",
   "NOGOSSIP",
   "NOAUCTION",
   "NOGRATS",
   "PKILLER",
   "ENFORCER",
   "NOTELL",
   "ASSIST",
   "AUTOSPLIT",
   "ATTACKER",
   "WIMPY",
   "SHOW-ROOM",
   "SHOW-HP",
   "SHOW-MANA",
   "SHOW-MOVE",
   "SHOW-EXITS",
   "NOHASSLE",
   "MUZZLED",
   "NOSYS",
   "NOIMM",
   "NOCLAN",
   "AUTOLOOT",
   "AUTOAGGR",
   "AUTOJUNK",
   "AUTOEAT",
   "\n"
};


const char *position_types[] = {
   "Dead",
   "Mortally wounded",
   "Incapacitated",
   "Stunned",
   "Sleeping",
   "Resting",
   "Sitting",
   "Fighting",
   "Standing",
   "\n"
};

const char *connected_types[] =  {
   "Playing",
   "Get name",
   "Confirm name",
   "Read Password",
   "Get new password",
   "Confirm new password",
   "Get sex",
   "Read messages of today",
   "Read Menu",
   "Get extra description",
   "Get class",
   "\n"
};

/* [class], [level] (all) */
const int thaco[MAX_CLASS][MAX_LEV] = {
   { 100,30,30,30,29,29,29,28,28,28,27,27,27,26,26,26,25,25,25,24,24,                 /*Mage*/
         24,23,23,23,22,22,22,20,20,20,19,19,19,18,18,18,17,17,17,16,16,16,15,15,15},
   { 100,30,30,29,29,28,28,27,27,26,26,25,25,24,24,23,23,22,22,21,21,                 /*Cleric*/
         20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10,10, 9, 9, 8},
   { 100,30,30,30,28,28,28,26,26,26,24,24,24,22,22,22,20,20,20,18,18,                 /*Thief*/
         18,16,16,16,14,14,14,12,12,12,10,10,10, 8, 8, 8, 6, 6, 6, 4, 4, 4, 2, 2, 2},
   { 100,30,29,28,27,26,25,24,23,22,21,20,18,17,16,15,14,13,12,11,10,                 /*Warrior*/
          9, 8, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,-13,-14,-15},
   { 100,30,30,29,29,28,28,27,27,26,26,25,25,24,24,23,23,22,22,21,21,                 /*Bard*/
         20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10,10, 9, 9, 8},
   { 100,30,30,30,28,28,28,26,26,26,24,24,24,22,22,22,20,20,20,18,18,                 /*Kai*/
         18,16,16,16,14,14,14,12,12,12,10,10,10, 8, 8, 8, 6, 6, 6, 4, 4, 4, 2, 2, 2},
   { 100,30,30,30,28,28,28,26,26,26,24,24,24,22,22,22,20,20,20,18,18,                 /*Drakkhar*/
         18,16,16,16,14,14,14,12,12,12,10,10,10, 8, 8, 8, 6, 6, 6, 4, 4, 4, 2, 2, 2},
   { 100,30,29,28,27,26,25,24,23,22,21,20,18,17,16,15,14,13,12,11,10,                 /*Demon-Knight*/
          9, 8, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,-13,-14,-15},
   { 100,30,29,28,27,26,25,24,23,22,21,20,18,17,16,15,14,13,12,11,10,                 /*Elf-Knight*/
          9, 8, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,-13,-14,-15},
   { 100,30,29,28,27,26,25,24,23,22,21,20,18,17,16,15,14,13,12,11,10,                /*Dragon Warrior*/
          9, 8, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,-13,-14,-15}
};

/* [ch] strength apply (all) */
const struct str_app_type str_app[31] = {
   { -6,-6,   0,  0 },  /* 0  */
   { -5,-5,   2,  1 },  /* 1  */
   { -4,-5,   5,  2 },
   { -3,-3,  10,  3 },  /* 3  */
   { -2,-2,  20,  4 },
   { -1,-2,  35,  5 },  /* 5  */
   {  0,-1,  50,  6 },
   {  0,-1,  65,  7 },  /* 7  */
   {  0,-1,  80,  8 },
   {  0, 0,  90,  9 },
   {  0, 0, 100, 10 }, /* 10  */
   {  0, 0, 110, 11 },
   {  0, 0, 120, 12 },
   {  0, 1, 130, 13 }, /* 13  */
   {  0, 1, 160, 14 },
   {  0, 1, 210, 15 }, /* 15  */
   {  1, 2, 270, 16 },
   {  2, 2, 345, 18 },
   {  2, 3, 425, 20 }, /* 18  */
   {  3, 7, 525, 22 },
   {  3, 8, 675, 25 }, /* 20  */
   {  4, 9, 850, 30 },
   {  4,10,1000, 35 },
   {  5,11,1200, 40 }, /* 23  */
   {  5,12,1500, 45 },
   {  6,13,2000, 50 }, /* 25  */
   {  6,14,6000, 60 },
   {  7,15,7000, 70 },  
   {  7,16,8000, 80 }, /* 28  */
   {  8,17,9000, 90 }, 
   {  8,18,10000,100}  /* 30  */
};

/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[31] = {
   {-99,-99,-90,-99,-60},   /* 0 */
   {-90,-90,-60,-90,-50},   /* 1 */
   {-80,-80,-40,-80,-45},
   {-70,-70,-30,-70,-40},
   {-60,-60,-30,-60,-35},
   {-50,-50,-20,-50,-30},   /* 5 */
   {-40,-40,-20,-40,-25},
   {-30,-30,-15,-30,-20},
   {-20,-20,-15,-20,-15},
   {-15,-10,-10,-20,-10},
   {-10, -5,-10,-15, -5},   /* 10 */
   { -5,  0, -5,-10,  0},
   {  0,  0,  0, -5,  0},
   {  0,  0,  0,  0,  0},
   {  0,  0,  0,  0,  0},
   {  0,  0,  0,  0,  0},   /* 15 */
   {  0,  5,  0,  0,  0},
   {  5, 10,  0,  5,  5},
   { 10, 15,  5, 10, 10},
   { 15, 20, 10, 15, 15},
   { 15, 20, 10, 15, 15},   /* 20 */
   { 20, 25, 10, 15, 20},
   { 20, 25, 15, 20, 20},
   { 25, 25, 15, 20, 20},
   { 25, 30, 15, 25, 25},
   { 25, 30, 15, 25, 25},   /* 25 */
   { 30, 35, 20, 20, 30},
   { 35, 40, 25, 25, 35},
   { 40, 45, 30, 30, 40},
   { 45, 50, 35, 35, 45},
   { 99, 99, 99, 99, 99}    /* 30 */
};

/* [level] backstab multiplyer (thieves only) */
const byte backstab_mult[MAX_LEV] = {
   1,   /* 0 */
   2,   /* 1 */
   2,
   2,
   2,
   3,   /* 5 */
   3,
   3,
   3,
   3,
   4,   /* 10 */
   4,
   4,
   4,
   4,
   5,   /* 15 */
   5,
   5,
   5,
   5,
   6,   /* 20 */
   6,
   6,
   6,
   6,
   7,    /* 25 */
   7,
   7,
   7,
   7,
   8,    /* 30 */
   8,
   8,
   8,
   8,
   9,    /* 35 */
   9,
   9,
   9,
   9,
   10,    /* 40 */
   11,
   12,
   13,
   14,
   15     /* 45 */

};

/* [dex] apply (all) */
const struct dex_app_type dex_app[31] = {
   {-6,-6, 6},   /* 0 */
   {-5,-5, 5},   /* 1 */
   {-4,-4, 5},
   {-3,-3, 4},
   {-2,-2, 3},
   {-1,-1, 2},   /* 5 */
   { 0, 0, 1},
   { 0, 0, 0},
   { 0, 0, 0},
   { 0, 0, 0},
   { 0, 0, 0},   /* 10 */
   { 0, 0, 0},
   { 0, 0, 0},
   { 0, 0, 0},
   { 0, 0, 0},
   { 1, 0,-1},   /* 15 */
   { 1, 1,-2},
   { 2, 2,-3},
   { 2, 2,-4},
   { 3, 3,-4},
   { 3, 3,-4},   /* 20 */
   { 4, 4,-5},
   { 4, 4,-5},
   { 5, 4,-5},
   { 5, 5,-6},
   { 6, 5,-6},   /* 25 */
   { 6, 6,-7}, 
   { 7, 7,-8},
   { 7, 8,-9},
   { 8, 9,-10},
   { 9,10,-11}   /* 30 */
};

/* [con] apply (all) */
const struct con_app_type con_app[31] = {
   {-6,20},   /* 0 */
   {-5,25},   /* 1 */
   {-4,30},
   {-3,35},
   {-2,40},
   {-2,45},   /* 5 */
   {-1,50},
   {-1,55},
   {-1,60},
   { 0,65},
   { 0,70},   /* 10 */
   { 0,75},
   { 0,80},
   { 1,85},
   { 1,88},
   { 1,90},   /* 15 */
   { 1,95},
   { 2,97},
   { 2,99},
   { 3,99},
   { 3,99},   /* 20 */
   { 4,99},
   { 4,99},
   { 5,99},
   { 5,99},
   { 6,99},   /* 25 */
   { 6,99},
   { 7,99},
   { 7,99},
   { 8,99},
   { 9,100}   /* 30 */

};

/* [int] apply (all) */
const struct int_app_type int_app[31] = {
   3,
   5,    /* 1 */
   7,
   8,
   9,
   10,   /* 5 */
   11,
   12,
   13,
   15,
   17,   /* 10 */
   19,
   22,
   25,
   30,
   35,   /* 15 */
   40,
   45,
   50,
   53,
   55,   /* 20 */
   58,
   61,
   64,
   67,
   70,   /* 25 */
   73,
   76,
   79,
   85,
   95   /* 30 */
};

/* [wis] apply (all) */
const struct wis_app_type wis_app[31] = {
   0,   /* 0 */
   0,   /* 1 */
   0,
   0,
   0,
   0,   /* 5 */
   0,
   1,
   1,
   1,
   1,   /* 10 */
   2,
   2,
   2,
   3,
   3,   /* 15 */
   4,
   4,
   5,   /* 18 */
   5,
   6,   /* 20 */
   6,
   7,
   7,
   8,
   8,  /* 25 */
   9,
   10,
   11,
   12,
   13   /* 30 */
};

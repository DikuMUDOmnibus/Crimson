/* ************************************************************************
*  file: spells.h , Implementation of magic spells.       Part of DIKUMUD *
*  Usage : Spells                                                         *
************************************************************************* */

#define MAX_BUF_LENGTH              240

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO */
#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM_PERSON            7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_IMPROVED_INVIS          9
#define SPELL_COLOUR_SPRAY           10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_EVIL            18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVISIBLE       19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROTECT_FROM_EVIL      34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE          41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */

/* types of attacks and skills must NOT use same numbers as spells! */

#define SKILL_SNEAK                  45 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                   46 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                  47 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BACKSTAB               48 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK              49 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_KICK                   50 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                   51 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE                 52 /* MAXIMUM SKILL NUMBER  */

#define SPELL_IDENTIFY               53
#define SPELL_CAUSE_CRITIC           54 
#define SPELL_CAUSE_LIGHT            55 
#define SKILL_DAM                    56
#define SKILL_SLASH                  57
#define SKILL_PIERCE                 58
#define SKILL_BLUDGEON               59
#define SKILL_PARRY                  60
#define SKILL_DODGE                  61
#define SKILL_SCALES                 62
#define SPELL_FIRE_BREATH            63
#define SPELL_GAS_BREATH             64
#define SPELL_FROST_BREATH           65
#define SPELL_ACID_BREATH            66
#define SPELL_LIGHTNING_BREATH       67
#define SPELL_DARKSIGHT              68
#define SKILL_SECOND                 69
#define SKILL_THIRD                  70
#define SPELL_FOURTH                 71

#define SPELL_UNDETECT_INVIS         72
#define SPELL_RESTORATION            73
#define SPELL_REGENERATION           74
#define SPELL_MINOR_WARDS            75
#define SPELL_MAJOR_WARDS            76
#define SPELL_MORD_SWORD             77
#define SPELL_SPIRIT_HAMMER          78
#define SPELL_TURN_UNDEAD            79
#define SPELL_SUCCOR                 80
#define SPELL_DONATE_MANA            81
#define SPELL_MANA_LINK              82
#define SPELL_MAGIC_RESIST           83
#define SPELL_MAGIC_IMMUNE           84
#define SPELL_DISPEL_MAGIC           85 
#define SPELL_CONFLAGRATION          86
#define SPELL_BREATHWATER            87
#define SPELL_SUSTENANCE             88
                                          
/* oh oh... will conflict with spells */
/* weapon types and spell types use common damage messages */                                          
#define TYPE_HIT                     100
#define TYPE_BLUDGEON                101
#define TYPE_PIERCE                  102
#define TYPE_SLASH                   103
#define TYPE_WHIP                    104 /* EXAMPLE */
#define TYPE_CLAW                    105  /* NO MESSAGES WRITTEN YET! */
#define TYPE_BITE                    106  /* NO MESSAGES WRITTEN YET! */
#define TYPE_STING                   107  /* NO MESSAGES WRITTEN YET! */
#define TYPE_CRUSH                   108  /* NO MESSAGES WRITTEN YET! */



#define TYPE_SUFFERING               200
/* More anything but spells and weapontypes can be insterted here! */

#define MAX_TYPES 70

#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4

#define MAX_SPL_LIST 127


#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO    64 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024

struct spell_info_type
{
   void (*spell_pointer) (signed char level, struct char_data *ch, char *arg, int type,
                          struct char_data *tar_ch, struct obj_data *tar_obj);
   signed char minimum_position;  /* Position for caster                    */
   ubyte min_usesmana;     /* Amount of mana used by a spell    */
   signed char beats;             /* Heartbeats until ready for next */

   signed char min_level_cleric;  /* Level required for cleric       */
   signed char min_level_magic;   /* Level required for magic user   */
   signed char min_level_bard;
   signed char min_level_kai;
   signed char min_level_drakkhar;
   signed char min_level_dknight;
   signed char min_level_eknight;
   sh_int targets;         /* See below for use with TAR_XXX  */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
  char *singular;
  char *plural;
};


/* and the two big arrays of course */
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern const char *spells[];

/* spell related functions */
extern bool saves_spell(struct char_data *ch, sh_int save_type);
extern void assign_spell_pointers(void);

/* the various spells */
SPELL_FN(spell_magic_missile);
CAST_FN (cast_magic_missile);
SPELL_FN(spell_chill_touch);
CAST_FN (cast_chill_touch);
SPELL_FN(spell_burning_hands);
CAST_FN (cast_burning_hands);
SPELL_FN(spell_shocking_grasp);
CAST_FN (cast_shocking_grasp);
SPELL_FN(spell_lightning_bolt);
CAST_FN (cast_lightning_bolt);
SPELL_FN(spell_colour_spray);
CAST_FN (cast_colour_spray);
SPELL_FN(spell_energy_drain);
CAST_FN (cast_energy_drain);
SPELL_FN(spell_fireball);
CAST_FN (cast_fireball);
SPELL_FN(spell_turn_undead);
CAST_FN (cast_turn_undead);
SPELL_FN(spell_earthquake);
CAST_FN (cast_earthquake);
SPELL_FN(spell_enchant_weapon);
CAST_FN (cast_enchant_weapon);
SPELL_FN(spell_dispel_evil);
CAST_FN (cast_dispel_evil);
SPELL_FN(spell_call_lightning);
CAST_FN (cast_call_lightning);
SPELL_FN(spell_harm);
CAST_FN (cast_harm);
SPELL_FN(spell_heal);
CAST_FN (cast_heal);
SPELL_FN(spell_invisibility);
CAST_FN (cast_invisibility);
SPELL_FN(spell_conflagration);
CAST_FN (cast_conflagration);
SPELL_FN(spell_armor);
CAST_FN (cast_armor);
SPELL_FN(spell_teleport_zone);
CAST_FN (cast_teleport_zone);
SPELL_FN(spell_teleport);
CAST_FN (cast_teleport);
SPELL_FN(spell_bless);
CAST_FN (cast_bless);
SPELL_FN(spell_blindness);
CAST_FN (cast_blindness);
SPELL_FN(spell_control_weather);
CAST_FN (cast_control_weather);
SPELL_FN(spell_create_food);
CAST_FN (cast_create_food);
SPELL_FN(spell_sustenance);
CAST_FN (cast_sustenance);
SPELL_FN(spell_create_water);
CAST_FN (cast_create_water);
SPELL_FN(spell_cure_blind);
CAST_FN (cast_cure_blind);
SPELL_FN(spell_cure_critic);
CAST_FN (cast_cure_critic);
SPELL_FN(spell_donate_mana);
CAST_FN (cast_donate_mana);
SPELL_FN(spell_cause_critic);
CAST_FN (cast_cause_critic);
SPELL_FN(spell_cure_light);
CAST_FN (cast_cure_light);
SPELL_FN(spell_cause_light);
CAST_FN (cast_cause_light);
SPELL_FN(spell_curse);
CAST_FN (cast_curse);
SPELL_FN(spell_detect_evil);
CAST_FN (cast_detect_evil);
SPELL_FN(spell_detect_invisibility);
CAST_FN (cast_detect_invisibility);
SPELL_FN(spell_detect_magic);
CAST_FN (cast_detect_magic);
SPELL_FN(spell_detect_poison);
CAST_FN (cast_detect_poison);
SPELL_FN(spell_improved_invis);
CAST_FN (cast_improved_invis);
SPELL_FN(spell_locate_object);
CAST_FN (cast_locate_object);
SPELL_FN(spell_poison);
CAST_FN (cast_poison);
SPELL_FN(spell_protection_from_evil);
CAST_FN (cast_protection_from_evil);
SPELL_FN(spell_remove_curse);
CAST_FN (cast_remove_curse);
SPELL_FN(spell_remove_poison);
CAST_FN (cast_remove_poison);
SPELL_FN(spell_sanctuary);
CAST_FN (cast_sanctuary);
SPELL_FN(spell_sleep);
CAST_FN (cast_sleep);
SPELL_FN(spell_strength);
CAST_FN (cast_strength);
SPELL_FN(spell_ventriloquate);
CAST_FN (cast_ventriloquate);
SPELL_FN(spell_word_of_recall);
CAST_FN (cast_word_of_recall);
SPELL_FN(spell_summon);
CAST_FN (cast_summon);
SPELL_FN(spell_succor);
CAST_FN (cast_succor);
SPELL_FN(spell_charm_person);
CAST_FN (cast_charm_person);
SPELL_FN(spell_sense_life);
CAST_FN (cast_sense_life);
SPELL_FN(spell_identify);
CAST_FN (cast_identify);
SPELL_FN(spell_fire_breath);
CAST_FN (cast_fire_breath);
SPELL_FN(spell_frost_breath);
CAST_FN (cast_frost_breath);
SPELL_FN(spell_acid_breath);
CAST_FN (cast_acid_breath);
SPELL_FN(spell_gas_breath);
CAST_FN (cast_gas_breath);
SPELL_FN(spell_lightning_breath);
CAST_FN (cast_lightning_breath);
SPELL_FN(spell_darksight);
CAST_FN (cast_darksight);
SPELL_FN(spell_restoration);
CAST_FN (cast_restoration);
SPELL_FN(spell_regeneration);
CAST_FN (cast_regeneration);
SPELL_FN(spell_magic_resist);
CAST_FN (cast_magic_resist);
SPELL_FN(spell_magic_immune);
CAST_FN (cast_magic_immune);
SPELL_FN(spell_breathwater);
CAST_FN (cast_breathwater);









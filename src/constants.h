#define MAX_CLASS 10
#define MAX_LEV 47
#define MAX_ARMOUR 250

#define IMP_LEV 45
#define IMO_LEV 41
#define IMO_LEV2 42
#define IMO_LEV3 43
#define IMO_LEV4 44
#define NPC_LEV 75

struct title_type
{
	char *title_m;
	char *title_f;
	int exp;
};


extern const char *ban_types[];
extern const char *spell_wear_off_msg[];
extern const int rev_dir[];
extern const int movement_loss[];
extern const char *dirs[];
extern const char *weekdays[7];
extern const char *month_name[17];
extern const int sharp[];
extern const char *where[];
extern const char *loc[];
extern const char *drinks[];
extern const char *drinknames[];
extern const int drink_aff[][3];
extern const char *color_liquid[];
extern const char *fullness[];
extern const struct title_type titles[MAX_CLASS][MAX_LEV];
extern const char *item_types[];
extern const char *wear_bits[];
extern const char *extra_bits[];
extern const char *weapon_bits[];
extern const char *room_bits[];
extern const char *zflag_bits[];
extern const char *dirty_bits[];
extern const char *exit_bits[];
extern const char *sector_types[];
extern const char *equipment_types[];
extern const char *affected_bits[];
extern const char *apply_types[];
extern const char *pc_class_types[];
extern const char *npc_class_types[];
extern const char *action_bits[];
extern const char *player_bits[];
extern const char *position_types[];
extern const char *connected_types[];
extern const int thaco[MAX_CLASS][MAX_LEV];
extern const struct str_app_type str_app[31];
extern const struct dex_skill_type dex_app_skill[31];
extern const byte backstab_mult[MAX_LEV];
extern const struct dex_app_type dex_app[31];
extern const struct con_app_type con_app[31];
extern const struct int_app_type int_app[31];
extern const struct wis_app_type wis_app[31];


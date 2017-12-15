/* ************************************************************************
*  file: spec_assign.c , Special module.                  Part of DIKUMUD *
*  Usage: Procedures assigning function pointers.                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include "structs.h"
#include "db.h"

void boot_the_shops(void);
void assign_the_shopkeepers(void);

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
	int cityguard(struct char_data *ch, int cmd, char *arg);
	int bountyhunter(struct char_data *ch, int cmd, char *arg);
	int slime(struct char_data *ch, int cmd, char *arg);
	int gremlin(struct char_data *ch, int cmd, char *arg);
	int strahd(struct char_data *ch, int cmd, char *arg);
	int nosferatu(struct char_data *ch, int cmd, char *arg);
	int dungeon_master(struct char_data *ch, int cmd, char *arg);
	int receptionist(struct char_data *ch, int cmd, char *arg);
	int guild(struct char_data *ch, int cmd, char *arg);
	int guild_guard(struct char_data *ch, int cmd, char *arg);
	int puff(struct char_data *ch, int cmd, char *arg);
	int fido(struct char_data *ch, int cmd, char *arg);
	int janitor(struct char_data *ch, int cmd, char *arg);
	int mayor(struct char_data *ch, int cmd, char *arg);
	int biff(struct char_data *ch, int cmd, char *arg);
	int snake(struct char_data *ch, int cmd, char *arg);
	int thief(struct char_data *ch, int cmd, char *arg);
	int magic_user(struct char_data *ch, int cmd, char *arg);
	int high_priest(struct char_data *ch, int cmd, char *arg);
	int academy_trainer(struct char_data *ch, int cmd, char *arg);
	int final_exam(struct char_data *ch, int cmd, char *arg);

/* MIDGAARD */
	mob_index[real_mobile(1)].func = puff;
	mob_index[real_mobile(3019)].func = high_priest;
	mob_index[real_mobile(3061)].func = janitor;
	mob_index[real_mobile(3062)].func = fido;
	mob_index[real_mobile(3066)].func = fido;
	mob_index[real_mobile(3005)].func = receptionist;
	mob_index[real_mobile(3143)].func = mayor;
	mob_index[real_mobile(3060)].func = cityguard;
	mob_index[real_mobile(3067)].func = cityguard;
	mob_index[real_mobile(3145)].func = bountyhunter;
	mob_index[real_mobile(3146)].func = bountyhunter;
	mob_index[real_mobile(3120)].func = biff;

/* these mobs remove objs left layign around from play */
	mob_index[real_mobile(3147)].func = slime;
	mob_index[real_mobile(8310)].func = slime;
	mob_index[real_mobile(8303)].func = gremlin;

/* Dungeonmaster and Tiamat */
	mob_index[real_mobile(8300)].func = dungeon_master;
	mob_index[real_mobile(8311)].func = dungeon_master;

/* Gryphon Hill mobs */
	mob_index[real_mobile(15450)].func = strahd;

/* SilverDale mobs */
	mob_index[real_mobile(4510)].func = nosferatu;

/* shaolintemple */
	mob_index[real_mobile(14120)].func = bountyhunter;
	mob_index[real_mobile(14122)].func = cityguard;
	mob_index[real_mobile(14121)].func = cityguard;
	mob_index[real_mobile(14128)].func = cityguard;

/* dragon realms */
	mob_index[real_mobile(13802)].func = bountyhunter;

/* snotlings */
	mob_index[real_mobile(14302)].func = bountyhunter;
	mob_index[real_mobile(14303)].func = bountyhunter;

/* egypt */
	mob_index[real_mobile(14226)].func = cityguard;


/* GUILDS and such */
	mob_index[real_mobile(3020)].func = guild;
	mob_index[real_mobile(3021)].func = guild;
	mob_index[real_mobile(3022)].func = guild;
	mob_index[real_mobile(3023)].func = guild;
	mob_index[real_mobile(3024)].func = guild_guard;
	mob_index[real_mobile(3025)].func = guild_guard;
	mob_index[real_mobile(3026)].func = guild_guard;
	mob_index[real_mobile(3027)].func = guild_guard;

/* MORIA */
	mob_index[real_mobile(4000)].func = snake;
	mob_index[real_mobile(4001)].func = snake;
	mob_index[real_mobile(4053)].func = snake;
	mob_index[real_mobile(4103)].func = thief;
	mob_index[real_mobile(4100)].func = magic_user;
	mob_index[real_mobile(4102)].func = snake;

/* SEWERS */
	mob_index[real_mobile(7006)].func = snake;

/* FOREST */
	mob_index[real_mobile(6113)].func = snake;
	mob_index[real_mobile(6114)].func = snake;
	mob_index[real_mobile(6910)].func = magic_user;

/* Gnomes */
	mob_index[real_mobile(8357)].func = receptionist;
	mob_index[real_mobile(8360)].func = bountyhunter;

/* Arnath */
	mob_index[real_mobile(14431)].func = receptionist;
	mob_index[real_mobile(14440)].func = bountyhunter;

/* Tuscany */
	mob_index[real_mobile(15600)].func = cityguard;
	mob_index[real_mobile(15690)].func = cityguard;
	mob_index[real_mobile(15696)].func = receptionist;
	mob_index[real_mobile(15697)].func = bountyhunter;

/* Royal Academy */
	mob_index[real_mobile(2902)].func = guild;
	mob_index[real_mobile(2903)].func = academy_trainer; 
	mob_index[real_mobile(2908)].func = final_exam;

/* Ancients */
	mob_index[real_mobile(16008)].func = cityguard;
	mob_index[real_mobile(16009)].func = cityguard;
	mob_index[real_mobile(16016)].func = cityguard;

/* Cowland1 */
	mob_index[real_mobile(8131)].func = receptionist;

/* Read the shp file */
	boot_the_shops();
	assign_the_shopkeepers();
}



/* assign special procedures to objects */
void assign_objects(void)
{
	int board(struct char_data *ch, int cmd, char *arg);
        int forge(struct char_data *ch, int cmd, char *arg);
        int academy_key(struct char_data *ch, int cmd, char *arg);

	obj_index[real_object(3096)].func = board;
	obj_index[real_object(3097)].func = board;
	obj_index[real_object(3098)].func = board;
	obj_index[real_object(3099)].func = board;
	obj_index[real_object(4510)].func = forge;
	obj_index[real_object(2914)].func = academy_key;
}



/* assign special procedures to rooms */
void assign_rooms(void)
{
	int dump(struct char_data *ch, int cmd, char *arg);
	int chalice(struct char_data *ch, int cmd, char *arg);
	int kings_hall(struct char_data *ch, int cmd, char *arg);
	int pet_shops(struct char_data *ch, int cmd, char *arg);
	int pray_for_items(struct char_data *ch, int cmd, char *arg);
	int citadel_altar(struct char_data *ch, int cmd, char *arg);
        int cliff(struct char_data *ch, int cmd, char *arg);
	int chamber_of_gods(struct char_data *ch, int cmd, char *arg);
	int academy_entrance(struct char_data *ch, int cmd, char *arg);
	int hall_of_fame(struct char_data *ch, int cmd, char *arg);

/* Midgaard */
	world[real_room(3030)].funct = dump;
	world[real_room(3031)].funct = pet_shops;
	world[real_room(3054)].funct = pray_for_items;

/* Elfin Citadel */
	world[real_room(14670)].funct = citadel_altar;

/* SilverDale */
	world[real_room(4513)].funct = cliff;

/* Tuscany */
	world[real_room(15746)].funct = chamber_of_gods;

/* Royal Academy */
	world[real_room(2900)].funct = academy_entrance;
	world[real_room(2916)].funct = hall_of_fame; 

/* Unused */
/*
	world[real_room(704)].funct = chalice;
	world[real_room(2518)].funct = kings_hall;
*/
}

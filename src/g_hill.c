/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utility.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "constants.h"


/* place
int citadel_altar(struct char_data *ch, int cmd, char *arg)
{
}
*/

int strahd(struct char_data *ch, int cmd, char *arg) {
	if (cmd || !AWAKE(ch))
		return FALSE;
/*
	if (!ch->specials.fighting && !number(0,50))
		cast_teleport_zone(GET_LEVEL(ch),ch,"", SPELL_TYPE_SPELL,0,0);
 teleport zone is wonky disabled till its fixed */
	
	return FALSE;
}


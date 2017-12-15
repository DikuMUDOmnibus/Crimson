
	if (!IS_NPC(ch) && (GET_LEVEL(ch)<IMO_LEV && IS_SET(ch->specials.act,PLR_MUZZLE)))
	{
		send_to_char("You are unable to make a sound!!!\n\r", ch);
		return;
	}


/* ************************************************************************
*  file: act.h , command proc declarations         .      Part of DIKUMUD *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

extern void do_move(struct char_data *ch, char *arg, int cmd);
extern void do_look(struct char_data *ch, char *arg, int cmd);
extern void do_read(struct char_data *ch, char *arg, int cmd);
extern void do_say(struct char_data *ch, char *arg, int cmd);
extern void do_exit(struct char_data *ch, char *arg, int cmd);
extern void do_speedwalk(struct char_data *ch, char *arg, int cmd);
extern void do_snoop(struct char_data *ch, char *arg, int cmd);
extern void do_insult(struct char_data *ch, char *arg, int cmd);
extern void do_quit(struct char_data *ch, char *arg, int cmd);
extern void do_qui(struct char_data *ch, char *arg, int cmd);
extern void do_help(struct char_data *ch, char *arg, int cmd);
extern void do_who(struct char_data *ch, char *arg, int cmd);
extern void do_emote(struct char_data *ch, char *arg, int cmd);
extern void do_echo(struct char_data *ch, char *arg, int cmd);
extern void do_trans(struct char_data *ch, char *arg, int cmd);
extern void do_kill(struct char_data *ch, char *arg, int cmd);
extern void do_stand(struct char_data *ch, char *arg, int cmd);
extern void do_sit(struct char_data *ch, char *arg, int cmd);
extern void do_rest(struct char_data *ch, char *arg, int cmd);
extern void do_sleep(struct char_data *ch, char *arg, int cmd);
extern void do_wake(struct char_data *ch, char *arg, int cmd);
extern void do_force(struct char_data *ch, char *arg, int cmd);
extern void do_get(struct char_data *ch, char *arg, int cmd);
extern void do_drop(struct char_data *ch, char *arg, int cmd);
extern void do_news(struct char_data *ch, char *arg, int cmd);
extern void do_score(struct char_data *ch, char *arg, int cmd);
extern void do_inventory(struct char_data *ch, char *arg, int cmd);
extern void do_equipment(struct char_data *ch, char *arg, int cmd);
extern void do_shout(struct char_data *ch, char *arg, int cmd);
extern void do_not_here(struct char_data *ch, char *arg, int cmd);
extern void do_tell(struct char_data *ch, char *arg, int cmd);
extern void do_wear(struct char_data *ch, char *arg, int cmd);
extern void do_wield(struct char_data *ch, char *arg, int cmd);
extern void do_grab(struct char_data *ch, char *arg, int cmd);
extern void do_remove(struct char_data *ch, char *arg, int cmd);
extern void do_put(struct char_data *ch, char *arg, int cmd);
extern void do_terminate(struct char_data *ch, char *arg, int cmd);
extern void do_save(struct char_data *ch, char *arg, int cmd);
extern void do_hit(struct char_data *ch, char *arg, int cmd);
extern void do_string(struct char_data *ch, char *arg, int cmd);
extern void do_give(struct char_data *ch, char *arg, int cmd);
extern void do_stat(struct char_data *ch, char *arg, int cmd);
extern void do_setskill(struct char_data *ch, char *arg, int cmd);
void do_time(struct char_data *ch, char *arg, int cmd);
void do_weather(struct char_data *ch, char *arg, int cmd);
void do_load(struct char_data *ch, char *arg, int cmd);
void do_purge(struct char_data *ch, char *arg, int cmd);
void do_shutdow(struct char_data *ch, char *arg, int cmd);
void do_idea(struct char_data *ch, char *arg, int cmd);
void do_typo(struct char_data *ch, char *arg, int cmd);
void do_bug(struct char_data *ch, char *arg, int cmd);
void do_whisper(struct char_data *ch, char *arg, int cmd);
void do_cast(struct char_data *ch, char *arg, int cmd);
void do_at(struct char_data *ch, char *arg, int cmd);
void do_goto(struct char_data *ch, char *arg, int cmd);
void do_ask(struct char_data *ch, char *arg, int cmd);
void do_drink(struct char_data *ch, char *arg, int cmd);
void do_eat(struct char_data *ch, char *arg, int cmd);
void do_pour(struct char_data *ch, char *arg, int cmd);
void do_sip(struct char_data *ch, char *arg, int cmd);
void do_taste(struct char_data *ch, char *arg, int cmd);
void do_order(struct char_data *ch, char *arg, int cmd);
void do_follow(struct char_data *ch, char *arg, int cmd);
void do_rent(struct char_data *ch, char *arg, int cmd);
void do_offer(struct char_data *ch, char *arg, int cmd);
void do_advance(struct char_data *ch, char *arg, int cmd);
void do_close(struct char_data *ch, char *arg, int cmd);
void do_open(struct char_data *ch, char *arg, int cmd);
void do_lock(struct char_data *ch, char *arg, int cmd);
void do_unlock(struct char_data *ch, char *arg, int cmd);
void do_exits(struct char_data *ch, char *arg, int cmd);
void do_enter(struct char_data *ch, char *arg, int cmd);
void do_leave(struct char_data *ch, char *arg, int cmd);
void do_write(struct char_data *ch, char *arg, int cmd);
void do_flee(struct char_data *ch, char *arg, int cmd);
void do_sneak(struct char_data *ch, char *arg, int cmd);
void do_hide(struct char_data *ch, char *arg, int cmd);
void do_backstab(struct char_data *ch, char *arg, int cmd);
void do_pick(struct char_data *ch, char *arg, int cmd);
void do_steal(struct char_data *ch, char *arg, int cmd);
void do_bash(struct char_data *ch, char *arg, int cmd);
void do_rescue(struct char_data *ch, char *arg, int cmd);
void do_kick(struct char_data *ch, char *arg, int cmd);
void do_examine(struct char_data *ch, char *arg, int cmd);
void do_info(struct char_data *ch, char *arg, int cmd);
void do_users(struct char_data *ch, char *arg, int cmd);
void do_where(struct char_data *ch, char *arg, int cmd);
void do_levels(struct char_data *ch, char *arg, int cmd);
void do_reroll(struct char_data *ch, char *arg, int cmd);
void do_brief(struct char_data *ch, char *arg, int cmd);
void do_wizlist(struct char_data *ch, char *arg, int cmd);
void do_consider(struct char_data *ch, char *arg, int cmd);
void do_group(struct char_data *ch, char *arg, int cmd);
void do_restore(struct char_data *ch, char *arg, int cmd);
void do_return(struct char_data *ch, char *arg, int cmd);
void do_switch(struct char_data *ch, char *arg, int cmd);
void do_quaff(struct char_data *ch, char *arg, int cmd);
void do_recite(struct char_data *ch, char *arg, int cmd);
void do_use(struct char_data *ch, char *arg, int cmd);
void do_pose(struct char_data *ch, char *arg, int cmd);
void do_noshout(struct char_data *ch, char *arg, int cmd);
void do_wizhelp(struct char_data *ch, char *arg, int cmd);
void do_credits(struct char_data *ch, char *arg, int cmd);
void do_compact(struct char_data *ch, char *arg, int cmd);
void do_title(struct char_data *ch, char *arg, int cmd);
void do_action(struct char_data *ch, char *arg, int cmd);
void do_practice(struct char_data *ch, char *arg, int cmd);
void do_junk(struct char_data *ch, char *arg, int cmd);
void do_spells(struct char_data *ch, char *arg, int cmd);
void do_breath(struct char_data *ch, char *arg, int cmd);
void do_visible(struct char_data *ch, char *arg, int cmd);
void do_withdraw(struct char_data *ch, char *arg, int cmd);
void do_balance(struct char_data *ch, char *arg, int cmd);
void do_deposit(struct char_data *ch, char *arg, int cmd);
void do_rhelp(struct char_data *ch, char *arg, int cmd);
void do_rcopy(struct char_data *ch, char *arg, int cmd);
void do_rlink(struct char_data *ch, char *arg, int cmd);
void do_rsect(struct char_data *ch, char *arg, int cmd);
void do_rflag(struct char_data *ch, char *arg, int cmd);
void do_rdesc(struct char_data *ch, char *arg, int cmd);
void do_rsave(struct char_data *ch, char *arg, int cmd);
void do_rname(struct char_data *ch, char *arg, int cmd);
void do_rlflag(struct char_data *ch, char *arg, int cmd);
void do_rlist(struct char_data *ch, char *arg, int cmd);
void do_gossip(struct char_data *ch, char *arg, int cmd);
void do_immtalk(struct char_data *ch, char *arg, int cmd);
void do_auction(struct char_data *ch, char *arg, int cmd);
void do_poofin(struct char_data *ch, char *arg, int cmd);
void do_poofout(struct char_data *ch, char *arg, int cmd);
void do_ansi(struct char_data *ch, char *arg, int cmd);
void do_noimm(struct char_data *ch, char *arg, int cmd);
void do_olist(struct char_data *ch, char *arg, int cmd);
void do_mlist(struct char_data *ch, char *arg, int cmd);
void do_oflag(struct char_data *ch, char *arg, int cmd);
void do_nogossip(struct char_data *ch, char *arg, int cmd);
void do_noauction(struct char_data *ch, char *arg, int cmd);
void do_nosummon(struct char_data *ch, char *arg, int cmd);
void do_holler(struct char_data *ch, char *arg, int cmd);
void do_ostat(struct char_data *ch, char *arg, int cmd);
void do_commands(struct char_data *ch, char *arg, int cmd);
void do_mflag(struct char_data *ch, char *arg, int cmd);
void do_osave(struct char_data *ch, char *arg, int cmd);
void do_msave(struct char_data *ch, char *arg, int cmd);
void do_mstat(struct char_data *ch, char *arg, int cmd);
void do_maffect(struct char_data *ch, char *arg, int cmd);
void do_rgoto(struct char_data *ch, char *arg, int cmd);
void do_rstat(struct char_data *ch, char *arg, int cmd);
void do_grats(struct char_data *ch, char *arg, int cmd);
void do_nograts(struct char_data *ch, char *arg, int cmd);
void do_notell(struct char_data *ch, char *arg, int cmd);
void do_assist(struct char_data *ch, char *arg, int cmd);
void do_wimpy(struct char_data *ch, char *arg, int cmd);
void do_vnum(struct char_data *ch, char *arg, int cmd);
void do_display(struct char_data *ch, char *arg, int cmd);
void do_owflag(struct char_data *ch, char *arg, int cmd);
void do_ovalues(struct char_data *ch, char *arg, int cmd);
void do_zlist(struct char_data *ch, char *arg, int cmd);
void do_ocost(struct char_data *ch, char *arg, int cmd);
void do_orent(struct char_data *ch, char *arg, int cmd);
void do_oweight(struct char_data *ch, char *arg, int cmd);
void do_okeywords(struct char_data *ch, char *arg, int cmd);
void do_otype(struct char_data *ch, char *arg, int cmd);
void do_osdesc(struct char_data *ch, char *arg, int cmd);
void do_oldesc(struct char_data *ch, char *arg, int cmd);
void do_owear(struct char_data *ch, char *arg, int cmd);
void do_oaffect(struct char_data *ch, char *arg, int cmd);
void do_ocreate(struct char_data *ch, char *arg, int cmd);
void do_msdesc(struct char_data *ch, char *arg, int cmd);
void do_mldesc(struct char_data *ch, char *arg, int cmd);
void do_mdesc(struct char_data *ch, char *arg, int cmd);
void do_mac(struct char_data *ch, char *arg, int cmd);
void do_malign(struct char_data *ch, char *arg, int cmd);
void do_mthaco(struct char_data *ch, char *arg, int cmd);
void do_mhitpoints(struct char_data *ch, char *arg, int cmd);
void do_mdamage(struct char_data *ch, char *arg, int cmd);
void do_mlevel(struct char_data *ch, char *arg, int cmd);
void do_mexp(struct char_data *ch, char *arg, int cmd);
void do_mgold(struct char_data *ch, char *arg, int cmd);
void do_mcreate(struct char_data *ch, char *arg, int cmd);
void do_zreset(struct char_data *ch, char *arg, int cmd);
void do_mkeywords(struct char_data *ch, char *arg, int cmd);
void do_nohassle(struct char_data *ch, char *arg, int cmd);
void do_wizinvis(struct char_data *ch, char *arg, int cmd);
void do_crashsave(struct char_data *ch, char *arg, int cmd);
void do_muzzle(struct char_data *ch, char *arg, int cmd);
void do_zedit(struct char_data *ch, char *arg, int cmd);
void do_ban(struct char_data *ch, char *arg, int cmd);
void do_unban(struct char_data *ch, char *arg, int cmd);
void do_nosys(struct char_data *ch, char *arg, int cmd);
void do_noclan(struct char_data *ch, char *arg, int cmd);
void do_split(struct char_data *ch, char *arg, int cmd);
void do_auto(struct char_data *ch, char *arg, int cmd);
void do_zcreate(struct char_data *ch, char *arg, int cmd);
void do_zsave(struct char_data *ch, char *arg, int cmd);
void do_msex(struct char_data *ch, char *arg, int cmd);
void do_zfirst(struct char_data *ch, char *arg, int cmd);
void do_zreboot(struct char_data *ch, char *arg, int cmd);
void do_zmax(struct char_data *ch, char *arg, int cmd);
void do_zstat(struct char_data *ch, char *arg, int cmd);
void do_zflag(struct char_data *ch, char *arg, int cmd);
void do_redesc(struct char_data *ch, char *arg, int cmd);
void do_oedesc(struct char_data *ch, char *arg, int cmd);
void do_donate(struct char_data *ch, char *arg, int cmd);

/* end of act.h - all procs from various act modules */

/* utility type procedures from act_info.c */
extern struct obj_data *get_object_in_equip_vis(struct char_data *ch,char *arg, struct obj_data *equipment[], int *j);
 
/* from act_obj2.c */
extern void weight_change_object(struct obj_data *obj, int weight);

/* from act_move.c */
extern int do_simple_move(struct char_data *ch, int cmd, int following);

/* from act_wizard.c */
extern void roll_abilities(struct char_data *ch);
extern void do_start(struct char_data *ch);

/* from act_social.c */
extern void boot_social_messages(void);

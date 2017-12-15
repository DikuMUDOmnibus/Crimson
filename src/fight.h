extern void load_messages(void);
extern void update_pos( struct char_data *victim );
extern void set_fighting(struct char_data *ch, struct char_data *vict);
extern void raw_kill(struct char_data *ch);
extern void death_cry(struct char_data *ch);
extern void stop_fighting(struct char_data *ch);
extern void hit(struct char_data *ch, struct char_data *victim, int type);
extern void damage(struct char_data *ch, struct char_data *victim, int dam, int attacktype, int hits);
 
#define DAMAGE(c,v,d,a) damage((c),(v),(d),(a),1)

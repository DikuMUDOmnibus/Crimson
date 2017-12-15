/* ************************************************************************
*  file: limits.h , Limit/Gain control module             Part of DIKUMUD *
*  Usage: declaration of title type                                       *
************************************************************************* */

#define READ_TITLE(ch) (GET_SEX(ch) == SEX_MALE ?   \
   titles[GET_CLASS(ch)-1][GET_LEVEL(ch)].title_m :  \
   titles[GET_CLASS(ch)-1][GET_LEVEL(ch)].title_f)


/* Public Procedures */
extern int mana_limit(struct char_data *ch);
extern int hit_limit(struct char_data *ch);
extern int move_limit(struct char_data *ch);
extern int mana_gain(struct char_data *ch);
extern int move_gain(struct char_data *ch);
extern int hit_gain(struct char_data *ch);
extern void set_title(struct char_data *ch);

extern void gain_condition(struct char_data *ch,int condition,int value);
extern void set_title(struct char_data *ch);
extern void advance_level(struct char_data *ch);
extern void gain_exp(struct char_data *ch, int gain);
extern void gain_exp_regardless(struct char_data *ch, int gain);
extern void check_idling(struct char_data *ch);
extern void point_update( void );


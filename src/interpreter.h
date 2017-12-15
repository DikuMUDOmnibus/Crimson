/* ************************************************************************
*  file: Interpreter.h , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
************************************************************************* */

extern void assign_command_pointers ( void );
void command_interpreter(struct char_data *ch, char *argument);
int search_block(char *arg, const char *list[], bool exact);
int old_search_block(char *argument,int begin,int length,const char *list[],int mode);
char lower( char c );
void argument_interpreter(char *argument, char *first_arg, char *second_arg);
char *one_argument(char *argument,char *first_arg);
int fill_word(char *argument);
void half_chop(char *string, char *arg1, char *arg2);
void nanny(struct descriptor_data *d, char *arg);
int is_abbrev(char *arg1, char *arg2);
int is_number(char *str);
int special(struct char_data *ch, int cmd, char *arg);


struct command_info
{
	void (*command_pointer) (struct char_data *ch, char *argument, int cmd);
	ubyte minimum_position;
	ubyte minimum_level;
};

#define MAX_CMD_LIST   311

#define CMD_NORTH      1
#define CMD_EAST       2
#define CMD_SOUTH      3
#define CMD_WEST       4
#define CMD_UP         5
#define CMD_DOWN       6
#define CMD_GET       10
#define CMD_SMILE     23
#define CMD_DANCE     24
#define CMD_SIGH      36 
#define CMD_DROP      60
#define CMD_PUT       67
#define CMD_BLUSH    105
#define CMD_BURP     106
#define CMD_COUGH    109
#define CMD_FART     111
#define CMD_WHISTLE  144
#define CMD_YAWN     147
#define CMD_TAKE     167
#define CMD_PRAY     176
#define CMD_SPEEDWALK 209
#define CMD_ZRESET   287
#define CMD_ZEDIT    293
#define CMD_AUTO     299
#define CMD_ZCREATE  300
#define CMD_ZSAVE    301
#define CMD_MSEX     302
#define CMD_ZFIRST   303
#define CMD_ZREBOOT  304
#define CMD_ZMAX     305
#define CMD_ZSTAT    306
#define CMD_ZFLAG    307
#define CMD_REDESC   308
#define CMD_OEDESC   309
#define CMD_DONATE   310

#include <stdio.h>
#include <time.h>

#include "../src/structs.h"

#define GET_LEVEL_TEXT(l)   \
  (l == 0 ? "NEW" :         \
	(l < 5  ? "LOW" :         \
	(l < 10  ? "MED" :         \
	(l < 20 ? "EXP" :         \
	(l < 30 ? "ADV" :         \
	(l < 40 ? "HIG" :         \
	(l ==40 ? "MAX" :         \
	(l < 45 ? "IMM" : "WIZ"))))))))

#define GET_CLASS_TEXT(c)                \
	(c == CLASS_MAGIC_USER ? "<MAGIC> " :  \
	(c == CLASS_CLERIC ?     "<CLERIC>" :  \
	(c == CLASS_THIEF ?      "<THIEF>"  :  \
        (c == CLASS_WARRIOR ?    "<FIGHT>"  :  \
        (c == CLASS_BARD  ?      "<BARD >"  :  \
	(c == CLASS_KAI ?        "<KAI>"    :  \
	(c == CLASS_DRAKKHAR ?   "<DRKHR>"  :  \
	(c == CLASS_DKNIGHT ?    "<DKNIG>"  :  \
        (c == CLASS_EKNIGHT ?    "<EKNIG>"  :  \
	(c == CLASS_DRAGONW ?    "<DRAGW>"  : "<UNDEF>"))))))))))


void DelPlayer(char *filename, int playerNumber)
{
	FILE *fl;
	struct char_file_u player;
	long end;

	if (!(fl = fopen(filename, "r+")))
	{
		perror("list");
		exit();
	}


	/* read the last player */
	fseek(fl, -sizeof(player)-1, SEEK_END);
	fread(&player, sizeof(player), 1, fl);
	fseek(fl, playerNumber*sizeof(player), SEEK_SET);
	fwrite(&player, sizeof(player), 1, fl);
	fseek(fl, 0, SEEK_END);
	end = ftell(fl);
	fclose(fl);

	if (truncate(filename, end-sizeof(player)))
		perror("truncate");
}

	
void DelScan(char *filename)
{
	FILE *fl;
	struct char_file_u buf;
	char *point;
	int num, days;
	long	filePos;

	if (!(fl = fopen(filename, "r")))
	{
		perror(filename);
		exit(1);
	}

	printf(" Num  Name                  Level    Class    Last On      Time On\n");

	for (num=0;;num++)
	{
		fread(&buf, sizeof(buf), 1, fl);
		if (feof(fl))
			break;

		days = (time(0)-buf.last_logon)/SECS_PER_REAL_DAY;
	
		if (days >= 150 || !*buf.name) {
			printf("%-20s ", buf.name);
			printf("<%s %2d> ", GET_LEVEL_TEXT(buf.level), buf.level);
	 
			printf("%-8s ", GET_CLASS_TEXT(buf.class));
	
			if (days >= 7)
				printf("[DAY %5d]  ", days);
				/* import delete player routine and place
				 a call here if days is > 30
				 */
			else
				printf("[--- -----]  ");
			printf("[%4dH %2dM]", 
				buf.played/SECS_PER_REAL_HOUR,
				buf.played%SECS_PER_REAL_HOUR/SECS_PER_REAL_MIN);
			printf("\n");
			
			fclose(fl);
			DelPlayer(filename, num);
			if (!(fl = fopen(filename, "r"))) {
				perror(filename);
				exit(1);
			}
			fseek(fl, num*sizeof(buf), SEEK_SET);
			num--;
		}
	}

	fclose(fl);
}

main(int argc, char **argv)
{
	if (argc != 2)
		fprintf(stderr, "Usage: %s <DikuMUD playerfile-name>\n", argv[0]);
	else
		DelScan(argv[1]);
}



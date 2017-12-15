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


void list(char *filename);

main(int argc, char **argv)
{
	if (argc != 2)
		fprintf(stderr, "Usage: %s <DikuMUD playerfile-name>\n", argv[0]);
	else
		list(argv[1]);
}


void list(char *filename)
{
	FILE *fl;
	struct char_file_u buf;
	char *point;
	int num, days;
	long played;

	if (!(fl = fopen(filename, "r")))
	{
		perror(filename);
		exit(1);
	}


	played = 0;
	printf(" Num  Name                  Level    Class    Last On      Time On\n");

	for (num=1;;num++)
	{
		fread(&buf, sizeof(buf), 1, fl);
		if (feof(fl))
			break;

		printf("[%3d] %-20s ", num, buf.name);
		printf("<%s %2d> ", GET_LEVEL_TEXT(buf.level), buf.level);
 
		printf("%-8s ", GET_CLASS_TEXT(buf.class));

		days = (time(0)-buf.last_logon)/SECS_PER_REAL_DAY;

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

		played += buf.played;
	}

	fclose(fl);

	printf("\nGrand total time played is %d hours, %d minutes\n", 
		played/SECS_PER_REAL_HOUR,
		played%SECS_PER_REAL_HOUR/SECS_PER_REAL_MIN);
}

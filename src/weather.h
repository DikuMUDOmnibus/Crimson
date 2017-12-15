/* ************************************************************************
*  file: weather.h , Weather and time module              Part of DIKUMUD *
*  Usage: Performing the clock and the weather                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

extern struct time_info_data time_info;
extern struct weather_data weather_info;

/* In this part. */

void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);


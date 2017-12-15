/* ************************************************************************
*  file: signals.c , trapping of signals from Unix.       Part of DIKUMUD *
*  Usage : Signal Trapping.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "structs.h"
#include "utility.h"
#include "signals.h"
#include "act.h"

void signal_setup(void)
{
	struct itimerval itime;
	struct timeval interval;

	signal(SIGUSR2, terminate_request);

	/* just to be on the safe side: */

	signal(SIGHUP, hupsig);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, hupsig);
	signal(SIGALRM, logsig);
	signal(SIGTERM, hupsig);

	/* set up the deadlock-protection */
#ifdef ARGH
	interval.tv_sec = 900;    /* 15 minutes */
	interval.tv_usec = 0;
	itime.it_interval = interval;
	itime.it_value = interval;
	setitimer(ITIMER_VIRTUAL, &itime, 0);
	signal(SIGVTALRM, checkpointing);
#endif
}



#ifdef ARGH
void checkpointing(int i)
{
	extern int tics;
	
	if (!tics)
	{
		log("CHECKPOINT terminate: tics not updated");
		abort(); /* would return FALSE here I guess */
	}
	else
		tics = 0;
}
#endif



void terminate_request(int i)
{
	extern int terminate;

	log("Received USR2 - terminate request");
	terminate = 1;
}



/* kick out players etc */
void hupsig(int i)
{
	log("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
	do_crashsave(0,0,0);
	exit(0);   /* something more elegant should perhaps be substituted */
}



void logsig(int i)
{
	log("Signal received. Ignoring.");
}

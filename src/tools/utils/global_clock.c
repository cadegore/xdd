/*
 * XDD - a data movement and benchmarking toolkit
 *
 * Copyright (C) 1992-2013 I/O Performance, Inc.
 * Copyright (C) 2009-2013 UT-Battelle, LLC
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */
/* xdd_init_global_clock_network() - Initialize the network so that we can
 *   talk to the global clock timeserver.
 *   
 */
#include "xint.h"

static int
xdd_init_global_clock_network(char *hostname) {
	struct hostent *hostent; /* used to init the time server info */
	in_addr_t addr;  /* Address of hostname from hostent */
#ifdef WIN32
	WSADATA wsaData; /* Data structure used by WSAStartup */
	int wsastatus; /* status returned by WSAStartup */
	char *reason;
	wsastatus = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsastatus != 0) { /* Error in starting the network */
		switch (wsastatus) {
		case WSASYSNOTREADY:
			reason = "Network is down";
			break;
		case WSAVERNOTSUPPORTED:
			reason = "Request version of sockets <2.2> is not supported";
			break;
		case WSAEINPROGRESS:
			reason = "Another Windows Sockets operation is in progress";
			break;
		case WSAEPROCLIM:
			reason = "The limit of the number of sockets tasks has been exceeded";
			break;
		case WSAEFAULT:
			reason = "Program error: pointer to wsaData is not valid";
			break;
		default:
			reason = "Unknown error code";
			break;
		};
		fprintf(xgp->errout,"%s: Error initializing network connection\nReason: %s\n",
			xgp->progname, reason);
		fflush(xgp->errout);
		WSACleanup();
		return(-1);
	} 
	/* Check the version number */
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		/* Couldn't find WinSock DLL version 2.2 or better */
		fprintf(xgp->errout,"%s: Error initializing network connection\nReason: Could not find version 2.2\n",
			xgp->progname);
		fflush(xgp->errout);
		WSACleanup();
		return(-1);
	}
#endif
	/* Network is initialized and running */
	hostent = gethostbyname(hostname);
	if (!hostent) {
		fprintf(xgp->errout,"%s: Error: Unable to identify host %s\n",xgp->progname,hostname);
		fflush(xgp->errout);
#ifdef WIN32
		WSACleanup();
#endif
		return(-1);
	}
	/* Got it - unscramble the address bytes and return to caller */
	addr = ntohl(((struct in_addr *)hostent->h_addr)->s_addr);
	return(addr);
} /* end of xdd_init_global_clock_network() */
/*----------------------------------------------------------------------------*/
/* xdd_init_global_clock() - Initialize the global clock if requested
 */

void
xdd_init_global_clock(nclk_t *nclkp, xdd_plan_t *planp) {
	nclk_t  now;  /* the current time returned by nclk() */


	/* Global clock stuff here */
	if (planp->gts_hostname) {
		int err = xdd_init_global_clock_network(planp->gts_hostname);
		if (err == -1) { /* Problem with the network */
			fprintf(xgp->errout, "%s: Error initializing global clock - network malfunction\n",
				xgp->progname);
			fflush(xgp->errout);
                        *nclkp = 0;
			return;
		} else {
			planp->gts_addr = err;
		}
		clk_initialize(planp->gts_addr, planp->gts_port, planp->gts_bounce, &planp->gts_delta);
		nclk_now(&now);
		planp->ActualLocalStartTime = planp->gts_time - planp->gts_delta; 
		planp->gts_seconds_before_starting = ((planp->ActualLocalStartTime - now) / BILLION); 
		fprintf(xgp->errout, "Global Time now is %lld. Starting in %lld seconds at Global Time %lld\n",
			(long long)(now + planp->gts_delta)/BILLION, 
			(long long)planp->gts_seconds_before_starting, 
			(long long)planp->gts_time/BILLION); 
		fflush(xgp->errout);
		*nclkp = planp->ActualLocalStartTime;
		return;
	}
	nclk_now(nclkp);
	return;
} /* end of xdd_init_global_clock() */


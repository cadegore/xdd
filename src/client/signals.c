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
/*
 * This file contains the signal handler and the routine that initializes
 * the signal handler call.
 */
#include "xint.h"

/*
 * Boolean flag indicating whether the user wishes to enter the debugger on
 * SIGINT (ctrl-c)
 */
static int enter_debugger_on_sigint = 0;
static xdd_plan_t *xdd_signal_planp = NULL;

/*----------------------------------------------------------------------------*/
/* xdd_signal_handler() - Routine that gets called when a signal gets caught. 
 * This will call the appropriate routines to shut down gracefully.
 */
void
xdd_signal_handler(int signum, siginfo_t *sip, void *ucp) {
	int enter_debugger = 0;
       
	fprintf(xgp->errout,"\n%s: xdd_signal_handler: Received signal %d: ", xgp->progname, signum);
	switch (signum) {
		case SIGINT:
			fprintf(xgp->errout,"SIGINT: Interrupt from keyboard\n");
                        if (enter_debugger_on_sigint)
                            enter_debugger = 1;
			break;
		case SIGQUIT:
			fprintf(xgp->errout,"SIGQUIT: Quit from keyboard\n");
			break;
		case SIGABRT:
			fprintf(xgp->errout,"SIGABRT: Abort\n");
			break;
		case SIGTERM:
			fprintf(xgp->errout,"SIGTERM: Termination\n");
			break;
		case SIGUSR1:
			fprintf(xgp->errout,"SIGUSR1: User requests debug mode\n");
                        enter_debugger = 1;
			break;
		default:
			fprintf(xgp->errout,"Unknown Signal - terminating\n");
			break;
	}
	// If plan exists, print existing results
	if (xdd_signal_planp) {
		// Display results header if not already done
		if (xdd_signal_planp->results_header_displayed == 0) {
			results_t* tmprp;
			tmprp = (results_t*)valloc(sizeof(results_t));
			// If failed to create results placeholder, notify of error; otherwise display the header
			if (tmprp == NULL) {
				fprintf(xgp->errout, "%s: xdd_signal_handler/xdd_results_manager: ERROR: Cannot allocate memory for a temporary results structure!\n", xgp->progname);
			} else {
				// Display header
				xdd_results_header_display(tmprp, xdd_signal_planp);
				xdd_signal_planp->results_header_displayed = 1;
			}
			free(tmprp);
		}
		// Timestamp for this finish time
		nclk_t xdd_signal_end_time;
		nclk_now(&xdd_signal_end_time);
		// Placeholder for current target being processed
		int32_t xdd_signal_targ_num;
		// Placeholder for current target's data
		target_data_t *xdd_signal_tdp;
		// For each target,
		for (xdd_signal_targ_num=0 ; xdd_signal_targ_num < xdd_signal_planp->number_of_targets ; xdd_signal_targ_num++) {
			// Get current target's data
			xdd_signal_tdp = xdd_signal_planp->target_datap[xdd_signal_targ_num];
			// Set the finish timestamp of the current target
			xdd_signal_tdp->td_counters.tc_pass_end_time = xdd_signal_end_time;
			xdd_signal_tdp->td_counters.tc_pass_elapsed_time = xdd_signal_tdp->td_counters.tc_pass_start_time - xdd_signal_tdp->td_counters.tc_pass_end_time;
			// Set ending CPU times
			times(&xdd_signal_tdp->td_counters.tc_current_cpu_times);
		}
		// Process final pass and all run data
		xdd_process_pass_results(xdd_signal_planp);
		xdd_process_run_results(xdd_signal_planp);
	}
	fflush(xgp->errout);

        if (enter_debugger) {
            fprintf(xgp->errout,"xdd_signal_handler: Starting Debugger\n");
            xdd_signal_start_debugger();
        }
        else {
            exit(signum);
        }
} /* end of xdd_signal_handler() */

/*----------------------------------------------------------------------------*/
/* xdd_signal_init() - Initialize all the signal handlers
 */
int32_t
xdd_signal_init(xdd_plan_t *planp) {
	int		status;			// status of the sigaction() system call

	
	xdd_signal_planp = planp;
	xgp->sa.sa_sigaction = xdd_signal_handler;			// Pointer to the signal handler
 	sigemptyset( &xgp->sa.sa_mask );				// The "empty set" - don't mask any signals
	xgp->sa.sa_flags = SA_SIGINFO; 					// This indicates that the signal handler will get a pointer to a siginfo structure indicating what happened
	status 	= sigaction(SIGINT,  &xgp->sa, NULL);	// Interrupt from keyboard - ctrl-c
	status += sigaction(SIGQUIT, &xgp->sa, NULL);	// Quit from keyboard 
	status += sigaction(SIGABRT, &xgp->sa, NULL);	// Abort signal from abort(3)
	status += sigaction(SIGTERM, &xgp->sa, NULL);	// Termination
	status += sigaction(SIGUSR1, &xgp->sa, NULL);	// Termination

	if (status) {
		fprintf(xgp->errout,"%s: xdd_signal_init: ERROR initializing signal handler(s)\n",xgp->progname);
		perror("Reason");
		return(-1);
	}

        /* If XDD_DEBUG_ENABLE is set at startup, ctrl-c will enter the debugger
           rather than cancel the task */
        if (0 != getenv("XDD_DEBUG_ENABLE"))
            enter_debugger_on_sigint = 1;
        
	return(0);

} /* end of xdd_signal_init() */

/*----------------------------------------------------------------------------*/
/* xdd_signal_start_debugger() - Will start the Interactive Debugger Thread
 */
void
xdd_signal_start_debugger() {
	int32_t		status;			// Status of a subroutine call


	if (xdd_signal_planp) {
		status = pthread_create(&xgp->Interactive_Thread, NULL, xdd_interactive, (void *)xdd_signal_planp);
		if (status) {
			fprintf(xgp->errout,"%s: xdd_signal_start_debugger: ERROR: Could not start debugger control processor\n", xgp->progname);
			fflush(xgp->errout);
			exit(1);
		}
	} else {
		fprintf(xgp->errout,"%s: xdd_signal_start_debugger: ERROR: Could not start debugger control processor - no planp pointer\n", xgp->progname);
		fflush(xgp->errout);
		return;
	} 

} // End of xdd_signal_start_debugger()

/*
 * Local variables:
 *  indent-tabs-mode: t
 *  default-tab-width: 4
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ts=4 sts=4 sw=4 noexpandtab
 */

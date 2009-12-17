/* Copyright (C) 1992-2010 I/O Performance, Inc. and the
 * United States Departments of Energy (DoE) and Defense (DoD)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program in a file named 'Copying'; if not, write to
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139.
 */
/* Principal Author:
 *      Tom Ruwart (tmruwart@ioperformance.com)
 * Contributing Authors:
 *       Steve Hodson, DoE/ORNL
 *       Steve Poole, DoE/ORNL
 *       Bradly Settlemyer, DoE/ORNL
 *       Russell Cattelan, Digital Elves
 *       Alex Elder
 * Funding and resources provided by:
 * Oak Ridge National Labs, Department of Energy and Department of Defense
 *  Extreme Scale Systems Center ( ESSC ) http://www.csm.ornl.gov/essc/
 *  and the wonderful people at I/O Performance, Inc.
 */
#include "xdd.h"

//******************************************************************************
// Before I/O Loop
//******************************************************************************
/*----------------------------------------------------------------------------*/
/* xdd_timer_calibration_before_io_loop() - This subroutine will check the 
 * resolution of the timer on the system and display information about it if 
 * requested.
 */
void
xdd_timer_calibration_before_io_loop(void) {
	pclk_t	t1,t2,t3;
#ifdef WIN32 
        int32_t i;
#else
        int 	status;
        int32_t i;
	struct timespec req;
	struct timespec rem;
#endif

	if (xgp->global_options & GO_TIMER_INFO) {
		pclk_now(&t1);
		for (i=0; i<100; i++) 
			pclk_now(&t2);
		pclk_now(&t2);
		t3=t2-t1;

		fprintf(xgp->output,"XDD Timer Calibration Info: Average time to get time stamp=%llu nanoseconds\n",t3/100);

#ifdef WIN32
		for (i=1; i< 1001; i*=10) {
			pclk_now(&t1);
			Sleep(i);
			pclk_now(&t3);
			t3 -= t1;
fprintf(xgp->output,"XDD Timer Calibration Info: Requested sleep time in microseconds=%d, Actual sleep time in microseconds=%llu\n",i*1000,t3/MILLION);
		}
#else // Do this for Systems other than Windows
		for (i=1; i< 1000001; i*=10) {
			req.tv_sec = 0;
			req.tv_nsec = i;
			rem.tv_sec = 0;
			rem.tv_nsec = 0;
			pclk_now(&t1);
			status = nanosleep(&req, &rem);
			pclk_now(&t3);
			if (status) { // This means that the nanosleep was interrupted early
				req.tv_sec = rem.tv_sec;
				req.tv_nsec = rem.tv_nsec;
				if (i > 1) 
					i=1;
				continue;
			}
			t3 -= t1;
fprintf(xgp->output,"XDD Timer Calibration Info: Requested sleep time in nanoseconds=%d, Actual sleep time in nanoseconds=%llu\n",i*1000,t3/MILLION);
		}
#endif
	}
} // End of xdd_timer_calibration_before_io_loop()

/*----------------------------------------------------------------------------*/
/* xdd_start_delay_before_io_loop() - This subroutine will sleep for the period 
 * of time specified by the -startdelay option.  
 */
void
xdd_start_delay_before_io_loop(ptds_t *p) {
	int32_t 	sleep_time_dw;
	int		status;
	uint64_t 	tmp;	// Temp 
	struct timespec	req;	// The requested sleep time
	struct timespec	rem;	// The remaining sleep time is awoken early


	/* Check to see if this thread has a start delay time. If so, just hang out
	 * until the start delay time has elapsed and then spring into action!
	 * The start_delay time is stored as a pclk_t which is units of pico seconds.
	 */
	if (p->start_delay <= 0)
		return; // No start delay

	// Convert the start delay from nanoseconds to milliseconds
	sleep_time_dw = (int32_t)(p->start_delay/BILLION);
#ifdef WIN32
	Sleep(sleep_time_dw);
#elif (IRIX )
	sginap((sleep_time_dw*CLK_TCK)/1000);
#else
	tmp = p->start_delay / 1000000000000; // Convert pclk to seconds
	req.tv_sec = (int)(tmp);
	req.tv_nsec = 0; // Nothing less than a second
	rem.tv_sec = 0; // zero this out just to make sure
	rem.tv_nsec = 0; // zero this out just to make sure
	status = nanosleep(&req, &rem);
	while (status) { // If we woke up early, just keep going to sleep for a while
		req.tv_sec = rem.tv_sec;
		req.tv_nsec = rem.tv_nsec;
		rem.tv_sec = 0; // zero this out just to make sure
		rem.tv_nsec = 0; // zero this out just to make sure
		status = nanosleep(&req, &rem);
	} 
#endif
} // End of xdd_start_delay_before_io_loop()


/*----------------------------------------------------------------------------*/
/* xdd_lockstep_before_io_loop() - This subroutine initializes the variables that
 * are used by the end-to-end option
 */
void // Lock Step Processing
xdd_lockstep_before_io_loop(ptds_t *p) {


	p->ls_slave_loop_counter = 0;
	if (p->ls_slave >= 0) { /* I am a master */
		p->ls_interval_base_value = 0;
		if (p->ls_interval_type & LS_INTERVAL_TIME) {
			p->ls_interval_base_value = p->my_pass_start_time;
		}
		if (p->ls_interval_type & LS_INTERVAL_OP) {
			p->ls_interval_base_value = 0;
		}
		if (p->ls_interval_type & LS_INTERVAL_PERCENT) {
			p->ls_interval_base_value = 1; 
		}
		if (p->ls_interval_type & LS_INTERVAL_BYTES) {
			p->ls_interval_base_value = 0;
		}
	} else { /* I am a slave */
		p->ls_task_base_value = 0;
		if (p->ls_task_type & LS_TASK_TIME) {
			p->ls_task_base_value = p->my_pass_start_time;
		}
		if (p->ls_task_type & LS_TASK_OP) {
			p->ls_task_base_value = 0;
		}
		if (p->ls_task_type & LS_TASK_PERCENT) {
			p->ls_task_base_value = 1; 
		}
		if (p->ls_task_type & LS_TASK_BYTES) {
			p->ls_task_base_value = 0;
		}
	}

} // xdd_lockstep_before_io_loop()

/*----------------------------------------------------------------------------*/
/* xdd_raw_before_io_loop() - This subroutine initializes the variables that
 * are used by the read_after_write option
 */
void
xdd_raw_before_io_loop(ptds_t *p) {

	if ((p->target_options & TO_READAFTERWRITE) == 0)
		return;

	// Initialize the read-after-write variables
	p->raw_msg_sent = 0;
	p->raw_msg_recv = 0;
	p->raw_msg_last_sequence = 0;
	p->raw_msg.sequence = 0;
	p->raw_prev_loc = 0;
	p->raw_prev_len = 0;
	p->raw_data_ready = 0;
	p->raw_data_length = 0;

} // End of xdd_raw_before_io_loop()

/*----------------------------------------------------------------------------*/
/* xdd_e2e_before_io_loop() - This subroutine initializes the variables that
 * are used by the end-to-end option
 */
void
xdd_e2e_before_io_loop(ptds_t *p) {

	if ((p->target_options & TO_ENDTOEND) == 0)
		return;

	// Initialize the read-after-write variables
	p->e2e_msg_sent = 0;
	p->e2e_msg_recv = 0;
	p->e2e_msg_last_sequence = 0;
	p->e2e_prev_loc = 0;
	p->e2e_prev_len = 0;
	p->e2e_data_ready = 0;
	p->e2e_data_length = 0;
	p->e2e_sr_time = 0;

} // End of xdd_e2e_before_io_loop()

/*----------------------------------------------------------------------------*/
/* xdd_io_loop_before_loop() - This subroutine will do all the stuff needed to 
 * be done before entering the inner-most loop that does all the I/O operations
 * that constitute a "pass".
 * Return values: 0 is good
 *                1 is bad
 */
int32_t
xdd_io_loop_before_loop(ptds_t *p) {
	int32_t  i;

	// Timer Calibration and Information
	xdd_timer_calibration_before_io_loop();

	// Process Start Delay
	xdd_start_delay_before_io_loop(p);

	// Lock Step Processing
	xdd_lockstep_before_io_loop(p);

	// Read-After_Write setup
	xdd_raw_before_io_loop(p);

	// End-to-End setup
	xdd_e2e_before_io_loop(p);

	/* Initialize counters, barriers, clocks, ...etc */
	p->syncio_barrier_index = 0;
	p->iosize = p->reqsize * p->block_size;

	/* Get the starting time stamp */
	if (p->my_current_pass_number == 1) {
		pclk_now(&p->first_pass_start_time);
		p->my_pass_start_time = p->first_pass_start_time;
		// Get the current CPU user and system times 
		times(&p->my_starting_cpu_times_this_run);
		memcpy(&p->my_starting_cpu_times_this_pass,&p->my_starting_cpu_times_this_run, sizeof(struct tms));
	} else { 
		pclk_now(&p->my_pass_start_time);
		times(&p->my_starting_cpu_times_this_pass);
	}

	// Init all the pass-related variables to 0
	p->my_elapsed_pass_time = 0;
	p->my_pass_ring = 0;
	p->my_first_op_start_time = 0;
	p->my_accumulated_op_time = 0; 
	p->my_accumulated_read_op_time = 0;
	p->my_accumulated_write_op_time = 0;
	p->my_accumulated_pattern_fill_time = 0;
	p->my_accumulated_flush_time = 0;
	//
	p->my_current_op_count = 0; 		// The number of read+write operations that have completed so far
	p->my_current_read_op_count = 0;	// The number of read operations that have completed so far 
	p->my_current_write_op_count = 0;	// The number of write operations that have completed so far 
	p->my_current_bytes_xfered = 0;		// Total number of bytes transferred to far (to storage device, not network)
	p->my_current_bytes_read = 0;		// Total number of bytes read to far (from storage device, not network)
	p->my_current_bytes_written = 0;	// Total number of bytes written to far (to storage device, not network)
	p->my_current_byte_location = 0; 	// Current byte location for this I/O operation 
	p->my_io_status = 0; 				// I/O Status of the last I/O operation for this qthread
	p->my_io_errno = 0; 				// The errno associated with the status of this I/O for this thread
	p->my_error_break = 0; 			// This is set after an I/O Operation if the op failed in some way
	p->my_current_error_count = 0;		// The number of I/O errors for this qthread
	p->my_current_state = CURRENT_STATE_IO;	// State of this thread at any given time (see Current State definitions below)
	// State Definitions for "my_current_state"
#define	CURRENT_STATE_IO		0x0000000000000001	// Currently waiting for an I/O operation to complete
#define	CURRENT_STATE_BTW		0x0000000000000002	// Currently between I/O operations
#define	CURRENT_STATE_BARRIER	0x0000000000000004	// Currently stuck in a barrier
	//
	// Longest and shortest op times - RESET AT THE START OF EACH PASS
	p->my_longest_op_time = 0;			// Longest op time that occured during this pass
	p->my_longest_op_number = 0; 		// Number of the operation where the longest op time occured during this pass
	p->my_longest_read_op_time = 0; 	// Longest read op time that occured during this pass
	p->my_longest_read_op_number = 0; 	// Number of the read operation where the longest op time occured during this pass
	p->my_longest_write_op_time = 0; 	// Longest write op time that occured during this pass
	p->my_longest_write_op_number = 0; 	// Number of the write operation where the longest op time occured during this pass
	p->my_shortest_op_time = PCLK_MAX; 	// Shortest op time that occurred during this pass
	p->my_shortest_op_number = 0; 		// Number of the operation where the shortest op time occured during this pass
	p->my_shortest_read_op_time = PCLK_MAX;	// Shortest read op time that occured during this pass
	p->my_shortest_read_op_number = 0; 	// Number of the read operation where the shortest op time occured during this pass
	p->my_shortest_write_op_time = PCLK_MAX;// Shortest write op time that occured during this pass
	p->my_shortest_write_op_number = 0;	// Number of the write operation where the shortest op time occured during this pass
	return(0);

} // End of xdd_io_loop_before_loop()

 

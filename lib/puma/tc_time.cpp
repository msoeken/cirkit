#include"tc_time.h"

#include <iostream>
using namespace std;


/*=============================================================================
 * Function:	long stop_watch::start();
 * Purpose:	defines the point to start measuring the CPU-time
 * Parameters:  -
 *===========================================================================*/
void stop_watch::start() {

if(times(buffer) == EFAULT) {
	cerr << "stop_watch::start() : Error! Cannot call function times() \n"; 
	exit(-1);
	}
else {
	start_ticks = buffer->tms_utime;
	}
}




/*=============================================================================
 * Function:	long stop_watch::stop();
 * Purpose:	returns CPU-time in 1/100 sec elapsed since last call of 
 *              start() or stop().
 *              Defines a new point to start measuring the CPU-time.
 * Parameters:  -
 *===========================================================================*/
long stop_watch::stop() {

if(times(buffer) == EFAULT) {
	cerr << "stop_watch::stop() : Error! Cannot call function times() \n"; 
	exit(-1);
	}
else {
	used_ticks = buffer->tms_utime - start_ticks;
	start_ticks = buffer->tms_utime;
	return(used_ticks * 100 / ticks_per_second);
	}
return(-1);
}


/*=============================================================================
 * Function:	long stop_watch::lap();
 * Purpose:	returns CPU-time in 1/100 sec elapsed since last call of 
 *              start() or stop().
 *              The old point to start measuring the CPU-time is retained.
 * Parameters:  -
 *===========================================================================*/
long stop_watch::lap() {

if(times(buffer) == EFAULT) {
	cerr << "stop_watch::lap() : Error! Cannot call function times() \n"; 
	exit(-1);
	}
else {
	used_ticks = buffer->tms_utime - start_ticks;
	return(used_ticks * 100 / ticks_per_second);
	}
return(-1);
}

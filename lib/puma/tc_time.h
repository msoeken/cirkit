/***************************************************************************
+
+  STOP_WATCH                                              04-09-1995
+
+  Class to measure CPU-time used by a c++ programm 
+
+  Based on tc_time.h developed by Rolf Krieger at J.W.Goethe University.
+  More portable by the use of function times() instead of getrusage().
+  Tested under the following configurations:
+
+	Hardware	OS			Compiler
+     -----------------------------------------------------------
+	SUN Sparc 	SunOS 4.1.4		GNU g++ 2.7.0
+	SUN Sparc	SunOS 5.4 (Solaris)	GNU g++ 2.6.3  
+	HP series 9000	HU-UX A.09.05		GNU g++ 2.5.8
+	Indy R4600	IRIX  5.3		Delta/C++
+
+
+  Copyright (C) 1995  by Albert-Ludwigs-University of Freiburg.
+  Am Flughafen 17, 79110 Freiburg, Germany
+  All rights reserved.
+ 
+
+ Author: Harry Hengster
+ E-mail: hengster@informatik.uni-freiburg.de
+
***************************************************************************/

/*=============================================================================
 * Class:	stop_watch
 * Purpose:	measure CPU-time used by a c++ programm
 *===========================================================================*/
#ifndef STOP_WATCH
#define STOP_WATCH = 1

#include<stdlib.h>
#include<unistd.h>
#include<sys/times.h>
#include<sys/errno.h>

// Defintion der Klasse stop_watch
class stop_watch {
      long start_ticks;
      long used_ticks;
      long ticks_per_second;
      tms* buffer;
  public:
      stop_watch() { buffer = new tms; 
		     ticks_per_second = sysconf(_SC_CLK_TCK); 
		   }
      ~stop_watch() { delete buffer;}
      void start();
      long stop();
      long lap();
};
#else
#endif

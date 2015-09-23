/*
 * serial.cc
 *
 * C++ 11 utility library (libnutil)
 *
 * Nomovok (C) 2015 A. Dureghello
 *
 */

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sched.h>
#include <sys/mman.h>
#include "realtime.hh"

#define MY_PRIORITY (49) /* we use 49 as the PRREMPT_RT use 50
                            as the priority of kernel tasklets
                            and interrupt handler by default */

#define MAX_SAFE_STACK (8*1024) /* The maximum stack size which is
                                   guaranteed safe to access without
                                   faulting */

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */

namespace nomovok {
namespace util {

void set_thread_rt_prio_or_die(int value)
{
	struct sched_param param;

        param.sched_priority = value;
        if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
                perror("set_thread_rt_prio_or_die(): failed");
                exit(-1);
        }
}

void init_realtime()
{
        struct sched_param param;

        param.sched_priority = MY_PRIORITY;

        if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
                perror("init_realtime(): sched_setscheduler failed");
                exit(-1);
        }

        /* Lock memory */

        if(mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
                perror("init_realtime(): mlockall failed");
                exit(-2);
        }
}

} /* end of ns util */
} /* end of ns nomovok */
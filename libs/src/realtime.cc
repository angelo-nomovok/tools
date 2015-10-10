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
#include <cstring>
#include <cinttypes>
#include <cassert>
#include <iostream>

#include <sched.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#include "realtime.hh"

using namespace std;

namespace nomovok {
namespace util {

int last_majflt = 0;
int last_minflt = 0;

/*
 * the number of uninterrupted microseconds to let a realtime task run
 * before killing it.
 */
typedef uint64_t rlim_t;
static const rlim_t cpu_limit_us = 3000000;
/*
 * Sets the CPU limit of a task.  0 means self.
 */
void rt_set_cpu_limit_or_die(pid_t pid, rlim_t limit)
{
	struct rlimit64 rl;

	// Obtain the current limits.
	assert(prlimit64(pid, RLIMIT_RTTIME, nullptr, &rl) == 0);

	// Set a CPU limit.
	rl.rlim_cur = limit;
	assert(prlimit64(pid, RLIMIT_RTTIME, &rl, nullptr) == 0);
}

/*
 * Each task has associated a static scheduling _policy_ and
 * a static scheduling _priority_.
 *
 * The scheduler makes its decisions based on knowledge of the scheduling
 * policy and static priority of all threads on the system.
 *
 * For threads scheduled under one of the normal scheduling policies
 * (SCHED_OTHER, SCHED_IDLE, SCHED_BATCH), sched_priority is not used
 * in scheduling decisions (it must be specified as 0).
 *
 */
void rt_init()
{
	/*
	 * 1st - lock memory to stay into RAM, no swap
	 * avoid page faults and related handling
	 *
	 */
        if(mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
                perror("init_realtime(): mlockall failed");
                exit(-2);
        }

        rt_stack_prefault();


	/*
	 * SCHED_FIFO, SCHED_RR have ranges from 1 to 99(higher prio.).
	 *
	 * Portable programs should use sched_get_priority_min(2) and
	 * sched_get_priority_max(2) to find the range of priorities
	 * supported for a particular policy.
	 *
	 * RT policies are:
	 *
	 * SCHED_FIFO  : First in-first out scheduling
	 * SCHED_RR    : Round-robin scheduling, as FIFO with max time quantum
	 *
	 * See man sched()
	 *
	 * We would use SCHED_RR here.
	 *
	 */

	cout << "rt_init(): SCHED_RR allowed priorities are: "
		<< "min:" << sched_get_priority_min(SCHED_RR) << ", "
		<< "max:" << sched_get_priority_max(SCHED_RR)
		<< "\n";

        struct rusage usage;

	getrusage(RUSAGE_SELF, &usage);

	cout << "rt_init(): faults during startup : maj:"
		<< usage.ru_majflt - last_majflt
		<< ", min: " << usage.ru_minflt - last_minflt
		<< "\n";

	last_majflt = usage.ru_majflt;
	last_minflt = usage.ru_minflt;
}

/*
 * The maximum stack size which is
 * guaranteed safe to access without
 * faulting
 */
static const int max_safe_stack = 8 * 1024;

void rt_stack_prefault()
{
	unsigned char dummy[max_safe_stack];

        memset(dummy, 0, max_safe_stack);

        return;
}

/*
 * From sched.h
 * ------------
 * Priority of a process goes from 0..MAX_PRIO-1, valid RT
 * priority is 0..MAX_RT_PRIO-1, and SCHED_NORMAL/SCHED_BATCH
 * tasks are in the range MAX_RT_PRIO..MAX_PRIO-1. Priority
 * values are inverted: lower p->prio value means higher priority.
 *
 */
void rt_set_thread_prio_or_die(pthread_t thread, int value)
{
	struct sched_param param;

	/* SCHED_RR, priority min is 1 max is 99 */

        param.sched_priority = value + sched_get_priority_min(SCHED_RR);

	/*
	 * we are using thread Posix API, so  pthread_setschedparam should
	 * be used instead of sched_xxx etc
	 */
        if(pthread_setschedparam(thread, SCHED_RR, &param) == -1) {
                perror("set_thread_rt_prio_or_die(): failed");
                exit(-1);
        }

        cout << "rt_set_thread_prio_or_die(): set priority as: "
		<< param.sched_priority << "\n";
}

void rt_set_thread_prio_or_die(int value)
{
	rt_set_cpu_limit_or_die(0, cpu_limit_us);
	rt_set_thread_prio_or_die(pthread_self(), value);
}

/*
 * this function migrate the process to desired cpu
 */
void rt_set_processor_affinity(int core_id)
{
	cpu_set_t cpuset;

	CPU_ZERO(&cpuset);
	CPU_SET(core_id, &cpuset);

	pthread_t current_thread = pthread_self();
	assert(pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset)
		== 0);

	cout << "pinned task " << syscall(SYS_gettid) << " to core " << core_id;
}

} /* end of ns util */
} /* end of ns nomovok */
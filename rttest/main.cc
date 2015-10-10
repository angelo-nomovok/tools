/*
 * Copyright (C) 2015 Peloton Technology, Inc. - All Rights Reserved
 * Author: Philipp Schrader (philipp@peloton-tech.com)
 *
 * This tool is intended to stress-test the real-time serial performance of a
 * VPC. It does this by saturating the serial port and reading bytes from the
 * same port as fast as it can. It expects the same application to be running
 * on the other end of the serial port. An 8-bit counter value is sent across
 * that the two instances of this tool try to keep in sync. If the counters
 * ever get out of sync, then it quits (or prints an error if
 * "-missed_packets_fatal=false" is passed as an argument).
 *
 * 23.09.2015  - modified by A. Dureghello - Nomovok OY
 *
 */

#include <cinttypes>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <sys/utsname.h>

#include "serial.hh"
#include "realtime.hh"
#include "general.hh"
#include "clock.hh"

static const int thread_stack_size = (100*1024);

using namespace nomovok;
using namespace std;

namespace peloton {

static bool exit_requested = false;

// Signal handler for CTRL-C and such.
void signal_handler(int signum)
{
	exit_requested = true;
}

/*
 * Be sure all page faults are generated befor use
 * and avoid default 8MB stack
 */
void setup_thread_stack_minimal(int stacksize)
{
	volatile char buffer[stacksize];
	int i;

	/* Prove that this thread is behaving well */
	for (i = 0; i < stacksize; i += sysconf(_SC_PAGESIZE)) {
		/* Each write to this buffer shall NOT generate a
			pagefault. */
		buffer[i] = i;
	}
}

void* thread_uart_rx(void *arg)
{
	util::serial *sp = (util::serial *)arg;
	int8_t rxchar = 0;
	int8_t rxnext = 0;
	static char buff[1024];

	setup_thread_stack_minimal(thread_stack_size);

	while (!exit_requested) {
		if (read(sp->fd(), &rxchar, 1) == 1) {
			if (rxchar != rxnext) {
				printf("err: exp %4d, received %4d\n",
					rxnext, rxchar
				);
				/* try to clear buffer */
				read(sp->fd(), buff, 1024);
			}
			rxnext = rxchar + 1;
		}
	}
}

void* thread_uart_tx(void *arg)
{
	util::serial *sp = (util::serial *)arg;

	setup_thread_stack_minimal(thread_stack_size);

	while (!exit_requested) {


	}
}

bool is_linux_rt()
{
	struct utsname u;
	char *p;
	FILE *fd;
	int rt = 0;

	uname(&u);
	p = strcasestr(u.version, "PREEMPT RT");

	if ((fd = fopen("/sys/kernel/realtime", "r")) != NULL) {
		int flag;
		rt = ((fscanf(fd, "%d", &flag) == 1) && (flag == 1));
		fclose(fd);
	}

	fprintf(stderr, "this is a %s kernel\n",
            (p && rt)  ? "PREEMPT RT" : "vanilla");

	return (p && rt);
}

/*
 * Here we create a thread with minimal stack, to leave as much as possible
 * memory space in physical ram to other applications.
 */

typedef void *(*start_routine) (void *);

static void start_rt_thread(pthread_t *tid, start_routine run_routine, void *arg)
{
	pthread_t thread;
	pthread_attr_t attr;
	int err;

	/* init to default values */
	if (pthread_attr_init(&attr))
		cout << "++err: start_rt_thread(), can't set attributes";
	/* Set the requested stacksize for this thread */
	if (pthread_attr_setstacksize(&attr,
		PTHREAD_STACK_MIN + thread_stack_size))
		cout << "++err: start_rt_thread(), can't set stack size";
	/* And finally start the actual thread */
	err = pthread_create(tid, &attr, run_routine, arg);

	if (err != 0)
            cout << "++err: start_rt_thread(), can't create thread :[" <<
		 strerror(err) << "]\n";
}

int run(const string& device)
{
	int err;
	pthread_t tid[2];

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	util::serial sp(device);
	sp.set_speed(B115200);

	start_rt_thread(&tid[0], thread_uart_rx, &sp);
	start_rt_thread(&tid[1], thread_uart_tx, &sp);

	pthread_join(tid[0], 0);
	pthread_join(tid[1], 0);

	return 0;
}

}  // namespace peloton

void usage()
{
	cout << "usage: rtt device [prio]\r\n\r\n";
}

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		usage();
		exit(0);
	}

	int priority = 1;

	if (argc == 3) {
		stringstream ss;

		ss << argv[2];
		ss >> priority;

		if (priority < 1 || priority > 99) {
			cout << "++err: invalid priority\n";
			exit(0);
		}
	}

	if (peloton::is_linux_rt()) {
		util::rt_init();
		util::rt_set_thread_prio_or_die(priority);
	}

	return peloton::run(argv[1]);
}


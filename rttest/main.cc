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
#include <thread>
#include <iomanip>
#include <sstream>

#include <unistd.h>

#include "serial.hh"
#include "realtime.hh"
#include "general.hh"
#include "clock.hh"

using namespace nomovok;
using namespace std;

namespace peloton {

static bool exit_requested = false;

// Signal handler for CTRL-C and such.
void signal_handler(int signum)
{
	exit_requested = true;
}

void* thread_uart_rx(void *arg)
{
	util::serial *sp = (util::serial *)arg;
	int8_t rxchar = 0;
	int8_t rxnext = 0;
	static char buff[1024];

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

	while (!exit_requested) {


	}
}

int run()
{
	int err;
	pthread_t tid[2];

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	util::serial sp(string("/dev/ttyS0"));
	sp.set_speed(B115200);

        err = pthread_create(&(tid[0]), NULL, &thread_uart_rx, &sp);
        if (err != 0)
            printf("\ncan't create thread :[%s]\n", strerror(err));

	err = pthread_create(&(tid[1]), NULL, &thread_uart_tx, &sp);
        if (err != 0)
            printf("\ncan't create thread :[%s]\n", strerror(err));

	pthread_join(tid[0], 0);
	pthread_join(tid[1], 0);

	return 0;
}

}  // namespace peloton

int main(int argc, char *argv[])
{
	util::rt_init();

	return peloton::run();
}


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

#if 0

const char usage[] = "Usage: uart_rt_test <options>";

::std::atomic_bool exit_requested{false};

// Helper class to take care of actually sending CAN frames and receiving them.
// One one side of the CAN interface, call the `Send` function in a loop and on
// the other call the `Receive` function in a loop.
class UartTester
{
public:
	UartTester(int fd) :
	fd_(fd),
	counter_(0),
	num_successes_(0),
	start_time_(util::monotonic_clock::min_time)
	{}

	// Send a single value with an incrementing counter value.
	void Send() {
		if (WritePacket(counter_)) {
			CaptureStartTime();
			++counter_;
			++num_successes_;
		}
	}

	// Receive the next counter value.
	void Receive() {
		int8_t received = 0;

		if (ReadPacket(&received)) {
			CaptureStartTime();

			if (FLAGS_missed_packets_fatal) {
				CHECK_EQ(counter_, received);
			} else if (counter_ != received) {
				stringstream ss;

				ss << "++ERR: expected "
					<< dec << setw(4) << setfill(' ')
					<< static_cast<int>(counter_)
					<< " ["
					<< hex << setw(2) << setfill('0')
					<< (static_cast<int>(counter_) & 0xff)
					<< "] got "
					<< dec << setw(4) << setfill(' ')
					<< static_cast<int>(received)
					<< " ["
					<< hex << setw(2) << setfill('0')
					<< (static_cast<int>(received) & 0xff)
					<< "]";

				LOG(ERROR) << ss.str();
			}

			// If we loose a packet we don't want to start generating
			// errors on every subsequent packet (since the counters
			// would then be out of sync).
			// Instead, skip the local counter ahead to what
			// the sending side
			// sent so that subsequent packets are back in sync.
			counter_ = received + 1;
			++num_successes_;
		}
	}

	int8_t counter() const { return counter_; }

	uint64_t num_successes() const { return num_successes_; }

	util::monotonic_clock::time_point start_time() const
	{ return start_time_; }

private:
	// In case this is the first time we send or receive a packet, we want to
	// note this as the start time. This helps the higher-level logic
	// determine when the first packet was _actually_ sent/received.
	void CaptureStartTime() {
		if (num_successes_ == 0) {
			start_time_ = util::monotonic_clock::now();
		}
	}

	bool WritePacket(int8_t byte) const {
		if (write(fd_, &byte, sizeof(byte)) ==
				static_cast<ssize_t>(sizeof(byte))) {
			return true;
		}

		return false;
	}

	bool ReadPacket(int8_t *byte) const {
		CHECK_NOTNULL(byte);

		if (read(fd_, byte, sizeof(*byte)) ==
			static_cast<ssize_t>(sizeof(*byte))) {
			return true;
		}

		return false;
	}

	int fd_;
	int8_t counter_;
	uint64_t num_successes_;
	util::monotonic_clock::time_point start_time_;
};

speed_t ParseBaudRate(int32_t baud_rate)
{
	speed_t result = B0;

	switch (baud_rate) {
	case 9600:
		result = B9600;
	break;
	case 115200:
		result = B115200;
	break;
	default:
		LOG(FATAL) << "Unsupported baud rate: " << baud_rate;
	}

	return result;
}

void PrintResults(const char *title,
                  const util::monotonic_clock::time_point &start_time,
                  const util::monotonic_clock::time_point &end_time,
                  const UartTester &tester)
{
	auto accurate_start_time = start_time;

	// If we managed to actually send/receive some frames, then the tester
	// will have a better estimate of when that first frame
	// was sent/received.
	if (tester.num_successes() > 0) {
		accurate_start_time = tester.start_time();
	}

	// Calculate and then print some statistics.
	const double total_time =
		util::duration_in_seconds(end_time - accurate_start_time);
	const double frames_per_sec = tester.num_successes() / total_time;
	const double avg_us_per_frame = 1000 * 1000 / frames_per_sec;

	printf("==== %s ====\n", title);
	printf("Elapsed time = %.6f\n", total_time);
	printf("Num packets = %" PRIu64 "\n", tester.num_successes());
	printf("Avg packets/s = %.2f\n", frames_per_sec);
	printf("Avg us/packet = %.2f\n", avg_us_per_frame);
}

void SendPacketsUntilCancelled(UartTester &tester) {
	while (!exit_requested && tester.num_successes() < FLAGS_num_packets) {
		tester.Send();
	}
}

void ReceivePacketsUntilCancelled(UartTester &tester) {
	while (!exit_requested && tester.num_successes() < FLAGS_num_packets) {
		tester.Receive();
	}
}

#endif

static ::std::thread thread_rx;

static bool exit_requested = false;

// Signal handler for CTRL-C and such.
void signal_handler(int signum)
{
	exit_requested = true;
}

void* thread_uart_rx(void *arg)
{
	util::serial *sp = (util::serial *)arg;
	char rxchar = 0;
	char rxlast;

	while (!exit_requested) {
		if (read(sp->fd(), &rxchar, 1) == 1) {
			if ((int)rxchar != (rxlast + 1)) {
				printf("err: exp %d, received %d\n",
					(int)rxchar, (int)rxlast
				);
			}
			rxlast = rxchar;
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


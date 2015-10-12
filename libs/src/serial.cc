/*
 * serial.cc
 *
 * C++ 11 utility library (libnutil)
 *
 * Nomovok (C) 2015 A. Dureghello
 *
 */

#include "serial.hh"

#include <cerrno>
#include <cstring>
#include <sstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using std::string;
using std::stringstream;

namespace nomovok {
namespace util {

/*
 * Posix std serial port notes
 *
 * Serial can work in
 *
 * a) blocking or non blocking
 *
 * Blocking is the default, non-blocking can be set with
 * O_NONBLOCK or O_NDELAY.
 *
 * b) canonical (read full lines) or
 * non-canonical (read single chars)
 *
 * This can be set with ICANON in c_flags.
 */

serial::serial(const string& device) : _device(device)
{
	open(device);
}

serial::~serial()
{
	if (fds) {
		tcsetattr(fds, TCSANOW, &oldterm);
		close(fds);
	}
}

void serial::open(const string &device)
{
	fds = ::open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

        if (fds == -1)
        {
                stringstream msg;

                msg << "ComPort::" << __func__
                    << " unable to open port: " << device;

                perror(msg.str().c_str());

                return;
        }
        else
        {
                /*
		 * set this if read have to return immediately (non-blocking)
		 */
                struct termios options;

                tcgetattr(fds, &options);

                memcpy (&oldterm, &options, sizeof(struct termios));
                memset (&options, 0, sizeof(struct termios));

		cfmakeraw(&options);

		/*
		 * non-canonical read (single char, raw),
		 * VMIN is the minimum char num to wait / read
		 */
		options.c_cc[VMIN] = 1;
		/*
		 * VTIME sets the timeout to block (deciseconds)
		 * but, if set to 0, wait until at lease VMIN is in the
		 * buffer
		 */
		options.c_cc[VTIME] = 10;

		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CRTSCTS;    /* no HW flow control? */
		options.c_cflag |= CLOCAL | CREAD;

		tcsetattr(fds, TCSANOW, &options);
		tcflush(fds, TCIFLUSH);
                tcflush(fds, TCOFLUSH);
        }
}

void serial::set_speed(speed_t speed)
{
	struct termios options;

	tcgetattr(fds, &options);

	cfsetispeed(&options, speed);
	cfsetospeed(&options, speed);

	tcsetattr(fds, TCSANOW, &options);
	tcflush(fds, TCIFLUSH);
}

void serial::flush_input()
{
	tcflush	(fds, TCIFLUSH);
}

void serial::reset()
{
	if (fds) {
		close(fds);
		open(_device);
	}
}

} /* end of ns util */
} /* end of ns nomovok */

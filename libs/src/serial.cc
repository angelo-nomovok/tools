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

serial::serial(fLS::clstring& device)
{
	fds = open(device.c_str(), O_RDWR | O_NOCTTY);

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
                 // fcntl(fds, F_SETFL, FNDELAY);

                struct termios options;

                tcgetattr(fds, &options);

                memcpy (&oldterm, &options, sizeof(struct termios));
                memset (&options, 0, sizeof(struct termios));

                options.c_cflag |= (CLOCAL | CREAD);
                options.c_cflag &= ~CSIZE;
                // no flow control
                options.c_cflag &= ~CRTSCTS;
		/* default parity none */
                options.c_cflag &= ~(PARODD | PARENB);
		/* 1 stop bit */
                options.c_cflag &= ~CSTOPB;
		/* 8 stop bits */
		options.c_cflag |= CS8;

                // turn off s/w flow ctrl
                options.c_iflag = IGNPAR | IGNBRK;

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
		options.c_cc[VTIME] = 0;

                options.c_lflag = 0; /* non canonical, ICANON not set */
                options.c_oflag = 0;

                tcsetattr	(fds, TCSANOW, &options);
                tcflush		(fds, TCIFLUSH);
                tcflush         (fds, TCOFLUSH);
        }
}

serial::~serial()
{
	if (fds) {
		tcsetattr(fds, TCSANOW, &oldterm);
		close(fds);
	}
}

void serial::set_speed(speed_t speed)
{
	struct termios options;

	tcgetattr(fds, &options);

	cfsetispeed(&options, speed);
	cfsetospeed(&options, speed);

	tcsetattr(fds, TCSANOW, &options);
	tcflush	(fds, TCIFLUSH);

}

void serial::flush_input()
{
	tcflush	(fds, TCIFLUSH);
}

} /* end of ns util */
} /* end of ns nomovok */
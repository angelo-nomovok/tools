#ifndef __serial_hh
#define __serial_hh

#include <termios.h>
#include "gflags/gflags.h"

namespace nomovok {
namespace util {

class serial
{
public:
	serial() {}
	serial(fLS::clstring&);
	~serial();

	void set_speed(speed_t speed);

	int fd() { return fds; }

	void flush_input();

private:
	int fds;
	struct termios oldterm;
};

} /* end of ns util */
} /* end of ns nomovok */

#endif // __serial_hh
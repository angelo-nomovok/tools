#ifndef __serial_hh
#define __serial_hh

#include <termios.h>
#include <string>

using std::string;

namespace nomovok {
namespace util {

class serial
{
public:
	serial() {}
	serial(const string &device);
	~serial();

	void set_speed(speed_t speed);

	int fd() { return fds; }

	void flush_input();
	void flush_output();
	void flush();
	void reset();
	void open(const string &device);

private:
	int fds;
	struct termios oldterm;
	string _device;
	speed_t _speed;
};

} /* end of ns util */
} /* end of ns nomovok */

#endif // __serial_hh
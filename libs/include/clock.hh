#ifndef __clock_hh
#define __clock_hh

#include <chrono>

namespace nomovok {
namespace util {

class monotonic_clock
{
public:
	monotonic_clock() {}

	typedef std::chrono::time_point<std::chrono::high_resolution_clock,
		std::chrono::nanoseconds> time_point;

	static time_point now()
	{ return std::chrono::high_resolution_clock::now(); }

	static time_point min_time;
};

double duration_in_seconds(const monotonic_clock::time_point::duration &tp);

} /* end of ns util */
} /* end of ns nomovok */

#endif // __clock_hh
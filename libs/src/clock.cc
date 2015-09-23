/*
 * clock.cc
 *
 * C++ 11 utility library (libnutil)
 *
 * Nomovok (C) 2015 A. Dureghello
 *
 */

#include "clock.hh"

namespace nomovok {
namespace util {

monotonic_clock::time_point monotonic_clock::min_time = monotonic_clock::now();

double duration_in_seconds(const monotonic_clock::time_point::duration &tp)
{
	return std::chrono::duration_cast<std::chrono::duration<double>>
		(tp).count();
}

} /* end of ns util */
} /* end of ns nomovok */
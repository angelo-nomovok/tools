/*
 * log.cc
 *
 * C++ utility library (libnutil)
 *
 * Nomovok (C) 2015 A. Dureghello
 *
 */

#include <sys/time.h>
#include <sstream>

#include "log.hh"

using std::stringstream;

namespace nomovok {
namespace util {

string timestamp()
{
	stringstream ss;
	timeval t;

        gettimeofday(&t, NULL);

	ss << "[" << t.tv_sec << "." << t.tv_usec << "] ";

	return ss.str();
}

}
}
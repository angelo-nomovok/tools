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
#include <iomanip>

#include "log.hh"

using namespace std;

namespace nomovok {
namespace util {

string timestamp()
{
	static unsigned long start_secs = 0;
	static timeval t;
	stringstream ss;
	
        gettimeofday(&t, NULL);

	if (!start_secs) start_secs = t.tv_sec;

	ss << "[" << setw(5) << setfill('0') << (t.tv_sec - start_secs) << "."
		  << setw(6) << setfill('0') << t.tv_usec << "] ";

	return ss.str();
}

}
}
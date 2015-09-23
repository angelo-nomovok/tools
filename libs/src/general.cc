/*
 * serial.cc
 *
 * C++ 11 utility library (libnutil)
 *
 * Nomovok (C) 2015 A. Dureghello
 *
 */

#include "gflags/gflags.h"

namespace nomovok {
namespace util {

void init(int *argc, char **argv[])
{
	 google::ParseCommandLineFlags(argc, argv, true);
}

} /* end of ns util */
} /* end of ns nomovok */
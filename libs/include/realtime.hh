#ifndef __realtime_hh
#define __realtime_hh

namespace nomovok {
namespace util {

void set_thread_rt_prio_or_die(int value);
void init_realtime();

} /* end of ns util */
} /* end of ns nomovok */

#endif // __realtime_hh
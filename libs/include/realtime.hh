#ifndef __realtime_hh
#define __realtime_hh

namespace nomovok {
namespace util {

void rt_init();
void rt_stack_prefault();
void rt_set_thread_prio_or_die(int value);

} /* end of ns util */
} /* end of ns nomovok */

#endif // __realtime_hh
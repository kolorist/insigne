#include "insigne/system.h"

namespace insigne {

void wait_for_initialization()
{
	detail::g_init_condvar.wait(detail::g_init_mtx);
}

}

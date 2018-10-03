#include "insigne/system.h"

namespace insigne {

settings_t										g_settings;

void wait_for_initialization()
{
	detail::g_init_condvar.wait(detail::g_init_mtx);
}

}

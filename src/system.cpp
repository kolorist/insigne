#include "insigne/system.h"

namespace insigne {

settings_t										g_settings;

void wait_for_initialization()
{
	detail::s_init_condvar.wait(detail::s_init_mtx);
}

}

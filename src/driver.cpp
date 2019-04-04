#include "insigne/driver.h"
#include "insigne/internal_states.h"

namespace insigne {

static gl_debug_info							s_driver_debug_info;

gl_debug_info& get_driver_info()
{
	return s_driver_debug_info;
}

void request_refresh_context()
{
	detail::g_context_dirty.store(true);
}


}

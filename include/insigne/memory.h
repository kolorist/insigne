#pragma once

#include <floral.h>
#include <helich.h>

namespace insigne {
	typedef helich::allocator<helich::stack_scheme, helich::no_tracking_policy> linear_allocator_t;
	extern linear_allocator_t					g_persistance_allocator;
}

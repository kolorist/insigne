#pragma once

#include <floral.h>

#include "commons.h"
#include "internal_states.h"

namespace insigne {

// -----------------------------------------
template <typename t_surface_list>
void											initialize_render_thread();

void											wait_for_initialization();

}

#include "system.hpp"

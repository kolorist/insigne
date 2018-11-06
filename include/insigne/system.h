#pragma once

#include <floral.h>

#include "commons.h"
#include "internal_states.h"

namespace insigne {

// ---------------------------------------------
void											initialize_render_thread(const u32 i_surfaceTypesCount);
void											wait_for_initialization();

}

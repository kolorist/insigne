#pragma once

#include <floral.h>

#include "commons.h"
#include "configs.h"
#include "internal_states.h"

namespace insigne {

// ---------------------------------------------
void											initialize_render_thread();
void											pause_render_thread();
void											resume_render_thread();
void											wait_finish_dispatching();
void											allocate_draw_command_buffers(const u32 i_maxSurfaceTypes);
template <typename t_surface>
void											register_surface_type();
void											wait_for_initialization();

}

#include "system.hpp"

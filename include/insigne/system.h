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
void											clean_up_and_stop_render_thread();
void											wait_finish_dispatching();

void											organize_memory();
void											allocate_draw_command_buffers(const u32 i_maxSurfaceTypes);
void											allocate_post_draw_command_buffers(const u32 i_maxSurfaceTypes);

template <typename t_surface>
void											register_surface_type();
template <typename t_surface>
void											unregister_surface_type();
template <typename t_surface>
void											register_post_surface_type();
template <typename t_surface>
void											unregister_post_surface_type();

void											unregister_all_surface_types();

void											wait_for_initialization();

}

#include "system.hpp"

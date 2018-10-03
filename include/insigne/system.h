#pragma once

#include <floral.h>

#include "commons.h"
#include "internal_states.h"

namespace insigne {

// -----------------------------------------
struct settings_t {
	u32											frame_allocator_size_mb;
	u32											frame_shader_allocator_size_mb;
	u32											frame_buffers_allocator_size_mb;
	u32											draw_command_buffer_size;
	u32											generic_command_buffer_size;
};
extern settings_t								g_settings;

// -----------------------------------------
template <typename t_surface_list>
void											initialize_render_thread();

void											wait_for_initialization();

}

#include "system.hpp"

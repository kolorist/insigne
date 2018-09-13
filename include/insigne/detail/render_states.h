#pragma once

#include <floral.h>

#include "insigne/commons.h"
#include "insigne/buffers.h"
#include "insigne/memory.h"

namespace insigne {

namespace detail {
// synchronization
extern floral::condition_variable		s_init_condvar;
extern floral::mutex					s_init_mtx;
extern floral::condition_variable		s_cmdbuffer_condvar;
extern floral::mutex					s_cmdbuffer_mtx;

// frame memory
extern arena_allocator_t*				s_gpu_frame_allocator[BUFFERED_FRAMES];

// frame buffering states
typedef floral::fixed_array<gpu_command, linear_allocator_t>	gpu_command_buffer_t;
template <typename t_surface>
struct draw_command_buffer_t {
	static gpu_command_buffer_t			command_buffer[BUFFERED_FRAMES];
};

extern gpu_command_buffer_t				s_generic_command_buffer[BUFFERED_FRAMES];
extern size								s_front_cmdbuff;
extern size								s_back_cmdbuff;

extern bool								s_waiting_for_swap;

// frame rendering states
extern render_state_t					s_render_state;
extern u32								s_render_state_changelog;
// materials are also render states
// TODO: this should be a memory pool of materials
extern floral::fixed_array<material_t, linear_allocator_t>	s_materials;
extern material_handle_t				s_current_material;
}

}

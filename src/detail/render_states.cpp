#include "insigne/detail/render_states.h"

namespace insigne {
namespace detail {

// synchronization
floral::condition_variable				s_init_condvar;
floral::mutex							s_init_mtx;
floral::condition_variable				s_cmdbuffer_condvar;
floral::mutex							s_cmdbuffer_mtx;

// frame memory
arena_allocator_t*						s_gpu_frame_allocator[BUFFERED_FRAMES];

gpu_command_buffer_t					s_generic_command_buffer[BUFFERED_FRAMES];
size									s_front_cmdbuff;
size									s_back_cmdbuff;
bool									s_waiting_for_swap;
// shading--------------------------------------
arena_allocator_t*								g_frame_shader_allocator[BUFFERS_COUNT];
gpu_command_buffer_t							g_shading_command_buffer[BUFFERS_COUNT];
// ---------------------------------------------

// frame rendering states
render_state_t							s_render_state;
u32										s_render_state_changelog;
// materials are also render states
// TODO: this should be a memory pool of materials
floral::fixed_array<material_t, linear_allocator_t>	s_materials;
material_handle_t						s_current_material;

}
}

#include "insigne/internal_states.h"

namespace insigne {
namespace detail {

// synchronization
floral::condition_variable						g_init_condvar;
floral::mutex									g_init_mtx;
floral::condition_variable						g_cmdbuffer_condvar;
floral::mutex									g_cmdbuffer_mtx;

size											g_front_cmdbuff;
size											g_back_cmdbuff;

// render---------------------------------------
arena_allocator_t*								g_frame_render_allocator[BUFFERS_COUNT];
arena_allocator_t*								g_frame_draw_allocator[BUFFERS_COUNT];
gpu_command_buffer_t							g_render_command_buffer[BUFFERS_COUNT];
// shading--------------------------------------
arena_allocator_t*								g_frame_shader_allocator[BUFFERS_COUNT];
gpu_command_buffer_t							g_shading_command_buffer[BUFFERS_COUNT];
// buffers--------------------------------------
arena_allocator_t*								g_frame_buffers_allocator[BUFFERS_COUNT];
gpu_command_buffer_t							g_buffers_command_buffer[BUFFERS_COUNT];
// textures-------------------------------------
arena_allocator_t*								g_frame_textures_allocator[BUFFERS_COUNT];
gpu_command_buffer_t							g_textures_command_buffer[BUFFERS_COUNT];
// ---------------------------------------------

bool											s_waiting_for_swap;

}
}

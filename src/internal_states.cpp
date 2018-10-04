#include "insigne/internal_states.h"

namespace insigne {
namespace detail {

// synchronization
floral::condition_variable						g_init_condvar;
floral::mutex									g_init_mtx;

size											g_composing_cmdbuff;
std::atomic<bool>								g_scene_presented;
waiting_cmdbuffs_t								g_waiting_cmdbuffs;

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

}
}

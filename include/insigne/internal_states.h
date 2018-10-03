#pragma once

#include <floral.h>

#include "insigne/commons.h"
#include "insigne/buffers.h"
#include "insigne/memory.h"

namespace insigne {
namespace detail {

// synchronization
extern floral::condition_variable				g_init_condvar;
extern floral::mutex							g_init_mtx;
extern floral::condition_variable				g_cmdbuffer_condvar;
extern floral::mutex							g_cmdbuffer_mtx;

extern size										g_front_cmdbuff;
extern size										g_back_cmdbuff;

// draw-----------------------------------------
/*
 * these command buffers will need to be defined in user code
 */
typedef floral::fixed_array<gpu_command, linear_allocator_t>	gpu_command_buffer_t;
template <typename t_surface>
struct draw_command_buffer_t {
	static gpu_command_buffer_t					command_buffer[BUFFERS_COUNT];
};

// render---------------------------------------
extern arena_allocator_t*						g_frame_render_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_render_command_buffer[BUFFERS_COUNT];
// shading--------------------------------------
extern arena_allocator_t*						g_frame_shader_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_shading_command_buffer[BUFFERS_COUNT];
// buffers--------------------------------------
extern arena_allocator_t*						g_frame_buffers_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_buffers_command_buffer[BUFFERS_COUNT];
// textures-------------------------------------
extern arena_allocator_t*						g_frame_textures_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_textures_command_buffer[BUFFERS_COUNT];
// ---------------------------------------------

extern bool										s_waiting_for_swap;

}
}

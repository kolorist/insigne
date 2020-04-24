#pragma once

#include <atomic>

#include <floral/stdaliases.h>
#include <floral/containers/fast_array.h>

#include "insigne/commons.h"
#include "insigne/buffers.h"
#include "insigne/memory.h"

namespace insigne
{
namespace detail
{
// -------------------------------------------------------------------

// synchronization
extern floral::condition_variable				g_init_condvar;
extern floral::mutex							g_init_mtx;

extern size										g_composing_cmdbuff;
extern std::atomic_bool							g_scene_presented;
extern std::atomic<u64>							g_processed_renderpasses;
extern std::atomic<u64>							g_submitted_renderpasses;
#if !defined(USE_BUSY_LOCK)
extern floral::condition_variable				g_scene_presented_condvar;
extern floral::mutex							g_scene_presented_mtx;
#endif
extern std::atomic_bool							g_context_dirty;

typedef floral::inplaced_ring_buffer_mt<size, BUFFERS_COUNT - 1> waiting_cmdbuffs_t;
extern waiting_cmdbuffs_t						g_waiting_cmdbuffs;
#if !defined(USE_BUSY_LOCK)
extern floral::condition_variable				g_waiting_cmdbuffs_condvar;
extern floral::mutex							g_waiting_cmdbuffs_mtx;
#endif

struct cmdbuffs_t
{
	gpu_command_buffer_t						command_buffer[BUFFERS_COUNT];
	states_setup_func_t							states_setup_func;
	vertex_data_setup_func_t					vertex_data_setup_func;
	geometry_mode_e								geometry_mode;
};

// render-------------------------------------------------------------
extern arena_allocator_t*						g_frame_render_allocator[BUFFERS_COUNT];
extern arena_allocator_t*						g_frame_draw_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_render_command_buffer[BUFFERS_COUNT];
// draw---------------------------------------------------------------
using command_buffers_array_t = floral::fast_fixed_array<cmdbuffs_t, linear_allocator_t>;
extern floral::mutex							g_draw_config_mtx;
extern command_buffers_array_t					g_draw_command_buffers;
extern floral::mutex							g_post_draw_config_mtx;
extern command_buffers_array_t					g_post_draw_command_buffers;
// shading------------------------------------------------------------
extern arena_allocator_t*						g_frame_shader_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_shading_command_buffer[BUFFERS_COUNT];
// buffers------------------------------------------------------------
extern arena_allocator_t*						g_frame_buffers_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_buffers_command_buffer[BUFFERS_COUNT];
// textures-----------------------------------------------------------
extern arena_allocator_t*						g_frame_textures_allocator[BUFFERS_COUNT];
extern gpu_command_buffer_t						g_textures_command_buffer[BUFFERS_COUNT];

// -------------------------------------------------------------------
}
}
